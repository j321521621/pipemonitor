#pragma once

#include "ipc_handler.h";
#include <Windows.h>

class ipc_server
{
public:
	ipc_server();
	DWORD serve(ipc_handler_factory* factory);

private:
	const static int BUFSIZE=4096;
	const static int INSTANCES=4;
	const static int PIPE_TIMEOUT=5000;
	const static LPTSTR lpszPipename;

	typedef struct 
	{ 
		OVERLAPPED oOverlap; 
		HANDLE hPipeInst; 
		CHAR chRequest[BUFSIZE]; 
		DWORD cbRead;
		DWORD dwState; 
		BOOL fPendingIO;
		ipc_handler *handler;
	} PIPEINST, *LPPIPEINST;

	enum PIPESTAT
	{
		CONNECTING_STATE,
		READING_STATE
	};

	PIPEINST Pipe[INSTANCES]; 
	HANDLE hEvents[INSTANCES];

	BOOL dsiconnect_pipe(PIPEINST &Pipe);
	BOOL connect_pipe(PIPEINST &Pipe,ipc_handler_factory *factory);
};
