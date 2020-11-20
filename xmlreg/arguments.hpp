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

#include "xmlreg.h"

#include <map>
#include <string>
#include <iostream>

#include <windows.h>

class arguments
{
	bool import = false;
	bool exprt = false;
	bool wipe = false;
	bool unattended = false;
	bool skip_err = false;
	int error_code = 0;

	std::wstring file;
	std::wstring program_path;

	HKEY input_hive = HKEY_CURRENT_USER, output_hive = HKEY_CURRENT_USER;
	REGSAM input_redirection = 0, output_redirection = 0;
	std::wstring input_key, output_key;

	std::map<std::wstring, std::wstring> matches;

public:
	arguments(int argc, wchar_t* argv[])
	{
		if (argc < 3)
		{
			error_code = ERROR_XRUSAGE_TOO_FEW_ARGUMENTS;
			return;
		}

		std::wstring current_switch, current_match;
		std::map<std::wstring, std::wstring> tokens;
		program_path = argv[0];

		for (int i = 1; i < argc; ++i)
		{
			std::wstring token = argv[i];
			bool is_switch = current_switch.length() == 0 && token[0] == L'-';
			if (is_switch)
			{
				current_switch = token;
				if ((current_match.length() == 0 && (current_switch == L"-rp" || current_switch == L"--replace"))
					|| (current_match.length() > 0 && current_switch != L"-rp" && current_switch != L"--replace"))
				{
					error_code = ERROR_XRUSAGE_NO_REPLACE_AFTER_MATCH;
					return;
				}
				if (current_switch == L"-y" || current_switch == L"--unattended")
				{
					tokens[L"unattended"] = L"true";
					current_switch = L"";
				}
				else if (current_switch == L"-se" || current_switch == L"--skip-errors")
				{
					tokens[L"skip-errors"] = L"true";
					current_switch = L"";
				}
			}
			else
			{
				if (current_switch == L"-i" || current_switch == L"--import")
					tokens[L"import"] = token;
				else if (current_switch == L"-e" || current_switch == L"--export")
					tokens[L"export"] = token;
				else if (current_switch == L"-w" || current_switch == L"--wipe")
					tokens[L"wipe"] = token;
				else if (current_switch == L"-h" || current_switch == L"--hive")
					tokens[L"hive"] = token;
				else if (current_switch == L"-k" || current_switch == L"--key")
					tokens[L"key"] = token;
				else if (current_switch == L"-r" || current_switch == L"--redirection")
					tokens[L"redirection"] = token;
				else if (current_switch == L"-ih" || current_switch == L"--input-hive")
					tokens[L"input-hive"] = token;
				else if (current_switch == L"-ik" || current_switch == L"--input-key")
					tokens[L"input-key"] = token;
				else if (current_switch == L"-ir" || current_switch == L"--input-redirection")
					tokens[L"input-redirection"] = token;
				else if (current_switch == L"-oh" || current_switch == L"--output-hive")
					tokens[L"output-hive"] = token;
				else if (current_switch == L"-ok" || current_switch == L"--output-key")
					tokens[L"output-key"] = token;
				else if (current_switch == L"-or" || current_switch == L"--output-redirection")
					tokens[L"output-redirection"] = token;
				else if (current_switch == L"-m" || current_switch == L"--match")
					current_match = token;
				else if (current_switch == L"-rp" || current_switch == L"--replace")
				{
					matches[current_match] = token;
					current_match = L"";
				}
				else if (current_switch.length() == 0)
				{
					error_code = ERROR_XRUSAGE_PARAMETER_WITHOUT_SWITCH;
					return;
				}

				current_switch = L"";
			}
		}

		skip_err = tokens.find(L"skip-errors") != tokens.end();
		unattended = tokens.find(L"unattended") != tokens.end();

		bool hasImport = tokens.find(L"import") != tokens.end();
		bool hasExport = tokens.find(L"export") != tokens.end();
		bool hasWipe = tokens.find(L"wipe") != tokens.end();
		if ((hasImport && hasExport) || (hasImport && hasWipe) || (hasExport && hasWipe))
		{
			error_code = ERROR_XRUSAGE_IMPORT_AND_EXPORT_AND_WIPE;
			return;
		}
		if (!hasImport && !hasExport && !hasWipe)
		{
			error_code = ERROR_XRUSAGE_NOIMPORT_AND_NOEXPORT_AND_NOWIPE;
			return;
		}

		if (hasImport) file = tokens[L"import"];
		else if (hasExport) file = tokens[L"export"];
		else if (hasWipe) file = tokens[L"wipe"];

		if (file.length() == 0) error_code = ERROR_XRUSAGE_NO_FILE;

		bool hasHive = tokens.find(L"hive") != tokens.end();
		bool hasKey = tokens.find(L"key") != tokens.end();
		bool hasRedir = tokens.find(L"redirection") != tokens.end();

		bool hasInHive = tokens.find(L"input-hive") != tokens.end();
		bool hasInKey = tokens.find(L"input-key") != tokens.end();
		bool hasInRedir = tokens.find(L"input-redirection") != tokens.end();

		bool hasOutHive = tokens.find(L"output-hive") != tokens.end();
		bool hasOutKey = tokens.find(L"output-key") != tokens.end();
		bool hasOutRedir = tokens.find(L"output-redirection") != tokens.end();

		import = hasImport;
		exprt = hasExport;
		wipe = hasWipe;

		if (exprt && !hasHive)
		{
			if (!hasInHive)
			{
				error_code = ERROR_XRUSAGE_NO_INPUT_HIVE;
				return;
			}
			if (!hasOutHive)
			{
				error_code = ERROR_XRUSAGE_NO_OUTPUT_HIVE;
				return;
			}
		}

		if (hasHive && hasInHive && hasOutHive)
			std::wcout << "warning: ignoring --hive because --input-hive and --output-hive were used" << std::endl;

		if (hasKey && hasInKey && hasOutKey)
			std::wcout << "warning: ignoring --key because --input-key and --output-key were used" << std::endl;

		if (hasRedir && hasInRedir && hasOutRedir)
			std::wcout << "warning: ignoring --redirection because --input-redirection and --output-redirection were used" << std::endl;

		if (hasInHive) input_hive = xrutils::stringToHive(tokens[L"input-hive"]);
		else if (hasHive) input_hive = xrutils::stringToHive(tokens[L"hive"]);

		if (hasInKey) input_key = tokens[L"input-key"];
		else if (hasKey) input_key = tokens[L"key"];

		if (hasInRedir) input_redirection = xrutils::stringToRedirection(tokens[L"input-redirection"]);
		else if (hasRedir) input_redirection = xrutils::stringToRedirection(tokens[L"redirection"]);

		if (hasOutHive) output_hive = xrutils::stringToHive(tokens[L"output-hive"]);
		else if (hasHive) output_hive = xrutils::stringToHive(tokens[L"hive"]);

		if (hasOutKey) output_key = tokens[L"output-key"];
		else if (hasKey) output_key = tokens[L"key"];

		if (hasOutRedir) output_redirection = xrutils::stringToRedirection(tokens[L"output-redirection"]);
		else if (hasRedir) output_redirection = xrutils::stringToRedirection(tokens[L"redirection"]);
	}

	bool isExport() { return exprt; }
	bool isImport() { return import; }
	bool isWipe() { return wipe; }
	std::wstring getFile() { return file; }

	HKEY getInputHive() { return input_hive; }
	std::wstring getInputKey() { return input_key; }
	REGSAM getInputRedirection() { return input_redirection; }

	HKEY getOutputHive() { return output_hive; }
	std::wstring getOutputKey() { return output_key; }
	REGSAM getOutputRedirection() { return output_redirection; }

	bool getUnattended() { return unattended; }
	bool getSkipErrors() { return skip_err; }

	std::map<std::wstring, std::wstring> getReplacements() { return matches; }

	int getError()
	{
		return error_code;
	}
};
