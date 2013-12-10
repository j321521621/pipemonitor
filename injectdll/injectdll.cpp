// injectdll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../detours/include/detours.h"
#include <windows.h>

static int (WINAPI* OLD_MessageBoxW)(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)=MessageBoxW;  
int WINAPI NEW_MessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)  
{  

	//修改输入参数，调用原函数  
	int ret=OLD_MessageBoxW(hWnd,L"输入参数已修改",L"[测试]",uType);  
	return ret;  
}  

VOID Hook()  
{  
	//DetourRestoreAfterWith();  
	DetourTransactionBegin();  
	DetourUpdateThread(GetCurrentThread());  

	//这里可以连续多次调用DetourAttach，表明HOOK多个函数  
	DetourAttach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);  

	DetourTransactionCommit();  
}  

VOID UnHook()  
{  
	DetourTransactionBegin();  
	DetourUpdateThread(GetCurrentThread());  

	//这里可以连续多次调用DetourDetach,表明撤销多个函数HOOK  
	DetourDetach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);  

	DetourTransactionCommit();  

}  

unsigned __stdcall threadproc(void*)
{
	::MessageBoxW(NULL,L"inject",L"success",MB_OK);
	Hook();  
	MessageBoxW(0,L"正常消息框",L"测试",0);  
	UnHook();  
	return 0;
}