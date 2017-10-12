// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <AtlCtrlx.h>		// for CWaitCursor
#include <atlctrls.h>
#include "mainfrm.h"

#include "props.h"

#define FILE_MENU_POSITION	0
#define RECENT_MENU_POSITION	9
#define POPUP_MENU_POSITION	0

// Status Bar Panes
enum status_panes
{
	STATUS_DEFAULT, STATUS_CURSOR, STATUS_PIXEL, STATUS_PERCENT, STATUS_PANES
};

LPCTSTR g_lpcstrMRURegKey = _T("Software\\Microsoft\\WTL Samples\\BmpView");
LPCTSTR g_lpcstrApp = _T("BmpView");

TCHAR szAppName[80];
TCHAR szFilter[256];

CMainFrame::CMainFrame() 
#ifndef _WIN32_WCE
	: m_bPrintPreview(false)
#endif // _WIN32_WCE
{
#ifndef _WIN32_WCE
	::SetRect(&m_rcMargin, 1000, 1000, 1000, 1000);
	m_printer.OpenDefaultPrinter();
	m_devmode.CopyFromPrinter(m_printer);
#endif // _WIN32_WCE
	AtlLoadString(IDR_MAINFRAME, szAppName, sizeof(szAppName));
	AtlLoadString(IDS_FILTER, szFilter, sizeof(szFilter));
	char* p;
	while ((p = strrchr(szFilter, '|')) != NULL)
		*p = '\0';
	OnNewDocument();
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	BOOL bEnable = !m_dib.IsNull();
#ifndef _WIN32_WCE
	UIEnable(ID_FILE_PRINT, bEnable);
	UIEnable(ID_FILE_PRINT_PREVIEW, bEnable);
	UISetCheck(ID_FILE_PRINT_PREVIEW, m_bPrintPreview);
#endif // _WIN32_WCE
	UIEnable(ID_EDIT_COPY, bEnable);
	UIEnable(ID_EDIT_PASTE, ::IsClipboardFormatAvailable(CF_BITMAP));
	UIEnable(ID_EDIT_CLEAR, bEnable);
	UIEnable(ID_VIEW_PROPERTIES, bEnable);
#ifndef _WIN32_WCE
	UISetCheck(ID_RECENT_BTN, m_list.IsWindowVisible());
#endif // _WIN32_WCE
#ifndef WIN32_PLATFORM_PSPC
	UIUpdateToolBar();
#endif // WIN32_PLATFORM_PSPC

	TCHAR szText[80];
	wsprintf(szText, "%u %%", m_view.GetZoomPercent());
	m_statbar.SetText(STATUS_PERCENT, szText);
	CPoint cursor = m_view.GetCursor();
	wsprintf(szText, "(%u,%u)", cursor.x, cursor.y);
	m_statbar.SetText(STATUS_CURSOR,  szText);
	COLORREF rgb = m_dib.GetPixel(cursor.x, cursor.y);
    int alpha = (rgb >> 24) & 255;
	if (rgb != INVALID_PIXEL)
		wsprintf(szText, "%02X%02X%02X%02X", alpha, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
	else
		lstrcpy(szText, "???");
	m_statbar.SetText(STATUS_PIXEL, szText);

	UIUpdateStatusBar();

	return FALSE;
}

#ifndef _WIN32_WCE
void CMainFrame::TogglePrintPreview()
{
	if(m_bPrintPreview)	// close it
	{
		ATLASSERT(m_hWndClient == m_wndPreview.m_hWnd);

		m_hWndClient = m_view;
		m_view.ShowWindow(SW_SHOW);
		m_wndPreview.DestroyWindow();
	}
	else			// display it
	{
		ATLASSERT(m_hWndClient == m_view.m_hWnd);

		m_wndPreview.SetPrintPreviewInfo(m_printer, m_devmode.m_pDevMode, this, 0, 0);
		m_wndPreview.SetPage(0);

		m_wndPreview.Create(m_hWnd, rcDefault, NULL, 0, WS_EX_CLIENTEDGE);
		m_view.ShowWindow(SW_HIDE);
		m_hWndClient = m_wndPreview;
	}

	m_bPrintPreview = !m_bPrintPreview;
	UpdateLayout();
}
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
void CMainFrame::UpdateTitleBar(LPCTSTR lpstrTitle)
{
	CString strDefault;
	strDefault.LoadString(IDR_MAINFRAME);
	CString strTitle = strDefault;
	if(lpstrTitle != NULL)
	{
		strTitle = lpstrTitle;
		strTitle += _T(" - ");
		strTitle += strDefault;
	}
	SetWindowText(strTitle);
}
#endif // WIN32_PLATFORM_PSPC

#ifndef _WIN32_WCE
//print job info callbacks
bool CMainFrame::IsValidPage(UINT nPage)
{
	return (nPage == 0);	// we have only one page
}

bool CMainFrame::PrintPage(UINT nPage, HDC hDC)
{
	if (nPage >= 1)		// we have only one page
		return false;

	ATLASSERT(!m_dib.IsNull());

	RECT rcPage = 
		{ 0, 0, 
		::GetDeviceCaps(hDC, PHYSICALWIDTH) - 2 * ::GetDeviceCaps(hDC, PHYSICALOFFSETX),
		::GetDeviceCaps(hDC, PHYSICALHEIGHT) - 2 * ::GetDeviceCaps(hDC, PHYSICALOFFSETY) };

	CDCHandle dc = hDC;
	CClientDC dcScreen(m_hWnd);
	CDC dcMem;
	dcMem.CreateCompatibleDC(dcScreen);
	HBITMAP hBmpOld = dcMem.SelectBitmap(m_dib);
	int cx = m_dib.GetWidth();
	int cy = m_dib.GetHeight();

	// calc scaling factor, so that bitmap is not too small
	// (based on the width only, max 3/4 width)
	int nScale = ::MulDiv(rcPage.right, 3, 4) / cx;
	if(nScale == 0)		// too big already
		nScale = 1;
	// calc margines to center bitmap
	int xOff = (rcPage.right - nScale * cx) / 2;
	if(xOff < 0)
		xOff = 0;
	int yOff = (rcPage.bottom - nScale * cy) / 2;
	if(yOff < 0)
		yOff = 0;
	// ensure that preview doesn't go outside of the page
	int cxBlt = nScale * cx;
	if(xOff + cxBlt > rcPage.right)
		cxBlt = rcPage.right - xOff;
	int cyBlt = nScale * cy;
	if(yOff + cyBlt > rcPage.bottom)
		cyBlt = rcPage.bottom - yOff;

	// now paint bitmap
	dc.StretchBlt(xOff, yOff, cxBlt, cyBlt, dcMem, 0, 0, cx, cy, SRCCOPY);

	dcMem.SelectBitmap(hBmpOld);

	return true;
}
#endif // _WIN32_WCE

int CMainFrame::OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
{
#ifndef _WIN32_WCE
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// atach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	// create toolbar and rebar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
#else // _WIN32_WCE
#ifndef WIN32_PLATFORM_PSPC
	CreateSimpleCECommandBar(MAKEINTRESOURCE(IDR_MAINFRAME));
	CreateSimpleToolBar();
#else // WIN32_PLATFORM_PSPC
	CreateSimpleCEMenuBar(IDR_MAINFRAME, SHCMBF_HMENU);
#endif // WIN32_PLATFORM_PSPC
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
	// create status bar
	CreateSimpleStatusBar();
	m_statbar = m_hWndStatusBar;
#endif // WIN32_PLATFORM_PSPC

	// create view window
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	m_view.m_pDoc = &m_dib;

#ifndef _WIN32_WCE
	// set up MRU stuff
	CMenuHandle menu = m_CmdBar.GetMenu();
	CMenuHandle menuFile = menu.GetSubMenu(FILE_MENU_POSITION);
	CMenuHandle menuMru = menuFile.GetSubMenu(RECENT_MENU_POSITION);
	m_mru.SetMenuHandle(menuMru);
	m_mru.SetMaxEntries(10);

	m_mru.ReadFromRegistry(g_lpcstrMRURegKey);

	// create MRU list
	m_list.Create(m_hWnd);
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
	// set up update UI
#ifndef _WIN32_WCE
	UIAddToolBar(hWndToolBar);
#else // _WIN32_WCE
	UIAddToolBar(m_hWndToolBar);
#endif // _WIN32_WCE
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
#endif // WIN32_PLATFORM_PSPC

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	OnViewZoom(0, ID_VIEW_ZOOM_100, m_view);

	return 0;
}

void CMainFrame::OnSize(UINT nType, CSize size)
{
	if (m_statbar.IsWindow())
	{
		int parts[4];
		parts[0] = size.cx - 300;
		parts[1] = size.cx - 225;
		parts[2] = size.cx - 75;
		parts[3] = size.cx;
		m_statbar.SetParts(4, parts);
	}
	BOOL bHandled;
	CFrameWindowImpl<CMainFrame>::OnSize(0, nType, MAKELONG(size.cx, size.cy), bHandled);
}

#ifndef _WIN32_WCE
void CMainFrame::OnContextMenu(CWindow wnd, CPoint point)
{
	if(wnd.m_hWnd == m_view.m_hWnd)
	{
		CMenu menu;
		menu.LoadMenu(IDR_CONTEXTMENU);
		CMenuHandle menuPopup = menu.GetSubMenu(POPUP_MENU_POSITION);
		m_CmdBar.TrackPopupMenu(menuPopup, TPM_RIGHTBUTTON | TPM_VERTICAL, point.x, point.y);
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}
#endif // _WIN32_WCE

void CMainFrame::OnFileExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::AppError(UINT id, int error)
{
	TCHAR szError[256];
	TCHAR szFormat[80];
	AtlLoadString(id, szFormat, sizeof(szFormat));
	wsprintf(szError, szFormat, error);
	MessageBox(szError, szAppName, MB_ICONERROR);
}

BOOL CMainFrame::QueryClose()
{
	if (!m_dib.IsModified())
		return TRUE;

	TCHAR szBuff[MAX_PATH + 30];
	TCHAR szFormat[80];
	AtlLoadString(IDS_SAVE_CHANGES, szFormat, sizeof(szFormat));

	wsprintf(szBuff, szFormat, m_szFilePath);
	int nRet = MessageBox(szBuff, szAppName, MB_YESNOCANCEL | MB_ICONEXCLAMATION);

	if(nRet == IDCANCEL)
		return FALSE;
	else if(nRet == IDYES)
	{
		if (DoFileSave(m_szFilePath) < 0)
			return FALSE;
	}

	return TRUE;
}

void CMainFrame::OnNewDocument()
{
	// Clear out font information
	m_dib.SetBitmap(NULL);

	// Clear filename
	AtlLoadString(IDS_UNTITLED, m_szFilePath, sizeof(m_szFilePath));
}

int CMainFrame::DoFileOpen(const char* pszFileName)
{
	// Try to bimap file
	int result = m_view.LoadFile(pszFileName);
	if (result >= 0)
	{
#ifndef _WIN32_WCE
		m_mru.AddToList(pszFileName);
		m_mru.WriteToRegistry(g_lpcstrMRURegKey);
#endif // _WIN32_WCE
#ifndef WIN32_PLATFORM_PSPC
		UpdateTitleBar(pszFileName);
#endif // WIN32_PLATFORM_PSPC
		lstrcpy(m_szFilePath, pszFileName);
	}
	else
	{
		AppError(IDS_ERR_FILE_OPEN, result);
	}
	return result;
}

LRESULT CMainFrame::OnDropFiles(HDROP hDropFiles)
{
	TCHAR szFileName[MAX_PATH];
	// Only support the first file dropped!
	UINT length = ::DragQueryFile(hDropFiles, 0, szFileName, sizeof(szFileName));
	if (length)
		DoFileOpen(szFileName);
	::DragFinish(hDropFiles);
	return 0;
}

int CMainFrame::DoFileSave(const char* filename)
{
	if (filename[0] == '\0' || filename[0] == '<')
		return DoFileSaveAs();

	/* save PFT file */

	int result = m_dib.SaveFile(filename);
	if (result < 0)
	{
		AppError(IDS_ERR_FILE_SAVE, result);
	}
	return result;
}

int CMainFrame::DoFileSaveAs()
{
	CFileDialog dlg(/*bOpen=*/FALSE, /*szDefExt=*/ ".pft", /*pszFileName=*/NULL, /*dwFlags=*/ 0, szFilter, m_hWnd);
	int result = -1;
	if (dlg.DoModal() == IDOK)
	{
		LPSTR filename = dlg.m_ofn.lpstrFile;
		lstrcpy(m_szFilePath, filename);
		result = DoFileSave(filename);
	}
	return result;
}

void CMainFrame::OnFileSave(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	DoFileSave(m_szFilePath);
}

void CMainFrame::OnFileSaveAs(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	DoFileSaveAs();
}

void CMainFrame::OnFileOpen(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CFileDialog dlg(TRUE, _T("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, m_hWnd);
	if(dlg.DoModal() == IDOK)
	{
		if(m_bPrintPreview)
			TogglePrintPreview();

		if (!QueryClose())
		{
			if(DoFileSave(m_szFilePath) < 0)
				return;
		}
		CWaitCursor wait;
		DoFileOpen(dlg.m_ofn.lpstrFile);
	}
}

#ifndef _WIN32_WCE
void CMainFrame::OnFileRecent(UINT /*uNotifyCode*/, int nID, CWindow /*wnd*/)
{
	if(m_bPrintPreview)
		TogglePrintPreview();

	// get file name from the MRU list
	TCHAR szFile[MAX_PATH];
	if(m_mru.GetFromList(nID, szFile, MAX_PATH))
	{
		// open file
		int result = m_view.LoadFile(szFile);
		if (result >= 0)
		{
			m_mru.MoveToTop(nID);
			// get file name without the path
			int nFileNamePos = 0;
			for(int i = lstrlen(szFile) - 1; i >= 0; i--)
			{
				if(szFile[i] == _T('\\'))
				{
					nFileNamePos = i + 1;
					break;
				}
			}
			UpdateTitleBar(&szFile[nFileNamePos]);
			lstrcpy(m_szFilePath, szFile);
		}
		else
		{
			m_mru.RemoveFromList(nID);
		}
		m_mru.WriteToRegistry(g_lpcstrMRURegKey);
	}
	else
	{
		::MessageBeep(MB_ICONERROR);
	}
}

void CMainFrame::OnRecentButton(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	UINT uBandID = ATL_IDW_BAND_FIRST + 1;	// toolbar is second added band
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(uBandID);
	REBARBANDINFO rbbi = { 0 };
	rbbi.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
	rbbi.fMask = RBBIM_CHILD;
	rebar.GetBandInfo(nBandIndex, &rbbi);
	CToolBarCtrl wndToolBar = rbbi.hwndChild;

	int nIndex = wndToolBar.CommandToIndex(ID_RECENT_BTN);
	CRect rect;
	wndToolBar.GetItemRect(nIndex, rect);
	wndToolBar.ClientToScreen(rect);

	// build and display MRU list in a popup
	m_list.BuildList(m_mru);
	m_list.ShowList(rect.left, rect.bottom);
}
#endif // _WIN32_WCE

#ifndef _WIN32_WCE
void CMainFrame::OnFilePrint(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CPrintDialog dlg(FALSE);
	dlg.m_pd.hDevMode = m_devmode.CopyToHDEVMODE();
	dlg.m_pd.hDevNames = m_printer.CopyToHDEVNAMES();
	dlg.m_pd.nMinPage = 1;
	dlg.m_pd.nMaxPage = 1;

	if(dlg.DoModal() == IDOK)
	{
		m_devmode.CopyFromHDEVMODE(dlg.m_pd.hDevMode);
		m_printer.ClosePrinter();
		m_printer.OpenPrinter(dlg.m_pd.hDevNames, m_devmode.m_pDevMode);

		CPrintJob job;
		job.StartPrintJob(false, m_printer, m_devmode.m_pDevMode, this, _T("BmpView Document"), 0, 0, (dlg.PrintToFile() != FALSE));
	}

	::GlobalFree(dlg.m_pd.hDevMode);
	::GlobalFree(dlg.m_pd.hDevNames);
}

void CMainFrame::OnFilePageSetup(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CPageSetupDialog dlg;
	dlg.m_psd.hDevMode = m_devmode.CopyToHDEVMODE();
	dlg.m_psd.hDevNames = m_printer.CopyToHDEVNAMES();
	dlg.m_psd.rtMargin = m_rcMargin;

	if(dlg.DoModal() == IDOK)
	{
		if(m_bPrintPreview)
			TogglePrintPreview();

		m_devmode.CopyFromHDEVMODE(dlg.m_psd.hDevMode);
		m_printer.ClosePrinter();
		m_printer.OpenPrinter(dlg.m_psd.hDevNames, m_devmode.m_pDevMode);
		m_rcMargin = dlg.m_psd.rtMargin;
	}

	::GlobalFree(dlg.m_psd.hDevMode);
	::GlobalFree(dlg.m_psd.hDevNames);
}

void CMainFrame::OnFilePrintPreview(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	TogglePrintPreview();
}
#endif // _WIN32_WCE

void CMainFrame::OnEditCopy(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	if(::OpenClipboard(NULL))
	{
        ::EmptyClipboard();
#ifndef _WIN32_WCE
		HBITMAP hBitmapCopy = (HBITMAP)::CopyImage(m_dib.m_hBitmap, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
#else // _WIN32_WCE
		// TODO - JoshHe 7/03/2003 - provide alternate BitMap copy routine.
		HBITMAP hBitmapCopy = NULL;
#endif // _WIN32_WCE
		if(hBitmapCopy != NULL)
			::SetClipboardData(CF_BITMAP, hBitmapCopy);
		else
			MessageBox(_T("Can't copy bitmap"), g_lpcstrApp, MB_OK | MB_ICONERROR);

		::CloseClipboard();
	}
	else
	{
		MessageBox(_T("Can't open clipboard to copy"), g_lpcstrApp, MB_OK | MB_ICONERROR);
	}
}

void CMainFrame::OnEditPaste(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
#ifndef _WIN32_WCE
	if(m_bPrintPreview)
		TogglePrintPreview();
#endif // _WIN32_WCE

	if(::OpenClipboard(NULL))
	{
		HBITMAP hBitmap = (HBITMAP)::GetClipboardData(CF_BITMAP);
		::CloseClipboard();
		if(hBitmap != NULL)
		{
#ifndef _WIN32_WCE
			HBITMAP hBitmapCopy = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
#else // _WIN32_WCE
			// TODO - JoshHe 7/03/2003 - provide alternate BitMap copy routine.
			HBITMAP hBitmapCopy = NULL;
#endif // _WIN32_WCE
			if(hBitmapCopy != NULL)
			{
				m_dib.SetBitmap(hBitmapCopy);
#ifndef WIN32_PLATFORM_PSPC
				UpdateTitleBar(_T("(Clipboard)"));
#endif // WIN32_PLATFORM_PSPC
				m_view.Invalidate();
				m_szFilePath[0] = 0;
			}
			else
			{
				MessageBox(_T("Can't paste bitmap"), g_lpcstrApp, MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			MessageBox(_T("Can't open bitmap from the clipboard"), g_lpcstrApp, MB_OK | MB_ICONERROR);
		}
	}
	else
	{
		MessageBox(_T("Can't open clipboard to paste"), g_lpcstrApp, MB_OK | MB_ICONERROR);
	}
}

void CMainFrame::OnEditClear(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
#ifndef _WIN32_WCE
	if(m_bPrintPreview)
		TogglePrintPreview();
#endif // _WIN32_WCE

#ifdef HAVE_CBITMAP
	m_view.SetBitmap(NULL);
#endif
#ifndef WIN32_PLATFORM_PSPC
	UpdateTitleBar(NULL);
#endif // WIN32_PLATFORM_PSPC
	m_szFilePath[0] = 0;
}

void CMainFrame::OnViewZoom(UINT /*uNotifyCode*/, int nID, CWindow /*wnd*/)
{
	switch (nID)
	{
	default:
		m_view.SetZoom(0);
		break;
	case ID_VIEW_ZOOM_IN:
		m_view.ZoomIn2x();
		break;
	case ID_VIEW_ZOOM_OUT:
		m_view.ZoomOut2x();
	}
}

#ifndef _WIN32_WCE
void CMainFrame::OnViewToolBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	static BOOL bNew = TRUE;	// initially visible
	bNew = !bNew;
	UINT uBandID = ATL_IDW_BAND_FIRST + 1;	// toolbar is second added band
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(uBandID);
	rebar.ShowBand(nBandIndex, bNew);
	UISetCheck(ID_VIEW_TOOLBAR, bNew);
	UpdateLayout();
}
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
void CMainFrame::OnViewStatusBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	BOOL bNew = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bNew);
	UpdateLayout();
}
#endif // WIN32_PLATFORM_PSPC

void CMainFrame::OnViewProperties(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CBmpProperties prop;
#ifndef HAVE_CBITMAP
	if(lstrlen(m_szFilePath) > 0)	// we have a file name
		prop.SetFileInfo(m_szFilePath, &m_dib);
	else				// must be clipboard then
#endif
		prop.SetFileInfo(NULL, NULL);
	prop.DoModal();
}

void CMainFrame::OnAppAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CSimpleDialog<IDD_ABOUTBOX> dlg;
	dlg.DoModal();
}
