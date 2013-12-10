// pipemonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <assert.h>
using namespace::std;

typedef enum 
{
	S_INJECT,
	E_INJECT_OPENPROCESS,
	E_INJECT_VIRTUALALLOCEX,
	E_INJECT_WRITEPROCESSMEMORY,
	E_INJECT_GETMODULEHANDLE,
	E_INJECT_CREATEREMOTETHREAD,
} inject_ret;

DWORD getpid(wstring processname)
{
	DWORD ret=NULL;
	HANDLE hProcs=0;
	if ( hProcs = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) )
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof( PROCESSENTRY32 );
		BOOL ok = Process32First(hProcs, &pe32);
		while (ok)
		{
			if(processname==pe32.szExeFile)
			{
				ret=pe32.th32ProcessID;
				break;
			}
			ok = Process32Next(hProcs, &pe32);
		}
		CloseHandle(hProcs);
	}
	return ret;
};

inject_ret inject(DWORD pid,LPWSTR path)
{
	HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
	if(hProcess==NULL)
	{
		return E_INJECT_OPENPROCESS;
	}

	VOID *pLibRemote = ::VirtualAllocEx(hProcess,NULL,2*wcslen(path)+2,MEM_COMMIT,PAGE_READWRITE);
	if(pLibRemote==NULL)
	{
		return E_INJECT_VIRTUALALLOCEX;
	}

	if(::WriteProcessMemory( hProcess,pLibRemote,(void*)path,2*wcslen(path)+2,NULL)==0)
	{
		return E_INJECT_WRITEPROCESSMEMORY;
	}
	
	HMODULE hKernel32 = ::GetModuleHandle(L"Kernel32");
	if(hKernel32==NULL)
	{
		return E_INJECT_GETMODULEHANDLE;
	}

	HANDLE hThread = ::CreateRemoteThread( hProcess,NULL,0,(LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"LoadLibraryW"),pLibRemote,0,NULL);
	if(hThread==NULL)
	{
		return E_INJECT_CREATEREMOTETHREAD;
	}

	return S_INJECT;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(inject(getpid(L"demon.exe"),L"E:\\code\\sw\\pipemonitor\\Debug\\injectdll.dll")!=S_INJECT)
	{
		assert(0);
	}
}