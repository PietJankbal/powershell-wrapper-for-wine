# powershell-wrapper for wine

Wrapper for powershell.exe from wine ( this gets for example Waves Central in wine a bit going ).

Uses pwsh.exe from Powershell Core to get functionality for powershell.exe in wine:

Various commands fed to powershell.exe have a slightly different syntax then what pwsh.exe understands, so these commands are rewritten so pwsh.exe "understands" them.
For example 'powershell.exe -Nologo 1+2' is internally reworked to 'pwsh.exe -Nologo -c 1+2'

If the command is still incompatible with pwsh.exe there's an option to replace (parts of) the command to fix things (in profile.ps1).
See profile.ps1 for an example: the ambigous command 'measure -s' (for which pwsh will throw an error) is replaced with 'measure -sum'

Note: If you are also looking for powershell 5.1 in wine see https://github.com/PietJankbal/Chocolatey-for-wine (but that takes much more time/space to install)

# Install 

```
 wget https://github.com/PietJankbal/powershell-wrapper-for-wine/raw/master/install_pwshwrapper.exe
```

 Then do:

```
 wine install_pwshwrapper.exe
```
Powershell Core (and ConEmu) are downloaded and installed at first invokation of powershell (i.e. wine powershell`)
(ConEmu is installed to work around bug https://bugs.winehq.org/show_bug.cgi?id=49780)
For an unattended install you could do (thanks brunoais for the tip):

```
wget https://github.com/PietJankbal/powershell-wrapper-for-wine/raw/master/install_pwshwrapper.exe
wine install_pwshwrapper.exe
wine powershell -noni -c 'echo "done"'
```
# Notes about Waves Central:
- Use most recent Wine-Staging (not vanilla-wine) to avoid several issues.

- After install the 1st run will still fail with message that powershell failed; cd into the directory where the Waves Central program is and try again. That message should be gone then.

- To avoid black window: run with Wine-staging >= 6.3; this bug should be fixed in the most recent wine-(staging) versions 

- Newer Waves version like 12.0.5 and maybe some earlier versions fail with "Central has encountered a system
permissions issue". Run with Wine-staging >= 6.4; this bug should be fixed in upcoming Wine-Staging 6.4.



# Some additional remarks:
If you want to compile yourself instead of downloading binaries:
See main.c howto compile 

Then do (Assuming ~/.wine is where your wineprefix is)
  
```
cp -rf ./powershell64.exe ~/.wine/drive_c/windows/system32/WindowsPowerShell/v1.0/powershell.exe
  
cp -rf ./powershell32.exe ~/.wine/drive_c/windows/syswow64/WindowsPowerShell/v1.0/powershell.exe
```
  
- WINEARCH=win32 is _not_ supported





