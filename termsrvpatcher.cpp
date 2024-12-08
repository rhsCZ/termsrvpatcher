#pragma warning(disable:6387)
#pragma warning(disable:6386)
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <vector>
#include <fstream>
#include <Windows.h>
typedef wchar_t wchar;
int execute(const wchar* command);
void pause();
using std::endl;
using std::wcin;
using std::wcout;
using std::wstring;
using std::wfstream;
using std::wstringstream;
using std::wifstream;
using std::string;
using std::istreambuf_iterator;
using std::ostream_iterator;
using std::vector;
using std::ios;
using std::hex;
int type = 0;
bool patched = false;

void pause()
{
	printf("Press any key to continue....");
	char c = getchar();
}
int execute(const wchar* command)
{
	wchar empty = *"";
	wchar cmdbuf[500] = {'\0'};
	wcscpy_s(cmdbuf, command);
	wchar* file = nullptr, *params = nullptr;
	for (unsigned int i = 0; i < wcslen(cmdbuf); i++)
	{
		if (cmdbuf[i] == *" ")
		{
			cmdbuf[i] = *"\0";
			file = cmdbuf;
			params = &cmdbuf[i + 1];
			break;
		}
		if (i == (wcslen(cmdbuf) - 1))
		{
			file = cmdbuf;
			params = &empty;
		}
	}
	SHELLEXECUTEINFOW ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = file;
	ShExecInfo.lpParameters = params;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;

	if (ShellExecuteExW(&ShExecInfo) == FALSE && ShExecInfo.hProcess != 0)
	{ 
		return -1;
	} else
	{
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	}
	DWORD rv;
	GetExitCodeProcess(ShExecInfo.hProcess, &rv);
	CloseHandle(ShExecInfo.hProcess);

	return rv;
}
int main(int argc,char** argv)
{
	wchar* buf = nullptr;
	buf = (wchar*)malloc(UNICODE_STRING_MAX_CHARS);
	if (buf == nullptr)
	{
		wcout << L"couldn't allocate memory for buffer" << endl;
		return -1;
	}
	wstring tempdir = { '\0' }, file1 = { '\0' }, file2 = { '\0' }, file3 = { '\0' }, file4 = { '\0' }, file5 = { '\0' };
	wfstream fout;
	wfstream fin;
	wstringstream ss;
	ss.clear();
	GetEnvironmentVariableW(L"TEMP", buf, UNICODE_STRING_MAX_CHARS-20);
	ss << L"\\\\?\\" << buf << L"\\patcher";
	tempdir = ss.str();
	file1.assign(L"\\\\?\\C:\\Windows\\System32\\termsrv.dll");
	file2.assign(L"\\\\?\\C:\\Windows\\System32\\termsrv-bak.dll");
	file3.assign(L"\\\\?\\C:\\Windows\\System32\\termsrv-new.dll");
	ss << L"\\termsrv.dll";
	file4 = ss.str();
	ss.str(L"");
	ss << L"\\\\?\\" << buf << L"\\patcher" << L"\\termsrv-new.dll";
	file5 = ss.str();
	CreateDirectoryW(tempdir.c_str(), NULL);
	CopyFileW(file1.c_str(), file4.c_str(), FALSE);
	fin.open(file4, wfstream::binary | wfstream::in, _SH_DENYWR);
	fout.open(file5, wfstream::binary | wfstream::out);
	if (fin.is_open() && fout.is_open())
	{
		size_t b;
		size_t pos=0;
		vector<TCHAR> buffer(istreambuf_iterator<TCHAR>(fin), {});
		b = buffer.size();
		for (size_t i = 0; i < buffer.size(); i++)
		{
			if ((buffer[i] == 0x39 && buffer[i + 1] == 0x81 && buffer[i + 2] == 0x3c && buffer[i + 3] == 0x06 && buffer[i + 4] == 0x00 && buffer[i + 5] == 0x00 && buffer[i + 6] == 0x0F && buffer[i + 7] == 0x84)  || (buffer[i] == 0x8B && buffer[i + 1] == 0x99 && buffer[i + 2] == 0x3c && buffer[i + 3] == 0x06 && buffer[i + 4] == 0x00 && buffer[i + 5] == 0x00 && buffer[i + 6] == 0x8B && buffer[i + 7] == 0xB9) || (buffer[i] == 0x39 && buffer[i + 1] == 0x81 && buffer[i + 2] == 0x3c && buffer[i + 3] == 0x8B && buffer[i + 4] == 0xC0 && buffer[i + 5] == 0x48 && buffer[i + 6] == 0x83 && buffer[i + 7] == 0xC4))
			{
				type = 0;
				pos = i;
				patched = true;
				break;
			}
			else if (buffer[i] == 0x8B && buffer[i + 1] == 0x81 && buffer[i + 2] == 0x38 && buffer[i + 3] == 0x06 && buffer[i + 4] == 0x00 && buffer[i + 5] == 0x00 && buffer[i + 6] == 0x39 && buffer[i + 7] == 0x81)
			{
				type = 1;
				pos = i;
				patched = true;
				break;
			}
			if (i == (buffer.size() - 1))
			{
				pos = 0;
				break;
			}
		}
		if(pos != 0)
		{
			switch (type)
			{
				case 0:
				{
					buffer[pos] = 0xb8;
					buffer[pos + 1] = 0x00;
					buffer[pos + 2] = 0x01;
					buffer[pos + 3] = 0x00;
					buffer[pos + 4] = 0x00;
					buffer[pos + 5] = 0x89;
					buffer[pos + 6] = 0x81;
					buffer[pos + 7] = 0x38;
					buffer[pos + 8] = 0x06;
					buffer[pos + 9] = 0x00;
					buffer[pos + 10] = 0x00;
					buffer[pos + 11] = 0x90;
					break;
				}
				case 1:
				{
					buffer[pos] = 0xb8;
					buffer[pos + 1] = 0x00;
					buffer[pos + 2] = 0x01;
					buffer[pos + 3] = 0x00;
					buffer[pos + 4] = 0x00;
					buffer[pos + 5] = 0x89;
					buffer[pos + 6] = 0x81;
					buffer[pos + 7] = 0x38;
					buffer[pos + 8] = 0x06;
					buffer[pos + 9] = 0x00;
					buffer[pos + 10] = 0x00;
					buffer[pos + 11] = 0x90;
					buffer[pos + 12] = 0xeb;
					break;
				}
			}
		ostream_iterator<TCHAR, TCHAR> output_iterator(fout);
		copy(buffer.begin(), buffer.end(), output_iterator);
		}
			//fout.write((char*)&buffer[0], buffer.size() * sizeof(buffer));
	}
	else
	{
		wcout << L"Couldn't open files for reading/writing!!" << endl;
		pause();
		return -1;
	}
	fin.close();
	fout.close();
	if(patched)
	{
		execute(L"sc config termservice start=disabled");
		execute(L"sc config umrdpservice start=disabled");
		execute(L"sc stop umrdpservice");
		Sleep(3000);
		execute(L"sc stop termservice");
		Sleep(3000);
		execute(L"taskkill /FI \"SERVICES eq umrdpservice\" /F");
		execute(L"taskkill /FI \"SERVICES eq termservice\" /F");
		execute(L"takeown /A /F C:\\Windows\\System32\\termsrv.dll");
		execute(L"icacls C:\\Windows\\System32\\termsrv.dll /grant Administrators:F");
		DeleteFileW(file1.c_str());
		CopyFileW(file4.c_str(), file2.c_str(), FALSE);
		CopyFileW(file5.c_str(), file1.c_str(), FALSE);
		CopyFileW(file5.c_str(), file3.c_str(), FALSE);
		DeleteFileW(file4.c_str());
		DeleteFileW(file5.c_str());
		RemoveDirectoryW(tempdir.c_str());
		execute(L"sc config termservice start=auto");
		execute(L"sc config umrdpservice start=auto");
		execute(L"sc start umrdpservice");
		execute(L"sc start termservice");
		wcout << "successfully patched!!" << endl;
		pause();
		return 0;
	} else
	{
		DeleteFile(file4.c_str());
		DeleteFile(file5.c_str());
		RemoveDirectory(tempdir.c_str());
		wcout << "no patch needed!!" << endl;
		pause();
		return -1;
	}
	return 0;
}
