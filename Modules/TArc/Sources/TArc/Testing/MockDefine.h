#pragma once

#include <gmock/gmock.h>

#if __clang__

    #define MOCK_METHOD0_VIRTUAL(m, arg) \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") \
    MOCK_METHOD0(m, arg); \
    _Pragma("clang diagnostic pop") \

    #define MOCK_METHOD1_VIRTUAL(m, arg) \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") \
    MOCK_METHOD1(m, arg); \
    _Pragma("clang diagnostic pop") \

    #define MOCK_METHOD2_VIRTUAL(m, arg) \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"") \
    MOCK_METHOD2(m, arg); \
    _Pragma("clang diagnostic pop") \

#else
    #define MOCK_METHOD0_VIRTUAL(m, arg) MOCK_METHOD0(m, arg);
    #define MOCK_METHOD1_VIRTUAL(m, arg) MOCK_METHOD1(m, arg);
    #define MOCK_METHOD2_VIRTUAL(m, arg) MOCK_METHOD2(m, arg);
#endif
