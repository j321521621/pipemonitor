#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <wtl80/atlapp.h>
#include <wtl80/atlmisc.h>
#include <wtl80/atlcrack.h>
#include <wtl80/atlctrls.h>
#include <mshtmcid.h>
#include "ui_printer_data.h"

#include "vt.h"

class MyListView: public CWindowImpl<MyListView,CListViewCtrl,CWinTraitsOR<LVS_REPORT|LVS_SHOWSELALWAYS,0,CControlWinTraits>>
{
public:
	BEGIN_MSG_MAP_EX(MyListView)
	END_MSG_MAP()

	VOID InitStyle()
	{
		SetExtendedListViewStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_AUTOSIZECOLUMNS);
	}


};

class MyToolBar: public CWindowImpl<MyToolBar,CToolBarCtrl,CWinTraitsOR<0,0,CControlWinTraits>>
{

public:
	BEGIN_MSG_MAP_EX(MyToolBar)
		MSG_WM_CREATE(OnCreate)
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		SetImageList(ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK,3, 0),0);
		LoadStdImages(IDB_STD_SMALL_COLOR);
		return 0;
	}
};

class ProductManagerWin : public CDialogImpl<ProductManagerWin,CWindow>
{
public:
	BEGIN_MSG_MAP_EX(MainWin)
	END_MSG_MAP()

	static DWORD const IDD=IDD_DIALOG1;


};

class MainWin : public CWindowImpl<MainWin,CWindow,CFrameWinTraits>
{
public:
		BEGIN_MSG_MAP_EX(MainWin)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_SIZE(OnSize)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_DESTROY(OnDestroy)
			COMMAND_HANDLER_EX(IDM_NEW,BN_CLICKED,OnOpen)
			COMMAND_HANDLER_EX(IDM_OPEN,BN_CLICKED,OnOpen)
			COMMAND_HANDLER_EX(IDM_SAVE,BN_CLICKED,OnOpen)
			MESSAGE_HANDLER(WM_ADDDATA,OnAddData)
		END_MSG_MAP()

		int OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			SetWindowText(L"程序监视器");
			m_toolbar.Create(m_hWnd,NULL);

			m_toolbar.AddButton(IDM_NEW,0,TBSTATE_ENABLED,STD_FILENEW,(INT_PTR)L"注入",0);
			m_toolbar.AddButton(IDM_OPEN,0,TBSTATE_ENABLED,STD_FILEOPEN,(INT_PTR)L"打开",0);
			m_toolbar.AddButton(IDM_SAVE,0,TBSTATE_ENABLED,STD_FILESAVE,(INT_PTR)L"关闭",0);
			m_toolbar.SetExtendedStyle(TBSTYLE_EX_DOUBLEBUFFER);
			m_toolbar.AutoSize();

			m_listview.Create(m_hWnd,NULL);
			m_listview.InitStyle();

			m_listview.AddColumn(L"进程ID", 0);
			m_listview.AddColumn(L"进程名", 1);
			m_listview.AddColumn(L"线程ID", 2);
			m_listview.AddColumn(L"消息长度", 3);
			m_listview.AddColumn(L"消息内容", 4);

			return 0;
		}

		void OnSize(UINT nType, CSize size)
		{
			CRect rc;
			GetClientRect(&rc);
			//m_toolbar.SetWindowPos(NULL,0,0,rc.right,30,SWP_NOZORDER);
			m_listview.SetWindowPos(NULL,0,50,rc.right,rc.bottom,SWP_NOZORDER);
		}

		void OnClose()
		{
			if(MessageBox(L"退出后现有链接将关闭,确认退出么?",L"退出",MB_OKCANCEL)==IDOK)
			{
				SetMsgHandled(FALSE);
			}
		}

		void OnDestroy()
		{
			PostQuitMessage(0);
		}

		LRESULT OnAddData(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
		{
			bHandled=TRUE;
			listdata *ld=(listdata*)lParam;

			m_listview.AddItem(0,0,ld->pid.c_str());
			m_listview.AddItem(0,1,ld->pname.c_str());
			m_listview.AddItem(0,2,ld->tid.c_str());
			m_listview.AddItem(0,4,ld->data.c_str());
			delete(ld);
			return 0;
		}

		void OnOpen(UINT uNotifyCode, int nID, CWindow wndCtl)
		{
			m_pm.DoModal();
		}

MyListView m_listview;
MyToolBar m_toolbar;
ProductManagerWin m_pm;

};