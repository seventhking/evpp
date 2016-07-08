#pragma once

#include "evpp/inner_pre.h"
#include "evpp/event_loop.h"
#include "evpp/event_loop_thread_pool.h"
#include "evpp/tcp_callbacks.h"
#include "evpp/any.h"

#include <map>
#include <atomic>
#include <mutex>

namespace evpp {
    class Connector;
    class EVPP_EXPORT TCPClient {
    public:
        TCPClient(EventLoop* loop, const std::string& remote_addr/*host:port*/, const std::string& name);
        ~TCPClient();
        void Connect();
        void Disconnect();
    public:
        // 设置一个连接相关的回调函数，当成功建立连接、或连接断开、或建立连接失败等事件发生时，都会调用该回调
        //  当成功建立连接时，回调中的参数 TCPConn::IsConnected() == true
        //  当连接断开时，回调中的参数 TCPConn::IsDisconnecting() == true
        //  当建立连接失败时，回调中的参数 TCPConn::IsDisconnected() == true 并且 TCPConn::fd() == -1
        void SetConnectionCallback(const ConnectionCallback& cb) { conn_fn_ = cb; }

        void SetMessageCallback(const MessageCallback& cb) { msg_fn_ = cb; }
        void SetWriteCompleteCallback(const WriteCompleteCallback& cb) { write_complete_fn_ = cb; }
        void set_auto_reconnect(bool v) { auto_reconnect_.store(v); }
        void set_connection_timeout(Duration timeout) { connection_timeout_ = timeout; }
        void set_context(const Any& c) { context_ = c; }
        const Any& context() const { return context_; }
        TCPConnPtr conn() const;
        const std::string& remote_addr() const { return remote_addr_; }
        const std::string& name() const { return name_; }
    private:
        void ConnectInLoop();
        void DisconnectInLoop();
        void OnConnection(int sockfd, const std::string& laddr);
        void OnRemoveConnection(const TCPConnPtr& conn);
        void Reconnect();
    private:
        EventLoop* loop_;
        std::string remote_addr_;
        std::string name_;
        std::atomic<bool> auto_reconnect_; // 自动重连标记，默认为 true
        Any context_;

        mutable std::mutex mutex_; // The guard of conn_
        TCPConnPtr conn_;

        std::shared_ptr<Connector> connector_;
        Duration connection_timeout_; // Default : 3 seconds

        ConnectionCallback conn_fn_;
        MessageCallback msg_fn_;
        WriteCompleteCallback write_complete_fn_;
    };
}