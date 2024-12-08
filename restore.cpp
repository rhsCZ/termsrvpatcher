#pragma warning(disable:6387)
#pragma warning(disable:6054)
#pragma warning(disable:6386)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <shobjidl.h>
#include <sstream>
#include <propsys.h>
#include <filesystem>
#include <vector>
#include <fstream>
#include <Windows.h>
typedef wchar_t wchar;
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
int execute(const wchar* command);
void pause();
HRESULT GetPropertyStore(PCWSTR pszFilename, GETPROPERTYSTOREFLAGS gpsFlags, IPropertyStore** ppps);
HRESULT GetPropertyValue(PCWSTR pszFilename, PCWSTR pszCanonicalName, wstring *pszValue);
//HRESULT PrintProperty(IPropertyStore* pps, REFPROPERTYKEY key, PCWSTR pszCanonicalName);

/*HRESULT PrintProperty(IPropertyStore* pps, REFPROPERTYKEY key, PCWSTR pszCanonicalName)
{
	PROPVARIANT propvarValue = { 0 };
	HRESULT hr = pps->GetValue(key, &propvarValue);
	if (SUCCEEDED(hr))
	{
		PWSTR pszDisplayValue = NULL;
		hr = PSFormatForDisplayAlloc(key, propvarValue, PDFF_DEFAULT, &pszDisplayValue);
		if (SUCCEEDED(hr))
		{
			wprintf(L"%s = %s\n", pszCanonicalName, pszDisplayValue);
			CoTaskMemFree(pszDisplayValue);
		}
		PropVariantClear(&propvarValue);
	}
	return hr;
}*/
HRESULT GetPropertyStore(PCWSTR pszFilename, GETPROPERTYSTOREFLAGS gpsFlags, IPropertyStore** ppps)
{
	WCHAR szExpanded[MAX_PATH];
	HRESULT hr = ExpandEnvironmentStrings(pszFilename, szExpanded, ARRAYSIZE(szExpanded)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	if (SUCCEEDED(hr))
	{
		WCHAR szAbsPath[MAX_PATH];
		hr = _wfullpath(szAbsPath, szExpanded, ARRAYSIZE(szAbsPath)) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = SHGetPropertyStoreFromParsingName(szAbsPath, NULL, gpsFlags, IID_PPV_ARGS(ppps));
		}
	}
	return hr;
}
HRESULT GetPropertyValue(PCWSTR pszFilename, PCWSTR pszCanonicalName, wstring *pszValue)
{
	// Convert the Canonical name of the property to PROPERTYKEY
	PROPERTYKEY key;
	HRESULT hr = PSGetPropertyKeyFromName(pszCanonicalName, &key);
	if (SUCCEEDED(hr))
	{
		IPropertyStore* pps = NULL;

		// Call the helper to get the property store for the initialized item
		hr = GetPropertyStore(pszFilename, GPS_DEFAULT, &pps);
		if (SUCCEEDED(hr))
		{
			//hr = PrintProperty(pps, key, pszCanonicalName);
			PROPVARIANT propvarValue = { 0 };
			HRESULT hrr = pps->GetValue(key, &propvarValue);
			if (SUCCEEDED(hrr))
			{
				PWSTR pszDisplayValue = NULL;
				hrr = PSFormatForDisplayAlloc(key, propvarValue, PDFF_DEFAULT, &pszDisplayValue);
				if (SUCCEEDED(hrr))
				{
					size_t t = wcslen(pszDisplayValue);
					//wprintf(L"%s = %s\n", pszCanonicalName, pszDisplayValue);
					//wcscpy_s(test, pszDisplayValue);
					pszValue->clear();
					pszValue->append(pszDisplayValue);
					pszValue->append(L"");
					CoTaskMemFree(pszDisplayValue);
				}
				PropVariantClear(&propvarValue);
			}
			pps->Release();
		}
		else
		{
			//wprintf(L"Error %x: getting the propertystore for the item.\n", hr);
			wcout << L"Error " << hr << L" getting the propertystore for the item." << endl;
		}
	}
	else
	{
		//wprintf(L"Invalid property specified: %s\n", pszCanonicalName);
		wcout << L"Press any key to continue...." << pszCanonicalName << endl;
	}
	return hr;
}
void pause()
{
	wcout << L"Press any key to continue...." << endl;
	char c = getchar();
}

bool compareFiles(const wstring& p1, const wstring& p2) {
	wifstream f1(p1, wifstream::binary | wifstream::ate);
	wifstream f2(p2, wifstream::binary | wifstream::ate);

	if (f1.fail() || f2.fail()) {
		if (f1.is_open()) {
			f1.close();
		}
		if (f2.is_open()) {
			f2.close();
		}
		return false; //file problem
	}

	if (f1.tellg() != f2.tellg()) {
		if (f1.is_open()) {
			f1.close();
		}
		if (f2.is_open()) {
			f2.close();
		}
		return false; //size mismatch
	}

	//seek back to beginning and use equal to compare contents
	f1.seekg(0, wifstream::beg);
	f2.seekg(0, wifstream::beg);
	bool ret = equal(istreambuf_iterator<wchar>(f1.rdbuf()), istreambuf_iterator<wchar>(), istreambuf_iterator<wchar>(f2.rdbuf()));
	if (f1.is_open()) {
		f1.close();
	}
	if (f2.is_open()) {
		f2.close();
	}
	return ret;
}
int execute(const wchar* command)
{
	wchar empty = *"";
	wchar cmdbuf[500] = { '\0' };
	wcscpy_s(cmdbuf, command);
	wchar* file = nullptr, * params = nullptr;
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
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
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
	}
	else
	{
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	}
	DWORD rv;
	GetExitCodeProcess(ShExecInfo.hProcess, &rv);
	CloseHandle(ShExecInfo.hProcess);

	return rv;
}
int main(int argc, char** argv)
{
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (hr == S_OK)
	{ 
		wchar *buf = nullptr;
		buf = (wchar*)malloc(UNICODE_STRING_MAX_CHARS);
		if (buf == nullptr)
		{
			wcout << L"couldn't allocate memory for buffer" << endl;
			return -1;
		}
		wstring file1 = { '\0' }, file2 = { '\0' };
		file1.assign(L"\\\\?\\C:\\Windows\\System32\\termsrv.dll");
		file2.assign(L"\\\\?\\C:\\Windows\\System32\\termsrv-bak.dll");

		if (compareFiles(file1, file2))
		{
			wcout << "termsrv.dll and termsrv.dll.bak is same. No restoration needed!!" << endl;
			pause();
			free(buf);
			return -1;
		}
		else
		{
			size_t x = (UNICODE_STRING_MAX_CHARS * sizeof(wchar) - 1);
			wstring prop1 = { '\0' }, prop2 = { '\0' };
			memset(buf, 0, UNICODE_STRING_MAX_CHARS);
			swscanf_s(file1.c_str(), L"\\\\?\\%s", buf,MAX_PATH);
			GetPropertyValue(buf, L"System.Software.ProductVersion", &prop1);
			memset(buf, 0, UNICODE_STRING_MAX_CHARS);
			swscanf_s(file2.c_str(), L"\\\\?\\%s", buf, MAX_PATH);
			GetPropertyValue(buf, L"System.Software.ProductVersion", &prop2);
			if (!prop1.compare(prop2))
			{
				
					execute(L"sc config termservice start=disabled");
					execute(L"sc config umrdpservice start=disabled");
					execute(L"sc stop umrdpservice");
					Sleep(1500);
					execute(L"sc stop termservice");
					execute(L"taskkill /FI \"SERVICES eq umrdpservice\" /F");
					execute(L"taskkill /FI \"SERVICES eq termservice\" /F");
					execute(L"takeown /A /F C:\\Windows\\System32\\termsrv.dll");
					execute(L"icacls C:\\Windows\\System32\\termsrv.dll /grant Administrators:F");
					DeleteFileW(file1.c_str());
					CopyFileW(file2.c_str(), file1.c_str(),FALSE);
					execute(L"sc config termservice start=auto");
					execute(L"sc config umrdpservice start=auto");
					execute(L"sc start umrdpservice");
					execute(L"sc start termservice");
					wcout << L"succesfully restored!!" << endl;
					free(buf);
					return 0;
				

			}
			else
			{
				wcout << "File versions is different between termsrv.dll and termsrv.dll.bak" << endl;
				wcout << "Prop1 = " << prop1 << endl;
				wcout << "Prop2 = " << prop2 << endl;
				wcout << "cannot continue!!" << endl;
				free(buf);
				pause();
				return -1;
			}
			pause();
		}
	
	}
	else
	{
		printf("Something went wrong druing initialization!!");
		return -1;
	}
	return 0;
}
