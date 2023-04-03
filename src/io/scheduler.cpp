/*
 * @Description:
 * @LastEditTime: 2023-03-29 15:31:57
 */
#include "../include/io/scheduler.h"

#include <unistd.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "../include/io/hook.h"
#include "../include/log/log_manager.h"
#include "../include/util/macro.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_schedule_fiber = nullptr;  // 调度器线程的主协程

Scheduler::Scheduler(size_t thread_num, bool use_caller, std::string name)
    : m_name(std::move(name)) {
    WTSCLWQ_ASSERT(thread_num > 0, "thread_num is 0");
    // !这个地方的关键点在于，是否把创建者线程放到协程调度器管理的线程池中。
    // !如果不放入，创建者线程是负责调度器的创建,启动和销毁,线程内没有调度器(t_scheduler=nullptr)
    // !如果放的话，那就要把协程调度器封装到一个协程中，称之为主协程或协程调度器协程。
    // 如果调用[调度器构造函数]的线程(创建者线程)作为调度器的[宿主线程],
    // 那么那么就要将线程id、调度协程绑定到调度器对象中,原定的目标线程数就可以-1
    if (use_caller) {
        // 创建者线程生成主协程(cur协程)
        Fiber::GetCurFiber();
        --thread_num;
        WTSCLWQ_ASSERT(GetThisThreadScheduler() == nullptr,
                       "创建者线程内存在调度器");
        t_scheduler = this;
        // m_root_fiber.reset(new Fiber(std::bind(&Scheduler::Run, this)));
        //?注意lamda的使用,因为Run是实例方法,不同的实例会有不同的效果,所以需要bind
        // 创建者线程生成子协程,作为创建者线程的调度器工作协程,该调度线程用来执行this.Run()
        // 协程创建时会取得当先线程的上下文，其中包括thread_local的变量
        m_root_fiber.reset(new Fiber([this] { Run(); }));
        wtsclwq::Thread::SetCurThreadName(m_name);
        // 创建者线程的调度协程不是main协程,而是调度器对象的m_root_fiber
        t_schedule_fiber = m_root_fiber.get();
        // 将创建者线程id存储起来,为什么其他线程不需要存储呢?
        // 因为线程池内所有线程的调度器都是同一个,因此线程池内的其他线程都可以使用这个属性获取到创建者线程id
        m_root_thread_id = wtsclwq::GetThreadId();
        m_thread_id_vec.push_back(m_root_thread_id);
    } else {
        // 如果不使用创建者协程,那么就不需要持有其id
        // 因为该线程只会负责调度,不需要绑定到调度器对象内
        m_root_thread_id = -1;
    }
    m_thread_count = thread_num;
}

Scheduler::~Scheduler() {
    WTSCLWQ_ASSERT(m_is_stop, "调度器未停止");
    if (GetThisThreadScheduler() == this) {
        t_scheduler = nullptr;
    }
}

auto Scheduler::GetName() const -> std::string { return m_name; }

auto Scheduler::GetMutex() const -> MutexType& { return m_mutex; }

auto Scheduler::IsHasIdleThread() const -> bool {
    return m_idle_thread_count > 0;
}

auto Scheduler::GetThisThreadScheduler() -> Scheduler* { return t_scheduler; }

auto Scheduler::GetScheduleFiber() -> Fiber* { return t_schedule_fiber; }

void Scheduler::Start() {
    {
        ScopedLock<MutexType> lock(m_mutex);
        // 不是stoping状态==>运行中
        if (!m_is_stop) {
            return;
        }

        m_is_stop = false;
        // WTSCLWQ_ASSERT(m_threads_vec.empty(), "调度器启动时线程池非空");
        m_threads_vec.resize(m_thread_count);
        for (size_t i = 0; i != m_thread_count; ++i) {
            // m_threads_vec[i].reset(new Thread(std::bind(&Scheduler::Run,
            // this),m_name + std::to_string(i)));
            //? 注意lamada的使用
            m_threads_vec[i].reset(
                new Thread([this] { Run(); }, m_name + std::to_string(i)));
            // 由于wtsclwq::Thread的实现中使用了信号量来实现同步，
            // 保证OS::thread启动后，不会立即结束构造函数，
            // 而是在Thread::Run()中初始化wtsclwq::Thread的对象基本信息之后
            // 再结束构造函数,就可以保证这里能拿到OS分配的线程ID
            m_thread_id_vec.push_back(m_threads_vec[i]->GetId());
        }
    }
}

void Scheduler::Stop() {  // NOLINT
    m_is_auto_stop = true;
    // user_caller为true时且thread_num为1时，说明调度器中只有一个线程(创建者线程)
    if (m_root_fiber && m_thread_count == 0 &&
        (m_root_fiber->GetState() == Fiber::TERM ||
         m_root_fiber->GetState() == Fiber::INIT)) {
        m_is_stop = true;
        if (OnStop()) {
            return;
        }
    }

    if (m_root_thread_id != -1) {
        WTSCLWQ_ASSERT(GetThisThreadScheduler() == this,
                       "使用了use_caller，创建者线程内调度器与欲注销调度器不符")
    }

    if (m_root_thread_id == -1) {
        WTSCLWQ_ASSERT(
            GetThisThreadScheduler() == nullptr,
            "未使用use_caller，创建者线程中的t_schedeler应为nullptr");
    }

    m_is_stop = true;
    for (size_t i = 0; i < m_thread_count; ++i) {
        Tickle();
    }

    if (m_root_fiber) {
        Tickle();
        if (!OnStop()) {
            // 如果usr_caller,那么调度器无法停止的时候需要把创建者线程切回调度协程
            // 调用Stop()后发现任务队列非空，让scheduer_fiber处理一次
            LOG_CUSTOM_ERROR(sys_logger, "创建者线程%d调用Stop()时任务队列非空",
                             GetThreadId());
            // m_root_fiber->Reset([this] { Run(); });
            m_root_fiber->SwapIn();
        }
    }
    for (const auto& thr : m_threads_vec) {
        thr->Join();
    }
    m_threads_vec.clear();
}

void Scheduler::Run() {
    // 所有线程中的Scheduler::Run都是绑定的同一个this
    t_scheduler = this;
    // 调度器中执行的io系统调用默认为自己hook的版本
    SetHookEnable(true);
    // 分两种情况:
    // 1.use_caller==true,this.m_root_thread_id==创建者线程ID
    // 2.use_caller==false,this.m_root_thread_id==-1
    if (wtsclwq::GetThreadId() != m_root_thread_id) {
        // 如果当前线程不是创建者线程==>也就是线程池中的线程,在线程中创建一个新的main协程
        // 无需启动，因为这些线程本身就在执行Run，只是为task_fiber的换回准备条件
        t_schedule_fiber = Fiber::GetCurFiber().get();
    }

    // 任务队列空闲时，用来执行空闲任务的协程
    Fiber::ptr idle_fiber{new Fiber([this] { OnIdle(); })};

    Task task{};
    while (true) {
        // 每次轮询时重置task状态
        task.Reset();
        bool need_tickle{false};
        {
            ScopedLock<MutexType> lock(m_mutex);
            auto iter{m_task_list.begin()};
            while (iter != m_task_list.end()) {
                // 任务指定了要再哪条线程执行,但是当前线程不是指定线程==>通知目标线程
                auto tar_tid = (*iter)->thread_id;
                if (tar_tid != -1 && tar_tid != GetThreadId()) {
                    ++iter;
                    need_tickle = true;
                    LOG_CUSTOM_DEBUG(sys_logger,
                                     "线程%d取得任务，但任务指定了线程%d",
                                     GetThreadId(), tar_tid);
                    continue;
                }
                WTSCLWQ_ASSERT((*iter)->fiber || (*iter)->func, "任务为空");
                // 任务是一个fiber,但fiber正在运行中的,不进行处理
                if ((*iter)->fiber &&
                    (*iter)->fiber->GetState() == Fiber::EXEC) {
                    ++iter;
                    LOG_CUSTOM_DEBUG(sys_logger,
                                     "线程%d取得任务，但任务已经在运行中",
                                     GetThreadId());
                    continue;
                }
                // 排除掉特殊情况,得到一个可执行的任务,将任务拷贝一份
                task = *(*iter);
                // 线程接到了任务==>活跃线程+1
                ++m_active_thread_count;
                m_task_list.erase(iter);
                break;
            }
        }
        // 如果需要通知其他线程
        if (need_tickle) {
            Tickle();
        }
        // 如果是一个方法任务,将其转变为协程任务
        if (task.func) {
            task.fiber = std::make_shared<Fiber>(std::move(task.func));
            task.func = nullptr;
        }
        // 经过上面的转变，此时剩下两种情况=>1.协程任务 2.任务内容为空
        // 如果：任务内容非空 且 协程中的任务未完成
        if (task.fiber && !task.fiber->IsFinish()) {
            // 任务开始，启动task_fiber
            // 上下文的切换对象是t_schedeler_fiber
            // 但其实非创建者线程的t_schedeler_fiber就是t_main_fiber,只有创建者线程比较特殊
            LOG_CUSTOM_DEBUG(sys_logger, "线程%d启动协程%lu执行任务主体",
                             GetThreadId(), task.fiber->GetId());
            task.fiber->SwapInFromScheduler();
            // 任务退出==>活跃线程-1
            --m_active_thread_count;
            Fiber::State fiber_state = task.fiber->GetState();
            // 检查协程的退出方式，如果是YiledToReady,那么任务继续放到队列中
            // 如果是EXCEPT或者TERM就不做任何处理
            // 如果是YiledToHold或者其他情况就将协程状态设置为HOLD
            if (fiber_state == Fiber::READY) {
                Schedule(std::move(task.fiber), task.thread_id);
            } else if (fiber_state != Fiber::EXCEPT &&
                       fiber_state != Fiber::TERM) {
                task.fiber->SetState(Fiber::HOLD);
            }
            task.Reset();
        } else {
            // 如果任务内容为空，就去执行idle_fiber
            if (idle_fiber->IsFinish()) {
                break;
            }
            ++m_idle_thread_count;
            // 启动空闲处理协程，上下文切换对象的情况分类与task_fiber一样
            idle_fiber->SwapInFromScheduler();
            --m_idle_thread_count;
            // 如果协程不是正常退出
            if (!idle_fiber->IsFinish()) {
                idle_fiber->SetState(Fiber::HOLD);
            }
        }
    }
}
auto Scheduler::HasIdleThread() const -> bool {
    return m_idle_thread_count != 0;
}

auto Scheduler::OnStop() -> bool {
    ScopedLock<MutexType> lock(m_mutex);
    // m_is_auto_stop是防止调度器空闲状态下自动关闭的关键
    return m_is_auto_stop && m_task_list.empty() && m_active_thread_count == 0;
}

void Scheduler::OnIdle() {
    // 每次从Run回到这里，都会判断是否可以Stop,只有满足了所有的stop条件，才能顺利让idle_fiber成为而term状态
    // 从而保证如果先执行start,再加入任务时，不会出现所有线程都已经结束，没有人做任务的情况
    while (!OnStop()) {
        wtsclwq::Fiber::YieldToHoldBackScheduler();
    }
}

void Scheduler::Tickle() { LOG_INFO(sys_logger, "Base schedule Tickle"); }

}  // namespace wtsclwq
