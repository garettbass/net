#ifndef TEST


#define TEST(name) \
        TEST_IDENTIFIER(__COUNTER__, name)
#define TEST_IDENTIFIER(id, name) \
        TEST_DEFINITION(id, name)
#define TEST_DEFINITION(id, name) \
        static struct TEST_TYPE_##id : tests::test { \
            using tests::test::test; void run() override; \
        } TEST_##id(name); \
        void TEST_TYPE_##id::run()


#define ECHO(expr) echo(#expr,expr)


#define FAIL(...) fail(__FILE__,__LINE__,__VA_ARGS__)


#define CHECK(expr) [&]{ if (not (expr)) FAIL("CHECK(",#expr,")"); }()


#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace tests {


    struct failure : std::exception {
        const std::string message;
        failure(std::string message) : message(std::move(message)) {}
        const char* what() const noexcept override { return message.c_str(); }
    };


    struct test {
        using instances_t = std::vector<test*>;

        const std::string name;

        test(const char* name) : name(name) { instances().push_back(this); }

        static instances_t& instances() { static instances_t _; return _; }

        virtual void run() = 0;

        template<typename... Args>
        void expand(Args&&...) {}

        void echo(const char* expr, const char* s) {
            std::cout<<expr<<": \""<<s<<"\"\n";
        }

        void echo(const char* expr, const std::string& s) {
            std::cout<<expr<<": \""<<s<<"\"\n";
        }

        template<typename T>
        void echo(const char* expr, const T& t) {
            std::cout<<std::boolalpha;
            std::cout<<expr<<": "<<t<<"\n";
        }

        template<typename... Args>
        void fail(const char* file, int line, Args&&... args) {
            std::stringstream out;
            out<<std::boolalpha;
            expand((out<<args)...);
            out<<'\n'<<file<<':'<<line;
            throw failure{ out.str() };
        }
    };


    inline
    int run() {
        int failed = 0;
        for (auto* test : test::instances()) {
            auto& name = test->name;
            try {
                test->run();
                printf("PASSED: %s\n\n", name.c_str());
                continue;
            }
            catch (const failure& f) {
                printf("FAILED: %s\n%s\n\n", name.c_str(), f.what());
            }
            catch (const std::exception& e) {
                printf("FAILED: %s\n%s\n\n", name.c_str(), e.what());
            }
            catch (...) {
                printf("FAILED: %s\n<unhandled exception>\n\n", name.c_str());
            }
            failed += 1;
        }
        return failed;
    }



} // namespace tests


#endif // TEST