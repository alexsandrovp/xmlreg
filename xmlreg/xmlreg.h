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

#pragma once

#include <map>
#include <string>

#include <windows.h>

#define EXCEPTION_CODE(error_code) system_error(error_code, generic_category())

#define ERROR_USAGE_TOO_FEW_ARGUMENTS					1
#define ERROR_USAGE_IMPORT_AND_EXPORT_AND_WIPE			2
#define ERROR_USAGE_NOIMPORT_AND_NOEXPORT_AND_NOWIPE	3
#define ERROR_USAGE_NO_FILE								4
#define ERROR_USAGE_PARAMETER_WITHOUT_SWITCH			5
#define ERROR_USAGE_NO_INPUT_HIVE						6
#define ERROR_USAGE_NO_OUTPUT_HIVE						7
#define ERROR_USAGE_NO_REPLACE_AFTER_MATCH				8

#define ERROR_GENERAL_FAILURE					100

bool import_reg(std::wstring file, std::map<std::wstring, std::wstring> replacements, bool unattended);
bool wipe_reg(std::wstring file);
bool export_reg(std::wstring file,
	HKEY input_hive, std::wstring input_key, REGSAM input_redirection,
	HKEY output_hive, std::wstring output_key, REGSAM output_redirection,
	bool unattended);

std::wstring errorToString(int error);

namespace utils {
	bool isWindows64();
	bool isDirectory(std::wstring file);
	bool isFile(std::wstring file);
	std::wstring hiveToString(HKEY hive);
	HKEY stringToHive(std::wstring str);
	std::wstring redirectionToString(REGSAM redirection);
	REGSAM stringToRedirection(std::wstring str);
	std::wstring propTypeToString(DWORD type);
	DWORD stringToPropType(std::wstring str);
}