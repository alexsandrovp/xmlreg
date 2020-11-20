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

#include <pugixml.hpp>

#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

int workOnProperty(HKEY hive, const wstring& key, REGSAM redirection, const vector<pair<wregex, wstring>>& replacements, pugi::xml_node& node)
{
	wstring name = node.attribute(L"name").value();
	wstring stype = node.attribute(L"type").value();
	pugi::xml_text svalue = node.text();

	if (replacements.size() > 0)
	{
		for (auto par : replacements)
		{
			wstring r = regex_replace(svalue.as_string(), par.first, par.second);
			if (r != svalue.as_string()) svalue.set(r.c_str());
		}
	}
	
	DWORD type = utils::stringToPropType(stype);

	if (winreg::propertyExists(hive, key, name, redirection))
	{
		if (type != REG_MULTI_SZ)
			wcout << "warning: replacing existing value " << name << " with " << utils::propTypeToString(type) << " = " << svalue.as_string()
				<< "\n\t at " << key << endl;
		else wcout << "warning: replacing existing value " << name << " with milti-string\n\t at " << key << endl;
	}
	else if (!winreg::keyExists(hive, key, redirection) && !winreg::createKey(hive, key, redirection))
	{
		wcout << "error: failed to create key\n\tat " << utils::redirectionToString(redirection) << key << endl;
		return ERROR_XRIMPORT_CREATEKEY;
	}

	switch (type)
	{
	case REG_SZ:
		if (!winreg::setString(hive, key, name, svalue.as_string(), redirection))
		{
			wcout << "error: failed to write string: " << name << ":" << svalue << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
		break;
	case REG_EXPAND_SZ:
		if (!winreg::setExpandString(hive, key, name, svalue.as_string(), redirection))
		{
			wcout << "error: failed to write expand-string: " << name << ":" << svalue << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
		break;
	case REG_MULTI_SZ:
	{
		vector<wstring> list;
		for (pugi::xml_node child : node.children())
		{
			wstring cname = child.name();
			if (cname != L"li")
			{
				wcout << "warning: ignoring unrecognized child element (" << cname << ") of multi-string " << name << "\n\ton " << key << endl;
				continue;
			}
			svalue = child.text();
			list.push_back(svalue.as_string());
		}
		if (!winreg::setMultiString(hive, key, name, list, redirection))
		{
			wcout << "error: failed to write multi-string: " << name << ", length: " << list.size() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
	}
		break;
	case REG_QWORD:
	{
		if (!winreg::setQword(hive, key, name, svalue.as_llong(), redirection))
		{
			wcout << "error: failed to write qword: " << name << ":" << svalue.as_string() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
	}
		break;
	case REG_DWORD:
	{
		if (!winreg::setDword(hive, key, name, (long)svalue.as_llong(), redirection))
		{
			wcout << "error: failed to write dword: " << name << ":" << svalue.as_string() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
	}
		break;
	case REG_DWORD_BIG_ENDIAN:
	{
		if (!winreg::setDwordBE(hive, key, name, (long)svalue.as_llong(), redirection))
		{
			wcout << "error: failed to write dword-be: " << name << ":" << svalue.as_string() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
	}
		break;
	case REG_BINARY:
		if (!winreg::setBinaryFromBase64(hive, key, name, utf8_from_wstring(svalue.as_string()), redirection))
		{
			wcout << "error: failed to write binary: " << name << ":" << svalue.as_string() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
		break;

	//case REG_NONE:
	//case REG_LINK:
	//case REG_RESOURCE_LIST:
	//case REG_FULL_RESOURCE_DESCRIPTOR:
	//case REG_RESOURCE_REQUIREMENTS_LIST:
	default:
		if (!winreg::setByteArrayFromBase64(hive, key, name, utf8_from_wstring(svalue.as_string()), type, redirection))
		{
			wcout << "error: failed to write " << utils::propTypeToString(type) << ": "
				<< name << ":" << svalue.as_string() << "\n\ton " << key;
			if (!xrerror_mode) return ERROR_XRIMPORT_SETPROPERTY;
		}
		break;
	}

	return 0;
}

int convertNode(HKEY hive, const wstring& key, REGSAM redirection, const vector<pair<wregex, wstring>>& replacements, pugi::xml_node& node)
{
	int ret = 0;
	for (pugi::xml_node child : node.children())
	{
		wstring s = child.name();
		if (s.length() == 0) continue;
		if (s == L"value")
		{
			ret = workOnProperty(hive, key, redirection, replacements, child);
			if (ret && !xrerror_mode) return ret;
		}
		else if (s == L"key")
		{
			s = child.attribute(L"name").value();
			wstring subkey = key + L"\\" + s;
			if (winreg::createKey(hive, subkey, redirection))
			{
				ret = convertNode(hive, subkey, redirection, replacements, child);
				if (ret && !xrerror_mode) return ret;
			}
			else
			{
				wcout << "error: failed to create key: " << subkey << endl;
				if (!xrerror_mode) return ERROR_XRIMPORT_CREATEKEY;
			}
		}
		else wcout << "warning: ignoring unknown element " << s << endl;
	}
	return ret;
}

int import_reg(wstring file, map<wstring, wstring> replacements, bool unattended)
{
	std::wcout << "importing from file " << file << std::endl;

	pugi::xml_document doc;
	auto parse_result = doc.load_file(file.c_str());
	if (parse_result.status == pugi::status_ok)
	{
		auto root = doc.first_element_by_path(L"fragment");
		bool isFragment = root.name() == wstring(L"fragment");

		if (isFragment)
		{
			wstring ahive = root.attribute(L"hive").value();
			wstring akey = root.attribute(L"key").value();
			wstring aredir = root.attribute(L"redirection").value();

			if (ahive.length() == 0)
				wcout << "warning: no hive, assuming HKCU" << endl;

			wstring key = akey;
			HKEY hive = utils::stringToHive(ahive);
			REGSAM redirection = utils::stringToRedirection(aredir);

			wcout << "to ("
				<< (redirection ? utils::redirectionToString(redirection) : L"0")
				<< L"): " << utils::hiveToString(hive) << L":\\" << key << endl;

			bool ok_to_go = true;
			if (winreg::keyExists(hive, key, redirection))
			{
				wcout << "warning: target key already exists, trees will be merged and some values might be overwritten" << endl;
				if (!unattended)
				{
					wstring option;
					wcout << "continue? (y/N): ";
					wcin >> option;
					transform(option.begin(), option.end(), option.begin(), ::tolower);
					ok_to_go = option == L"1" || option == L"y" || option == L"yes" || option == L"true";
				}
			}

			if (ok_to_go)
			{
				vector<pair<wregex, wstring>> rgxmap;
				for (auto repl : replacements)
				{
					pair<wregex, wstring> p(wregex(repl.first), repl.second);
					rgxmap.push_back(p);
				}
				convertNode(hive, key, redirection, rgxmap, root);
				return 0;
			}
		}
		else
		{
			wcout << "error: root element is not 'fragment'" << endl;
			return ERROR_XRIMPORT_XMLSCHEMA;
		}
	}
	else
	{
		wcout << "error: " << parse_result.description() << endl;
		return ERROR_XRIMPORT_PARSEXML;
	}
	return ERROR_XRGENERAL_FAILURE;
}