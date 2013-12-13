// injectdll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <windows.h>
#include "../detours/include/detours.h"
#include "WriteFile.h"
#include "MessageBox.h"

VOID Hook()  
{  
	DetourTransactionBegin();
	DetourAttach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);  
	DetourAttach(&(PVOID&)OLD_WriteFile,NEW_WriteFile);  
	DetourTransactionCommit();
}

VOID UnHook()  
{  
	DetourTransactionBegin();  
	DetourDetach(&(PVOID&)OLD_MessageBoxW,NEW_MessageBoxW);  
	DetourDetach(&(PVOID&)OLD_WriteFile,NEW_WriteFile);  
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