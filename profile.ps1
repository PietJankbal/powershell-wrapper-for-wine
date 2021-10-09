#put workarounds/hacks here...

#remove ~/Documents/Powershell/Modules from path; it gets spoiled because it`s not removed when one deletes the wineprefix... 
$path = $env:PSModulePath -split ';'
$env:PSModulePath  = ( $path | Select-Object -Skip 1 | Sort-Object -Unique) -join ';'

#Register-WMIEvent not available in PS Core, so just change into noop
Set-Alias Register-WMIEvent Write-Host
