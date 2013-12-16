// fackclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
using std::wstring;
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

		DWORD written;
		if(!WriteFile(m_pipe,"conectedconectedconectedconectedconectedconected",48,&written,NULL))
		{
			return false;
		}

		CloseHandle(m_pipe);


		m_inited=true;

		return true;
	}


	void unint()
	{
		CloseHandle(m_pipe);
		m_pipe=NULL;
		m_inited=false;
	}

	HANDLE m_pipe;
	bool m_inited;
};


int _tmain(int argc, _TCHAR* argv[])
{
	ipc ip;
	ip.init();
	return 0;

}

