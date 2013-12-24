#pragma once

#include "ipc_handler.h"
#include <windows.h>
#include <string>
using std::wstring;


class printer:public ipc_handler
{
public:
	printer(int i):m_index(i)
	{

	}

	void connect(ipc_handler_mode mode) override
	{
		printf("[%d][%s] === CONNECT ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
	}

	void disconnect(ipc_handler_mode mode) override
	{
		printf("[%d][%s] === DISCONNECT ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
	}

	void handle(ipc_handler_mode mode,string data)
	{
		wstring wstr((PWCHAR)(data.c_str()),data.size()/2);
		wprintf(L"[%d][%s] %s\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T",wstr.c_str());
	}

	void exception(ipc_handler_mode mode,ipc_handler_exception)
	{
		printf("[%d][%s] === EXCEPTION ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
	}

private:
	int m_index;

};

class printer_factory:public ipc_handler_factory
{
public:
	printer_factory():m_index(0)
	{

	}

	ipc_handler *create() override
	{
		return new printer(m_index++);
	}

private:
	int m_index;
};