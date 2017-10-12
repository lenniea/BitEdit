// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "list.h"
#include "view.h"
#include "resource.h"
#include "Dib.h"

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
#ifndef _WIN32_WCE
		, public CPrintJobInfo
#endif // _WIN32_WCE
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

#ifndef _WIN32_WCE
	CCommandBarCtrl m_CmdBar;
	CRecentDocumentList m_mru;
	CMruList m_list;
	CStatusBarCtrl m_statbar;
#endif // _WIN32_WCE
	CBitmapView m_view;
	CDib m_dib;

	TCHAR m_szFilePath[MAX_PATH];

	// printing support
#ifndef _WIN32_WCE
	CPrinter m_printer;
	CDevMode m_devmode;
	CPrintPreviewWindow m_wndPreview;
	CEnhMetaFile m_enhmetafile;
	RECT m_rcMargin;
	bool m_bPrintPreview;
#endif // _WIN32_WCE

	void AppError(UINT id, int error);
	BOOL QueryClose();
	void OnNewDocument();
	int DoFileOpen(const char* filename);
	int DoFileSave(const char* filename);
	int DoFileSaveAs();

	CMainFrame();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
#ifndef _WIN32_WCE
	void TogglePrintPreview();
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
	void UpdateTitleBar(LPCTSTR lpstrTitle);
#endif // WIN32_PLATFORM_PSPC

#ifndef _WIN32_WCE
	//print job info callbacks
	virtual bool IsValidPage(UINT nPage);
	virtual bool PrintPage(UINT nPage, HDC hDC);
#endif // _WIN32_WCE

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
#ifndef _WIN32_WCE
		MSG_WM_CONTEXTMENU(OnContextMenu)
#endif // _WIN32_WCE
		MSG_WM_SIZE(OnSize)
		MSG_WM_DROPFILES(OnDropFiles);

		COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER_EX(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
#ifndef _WIN32_WCE
		COMMAND_RANGE_HANDLER_EX(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)
		COMMAND_ID_HANDLER_EX(ID_RECENT_BTN, OnRecentButton)
#endif // _WIN32_WCE
#ifndef _WIN32_WCE
		COMMAND_ID_HANDLER_EX(ID_FILE_PRINT, OnFilePrint);
		COMMAND_ID_HANDLER_EX(ID_FILE_PAGE_SETUP, OnFilePageSetup)
		COMMAND_ID_HANDLER_EX(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview);
#endif // _WIN32_WCE
		COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER_EX(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER_EX(ID_EDIT_CLEAR, OnEditClear)
		COMMAND_ID_HANDLER_EX(ID_VIEW_ZOOM_IN, OnViewZoom)
		COMMAND_ID_HANDLER_EX(ID_VIEW_ZOOM_OUT, OnViewZoom)
#ifndef _WIN32_WCE
		COMMAND_ID_HANDLER_EX(ID_VIEW_TOOLBAR, OnViewToolBar)
#endif // _WIN32_WCE
#ifndef WIN32_PLATFORM_PSPC
		COMMAND_ID_HANDLER_EX(ID_VIEW_STATUS_BAR, OnViewStatusBar)
#endif // WIN32_PLATFORM_PSPC
		COMMAND_ID_HANDLER_EX(ID_VIEW_PROPERTIES, OnViewProperties)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
#ifndef _WIN32_WCE
		UPDATE_ELEMENT(ID_FILE_PRINT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FILE_PRINT_PREVIEW, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
#endif // _WIN32_WCE
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CLEAR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
#ifndef _WIN32_WCE
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
#endif // _WIN32_WCE
#ifndef WIN32_PLATFORM_PSPC
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
#endif // WIN32_PLATFORM_PSPC
		UPDATE_ELEMENT(ID_VIEW_PROPERTIES, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_RECENT_BTN, UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/);
	void OnSize(UINT nFlags, CSize size);
	LRESULT OnDropFiles(HDROP hDropFiles);

#ifndef _WIN32_WCE
	void OnContextMenu(CWindow wnd, CPoint point);
#endif // _WIN32_WCE

	void OnFileExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnFileOpen(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnFileSave(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnFileSaveAs(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);

#ifndef _WIN32_WCE
	void OnFileRecent(UINT /*uNotifyCode*/, int nID, CWindow /*wnd*/);
	void OnRecentButton(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
#endif // _WIN32_WCE

#ifndef _WIN32_WCE
	void OnFilePrint(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnFilePageSetup(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnFilePrintPreview(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
#endif // _WIN32_WCE

	void OnEditCopy(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnEditPaste(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnEditClear(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);

	void OnViewZoom(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
#ifndef _WIN32_WCE
	void OnViewToolBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
#endif // _WIN32_WCE

#ifndef WIN32_PLATFORM_PSPC
	void OnViewStatusBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
#endif // WIN32_PLATFORM_PSPC

	void OnViewProperties(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnAppAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
};

#endif // __MAINFRM_H__
