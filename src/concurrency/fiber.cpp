/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-18 17:44:24
 * @LastEditTime: 2023-03-23 23:23:11
 */
#include "../include/concurrency/fiber.h"

#include <ucontext.h>

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <memory>
#include <sstream>

#include "../include/concurrency/scheduler.h"
#include "../include/config/config.h"
#include "../include/log/log_manager.h"
#include "../include/util/macro.h"

namespace wtsclwq {

static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

namespace FiberInfo {
static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};

static thread_local Fiber *t_cur_fiber{nullptr};
static thread_local Fiber::ptr t_main_fiber{nullptr};

static ConfigVar<uint32_t>::ptr g_fiber_static_size{Config::Lookup<uint32_t>(
    "fiber.stack_size", FIBER_STACK_SIZE, "fiber stack size")};
}  // namespace FiberInfo

class MallocStackAllocator {
  public:
    static auto Alloc(size_t size) -> void * {
        return std::malloc(size);  // NOLINT
    }
    static void Dealloc(void *pointer, size_t /*size*/) {
        std::free(pointer);  // NOLINT
    }
};
using StackAlloctor = MallocStackAllocator;

Fiber::Fiber()
    : m_id(FiberInfo::s_fiber_id++), m_stack_size(0), m_stack(nullptr),
      m_state(EXEC), m_call_back(nullptr) {
    SetCurFiber(this);
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext error");
    ++FiberInfo::s_fiber_count;

    LOG_CUSTOM_DEBUG(sys_logger, "协程 %lu 创建成功", m_id);
}

Fiber::Fiber(std::function<void()> call_back, size_t stack_size)
    : m_id(FiberInfo::s_fiber_id++),
      m_stack_size(stack_size != 0
                       ? stack_size
                       : FiberInfo::g_fiber_static_size->GetValue()),
      m_stack(StackAlloctor::Alloc(m_stack_size)), m_state(INIT),
      m_call_back(std::move(call_back)) {
    ++FiberInfo::s_fiber_count;
    // 获取栈上下文填充到 m_ctx 中
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext error");
    // 设置上下文内容
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;            // 设置栈空间
    m_ctx.uc_stack.ss_size = m_stack_size;     // 设置栈大小
    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 关联上下文和方法
    LOG_CUSTOM_DEBUG(sys_logger, "协程 %lu 创建成功", m_id);
}

Fiber::~Fiber() {
    --FiberInfo::s_fiber_count;
    if (m_stack != nullptr) {  // 子协程有自己的栈空间
        WTSCLWQ_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT,
                       "try to deconstract a activ fiber");
        // 回收栈空间
        StackAlloctor::Dealloc(m_stack, m_stack_size);
    } else {  // 主协程没有自己的栈空间

        // 主协程没有callback函数
        WTSCLWQ_ASSERT(!m_call_back, "call back func is null");
        // 主协程一定出处于exec状态
        WTSCLWQ_ASSERT(m_state == EXEC, "fiber is not exec");
        Fiber *cur = FiberInfo::t_cur_fiber;
        // 如果当前运行中的协程是主协程
        if (cur == this) {
            SetCurFiber(nullptr);
        }
    }
    LOG_CUSTOM_DEBUG(sys_logger, "协程 %lu 析构成功", m_id);  // NOLINT
}

void Fiber::Reset(std::function<void()> call_back) {
    WTSCLWQ_ASSERT(m_stack != nullptr, "stack is null, it is main fiber");
    WTSCLWQ_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT,
                   "fiber is running");

    m_call_back = std::move(call_back);
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext error");

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    m_state = INIT;
}

void Fiber::SwapIn() {
    WTSCLWQ_ASSERT(FiberInfo::t_main_fiber, "当前线程不存在主协程");
    WTSCLWQ_ASSERT(m_state == INIT || m_state == READY || m_state == HOLD,
                   "只有协程是等待执行的状态才能SwapIn()");
    SetCurFiber(this);
    // 协程状态改为EXEC
    m_state = EXEC;
    // 交换上下文
    int flag = swapcontext(&(FiberInfo::t_main_fiber->m_ctx), &m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "swapcontext error");
}

void Fiber::SwapInFromScheduler() {
    WTSCLWQ_ASSERT(Scheduler::GetScheduleFiber(), "Scheduler协程为空");
    WTSCLWQ_ASSERT(m_state == INIT || m_state == READY || m_state == HOLD,
                   "只有协程是等待执行的状态才能SwapIn()");
    SetCurFiber(this);
    // 协程状态改为EXEC
    m_state = EXEC;
    // 交换上下文
    int flag = swapcontext(&(Scheduler::GetScheduleFiber()->m_ctx), &m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "swapcontext error");
}

void Fiber::SwapOut() {
    WTSCLWQ_ASSERT(FiberInfo::t_main_fiber, "当前线程不存在主协程");
    WTSCLWQ_ASSERT(m_stack != nullptr, "协程栈指针为null")
    SetCurFiber(FiberInfo::t_main_fiber.get());
    int flag = swapcontext(&m_ctx, &(FiberInfo::t_main_fiber->m_ctx));
    WTSCLWQ_ASSERT(flag == 0, "swapcontext error");
}

void Fiber::SwapOutBackScheduler() {
    WTSCLWQ_ASSERT(Scheduler::GetScheduleFiber(), "Scheduler协程为空");
    WTSCLWQ_ASSERT(m_stack != nullptr, "协程栈指针为null")
    SetCurFiber(Scheduler::GetScheduleFiber());
    int flag = swapcontext(&m_ctx, &(Scheduler::GetScheduleFiber()->m_ctx));
    WTSCLWQ_ASSERT(flag == 0, "swapcontext error");
}

auto Fiber::GetId() const -> uint64_t { return m_id; }

auto Fiber::GetState() const -> Fiber::State { return m_state; }

void Fiber::SetState(Fiber::State state) { m_state = state; }

auto Fiber::IsFinish() const noexcept -> bool {
    return m_state == TERM || m_state == EXCEPT;
}

/* ************************************************************** */
/* ************************************************************** */
/* ************************************************************** */

void Fiber::SetCurFiber(Fiber *fiber) { FiberInfo::t_cur_fiber = fiber; }

auto Fiber::GetCurFiber() -> Fiber::ptr {
    if (FiberInfo::t_cur_fiber != nullptr) {
        return FiberInfo::t_cur_fiber->shared_from_this();
    }
    // 如果没有cur协程，说明当前线程还没有创建过协程
    // 这里new Fiber会调用无参构造，其中会SetCurFiber(this)
    // 设置主协程为第一个创建的协程
    FiberInfo::t_main_fiber.reset(new Fiber());
    //? return main_fiber;
    // 效果应该相同？因为main_fiber持有的应该就是t_cur_fiber指向的对象
    return FiberInfo::t_cur_fiber->shared_from_this();
}

auto Fiber::GetCurFiberId() -> uint64_t {
    // 如果有cur协程（创建过协程）
    if (FiberInfo::t_cur_fiber != nullptr) {
        return FiberInfo::t_cur_fiber->GetId();
    }
    // 如果没有创建过协程，默认为主协程（0号）
    return 0;
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetCurFiber();
    cur->m_state = READY;
    cur->SwapOut();
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = GetCurFiber();
    WTSCLWQ_ASSERT(cur->m_state == EXEC, "try to yield a not runing fiber");
    cur->m_state = HOLD;
    cur->SwapOut();
}

auto Fiber::TotalFibers() -> uint64_t { return FiberInfo::s_fiber_count; }

void Fiber::MainFunc() {
    Fiber::ptr cur_smart_ptr = GetCurFiber();
    WTSCLWQ_ASSERT(cur_smart_ptr != nullptr, "current fiber is nullptr");
    try {
        cur_smart_ptr->m_call_back();  // 调用回调函数
        cur_smart_ptr->m_call_back = nullptr;
        cur_smart_ptr->m_state = TERM;
    } catch (std::exception &exp) {
        cur_smart_ptr->m_state = EXCEPT;
        std::stringstream sstream;
        sstream << "Fiber Except: " << exp.what();
        LOG_ERROR(sys_logger, sstream.str());
    } catch (...) {
        cur_smart_ptr->m_state = EXCEPT;
        LOG_ERROR(sys_logger, "Fiber Except: Unknown");
    }
    // 释放智能指针，防止切换协程后引用计数未-1
    // SwapOut会直接切换执行栈,所以不会正常执行方法内对象的析构
    Fiber *cur_raw_ptr = cur_smart_ptr.get();
    cur_smart_ptr.reset();

    auto *scheduler = Scheduler::GetThisThreadScheduler();
    // 1.  对于用户自定义协程，线程内不存在调度器，因此直接swap回t_main_fiber
    // 2.  使用调度器时：
    //                所有的task_fiber都应该swap回t_scheduler_fiber，
    //                只有创建者线程的t_scheduler_fiber才需要swap回t_main_fiber
    // （如果use_caller=false,那么scheduler->m_root_fiber.get()应该是nullptr）
    if (scheduler == nullptr || scheduler->m_root_fiber.get() == cur_raw_ptr) {
        cur_raw_ptr->SwapOut();
    } else {
        cur_raw_ptr->SwapOutBackScheduler();
    }
    WTSCLWQ_ASSERT(false, "永不到达");
}
}  // namespace wtsclwq