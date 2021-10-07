#remove ~/Documents/Powershell/Modules from path; it gets spoiled because it`s not removed when one deletes the wineprefix... 
$path = $env:PSModulePath -split ';'
$env:PSModulePath  = ( $path | Select-Object -Skip 1 | Sort-Object -Unique) -join ';'

#Register-WMIEvent not available in PS Core, so just exit
function Register-WMIEvent
{
    exit 0
}