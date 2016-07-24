#pragma once


namespace net {
namespace http {


    const char*
    method_to_string(http::method m) {
        switch (m) {
            default:      return "UNKNOWN";
            case CONNECT: return "CONNECT";
            case DELETE:  return "DELETE";
            case GET:     return "GET";
            case HEAD:    return "HEAD";
            case OPTIONS: return "OPTIONS";
            case PATCH:   return "PATCH";
            case POST:    return "POST";
            case PUT:     return "PUT";
            case TRACE:   return "TRACE";
        }
    }

    http::method
    string_to_method(const char* str) {
        substr sub(str);
        if (sub.has_prefix("CONNECT")) return CONNECT;
        if (sub.has_prefix("DELETE"))  return DELETE;
        if (sub.has_prefix("GET"))     return GET;
        if (sub.has_prefix("HEAD"))    return HEAD;
        if (sub.has_prefix("OPTIONS")) return OPTIONS;
        if (sub.has_prefix("PATCH"))   return PATCH;
        if (sub.has_prefix("POST"))    return POST;
        if (sub.has_prefix("PUT"))     return PUT;
        if (sub.has_prefix("TRACE"))   return TRACE;
        return METHOD_UNKNOWN;
    }


}} // namespace net::http