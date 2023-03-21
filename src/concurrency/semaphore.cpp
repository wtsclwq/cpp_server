/*
 * @Description: c
 * @LastEditTime: 2023-03-20 20:51:03
 */
#include "../include/concurrency/semaphore.h"

#include <semaphore.h>

#include <stdexcept>

namespace wtsclwq {
Semaphore::Semaphore(uint32_t count) {
    int flag = sem_init(&m_semaphore, 0, count);
    if (flag != 0) {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }

void Semaphore::wait() {
    //! 注意库函数的使用
    if (sem_wait(&m_semaphore) != 0) {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify() {
    //! 注意库函数的使用
    if (sem_post(&m_semaphore) != 0) {
        throw std::logic_error("sem_post error");
    }
}
}  // namespace wtsclwq