// props.h : interface for the properties classes
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PROPS_H__
#define __PROPS_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"
#include "Dib.h"

class CFileName : public CWindowImpl<CFileName>
{
public:
	DECLARE_WND_CLASS_EX(NULL, 0, COLOR_3DFACE)

	LPCTSTR m_lpstrFilePath;

#ifndef _WIN32_WCE
	enum { m_nToolTipID = 1313 };
	CToolTipCtrl m_tooltip;
#endif // _WIN32_WCE


	CFileName();
	void Init(HWND hWnd, LPCTSTR lpstrName);

	BEGIN_MSG_MAP(CFileName)
#ifndef _WIN32_WCE
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
#endif // _WIN32_WCE
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

#ifndef _WIN32_WCE
	LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif // _WIN32_WCE

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

class CPageOne : public CPropertyPageImpl<CPageOne>
{
public:
	enum { IDD = IDD_PROP_PAGE1 };

	LPCTSTR m_lpstrFilePath;
	CFileName m_filename;


	CPageOne();

	BEGIN_MSG_MAP(CPageOne)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageOne>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

class CPageTwo : public CPropertyPageImpl<CPageTwo>
{
public:
	enum { IDD = IDD_PROP_PAGE2 };

	CDib* pDIB;

	CPageTwo();

	BEGIN_MSG_MAP(CPageTwo)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageTwo>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

class CPageThree : public CPropertyPageImpl<CPageThree>
{
public:
	enum { IDD = IDD_PROP_PAGE3 };

	CDib* pDIB;

	CPageThree();
	BEGIN_MSG_MAP(CPageThree)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageThree>)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CBmpProperties : public CPropertySheetImpl<CBmpProperties>
{
public:
	CPageOne m_page1;
	CPageTwo m_page2;
	CPageThree m_page3;


	CBmpProperties();
	void SetFileInfo(LPCTSTR lpstrFilePath, CDib* pDIB);

	BEGIN_MSG_MAP(CBmpProperties)
		CHAIN_MSG_MAP(CPropertySheetImpl<CBmpProperties>)
	END_MSG_MAP()
};

#endif // __PROPS_H__
