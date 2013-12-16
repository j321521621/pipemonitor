#pragma once

#include "ipc_handler.h"
#include <windows.h>
#include <string>
using std::string;

class printer:public ipc_handler
{
public:
	printer(int i):m_index(i)
	{

	}

	void connect() override
	{
		printf("[%d] === CONNECT ===\n",m_index);
	}

	void disconnect() override
	{
		printf("[%d] === DISCONNECT ===\n",m_index);
	}

	void handle(char *buf,int size)
	{
		string str(buf,size);
		printf("[%d] %s\n",m_index,str.c_str());
	}

	void exception()
	{
		printf("[%d] === EXCEPTION ===\n",m_index);
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