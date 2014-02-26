#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <wtl80/atlapp.h>
#include <wtl80/atlmisc.h>
#include <wtl80/atlcrack.h>
#include <wtl80/atlctrls.h>
#include <mshtmcid.h>
#include "ui_printer_data.h"
#include "common.h"
#include "doctor.h"

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
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_NOTIFY(OnNotify)
	END_MSG_MAP()

	static DWORD const IDD=IDD_DIALOG1;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CRect rc;
		GetClientRect(&rc);
		m_listview.Create(m_hWnd,rc);
		m_listview.InitStyle();
		m_listview.AddColumn(L"进程ID", 0);
		m_listview.SetColumnWidth(0,100);
		m_listview.AddColumn(L"进程名", 1);
		m_listview.SetColumnWidth(1,200);
		m_listview.AddColumn(L"命令行", 2);
		m_listview.SetColumnWidth(2,300);
		Update();

		return TRUE;
	}

	void OnClose()
	{
		EndDialog(0);
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
	}

	LRESULT OnNotify(int idCtrl, LPNMHDR pnmh)
	{
		if(pnmh->hwndFrom==m_listview && pnmh->code==NM_DBLCLK)
		{
			LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pnmh;
			WCHAR title[4096];
			m_listview.GetItemText(lpnmitem->iItem,0,title,4096);
			EndDialog(_wtoi(title));
		}
		return 0;
	}

private:

	void Update()
	{
		m_listview.SetRedraw(FALSE);
		m_listview.DeleteAllItems();
		vector<vector<wstring>> ps=common::proc_snapshot();
		for(vector<vector<wstring>>::iterator it=ps.begin();it!=ps.end();++it)
		{
			m_listview.AddItem(0,0,(*it)[0].c_str());
			m_listview.AddItem(0,1,(*it)[1].c_str());
		}
		m_listview.SetRedraw(TRUE);
	}

	MyListView m_listview;
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
			SetMsgHandled(FALSE);
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
			int pid=m_pm.DoModal();
			if(pid==0)
			{
				return;
			}

			inject_ret rslt=inject(pid,L"..\\Debug\\injectdll.dll");
			if(rslt!=S_INJECT)
			{
				wstringstream ss;
				ss<<L"注入失败("<<rslt<<L"),pid="<<pid;
				::MessageBox(m_hWnd,ss.str().c_str(),L"注入失败" ,MB_OK);
			}
		}

MyListView m_listview;
MyToolBar m_toolbar;
ProductManagerWin m_pm;

};