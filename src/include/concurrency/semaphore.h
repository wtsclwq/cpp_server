/*
 * @Description:
 * @LastEditTime: 2023-03-20 20:41:57
 */
#pragma once
#include <semaphore.h>  //TODO 用C++20的semaphore简化

#include <cstdint>

namespace wtsclwq {
class Semaphore {
  public:
    explicit Semaphore(uint32_t count = 0);

    Semaphore(const Semaphore &) = default;
    Semaphore(Semaphore &&) = default;
    auto operator=(const Semaphore &) -> Semaphore & = delete;
    auto operator=(Semaphore &&) -> Semaphore & = delete;

    ~Semaphore();

    void wait();
    void notify();

  private:
    sem_t m_semaphore{};
};
}  // namespace wtsclwq