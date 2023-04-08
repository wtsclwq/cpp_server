#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"
/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-18 17:44:24
 * @LastEditTime: 2023-03-29 17:39:13
 */
#include "../include/concurrency/fiber.h"

#include <ucontext.h>

#include <atomic>
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

namespace fiber_info {
/// 全局static,自增生成协程id
static std::atomic<uint64_t> s_fiber_id{0};
/// 全局static,统计运行中协程总数
static std::atomic<uint64_t> s_fiber_count{0};

/// 线程static，当前运行的协程
static thread_local Fiber *t_cur_fiber{nullptr};
/// 线程static，线程的主协程，如果要在一个线程中使用协程，那么一定会先创建主协程
/// 由于非对称协程的设计，只能主协程<->子协程，因此主协程必须在线程内可见，便于协程切换
static thread_local Fiber::ptr t_main_fiber{nullptr};

/// 协程栈大小
static ConfigVar<uint32_t>::ptr g_fiber_static_size{Config::Lookup<uint32_t>(
    "fiber.stack_size", FIBER_STACK_SIZE, "fiber stack size")};
}  // namespace fiber_info

/**
 * @brief 内存分配器
 */
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
    : m_id(fiber_info::s_fiber_id++), m_stack_size(0), m_stack(nullptr),
      m_state(EXEC), m_call_back(nullptr), m_running_in_scheduler(false) {
    SetCurFiber(this);
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext error");
    ++fiber_info::s_fiber_count;
}

Fiber::Fiber(std::function<void()> call_back, size_t stack_size,
             bool run_in_scheduler)
    : m_id(fiber_info::s_fiber_id++),
      m_stack_size(stack_size != 0
                       ? stack_size
                       : fiber_info::g_fiber_static_size->GetValue()),
      m_stack(StackAlloctor::Alloc(m_stack_size)), m_state(READY),
      m_call_back(std::move(call_back)),
      m_running_in_scheduler(run_in_scheduler) {
    ++fiber_info::s_fiber_count;
    // 获取栈上下文填充到 m_ctx 中
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext error");
    // 设置上下文内容
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;            // 设置栈空间
    m_ctx.uc_stack.ss_size = m_stack_size;     // 设置栈大小
    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 关联上下文和方法
}

Fiber::~Fiber() {
    --fiber_info::s_fiber_count;
    if (m_stack != nullptr) {  // 子协程有自己的栈空间
        WTSCLWQ_ASSERT(m_state == TERM, "尝试析构一个运行中的协程");
        // 回收栈空间
        StackAlloctor::Dealloc(m_stack, m_stack_size);
    } else {  // 主协程没有自己的栈空间
        WTSCLWQ_ASSERT(m_call_back == nullptr, "主协程没有callback");
        WTSCLWQ_ASSERT(m_state == EXEC, "主协程必定在运行中");
        Fiber *cur = fiber_info::t_cur_fiber;
        // 如果当前运行中的协程是主协程
        if (cur == this) {
            SetCurFiber(nullptr);
        }
    }
}

void Fiber::Reset(std::function<void()> call_back) {
    WTSCLWQ_ASSERT(m_stack != nullptr, "尝试reset主协程");
    WTSCLWQ_ASSERT(m_state == TERM, "尝试reset运行中的协程");

    m_call_back = std::move(call_back);
    int flag = getcontext(&m_ctx);
    WTSCLWQ_ASSERT(flag == 0, "getcontext()错误");

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = READY;
}

void Fiber::Resume() {
    WTSCLWQ_ASSERT(m_state != TERM && m_state != EXEC,
                   "只能resume就绪状态的协程");
    SetCurFiber(this);
    m_state = EXEC;
    if (m_running_in_scheduler) {
        int flag = swapcontext(&(Scheduler::GetScheduleFiber()->m_ctx), &m_ctx);
        WTSCLWQ_ASSERT(flag == 0, "swapcontext()错误");
    } else {
        int flag = swapcontext(&(fiber_info::t_main_fiber->m_ctx), &m_ctx);
        WTSCLWQ_ASSERT(flag == 0, "swapcontext()错误");
    }
}

void Fiber::Yield() {
    // 两种情况：1.协程运行结束，将自身状态设置为TERM  2.协程主动让出CPU，状态为EXEC
    WTSCLWQ_ASSERT(m_state == TERM || m_state == EXEC,
                   "无法yield早就处于READY状态的协程");
    if (m_state != TERM) {
        m_state = READY;
    }
    if (m_running_in_scheduler) {
        SetCurFiber(Scheduler::GetScheduleFiber());
        int flag = swapcontext(&m_ctx, &(Scheduler::GetScheduleFiber()->m_ctx));
        WTSCLWQ_ASSERT(flag == 0, "swapcontext()错误");
    } else {
        SetCurFiber(fiber_info::t_main_fiber.get());
        int flag = swapcontext(&m_ctx, &(fiber_info::t_main_fiber->m_ctx));
        WTSCLWQ_ASSERT(flag == 0, "swapcontext()错误");
    }
}

auto Fiber::GetId() const -> uint64_t { return m_id; }

auto Fiber::GetState() const -> Fiber::State { return m_state; }

void Fiber::SetState(Fiber::State state) { m_state = state; }

auto Fiber::IsFinish() const noexcept -> bool { return m_state == TERM; }

/* ************************************************************** */
/* ************************************************************** */
/* ************************************************************** */

void Fiber::SetCurFiber(Fiber *fiber) { fiber_info::t_cur_fiber = fiber; }

auto Fiber::GetCurFiber() -> Fiber::ptr {
    if (fiber_info::t_cur_fiber != nullptr) {
        return fiber_info::t_cur_fiber->shared_from_this();
    }
    // 如果没有cur协程，说明当前线程还没有创建过协程
    // 这里new Fiber会调用无参构造，其中会SetCurFiber(this)
    // 设置主协程为第一个创建的协程
    fiber_info::t_main_fiber.reset(new Fiber());
    //? return main_fiber;
    // 效果应该相同？因为main_fiber持有的应该就是t_cur_fiber指向的对象
    return fiber_info::t_cur_fiber->shared_from_this();
}

auto Fiber::GetCurFiberId() -> uint64_t {
    // 如果有cur协程（创建过协程）
    if (fiber_info::t_cur_fiber != nullptr) {
        return fiber_info::t_cur_fiber->GetId();
    }
    // 如果没有创建过协程，默认为主协程（0号）
    return 0;
}

auto Fiber::TotalFibers() -> uint64_t { return fiber_info::s_fiber_count; }

void Fiber::MainFunc() {
    Fiber::ptr cur = GetCurFiber();  // 引用计数+1
    WTSCLWQ_ASSERT(cur != nullptr, "当前协程为空");

    cur->m_call_back();
    cur->m_call_back = nullptr;
    cur->m_state = TERM;

    auto raw_ptr = cur.get();
    cur.reset();  // 防止yield之后，引用计数不会-1,所以这里提前释放引用计数

    raw_ptr->Yield();
}
}  // namespace wtsclwq
#pragma clang diagnostic pop