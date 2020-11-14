# xmlreg
A tool to convert windows registry trees to/from xml files

<br>

## Import
```
xmlreg.exe --import file.xml
xmlreg.exe -i file.xml
```

## Export
```
xmlreg.exe --export file.xml --hive hklm --key Software --redirection 32
xmlreg.exe -e file.xml -h hklm -k Software -r 32
```
In this example, we are exporting the contents of __HKEY_LOCAL_MACHINE\Software__ to __file.xml__
Since we specified 32 bits redirection, this command actually exports __HKEY_LOCAL_MACHINE\Software\Wow6432Node__ on 64-bits Windows, and __HKEY_LOCAL_MACHINE\Software__ on 32-bits Windows

The tool also supports the switches __--output-hive__ (-oh), __--output-key__ (-ok) and __--output-redirection__ (-or), to save different path directions inside the exported file.

```
xmlreg.exe -e file.xml -h hklm -k Software -r 64 -oh hkcu -ok MyBackup
```

In this example, the contents of __HKEY_LOCAL_MACHINE\Software__ are exported to file.xml. When reimported, this file will recreate everything inside __HKEY_CURRENT_USER\MyBackup__

Alternatively, we could have used the switches __--input-hive__ (-ih), __--input-key__ (-ik) and __--input-redirection__ (-ir) to achieve the same results.

```
xmlreg.exe -e file.xml -ih hklm -ik Software -ir 64 -h hkcu -k MyBackup
```

Switches starting with __--input__ and __--output-__ always take precedence over __--hive__, __--key__ and __--redirection__

<br>
<br>

## File format

The xml file is always saved with UTF-8 encoding without BOM. The xml declaration will indicate the encoding used. The file is always saved idented with tabs. Tabs are better than spaces !! ;-)

The root element is called `<fragment>` and its attributes indicate where that registry fragment must be imported

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