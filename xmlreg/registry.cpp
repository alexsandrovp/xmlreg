/*
Copyright (c) 2016 Alex Vargas

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

#include "registry.h"
#include "base64.h"

#include <codecvt>

#include <Shlwapi.h>

using namespace std;

inline string b64encode(const char* data, size_t data_length)
{
	char* buffer = nullptr;
	size_t len = base64_encode(data, data_length, &buffer, 1);
	string ret(buffer, len);
	delete[] buffer;
	return ret;
}

inline string b64decode(string data)
{
	char* buffer = nullptr;
	size_t len = base64_decode(data.c_str(), data.length(), &buffer);
	string ret(buffer, len);
	delete[] buffer;
	return ret;
}

inline wstring wstring_from_utf8(const string& str)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

inline string utf8_from_wstring(const wstring& str)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
}

namespace winreg {

	HKEY splitHiveFromKey(string path, string& key)
	{
		wstring wk;
		HKEY ret = splitHiveFromKey(wstring_from_utf8(path), wk);
		key = utf8_from_wstring(wk);
		return ret;
	}

	HKEY splitHiveFromKey(wstring path, wstring& key)
	{
		key = L"";
		HKEY ret = HKEY_CURRENT_USER;
		if (!path.empty())
		{
			size_t pos = path.find_first_of(L'\\');
			wstring hive = path;
			if (pos != wstring::npos)
			{
				hive = path.substr(0, pos);
				path = path.substr(pos + 1);
			}
			else path = L"";
			if (hive == L"HKLM" || hive == L"HKLM:" || hive == L"HKEY_LOCAL_MACHINE" || hive == L"HKEY_LOCAL_MACHINE:") ret = HKEY_LOCAL_MACHINE;
			if (hive == L"HKCR" || hive == L"HKCR:" || hive == L"HKEY_CLASSES_ROOT" || hive == L"HKEY_CLASSES_ROOT:") ret = HKEY_CLASSES_ROOT;
			if (hive == L"HKU" || hive == L"HKU:" || hive == L"HKEY_USERS" || hive == L"HKEY_USERS:") ret = HKEY_USERS;
			if (hive == L"HKCU" || hive == L"HKCU:" || hive == L"HKEY_CURRENT_USER" || hive == L"HKEY_CURRENT_USER:") ret = HKEY_CURRENT_USER;

			key = path;
		}
		return ret;
	}

	bool keyExists(HKEY hive, string key, REGSAM redirection)
	{
		return keyExists(hive, wstring_from_utf8(key), redirection);
	}
	bool keyExists(HKEY hive, wstring key, REGSAM redirection)
	{
		HKEY hKey = NULL;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) != ERROR_SUCCESS) return false;
		RegCloseKey(hKey);
		return true;
	}
	bool createKey(HKEY hive, string key, REGSAM redirection)
	{
		return createKey(hive, wstring_from_utf8(key), redirection);
	}
	bool createKey(HKEY hive, wstring key, REGSAM redirection)
	{
		HKEY hKey;
		bool b = RegCreateKeyExW(hive, key.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | redirection, NULL, &hKey, NULL) == ERROR_SUCCESS;
		RegCloseKey(hKey);
		return b;
	}

	bool propertyExists(HKEY hive, string key, string property, REGSAM redirection)
	{
		return propertyExists(hive, wstring_from_utf8(key), wstring_from_utf8(property), redirection);
	}
	bool propertyExists(HKEY hive, wstring key, wstring property, REGSAM redirection)
	{
		HKEY hKey;
		DWORD type, nsize = 0;
		bool ret = true;

		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_QUERY_VALUE | redirection, &hKey) != ERROR_SUCCESS)
			return false;

		if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, NULL, &nsize) == ERROR_FILE_NOT_FOUND)
			ret = false;

		RegCloseKey(hKey);
		return ret;
	}

	DWORD getPropertyType(HKEY hive, string key, string property, REGSAM redirection)
	{
		return getPropertyType(hive, wstring_from_utf8(key), wstring_from_utf8(property), redirection);
	}
	DWORD getPropertyType(HKEY hive, wstring key, wstring property, REGSAM redirection)
	{
		HKEY hKey;
		DWORD type;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, NULL, NULL) == ERROR_SUCCESS)
				return type;
		}
		return REG_NONE;
	}

	vector<string> enumerateProperties(HKEY hive, string key, REGSAM redirection)
	{
		vector<string> ret;
		vector<wstring> res = enumerateProperties(hive, wstring_from_utf8(key), redirection);
		for (auto it = res.begin(); it != res.end(); ++it) ret.push_back(utf8_from_wstring(*it));
		return ret;
	}
	vector<wstring> enumerateProperties(HKEY hive, wstring key, REGSAM redirection)
	{
		vector<wstring> ret;
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_QUERY_VALUE | redirection, &hKey) == ERROR_SUCCESS)
		{
			const unsigned long sz = 32767;
			unsigned long size = sz;
			unsigned long i = 0, loop = 0;
			wchar_t* buffer = new wchar_t[sz];
			while (loop != ERROR_NO_MORE_ITEMS)
			{
				size = sz;
				loop = RegEnumValueW(hKey, i++, buffer, &size, NULL, NULL, NULL, NULL);
				if (loop == ERROR_SUCCESS) ret.push_back(buffer);
			}

			delete[] buffer;
			RegCloseKey(hKey);
		}
		return ret;
	}

	vector<string> enumerateSubkeys(HKEY hive, string key, REGSAM redirection)
	{
		vector<string> ret;
		vector<wstring> res = enumerateSubkeys(hive, wstring_from_utf8(key), redirection);
		for (auto it = res.begin(); it != res.end(); ++it) ret.push_back(utf8_from_wstring(*it));
		return ret;
	}
	vector<wstring> enumerateSubkeys(HKEY hive, wstring key, REGSAM redirection)
	{
		HKEY hKey = NULL;
		vector<wstring> ret;

		const unsigned int keyBufferSize = 256;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | redirection, &hKey) == ERROR_SUCCESS)
		{
			for (unsigned int i = 0; ; ++i)
			{
				wchar_t keyName[keyBufferSize] = { 0 };
				DWORD buffSize = keyBufferSize;
				if (RegEnumKeyExW(hKey, i, keyName, &buffSize, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS) break;
				if (keyName[0] != L'\0') ret.push_back(keyName);
			}
			RegCloseKey(hKey);
		}

		return ret;
	}

	bool deleteProperty(HKEY hive, string key, string property, REGSAM redirection)
	{
		return deleteProperty(hive, wstring_from_utf8(key), wstring_from_utf8(property), redirection);
	}
	bool deleteProperty(HKEY hive, wstring key, wstring property, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), NULL, KEY_ALL_ACCESS | redirection, &hKey) == ERROR_SUCCESS)
		{
			RegDeleteValueW(hKey, property.c_str());
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}

	bool killKey(HKEY hive, string key, REGSAM redirection)
	{
		return deleteKey(hive, key, "", true, redirection);
	}
	bool killKey(HKEY hive, wstring key, REGSAM redirection)
	{
		return deleteKey(hive, key, L"", true, redirection);
	}
	bool deleteKey(HKEY hive, string key, string subkey, bool recurse, REGSAM redirection)
	{
		return deleteKey(hive, wstring_from_utf8(key), wstring_from_utf8(subkey), recurse, redirection);
	}
	bool deleteKey(HKEY hive, wstring key, wstring subkey, bool recurse, REGSAM redirection)
	{
		HKEY hKey;
		bool ret = false;
		if (!key.empty() && key[key.length() - 1] != L'\\') key += L"\\";
		if (!keyExists(hive, key + subkey, redirection)) return true;
		if (RegOpenKeyExW(hive, key.c_str(), NULL, KEY_ALL_ACCESS | redirection, &hKey) == ERROR_SUCCESS)
		{
			if (recurse) ret = SHDeleteKeyW(hKey, subkey.c_str()) == ERROR_SUCCESS ? true : false;
			else ret = RegDeleteKeyW(hKey, subkey.c_str()) == ERROR_SUCCESS ? true : false;
			RegCloseKey(hKey);
		}

		return ret;
	}

	bool copyKey(HKEY srcHive, wstring srcKey, HKEY dstHive, wstring dstKey, REGSAM srcRedirection, REGSAM dstRedirection)
	{
		HKEY hSKey, hDKey;
		bool ret = false;
		bool dstCreated = false;

		/*
		*	If you want to compile this for versions of windows that don't have RegCopyTreeW (windows XP), you can dynamically check for the function:
		* 
			typedef LSTATUS(WINAPI* PtrRegCopyTreeW)(HKEY, LPCWSTR, HKEY);
			PtrRegCopyTreeW func = (PtrRegCopyTreeW)GetProcAddress(GetModuleHandleW(L"Advapi32.dll"), "RegCopyTreeW");
			ret = func(hSKey, L"", hDKey) == ERROR_SUCCESS;
		*/

		if (RegOpenKeyExW(srcHive, srcKey.c_str(), 0, KEY_READ | srcRedirection, &hSKey) == ERROR_SUCCESS)
		{
			if (!keyExists(dstHive, dstKey, dstRedirection))
				dstCreated = createKey(dstHive, dstKey, dstRedirection);

			if (RegOpenKeyExW(dstHive, dstKey.c_str(), 0, KEY_ALL_ACCESS | dstRedirection, &hDKey) == ERROR_SUCCESS)
			{
				ret = RegCopyTreeW(hSKey, L"", hDKey) == ERROR_SUCCESS;
				RegCloseKey(hDKey);
			}
			RegCloseKey(hSKey);
		}

		if (!ret && dstCreated && keyExists(dstHive, dstKey, dstRedirection))
			killKey(dstHive, dstKey, dstRedirection);

		return ret;
	}

	// string
	string getString(HKEY hive, string key, string property, string default_value, REGSAM redirection)
	{
		return utf8_from_wstring(getString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wstring_from_utf8(default_value), redirection));
	}
	wstring getString(HKEY hive, wstring key, wstring property, wstring default_value, REGSAM redirection)
	{
		wstring ret = L"";
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			DWORD type;
			DWORD nsize = 0;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, NULL, &nsize) == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ))
			{
				BYTE* buffer = new BYTE[nsize];
				if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &nsize) == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ))
				{
					for (DWORD i = 0; i < nsize; i += 2)
					{
						wchar_t c = (wchar_t)buffer[i] + (((wchar_t)buffer[i + 1]) << 8);
						if (c == L'\0') break;
						ret += c;
					}
				}
				RegCloseKey(hKey);
				delete[] buffer;
				return ret;
			}
		}
		return default_value;
	}
	bool setString(HKEY hive, string key, string property, string value, REGSAM redirection)
	{
		return setString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wstring_from_utf8(value), redirection);
	}
	bool setString(HKEY hive, wstring key, wstring property, wstring value, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			const size_t size = value.length() * 2 + 2;
			BYTE* buffer = new BYTE[size];
			for (size_t i = 0, j = 0; j < size - 1 && i < value.length(); ++i, j += 2)
			{
				buffer[j] = (BYTE)(value[i] & 0xFF);
				buffer[j + 1] = (BYTE)((value[i] & 0xFF00) >> 8);
			}
			buffer[size - 2] = 0;
			buffer[size - 1] = 0;
			RegSetValueExW(hKey, property.c_str(), NULL, REG_SZ, buffer, (DWORD)size);
			delete[] buffer;
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}

	//expand string
	string getExpandString(HKEY hive, string key, string property, string default_value, REGSAM redirection)
	{
		return utf8_from_wstring(getExpandString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wstring_from_utf8(default_value), redirection));
	}
	wstring getExpandString(HKEY hive, wstring key, wstring property, wstring default_value, REGSAM redirection)
	{
		wstring ret = getString(hive, key, property, default_value, redirection);
		DWORD size = 0;
		size = ExpandEnvironmentStringsW(ret.c_str(), NULL, size);
		if (size == 0) return default_value;
		wchar_t* buffer = new wchar_t[size];
		size = ExpandEnvironmentStringsW(ret.c_str(), buffer, size);
		if (size == 0) ret = default_value;
		else ret = buffer;
		delete[] buffer;
		return ret;
	}
	bool setExpandString(HKEY hive, string key, string property, string value, REGSAM redirection)
	{
		return setExpandString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wstring_from_utf8(value), redirection);
	}
	bool setExpandString(HKEY hive, wstring key, wstring property, wstring value, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			const size_t size = value.length() * 2 + 2;
			BYTE* buffer = new BYTE[size];
			for (unsigned int i = 0, j = 0; j < size - 1 && i < value.length(); ++i, j += 2)
			{
				buffer[j] = (BYTE)(value[i] & 0xFF);
				buffer[j + 1] = (BYTE)((value[i] & 0xFF00) >> 8);
			}
			buffer[size - 2] = 0;
			buffer[size - 1] = 0;
			RegSetValueExW(hKey, property.c_str(), NULL, REG_EXPAND_SZ, buffer, (DWORD)size);
			delete[] buffer;
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}

	//multistring
	bool getMultiString(HKEY hive, string key, string property, vector<string>& value, REGSAM redirection)
	{
		vector<wstring> wvalue;
		bool ret = getMultiString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wvalue, redirection);
		for (auto it = wvalue.begin(); it != wvalue.end(); ++it)
			value.push_back(utf8_from_wstring(*it));
		return ret;
	}
	bool getMultiString(HKEY hive, wstring key, wstring property, vector<wstring>& value, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			DWORD buffersize = 1;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, NULL, NULL, &buffersize) == ERROR_SUCCESS && buffersize > 1)
			{
				DWORD type;
				BYTE* buffer = new BYTE[buffersize];
				if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &buffersize) == ERROR_SUCCESS && type == REG_MULTI_SZ)
				{
					BYTE* pb = buffer;
					while (pb < buffer + buffersize)
					{
						wstring tmp = L"";
						while (true)
						{
							if (pb[0] == 0 && pb[1] == 0) break;
							tmp += (wchar_t)(*pb) + (((wchar_t)(*(pb + 1)) << 8));
							pb += 2;
						}

						pb += 2;
						if (!tmp.empty()) value.push_back(tmp);
					}
				}
				delete[] buffer;
				RegCloseKey(hKey);
				return true;
			}
			RegCloseKey(hKey);
		}
		return false;
	}
	bool setMultiString(HKEY hive, string key, string property, const vector<string>& value, REGSAM redirection)
	{
		vector<wstring> wvalue;
		for (auto it = value.begin(); it != value.end(); ++it)
			wvalue.push_back(wstring_from_utf8(*it));
		return setMultiString(hive, wstring_from_utf8(key), wstring_from_utf8(property), wvalue, redirection);
	}
	bool setMultiString(HKEY hive, wstring key, wstring property, const vector<wstring>& value, REGSAM redirection)
	{
		bool ret = false;
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_FILE_NOT_FOUND)
		{
			RegCreateKeyW(hive, key.c_str(), &hKey);
			RegCloseKey(hKey);
		}
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			size_t trueSize = 2;
			size_t buffersize = 0;
			vector<wstring>::const_iterator it = value.begin();
			for (; it != value.end(); ++it) buffersize += it->length() * 2 + 2;
			buffersize += 2;

			size_t i = 0;
			BYTE* buffer;
			BYTE* pb;
			buffer = new BYTE[buffersize];
			memset(buffer, 0, buffersize);
			it = value.begin();
			for (pb = buffer; i < buffersize && it != value.end(); ++it)
			{
				size_t k = 0;
				if (it->length() > 0)
				{
					for (size_t j = 0; j < it->length(); ++j, k += 2)
					{
						wchar_t wc = (*it)[j];
						pb[k] = (BYTE)(wc & 0xFF);
						pb[k + 1] = (BYTE)((wc & 0xFF00) >> 8);
					}
					i += k + 2;
					trueSize += k + 2;
					pb = buffer + i;
				}
			}

			if (RegSetValueExW(hKey, property.c_str(), NULL, REG_MULTI_SZ, (BYTE*)buffer, (DWORD)trueSize) == ERROR_SUCCESS)
				ret = true;

			RegCloseKey(hKey);
		}
		return ret;
	}



	//dword
	long getDword(HKEY hive, string key, string property, long default_value, REGSAM redirection)
	{
		return getDword(hive, wstring_from_utf8(key), wstring_from_utf8(property), default_value, redirection);
	}
	long getDword(HKEY hive, wstring key, wstring property, long default_value, REGSAM redirection)
	{
		long ret = 0;
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			const int size = sizeof(DWORD);
			DWORD nsize = size;
			BYTE buffer[size];
			DWORD type;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &nsize) == ERROR_SUCCESS && type == REG_DWORD)
			{
				memcpy(&ret, buffer, size);
				RegCloseKey(hKey);
				return ret;
			}
			RegCloseKey(hKey);
		}
		return default_value;
	}
	bool setDword(HKEY hive, string key, string property, long number, REGSAM redirection)
	{
		return setDword(hive, wstring_from_utf8(key), wstring_from_utf8(property), number, redirection);
	}
	bool setDword(HKEY hive, wstring key, wstring property, long number, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			DWORD val = number;
			RegSetValueExW(hKey, property.c_str(), NULL, REG_DWORD, (BYTE*)&val, sizeof(DWORD));
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}



	//dword big endian
	long getDwordBE(HKEY hive, string key, string property, long default_value, REGSAM redirection)
	{
		return getDwordBE(hive, wstring_from_utf8(key), wstring_from_utf8(property), default_value, redirection);
	}
	long getDwordBE(HKEY hive, wstring key, wstring property, long default_value, REGSAM redirection)
	{
		long ret = 0;
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			const int size = sizeof(DWORD);
			DWORD nsize = size;
			BYTE buffer[size], invertedbuffer[size];
			DWORD type;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &nsize) == ERROR_SUCCESS && type == REG_DWORD_BIG_ENDIAN)
			{
				for (int i = 0; i < size; ++i)
					invertedbuffer[i] = buffer[size - i - 1];
				memcpy(&ret, invertedbuffer, size);
				RegCloseKey(hKey);
				return ret;
			}
			RegCloseKey(hKey);
		}
		return default_value;
	}
	bool setDwordBE(HKEY hive, string key, string property, long number, REGSAM redirection)
	{
		return setDwordBE(hive, wstring_from_utf8(key), wstring_from_utf8(property), number, redirection);
	}
	bool setDwordBE(HKEY hive, wstring key, wstring property, long number, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			const int size = sizeof(DWORD);
			unsigned char* bytes = new unsigned char[size];
			unsigned char* invertedbytes = new unsigned char[size];
			memcpy(bytes, &number, size);
			for (int i = 0; i < size; ++i)
				invertedbytes[i] = bytes[size - i - 1];
			RegSetValueExW(hKey, property.c_str(), NULL, REG_DWORD_BIG_ENDIAN, (BYTE*)invertedbytes, size);
			delete[] bytes;
			delete[] invertedbytes;
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}




	//qword
	long long getQword(HKEY hive, string key, string property, long long default_value, REGSAM redirection)
	{
		return getQword(hive, wstring_from_utf8(key), wstring_from_utf8(property), default_value, redirection);
	}
	long long getQword(HKEY hive, wstring key, wstring property, long long default_value, REGSAM redirection)
	{
		long long ret = 0;
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			const int size = sizeof(long long);
			DWORD nsize = size;
			BYTE buffer[size];
			DWORD type;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &nsize) == ERROR_SUCCESS && type == REG_QWORD)
			{
				memcpy(&ret, buffer, size);
				RegCloseKey(hKey);
				return ret;
			}
			RegCloseKey(hKey);
		}
		return default_value;
	}
	bool setQword(HKEY hive, string key, string property, long long number, REGSAM redirection)
	{
		return setQword(hive, wstring_from_utf8(key), wstring_from_utf8(property), number, redirection);
	}
	bool setQword(HKEY hive, wstring key, wstring property, long long number, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			long long val = number;
			RegSetValueExW(hKey, property.c_str(), NULL, REG_QWORD, (BYTE*)&val, sizeof(long long));
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}




	// binary
	bool getBinary(HKEY hive, string key, string property, char*& data, size_t& datalen, REGSAM redirection)
	{
		return getBinary(hive, wstring_from_utf8(key), wstring_from_utf8(property), data, datalen, redirection);
	}
	bool getBinary(HKEY hive, wstring key, wstring property, char*& data, size_t& datalen, REGSAM redirection)
	{
		HKEY hKey = NULL;
		BYTE* buffer = NULL;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_READ | redirection, &hKey) == ERROR_SUCCESS)
		{
			DWORD type;
			DWORD nsize = 0;
			if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, NULL, &nsize) == ERROR_SUCCESS && (type == REG_BINARY))
			{
				BYTE* buffer = new BYTE[nsize];
				if (RegQueryValueExW(hKey, property.c_str(), NULL, &type, buffer, &nsize) == ERROR_SUCCESS && (type == REG_BINARY))
				{
					data = new char[nsize];
					memcpy(data, buffer, nsize);
					datalen = nsize;
					delete[] buffer;
					RegCloseKey(hKey);
					return true;
				}
			}
			if (buffer != NULL) delete[] buffer;
			RegCloseKey(hKey);
		}
		return false;
	}
	string getBinaryAsBase64(HKEY hive, string key, string property, string default_value, REGSAM redirection)
	{
		return getBinaryAsBase64(hive, wstring_from_utf8(key), wstring_from_utf8(property), default_value, redirection);
	}
	string getBinaryAsBase64(HKEY hive, wstring key, wstring property, string default_value, REGSAM redirection)
	{
		string ret = default_value;
		char* buffer;
		size_t datalen;
		if (getBinary(hive, key, property, buffer, datalen, redirection))
		{
			std::string s = b64encode(buffer, datalen);
			if (s.length() > 0) ret = s;
			delete[] buffer;
		}
		return ret;
	}
	bool setBinary(HKEY hive, string key, string property, const char* const data, size_t datalen, REGSAM redirection)
	{
		return setBinary(hive, wstring_from_utf8(key), wstring_from_utf8(property), data, datalen, redirection);
	}
	bool setBinary(HKEY hive, wstring key, wstring property, const char* const data, size_t datalen, REGSAM redirection)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hive, key.c_str(), 0, KEY_WRITE | redirection, &hKey) == ERROR_SUCCESS)
		{
			RegSetValueExW(hKey, property.c_str(), NULL, REG_BINARY, (BYTE*)data, (DWORD)datalen);
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}
	bool setBinaryFromBase64(HKEY hive, string key, string property, string data, REGSAM redirection)
	{
		string s = b64decode(data);
		return setBinary(hive, key, property, s.c_str(), s.length(), redirection);
	}
	bool setBinaryFromBase64(HKEY hive, wstring key, wstring property, string data, REGSAM redirection)
	{
		string s = b64decode(data);
		return setBinary(hive, key, property, s.c_str(), s.length(), redirection);
	}


	namespace remap {

		bool start(HKEY sourceHive, HKEY targetHive, const string& targetKey, REGSAM redirection)
		{
			return start(sourceHive, targetHive, wstring_from_utf8(targetKey), redirection);
		}
		bool start(HKEY sourceHive, HKEY targetHive, const wstring& targetKey, REGSAM redirection)
		{
			if (!keyExists(targetHive, targetKey, redirection))
				createKey(targetHive, targetKey, redirection);

			HKEY hRemappedKey;
			if (RegOpenKeyExW(targetHive, targetKey.c_str(), 0, KEY_READ | redirection, &hRemappedKey) == ERROR_SUCCESS)
			{
				bool ret = RegOverridePredefKey(sourceHive, hRemappedKey) == ERROR_SUCCESS;
				RegCloseKey(hRemappedKey);
				return ret;
			}
			return false;
		}
		bool stop(HKEY sourceHive)
		{
			return RegOverridePredefKey(sourceHive, NULL) == ERROR_SUCCESS;
		}
	}
}