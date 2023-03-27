/*
 * @Description:
 * @LastEditTime: 2023-03-28 01:05:34
 */
#include "../src/include/concurrency/io_manager.h"

#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include "../src/include/log/log_manager.h"

auto logger = ROOT_LOGGER;

void test_fiber() {
    // 获取socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // 将sock设置为非阻塞模式（访问失败不会阻塞而是返回错误码）
    fcntl(sock, F_SETFL, O_NONBLOCK);
    sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    inet_pton(AF_INET, "110.242.68.66", &addr.sin_addr.s_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    if (!connect(sock, (sockaddr*)&addr, sizeof(addr))) {
    } else if (errno == EINPROGRESS) {
        LOG_CUSTOM_INFO(logger, "add event errno = %d", strerror(errno));

        wtsclwq::IOManager::GetCurIOManager()->AddEvent(
            sock, wtsclwq::IOManager::READ, []() { LOG_INFO(logger, "READ"); });

        wtsclwq::IOManager::GetCurIOManager()->AddEvent(
            sock, wtsclwq::IOManager::WRITE, [sock]() {
                LOG_INFO(logger, "WRITE");
                wtsclwq::IOManager::GetCurIOManager()->CancelEvent(
                    sock, wtsclwq::IOManager::READ);
            });
    } else {
        LOG_CUSTOM_INFO(logger, "errno = %d", errno);
    }
}
void test1() {
    wtsclwq::IOManager iom(5, false, "aaa");
    iom.Schedule(test_fiber);
}

wtsclwq::Timer::ptr timer{};
void test_timer() {
    wtsclwq::IOManager iom(2);
    timer = iom.AddTimer(
        1000,
        []() {
            static int i = 0;
            LOG_CUSTOM_INFO(logger, "test_timer i = %d", i);
            if (++i == 5) {
                timer->Reset(2000, false);
            }
            if (i == 10) {
                timer->Cancel();
            }
        },
        true);
}

auto main() -> int {
    // test1();
    test_timer();
    return 0;
}