#include <cassert>
#include <chrono>
#include <thread>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <net/ip.h>
#include <net/http.h>
#include "substr.h"


#if _WIN32
    #define close closesocket
#endif


extern "C" {
    void* memset(void *, int, size_t);
    int   socket(int, int, int);
    int   close(int);
    char* strerror(int);
}


#include "http.method.inl"
#include "http.request.inl"
#include "http.response.inl"
#include "http.server.inl"
#include "http.status.inl"


#include "ip.address.inl"
#include "ip.error.inl"
#include "ip.socket.inl"
#include "ip.target.inl"