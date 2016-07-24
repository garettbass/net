#pragma once

//------------------------------------------------------------------------------
/** @name CPU recognition macros

    Identifies the CPU architecture targeted by the build toolchain.

    One of the following is defined to the integer constant 1

        NET_CPU_ARM
        NET_CPU_PPC (and one of: CPU_PPC_32, CPU_PPC_64)
        NET_CPU_X86 (and one of: CPU_X86_32, CPU_X86_64)

    Additionally, the following are defined:

        NET_CPU_ID - a valid C++ identifier, e.g. arm, ppc_32, x86_64
        NET_CPU_NAME - a string literal name for the CPU architecture
        NET_CPU_BITS - an integer constant, 32 or 64
*/

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
/** @name Endianness macros

    Identifies the CPU endianness targeted by the build toolchain.

    One of the following defines the endianness to an integer constant
    representing the byte order:

        ENDIAN_LE (e.g. x86)
        ENDIAN_BE    (e.g. PPC, network byte order)

    Additionally, the following are defined:

        ENDIAN - as either ENDIAN_LE, or ENDIAN_BE
        ENDIAN_name - a string literal name for the CPU endianness
*/

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

//------------------------------------------------------------------------------