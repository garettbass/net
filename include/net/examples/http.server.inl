#include <chrono>
#include <thread>
#include <net/http.h>


int main(int argc, const char * argv[]) {
    using namespace net;
    
    ip::addresses(ip::TCP, "google.com", [](ip::address a){
        printf("address: %i.%i.%i.%i\n", a.a, a.b, a.c, a.d);
    });

    http::server server([](const http::request& req, http::response& res){
        res.status  = http::OK;
        res.content = "OK, THANKS!\n\n";
    });

    if (auto err = server.start(7200)) {
        printf("server.start(7200) error: %s\n", err.message());
        return 1;
    }

    for (int i = 20; i --> 1;) {
        printf("%i second%s remaining\n", i, (i==1)?"":"s"); fflush(stdout);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}