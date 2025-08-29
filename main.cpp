#define UNICODE
#include "RtdDll.h"
#include <fstream>
#include <wininet.h>

std::wstring cmdProcess(const std::wstring &cmdLine);

int createRtdTask(Topic *topic) {
    size_t count = topic->getArgCount();
    if (count < 1) {
        return 1;
    }
    std::wstring firstArg = topic->getArg(0);

    // =RTD("rtdserver","","clock")
    if (firstArg == L"clock") {
        topic->setTask([](Topic *topic) -> int {
            SYSTEMTIME st;
            while (true) {
                GetLocalTime(&st);
                wchar_t buffer[100];
                swprintf(buffer, 100, L"【%d】🕒 %04d-%02d-%02d %02d:%02d:%02d", topic->getID(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                topic->setValue(buffer);
                Sleep(500);
            }
            return 0;
        }, true);
        return 0;
    }

    // =RTD("rtdserver","","file","C:\path\to\file.txt")
    if (firstArg == L"file") {
        if (count < 2) {
            topic->setDefaultValue(L"缺少文件路径参数");
            return 1;
        }
        topic->setValue(L"等待文件读取...");
        std::wstring filePath = topic->getArg(1);
        topic->setTask([filePath](Topic *topic) -> int {
            std::ifstream file(filePath.c_str(), std::ios::binary);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();

                if (!content.empty()) {
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, NULL, 0);
                    if (wlen > 0) {
                        std::wstring wContent(wlen - 1, 0); // -1 to exclude null terminator
                        MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, &wContent[0], wlen);
                        topic->setValue(wContent);
                    } else {
                        topic->setValue(L"文件编码转换失败");
                    }
                } else {
                    topic->setValue(L"文件为空");
                }
            } else {
                topic->setValue(L"无法打开文件: " + filePath);
            }
            return 0;
        });
        return 0;
    }

    // =RTD("rtdserver","","path")
    if (firstArg == L"path") {
        topic->setTask([](Topic *topic) -> int {
            wchar_t buffer[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, buffer);
            topic->setValue(buffer);
            return 0;
        });
        return 0;
    }

    // =RTD("rtdserver","","cmd","ls")
    if (firstArg == L"cmd") {
        if (count < 2) {
            topic->setDefaultValue(L"缺少命令参数");
            return 1;
        }
        std::wstring command = topic->getArg(1);
        topic->setTask([command](Topic *topic) -> int {
            Sleep(500);
            std::wstring result = cmdProcess(command);
            topic->setValue(result);
            return 0;
        }, true);
        return 0;
    }

    // =RTD("rtdserver","","web","https://example.com")
    if (firstArg == L"web") {
        if (count < 2) {
            topic->setDefaultValue(L"缺少URL参数");
            return 1;
        }
        std::wstring url = topic->getArg(1);
        topic->setValue(L"等待网络请求...");
        topic->setTask([url](Topic *topic) -> int {
            Sleep(500);
            HINTERNET hInternet = InternetOpen(L"RTDServer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
            if (!hInternet) {
                topic->setValue(L"无法初始化网络连接");
                return 1;
            }

            HINTERNET hUrl = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
            if (!hUrl) {
                InternetCloseHandle(hInternet);
                topic->setValue(L"无法打开URL: " + url);
                return 1;
            }

            std::string content;
            char buffer[4096];
            DWORD bytesRead;
            while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                content.append(buffer, bytesRead);
            }

            InternetCloseHandle(hUrl);
            InternetCloseHandle(hInternet);

            // Convert to wide string
            int wlen = MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, NULL, 0);
            std::wstring wContent(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, &wContent[0], wlen);

            topic->setValue(wContent);
            return 0;
        }, true);
        return 0;
    }

    topic->setDefaultValue(L"未知的命令");
    return 0;
}

std::wstring cmdProcess(const std::wstring &cmdLine) {
    /* 创建匿名管道 */
    SECURITY_ATTRIBUTES _security = {0};
    _security.bInheritHandle = TRUE;
    _security.nLength = sizeof(_security);
    _security.lpSecurityDescriptor = NULL;
    HANDLE hRead = NULL, hWrite = NULL;
    if (!CreatePipe(&hRead, &hWrite, &_security, 0)) {
        printf("创建管道失败,error code=%d \n", GetLastError());
    }
    std::wstring wCmdLine = cmdLine;
    /* 创建新进程执行cmd命令并将结果写入到管道 */
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE; // 隐藏cmd执行的窗口
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    if (!CreateProcess(NULL,
                       &wCmdLine[0],
                       NULL,
                       NULL,
                       TRUE,
                       0,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
        printf("创建子进程失败,error code=%d \n", GetLastError());
    }
    /* 等待进程执行命令结束 */
    ::WaitForSingleObject(pi.hThread, INFINITE);
    ::WaitForSingleObject(pi.hProcess, INFINITE);
    /* 从管道中读取数据 */
    DWORD bufferLen = 10240;
    char *buffer = (char *)malloc(10240);
    memset(buffer, '\0', bufferLen);
    DWORD recLen = 0;
    if (!ReadFile(hRead, buffer, bufferLen, &recLen, NULL)) {
        printf("读取管道内容失败, error code=%d\n", GetLastError());
    }
    /* 关闭句柄 */
    CloseHandle(hRead);
    CloseHandle(hWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    /* cmd命令行转换为Unicode编码 */
    int convLength = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)strlen(buffer), NULL, 0);
    if (convLength <= 0) {
        printf("字符串转换长度计算出错\n");
    }
    std::wstring ret;
    ret.resize(convLength + 10);
    convLength = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)strlen(buffer), &ret[0], (int)ret.size());
    if (convLength <= 0) {
        printf("字符串转换出错\n");
    }
    free(buffer);
    return ret;
}
