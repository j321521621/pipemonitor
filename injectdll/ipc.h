#pragma once
#include "WriteFile.h"
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

	bool init();
	bool send(wstring str);
	void unint();

	HANDLE m_pipe;
	bool m_inited;
};

extern ipc* logger;