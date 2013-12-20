#pragma once
#include <string>
using std::string;

class ipc_handler
{
public:
	enum ipc_handler_mode
	{
		sync_mode,
		async_mode,
		truncate_mode
	};

	enum ipc_handler_exception
	{
		unknown
	};
	virtual void connect(ipc_handler_mode)=0;
	virtual void disconnect(ipc_handler_mode)=0;
	virtual void handle(ipc_handler_mode,string data)=0;
	virtual void exception(ipc_handler_mode,ipc_handler_exception)=0;
};

class ipc_handler_factory
{
public:
	virtual ipc_handler *create()=0;
};