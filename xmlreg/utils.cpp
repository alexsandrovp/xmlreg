/*
Copyright (c) 2020 Alex Vargas

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "xmlreg.h"
#include "registry.h"

#include <string>
#include <sstream>
#include <algorithm>

#include <Windows.h>

using namespace std;

namespace xrutils {

	bool isWindows64()
	{
#if defined(_WIN64)
		return true;
#elif defined(_WIN32)
		BOOL isWow = FALSE;
		return IsWow64Process(GetCurrentProcess(), &isWow) && isWow;
#else
		return false;
#endif
	}

	bool isDirectory(wstring path)
	{
		auto attributes = GetFileAttributesW(path.c_str());
		return attributes != 0xFFFFFFFF && (attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool isFile(wstring path)
	{
		auto attributes = GetFileAttributesW(path.c_str());
		return attributes != 0xFFFFFFFF && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	wstring getFullPath(wstring relative, wstring &out_directory)
	{
		auto size = GetFullPathNameW(relative.c_str(), 0, nullptr, nullptr);

		wchar_t* buffer = new wchar_t[size];
		size = GetFullPathNameW(relative.c_str(), size, buffer, nullptr);
		wstring ret(buffer, size);
		delete[] buffer;

		auto pos = ret.find_last_of(L'\\');
		if (pos == wstring::npos) pos = ret.find_last_of(L'/');
		if (pos == 0 || pos == wstring::npos) out_directory = L"";
		else out_directory = ret.substr(0, pos);
		return ret;
	}

	wstring getShorPath(wstring longpath)
	{
		auto size = GetShortPathNameW(longpath.c_str(), nullptr, 0);
		wchar_t* buffer = new wchar_t[size];
		size = GetShortPathNameW(longpath.c_str(), buffer, size);
		wstring ret(buffer, size);
		delete[] buffer;
		return ret;
	}

	wstring hiveToString(HKEY hive)
	{
		if (hive == HKEY_LOCAL_MACHINE) return L"HKLM";
		else if (hive == HKEY_CURRENT_USER) return L"HKCU";
		else if (hive == HKEY_USERS) return L"HKU";
		else if (hive == HKEY_CLASSES_ROOT) return L"HKCR";
		return L"INVALID";
	}

	HKEY stringToHive(wstring str)
	{
		wstring temp;
		transform(str.begin(), str.end(), str.begin(), ::toupper);
		return winreg::splitHiveFromKey(str, temp);
	}

	wstring redirectionToString(REGSAM redirection)
	{
		switch (redirection)
		{
		case KEY_WOW64_32KEY: return L"32";
		case KEY_WOW64_64KEY: return L"64";
		}
		return L"";
	}
	REGSAM stringToRedirection(wstring str)
	{
		if (str == L"32") return KEY_WOW64_32KEY;
		if (str == L"64") return KEY_WOW64_64KEY;
		return 0;
	}
	wstring propTypeToString(DWORD type)
	{
		switch (type)
		{
		case REG_SZ: return L"string";
		case REG_EXPAND_SZ: return L"expand-string";
		case REG_MULTI_SZ: return L"multi-string";
		case REG_BINARY: return L"binary";
		case REG_QWORD: return L"qword";
		case REG_DWORD: return L"dword";
		case REG_DWORD_BIG_ENDIAN: return L"dword-be";

		case REG_LINK: return L"link";
		case REG_RESOURCE_LIST: return L"resource-list";
		case REG_FULL_RESOURCE_DESCRIPTOR: return L"full-resource-descriptor";
		case REG_RESOURCE_REQUIREMENTS_LIST: return L"resource-requirements-list";
		}
		return L"none";
	}

	DWORD stringToPropType(wstring str)
	{
		if (str == L"string") return REG_SZ;
		if (str == L"expand-string") return REG_EXPAND_SZ;
		if (str == L"multi-string") return REG_MULTI_SZ;
		if (str == L"binary") return REG_BINARY;
		if (str == L"qword") return REG_QWORD;
		if (str == L"dword") return REG_DWORD;
		if (str == L"dword-be") return REG_DWORD_BIG_ENDIAN;
		if (str == L"link") return REG_LINK;
		if (str == L"resource-list") return REG_RESOURCE_LIST;
		if (str == L"full-resource-descriptor") return REG_FULL_RESOURCE_DESCRIPTOR;
		if (str == L"resource-requirements-list") return REG_RESOURCE_REQUIREMENTS_LIST;
		return REG_NONE;
	}

	wstring errorToString(int error)
	{
		switch (error)
		{
		case ERROR_XRUSAGE_TOO_FEW_ARGUMENTS: return L"too few arguments";
		case ERROR_XRUSAGE_IMPORT_AND_EXPORT_AND_WIPE: return L"cannot use --import, --export and --wipe at the same time";
		case ERROR_XRUSAGE_NOIMPORT_AND_NOEXPORT_AND_NOWIPE: return L"must use either --import or --export -or --wipe";
		case ERROR_XRUSAGE_NO_FILE: return L"no file specified";
		case ERROR_XRUSAGE_PARAMETER_WITHOUT_SWITCH: return L"parameter without preceding switch";
		case ERROR_XRUSAGE_NO_INPUT_HIVE: return L"no input hive";
		case ERROR_XRUSAGE_NO_OUTPUT_HIVE: return L"no output hive";
		case ERROR_XRUSAGE_NO_REPLACE_AFTER_MATCH: return L"must use --replace after --match";
		}

		wstringstream ss;
		ss << "unspecified error (" << error << ")";
		return ss.str();
	}
}
