#pragma once

#include "ipc_handler.h";
#include <Windows.h>
#include <string>
using namespace std;

class ipc_server
{
public:
	ipc_server(ipc_handler_factory *factory);
	DWORD serve();

private:
	const static int BUFSIZE=4096;
	const static int INSTANCES=4;
	const static int PIPE_TIMEOUT=5000;
	const static LPTSTR lpszPipename;

	typedef struct 
	{ 
		HANDLE handle; 
		OVERLAPPED overlap; 
		CHAR buffer[BUFSIZE];
		DWORD state; 
		ipc_handler *handler;
		string truncate;
	} PIPE;

	enum PIPESTAT
	{
		CONNECTING_PENDING,
		CONNECTED,
		READING_PENDING
	};

	PIPE m_pipe[INSTANCES]; 
	HANDLE hEvents[INSTANCES];
	ipc_handler_factory *m_factory;

	BOOL dsiconnect_pipe(PIPE &Pipe,ipc_handler::ipc_handler_mode mode);
	BOOL connect_pipe(PIPE &Pipe);
};
