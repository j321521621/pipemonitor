#pragma once

class ipc_handler
{
public:
	virtual void connect()=0;
	virtual void disconnect()=0;
	virtual void handle(char* buf,int size)=0;
	virtual void exception()=0;
};

class ipc_handler_factory
{
public:
	virtual ipc_handler *create()=0;
};