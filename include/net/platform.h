#pragma once
#include <ciso646>


//------------------------------------------------------------------------------


#if !defined(NET_DEBUG)
    #if defined(_MSC_VER) && defined(_DEBUG)
        #define NET_DEBUG 1
    #elif defined(__GNUC__) && !defined(__OPTIMIZE)
        #define NET_DEBUG 1
    #endif
#endif


//------------------------------------------------------------------------------


#if defined(__ANDROID__)

    #define NET_PLATFORM_ANDROID 1

#elif defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)

    #define NET_PLATFORM_APPLE 1
    #define NET_PLATFORM_IOS 1

#elif defined(__linux)

    #define NET_PLATFORM_LINUX 1

#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)

    #define NET_PLATFORM_APPLE 1
    #define NET_PLATFORM_MACOS 1

#elif defined(_WIN32)

    #define NET_PLATFORM_WINDOWS 1

    #if __cplusplus_winrt
        #define NET_PLATFORM_WINRT 1
    #endif

#else

    #error "unrecognized platform"

#endif


//------------------------------------------------------------------------------


#if defined(__GNUC__) && defined(__clang__)

    #define NET_COMPILER_ID     clang
    #define NET_COMPILER_CLANG  (__clang_major__ * 10 + __clang_minor__)
    #define NET_COMPILER_NAME   "Clang " __clang_version__

#elif defined(__GNUC__) && !defined(__clang__)

    #define NET_COMPILER_ID    gcc
    #define NET_COMPILER_GCC   (__GNUC__ * 10 + __GNUC_MINOR__)
    #define NET_COMPILER_NAME  "GCC " __VERSION__

#elif defined(_MSC_VER)

    #define NET_COMPILER_ID  msvc
    #if (_MSC_VER >= 1900)
        #define NET_COMPILER_MSVC 14
        #define NET_COMPILER_NAME "Microsoft Visual C++ 14 (2015)"
    #elif (_MSC_VER >= 1800)
        #define NET_COMPILER_MSVC 12
        #define NET_COMPILER_NAME "Microsoft Visual C++ 12 (2013)"
    #elif (_MSC_VER >= 1700)
        #define NET_COMPILER_MSVC 11
        #define NET_COMPILER_NAME "Microsoft Visual C++ 11 (2012)"
    #elif (_MSC_VER >= 1600)
        #define NET_COMPILER_MSVC 10
        #define NET_COMPILER_NAME "Microsoft Visual C++ 10 (2010)"
    #elif (_MSC_VER == 1500)
        #define NET_COMPILER_MSVC 9
        #define NET_COMPILER_NAME "Microsoft Visual C++ 9 (2008)"
    #elif (_MSC_VER == 1400)
        #define NET_COMPILER_MSVC 8
        #define NET_COMPILER_NAME "Microsoft Visual C++ 8 (2005)"
    #endif

    // warning C4201: nonstandard extension used: nameless struct/union
    #pragma warning(disable:4201)

#else

    #error "unrecognized compiler"

#endif


//------------------------------------------------------------------------------


#if defined(__aarch64__)

    // ARM 64
    #define NET_CPU_ARM    1
    #define NET_CPU_ARM_64 1
    #define NET_CPU_ID     arm_64
    #define NET_CPU_NAME   "ARM64"
    #define NET_CPU_BITS   64

#elif \
    defined(ARM)     || \
    defined(_ARM)    || \
    defined(__arm)   || \
    defined(__arm__)

    // ARM 32
    #define NET_CPU_ARM    1
    #define NET_CPU_ARM_32 1
    #define NET_CPU_ID     arm_32
    #define NET_CPU_NAME   "ARM"
    #define NET_CPU_BITS   32

#elif \
    defined(__powerpc64__) || \
    defined(__ppc64__)

    // PPC 64
    #define NET_CPU_PPC    1
    #define NET_CPU_PPC_64 1
    #define NET_CPU_ID     ppc_64
    #define NET_CPU_NAME   "PowerPC 64"
    #define NET_CPU_BITS   64

#elif \
    defined(__POWERPC__) || \
    defined(__PPC__)     || \
    defined(__powerpc__) || \
    defined(__ppc__)     || \
    defined(_POWER)      || \
    defined(powerpc)

    // PPC 32
    #define NET_CPU_PPC    1
    #define NET_CPU_PPC_32 1
    #define NET_CPU_ID     ppc_32
    #define NET_CPU_NAME   "PowerPC"
    #define NET_CPU_BITS   32

#elif \
    defined(_M_X64)     || \
    defined(_M_AMD64)   || \
    defined(__x86_64__)

    // x86-64
    #define NET_CPU_X86    1
    #define NET_CPU_X86_64 1
    #define NET_CPU_ID     x86_64
    #define NET_CPU_NAME   "x86-64"
    #define NET_CPU_BITS   64

#elif \
    defined(_M_IX86)  || \
    defined(__386__)  || \
    defined(__i386__) || \
    defined(__X86__)  || \
    defined(i386)

    // x86
    #define NET_CPU_X86    1
    #define NET_CPU_X86_32 1
    #define NET_CPU_ID     x86_32
    #define NET_CPU_NAME   "x86"
    #define NET_CPU_BITS   32

#else

    #error "unrecognized CPU architecture"

#endif


//------------------------------------------------------------------------------


#if (NET_CPU_X86) || (NET_CPU_ARM && !__BIG_ENDIAN__)

    #define NET_ENDIAN_LE     0x01020304u
    #define NET_ENDIAN_BE     0
    #define NET_ENDIAN        NET_ENDIAN_LE
    #define NET_ENDIAN_NAME   "little endian"
    static_assert(0x31323334 == '1234', "byte order mismatch");

#elif (NET_CPU_PPC)

    #define NET_ENDIAN_LE     0
    #define NET_ENDIAN_BE     0x04030201u
    #define NET_ENDIAN        NET_ENDIAN_BE
    #define NET_ENDIAN_NAME   "big endian"
    static_assert(0x34333231 == '1234', "byte order mismatch");

#else

    #error "unrecognized endianness"

#endif
