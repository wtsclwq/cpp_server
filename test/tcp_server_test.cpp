/**
 * @file test_socket.cc
 * @brief 测试Socket类，tcp服务器
 * @version 0.1
 * @date 2021-09-18
 */

#include "../src/include/server/tcp_server.h"

#include "../src/include/log/log_manager.h"
#include "../src/include/socket/ip_address.h"

static wtsclwq::Logger::ptr g_logger = ROOT_LOGGER;

void test_tcp_server() {
    int ret;

    auto addr = wtsclwq::IPAddress::LookupAnyAddress("0.0.0.0:12345");
    assert(addr);

    auto socket = wtsclwq::Socket::CreateIpv4TcpSocket();
    assert(socket);

    ret = socket->Bind(addr);
    assert(ret);

    LOG_INFO(g_logger, "bind success");

    ret = socket->Listen(SOMAXCONN);
    assert(ret);

    LOG_INFO(g_logger, socket->ToString().c_str());
    LOG_INFO(g_logger, "listening");

    while (1) {
        auto client = socket->Accept();
        assert(client);
        LOG_INFO(g_logger, "new client:" + client->ToString());
        client->Send("hello world", strlen("hello world"), 0);
        client->Close();
    }
}

int main(int argc, char *argv[]) {
    wtsclwq::IOManager iom(2);
    iom.Schedule(&test_tcp_server);

    return 0;
}