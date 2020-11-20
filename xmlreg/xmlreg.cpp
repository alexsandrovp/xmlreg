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
#include "arguments.hpp"
#include "version.h"

#include <sstream>
#include <iostream>

//#ifdef _M_X64
#ifdef _WIN64
#define LOGO_STR "xmlreg build " TOOL_FILEVERSION_STR " (64 bits)"
#else
#define LOGO_STR "xmlreg build " TOOL_FILEVERSION_STR " (32 bits)"
#endif

using namespace std;

void disallowWow()
{
#if !defined (_WIN64)
	if (xrutils::isWindows64())
		throw exception("wrong architecture");
#endif
}

int wmain(int argc, wchar_t* argv[])
{
	try
	{
		wcout << LOGO_STR << endl << endl;

		disallowWow();

		arguments args(argc, argv);
		if (args.getError())
		{
			wcout << "error: " << xrutils::errorToString(args.getError()) << endl;
			return args.getError();
		}

		int xrerror_code = false;
		if (args.isImport()) xrerror_code = import_reg(args.getFile(), args.getReplacements(), args.getUnattended(), args.getSkipErrors());
		else if (args.isExport())
			xrerror_code = export_reg(args.getFile(),
				args.getInputHive(), args.getInputKey(), args.getInputRedirection(),
				args.getOutputHive(), args.getOutputKey(), args.getOutputRedirection(),
				args.getUnattended(), args.getSkipErrors());
		else if (args.isWipe()) xrerror_code = wipe_reg(args.getFile(), args.getUnattended(), args.getSkipErrors());

		if (!xrerror_code)
		{
			wcout << "completed successfully" << endl;
			return 0;
		}
		
		wcout << "failed: " << xrutils::errorToString(xrerror_code) << endl;

		return xrerror_code;
		
	}
	catch (exception& ex)
	{
		wcout << "fatal: " << ex.what() << endl << endl;
	}

	return -1;
}

