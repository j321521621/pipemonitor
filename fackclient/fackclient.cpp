// fackclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

#pragma once
#include <string>
#include <sstream>
using namespace std;
#include <Windows.h>

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

		int pid=GetCurrentProcessId();
		wstringstream ss;
		ss<<L"connected from pid "<<pid<<" "<<getprocessname(pid);
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
		if(!WriteFile(m_pipe,str.c_str(),2*str.size(),&written,NULL))
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
int _tmain(int argc, _TCHAR* argv[])
{
	string str;
	str.resize(5000,'a');

	ipc* logger1=new ipc();
	ipc* logger2=new ipc();

	logger1->init();
	logger2->init();
	logger2->send(L"inject succeed");
	logger1->send(L"inject succeed");
	logger2->unint();
	logger1->unint();
}