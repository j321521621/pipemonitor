// injectdll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../detours/include/detours.h"
#include "WriteFile.h"
#include "MessageBox.h"
#include "ipc.h"
#include <vector>
using std::vector;
#include <windows.h>
#include <tlhelp32.h>



vector<HANDLE> allthread()
{
	vector<HANDLE> ret;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if( hThreadSnap == INVALID_HANDLE_VALUE ) 
	{
		return ret; 
	}

	DWORD dwOwnerPID=GetCurrentProcessId();
	THREADENTRY32 te32; 
	te32.dwSize = sizeof(THREADENTRY32 ); 

	if( !Thread32First( hThreadSnap, &te32 ) ) 
	{
		CloseHandle( hThreadSnap );
		return ret;
	}

	do
	{ 
		if( te32.th32OwnerProcessID == dwOwnerPID && te32.th32ThreadID!=GetCurrentThreadId())
		{
			HANDLE handle=OpenThread(THREAD_ALL_ACCESS,FALSE,te32.th32ThreadID);
			if(handle)
			{
				ret.push_back(handle);
			}
		}
	} while(Thread32Next(hThreadSnap,&te32));

	CloseHandle( hThreadSnap );

	return ret;
}

DWORD Hook(PVOID *ppPointer,PVOID pDetour)
{
	DetourTransactionBegin();

	vector<HANDLE> thread=allthread();
	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		DetourUpdateThread(*it);
	}

	DetourAttach(ppPointer,pDetour);

	DWORD ret=DetourTransactionCommit();

	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		CloseHandle(*it);  
	}

	return ret;
}


unsigned __stdcall threadproc(void*)
{
	::MessageBox(NULL,L" success",L"注入成功",MB_OK);

	logger=new ipc();
	if(!logger->init())
	{
		::MessageBox(NULL,L"error",L"管道初始化失败",MB_OK);
		return -1;
	}

	Hook(&(PVOID&)OLD_WriteFile,NEW_WriteFile);
	return 0;
}