#pragma once


namespace net {
namespace http {

    
    //==========================================================================


    enum {
        RECV_TIMEOUT_MS =  1000, //  1 second
        SEND_TIMEOUT_MS =  1000, //  1 second
        IDLE_TIMEOUT_MS = 30000, // 30 seconds
    };


    using clock = std::chrono::steady_clock;
    using time  = clock::time_point;


    static inline
    uint64_t
    elapsed_ms(const time& t) {
        using namespace std::chrono;
        const time now = clock::now();
        return duration_cast<milliseconds>(now - t).count();
    }


    static inline
    bool
    timedout(const time& last_active_time, const uint64_t timeout_ms) {
        return elapsed_ms(last_active_time) > timeout_ms;
    }


    //==========================================================================


    using lock = std::lock_guard<std::mutex>;


    struct trylock : std::unique_lock<std::mutex> {
        trylock(std::mutex& m)
        : std::unique_lock<std::mutex>(m, std::try_to_lock)
        {}
    };


    //==========================================================================


    server::client::client(ip::socket&& s)
    : socket(std::move(s)) {
        {
            struct timeval recv_timeout;
            recv_timeout.tv_sec  = 0;
            recv_timeout.tv_usec = RECV_TIMEOUT_MS * 1000;
            socket.setsockopt(SOL_SOCKET, SO_SNDTIMEO, &recv_timeout);
        }
        {
            struct timeval send_timeout;
            send_timeout.tv_sec  = 0;
            send_timeout.tv_usec = SEND_TIMEOUT_MS * 1000;
            socket.setsockopt(SOL_SOCKET, SO_RCVTIMEO, &send_timeout);
        }
        //printf("client::client(%i)\n", socket.id);
    }


    server::client::~client() {
        assert(this);
        socket.close();
        lock serve_lock(serve_mutex);
    }


    //--------------------------------------------------------------------------


    ip::error
    server::start(uint16_t port) {
        stop();

        if (auto err = listener.open(ip::TCP))
            return err;

        if (auto err = listener.setsockopt(SOL_SOCKET, SO_REUSEADDR, true))
            return err;

        if (auto err = listener.setsockopt(SOL_SOCKET, SO_NOSIGPIPE, true))
            return err;

        if (auto err = listener.bind(port))
            return err;

        if (auto err = listener.listen())
            return err;

        std::thread([this]{ listen(); }).detach();
        return ip::error::none();
    }


    void
    server::stop() {
        listener.close();
        lock listen_lock(listen_mutex);
        lock clients_lock(clients_mutex);
        for (auto& c : clients) { c->socket.close(); }
        clients.clear();
        //printf("%s() DONE\n", __func__);
    }


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


    void
    server::listen() {
        if (trylock listen_lock{listen_mutex}) {
            while (listener.ok()) {
                if (auto err = listener.listen()) {
                    printf("server::listen() error: '%s'\n", err.message());
                    continue;
                }
                accept();
            }
        }
        //printf("%s() DONE\n", __func__);
    }


    void
    server::accept() {
        ip::socket socket = listener.accept();
        if (not socket.ok()) return;
        if (trylock clients_lock{clients_mutex}) {
            auto result = clients.emplace(new client(std::move(socket)));
            assert(result.second); // successfully emplaced
            std::thread([this,result]{ serve(*result.first); }).detach();
        }
    }


    void
    server::serve(const client_ptr& client_ptr) {
        client& client = *client_ptr;
        socket& socket = client.socket;
        const int client_id = socket.id;

        request  request;  string request_buffer;
        response response; string response_buffer;

        ip::transfer transfer; char block[4096];

        if (trylock serve_lock{client.serve_mutex}) {
            time last_active_time = clock::now();
            while ((transfer = socket.recv(block))) {
                if (transfer.size) {
                    request_buffer.append(block, transfer.size);
                    while (request.read(request_buffer)) {
                        response.reset(NOT_IMPLEMENTED);
                        service(request, response);
                        response.write(response_buffer);
                        socket.sendall(response_buffer);
                        response_buffer.clear();
                    }
                    last_active_time = clock::now();
                    continue;
                }
                if (timedout(last_active_time, IDLE_TIMEOUT_MS)) {
                    break;
                }
            }
        }

        close(client_ptr);
        //printf("%s(client(%i)) DONE\n", __func__, client_id);
    }


    void
    server::close(const client_ptr& client_ptr) {
        if (trylock clients_lock{clients_mutex}) {
            clients.erase(client_ptr);
        }
    }


}} // namespace net::http