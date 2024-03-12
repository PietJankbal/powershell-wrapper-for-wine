/* Installs PowerShell Core, wraps powershell`s commandline into correct syntax for pwsh.exe.
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
 * Compile: // For fun I changed code from standard main(argc,*argv[]) to something like https://nullprogram.com/blog/2016/01/31/)
 * x86_64-w64-mingw32-gcc -O1 -fno-ident -fno-stack-protector -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -falign-functions=1 -falign-jumps=1 -falign-loops=1 -fwhole-program\
 -mconsole -municode -mno-stack-arg-probe -Xlinker --stack=0x200000,0x200000 -nostdlib  -Wall -Wextra -ffreestanding  main.c -lurlmon -lkernel32 -lucrtbase -nostdlib -lshell32 -lshlwapi -s -o powershell64.exe
 * i686-w64-mingw32-gcc -O1 -fno-ident -fno-stack-protector -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -falign-functions=1 -falign-jumps=1 -falign-loops=1 -fwhole-program\
 -mconsole -municode -mno-stack-arg-probe -Xlinker --stack=0x200000,0x200000 -nostdlib  -Wall -Wextra -ffreestanding main.c -lurlmon -lkernel32 -lucrtbase -nostdlib -lshell32 -lshlwapi -s -o powershell32.exe
 */
 
#include <windows.h>
#include <winternl.h> 
#include <stdio.h>
#include "shlwapi.h"

static inline BOOL is_single_or_last_option (WCHAR *opt)
{
    return ( ( ( !_wcsnicmp( opt, L"-c", 2 ) && _wcsnicmp( opt, L"-config", 7 ) ) || !_wcsnicmp( opt, L"-n", 2 ) || !_wcsnicmp( opt, L"-f", 2 ) ||\
                 !wcscmp( opt, L"-" ) || !_wcsnicmp( opt, L"-enc", 4 ) || !_wcsnicmp( opt, L"-m", 2 ) || !_wcsnicmp( opt, L"-s", 2 ) ) ? TRUE : FALSE );
}
/* Following function taken from https://creativeandcritical.net/downloads/replacebench.c which is in public domain; Credits to the there mentioned authors*/
/* replaces in the string "str" all the occurrences of the string "sub" with the string "rep" */
static inline wchar_t* replace_smart (wchar_t *str, wchar_t *sub, wchar_t *rep)
{
    size_t slen = wcslen(sub); size_t rlen = wcslen(rep);
    size_t size = wcslen(str) + 1;
    size_t diff = rlen - slen;
    size_t capacity = (diff>0 && slen) ? 2 * size : size;
    wchar_t *buf = (wchar_t *)HeapAlloc(GetProcessHeap(),8,sizeof(wchar_t)*capacity);
    wchar_t *find, *b = buf;

    if (b == NULL) return NULL;
    if (slen == 0) return memcpy(b, str, sizeof(wchar_t)*size);

    while ( ( find = StrStrIW( str , sub ) ) ) { /* do case insensitive search */
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

__attribute__((externally_visible))  /* for -fwhole-program */
int mainCRTStartup(void)
{
    BOOL read_from_stdin = FALSE, ps_console = FALSE;
    wchar_t cmdlineW[4096]=L"", pwsh_pathW[MAX_PATH] =L"", bufW[MAX_PATH] = L"", **argv;;
    DWORD exitcode;       
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    int i, j, argc;
    
    argv = CommandLineToArgvW ( GetCommandLineW(), &argc);
    ExpandEnvironmentStringsW(L"%ProgramW6432%\\Powershell\\7\\pwsh.exe", pwsh_pathW, MAX_PATH+1);

    /* Download and Install */
    if ( GetFileAttributesW( pwsh_pathW ) == INVALID_FILE_ATTRIBUTES ) /* Download and install*/
    {    
        ExpandEnvironmentStringsW( L"%TMP%\\PowerShell-7.4.1-win-x64.msi", bufW, MAX_PATH + 1 );
        fputws(L"\033[1;33mDownloading Powershell Core",stderr);fputws(cmdlineW,stderr);fputws(L"\033[0m\n",stderr);
        if( URLDownloadToFileW( NULL, L"https://github.com/PowerShell/PowerShell/releases/download/v7.4.1/PowerShell-7.4.1-win-x64.msi", bufW, 0, NULL ) != S_OK )
            { fputs("download failed",stderr ); exit(1); }
   
        memset( &si, 0, sizeof( STARTUPINFO )); si.cb = sizeof( STARTUPINFO ); memset( &pi, 0, sizeof( PROCESS_INFORMATION ));
        ExpandEnvironmentStringsW( L"%winsysdir%\\msiexec.exe /i %TMP%\\PowerShell-7.4.1-win-x64.msi ENABLE_PSREMOTING=1 REGISTER_MANIFEST=1  /q", bufW, MAX_PATH + 1 );
        CreateProcessW( NULL, bufW , 0, 0, 0, 0, 0, 0, &si, &pi);
        WaitForSingleObject( pi.hProcess, INFINITE ); CloseHandle( pi.hProcess ); CloseHandle( pi.hThread );   
            
        ExpandEnvironmentStringsW( L"%TMP%\\ConEmuPack.230724.7z", bufW, MAX_PATH + 1 );
        fputws(L"\033[1;33mDownloading ConEmu",stderr);fputws(cmdlineW,stderr);fputws(L"\033[0m\n",stderr);
        if( URLDownloadToFileW( NULL, L"https://github.com/Maximus5/ConEmu/releases/download/v23.07.24/ConEmuPack.230724.7z", bufW, 0, NULL ) != S_OK )
            { fputs("download failed",stderr ); exit(1); }         

        ExpandEnvironmentStringsW( L"%TMP%\\7zr.exe", bufW, MAX_PATH + 1 );
        fputws(L"\033[1;33mDownloading 7zr",stderr);fputws(cmdlineW,stderr);fputws(L"\033[0m\n",stderr);
        if( URLDownloadToFileW( NULL, L"https://www.7-zip.org/a/7zr.exe", bufW, 0, NULL ) != S_OK ) 
            { fputs("download failed",stderr ); exit(1); }  

        memset( &si, 0, sizeof( STARTUPINFO ) ); si.cb = sizeof( STARTUPINFO ); memset( &pi , 0, sizeof( PROCESS_INFORMATION ) );
        ExpandEnvironmentStringsW( L"%TMP%\\7zr.exe x %TMP%\\ConEmuPack.230724.7z -o\"%SystemDrive%\\ConEmu\"", bufW, MAX_PATH + 1 );
        CreateProcessW( NULL, bufW, 0, 0, 0, 0, 0, 0, &si, &pi);
        WaitForSingleObject( pi.hProcess, INFINITE ); CloseHandle( pi.hProcess ); CloseHandle( pi.hThread ); 
      
        fputws(L"\033[1;33mDownloading profile.ps1",stderr);fputws(cmdlineW,stderr);fputws(L"\033[0m\n",stderr);
        ExpandEnvironmentStringsW( L"%ProgramW6432%\\Powershell\\7\\profile.ps1", bufW, MAX_PATH + 1 );
        if( URLDownloadToFileW(NULL, L"https://raw.githubusercontent.com/PietJankbal/powershell-wrapper-for-wine/master/profile.ps1", bufW, 0, NULL) != S_OK )
            { fputs("download failed",stderr ); exit(1); }
    }
    /* Main program: wrap the original powershell-commandline into correct syntax, and send it to pwsh.exe */ 
    /* pwsh requires a command option "-c" , powershell doesn`t, so we have to insert it somewhere e.g. 'powershell -nologo 2+1' should go into 'powershell -nologo -c 2+1'*/ 
    for (i = 1;  argv[i] &&  !wcsncmp(  argv[i], L"-" , 1 ); i++ ) { if ( !is_single_or_last_option ( argv[i] ) ) i++; if(!argv[i]) break;} /* Search for 1st argument after options */
    for (j = 1; j < i ; j++ ) /* concatenate options into new cmdline, meanwhile working around some incompabilities */ 
    { 
        if ( !wcscmp( L"-", argv[j] ) ) { if(j == (argc-1)) {read_from_stdin = TRUE; continue;} else {fputs("\033[1;35mInvalid usage\033[0m",stderr);exit(1);}}   /* hyphen handled later on */
        if ( !_wcsnicmp(  argv[j], L"-ve", 3 ) ) {j++;  continue;}            /* -Version, exclude from new cmdline, incompatible... */
        if ( !_wcsnicmp( argv[j], L"-nop", 4 ) ) continue;                    /* -NoProfile, also exclude to always enable profile.ps1 to work around possible incompatibilities */   
        wcscat( wcscat( cmdlineW, L" " ), argv[j] );
    }
    /* now insert a '-c' (if necessary) */
    if ( argv[i] && _wcsnicmp( argv[i-1], L"-c", 2 ) && _wcsnicmp( argv[i-1], L"-enc", 4 ) && _wcsnicmp( argv[i-1], L"-f", 2 ) && _wcsnicmp( argv[i], L"/c", 2 ) )
        wcscat( cmdlineW, L" -c " );
    /* concatenate the rest of the arguments into the new cmdline */
    for( j = i; j < argc; j++ ) wcscat( wcscat( cmdlineW, L" " ), argv[j] );
    /* support pipeline to handle something like " '$(get-date) | powershell - ' */
    if( read_from_stdin ) {
        WCHAR defline[8192]; wint_t wc; char line[8192] = " \"&  {" ; /* embed cmd in scriptblock */ int m = strlen(line) - 1;
        HANDLE input = GetStdHandle(STD_INPUT_HANDLE); DWORD type = GetFileType(input);
        /* handle pipe */
        if ( type != FILE_TYPE_CHAR ) { /* not redirected (FILE_TYPE_PIPE or FILE_TYPE_DISK) */
            if( !wcscmp(argv[argc-1], L"-" ) && _wcsnicmp(argv[argc-2], L"-c", 2 ) ) wcscat(cmdlineW, L" -c ");
            while( ( wc = fgetc(stdin) ) != WEOF ) line[++m] = wc;
            line[++m] = '}';line[++m] = '"';line[++m] = '\0'; 
            mbstowcs(defline, line, 8192);
            wcscat(cmdlineW, defline);
        }
    } /* end support pipeline */
    if ( ( i == argc ) && !read_from_stdin ) ps_console = TRUE;
    /* replace incompatible commands/strings in the cmdline fed to pwsh.exe; see profile.ps1 howto replace */
    if ( GetEnvironmentVariableW( L"PSHACKS", bufW, MAX_PATH + 1 ) ) {

        WCHAR buf_fromW[MAX_PATH];WCHAR buf_toW[MAX_PATH]; WCHAR *buf_replacedW=NULL;

        if (GetEnvironmentVariableW( L"PS_FROM", buf_fromW, MAX_PATH + 1 ) && GetEnvironmentVariableW( L"PS_TO", buf_toW, MAX_PATH + 1 )) {
            wchar_t *bufferA, *bufferB = 0;

            wchar_t* tokenA = wcstok_s(buf_fromW, L"¶", &bufferA); /* Use ¶ as separator, it will likely never show up in a command */
            wchar_t* tokenB = wcstok_s(buf_toW, L"¶", &bufferB);

            while (tokenA && tokenB) {
                buf_replacedW = replace_smart(cmdlineW, tokenA, tokenB);
                wcscpy( cmdlineW, buf_replacedW ); HeapFree(GetProcessHeap(), 0, buf_replacedW);

                tokenA = wcstok_s(NULL, L"¶", &bufferA);
                tokenB = wcstok_s(NULL, L"¶", &bufferB);
            }
        }
    }
    /* fputws(L"\033[1;33mcmdline:",stderr);fputws(cmdlineW,stderr);fputws(L"\033[0m\n",stderr); */
    /* Execute the command through pwsh.exe (or start PSconsole via ConEmu if no command found) */
    if ( ps_console ) ExpandEnvironmentStringsW( L" -c %SystemDrive%\\ConEmu\\ConEmu64.exe -NoUpdate -LoadRegistry -run %ProgramW6432%\\Powershell\\7\\pwsh.exe ", bufW, MAX_PATH+1);
    CreateProcessW( pwsh_pathW, ps_console ? wcscat( bufW , cmdlineW ) : cmdlineW, 0, 0, 0, 0, 0, 0, &si, &pi );
    WaitForSingleObject( pi.hProcess, INFINITE ); GetExitCodeProcess( pi.hProcess, &exitcode ); CloseHandle( pi.hProcess ); CloseHandle( pi.hThread );    
    LocalFree(argv);
    
    exit ( exitcode ); 
}
