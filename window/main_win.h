#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <wtl80/atlapp.h>
#include <wtl80/atlmisc.h>
#include <wtl80/atlcrack.h>
#include <wtl80/atlctrls.h>
#include "ui_printer_data.h"

#include "vt.h"

class MyListView: public CWindowImpl<MyListView,CListViewCtrl,CWinTraitsOR<LVS_REPORT|LVS_SHOWSELALWAYS,0,CControlWinTraits>>
{
public:
	BEGIN_MSG_MAP_EX(MyListView)
		MESSAGE_HANDLER(WM_ADDDATA,OnAddData)
	END_MSG_MAP()

	VOID InitStyle()
	{
		DWORD dwExStyle = 0;
		dwExStyle |= LVS_EX_GRIDLINES;
		dwExStyle |= LVS_EX_FULLROWSELECT;
		dwExStyle |= LVS_EX_DOUBLEBUFFER; // reduces flicker
		dwExStyle |= LVS_EX_AUTOSIZECOLUMNS; // allow column rearranging

		SetExtendedListViewStyle(dwExStyle);

		AddColumn(L"进程ID", 0);
		AddColumn(L"进程名", 1);
		AddColumn(L"线程ID", 2);
		AddColumn(L"消息长度", 3);
		AddColumn(L"消息内容", 4);

	}

	LRESULT OnAddData(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
	{
		bHandled=TRUE;
		listdata *ld=(listdata*)lParam;

		AddItem(0,0,ld->pid.c_str());
		AddItem(0,1,ld->pname.c_str());
		AddItem(0,2,ld->tid.c_str());
		AddItem(0,4,ld->data.c_str());

		delete(ld);

		return 0;
	}
};

class MyToolBar: public CWindowImpl<MyToolBar,CToolBarCtrl,CWinTraitsOR<LVS_REPORT|LVS_SHOWSELALWAYS,0,CControlWinTraits>>
{

public:
	BEGIN_MSG_MAP_EX(MyToolBar)
	END_MSG_MAP()
};

class MainWin : public CWindowImpl<MainWin,CWindow,CFrameWinTraits>
{
public:
		BEGIN_MSG_MAP_EX(MainWin)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_SIZE(OnSize)
		END_MSG_MAP()

		int OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			m_toolbar.Create(m_hWnd,NULL);
			m_listview.Create(m_hWnd,NULL);
			m_listview.InitStyle();

			return 0;
		}

		void OnSize(UINT nType, CSize size)
		{
			CRect rc;
			GetClientRect(&rc);
			m_toolbar.SetWindowPos(NULL,0,0,rc.right,30,SWP_NOZORDER);
			m_listview.SetWindowPos(NULL,0,30,rc.right,rc.bottom,SWP_NOZORDER);
		}

MyListView m_listview;
MyToolBar m_toolbar;

};