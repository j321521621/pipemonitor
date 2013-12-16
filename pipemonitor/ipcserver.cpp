#include "stdafx.h"
#include "ipcserver.h"
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <assert.h>
#include <string>
using namespace std;

ipc_server::ipc_server()
{
	for(int i=0;i<INSTANCES;i++)
	{
		memset((char*)&Pipe[i],0,sizeof(PIPEINST));
	}
}


BOOL ipc_server::dsiconnect_pipe(PIPEINST &Pipe)
{
	Pipe.handler->disconnect();
	Pipe.handler=NULL;
	return DisconnectNamedPipe(Pipe.hPipeInst);
}

BOOL ipc_server::connect_pipe(PIPEINST &Pipe,ipc_handler_factory *factory)
{
	if (ConnectNamedPipe(Pipe.hPipeInst,&(Pipe.oOverlap))) 
	{
		assert(0);
		return FALSE;
	}

	DWORD err=GetLastError();
	if(err==ERROR_IO_PENDING)
	{
		Pipe.fPendingIO = TRUE; 
		Pipe.dwState=CONNECTING_STATE;
		return TRUE; 
	}
	else if(err==ERROR_PIPE_CONNECTED)
	{
		Pipe.fPendingIO=FALSE;
		Pipe.dwState=READING_STATE;
		if (SetEvent(Pipe.oOverlap.hEvent))
		{
			Pipe.handler=factory->create();
			Pipe.handler->connect();
			return TRUE; 
		}
		else
		{
			assert(0);
			return FALSE;
		}
	}
	else
	{
		assert(0);
		return FALSE;
	}
}



DWORD ipc_server::serve(ipc_handler_factory* factory)
{

	for (int i = 0; i < INSTANCES; i++) 
	{

		hEvents[i] = CreateEvent(NULL,TRUE,TRUE,NULL); 

		if (hEvents[i] == NULL) 
		{
			assert(0);
			return -1;
		}

		Pipe[i].oOverlap.hEvent = hEvents[i]; 

		Pipe[i].hPipeInst = CreateNamedPipe( 
			lpszPipename,            // pipe name 
			PIPE_ACCESS_DUPLEX |     // read/write access 
			FILE_FLAG_OVERLAPPED,    // overlapped mode 
			PIPE_TYPE_MESSAGE |      // message-type pipe 
			PIPE_READMODE_MESSAGE |  // message-read mode 
			PIPE_WAIT,               // blocking mode 
			INSTANCES,               // number of instances 
			BUFSIZE,   // output buffer size 
			BUFSIZE,   // input buffer size 
			PIPE_TIMEOUT,            // client time-out 
			NULL);                   // default security attributes 

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) 
		{
			assert(0);
			return -2;
		}

		if(!connect_pipe(Pipe[i],factory))
		{
			assert(0);
			return -3;
		}
	}

	while (1)
	{
		DWORD i = WaitForMultipleObjects(INSTANCES,hEvents,FALSE,INFINITE) - WAIT_OBJECT_0;
		if (i < 0 || i > (INSTANCES - 1)) 
		{
			assert(0); 
			return -4;
		}

		if (Pipe[i].fPendingIO) 
		{ 
			DWORD cbRet;
			BOOL fSuccess = GetOverlappedResult(Pipe[i].hPipeInst,&Pipe[i].oOverlap,&cbRet,FALSE);

			if(Pipe[i].dwState==CONNECTING_STATE)
			{
				if (!fSuccess) 
				{
					assert(0); 
					return -5;
				}

				Pipe[i].dwState = READING_STATE;
				Pipe[i].handler=factory->create();
				Pipe[i].handler->connect();
			}
			else
			{
				DWORD err=GetLastError();
				if (fSuccess && Pipe[i].cbRead != 0) 
				{
					Pipe[i].handler->handle(Pipe[i].chRequest,Pipe[i].cbRead);
				}
				else if (!fSuccess && (err == ERROR_MORE_DATA)) 
				{
					Pipe[i].handler->exception();
					if(!SetEvent(Pipe[i].oOverlap.hEvent))
					{
						assert(0);
						return -7;
					}
					continue;
				}
				else if (!fSuccess && (err == ERROR_BROKEN_PIPE)) 
				{ 
					dsiconnect_pipe(Pipe[i]);
					connect_pipe(Pipe[i],factory); 
					continue; 
				}
				else
				{
					assert(0);
					return -8;
				}
			}  
		}

		BOOL fSuccess = ReadFile(Pipe[i].hPipeInst,Pipe[i].chRequest,BUFSIZE,&Pipe[i].cbRead,&Pipe[i].oOverlap);

		DWORD err=GetLastError();
		if (fSuccess && Pipe[i].cbRead != 0) 
		{
			Pipe[i].fPendingIO = FALSE; 
			Pipe[i].handler->handle(Pipe[i].chRequest,Pipe[i].cbRead);
		}
		else if (!fSuccess && (err == ERROR_IO_PENDING)) 
		{ 
			Pipe[i].fPendingIO = TRUE; 
		}
		else if (!fSuccess && (err == ERROR_MORE_DATA)) 
		{
			Pipe[i].handler->exception();
			if(!SetEvent(Pipe[i].oOverlap.hEvent))
			{
				assert(0);
				return -7;
			}
		}
		else if (! fSuccess && (err == ERROR_BROKEN_PIPE)) 
		{
			dsiconnect_pipe(Pipe[i]);
			connect_pipe(Pipe[i],factory);
		}
		else
		{
			assert(0);
			return -8;
		}
	} 

	return 0; 
} 

const LPTSTR ipc_server::lpszPipename = TEXT("\\\\.\\pipe\\1CA1185E-3F67-4AC4-B7A4-7CAADC9F994E"); 
