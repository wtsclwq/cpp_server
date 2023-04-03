/**
 * @file test_socket_tcp_client.cc
 * @brief 测试Socket类，tcp客户端
 * @version 0.1
 * @date 2021-09-18
 */
#include "../src/include/log/log_manager.h"
#include "../src/include/server/tcp_server.h"
#include "../src/include/socket/ip_address.h"

static wtsclwq::Logger::ptr g_logger = ROOT_LOGGER;

void test_tcp_client() {
    int ret;

    auto socket = wtsclwq::Socket::CreateIpv4TcpSocket();
    assert(socket);

    auto addr = wtsclwq::IPAddress::LookupAnyAddress("0.0.0.0:12345");
    assert(addr);

    ret = socket->Connect(addr, UINT64_MAX);
    assert(ret);

    LOG_INFO(g_logger, "connect success, peer address: " +
                           socket->GetRemoteAddress()->ToString());

    std::string buffer;
    buffer.resize(1024);
    socket->Recv(&buffer[0], buffer.size(), 0);
    LOG_INFO(g_logger, "recv: " + buffer);
    socket->Close();
    return;
}

int main(int argc, char *argv[]) {
    wtsclwq::IOManager iom;
    iom.Schedule(&test_tcp_client);
    return 0;
}