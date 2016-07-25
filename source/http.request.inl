#pragma once


namespace net {
namespace http {


    static
    substr
    read_head(const substr& head, const substr& name) {
        return
            head
            .after(name).after(":").skip(isspace)
            .before("\r\n").truncate(isspace);
    }

    template<typename T>
    static
    T
    read_head(const substr& headers, const substr& name, T fallback = {}) {
        substr value = read_head(headers, name);
        return (value.empty()) ? fallback : string_to<T>(value);
    }


    //--------------------------------------------------------------------------


    void
    request::reset() {
        method = METHOD_UNKNOWN;
        uri.clear();
        query.clear();
        headers.clear();
        content.clear();
    }


    bool
    request::read(string& buffer) {
        reset();

        const substr data = buffer;

        substr head = data.including("\r\n\r\n");
        if (not head) return false;

        const size_t heading_length = head.size();
        const size_t content_length = read_head(head,"Content-Length",0);
        const size_t message_length = heading_length + content_length;

        const substr message = data.prefix(message_length);
        if (not message) return false;

        // <method> <uri> HTTP/1.1\r\n
        method = string_to_method(head.begin());
        if (not method) {
            reset();
            return false;
        }

        substr url = 
            head
            .after(method_to_string(method))
            .skip(isspace)
            .before("HTTP/1.1\r\n")
            .truncate(isspace);

        uri = url;

        if (url.seek('?')) {
            uri = url.before('?');
            url = url.after('?');
            while (url) {
                if (const substr key = url.before('=')) {
                    const substr value =
                        (url.seek('&'))
                        ? url.after('=').before('&')
                        : url.after('=');
                    query.set(string(key), string(value));
                }
                url = url.after('&');
            }
        }

        head = head.after("HTTP/1.1\r\n");
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
            head = head.after("\r\n");
        }

        content = message.after("\r\n\r\n");

        buffer.erase(0, message.length());
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