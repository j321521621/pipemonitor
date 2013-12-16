// injectdll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../detours/include/detours.h"
#include "WriteFile.h"
#include "MessageBox.h"
#include <vector>
using std::vector;
#include <windows.h>
#include <tlhelp32.h>


vector<HANDLE> allthread()
{
	vector<HANDLE> ret;

	THREADENTRY32 th32;
	th32.dwSize = sizeof(th32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot调用失败");
		return ret;
	}
	for(BOOL flag = ::Thread32First(hProcessSnap,&th32);flag;flag=::Thread32Next(hProcessSnap,&th32))
	{
		ret.push_back(OpenThread(THREAD_ALL_ACCESS,FALSE,th32.th32ThreadID));
	}
	return ret;
}

VOID Hook()  
{  
	DetourTransactionBegin();  

	vector<HANDLE> thread=allthread();
	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		DetourUpdateThread(*it);  
	}

	DetourAttach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);

	DetourTransactionCommit();

	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		CloseHandle(*it);  
	}
}  

VOID UnHook()  
{  
	DetourTransactionBegin();  
	vector<HANDLE> thread=allthread();
	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		DetourUpdateThread(*it);  
	}

	DetourDetach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);  

	DetourTransactionCommit();

	for(vector<HANDLE>::iterator it=thread.begin();it!=thread.end();++it)
	{
		CloseHandle(*it);  
	}

}  
unsigned __stdcall threadproc(void*)
{
	::MessageBoxW(NULL,L"inject",L"success",MB_OK);
	Hook();  
	MessageBoxW(0,L"正常消息框",L"测试",0);  
	UnHook();  
	return 0;
}