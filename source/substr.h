#pragma once
#include <string>
#include <net/assign.h>


namespace net {


    class substr {

        const char* const _begin  = "";
        const size_t      _length = 0;

    public: // structors

        substr() = default;

        substr(const substr& src) = default;

        substr(decltype(nullptr))
        : _begin(nullptr), _length(0) {}

        substr(const char* begin, size_t length)
        : _begin(begin), _length(begin ? length : 0) {}

        substr(const char* begin, const char* end)
        : _begin(begin), _length(size_t(end) - size_t(begin)) {}

        substr(const char* str)
        : substr(str, str ? strlen(str) : 0) {}

        substr(const std::string& s)
        : substr(s.c_str(), s.length()) {}

    public: // operators

        substr& operator =(const substr& src) { return assign(this, src); }

        bool operator ==(const substr& s) const { return compare(s) == 0; }
        bool operator !=(const substr& s) const { return compare(s) != 0; }
        bool operator  <(const substr& s) const { return compare(s)  < 0; }
        bool operator <=(const substr& s) const { return compare(s) <= 0; }
        bool operator  >(const substr& s) const { return compare(s)  > 0; }
        bool operator >=(const substr& s) const { return compare(s) >= 0; }

        char operator [](size_t index) const {
            return (index < _length) ? _begin[index] : '\0';
        }

        explicit operator bool() const { return not empty(); }

        operator std::string() const {
            return empty() ? std::string() : std::string(_begin, _length);
        }

    public: // properties
        bool    empty() const { return _length == 0; }
        size_t length() const { return _length; }
        size_t   size() const { return _length; }

    public: // iterators
        const char* begin() const { return _begin; }
        const char*   end() const { return _begin + _length; }

    public: // queries
        int compare(const substr&) const;

        bool has_prefix(char) const;
        bool has_prefix(const substr&) const;

        bool has_suffix(char) const;
        bool has_suffix(const substr&) const;

        substr prefix(size_t length) const;
        substr suffix(size_t length) const;

        substr after(char) const;
        substr after(const substr&) const;

        substr before(char) const;
        substr before(const substr&) const;

        substr including(char) const;
        substr including(const substr&) const;

        substr seek(int(&)(int)) const;
        substr seek(char) const;
        substr seek(const substr&) const;
        substr seek(size_t offset) const;

        substr skip(int(&)(int)) const;
        substr skip(char) const;
        substr skip(const substr&) const;
        substr skip(size_t length) const;

        substr truncate(int(&)(int)) const;
        substr truncate(char) const;
        substr truncate(const substr&) const;
        substr truncate(size_t length) const;
    };

    //--------------------------------------------------------------------------

    inline
    std::string
    operator +(const std::string& a, const substr& b) {
        std::string c = a; c.append(b.begin(), b.length());
        return c;
    }

    //--------------------------------------------------------------------------

    inline
    int
    substr::compare(const substr& s) const {
        if (this == &s)
            return 0;

        const size_t a_len =   _length;
        const size_t b_len = s._length;
        if (a_len == 0 and b_len == 0)
            return 0;

        const char* a =   _begin;
        const char* b = s._begin;
        const int subcmp = strncmp(a, b, std::min(a_len, b_len));
        if (subcmp == 0) {
            if (a_len < b_len) return -1;
            if (a_len > b_len) return +1;
        }
        return subcmp;
    }


    //--------------------------------------------------------------------------


    inline
    bool
    substr::has_prefix(const char c) const {
        if (not _length) return {};
        return _begin[0] == c;
    }


    inline
    bool
    substr::has_prefix(const substr& p) const {
        return p == prefix(p._length);
    }


    //--------------------------------------------------------------------------


    inline
    bool
    substr::has_suffix(const char c) const {
        if (not _length) return {};
        return _begin[_length - 1] == c;
    }


    inline
    bool
    substr::has_suffix(const substr& s) const {
        return s == suffix(s._length);
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::prefix(size_t length) const {
        if (length > _length) return {};
        return { _begin, length };
    }


    inline
    substr
    substr::suffix(size_t length) const {
        if (length > _length) return {};
        return { _begin + _length - length, length };
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::after(char c) const {
        return seek(c).skip(c);
    }

    inline
    substr
    substr::after(const substr& s) const {
        return seek(s).skip(s);
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::before(char c) const {
        if (auto sub = seek(c)) {
            return { _begin, _length - sub._length };
        }
        return {};
    }


    inline
    substr
    substr::before(const substr& s) const {
        if (auto sub = seek(s)) {
            return { _begin, _length - sub._length };
        }
        return {};
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::including(char c) const {
        if (auto sub = seek(c)) {
            return { _begin, sub._length + 1 };
        }
        return {};
    }


    inline
    substr
    substr::including(const substr& s) const {
        if (auto sub = before(s)) {
            return { _begin, sub._length + s._length };
        }
        return {};
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::seek(int(&cc)(int)) const {
        if (empty()) return {};
        const char* itr = _begin;
        const char* const end = itr + _length;
        for (;itr < end;++itr) {
            if (cc(itr[0]))
                return {itr, end};
        }
        return {};
    }


    inline
    substr
    substr::seek(const char c) const {
        if (empty()) return {};
        const char* itr = _begin;
        const char* const end = itr + _length;
        for (;itr < end;++itr) {
            if (itr[0] == c)
                return {itr, end};
        }
        return {};
    }


    inline
    substr
    substr::seek(const substr& p) const {
        if (empty()) return {};
        if (p.empty()) return *this;
        substr sub(*this);
        const char p0 = p[0];
        while ((sub = sub.seek(p0))) {
            if (sub.has_prefix(p))
                return sub;
            sub = sub.skip(p0);
        }
        return {};
    }


    inline
    substr
    substr::seek(const size_t offset) const {
        if (offset >= _length) return {};
        return { _begin + offset, _length - offset };
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::skip(int(&cc)(int)) const {
        if (_length == 0) return {};
        const char* itr = _begin;
        const char* const end = itr + _length;
        while (itr < end and cc(itr[0])) { ++itr; }
        return {itr, end};
    }


    inline
    substr
    substr::skip(const char c) const {
        if (_length == 0) return {};
        if (_begin[0] != c) return {};
        return { _begin + 1, _length - 1 };
    }


    inline
    substr
    substr::skip(const substr& p) const {
        return has_prefix(p) ? skip(p._length) : *this;
    }

    inline
    substr
    substr::skip(size_t length) const {
        if (length >= _length) return {};
        return substr(_begin + length, _length - length);
    }


    //--------------------------------------------------------------------------


    inline
    substr
    substr::truncate(int(&cc)(int)) const {
        if (_length == 0) return {};
        const char* const rend = _begin - 1;
        const char* ritr = rend + _length;
        while (ritr > rend and cc(ritr[0])) { --ritr; }
        return {_begin, size_t(ritr) - size_t(rend)};
    }


    inline
    substr
    substr::truncate(const char c) const {
        return has_suffix(c) ? truncate(size_t(1)) : *this;
    }

    inline
    substr
    substr::truncate(const substr& s) const {
        return has_suffix(s) ? truncate(s._length) : *this;
    }

    inline
    substr
    substr::truncate(size_t length) const {
        if (length >= _length) return {};
        return substr(_begin, _length - length);
    }


} // namespace net