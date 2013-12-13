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
		m_pipe=CreateFile(L"\\.\pipe\7E90B034-BDDF-4CD5-9638-371EB9918763",GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);
		if(m_pipe==INVALID_HANDLE_VALUE)
		{
			return false;
		}

		DWORD written;
		if(!WriteFile(m_pipe,"conected",8,&written,NULL))
		{
			return false;
		}

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