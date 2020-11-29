# xmlreg
A tool to convert windows registry trees to/from xml files.

Modes of operation:

```
xmlreg.exe --export <file.xml> --hive <hive> [--key <key>] [--redirection <wow-mode>]
xmlreg.exe --import <file.xml> [--match <regex> --replace <string>] [--com-dll <path>]
xmlreg.exe --wipe <file.xml>
```

Options common to all modes:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-y`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--unattended`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Assume yes on all queries.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-se`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--skip-errors`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Don't abort on first error.

<br>

The xml file is always required. In `import` and `wipe` modes, it is the input and the Windows Registry is the output. In `export` mode, it is the other way around. See [file format](#File-format).

<br>

## Import

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-m`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--match` < regex >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Regex to search and replace. Must be followed by `--replace`. Whenever this regex pattern is found in a `string` value in the xml file, it is replaced in the Windows Registry by its companion replace pattern.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-rp`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--replace` < string >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Replacement pattern for the preceding regex.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-cd`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--com-dll` < path >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Creates four special match/replace pairs.
```
%file%      => replaced with the path given in this parameter
%dir%       => replaced with the parent path of %file%
%file83%    => same as %file%, but in dos 8.3 format
%dir83%     => same as %dir%, but in dos 8.3 format
```

<br>

Examples:
```
xmlreg.exe --import file.xml
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Imports the contents of file.xml into the registry.

<br>

```
xmlreg.exe -i file.xml -cd c:\installdir\my-com-server.dll
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Imports the contents of file.xml into the registry, replacing:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`%file%` with __c:\installdir\my-com-server.dll__  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`%dir%` with __c:\installdir__  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`%file83%` with __c:\instal\~1\my-com\~1.dll__  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`%dir83%` with __c:\instal\~1__

<br>

## Export

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-h`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--hive` < hive >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sets both the input and output hives. Can be one of the following values (case insensitive): __hklm__, __hkcu__, __hkcr__, __hku__

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-k`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--key` < key >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sets both the input and output keys.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-r`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--redirection` < wow-mode >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sets both the input and output wow redirection mode. Can be __0__, __32__ or __64__  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Using __32__ causes the program to add __KEY_WOW64_32KEY__ to the desired access rights (__REGSAM__) of [Win32 registry](https://docs.microsoft.com/en-us/windows/win32/api/winreg/) function calls.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;__64__ is equivalent to __KEY_WOW64_64KEY__.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Anything else is equivalent to __0__ (meaning Windows decides redirection based on the bitness of xmlreg.exe).  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;See [Windows registry redirector](https://docs.microsoft.com/en-us/windows/win32/winprog64/registry-redirector) to understand the rules.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-oh`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--output-hive` < hive >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--hive` in the xml output.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-ok`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--output-key` < key >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--key` in the xml output.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-or`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--output-redirection` < wow-mode >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--redirection` in the xml output.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-ih`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--input-hive` < hive >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--hive` when reading the registry.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-ik`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--input-key` < key >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--key` when reading the registry.

<br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`-ir`  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`--input-redirection` < wow-mode >  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Overrides the value set by `--redirection` when reading the registry.

<br>

Examples:

```
xmlreg.exe --export file.xml --hive hklm
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exports the entire hive __HKEY_LOCAL_MACHINE__ to __file.xml__

<br>

```
xmlreg.exe -e file.xml -h hklm -k Software\Windows
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exports the key "Software\Windows" from __HKEY_LOCAL_MACHINE__ to __file.xml__. Lets the bitness of xmlreg decide redirection.

<br>

```
xmlreg.exe -e file.xml -h hklm -k Software\Windows -r 32
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exports to __file.xml__ the contents of one of these keys from __HKEY_LOCAL_MACHINE__:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Software\Windows" from 32 bits Windows  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Software\Wow6432Node\Windows" from 64 bits Windows  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; It creates an attribute `redirection="32"` in the xml output to enforce this redirection when importing.

<br>

```
xmlreg.exe -e file.xml -h hklm -k Software\Windows -r 64
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exports the key __HKEY_LOCAL_MACHINE\Software\Windows__ to __file.xml__, regardless of the Windows and xmlreg bitness.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; It creates an attribute `redirection="64"` in the xml output to enforce this redirection when importing.

<br>

```
xmlreg.exe -e file.xml -h hklm -k Software\Windows -r 64 -or 0
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exports the key __HKEY_LOCAL_MACHINE\Software\Windows__ to __file.xml__, regardless of the Windows and xmlreg bitness.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; It doesn't create an attribute `redirection` in the xml output, and the import redirection will be decided by Windows based on the bitness of the xmlreg used to import.

<br>

```
xmlreg.exe -e file.xml -h hklm -k Software -r 64 -oh hkcu -ok MyBackup
```

In this example, the contents of __HKEY_LOCAL_MACHINE\Software__ are exported to file.xml. When reimported, this file causes xmlreg to recreate everything inside __HKEY_CURRENT_USER\MyBackup__

Alternatively, we could have used the switches __--input-hive__, __--input-key__ and __--input-redirection__ to achieve the same results.

```
xmlreg.exe -e file.xml -ih hklm -ik Software -ir 64 -h hkcu -k MyBackup
```

Switches starting with __--input__ and __--output-__ always take precedence over __--hive__, __--key__ and __--redirection__

<br>

## Wipe

```
xmlreg.exe --wipe file.xml
```

This mode is meant to be an `uninstallation` mode. After importing a file with `--import`, you can (partially) undo the changes in the registry by using wipe mode.

The tool will simply go through the `<key>` and `<value>` elements of the xml file and remove them from the registry. `<key>` elements are only removed if they are left empty after the process. `<value>` elements are always removed.

Note that `xmlreg` doesn't memorize in any way what was the original state of the registry before importing. If a value is overwritten during the import process, the original value will not be restored while using wipe mode. It will simply be removed.

<br>

## File format

The xml file is always saved with UTF-8 encoding without BOM. The xml declaration will indicate the encoding used. The file is always saved idented with tabs. Tabs are better than spaces !! ;-)

The root element is called `<fragment>` and its attributes indicate where that registry fragment must be imported.

```xml
<fragment hive="HKCU" key="Software\Classes\CLSID" redirection="32">
    ...
</fragment>
```

Registry keys are represented by the `<key>` element. Registry values are represented by the `<value>` element. Elements `<fragment>` and `<key>` can contain several instances of `<key>` and `<value>`.

The `<key>` element only accepts the attribute `name`.

```xml
<key name="Explosive Red Barrels Inc."></key>
```

The `<value>` element accepts the attributes `name` and `type`. The text content of this element is the actual value to be stored in the registry (except for multi-string).

```xml
<value name="MyToolPath" type="string">c:\mytool</value>
```

The value types map to the Windows API registry types as follows:

```
string                      <-> REG_SZ
expand-string               <-> REG_EXPAND_SZ
multi-string                <-> REG_MULTI_SZ
binary                      <-> REG_BINARY
qword                       <-> REG_QWORD
dword                       <-> REG_DWORD
dword-be                    <-> REG_DWORD_BIG_ENDIAN
link                        <-> REG_LINK
resource-list               <-> REG_RESOURCE_LIST
full-resource-descriptor    <-> REG_FULL_RESOURCE_DESCRIPTOR
resource-requirements-list  <-> REG_RESOURCE_REQUIREMENTS_LIST
none                        <-> REG_NONE
```

Types `binary`, `link`, `resource-list`, `full-resource-descriptor`, `resource-requirements-list` and `none` are treated internally as byte arrays, and are represented in the xml using base64 encoding.

```xml
<value name="MyBinData" type="binary">Y2FuIHlvdSBkZWNvZGUgdGhpcz8=</value>
```

Multi-strings are represented in xml using `<li>` elements inside `<value>`.
```xml
<value name="MyList" type="multi-string">
    <li>item 1</li>
    <li>item 2</li>
    <li>item 3</li>
    <li>item 4</li>
    <li>item 5</li>
</value>
```

Values of types `qword`, `dword` and `dword-be` are represented in xml as signed integers.

Values of type `expand-string` are treated as normal strings (env-vars are not expanded), otherwise the actual data would not be preserved through export/import cycles. However, I came across a problem inherent of the Win32 API when experimenting with `expand-string`. When calling `RegSetValueExW`, some environment variables are transformed into others, depending on the bitness of the compiled executable and the redirection chosen.

For example, if you write `%ProgramFiles%` (case sensitive) as a `REG_EXPAND_SZ` using `KEY_WOW64_32KEY` redirection by a 32-bits executable on 64-bits Windows, the actual value written is `%ProgramFiles(x86)%`. That ruins the process of exporting/reimporting, because the original data is not preserved. The only solution for this is to disallow WoW mode for this tool. (WoW stands for 'Windows on Windows' and it is what is used to run 32-bits programs on 64-bits Windows).

Therefor, if you try to run a 32-bits build of `xmlreg.exe` on 64-bits Windows, you will get an error message. Only 64-bits builds run on 64-bits Windows.

<br>

## Note for powershell users

Sometimes, you might need to use a command line with an empty string like this:

```
xmlreg.exe -h hklm -k "" -oh hkcu -ok "my backup"
```

But powershell will remove "" from the command line, resulting in the following call

```
xmlreg.exe -h hklm -k -oh hkcu -ok "my backup"
```

Which is invalid, because `--key` will be set to "-oh" (defaults to HKCU) and then `hkcu` is an invalid switch.

To ensure powershell usese the correct command line, use `'""'`:

```
xmlreg.exe -h hklm -k '""' -oh hkcu -ok "my backup"
```
