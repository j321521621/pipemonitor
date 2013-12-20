#include "stdafx.h"
#include "ipc.h"
#include "WriteFile.h"
#include <windows.h>

FUN_WriteFile *OLD_WriteFile=WriteFile;

BOOL
	WINAPI
	NEW_WriteFile(
	__in        HANDLE hFile,
	__in_bcount_opt(nNumberOfBytesToWrite) LPCVOID lpBuffer,
	__in        DWORD nNumberOfBytesToWrite,
	__out_opt   LPDWORD lpNumberOfBytesWritten,
	__inout_opt LPOVERLAPPED lpOverlapped
	)
{
	TCHAR buf[MAX_PATH+1];
	DWORD sz=GetFinalPathNameByHandle(hFile,buf,MAX_PATH,0);
	buf[sz]=0;

	wstringstream wss;
	wss<<L"WriteFile \""<<buf<<"\" with ["<<nNumberOfBytesToWrite<<"] bytes {} "<<(lpOverlapped?L"A":L"S");

	logger->send(wss.str());
	return OLD_WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped);
}