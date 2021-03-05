# powershell-wrapper for wine

Wrapper for powershell.exe from wine, that gets Waves Central in wine a bit going.
Uses pwsh.exe from Powershell Core to get at least some functionality for powershell.exe from wine.

# Install (Assuming ~/.wine is where your wineprefix is):

wget https://github.com/PietJankbal/powershell-wrapper/raw/master/powershell64.exe && cp -rf ./powershell64.exe ~/.wine/drive_c/windows/system32/WindowsPowerShell/v1.0/powershell.exe

wget https://github.com/PietJankbal/powershell-wrapper/raw/master/powershell32.exe && cp -rf ./powershell32.exe ~/.wine/drive_c/windows/syswow64/WindowsPowerShell/v1.0/powershell.exe

That's really it...

# Notes:
- Powershell Core (and ConEmu) are downloaded and installed at first invokation of powershell (i.e. "wine powershell")
  (ConEmu is installed to work around bug https://bugs.winehq.org/show_bug.cgi?id=49780)

# Notes about Waves Central:
- Use most recent Wine-Staging (not vanilla-wine) to avoid several issues.

- After install the 1st run will still fail with message that powershell failed; cd into the directory where the program is and try again. That message should be gone then.

- To avoid black window: run with Wine-staging >6.3; this bug should be fixed in the most recent wine-(staging) versions 

- Newer Waves version like 12.0.5 and maybe some earlier versions fail with "Central has encountered a system
permissions issue". Run with Wine-staging >6.4; this bug should be fixed in upcoming Wine-Staging 6.4.



# Some additional remarks:
- If you want to compile yourself instead of downloading binaries:

  i686-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell32.exe

  x86_64-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell64.exe
  
- WINEARCH=win32 is _not_ supported





