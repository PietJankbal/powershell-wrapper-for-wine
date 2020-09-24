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

- If you want to compile yourself instead of downloading binaries:

  i686-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell32.exe

  x86_64-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell64.exe
  
- WINEARCH=win32 is _not_ supported





