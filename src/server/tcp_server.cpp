//
// Created by wtsclwq on 23-4-3.
//

#include "../include/server/tcp_server.h"

#include "../include/config/config.h"
#include "../include/log/log_manager.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");
static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
    Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
                   "tcp server read timeout");

TcpServer::TcpServer()
    : m_name("wtsclwq/1.0"), m_type("TCP"), m_is_stop(true), m_recv_timeout(),
      m_worker(IOManager::GetThisThreadIOManager()),
      m_acceptor(IOManager::GetThisThreadIOManager()) {}

TcpServer::TcpServer(IOManager *worker, IOManager *acceptor)
    : m_name("wtsclwq/1.0"), m_type("TCP"), m_is_stop(true), m_recv_timeout(),
      m_worker(worker), m_acceptor(acceptor) {}

TcpServer::~TcpServer() {
    for (auto &i : m_sockets) {
        i->Close();
    }
    m_sockets.clear();
}

auto TcpServer::Bind(const Address::ptr &address) -> bool {
    std::vector<Address::ptr> address_vec;
    std::vector<Address::ptr> fail_vec;
    address_vec.push_back(address);
    return BindVec(address_vec, fail_vec);
}

auto TcpServer::BindVec(const std::vector<Address::ptr> &address_vec,
                        std::vector<Address::ptr> &fails_vec) -> bool {
    for (auto &addr : address_vec) {
        // 服务端socket，用来监听连接
        Socket::ptr socket = Socket::CreateTcpSocket(addr);
        if (!socket->Bind(addr)) {
            LOG_CUSTOM_ERROR(sys_logger,
                             "bind fail errno = %d, errstr = %s, addr = [%s]",
                             errno, strerror(errno), addr->ToString().c_str())
            fails_vec.push_back(addr);
            continue;
        }
        if (!socket->Listen(SOMAXCONN)) {
            LOG_CUSTOM_ERROR(sys_logger,
                             "listen fail errno = %d, errstr = %s, addr = [%s]",
                             errno, strerror(errno), addr->ToString().c_str())
            fails_vec.push_back(addr);
            continue;
        }
        // 没有错误的socket可以被加入到监听列表中
        m_sockets.push_back(socket);
    }
    // 如果有失败的地址，认为此次bind全部失败
    if (!fails_vec.empty()) {
        m_sockets.clear();
        return false;
    }
    for (auto &i : m_sockets) {
        LOG_CUSTOM_INFO(sys_logger,
                        "type = %s, name = %s,server bind success: %s",
                        m_type.c_str(), m_name.c_str(), i->ToString().c_str())
    }
    return true;
}

auto TcpServer::Start() -> bool {
    // 如果当前处于运行状态
    if (!m_is_stop) {
        return true;
    }
    m_is_stop = false;
    for (auto &socket : m_sockets) {
        m_acceptor->Schedule([capture0 = shared_from_this(), socket] {
            capture0->StartAccept(socket);
        });
    }
    return true;
}

void TcpServer::StartAccept(const Socket::ptr &socket) {
    while (!m_is_stop) {
        Socket::ptr client = socket->Accept();
        if (client != nullptr) {
            client->SetRecvTimeout(m_recv_timeout);
            m_worker->Schedule([capture0 = shared_from_this(), client] {
                capture0->HandleClient(client);
            });
        } else {
            LOG_CUSTOM_ERROR(sys_logger, "accept error = %d, errstr = %s",
                             errno, strerror(errno))
        }
    }
}

void TcpServer::HandleClient(const Socket::ptr &client) {
    LOG_CUSTOM_INFO(sys_logger, "Handle Client: %s", client->ToString().c_str())
}

void TcpServer::Stop() {
    m_is_stop = true;
    auto self = shared_from_this();
    m_acceptor->Schedule([this, self]() {
        for (auto &socket : m_sockets) {
            socket->CancelAll();
            socket->Close();
        }
        m_sockets.clear();
    });
}

auto TcpServer::ToString(const std::string &prefix) -> std::string {
    std::stringstream ss;
    ss << prefix << "[type=" << m_type << " name=" << m_name
       << " io_worker=" << (m_worker != nullptr ? m_worker->GetName() : "")
       << " accept=" << (m_acceptor != nullptr ? m_acceptor->GetName() : "")
       << " recv_timeout=" << m_recv_timeout << "]" << std::endl;
    std::string pfx = prefix.empty() ? "    " : prefix;
    for (auto &i : m_sockets) {
        ss << pfx << i->ToString() << std::endl;
    }
    return ss.str();
}

auto TcpServer::GetName() const -> const std::string & { return m_name; }

void TcpServer::SetName(const std::string &name) { m_name = name; }

auto TcpServer::GetType() const -> const std::string & { return m_type; }

void TcpServer::SetType(const std::string &type) { m_type = type; }

auto TcpServer::IsStop() const -> bool { return m_is_stop; }

void TcpServer::SetIsStop(bool is_stop) { m_is_stop = is_stop; }

auto TcpServer::GetRecvTimeout() const -> uint64_t { return m_recv_timeout; }

void TcpServer::SetRecvTimeout(uint64_t recv_timeout) {
    m_recv_timeout = recv_timeout;
}

}  // namespace wtsclwq