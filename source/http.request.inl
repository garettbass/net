#pragma once


namespace net {
namespace http {


    void
    request::reset() {
        method = METHOD_UNKNOWN;
        uri.clear();
        query.clear();
        headers.clear();
        content.clear();
    }


    static
    substr
    read_head(const substr& head, const substr& name) {
        return
            head
            .after(name).after(":").skip(isspace)
            .before("\r\n").truncate(isspace);
    }

    template<typename T>
    T
    read_head(const substr& headers, const substr& name, T fallback = {}) {
        substr value = read_head(headers, name);
        return (value.empty()) ? fallback : string_to<T>(value);
    }


    bool
    request::read(string& buffer) {
        static const substr HTTP_VERSION = "HTTP/1.1\r\n";
        static const substr LINE_BREAK = "\r\n";
        static const substr HEAD_BREAK = "\r\n\r\n";

        reset();

        const substr data = buffer;

        substr head = data.including(HEAD_BREAK);
        if (not head) return false;

        const size_t heading_length = head.size();
        const size_t content_length = read_head(head,"Content-Length",0);
        const size_t request_length = heading_length + content_length;

        const substr request = data.prefix(request_length);
        if (not request) return false;

        method = string_to_method(request.begin());
        assert(method);

        uri =
            request
            .after(method_to_string(method))
            .skip(isspace)
            .before(HTTP_VERSION)
            .truncate(isspace);

        head = head.after(HTTP_VERSION);
        while (head) {
            if (const substr key = head.before(':')) {
                const substr value =
                    head
                    .after(':')
                    .skip(isspace)
                    .before("\r\n")
                    .truncate(isspace);
                headers.set(string(key), string(value));
            }
            head = head.after(LINE_BREAK);
        }

        content = request.after(HEAD_BREAK);

        buffer.erase(0, request.length());
        return true;
    }


    void
    request::write(string& buffer) const {
        buffer.append(method_to_string(method));
        buffer.append(" ");
        buffer.append(uri);
        buffer.append(" HTTP/1.1\r\n");
        for (auto& pair : headers) {
            buffer.append(pair.first);
            buffer.append(": ");
            buffer.append(pair.second);
            buffer.append("\r\n");
        }
        buffer.append("\r\n");
        buffer.append(content);
    }

    string
    request::write() const {
        string buffer;
        write(buffer);
        return buffer;
    }


}} // namespace net::http