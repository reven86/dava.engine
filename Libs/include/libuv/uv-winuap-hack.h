
/* Declarations from Windows headers */

/* for FormatMessage */
#ifndef FORMAT_MESSAGE_ALLOCATE_BUFFER
# define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#endif

/* for LoadLibraryEx */
#ifndef LOAD_WITH_ALTERED_SEARCH_PATH
# define LOAD_WITH_ALTERED_SEARCH_PATH  0x00000008
#endif

/* for SetHandleInformation */
#ifndef HANDLE_FLAG_INHERIT
# define HANDLE_FLAG_INHERIT    0x00000001
#endif

/* for SetErrorMode */
#ifndef SEM_FAILCRITICALERRORS
# define SEM_FAILCRITICALERRORS 0x0001
#endif

#ifndef SEM_NOGPFAULTERRORBOX
# define SEM_NOGPFAULTERRORBOX  0x0002
#endif

#ifndef SEM_NOALIGNMENTFAULTEXCEPT
# define SEM_NOALIGNMENTFAULTEXCEPT 0x0004
#endif

#ifndef SEM_NOOPENFILEERRORBOX
# define SEM_NOOPENFILEERRORBOX 0x8000
#endif

/* for overlapped I/O */
#ifndef HasOverlappedIoCompleted
# define HasOverlappedIoCompleted(lpOverlapped) \
                (((DWORD)(lpOverlapped)->Internal) != STATUS_PENDING)
#endif

/* for GetAdaptersAddresses */
#ifndef IF_TYPE_SOFTWARE_LOOPBACK
# define IF_TYPE_SOFTWARE_LOOPBACK  24
#endif

/* Ensure Windows structures packing */
#pragma pack(push)
#pragma pack(4)
typedef struct _MEMORYSTATUSEX {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORDLONG ullTotalPhys;
    DWORDLONG ullAvailPhys;
    DWORDLONG ullTotalPageFile;
    DWORDLONG ullAvailPageFile;
    DWORDLONG ullTotalVirtual;
    DWORDLONG ullAvailVirtual;
    DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION,
  *PBY_HANDLE_FILE_INFORMATION,
  *LPBY_HANDLE_FILE_INFORMATION;
#pragma pack(pop)

// Functions from KernelBase.dll
extern HMODULE (WINAPI* GetModuleHandleA)(LPCSTR lpModuleName);
extern HMODULE (WINAPI* LoadLibraryExW)(LPCWSTR lpLibFileName,
                                        HANDLE hFile,
                                        DWORD dwFlags);
extern BOOL (WINAPI* GlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer);
extern HLOCAL (WINAPI* LocalFree)(HLOCAL hMem);
extern HANDLE (WINAPI* CreateIoCompletionPort)(HANDLE FileHandle,
                                        HANDLE ExistingCompletionPort,
                                        ULONG_PTR CompletionKey,
                                        DWORD NumberOfConcurrentThreads);
extern BOOL (WINAPI* GetQueuedCompletionStatus)(HANDLE CompletionPort,
                                         LPDWORD lpNumberOfBytesTransferred,
                                         PULONG_PTR lpCompletionKey,
                                         LPOVERLAPPED* lpOverlapped,
                                         DWORD dwMilliseconds);
extern BOOL (WINAPI* PostQueuedCompletionStatus)(HANDLE CompletionPort,
                                          DWORD dwNumberOfBytesTransferred,
                                          ULONG_PTR dwCompletionKey,
                                          LPOVERLAPPED lpOverlapped);
extern BOOL (WINAPI* CancelIo)(HANDLE hFile);
extern BOOL (WINAPI* QueueUserWorkItem)(LPTHREAD_START_ROUTINE Function,
                                        PVOID Context,
                                        ULONG Flags);
extern BOOL (WINAPI* RegisterWaitForSingleObjectEx)(PHANDLE phNewWaitObject,
                                             HANDLE hObject,
                                             WAITORTIMERCALLBACK Callback,
                                             PVOID Context,
                                             ULONG dwMilliseconds,
                                             ULONG dwFlags);
extern BOOL (WINAPI* UnregisterWaitEx)(HANDLE WaitHandle,
                                       HANDLE CompletionEvent);
extern BOOL (WINAPI* GetProcessTimes)(HANDLE hProcess,
                                      LPFILETIME lpCreationTime,
                                      LPFILETIME lpExitTime,
                                      LPFILETIME lpKernelTime,
                                      LPFILETIME lpUserTime);
extern BOOL (WINAPI* SetHandleInformation)(HANDLE hObject,
                                           DWORD dwMask,
                                           DWORD dwFlags);
extern BOOL (WINAPI* GetFileInformationByHandle)(HANDLE hFile,
                               LPBY_HANDLE_FILE_INFORMATION lpFileInformation);
extern UINT (WINAPI* SetErrorMode)(UINT uMode);
