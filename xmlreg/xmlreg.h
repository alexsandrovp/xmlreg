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

#define ERROR_XRUSAGE_TOO_FEW_ARGUMENTS					1
#define ERROR_XRUSAGE_IMPORT_AND_EXPORT_AND_WIPE		2
#define ERROR_XRUSAGE_NOIMPORT_AND_NOEXPORT_AND_NOWIPE	3
#define ERROR_XRUSAGE_NO_FILE							4
#define ERROR_XRUSAGE_PARAMETER_WITHOUT_SWITCH			5
#define ERROR_XRUSAGE_NO_INPUT_HIVE						6
#define ERROR_XRUSAGE_NO_OUTPUT_HIVE					7
#define ERROR_XRUSAGE_NO_REPLACE_AFTER_MATCH			8

#define ERROR_XRGENERAL_FAILURE			100

#define ERROR_XRIMPORT_PARSEXML			200
#define ERROR_XRIMPORT_XMLSCHEMA		201
#define ERROR_XRIMPORT_CREATEKEY		202
#define ERROR_XRIMPORT_SETPROPERTY		203

#define ERROR_XREXPORT_NOKEY			200
#define ERROR_XREXPORT_FILEISDIRECTORY	201
#define ERROR_XREXPORT_DONTOVERWRITE	202
#define ERROR_XREXPORT_WRITEOUTPUT1		203
#define ERROR_XREXPORT_WRITEOUTPUT2		204

#define ERROR_XRWIPE_PARSEXML			400
#define ERROR_XRWIPE_XMLSCHEMA			401
#define ERROR_XRWIPE_DELETEKEY			402
#define ERROR_XRWIPE_DELETEPROPERTY		403

int import_reg(std::wstring file, std::map<std::wstring, std::wstring> replacements,
	std::wstring com_dll, bool unattended, bool skip_errors);

int wipe_reg(std::wstring file, bool unattended, bool skip_errors);

int export_reg(std::wstring file,
	HKEY input_hive, std::wstring input_key, REGSAM input_redirection,
	HKEY output_hive, std::wstring output_key, REGSAM output_redirection,
	bool unattended, bool skip_errors);

namespace xrutils {
	bool isWindows64();
	bool isDirectory(std::wstring file);
	bool isFile(std::wstring file);
	std::wstring getFullPath(std::wstring relative, std::wstring& out_directory);
	std::wstring getShorPath(std::wstring longpath);
	std::wstring hiveToString(HKEY hive);
	HKEY stringToHive(std::wstring str);
	std::wstring redirectionToString(REGSAM redirection);
	REGSAM stringToRedirection(std::wstring str);
	std::wstring propTypeToString(DWORD type);
	DWORD stringToPropType(std::wstring str);
	std::wstring errorToString(int error);
}