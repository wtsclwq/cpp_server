//
// Created by wtsclwq on 23-4-1.
//
#include "../src/include/socket/socket.h"

#include "../src/include/io/io_manager.h"
#include "../src/include/log/log_manager.h"
#include "../src/include/socket/ip_address.h"

wtsclwq::Logger::ptr logger = GET_LOGGER_BY_NAME("system");
void TestSocket() {
    std::vector<wtsclwq::Address::ptr> result;
    wtsclwq::Address::Lookup(result, "www.baidu.com", AF_INET);
    wtsclwq::IPAddress::ptr addr;
    for (auto& i : result) {
        LOG_INFO(logger, i->ToString());
        addr = std::dynamic_pointer_cast<wtsclwq::IPAddress>(i);
        if (addr) {
            break;
        }
    }
    if (addr != nullptr) {
        LOG_CUSTOM_INFO(logger, "get address %s", addr->ToString().c_str())
    } else {
        LOG_ERROR(logger, "get address error");
    }
    wtsclwq::Socket::ptr sock = wtsclwq::Socket::CreateTcpSocket(addr);
    addr->SetPort(80);
    if (!sock->Connect(addr, UINT64_MAX)) {
        LOG_CUSTOM_ERROR(logger, "connect to %s fail", addr->ToString().c_str())
    } else {
        LOG_CUSTOM_INFO(logger, "connect to %s success",
                        addr->ToString().c_str())
    }
    const char buff[] = "GET / HTTP/1.1\r\n\r\n";
    ssize_t ret = sock->Send(buff, sizeof(buff), 0);
    if (ret <= 0) {
        LOG_CUSTOM_ERROR(logger, "send fail ret = %ld", ret)
        return;
    }
    std::string buffers;
    buffers.resize(4096);
    ret = sock->Recv(buffers.data(), buffers.size(), 0);
    if (ret <= 0) {
        LOG_CUSTOM_ERROR(logger, "recv fail ret = %ld", ret)
        return;
    }
    buffers.resize(ret);
    LOG_INFO(logger, buffers);
}
auto main() -> int {
    wtsclwq::IOManager iom;
    iom.Schedule(TestSocket);
    return 0;
}