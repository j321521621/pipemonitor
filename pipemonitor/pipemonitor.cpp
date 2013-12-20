// pipemonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ipcserver.h"
#include "printer.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <assert.h>
using namespace::std;




int _tmain(int argc, _TCHAR* argv[])
{
	if(!CreateMutex(NULL,TRUE,L"E24EE079-73DC-4617-8658-5FAB54BD1F13") || GetLastError()==ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	HANDLE pipe = CreateNamedPipe( 
		L"\\\\.\\pipe\\7E90B034-BDDF-4CD5-9638-371EB9918763",
		PIPE_ACCESS_OUTBOUND,    // overlapped mode 
		PIPE_TYPE_MESSAGE |      // message-type pipe 
		PIPE_READMODE_MESSAGE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		1,               // number of instances 
		4096*sizeof(TCHAR),   // output buffer size 
		4096*sizeof(TCHAR),   // input buffer size 
		0,            // client time-out 
		NULL);                   // default security attributes 

	if (pipe == INVALID_HANDLE_VALUE) 
	{
		DWORD err=GetLastError();
		assert(0);
		return -1;
	}

	printer_factory prfct;

	ipc_server pps(&prfct);
	pps.serve();
}