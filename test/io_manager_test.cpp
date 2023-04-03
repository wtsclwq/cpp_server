/*
 * @Description:
 * @LastEditTime: 2023-03-29 18:48:12
 */
#include "../src/include/io/io_manager.h"

#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "../src/include/io/hook.h"
#include "../src/include/log/log_manager.h"

auto logger = ROOT_LOGGER;

void TestFiber() {
    // 获取socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // 将sock设置为非阻塞模式（访问失败不会阻塞而是返回错误码）
    fcntl(sock, F_SETFL, O_NONBLOCK);
    sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        if (errno == EINPROGRESS) {
            LOG_CUSTOM_INFO(logger, "add event errno = %s", strerror(errno));

            wtsclwq::IOManager::GetCurIOManager()->AddEvent(
                sock, wtsclwq::READ, []() { LOG_INFO(logger, "READ"); });

            wtsclwq::IOManager::GetCurIOManager()->AddEvent(
                sock, wtsclwq::WRITE, [sock]() {
                    LOG_INFO(logger, "WRITE");
                    wtsclwq::IOManager::GetCurIOManager()->CancelEvent(
                        sock, wtsclwq::READ);
                });
        } else {
            LOG_CUSTOM_INFO(logger, "errno = %d", errno);
        }
    } else {
    }
}
void Test1() {
    wtsclwq::IOManager iom(5, false, "aaa");
    iom.Schedule(TestFiber);
}

wtsclwq::Timer::ptr timer{};
void TestTimer() {
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

void TestHook() {
    wtsclwq::IOManager iom(1);
    iom.Schedule([]() {
        sleep(2);
        LOG_INFO(logger, "sleep 2");
    });
    iom.Schedule([]() {
        sleep(4);
        LOG_INFO(logger, "sleep 4 ");
    });
    LOG_INFO(logger, "test hook");
}

void TestSock() {
    // wtsclwq::SetHookEnable(true);
    // 获取socket
    LOG_INFO(logger, "test hook start");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "13.249.167.72", &addr.sin_addr.s_addr);

    int ret = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    LOG_CUSTOM_INFO(logger, "connect ret = %d, error = %d", ret, errno);
    if (ret != 0) {
        return;
    }
    const char data[] = "GET /HTTP/1.0\r\n\r\n";
    ret = static_cast<int>(send(sock, data, sizeof(data), 0));
    LOG_CUSTOM_INFO(logger, "send ret = %d, errno = %d", ret, errno);
    if (ret <= 0) {
        return;
    }

    std::string buff;
    buff.resize(10240);
    ret = static_cast<int>(recv(sock, &buff[0], buff.size(), 0));
    LOG_CUSTOM_INFO(logger, "recv ret = %d, errno = %d", ret, errno);
    if (ret <= 0) {
        return;
    }
    buff.resize(ret + 1);
    LOG_INFO(logger, buff);
    LOG_INFO(logger, "test hook end");
}
auto main() -> int {
    // test1();
    // test_timer();
    // test_hook();
    //    TestTimer();
    wtsclwq::IOManager iom(2, false,"aaa");
    std::this_thread::sleep_for(std::chrono::seconds (10));
    return 0;
}