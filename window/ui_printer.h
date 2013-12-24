#pragma once

#include "ipc_handler.h"
#include <windows.h>
#include <string>
using std::string;
using std::wstring;


class ui_printer:public ipc_handler
{
public:
	ui_printer(HWND hwnd);

	void connect(ipc_handler_mode mode) override;
	void disconnect(ipc_handler_mode mode) override;
	void handle(ipc_handler_mode mode,string data);
	void exception(ipc_handler_mode mode,ipc_handler_exception);

private:
	HWND m_hwnd;
	bool m_inited;
	bool m_closed;
	wstring m_pid;
	wstring m_pname;
};

class ui_printer_factory:public ipc_handler_factory
{
public:
	ui_printer_factory(HWND list);
	ipc_handler *create() override;

private:
	int m_index;
	HWND m_list;
};