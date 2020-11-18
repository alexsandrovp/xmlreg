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

using namespace std;

void wipeNode(HKEY hive, const wstring& key, REGSAM redirection, pugi::xml_node& node)
{
	if (winreg::keyExists(hive, key, redirection))
	{
		auto properties = winreg::enumerateProperties(hive, key, redirection);
		auto subkeys = winreg::enumerateSubkeys(hive, key, redirection);

		for (pugi::xml_node child : node.children())
		{
			wstring elemname = child.name();
			if (elemname.length() == 0) continue;
			bool isKey = elemname == L"key";
			bool isValue = !isKey && elemname == L"value";
			wstring name = child.attribute(L"name").value();
			if (isValue)
			{
				if (!winreg::deleteProperty(hive, key, name, redirection))
				{
					wcout << "warning: failed to delete " << name << "\n\tfrom ("
						<< utils::redirectionToString(redirection) << ") " << key << endl;
				}
			}
			else if (isKey)
			{
				wstring subkey = key + L"\\" + name;
				wipeNode(hive, subkey, redirection, child);
			}
			else wcout << "warning: ignoring unknown element " << elemname << endl;
		}

		properties = winreg::enumerateProperties(hive, key, redirection);
		bool kill = properties.size() == 0;
		if (!kill && properties.size() == 1 && properties[0].length() == 0)
		{
			string bytes;
			unsigned long type;
			if (winreg::getAsByteArray(hive, key, L"", bytes, type, redirection))
				kill = bytes.length() == 0;
			else kill = true;
		}

		if (kill)
		{
			subkeys = winreg::enumerateSubkeys(hive, key, redirection);
			kill = subkeys.size() == 0;
		}

		if (kill)
		{
			if (!winreg::killKey(hive, key, redirection))
			{
				wcout << "warning: failed to delete empty key\n\tfrom ("
					<< utils::redirectionToString(redirection) << ") " << key << endl;
			}
		}
		else wcout << "warning: will not delete key\n\t(" << utils::redirectionToString(redirection) << ") " << key
			<< "\n\tbecause it has contents that are not defined in xml" << endl;
	}
}

bool wipe_reg(wstring file)
{
	std::wcout << "wiping from registry items defined in file " << file << std::endl;

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
				wcout << "no hive, assuming HKCU" << endl;

			wstring key = akey;
			HKEY hive = utils::stringToHive(ahive);
			REGSAM redirection = utils::stringToRedirection(aredir);

			wcout << "from ("
				<< (redirection ? utils::redirectionToString(redirection) : L"0")
				<< L"): " << utils::hiveToString(hive) << L":\\" << key << endl;

			if (!winreg::keyExists(hive, key, redirection))
			{
				return true;
			}

			wipeNode(hive, key, redirection, root);
			return true;
		}
		else wcout << "error: root element is not 'fragment'" << endl;
	}
	else wcout << "error: " << parse_result.description() << endl;
	return false;
}