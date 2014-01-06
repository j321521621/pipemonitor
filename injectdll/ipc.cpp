#include "stdafx.h"
#include "ipc.h"
#include <tlhelp32.h>
#include <time.h>

ipc* logger;

wstring getprocessname(int pid)
{
	wstring ret;
	HANDLE hProcs=0;
	if ( hProcs = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) )
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof( PROCESSENTRY32 );
		BOOL ok = Process32First(hProcs, &pe32);
		while (ok)
		{
			if(pid==pe32.th32ProcessID)
			{
				ret=pe32.szExeFile;
				break;
			}
			ok = Process32Next(hProcs, &pe32);
		}
		CloseHandle(hProcs);
	}
	return ret;
};


bool ipc::init()
{
	m_pid=GetCurrentProcessId();
	m_pname=getprocessname(m_pid);

	return true;
}

bool ipc::send(wstring str)
{
	time_t current=time(NULL);
	if(m_pipe==INVALID_HANDLE_VALUE && current-m_last_connect_time >10)
	{
		m_last_connect_time=current;
		m_pipe=CreateFile(L"\\\\.\\pipe\\1CA1185E-3F67-4AC4-B7A4-7CAADC9F994E",GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);

		wstringstream ss;
		ss<<L"connected from pid "<<m_pid<<" "<<m_pname;
		send_internal(ss.str());
	}

	return send_internal(str);
}

bool ipc::send_internal(wstring str)
{
	if(m_pipe==INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD written;
	if(OLD_WriteFile(m_pipe,str.c_str(),2*str.size(),&written,NULL))
	{
		return true;
	}
	else
	{
		m_pipe=INVALID_HANDLE_VALUE;
		return false;
	}
}

void ipc::unint()
{
	send(L"disconnected");
	CloseHandle(m_pipe);
	m_pipe=INVALID_HANDLE_VALUE;
}