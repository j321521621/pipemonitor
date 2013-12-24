// WTLVirtualList.h : interface of the CWTLVirtualList class
//
// Version 1.0 06/26/2011
//
// This is an extension to CListViewCtrl for WTL (Windows Template Library).
// It supports exchange of most OLEDB datatypes between an OleDB source and
// a virtual Listview control. Has been tested with SQL Server 2005, should
// work with other versions and vendors.
//
// This version does not support blob datatypes such as image or large text.
// It does not support multiple accessors. It does not support sorting.
//
// This software is distributed AS-IS, without warranties of any kind.
//
///////////////////////////////////////////////////////////////////////////
//
// WARNING: THIS CODE IS DESIGNED TO CAUSE PERMANENT DATA DELETION WHEN THE
// DELETE FUNCTION IS CALLED. USE AT YOUR OWN RISK!
//
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wtl80\atlmisc.h> // CString support
#include <atldbcli.h> // OleDB consumer support for ATL/WTL

// OleDB data conversion library
#include <msdadc.h>
#pragma comment (lib, "msdasc.lib")

// Macro to support update & insert with bookmarks enabled
#ifndef BOOKMARK_ENTRY_STATUS
#define BOOKMARK_ENTRY_STATUS(variable, status) \
	COLUMN_ENTRY_TYPE_STATUS(0, DBTYPE_BYTES, status, variable##.m_rgBuffer)
#endif

// Max size of OleDB decimal, numeric, date, dbdate, dbimestamp and GUID
// datatypes when converted to strings is 50 characters plus terminator.
// Int, real, currency, etc. are 25 characters plus terminator. Max size 
// of TCHAR string easily obtainable without using blob methods is 1000
#define MAXOLEDBNUM 64
#define MAXOLEDBSTR 1024

// Colors for bars used to highlight alternate rows
enum { NOBAR=0x00FFFFFF, BLUEBAR=0x00FFF9F0, GRAYBAR=0x00F0F0F0,
	GREENBAR=0x00F0FFF0, REDBAR=0x00F0F0FF };

// Forward reference to listview menu commands class
template<class T> class CLVMenuCommands;

// Forward reference to in-place edit control for listview
class CListViewEdit;

// The WTLVirtualList interfaces an OleDB consumer to an owner-supplied
// data (LVS_OWNERDATA) report-style listview control. A handler provides data
// in response to LVN_GETDISPINFO messages. This control dynamically creates
// listview columns using the consumer-supplied column names and displays an
// in-place edit control in response to a double click on a subitem.
//
template <class T> // Your OleDB consumer class
class CWTLVirtualList : public CWindowImpl<CWTLVirtualList<T>, CListViewCtrl>,
	public CLVMenuCommands<CWTLVirtualList<T>>
{
public:
	T m_data;

	BOOL m_FirstShow, m_ReadOnly, m_ShowBookmarks, m_SingleSelect;
	CComHeapPtr<DBBINDING> m_prgBindings; // column binding
	CComHeapPtr<DBCOLUMNINFO> m_pColumnInfo; // column info
	CComHeapPtr<OLECHAR> m_pStringsBuffer; // column names
	CFont m_Font;
	CListViewEdit m_Edit;
	COLORREF m_BarColor;
	DBLENGTH m_dbLength;
	DBSTATUS m_dbStatus;
	HRESULT m_hr;
	IDataConvert* m_pIcvt;
	int m_PointSize;
	LPCTSTR m_TypeFace;
	ULONG m_cColumnCount, m_cRowCount, m_ulActiveRow;

	// Constructor
	CWTLVirtualList() : m_ReadOnly(FALSE), m_ShowBookmarks(TRUE),
		m_SingleSelect(TRUE), m_BarColor(BLUEBAR), 
		m_FirstShow(TRUE), m_cColumnCount(0),
		m_cRowCount(0), m_PointSize(100),
		m_TypeFace(_T("MS Shell Dlg 2"))
	{
		// Initialize the OLEDB datatype conversion library
		CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY, NULL, 
			CLSCTX_INPROC_SERVER, IID_IDataConvert, (void **)&m_pIcvt);
	}

	// Destructor
	~CWTLVirtualList() { m_pIcvt->Release(); }

	BOOL PreTranslateMessage(MSG* pMsg) { pMsg; return FALSE; }

	BEGIN_MSG_MAP(CWTLVirtualList)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnLVEndLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnLVClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnLVCustomDraw)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnLVDoubleClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnLVGetDispInfo)
		CHAIN_MSG_MAP_ALT(CLVMenuCommands<CWTLVirtualList<T>>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// Used with Views: Overridden create function sets required listview styles
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0, ATL::_U_MENUorID MenuOrID = 0U, 
		LPVOID lpCreateParam = NULL)
	{
		dwStyle |= LVS_OWNERDATA; // We provide item data via the GETDISPINFO handler
		dwStyle |= LVS_REPORT; // "Details" mode
		dwStyle |= LVS_SHOWSELALWAYS;
		if (m_SingleSelect == TRUE) dwStyle |= LVS_SINGLESEL;

		return CWindowImpl<CWTLVirtualList<T>, CListViewCtrl>::Create(hWndParent,
			rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu,
			lpCreateParam);
	}

	// Use with Dialogs: Call this function from OnInitDialog to initialize
	// a dialog-hosted listview control. NOTE: You must set Owner Data true 
	// in the resource editor for the listview you wish to subclass
	LRESULT Init(HWND hWnd)
	{
		if (hWnd == NULL) return 0; else SubclassWindow(hWnd);

		DWORD dwStyle = LVS_REPORT | LVS_SHOWSELALWAYS;
		if (m_SingleSelect == TRUE) dwStyle |= LVS_SINGLESEL;
		ModifyStyle(0, dwStyle);

		BOOL b = 0;
		OnCreate(0, 0, 0, (BOOL&)b);
		OnShowWindow(0, 0, 0, (BOOL&)b);

		return 0;
	}

	// Some extended styles can only be set after the control is created
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		DWORD dwExStyle = 0;
		dwExStyle |= LVS_EX_GRIDLINES;
		dwExStyle |= LVS_EX_FULLROWSELECT;
		dwExStyle |= LVS_EX_DOUBLEBUFFER; // reduces flicker
		dwExStyle |= LVS_EX_HEADERDRAGDROP; // allow column rearranging
		SetExtendedListViewStyle(dwExStyle);

		bHandled = FALSE;
		return 0;
	}

	// Other things are set the first time the window is shown
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (m_FirstShow == TRUE)
		{
			// Create the listview and edit control font
			m_Font.CreatePointFont(m_PointSize, m_TypeFace);

			// Create the edit used for subitem data entry
			m_Edit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS |
				WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_STATICEDGE);
			m_Edit.SetFont(m_Font);

			SetFont(m_Font); // Listview font

			m_hr = OpenDatabase(); // Open the database
			if (m_hr == S_OK) m_hr = SetHeaders(); // Create the listview columns
			EnsureVisible(0, TRUE); // Make sure top row shows

			m_FirstShow = FALSE;
		}

		return 0;
	}

	// End the edit session if the user scrolls the window
	LRESULT OnScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_Edit.IsWindowVisible()) m_Edit.EndLabelEdit(NULL);

		bHandled = FALSE;
		return 0;
	}

	// Listview notification handlers

	LRESULT OnLVClick(int, LPNMHDR pNMHDR, BOOL& bHandled)
	{
		if (pNMHDR->hwndFrom != m_hWnd) return 0;
		m_ulActiveRow = SetActiveRow(((NMLISTVIEW*)pNMHDR)->iItem);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnLVCustomDraw(int, LPNMHDR pNMHDR, BOOL&)
	{
		LPNMLVCUSTOMDRAW pLVCD = (LPNMLVCUSTOMDRAW)pNMHDR;
		switch(pLVCD->nmcd.dwDrawStage) 
		{
		case CDDS_PREPAINT : return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			{
				// Paint alternate rows with "bar" background
				if ((pLVCD->nmcd.dwItemSpec & 1) == 1) pLVCD->clrTextBk = m_BarColor;
				return CDRF_NEWFONT;
			}
		}

		return CDRF_DODEFAULT;
	}

	LRESULT OnLVDoubleClick(int, LPNMHDR pNMHDR, BOOL&)
	{
		if (!IsUpdateableRowset() || pNMHDR->hwndFrom != m_hWnd) return 0;
		m_ulActiveRow = SetActiveRow(((NMLISTVIEW*)pNMHDR)->iItem);
		return StartLabelEdit();
	}

	// Called when editing has ended to save any changes to the database
	LRESULT OnLVEndLabelEdit(int, LPNMHDR pNMHDR, BOOL&)
	{
		if (pNMHDR->hwndFrom != m_Edit.m_hWnd) return 0;
		if (!IsUpdateableRowset() || !m_Edit.GetModify()) return 0;

		// Create a pointer to the item to update
		LVITEM* pItem = &((NMLVDISPINFO*)pNMHDR)->item;
		if (pItem->pszText != NULL)
		{
			m_ulActiveRow = SetActiveRow(pItem->iItem);

			// Set all columns to status IGNORE. The modified column is set
			// to status OK in the string conversion function
			for (ULONG ulBinding = 0; ulBinding < m_cColumnCount; ulBinding++)
				*(m_data.GetBuffer() + m_prgBindings[ulBinding].obStatus) = DBSTATUS_S_IGNORE;

			// Convert the edit control text and save it to the database
			m_hr = StringToOledb(pItem->pszText, m_prgBindings[pItem->iSubItem]);
			if (SUCCEEDED(m_hr)) m_hr = m_data.SetData();
			else GetOledbError(m_hr);
		}

		return 0;
	}

	// This function is called when the list needs database data for items
	// that are displayed in the visible portion of the virtual list
	LRESULT OnLVGetDispInfo(int, LPNMHDR pNMHDR, BOOL&)
	{
		if (pNMHDR->hwndFrom != m_hWnd) return 0;

		// Clear the list if a data acquisition error is encountered.
		if (!IsValidRow()) return SetRowCount();

		// Create a pointer to the item that needs data
		LVITEM* pItem = &((NMLVDISPINFO*)pNMHDR)->item;
		if (pItem->mask & LVIF_TEXT)
		{
			// Obtain the data for the virtual listview control
			if (pItem->iSubItem == 0) // Bookmark
			{
				ULONG row = SetActiveRow(pItem->iItem);
				_ltot_s(row, pItem->pszText, MAXOLEDBNUM, 10);
			}
			else OledbToString(m_prgBindings[pItem->iSubItem], pItem->pszText); // Data
		}

		return 0;
	}

	// Data conversion functions

	// From database to listview
	HRESULT OledbToString(DBBINDING data, LPTSTR pDest)
	{
		void *pSrc = m_data.GetBuffer() + data.obValue;
		DBLENGTH len;
#ifdef _UNICODE
		if (data.wType == DBTYPE_WSTR) len = (DBLENGTH)_tcslen((wchar_t *)pSrc) * 2;
#else
		if (data.wType == DBTYPE_STR) len = (DBLENGTH)_tcslen((char *)pSrc);
#endif
		else len = sizeof(pSrc);

#ifdef _UNICODE
		return m_pIcvt->DataConvert(data.wType, DBTYPE_WSTR, len,
			&m_dbLength, pSrc, pDest, MAXOLEDBSTR, DBSTATUS_S_OK,
			&m_dbStatus, 0, 0, DBDATACONVERT_DEFAULT);
#else
		return m_pIcvt->DataConvert(data.wType, DBTYPE_STR, len,
			&m_dbLength, pSrc, pDest, MAXOLEDBSTR, DBSTATUS_S_OK,
			&m_dbStatus, 0, 0, DBDATACONVERT_DEFAULT);
#endif
	}

	// From listview to database
	HRESULT StringToOledb(LPTSTR pSrc, DBBINDING data)
	{
		if (!IsUpdateableRowset()) return 0;

		void *pDest = m_data.GetBuffer() + data.obValue;
		DBLENGTH *pSrcLength = (DBLENGTH*)(m_data.GetBuffer() + data.obLength);
#ifdef _UNICODE
		*pSrcLength = (DBLENGTH)_tcslen(pSrc) * 2;
#else
		*pSrcLength = (DBLENGTH)_tcslen(pSrc);
#endif
		DBSTATUS *pSrcStatus = (DBSTATUS*)(m_data.GetBuffer() + data.obStatus);
		*pSrcStatus = DBSTATUS_S_OK;

#ifdef _UNICODE
		return m_pIcvt->DataConvert(DBTYPE_WSTR, data.wType, *pSrcLength, &m_dbLength,
			pSrc, pDest, data.cbMaxLen, *pSrcStatus, &m_dbStatus, data.bPrecision,
			data.bScale, DBDATACONVERT_LENGTHFROMNTS | DBDATACONVERT_DECIMALSCALE);
#else
		return m_pIcvt->DataConvert(DBTYPE_STR, data.wType, *pSrcLength, &m_dbLength,
			pSrc, pDest, data.cbMaxLen, *pSrcStatus, &m_dbStatus, data.bPrecision,
			data.bScale, DBDATACONVERT_LENGTHFROMNTS | DBDATACONVERT_DECIMALSCALE);
#endif
	}

	// Getters & setters

	ULONG GetActiveRow()
	{
		BYTE b[4];
		b[3] = m_data.m_Bookmark.m_rgBuffer[3];
		b[2] = m_data.m_Bookmark.m_rgBuffer[2];
		b[1] = m_data.m_Bookmark.m_rgBuffer[1];
		b[0] = m_data.m_Bookmark.m_rgBuffer[0];
		return (ULONG)( (b[3]<<24) | (b[2]<<16) | (b[1]<<8) | (b[0]) );
	}

	ULONG SetActiveRow(ULONG ulRow)
	{
		ULONG ul = ulRow + 1;
		CBookmark<4> bm;
		bm.m_rgBuffer[3] = (BYTE) ((ul>>24) & 0x000000FF);
		bm.m_rgBuffer[2] = (BYTE) ((ul>>16) & 0x000000FF);
		bm.m_rgBuffer[1] = (BYTE) ((ul>>8) & 0x000000FF);
		bm.m_rgBuffer[0] = (BYTE) (ul & 0x00FF);
		m_data.MoveToBookmark(bm);
		return GetActiveRow();
	}

	COLORREF GetBarColor() { return m_BarColor; }

	COLORREF SetBarColor(COLORREF barColor)
	{
		m_BarColor = barColor;
		return m_BarColor;
	}

	HRESULT SetHeaders()
	{
		CComPtr<IAccessor> pBindings;
		DBACCESSORFLAGS dwAccessorFlags;
		DBORDINAL ulColumns = 0, ulCol = 0;

		SetRedraw(FALSE);

		// Get a copy of the rowset bindings. Each binding contains the offsets
		// of the data, length and status variables for a specific column
		m_hr = m_data.GetInterface()->QueryInterface(&pBindings);
		if (SUCCEEDED(m_hr))
			m_hr = pBindings->GetBindings(m_data.GetHAccessor(0), &dwAccessorFlags,
			&m_cColumnCount, &m_prgBindings);
		else return GetOledbError(m_hr);

		// m_pStringsBuffer is a double null terminated (DNT) string of column
		// names. ulColumns is the column count, including the bookmark
		m_hr = m_data.GetColumnInfo(&ulColumns, &m_pColumnInfo, &m_pStringsBuffer);
		if (FAILED(m_hr)) return GetOledbError(m_hr);

		// Create a listview column for the bookmarks
		AddColumn(_T("Row"), ulCol++);
		SetShowBookmarks(m_ShowBookmarks);

		// Add one listview column for each data column and populate the headers
		// by stepping thru the DNT string and extracting the column names.
		// Modify your SELECT statement with AS to change names if desired
		LPOLESTR lpOleString = m_pStringsBuffer;
		while (ulCol < ulColumns)
		{
			AddColumn(COLE2T(lpOleString), ulCol);
			SetColumnWidth(ulCol++, LVSCW_AUTOSIZE_USEHEADER);

			// Increment the pointer to the next string in the DNT
			lpOleString = &lpOleString[_tcslen(COLE2T(lpOleString)) + 1];
		}

		SetRedraw(TRUE);

		return m_hr;
	}

	BOOL GetReadOnly() { return m_ReadOnly; }

	void SetReadOnly(BOOL bSet) { m_ReadOnly = bSet; }

	ULONG GetRowCount() { return m_cRowCount; }

	ULONG SetRowCount()
	{
		m_cRowCount = 0;
		if (IsValidRowset())
		{
			m_hr = m_data.MoveLast();
			if (m_hr == S_OK)
			{
				m_cRowCount = GetActiveRow();
				m_hr = m_data.MoveFirst();
				if (m_hr == S_OK) m_ulActiveRow = SetActiveRow(0);
			}
		}
		SetItemCount(m_cRowCount);
		return m_cRowCount;
	}

	BOOL GetShowBookmarks() { return m_ShowBookmarks; }

	void SetShowBookmarks(BOOL bSet)
	{
		if (bSet == FALSE) SetColumnWidth(0, 0);
		else SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
		m_ShowBookmarks = bSet;
	}

	BOOL GetSingleSelect() { return m_SingleSelect; }

	void SetSingleSelect(BOOL bSet)
	{
		m_SingleSelect = bSet;
		if (m_SingleSelect == TRUE) ModifyStyle(0, LVS_SINGLESEL);
		else  ModifyStyle(LVS_SINGLESEL, 0);
	}

	// Helper functions

	inline BOOL IsUpdateableRowset()
	{
		if (m_ReadOnly == TRUE || m_data.m_spRowsetChange == NULL) return FALSE;
		else return TRUE;
	}

	inline BOOL IsValidRowset() { return (m_data.m_spRowset == NULL) ? FALSE : TRUE; }

	inline BOOL IsValidRow() { return (m_data.m_hRow == NULL) ? FALSE : TRUE; }

	HRESULT OpenDatabase()
	{
		// Old-style OLEDB consumer wizard code
		__if_not_exists(T::GetRowsetProperties) { m_data.Close(); }
		__if_not_exists(T::GetRowsetProperties) { m_hr = m_data.Open(); }

		// New-style OLEDB consumer wizard code
		__if_exists(T::GetRowsetProperties) { m_data.CloseAll(); }
		__if_exists(T::GetRowsetProperties) { m_hr = m_data.OpenAll(); }

		if (FAILED(m_hr))
		{
			if (m_hr == E_FAIL)
				AtlMessageBox(m_hWnd, _T("Unable to open database."),
				_T("Problem Detected"), MB_ICONERROR);
			else GetOledbError(m_hr);
			AtlTraceErrorRecords(m_hr);
		}
		else SetRowCount();

		return m_hr;
	}

	// Copy column data to edit control and begin label edit session
	LRESULT StartLabelEdit()
	{
		if (!IsUpdateableRowset()) return 0;

		// Determine subitem to be edited
		LVHITTESTINFO lvhti;
		::GetCursorPos((LPPOINT)&lvhti.pt);
		ScreenToClient(&lvhti.pt);
		SubItemHitTest(&lvhti);

		// Bookmark is non-editable
		if (lvhti.iSubItem == 0 && GetShowBookmarks()) return 0;

		// Populate and show the edit control atop the subitem
		if (lvhti.flags & LVHT_ONITEMLABEL)
		{
			TCHAR szText[MAXOLEDBSTR];
			RECT rect;
			m_Edit.m_iItem = lvhti.iItem;
			m_Edit.m_iSubItem = lvhti.iSubItem;
			m_ulActiveRow = SetActiveRow(lvhti.iItem);
			m_hr = OledbToString(m_prgBindings[lvhti.iSubItem], szText);
			if (FAILED(m_hr)) return GetOledbError(m_hr);
			m_Edit.SetWindowText(szText);
			GetSubItemRect(lvhti.iItem, lvhti.iSubItem, LVIR_LABEL, &rect);
			m_Edit.SetWindowPos(NULL, rect.left, rect.top - 1, rect.right - rect.left + 1,
				rect.bottom - rect.top, SWP_SHOWWINDOW);
			m_Edit.SetFocus();
		}

		return 0;
	}
};


////////////////////////////////////////////////////////////////////////////////////
// CLVMenuCommands - message handlers for standard DATA commands for WTLVirtualList
////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CLVMenuCommands
{
public:
	T* pT;
	BOOL m_PromptForDelete;

	CLVMenuCommands() : m_PromptForDelete(TRUE)
	{
		pT = static_cast<T*>(this);
	}

	BEGIN_MSG_MAP(CLVMenuCommands< T >)
		ALT_MSG_MAP(1)
#ifdef ID_DATA_DELETE
		COMMAND_ID_HANDLER(ID_DATA_DELETE, OnDataDelete)
#endif
#ifdef ID_DATA_DELETE_ALL
		COMMAND_ID_HANDLER(ID_DATA_DELETE_ALL, OnDataDeleteAll)
#endif
#ifdef ID_DATA_NEW
		COMMAND_ID_HANDLER(ID_DATA_NEW, OnDataNew)
#endif
#ifdef ID_MOVE_FIRST
		COMMAND_ID_HANDLER(ID_MOVE_FIRST, OnMoveFirst)
#endif
#ifdef ID_MOVE_LAST
		COMMAND_ID_HANDLER(ID_MOVE_LAST, OnMoveLast)
#endif
#ifdef ID_MOVE_NEXT
		COMMAND_ID_HANDLER(ID_MOVE_NEXT, OnMoveNext)
#endif
#ifdef ID_MOVE_PREVIOUS
		COMMAND_ID_HANDLER(ID_MOVE_PREVIOUS, OnMovePrevious)
#endif
	END_MSG_MAP()

	// Delete selected row(s)
	LRESULT OnDataDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanDelete()) DeleteSelectedRows();
		return 0;
	}

	// Delete all existing row(s)
	LRESULT OnDataDeleteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanPerform()) DeleteAllRows();
		return 0;
	}

	// Will copy an existing selected row or can be used to insert a blank row. To insert a blank,
	// call m_data.ClearRecordMemory() first, then supply key and non-null values and execute this
	// function. For a copy, set any key values then call this function
	LRESULT OnDataNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanInsert()) InsertNewRow();
		return 0;
	}

	// Scroll to and select the first row
	LRESULT OnMoveFirst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanPerform()) MoveFirstRow();
		return 0;
	}

	// Scroll to and select the last row
	LRESULT OnMoveLast(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanPerform()) MoveLastRow();
		return 0;
	}

	// Scroll to and select the row after the current row
	LRESULT OnMoveNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanPerform()) MoveNextRow();
		return 0;
	}

	// Scroll to and select the row before the current row
	LRESULT OnMovePrevious(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (CanPerform()) MovePreviousRow();
		return 0;
	}

	// Helper functions
	inline BOOL CanDelete() { return pT->IsValidRow(); }

	inline BOOL CanInsert()
	{
		BOOL can = FALSE;
		if (!pT->IsValidRowset()) return can;
		if (pT->GetSelectedCount() < 2) can = TRUE;
		return can;
	}

	inline BOOL CanPerform()
	{
		BOOL can = FALSE;
		if (pT->GetRowCount() > 0) can = TRUE;
		return can;
	}

	// Delete selected rows. Calls DeleteAll if all rows are selected
	LRESULT DeleteSelectedRows()
	{
		if (!pT->IsUpdateableRowset() || pT->GetSelectedCount() == 0) return 0;

		if (pT->GetSelectedCount() == pT->GetRowCount()) return DeleteAllRows();

		// Prompt to make sure user wants to delete
		int prompt = IDYES;
		if (m_PromptForDelete)
		{
			prompt = AtlMessageBox(pT->m_hWnd,
				_T("This will permanently delete the selected row(s).\r\nAre you sure you want to do this?"),
				_T("Confirm Delete Selected"), MB_YESNO | MB_ICONQUESTION);
		}
		if (prompt == IDYES)
		{
			// Create an array of integers and load the selected item indices
			CSimpleArray<int> iArray;
			int pos = pT->GetNextItem(-1, LVNI_SELECTED);
			while (pos > -1)
			{
				iArray.Add(pos);
				pos = pT->GetNextItem(pos, LVNI_SELECTED);
			}

			// Delete items in reverse order because the bookmarks change as items 
			// are deleted. Could use a better way since this takes a while if lots
			// of rows are selected for deletion
			pT->SetRedraw(FALSE); // reduces flicker
			for (int i = iArray.GetSize(); i > 0; i--)
			{
				// Delete the active row
				pT->SetActiveRow(iArray[i - 1]);
				HRESULT hr = pT->m_data.Delete();
				if (FAILED(hr))
				{
					AtlMessageBox(pT->m_hWnd, _T("Unable to delete this row."), _T("Problem Detected"), MB_ICONERROR);
					AtlTraceErrorRecords(hr);
				}
				else pT->DeleteItem(iArray[i - 1]);

			}
			pT->OpenDatabase();
			pT->SetRedraw(TRUE);
		}

		return 0;
	}

	// Delete all records in the rowset
	LRESULT DeleteAllRows()
	{
		if (!pT->IsUpdateableRowset()) return 0;

		// Prompt to make sure user wants to delete
		int prompt = IDYES;
		if (m_PromptForDelete)
		{
			prompt = AtlMessageBox(pT->m_hWnd,
				_T("This will permanently delete all rows.\r\nAre you sure you want to do this?"),
				_T("Confirm Delete All"), MB_YESNO | MB_ICONQUESTION);
		}
		if (prompt == IDYES)
		{
			CComPtr<ICommandText> spCommandText;
			if (SUCCEEDED(pT->m_data.m_spCommand->QueryInterface(&spCommandText)))
			{
				// Get the default command text
				LPOLESTR szCommand;
				GUID guidCommand = DBGUID_DEFAULT;
				DBROWCOUNT pcRowsAffected = 0;
				if (FAILED(spCommandText->GetCommandText(&guidCommand, &szCommand))) return 0;

				// Build the delete command using the FROM clause of the default command.
				// If a WHERE clause exists this function will deletes records that meet
				// the where criteria, otherwise all records are deleted
				CString cmd(szCommand);
				cmd = _T("DELETE ") + cmd.Right(cmd.GetLength() - cmd.Find(_T("FROM")));

				// Execute the delete and reload the now empty database
				pT->m_data.Close();
				if (SUCCEEDED(spCommandText->SetCommandText(guidCommand, CComBSTR(LPCTSTR(cmd)))))
				{
					HRESULT hr = spCommandText->Execute(NULL, IID_NULL, NULL, &pcRowsAffected, NULL);
					if (FAILED(hr)) GetOledbError(hr);
				}
				pT->OpenDatabase();
			}
		}
		return 0;
	}

	// Call m_data.ClearRecordMemory() first to initialize the new row to 0s and then call
	// this function or else call this from a selected item to copy an existing row
	//
	// Note that any key values and non-nulls must be initialized before calling unless
	// DBPROP_SERVERDATAONINSERT is enabled and the server supplies those values
	LRESULT InsertNewRow()
	{
		if (!pT->IsUpdateableRowset() || !CanInsert()) return 0;

		for (ULONG ulBinding = 0; ulBinding < pT->m_cColumnCount; ulBinding++)
		{
			DBBINDING binding = pT->m_prgBindings[ulBinding];
			DBCOLUMNINFO colinfo = pT->m_pColumnInfo[ulBinding];

			// Ignore the bookmark column
			if ((colinfo.dwFlags & DBCOLUMNFLAGS_ISBOOKMARK) == DBCOLUMNFLAGS_ISBOOKMARK)
			{
				*(pT->m_data.GetBuffer() + binding.obStatus) = DBSTATUS_S_IGNORE;
			}
			// Handle primary key based on whether it is an identity column or a supplied value
			else if ((colinfo.dwFlags & DBCOLUMNFLAGS_RESERVED) == DBCOLUMNFLAGS_RESERVED)
			{
				if (*(pT->m_data.GetBuffer() + binding.obValue) == NULL)
				{
					*(pT->m_data.GetBuffer() + binding.obStatus) = DBSTATUS_S_IGNORE;
				}
				else *(pT->m_data.GetBuffer() + binding.obStatus) = DBSTATUS_S_OK;
			}
			// Tell the provider when a null value is coming
			else if ((colinfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE) == DBCOLUMNFLAGS_ISNULLABLE)
			{
				if (*(pT->m_data.GetBuffer() + binding.obValue) == NULL)
				{
					*(pT->m_data.GetBuffer() + binding.obStatus) = DBSTATUS_S_ISNULL;
				}
			}
			// Otherwise, set the status to OK
			else *(pT->m_data.GetBuffer() + binding.obStatus) = DBSTATUS_S_OK;

			// Set the length for string data types
			if (binding.wType == DBTYPE_STR || binding.wType == DBTYPE_WSTR)
			{
				DBLENGTH *pSrcLength = (DBLENGTH*)(pT->m_data.GetBuffer() + binding.obLength);
#ifdef _UNICODE
				*pSrcLength = (DBLENGTH)_tcslen((wchar_t*)(pT->m_data.GetBuffer() + binding.obValue)) * 2;
#else
				*pSrcLength = (DBLENGTH)_tcslen((char*)(pT->m_data.GetBuffer() + binding.obValue));
#endif
			}
		}

		HRESULT hr = pT->m_data.Insert();
		if (hr == S_OK)
		{
			// Insert the new item and select it
			pT->InsertItem(pT->GetItemCount(), NULL);
			pT->SetRowCount();
			MoveLastRow();
		}
		else if (hr == E_FAIL || hr == DB_E_ERRORSOCCURRED)
		{
			AtlMessageBox(pT->m_hWnd, _T("Unable to insert new row."), _T("Problem Detected"), MB_ICONERROR);
			AtlTraceErrorRecords(hr);
		}
		else GetOledbError(hr);

		return 0;
	}

	LRESULT MoveFirstRow()
	{
		pT->SetActiveRow(0);
		return SetSelectedItem();
	}

	LRESULT MoveLastRow()
	{
		pT->SetActiveRow(pT->GetRowCount() - 1);
		return SetSelectedItem();
	}

	LRESULT MoveNextRow()
	{
		if (pT->GetSelectedCount() != 1) return 0;
		ULONG row = pT->GetNextItem(-1, LVNI_SELECTED) + 1;
		if (pT->GetRowCount() == row) return SetSelectedItem();
		pT->SetActiveRow(row);
		return SetSelectedItem();
	}

	LRESULT MovePreviousRow()
	{
		if (pT->GetSelectedCount() != 1) return 0;
		pT->SetActiveRow((ULONG)(pT->GetNextItem(-1, LVNI_SELECTED) - 1));
		return SetSelectedItem();
	}

	LRESULT SetSelectedItem()
	{
		// Deselect all currently selected items
		pT->SetItemState(-1, 0, LVIS_SELECTED | LVIS_FOCUSED);

		// Select and focus the desired item
		pT->EnsureVisible(pT->GetActiveRow() - 1, FALSE);
		pT->SetItemState(pT->GetActiveRow() - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		pT->SetFocus();

		return 0;
	}

	HRESULT GetOledbError(HRESULT hr)
	{
		switch (hr)
		{
		case DB_E_ROWLIMITEXCEEDED:
			{
				DisplayOledbError(_T("Row could not be inserted into the rowset without\r\nexceeding provider's maximum number of active rows."));
				break;
			}
		case DB_E_READONLYACCESSOR:
			{
				DisplayOledbError(_T("Unable to perform this operation. The database is read only."));
				break;
			}
		case DB_E_BADROWHANDLE:
			{
				DisplayOledbError(_T("Unable to perform this operation. The row handle is invalid."));
				break;
			}
		case DB_SEC_E_PERMISSIONDENIED:
			{
				DisplayOledbError(_T("Unable to perform this operation. Permission was denied."));
				break;
			}
		case DB_E_INTEGRITYVIOLATION:
			{
				DisplayOledbError(_T("Unable to perform this operation. An integrity\r\nconstraint for a column or table was violated."));
				break;
			}
		case DB_E_NOTABLE:
			{
				DisplayOledbError(_T("Unable to perform this operation. Table does not exist."));
				break;
			}
		case DB_SEC_E_AUTH_FAILED:
			{
				DisplayOledbError(_T("Authentication failed."));
				break;
			}
		case DB_E_NOTSUPPORTED:
			{
				DisplayOledbError(_T("Method is not supported by this provider."));
				break;
			}
		case DB_E_NOCOLUMN:
			{
				DisplayOledbError(_T("Column ID does not exist."));
				break;
			}
		case DB_E_READONLY:
			{
				DisplayOledbError(_T("Caller requested write access to a read-only object."));
				break;
			}
		case DB_E_CANNOTCONNECT:
			{
				DisplayOledbError(_T("Cannot connect to the data source."));
				break;
			}
		case DB_E_TIMEOUT:
			{
				DisplayOledbError(_T("Timeout occurred when attempting to bind to the object."));
				break;
			}
		case DB_E_OUTOFSPACE:
			{
				DisplayOledbError(_T("Object cannot be created at this URL because\r\nthe server is out of physical storage."));
				break;
			}
		default:
			{
				DisplayOledbError(_T("Unable to comply with request. An\r\nOLEDB error was encountered."));
				break;
			}
		}

		AtlTraceErrorRecords(hr);
		return hr;
	}

	void DisplayOledbError(LPCTSTR lpszMessage)
	{
		AtlMessageBox(pT->m_hWnd, lpszMessage, _T("Problem Detected"), MB_ICONERROR);
	}
};


// Edit control used for in-place editing of listview subitems
class CListViewEdit : public CWindowImpl<CListViewEdit, CEdit>, public CEditCommands<CListViewEdit>
{
public:
	int m_iItem, m_iSubItem;

	BEGIN_MSG_MAP(CListViewEdit)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		CHAIN_MSG_MAP_ALT(CEditCommands<CListViewEdit>, 1)
	END_MSG_MAP()

	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == VK_TAB || wParam == VK_RETURN || wParam == VK_ESCAPE)
		{
			EndLabelEdit(wParam);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return EndLabelEdit(NULL);
	}

	// Prepare to copy the edit text to the listview
	LRESULT EndLabelEdit(WPARAM wParam)
	{
		if (GetModify() == TRUE)
		{
			BOOL bESC = FALSE;
			if (wParam == VK_ESCAPE) bESC = TRUE;
			TCHAR szText[MAXOLEDBSTR];
			GetWindowText((LPTSTR)szText, MAXOLEDBSTR);
			NMLVDISPINFO dispinfo = { m_hWnd, GetDlgCtrlID(), LVN_ENDLABELEDIT,
				LVIF_TEXT, m_iItem, m_iSubItem, NULL, (UINT)_tcslen(szText) };
			dispinfo.item.pszText = bESC ? NULL : szText;

			// Notify the listview control that editing has ended so it can copy the text
			::SendMessage(GetParent(), WM_NOTIFY, (WPARAM)dispinfo.hdr.idFrom, (LPARAM)&dispinfo);
		}

		// Reset the edit control
		Clear();
		SetModify(FALSE);
		SetWindowPos(NULL, 0, 0, 0, 0, 0);

		return 0;
	}
};