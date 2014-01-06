#pragma once
#include "WriteFile.h"
#include <string>
#include <sstream>
using namespace std;
#include <Windows.h>


class ipc
{
public:
	ipc():m_pid(0),m_pipe(INVALID_HANDLE_VALUE)
	{

	}

	bool init();
	bool send(wstring str);
	void unint();

private:
	bool send_internal(wstring str);

	HANDLE m_pipe;
	int m_pid;
	wstring m_pname;
	time_t m_last_connect_time;
};

extern ipc* logger;