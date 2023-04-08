//
// Created by wtsclwq on 23-4-4.
//

#include "../include/serialize/socket_stream.h"

#include <utility>

namespace wtsclwq {
SocketStream::SocketStream(Socket::ptr socket, bool owner)
    : m_socket(std::move(socket)), m_owner(owner) {}

SocketStream::~SocketStream() {
    if (m_owner && m_socket != nullptr) {
        m_socket->Close();
    }
}

auto SocketStream::ReadToBuf(void *buffer, size_t length) -> int {
    if (!IsConnected()) {
        return -1;
    }
    return m_socket->Recv(buffer, length, 0);
}

auto SocketStream::ReadToArray(ByteArray::ptr byte_array, size_t length)
    -> int {
    if (!IsConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    byte_array->GetWriteBuffers(iovs, length);
    int flag = m_socket->RecvIovec(iovs.data(), iovs.size(), 0);
    if (flag > 0) {
        byte_array->SetPosition(byte_array->GetPosition() + flag);
    }
    return flag;
}

auto SocketStream::WriteFromBuf(const void *buffer, size_t length) -> int {
    if (!IsConnected()) {
        return -1;
    }
    return m_socket->Send(buffer, length, 0);
}

auto SocketStream::WriteFromArray(ByteArray::ptr byte_array, size_t length)
    -> int {
    if (!IsConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    byte_array->GetReadBuffers(iovs, length);
    int flag = m_socket->SendIovec(iovs.data(), iovs.size(), 0);
    if (flag > 0) {
        byte_array->SetPosition(byte_array->GetPosition() + flag);
    }
    return flag;
}

void SocketStream::Close() {
    if (m_socket != nullptr) {
        m_socket->Close();
    }
}

auto SocketStream::IsConnected() const -> bool {
    return m_socket != nullptr && m_socket->IsConnected();
}
}  // namespace wtsclwq