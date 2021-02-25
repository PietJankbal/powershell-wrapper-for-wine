/* powershell.exe - this program only calls pwsh.exe
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Commands that Waves Central calls, the last one fails but not fatal for Waves Central
 * powershell.exe Write-Host $PSVersionTable.PSVersion.Major $PSVersionTable.PSVersion.Minor
 * powershell.exe -command &{'{0}.{1}' -f $PSVersionTable.PSVersion.Major, $PSVersionTable.PSVersion.Minor}
 * powershell.exe (Get-PSDrive C).Free
 * powershell.exe -noLogo -noExit  -c Register-WMIEvent -Query 'SELECT * FROM Win32_DeviceChangeEvent WHERE (EventType = 2 OR EventType = 3) GROUP WITHIN 4' -Action { [System.Console]::WriteLine('Devices Changed') }
 *
 * Compile: *
 * i686-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell.exe
 * x86_64-w64-mingw32-gcc -municode  -mconsole main.c -lurlmon -luser32 -lntdll -s -o powershell.exe
 */
#define WIN32_LEAN_AND_MEAN
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include <windows.h>
#include <winuser.h>
#include <winternl.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <urlmon.h>
#include <wchar.h>

/* Following function taken from https://creativeandcritical.net/downloads/replacebench.c which is in public domain; Credits to the there mentioned authors*/
/* replaces in the string "str" all the occurrences of the string "sub" with the string "rep" */
wchar_t* replace_smart (const wchar_t *str, const wchar_t *sub, const wchar_t *rep)
{
    size_t slen = lstrlenW(sub);
    size_t rlen = lstrlenW(rep);
    size_t size = lstrlenW(str) + 1;
    size_t diff = rlen - slen;
    size_t capacity = (diff>0 && slen) ? 2 * size : size;
    wchar_t *buf = (wchar_t *)HeapAlloc(GetProcessHeap(),8,sizeof(wchar_t)*capacity);
    wchar_t *find, *b = buf;

    if (b == NULL) return NULL;
    if (slen == 0) return memcpy(b, str, sizeof(wchar_t)*size);

    while((find = /*strstrW*/(wchar_t *)wcsstr((const wchar_t *)str, (const wchar_t *)sub))) {
        if ((size += diff) > capacity) {
            wchar_t *ptr = (wchar_t *)HeapReAlloc(GetProcessHeap(), 0, buf, capacity = 2 * size*sizeof(wchar_t));
            if (ptr == NULL) {HeapFree(GetProcessHeap(), 0, buf); return NULL;}
            b = ptr + (b - buf);
            buf = ptr;
        }
        memcpy(b, str, (find - str) * sizeof(wchar_t)); /* copy up to occurrence */
        b += find - str;
        memcpy(b, rep, rlen * sizeof(wchar_t));       /* add replacement */
        b += rlen;
        str = find + slen;
    }
    memcpy(b, str, (size - (b - buf)) * sizeof(wchar_t));
    b = (wchar_t *)HeapReAlloc(GetProcessHeap(), 0, buf, size * sizeof(wchar_t));         /* trim to size */
    return b ? b : buf;
}

int __cdecl wmain(int argc, WCHAR *argv[])
{
    int i, cmd_idx = 0;
    WCHAR pwsh_pathW[MAX_PATH], *bufW = NULL, cmdlineW [MAX_PATH]=L"", cmdW[MAX_PATH] = L"-c "; const WCHAR *new_args[3];//, pwsh_exeW[] = L"pwsh.exe";

    if(!ExpandEnvironmentStringsW(L"%ProgramW6432%", pwsh_pathW, MAX_PATH+1)) goto failed; /* win32 only apparently, not supported... */
    lstrcatW(pwsh_pathW, L"\\Powershell\\7\\pwsh.exe");

    if ( (GetFileAttributesW(pwsh_pathW) != INVALID_FILE_ATTRIBUTES) )
        goto already_installed;
    /* Download and install*/
    char url[] = "https://github.com/PowerShell/PowerShell/releases/download/v7.0.3/PowerShell-7.0.3-win-x64.msi";
    char destination[] = "PowerShell-7.0.3-win-x64.msi";
    /* FIXME Install ConEmu to work around bug https://bugs.winehq.org/show_bug.cgi?id=49780 */ 
    char urlcon[] = "https://conemu.github.io/install2.ps1", destinationcon[] = "install2.ps1";
    NTSTATUS WINAPI RtlGetVersion(RTL_OSVERSIONINFOEXW*); RTL_OSVERSIONINFOEXW info;
    WCHAR tempW[MAX_PATH], cur_dirW[MAX_PATH];

    info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    if (RtlGetVersion(&info)) goto failed;
    if (info.dwMajorVersion < 6 || (info.dwMajorVersion == 6 && info.dwMinorVersion < 1) ) goto failed;

    MessageBoxA(NULL, "Looks like Powershell Core is not installed \nWill start downloading and install now\n \
    This will take quite some time!!!\nNo progress bar is shown!", "Message", MB_ICONWARNING | MB_OK);

    GetCurrentDirectoryW(MAX_PATH+1, cur_dirW); GetTempPath(MAX_PATH, tempW); SetCurrentDirectoryW(tempW);
    fwprintf(stderr, L"\033[1;34mDownloading File From: %s, To: %s \n\033[0m\n", url, destination);

    if( URLDownloadToFileA(NULL, url, destination, 0, NULL) != S_OK )
        goto failed;
    else
        fwprintf(stderr, L"\033[1;34mFile Successfully Downloaded \n\033[0m\n");

    system("start /WAIT msiexec.exe /i PowerShell-7.0.3-win-x64.msi /*INSTALLFOLDER=\"C:\\Windows\\Powershell6\\\"*/ /q");

    if( URLDownloadToFileA(NULL, urlcon, destinationcon, 0, NULL) != S_OK )
        goto failed;
    else
        fwprintf(stderr, L"\033[1;34mFile Successfully Downloaded \n\033[0m\n");

    STARTUPINFOW startup_info; PROCESS_INFORMATION process_info;
    memset(&startup_info, 0, sizeof(STARTUPINFO)); memset(&process_info, 0, sizeof(PROCESS_INFORMATION));
    startup_info.cb = sizeof(STARTUPINFO);

    CreateProcessW(pwsh_pathW,L" -file install2.ps1",0,0,0,0,0,0,&startup_info,&process_info); SetCurrentDirectoryW(cur_dirW);

    fwprintf(stderr, L"\033[1;34mFIXME Waiting for 5 secs to finish things, otherwise it just fails on first run...  \n\033[0m\n");
    Sleep(5000);

already_installed:

    for (i = 1; i < argc; i++) /* concatenate all args into one single commandline */
    {
        if (!_wcsnicmp(L"-ve", argv[i],3) || !_wcsnicmp(L"-noe", argv[i],4)) i +=2;    /* -Version or -NoExit, just skip for now*/
        if(!argv[i]) break;
        lstrcatW(cmdlineW, L" "); lstrcatW(cmdlineW, argv[i]); 
    }

    i = 1; fwprintf(stderr, L"\033[1;35m"); fwprintf(stderr, L"\nold command line is %ls \n", cmdlineW); fwprintf(stderr, L"\033[0m\n");
     /* pwsh requires a command option "-c" , powershell doesn`t, so we have to insert it e.g. "powershell -NoLogo echo $env:username" should go
    into "pwsh -NoLogo -c echo $env:username".... */
    while(i<argc) /* Step through all options until we reach command */
    {
        if ( !_wcsnicmp(L"-c", argv[i],2) )    /* -Command or -c */
        {
            cmd_idx = i;
            break;
        }     
        /* try handle something like powershell -nologo -windowstyle normal -outputformat text -version 1.0 echo \$env:username */
        if ( !_wcsnicmp(L"-f", argv[i],2)  ||  /* -File */              !_wcsnicmp(L"-ps", argv[i],3) ||  /* -PSConsoleFile */ \
             !_wcsnicmp(L"-ve", argv[i],3) ||  /* -Version */           !_wcsnicmp(L"-in", argv[i],3) ||  /* -InputFormat */ \
             !_wcsnicmp(L"-ou", argv[i],3) ||  /* -OutputFormat */      !_wcsnicmp(L"-wi", argv[i],3) ||  /* -WindowStyle */ \
             !_wcsnicmp(L"-en", argv[i],3) ||  /* -EncodedCommand */    !_wcsnicmp(L"-ex", argv[i],3)     /* -ExecutionPolicy */ )
        {
             i++;
             goto done;
        }

        if ( !_wcsnicmp(L"-", argv[i],1) ) goto done;      /* -nologo or -noexit etc.*/
        /* We should now have arrived at (1st) command */
        lstrcatW(cmdW,argv[i]); cmd_idx = i; break;

        done: i++;
    }
    /* Replace incompitable commands here (e.g. Get-WmiObject --> Get-CimInstance which btw doesn't work anyway atm, wine-bug??)             */
    /* Or just feed your app what it it wants, e.g. if something like [System.Math]::sqrt(64) wouldn`t work (it does work btw),              */
    /* feed it the desired output like below ....                                                                                            */
    /* put replacements here....        from                                         to                                                      */
    const WCHAR from_to[][MAX_PATH] = { L"[System.Math]::sqrt(64)",                  L"Write-Host 8",  /* just an example, not necassary.... */
                                        L"Get-WmiObject",                            L"Get-CimInstance",
                                        L"Register-WMIEvent",                        L"Register-CimIndicationEvent"
                                      };

    if(cmd_idx)
    {
        bufW = replace_smart(cmdlineW, argv[cmd_idx], cmdW);
        lstrcpyW(cmdlineW, bufW); HeapFree(GetProcessHeap(), 0, bufW);
    }

    for(i=0; i < ARRAY_SIZE(from_to);i+=2)
    {
        bufW = replace_smart(cmdlineW, from_to[i], from_to[i+1]);
        lstrcpyW( cmdlineW, bufW ); HeapFree(GetProcessHeap(), 0, bufW);
    }

    fwprintf(stderr, L"\033[1;93mnew command line is %ls \n\033[0m\n", cmdlineW);

    if(!cmd_idx)  
    {
        WCHAR start_conemuW[MAX_PATH], conemu_optionsW[MAX_PATH] = L" -Title \"This is Powershell Core (pwsh.exe), not (!) powershell.exe\" -resetdefault -run ";
        if(!ExpandEnvironmentStringsW(L"%SystemDrive%", start_conemuW, MAX_PATH+1)) goto failed; 
        lstrcatW(start_conemuW, L"\\ConEmu\\ConEmu.exe");
        new_args[0] = start_conemuW; new_args[1] = lstrcatW( lstrcatW(conemu_optionsW, pwsh_pathW), cmdlineW); new_args[2] = NULL;
        _wspawnv(1 /* _P_NOWAIT */, start_conemuW, new_args);
    }
    else
    {
        new_args[0] = pwsh_pathW; new_args[1] = cmdlineW; new_args[2] = NULL;
        _wspawnv(2 /*_P_OVERLAY*/, pwsh_pathW, new_args);
    }
    return 0;

failed:
    fprintf(stderr, "Something went wrong :( (32-bit?, winversion < win7?, failing download?....  \n");
    return 0; /* fake success anyway */
}
