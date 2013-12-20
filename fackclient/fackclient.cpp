// fackclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma once
#include <string>
#include <sstream>
using namespace std;
#include <Windows.h>


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
	logger1->send(L"");
	logger2->send(L"");
	logger2->send(L"inject succeed");
	logger1->send(L"inject succeed");
	logger2->unint();
	logger1->unint();
}