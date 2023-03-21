/*
 * @Description:
 * @LastEditTime: 2023-03-21 13:53:14
 */
#include "../include/concurrency/thread.h"

#include <pthread.h>
#include <sched.h>

#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

#include "../include/log/log_manager.h"

namespace wtsclwq {
const int THREAD_NAME_SIZE = 15;

static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

static thread_local Thread* t_cur_thread = nullptr;
static thread_local std::string t_cur_thread_name = "default";

auto Thread::GetCurThread() -> Thread* { return t_cur_thread; }

auto Thread::GetCurThreadName() -> std::string { return t_cur_thread_name; }

void Thread::SetCurThreadName(std::string name) {
    if (name.empty()) {
        return;
    }
    if (t_cur_thread != nullptr) {
        t_cur_thread->m_name = name;
    }
    t_cur_thread_name = std::move(name);
}

/* ****************************************************************** */
/* ****************************************************************** */
/* ****************************************************************** */

Thread::Thread(std::function<void()> call_back, std::string name)
    : m_call_back(std::move(call_back)), m_name(std::move(name)) {
    // TODO(wtsclwq): 将this指针中的数据封装成一个对象作为参数，而不是传递this
    int flag = pthread_create(&m_thread, nullptr, &Thread::Run, this);
    if (flag != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "pthread_create error, return %d, name = %s", flag,
                         name.c_str());

        throw std::logic_error("pthread_create error");
    }
    //! 确保构造函数结束之前，线程能成功启动
    m_semphore.wait();
}

Thread::~Thread() {
    if (m_thread != 0) {
        pthread_detach(m_thread);
    }
}

auto Thread::GetName() const -> std::string { return m_name; }

auto Thread::GetId() const -> pid_t { return m_id; }

void Thread::Join() {
    if (m_thread != 0) {
        int flag = pthread_join(m_thread, nullptr);
        if (flag != 0) {
            LOG_CUSTOM_ERROR(sys_logger,
                             "pthread_join error, return %d, name = %s", flag,
                             m_name.c_str());
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

auto Thread::Run(void* arg) -> void* {
    // Run的this指针以参数的形式传进来，从而能获取到其他成员的信息
    auto* thread = static_cast<Thread*>(arg);
    // 将当前线程设置为this
    t_cur_thread = thread;
    t_cur_thread_name = thread->m_name;
    // 拿到OS分配的线程ID
    thread->m_id = wtsclwq::GetThreadId();
    pthread_setname_np(pthread_self(),
                       thread->m_name.substr(0, THREAD_NAME_SIZE).c_str());

    std::function<void()> call_back;

    //? 为什么不直接执行thread->m_call_back()
    // 为了避免thread->m_call_back被多个线程同时执行
    call_back.swap(thread->m_call_back);

    // !线程状态初始化完成，准备启动时，通知构造函数（处于主线程）
    thread->m_semphore.notify();
    call_back();
    return nullptr;
}
}  // namespace wtsclwq