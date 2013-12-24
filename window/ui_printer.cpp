#include "stdafx.h"
#include "ui_printer.h"
#include "ui_printer_data.h"
#include <Windows.h>
#include <sstream>
using std::wstringstream;




ui_printer::ui_printer(HWND hwnd):m_hwnd(hwnd),m_inited(false),m_closed(false)
{

}

void ui_printer::connect(ipc_handler_mode mode)
{
	//printf("[%d][%s] === CONNECT ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
}

void ui_printer::disconnect(ipc_handler_mode mode)
{
	//printf("[%d][%s] === DISCONNECT ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
}

void ui_printer::handle(ipc_handler_mode mode,string data)
{
	wstring wstr((PWCHAR)(data.c_str()),data.size()/2);

	if(m_closed)
	{
		return;
	}

	if(!m_inited)
	{
		if(wstr.length()>=19 && wstring(wstr,0,19)==L"connected from pid ")
		{
			wstring postfix=wstring(wstr,19);
			int i=postfix.find_first_of(L" ");
			if(i<0)
			{
				return;
			}
			m_pid=wstring(postfix,0,i);
			m_pname=wstring(postfix,i);
			m_inited=true;
		}
		return;
	}
	
	if(wstr==L"disconnected")
	{
		m_inited=true;
	}
	else
	{
		listdata* ld=new listdata();
		ld->pid=m_pid;
		ld->pname=m_pname;
		ld->tid=L"0";
		ld->data=wstr;
		::PostMessage(m_hwnd,WM_ADDDATA,0,(LPARAM)ld);
	}
}

void ui_printer::exception(ipc_handler_mode mode,ipc_handler_exception)
{
	//printf("[%d][%s] === EXCEPTION ===\n",m_index,mode==sync_mode?"S":mode==async_mode?"A":"T");
}

ui_printer_factory::ui_printer_factory(HWND list):m_index(0),m_list(list)
{

}

ipc_handler *ui_printer_factory::create()
{
	return new ui_printer(m_list);
}

