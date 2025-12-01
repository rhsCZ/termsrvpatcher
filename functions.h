#pragma once
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <filesystem>
#include <vector>
#include <fstream>
#include <Windows.h>
#define BUFSIZE 4096 



template<size_t size>
void GetDataFromProcess(char(&cmdline)[size], LPSTR data);
template<size_t size>
void GetDataFromProcess(char (&cmdline)[size], LPSTR data)
{
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    DWORD dwRead, dwWritten,error, processExitCode;
    CHAR chBuf[BUFSIZE] = {'\0'};
    CHAR chBuf2[BUFSIZE] = {'\0'};
    BOOL bSuccess = FALSE;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = g_hChildStd_OUT_Wr;
    si.hStdOutput = g_hChildStd_OUT_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;
    CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);
    error = GetLastError();
    SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
    error = GetLastError();
    error = GetLastError();
    bSuccess = CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
    while (1)
    {
        GetExitCodeProcess(pi.hProcess, &processExitCode);
        if (processExitCode != STILL_ACTIVE)
            break;
        else
            Sleep(1);
    }
    CloseHandle(g_hChildStd_OUT_Wr);
    //Sleep(600);
    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        error = GetLastError();
        if (!bSuccess || dwRead == 0)
        {
            break;
        }
        else
        {
            strcat_s(chBuf2, chBuf);
        }
    }
    memcpy(data, chBuf2, BUFSIZE);
    printf(chBuf);
    printf(chBuf2);
    system("pause");
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(si.hStdOutput);
    CloseHandle(si.hStdError);
    CloseHandle(g_hChildStd_OUT_Rd);
}
#endif