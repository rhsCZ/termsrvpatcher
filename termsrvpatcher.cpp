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
#include <optional>
using Byte = unsigned char;
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
bool patched = false;

struct Pattern
{
	vector<Byte> find;
	vector<bool> mask; // true -> exact match, false -> wildcard
	vector<Byte> replace;
	bool keep_next_byte = false; // keep byte immediately after the pattern (jump offset)
};

bool matches_at(const vector<Byte>& buffer, size_t pos, const Pattern& pattern)
{
	if (buffer.size() < pos + pattern.find.size())
	{
		return false;
	}

	for (size_t i = 0; i < pattern.find.size(); ++i)
	{
		if (pattern.mask[i] && buffer[pos + i] != pattern.find[i])
		{
			return false;
		}
	}
	return true;
}

std::optional<size_t> find_pattern(const vector<Byte>& buffer, const Pattern& pattern)
{
	if (pattern.find.empty() || pattern.find.size() != pattern.mask.size())
	{
		return std::nullopt;
	}

	for (size_t i = 0; i + pattern.find.size() <= buffer.size(); ++i)
	{
		if (matches_at(buffer, i, pattern))
		{
			if (pattern.keep_next_byte && i + pattern.find.size() >= buffer.size())
			{
				continue;
			}
			return i;
		}
	}
	return std::nullopt;
}

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
	std::ofstream fout;
	std::ifstream fin;
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
	fin.open(file4, std::ios::binary);
	fout.open(file5, std::ios::binary | std::ios::out);
	if (fin.is_open() && fout.is_open())
	{
		size_t pos=0;
		vector<Byte> buffer((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());

		const vector<Byte> replace_standard = { 0xB8, 0x00, 0x01, 0x00, 0x00, 0x89, 0x81, 0x38, 0x06, 0x00, 0x00, 0x90 };
		const vector<Byte> replace_with_jump = { 0xB8, 0x00, 0x01, 0x00, 0x00, 0x89, 0x81, 0x38, 0x06, 0x00, 0x00, 0x90, 0xEB };
		const vector<Pattern> patterns = {
			// 39 81 3C 06 00 00 0F 84 xx xx xx xx
			{ { 0x39, 0x81, 0x3C, 0x06, 0x00, 0x00, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00 },
			  { true, true, true, true, true, true, true, true, false, false, false, false },
			  replace_standard },
			// 8B 99 3C 06 00 00 8B B9 38 06 00 00
			{ { 0x8B, 0x99, 0x3C, 0x06, 0x00, 0x00, 0x8B, 0xB9, 0x38, 0x06, 0x00, 0x00 },
			  vector<bool>(12, true),
			  replace_standard },
			// 39 81 3C 8B C0 48 83 C4 28 C3 CC CC
			{ { 0x39, 0x81, 0x3C, 0x8B, 0xC0, 0x48, 0x83, 0xC4, 0x28, 0xC3, 0xCC, 0xCC },
			  vector<bool>(12, true),
			  replace_standard },
			// 8B 81 38 06 00 00 39 81 3C 06 00 00 75 (offset follows)
			{ { 0x8B, 0x81, 0x38, 0x06, 0x00, 0x00, 0x39, 0x81, 0x3C, 0x06, 0x00, 0x00, 0x75 },
			  vector<bool>(13, true),
			  replace_with_jump,
			  true }
		};

		for (const auto& pattern : patterns)
		{
			std::optional<size_t> found = find_pattern(buffer, pattern);
			if (found.has_value())
			{
				pos = found.value();
				for (size_t j = 0; j < pattern.replace.size(); ++j)
				{
					buffer[pos + j] = pattern.replace[j];
				}
				// leave the following byte untouched for short jump patterns (offset stays valid)
				patched = true;
				break;
			}
		}
		if(patched)
		{
			ostream_iterator<char, char> output_iterator(fout);
			copy(buffer.begin(), buffer.end(), output_iterator);
		}
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
