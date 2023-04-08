/*
 * @Description:
 * @LastEditTime: 2023-03-29 15:31:57
 */
#include "../include/concurrency/scheduler.h"

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
static thread_local Fiber* t_schedule_fiber = nullptr;

Scheduler::Scheduler(size_t thread_num, bool use_builder, std::string name)
    : m_name(std::move(name)), m_is_use_builder(use_builder) {
    WTSCLWQ_ASSERT(thread_num > 0, "thread_num is 0");
    if (use_builder) {
        --thread_num;
        // 创建者线程生成主协程(cur协程)
        Fiber::GetCurFiber();
        WTSCLWQ_ASSERT(GetThisThreadScheduler() == nullptr,
                       "创建者线程内存在调度器");
        t_scheduler = this;
        // m_root_fiber.reset(new Fiber(std::bind(&Scheduler::Run, this)));
        //?注意lambda的使用,因为Run是实例方法,不同的实例会有不同的效果,所以需要将this绑定进去
        // 创建者线程生成调度协程用来执行this.Run()
        m_root_fiber.reset(new Fiber([this] { Run(); }, 0, false));

        wtsclwq::Thread::SetCurThreadName(m_name);
        // 创建者线程的调度协程不是主协程,而是调度器对象的m_root_fiber
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

auto Scheduler::HasIdleThread() const -> bool {
    return m_idle_thread_count > 0;
}

auto Scheduler::GetThisThreadScheduler() -> Scheduler* { return t_scheduler; }

auto Scheduler::GetScheduleFiber() -> Fiber* { return t_schedule_fiber; }

void Scheduler::Start() {
    ScopedLock<MutexType> lock(m_mutex);
    // 不是stopping状态==>运行中
    if (!m_is_stop) {
        return;
    }
    m_is_stop = false;
    m_threads_vec.resize(m_thread_count);
    for (size_t i = 0; i != m_thread_count; ++i) {
        m_threads_vec[i].reset(
            new Thread([this] { Run(); }, m_name + "_" + std::to_string(i)));
        // 由于wtsclwq::Thread的实现中使用了信号量来实现同步，
        // 保证OS::thread启动后，不会立即结束构造函数，
        // 而是在Thread::Run()中初始化wtsclwq::Thread的对象基本信息之后
        // 再结束构造函数,就可以保证这里能拿到OS分配的线程ID
        m_thread_id_vec.push_back(m_threads_vec[i]->GetId());
    }
}

void Scheduler::Stop() {
    if (OnStop()) {
        return;
    }
    m_is_stop = true;

    if (m_is_use_builder) {
        WTSCLWQ_ASSERT(GetThisThreadScheduler() == this,
                       "使用了builder，builder内必定存在调度器，且就是this")
    } else {
        WTSCLWQ_ASSERT(GetThisThreadScheduler() != this,
                       "未使用builder，builder中的调度器应为nullptr");
    }

    for (size_t i = 0; i < m_thread_count; ++i) {
        Tickle();
    }

    if (m_is_use_builder && m_root_fiber != nullptr) {
        Tickle();
        // 如果使用了builder，调度器停止时要使用builder线程的调度协程收尾
        // 如果只有一个builder线程，那么所有的任务都会在此时开始调度
        if (!OnStop()) {
            m_root_fiber->Resume();
        }
    }
    for (const auto& thr : m_threads_vec) {
        thr->Join();
    }
    m_threads_vec.clear();
}

void Scheduler::Run() {
    // 调度器中执行的io系统调用默认为自己hook的版本
    SetHookEnable(true);
    // 所有线程中的Scheduler::Run都是绑定的同一个this
    t_scheduler = this;
    // 如果不是builder线程，那么需要为这些线程初始化主协程
    if (wtsclwq::GetThreadId() != m_root_thread_id) {
        // 无需启动，因为这些线程本身就在执行Run，只是为task_fiber的换回准备条件
        t_schedule_fiber = Fiber::GetCurFiber().get();
    }

    // 任务队列空闲时，用来执行空闲任务的协程
    Fiber::ptr idle_fiber{new Fiber([this] { OnIdle(); })};

    Task task{};
    Fiber::ptr task_fiber;
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
                // 调用被hook的io在检测到未就绪的时，会先添加对应的事件，再yield当前协程，等IO就绪后再resume当前协程
                // 多线程高并发情境下，有可能发生刚添加事件就被触发的情况，如果此时当前协程还未来得及yield，则这里就有可能出现协程状态仍为RUNNING的情况
                // 这里简单地跳过这种情况，当作任务已经被执行了
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
                m_task_list.erase(iter++);
                break;
            }
            // 当前线程拿完一个任务后，发现任务队列还有剩余，那么tickle一下其他线程
            need_tickle |= (iter != m_task_list.end());
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
            LOG_CUSTOM_DEBUG(sys_logger, "线程%s[%d]启动协程%lu执行任务",
                             GetThreadName().c_str(), GetThreadId(),
                             task.fiber->GetId());
            // 任务开始，调度协程-->任务协程
            task.fiber->Resume();
            // 任务退出，活跃线程-1
            --m_active_thread_count;
            task.fiber.reset();
            task.Reset();
        } else {
            // 如果任务内容为空，就去执行idle_fiber
            if (idle_fiber->IsFinish()) {
                // 如果调度器没有调度任务，那么idle协程会不停地resume/yield，不会结束，
                // 如果idle协程结束了，那一定是调度器停止了
                LOG_DEBUG(sys_logger, "idle协程结束");
                break;
            }
            // 调度协程-->idle协程
            ++m_idle_thread_count;
            idle_fiber->Resume();
            --m_idle_thread_count;
        }
    }
    LOG_DEBUG(sys_logger, "调度器Run流程结束");
}

auto Scheduler::OnStop() -> bool {
    ScopedLock<MutexType> lock(m_mutex);
    // m_is_auto_stop是防止调度器空闲状态下自动关闭的关键
    return m_is_stop && m_task_list.empty() && m_active_thread_count == 0;
}

void Scheduler::OnIdle() {
    // 每次从Run回到这里，都会判断是否可以Stop,只有满足了所有的stop条件，才能顺利让idle_fiber成为而term状态
    // 从而保证如果先执行start,再加入任务时，不会出现所有线程都已经结束，没有人做任务的情况
    while (!OnStop()) {
        wtsclwq::Fiber::GetCurFiber()->Yield();
    }
}

void Scheduler::Tickle() { LOG_INFO(sys_logger, "基类的Tickle(),不做动作"); }

}  // namespace wtsclwq
