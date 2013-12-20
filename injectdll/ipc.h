#pragma once
#include "WriteFile.h"
#include <string>
#include <sstream>
using namespace std;
#include <Windows.h>

typedef
	BOOL
	WINAPI
	FUN_WriteFile(
	__in        HANDLE hFile,
	__in_bcount_opt(nNumberOfBytesToWrite) LPCVOID lpBuffer,
	__in        DWORD nNumberOfBytesToWrite,
	__out_opt   LPDWORD lpNumberOfBytesWritten,
	__inout_opt LPOVERLAPPED lpOverlapped
	);

class ipc
{
public:
	ipc():m_pipe(NULL),m_inited(false)
	{
	}


	bool init()
	{
		m_pipe=CreateFile(L"\\\\.\\pipe\\1CA1185E-3F67-4AC4-B7A4-7CAADC9F994E",GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);
		if(m_pipe==INVALID_HANDLE_VALUE)
		{
			DWORD err=GetLastError();
			return false;
		}
		m_inited=true;

		wstringstream ss;
		ss<<L"connected from pid "<<GetCurrentProcessId();
		send(ss.str());

		return true;
	}

	bool send(wstring str)
	{
		if(!m_inited)
		{
			return false;
		}

		DWORD written;
		if(!OLD_WriteFile(m_pipe,str.c_str(),2*str.size(),&written,NULL))
		{
			m_inited=false;
			return false;
		}

		return true;
	}

	void unint()
	{
		send(L"disconnected");
		CloseHandle(m_pipe);
		m_pipe=NULL;
		m_inited=false;
	}

	HANDLE m_pipe;
	bool m_inited;
};

extern ipc* logger;