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

#pragma once

#include <string>
#include <vector>

#include <Windows.h>

#pragma comment (lib, "Shlwapi.lib")

inline std::wstring wstring_from_utf8(const std::string& str);
inline std::string utf8_from_wstring(const std::wstring& str);

namespace winreg {

	HKEY splitHiveFromKey(std::string path, std::string& key);
	HKEY splitHiveFromKey(std::wstring path, std::wstring& key);

	bool keyExists(HKEY hive, std::string key, REGSAM redirection = 0);
	bool keyExists(HKEY hive, std::wstring key, REGSAM redirection = 0);
	bool createKey(HKEY hive, std::string key, REGSAM redirection = 0);
	bool createKey(HKEY hive, std::wstring key, REGSAM redirection = 0);

	/* careful, there is no check to see if dst in nested inside src */
	bool copyKey(HKEY srcHive, std::wstring srcKey, HKEY dstHive, std::wstring dstKey, REGSAM srcRedirection = 0, REGSAM dstRedirection = 0);

	bool propertyExists(HKEY hive, std::string key, std::string property, REGSAM redirection = 0);
	bool propertyExists(HKEY hive, std::wstring key, std::wstring property, REGSAM redirection = 0);

	DWORD getPropertyType(HKEY hive, std::string key, std::string property, REGSAM redirection = 0);
	DWORD getPropertyType(HKEY hive, std::wstring key, std::wstring property, REGSAM redirection = 0);

	std::vector<std::string> enumerateProperties(HKEY hive, std::string key, REGSAM redirection = 0);
	std::vector<std::wstring> enumerateProperties(HKEY hive, std::wstring key, REGSAM redirection = 0);

	std::vector<std::string> enumerateSubkeys(HKEY hive, std::string key, REGSAM redirection = 0);
	std::vector<std::wstring> enumerateSubkeys(HKEY hive, std::wstring key, REGSAM redirection = 0);

	bool deleteProperty(HKEY hive, std::string key, std::string property, REGSAM redirection = 0);
	bool deleteProperty(HKEY hive, std::wstring key, std::wstring property, REGSAM redirection = 0);

	bool deleteKey(HKEY hive, std::string key, std::string subkey, bool recurse = false, REGSAM redirection = 0);
	bool deleteKey(HKEY hive, std::wstring key, std::wstring subkey, bool recurse = false, REGSAM redirection = 0);
	bool killKey(HKEY hive, std::string key, REGSAM redirection = 0);
	bool killKey(HKEY hive, std::wstring key, REGSAM redirection = 0);

	// string
	std::string getString(HKEY hive, std::string key, std::string property, std::string default_value, REGSAM redirection = 0);
	std::wstring getString(HKEY hive, std::wstring key, std::wstring property, std::wstring default_value, REGSAM redirection = 0);
	bool setString(HKEY hive, std::string key, std::string property, std::string value, REGSAM redirection = 0);
	bool setString(HKEY hive, std::wstring key, std::wstring property, std::wstring value, REGSAM redirection = 0);

	//expand string
	std::string getExpandString(HKEY hive, std::string key, std::string property, std::string default_value, REGSAM redirection = 0);
	std::wstring getExpandString(HKEY hive, std::wstring key, std::wstring property, std::wstring default_value, REGSAM redirection = 0);
	bool setExpandString(HKEY hive, std::string key, std::string property, std::string value, REGSAM redirection = 0);
	bool setExpandString(HKEY hive, std::wstring key, std::wstring property, std::wstring value, REGSAM redirection = 0);

	//multistring
	bool getMultiString(HKEY hive, std::string key, std::string property, std::vector<std::string>& value, REGSAM redirection = 0);
	bool getMultiString(HKEY hive, std::wstring key, std::wstring property, std::vector<std::wstring>& value, REGSAM redirection = 0);
	bool setMultiString(HKEY hive, std::string key, std::string property, const std::vector<std::string>& value, REGSAM redirection = 0);
	bool setMultiString(HKEY hive, std::wstring key, std::wstring property, const std::vector<std::wstring>& value, REGSAM redirection = 0);

	//dword
	long getDword(HKEY hive, std::string key, std::string property, long default_value, REGSAM redirection = 0);
	long getDword(HKEY hive, std::wstring key, std::wstring property, long default_value, REGSAM redirection = 0);
	bool setDword(HKEY hive, std::string key, std::string property, long number, REGSAM redirection = 0);
	bool setDword(HKEY hive, std::wstring key, std::wstring property, long number, REGSAM redirection = 0);

	//dword be
	long getDwordBE(HKEY hive, std::string key, std::string property, long default_value, REGSAM redirection = 0);
	long getDwordBE(HKEY hive, std::wstring key, std::wstring property, long default_value, REGSAM redirection = 0);
	bool setDwordBE(HKEY hive, std::string key, std::string property, long number, REGSAM redirection = 0);
	bool setDwordBE(HKEY hive, std::wstring key, std::wstring property, long number, REGSAM redirection = 0);

	//qword
	long long getQword(HKEY hive, std::string key, std::string property, long long default_value, REGSAM redirection = 0);
	long long getQword(HKEY hive, std::wstring key, std::wstring property, long long default_value, REGSAM redirection = 0);
	bool setQword(HKEY hive, std::string key, std::string property, long long number, REGSAM redirection = 0);
	bool setQword(HKEY hive, std::wstring key, std::wstring property, long long number, REGSAM redirection = 0);

	// binary
	bool getBinary(HKEY hive, std::string key, std::string property, std::string& result, REGSAM redirection = 0);
	bool getBinary(HKEY hive, std::wstring key, std::wstring property, std::string& result, REGSAM redirection = 0);
	std::string getBinaryAsBase64(HKEY hive, std::string key, std::string property, std::string default_value, REGSAM redirection = 0);
	std::string getBinaryAsBase64(HKEY hive, std::wstring key, std::wstring property, std::string default_value, REGSAM redirection = 0);
	bool setBinary(HKEY hive, std::string key, std::string property, const char* const data, size_t datalen, REGSAM redirection = 0);
	bool setBinary(HKEY hive, std::wstring key, std::wstring property, const char* const data, size_t datalen, REGSAM redirection = 0);
	bool setBinaryFromBase64(HKEY hive, std::string key, std::string property, std::string data, REGSAM redirection = 0);
	bool setBinaryFromBase64(HKEY hive, std::wstring key, std::wstring property, std::string data, REGSAM redirection = 0);

	// other types
	bool getAsByteArray(HKEY hive, std::wstring key, std::wstring property, std::string& result, unsigned long& type, REGSAM redirection = 0);
	bool getAsByteArray(HKEY hive, std::string key, std::string property, std::string& result, unsigned long& type, REGSAM redirection = 0);
	std::string getAsBase64ByteArray(HKEY hive, std::string key, std::string property, std::string default_value, unsigned long& type, REGSAM redirection = 0);
	std::string getAsBase64ByteArray(HKEY hive, std::wstring key, std::wstring property, std::string default_value, unsigned long& type, REGSAM redirection = 0);
	bool setByteArray(HKEY hive, std::wstring key, std::wstring property, const char* const data, size_t datalen, unsigned long type, REGSAM redirection = 0);
	bool setByteArray(HKEY hive, std::string key, std::string property, const char* const data, size_t datalen, unsigned long type, REGSAM redirection = 0);
	bool setByteArrayFromBase64(HKEY hive, std::string key, std::string property, std::string data, unsigned long type, REGSAM redirection = 0);
	bool setByteArrayFromBase64(HKEY hive, std::wstring key, std::wstring property, std::string data, unsigned long type, REGSAM redirection = 0);

	namespace remap {
		bool start(HKEY sourceHive, HKEY targetHive, const std::string& targetKey, REGSAM redirection);
		bool start(HKEY sourceHive, HKEY targetHive, const std::wstring& targetKey, REGSAM redirection);
		bool stop(HKEY sourceHive);
	}
}