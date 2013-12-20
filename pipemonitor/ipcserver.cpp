#include "stdafx.h"
#include "ipcserver.h"
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <assert.h>
#include <string>
using namespace std;

ipc_server::ipc_server(ipc_handler_factory *factory):m_factory(factory)
{
	for(int i=0;i<INSTANCES;i++)
	{
		memset((char*)&m_pipe[i],0,sizeof(PIPE));
	}
}


BOOL ipc_server::dsiconnect_pipe(PIPE &Pipe,ipc_handler::ipc_handler_mode mode)
{
	Pipe.handler->disconnect(mode);
	Pipe.handler=NULL;
	return DisconnectNamedPipe(Pipe.handle);
}

BOOL ipc_server::connect_pipe(PIPE &Pipe)
{
	if (ConnectNamedPipe(Pipe.handle,&(Pipe.overlap))) 
	{
		assert(0);
		return FALSE;
	}

	DWORD err=GetLastError();
	if(err==ERROR_IO_PENDING)
	{
		Pipe.state=CONNECTING_PENDING;
		return TRUE; 
	}
	else if(err==ERROR_PIPE_CONNECTED)
	{
		Pipe.state=CONNECTED;
		SetEvent(Pipe.overlap.hEvent);
		return TRUE;
	}
	else
	{
		assert(0);
		return FALSE;
	}
}



DWORD ipc_server::serve()
{

	for (int i = 0; i < INSTANCES; i++) 
	{
		hEvents[i] = CreateEvent(NULL,TRUE,TRUE,NULL); 

		if (hEvents[i] == NULL) 
		{
			assert(0);
			return -1;
		}

		m_pipe[i].overlap.hEvent = hEvents[i]; 

		m_pipe[i].handle = CreateNamedPipe( 
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

		if (m_pipe[i].handle == INVALID_HANDLE_VALUE) 
		{
			assert(0);
			return -2;
		}
	}

	for (int i = 0; i < INSTANCES; i++) 
	{
		connect_pipe(m_pipe[i]);
	}

	while (true)
	{
		DWORD i = WaitForMultipleObjects(INSTANCES,hEvents,FALSE,INFINITE) - WAIT_OBJECT_0;
		if (i < 0 || i > (INSTANCES - 1)) 
		{
			assert(0); 
			return -4;
		}

		if(m_pipe[i].state==CONNECTED)
		{
			m_pipe[i].handler=m_factory->create();
			m_pipe[i].handler->connect(ipc_handler::sync_mode);
		}
		else if(m_pipe[i].state==CONNECTING_PENDING)
		{
			DWORD read;
			BOOL fSuccess = GetOverlappedResult(m_pipe[i].handle,&m_pipe[i].overlap,&read,FALSE);

			if (!fSuccess) 
			{
				connect_pipe(m_pipe[i]);
				continue;
			}
			else
			{
				m_pipe[i].handler=m_factory->create();
				m_pipe[i].handler->connect(ipc_handler::async_mode);
			}
		}
		else if(m_pipe[i].state==READING_PENDING)
		{
			DWORD read;
			BOOL fSuccess = GetOverlappedResult(m_pipe[i].handle,&m_pipe[i].overlap,&read,FALSE);

			if (fSuccess) 
			{
				ipc_handler::ipc_handler_mode mode=ipc_handler::async_mode;
				if(!m_pipe[i].truncate.empty())
				{
					mode=ipc_handler::truncate_mode;
				}
				m_pipe[i].truncate=m_pipe[i].truncate+string(m_pipe[i].buffer,read);
				m_pipe[i].handler->handle(mode,m_pipe[i].truncate);
				m_pipe[i].truncate.resize(0);
			}
			else
			{
				DWORD err=GetLastError();

				if (err == ERROR_MORE_DATA)
				{
					m_pipe[i].truncate=m_pipe[i].truncate+string(m_pipe[i].buffer,read);
				}
				else if (err == ERROR_BROKEN_PIPE)
				{
					dsiconnect_pipe(m_pipe[i],ipc_handler::async_mode);
					connect_pipe(m_pipe[i]); 
					continue;
				}
				else
				{
					m_pipe[i].handler->exception(ipc_handler::async_mode,ipc_handler::unknown);
					dsiconnect_pipe(m_pipe[i],ipc_handler::async_mode);
					connect_pipe(m_pipe[i]); 
					continue;
				}
			}
		}

		BOOL fSuccess = ReadFile(m_pipe[i].handle,m_pipe[i].buffer,BUFSIZE,NULL,&m_pipe[i].overlap);

		if (fSuccess)
		{
			m_pipe[i].state = READING_PENDING;
		}
		else
		{
			DWORD err=GetLastError();
			if (err == ERROR_MORE_DATA)
			{
				m_pipe[i].state = READING_PENDING;
			}
			else if (err == ERROR_IO_PENDING)
			{
				m_pipe[i].state = READING_PENDING;
			}
			else if (err == ERROR_BROKEN_PIPE)
			{

				dsiconnect_pipe(m_pipe[i],ipc_handler::sync_mode);
				connect_pipe(m_pipe[i]);
			}
			else
			{
				m_pipe[i].handler->exception(ipc_handler::sync_mode,ipc_handler::unknown);
				dsiconnect_pipe(m_pipe[i],ipc_handler::sync_mode);
				connect_pipe(m_pipe[i]);
			}
		}
	}

	return 0; 
} 

const LPTSTR ipc_server::lpszPipename = TEXT("\\\\.\\pipe\\1CA1185E-3F67-4AC4-B7A4-7CAADC9F994E"); 
