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

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <windows.h>

using namespace std;

// recursive function
void convertKey(HKEY hive, const wstring& key, REGSAM redirection, pugi::xml_node &node)
{
	wstringstream ss;

	auto properties = winreg::enumerateProperties(hive, key, redirection);
	for (auto property : properties)
	{
		auto elem = node.append_child(L"value");
		elem.append_attribute(L"name").set_value(property.c_str());
		auto type = winreg::getPropertyType(hive, key, property, redirection);
		elem.append_attribute(L"type").set_value(utils::propTypeToString(type).c_str());
		switch (type)
		{
			case REG_QWORD:
			{
				ss.str(L"");
				auto value = winreg::getQword(hive, key, property, 0, redirection);
				ss << value;
				elem.append_child(pugi::node_pcdata).set_value(ss.str().c_str());
			}
			break;

			case REG_DWORD:
			{
				ss.str(L"");
				auto value = winreg::getDword(hive, key, property, 0, redirection);
				ss << value;
				elem.append_child(pugi::node_pcdata).set_value(ss.str().c_str());
			}
			break;

			case REG_DWORD_BIG_ENDIAN:
			{
				ss.str(L"");
				auto value = winreg::getDwordBE(hive, key, property, 0, redirection);
				ss << value;
				elem.append_child(pugi::node_pcdata).set_value(ss.str().c_str());
			}
			break;

			case REG_SZ:
			case REG_EXPAND_SZ:
			{
				auto value = winreg::getString(hive, key, property, L"", redirection);
				elem.append_child(pugi::node_pcdata).set_value(value.c_str());
			}
			break;

			case REG_MULTI_SZ:
			{
				vector<wstring> list;
				if (winreg::getMultiString(hive, key, property, list, redirection))
				{
					for (auto item : list)
					{
						auto li = elem.append_child(L"li");
						li.append_child(pugi::node_pcdata).set_value(item.c_str());
					}
				}
			}
			break;

			case REG_BINARY:
			{
				auto value = winreg::getBinaryAsBase64(hive, key, property, "", redirection);
				elem.append_child(pugi::node_pcdata).set_value(wstring_from_utf8(value).c_str());
			}
			break;

			case REG_LINK:
			case REG_RESOURCE_LIST:
			case REG_FULL_RESOURCE_DESCRIPTOR:
			case REG_RESOURCE_REQUIREMENTS_LIST:
			default:
				wcout << "ignoring unsupported registry value type: "
					<< utils::propTypeToString(type) << ", value name: " << property << "\n\ton: " << key << endl;
		}
	}

	auto subkeys = winreg::enumerateSubkeys(hive, key, redirection);
	for (auto subkey : subkeys)
	{
		auto elem = node.append_child(L"key");
		elem.append_attribute(L"name").set_value(subkey.c_str());
		if (key.length() == 0) convertKey(hive, subkey, redirection, elem);
		else convertKey(hive, key + L"\\" + subkey, redirection, elem);
	}
}

bool export_reg(wstring file, HKEY input_hive, wstring input_key, REGSAM input_redirection, HKEY output_hive, wstring output_key, REGSAM output_redirection)
{
	if (winreg::keyExists(input_hive, input_key, input_redirection))
	{
		if (utils::isDirectory(file))
		{
			wcout << "error: path already exists and is a directory" << endl;
			return false;
		}

		if (utils::isFile(file))
		{
			wstring option;
			wcout << "file already exists, overwrite? (y/N) ";
			wcin >> option;
			transform(option.begin(), option.end(), option.begin(), tolower);
			bool ok_to_go = option == L"1" || option == L"y" || option == L"yes" || option == L"true";
			if (!ok_to_go) return false;
		}

		pugi::xml_document doc;

		pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
		decl.append_attribute(L"version") = L"1.0";
		decl.append_attribute(L"encoding") = L"utf-8";

		//save empty document now to quickly detect filesystem permission denial
		if (!doc.save_file(file.c_str(), L"\t", 1, pugi::encoding_utf8))
		{
			wcout << "error: failed to save output file" << endl;
			return false;
		}

		try
		{
			auto root = doc.append_child(L"fragment");
			root.append_attribute(L"hive").set_value(utils::hiveToString(output_hive).c_str());
			root.append_attribute(L"key").set_value(output_key.c_str());
			if (output_redirection) root.append_attribute(L"redirection").set_value(utils::redirectionToString(output_redirection).c_str());
			convertKey(input_hive, input_key, input_redirection, root);
			if (doc.save_file(file.c_str(), L"\t", 1, pugi::encoding_utf8)) return true;
			else wcout << "error: failed to save output file (second attempt)" << endl;
		}
		catch (...)
		{
			DeleteFileW(file.c_str());
			throw;
		}
		
	}
	else wcout << "error: input key does not exist" << endl;
	return false;
}