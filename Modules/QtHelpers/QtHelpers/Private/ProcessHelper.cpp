#include "QtHelpers/ProcessHelper.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <TlHelp32.h>
#endif

#include <QUrl>
#include <QDesktopServices>
#include <QProcess>
#include <QStringList>

#ifdef Q_OS_MAC
void ProcessHelper::RunProcess(const QString& path)
{
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
}

void ProcessHelper::OpenApplication(const QString& path)
{
    QProcess::startDetached("open", QStringList() << "-n" << path, path);
}

#endif

#ifdef Q_OS_WIN

void ProcessHelper::RunProcess(const QString& path)
{
    int lastPos = path.lastIndexOf('/');
    QString workingDir = path.left(lastPos);
    QProcess::startDetached(path, QStringList(), workingDir);
}

void ProcessHelper::OpenApplication(const QString& path)
{
    RunProcess(path);
}

namespace ProcessHelperDetails
{
bool GetProcessID(QString path, quint32& dwPID)
{
    path.replace("/", "\\");

    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;

    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap); // clean the snapshot object
        return (FALSE);
    }

    bool bRes = false;
    do
    {
        // Retrieve the priority class.
        dwPriorityClass = 0;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess)
        {
            CHAR lpPath[MAX_PATH]; //will hold .exe path to be returned
            DWORD charsCarried = MAX_PATH; // holds size of path[], will then hold amount of characters returned by QueryFullProcessImageName
            QueryFullProcessImageNameA(hProcess, 0, lpPath, &charsCarried);
            CloseHandle(hProcess);

            QString curPath(lpPath);
            if (curPath.compare(path, Qt::CaseInsensitive) == 0)
            {
                bRes = true;
                dwPID = pe32.th32ProcessID;
                break;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return bRes;
}

typedef struct tagENUMINFO
{
    // In Parameters
    DWORD PId;

    // Out Parameters
    HWND hWnd;
    HWND hEmptyWnd;
    HWND hInvisibleWnd;
    HWND hEmptyInvisibleWnd;
} ENUMINFO, *PENUMINFO;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    DWORD pid = 0;
    PENUMINFO pInfo = (PENUMINFO)lParam;
    TCHAR szTitle[_MAX_PATH + 1];

    // sanity checks
    if (pInfo == NULL)
        // stop the enumeration if invalid parameter is given
        return (FALSE);

    // get the processid for this window
    if (!::GetWindowThreadProcessId(hWnd, &pid))
        // this should never occur :-)
        return (TRUE);

    // compare the process ID with the one given as search parameter
    if (pInfo->PId == pid)
    {
        // look for the visibility first
        if (::IsWindowVisible(hWnd))
        {
            // look for the title next
            if (::GetWindowText(hWnd, szTitle, _MAX_PATH) != 0)
            {
                pInfo->hWnd = hWnd;

                // we have found the right window
                return (FALSE);
            }
            else
                pInfo->hEmptyWnd = hWnd;
        }
        else
        {
            // look for the title next
            if (::GetWindowText(hWnd, szTitle, _MAX_PATH) != 0)
            {
                pInfo->hInvisibleWnd = hWnd;
            }
            else
                pInfo->hEmptyInvisibleWnd = hWnd;
        }
    }

    // continue the enumeration
    return (TRUE);
}
}
#endif

bool ProcessHelper::IsProcessRuning(const QString& path)
{
#ifdef Q_OS_MAC
//    ProcessSerialNumber psn;
//    return ProcessHelper::GetProcessPSN(path, psn);
#elif defined Q_OS_WIN
    quint32 pid = 0;
    return ProcessHelperDetails::GetProcessID(path, pid);
#endif
    return false;
}

void ProcessHelper::SetActiveProcess(const QString& path)
{
#ifdef Q_OS_MAC
//    ProcessSerialNumber psn;
//    if (ProcessHelper::GetProcessPSN(path, psn))
//        SetFrontProcess(&psn);
#elif defined Q_OS_WIN
    quint32 pid = 0;
    HWND hWnd = 0;
    if (ProcessHelperDetails::GetProcessID(path, pid))
    {
        ProcessHelperDetails::ENUMINFO EnumInfo;

        // set the search parameters
        EnumInfo.PId = pid;

        // set the return parameters to default values
        EnumInfo.hWnd = NULL;
        EnumInfo.hEmptyWnd = NULL;
        EnumInfo.hInvisibleWnd = NULL;
        EnumInfo.hEmptyInvisibleWnd = NULL;

        // do the search among the top level windows
        ::EnumWindows((WNDENUMPROC)ProcessHelperDetails::EnumWindowsProc, (LPARAM)&EnumInfo);

        // return the one found if any
        if (EnumInfo.hWnd != NULL)
            hWnd = EnumInfo.hWnd;
        else if (EnumInfo.hEmptyWnd != NULL)
            hWnd = EnumInfo.hEmptyWnd;
        else if (EnumInfo.hInvisibleWnd != NULL)
            hWnd = EnumInfo.hInvisibleWnd;
        else
            hWnd = EnumInfo.hEmptyInvisibleWnd;
    }
    if (hWnd)
        ::SetForegroundWindow(hWnd);
#endif
}
