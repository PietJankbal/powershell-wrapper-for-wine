#Put workarounds/hacks here.../Adjust to your own needs. It goes into c:\\Program Files\\Powershell\\7\\profile.ps1

#Remove ~/Documents/Powershell/Modules from modulepath; it becomes a mess because it`s not removed when one deletes the wineprefix... 
$path = $env:PSModulePath -split ';' ; $env:PSModulePath  = ( $path | Select-Object -Skip 1 | Sort-Object -Unique) -join ';'

#Dry-run 'powershell.exe' once to set env vars below (after you changed them); They are written to HKCU:Environment
#To enable/disable replacing strings in the cmdline fed to pwsh.exe set/unset this env var:
[System.Environment]::SetEnvironmentVariable('PSHACKS','1','User') 
#Set strings to replace in the cmdline fed to pwsh by setting following two env vars. Use ¶ as seperator for the strings.
#Failing command: powershell.exe -noLogo -command 'ls -r "C:\windows" | measure -s Length | Select -ExpandProperty Sum'
#Powershell Core needs 'measure -sum' instead of 'measure -s' so we replace it
#Also replace for another failing command: the incompatible ' -noExit Register-WMIEvent ' with harmless ' Write-Host FIXME stub!! ' 
[System.Environment]::SetEnvironmentVariable('PS_FROM',' measure -s ¶ -noExit Register-WMIEvent ','User')
[System.Environment]::SetEnvironmentVariable('PS_TO',' measure -sum ¶ Write-Host FIXME stub!! ','User')
#End hacks

if(!(Test-Path "HKCU:\\Software\\Wine\\AppDefaults\\ConEmu64.exe")) {New-Item  -Path "HKCU:\\Software\\Wine\\AppDefaults\\ConEmu64.exe" -force
New-ItemProperty -Path 'HKCU:\\Software\\Wine\\AppDefaults\\ConEmu64.exe' -Name 'Version' -Value 'win81' -PropertyType 'String' -force }

if(!(Test-Path "HKCU:\\Software\\ConEmu\\.Vanilla")) {New-Item  -Path "HKCU:\\Software\\ConEmu\\.Vanilla" -force}
if (!(Get-ItemProperty 'HKCU:\\Software\\ConEmu\\.Vanilla').PSObject.Properties.Name -contains 'ColorTable00') {
    New-ItemProperty -Path 'HKCU:\\Software\\ConEmu\\.Vanilla' -Name 'ColorTable00' -Value '0x00562401' -PropertyType 'DWORD' -force
    New-ItemProperty -Path 'HKCU:\\Software\\ConEmu\\.Vanilla' -Name 'ColorTable14' -Value '0x0000ffff' -PropertyType 'DWORD' -force }


#Register-WMIEvent not available in PS Core, so for now just change into noop
function Register-WMIEvent
{
    exit 0
}

#Prerequisite: Stuff below requires native dotnet (winetricks -q dotnet48) to be installed, otherwise it will just fail!
#
#Only works as of wine-6.20, see https://bugs.winehq.org/show_bug.cgi?id=51871
#
#Examples of usage:
#$(Get-WmiObject win32_videocontroller).name
Function Get-WmiObject([parameter(mandatory=$true, position = 0, parametersetname = 'class')] [string]$class, `
                       [parameter( position = 1, parametersetname = 'class')][string[]]$property="*", `
                       [string]$computername = "localhost", [string]$namespace = "root\cimv2", `
                       [string]$filter, [parameter(parametersetname = 'query')] [string]$query)
{
    if (!(Test-Path  "HKLM:\Software\Microsoft\.NETFramework\v4.0.30319"))
    {  Add-Type -AssemblyName PresentationCore,PresentationFramework; [System.Windows.MessageBox]::Show('This function requires native .NET48 to be installed!   Do "winetricks -q dotnet48" and try again ','Warning','ok','exclamation'); return}
    if(!$query) {    
        $query = "SELECT " +  $($property | Join-String -Separator ",") + " FROM " + $class + (($filter) ? (' where ' + $filter ) : ('')) }

    $searcher = [wmisearcher]$query

    $searcher.scope.path = "\\" + $computername + "\" + $namespace

    return [System.Management.ManagementObjectCollection]$searcher.get()
}

Function Get-CIMInstance ( [parameter(mandatory)] [string]$classname, [string[]]$property="*", [string]$filter)
{
     Get-WMIObject $classname -property $property -filter $filter
}

Set-Alias gwmi Get-WmiObject
