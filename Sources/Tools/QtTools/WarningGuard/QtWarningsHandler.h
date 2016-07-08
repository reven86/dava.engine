#ifndef __QTTOOLS_QTWARNINGHANDLER__
#define __QTTOOLS_QTWARNINGHANDLER__

#if defined(__clang__)
    #define PUSH_QT_WARNING_SUPRESSOR \
        _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"")
    #define POP_QT_WARNING_SUPRESSOR \
        _Pragma("clang diagnostic pop")
#else
    #define PUSH_QT_WARNING_SUPRESSOR
    #define POP_QT_WARNING_SUPRESSOR
#endif

#endif // __QTTOOLS_QTWARNINGHANDLER__
