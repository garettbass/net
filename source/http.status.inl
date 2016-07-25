#pragma once


namespace net {
namespace http {


    http::status
    string_to_status(const char* str) {
        substr sub(str);
        if (sub.has_prefix("100") or sub.has_prefix("Continue"))                        return CONTINUE;
        if (sub.has_prefix("101") or sub.has_prefix("Switching Protocols"))             return SWITCHING_PROTOCOLS;
        if (sub.has_prefix("102") or sub.has_prefix("Processing"))                      return PROCESSING;
        if (sub.has_prefix("200") or sub.has_prefix("OK"))                              return OK;
        if (sub.has_prefix("201") or sub.has_prefix("Created"))                         return CREATED;
        if (sub.has_prefix("202") or sub.has_prefix("Accepted"))                        return ACCEPTED;
        if (sub.has_prefix("203") or sub.has_prefix("Non-Authoritative Information"))   return NON_AUTHORITATIVE_INFORMATION;
        if (sub.has_prefix("204") or sub.has_prefix("No Content"))                      return NO_CONTENT;
        if (sub.has_prefix("205") or sub.has_prefix("Reset Content"))                   return RESET_CONTENT;
        if (sub.has_prefix("206") or sub.has_prefix("Partial Content"))                 return PARTIAL_CONTENT;
        if (sub.has_prefix("300") or sub.has_prefix("Multiple Choices"))                return MULTIPLE_CHOICES;
        if (sub.has_prefix("301") or sub.has_prefix("Moved Permanently"))               return MOVED_PERMANENTLY;
        if (sub.has_prefix("302") or sub.has_prefix("Found"))                           return FOUND;
        if (sub.has_prefix("303") or sub.has_prefix("See Other"))                       return SEE_OTHER;
        if (sub.has_prefix("304") or sub.has_prefix("Not Modified"))                    return NOT_MODIFIED;
        if (sub.has_prefix("305") or sub.has_prefix("Use Proxy"))                       return USE_PROXY;
        if (sub.has_prefix("306") or sub.has_prefix("Switch Proxy"))                    return SWITCH_PROXY;
        if (sub.has_prefix("307") or sub.has_prefix("Temporary Redirect"))              return TEMPORARY_REDIRECT;
        if (sub.has_prefix("400") or sub.has_prefix("Bad Request"))                     return BAD_REQUEST;
        if (sub.has_prefix("401") or sub.has_prefix("Unauthorized"))                    return UNAUTHORIZED;
        if (sub.has_prefix("402") or sub.has_prefix("Payment Required"))                return PAYMENT_REQUIRED;
        if (sub.has_prefix("403") or sub.has_prefix("Forbidden"))                       return FORBIDDEN;
        if (sub.has_prefix("404") or sub.has_prefix("Not Found"))                       return NOT_FOUND;
        if (sub.has_prefix("405") or sub.has_prefix("Method Not Allowed"))              return METHOD_NOT_ALLOWED;
        if (sub.has_prefix("406") or sub.has_prefix("Not Acceptable"))                  return NOT_ACCEPTABLE;
        if (sub.has_prefix("407") or sub.has_prefix("Proxy Authentication Failed"))     return PROXY_AUTHENTICATION_FAILED;
        if (sub.has_prefix("408") or sub.has_prefix("Request Time-out"))                return REQUEST_TIMEOUT;
        if (sub.has_prefix("409") or sub.has_prefix("Conflict"))                        return CONFLICT;
        if (sub.has_prefix("410") or sub.has_prefix("Gone"))                            return GONE;
        if (sub.has_prefix("411") or sub.has_prefix("Length Required"))                 return LENGTH_REQUIRED;
        if (sub.has_prefix("412") or sub.has_prefix("Precondition Failed"))             return PRECONDITION_FAILED;
        if (sub.has_prefix("413") or sub.has_prefix("Request Entity Too Large"))        return REQUEST_ENTITY_TOO_LARGE;
        if (sub.has_prefix("414") or sub.has_prefix("Request-URI Too Long"))            return REQUEST_URI_TOO_LONG;
        if (sub.has_prefix("415") or sub.has_prefix("Unsupported Media Type"))          return UNSUPPORTED_MEDIA_TYPE;
        if (sub.has_prefix("416") or sub.has_prefix("Requested Range Not Satisfiable")) return REQUESTED_RANGE_NOT_SATISFIABLE;
        if (sub.has_prefix("417") or sub.has_prefix("Expectation Failed"))              return EXPECTATION_FAILED;
        if (sub.has_prefix("500") or sub.has_prefix("Internal Server Error"))           return INTERNAL_SERVER_ERROR;
        if (sub.has_prefix("501") or sub.has_prefix("Not Implemented"))                 return NOT_IMPLEMENTED;
        if (sub.has_prefix("502") or sub.has_prefix("Bad Gateway"))                     return BAD_GATEWAY;
        if (sub.has_prefix("503") or sub.has_prefix("Service Unavailable"))             return SERVICE_UNAVAILABLE;
        if (sub.has_prefix("504") or sub.has_prefix("Gateway Timeout"))                 return GATEWAY_TIMEOUT;
        if (sub.has_prefix("505") or sub.has_prefix("HTTP Version Not Supported"))      return HTTP_VERSION_NOT_SUPPORTED;
        return STATUS_UNKNOWN;
    }


    const char*
    status_to_string(http::status s) {
        switch (s) {
            default: return "UNKNOWN";

            // 1XX - Informational
            case CONTINUE:                        return "Continue";
            case SWITCHING_PROTOCOLS:             return "Switching Protocols";
            case PROCESSING:                      return "Processing";

            // 2XX - Success
            case OK:                              return "OK";
            case CREATED:                         return "Created";
            case ACCEPTED:                        return "Accepted";
            case NON_AUTHORITATIVE_INFORMATION:   return "Non-Authoritative Information";
            case NO_CONTENT:                      return "No Content";
            case RESET_CONTENT:                   return "Reset Content";
            case PARTIAL_CONTENT:                 return "Partial Content";

            // 3XX - Redirection
            case MULTIPLE_CHOICES:                return "Multiple Choices";
            case MOVED_PERMANENTLY:               return "Moved Permanently";
            case FOUND:                           return "Found";
            case SEE_OTHER:                       return "See Other";
            case NOT_MODIFIED:                    return "Not Modified";
            case USE_PROXY:                       return "Use Proxy";
            case SWITCH_PROXY:                    return "Switch Proxy";
            case TEMPORARY_REDIRECT:              return "Temporary Redirect";

            // 4XX - Client Error
            case BAD_REQUEST:                     return "Bad Request";
            case UNAUTHORIZED:                    return "Unauthorized";
            case PAYMENT_REQUIRED:                return "Payment Required";
            case FORBIDDEN:                       return "Forbidden";
            case NOT_FOUND:                       return "Not Found";
            case METHOD_NOT_ALLOWED:              return "Method Not Allowed";
            case NOT_ACCEPTABLE:                  return "Not Acceptable";
            case PROXY_AUTHENTICATION_FAILED:     return "Proxy Authentication Failed";
            case REQUEST_TIMEOUT:                 return "Request Time-out";
            case CONFLICT:                        return "Conflict";
            case GONE:                            return "Gone";
            case LENGTH_REQUIRED:                 return "Length Required";
            case PRECONDITION_FAILED:             return "Precondition Failed";
            case REQUEST_ENTITY_TOO_LARGE:        return "Request Entity Too Large";
            case REQUEST_URI_TOO_LONG:            return "Request-URI Too Long";
            case UNSUPPORTED_MEDIA_TYPE:          return "Unsupported Media Type";
            case REQUESTED_RANGE_NOT_SATISFIABLE: return "Requested Range Not Satisfiable";
            case EXPECTATION_FAILED:              return "Expectation Failed";

            // 5XX - Server Error
            case INTERNAL_SERVER_ERROR:           return "Internal Server Error";
            case NOT_IMPLEMENTED:                 return "Not Implemented";
            case BAD_GATEWAY:                     return "Bad Gateway";
            case SERVICE_UNAVAILABLE:             return "Service Unavailable";
            case GATEWAY_TIMEOUT:                 return "Gateway Timeout";
            case HTTP_VERSION_NOT_SUPPORTED:      return "HTTP Version Not Supported";
        }
    }


}} // namespace net::http