// props.cpp : implementation for the properties classes
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "props.h"

// not defined in original the VC6 headers
#ifndef BI_JPEG
#define BI_JPEG       4L
#endif
#ifndef BI_PNG
#define BI_PNG        5L
#endif

CFileName::CFileName() : m_lpstrFilePath(NULL)
{ }

void CFileName::Init(HWND hWnd, LPCTSTR lpstrName)
{
	ATLASSERT(::IsWindow(hWnd));
	SubclassWindow(hWnd);

#ifndef _WIN32_WCE
	// Set tooltip
	m_tooltip.Create(m_hWnd);
	ATLASSERT(m_tooltip.IsWindow());
	RECT rect;
	GetClientRect(&rect);
	CToolInfo ti(0, m_hWnd, m_nToolTipID, &rect, NULL);
	m_tooltip.AddTool(&ti);

	// set text
	m_lpstrFilePath = lpstrName;
	if(m_lpstrFilePath == NULL)
		return;

	CClientDC dc(m_hWnd);	// will not really paint
	HFONT hFontOld = dc.SelectFont(AtlGetDefaultGuiFont());

	RECT rcText = rect;
	dc.DrawText(m_lpstrFilePath, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);
	BOOL bTooLong = (rcText.right > rect.right);
	if(bTooLong)
		m_tooltip.UpdateTipText(m_lpstrFilePath, m_hWnd, m_nToolTipID);
	m_tooltip.Activate(bTooLong);

	dc.SelectFont(hFontOld);
#endif // _WIN32_WCE

	Invalidate();
	UpdateWindow();
}

#ifndef _WIN32_WCE
LRESULT CFileName::OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_tooltip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		m_tooltip.RelayEvent(&msg);
	}
	bHandled = FALSE;
	return 1;
}
#endif // _WIN32_WCE

LRESULT CFileName::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	if(m_lpstrFilePath != NULL)
	{
		RECT rect;
		GetClientRect(&rect);

		dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		dc.SetBkMode(TRANSPARENT);
		HFONT hFontOld = dc.SelectFont(AtlGetDefaultGuiFont());

#ifndef _WIN32_WCE
		dc.DrawText(m_lpstrFilePath, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_PATH_ELLIPSIS);
#else // _WIN32_WCE
		dc.DrawText(m_lpstrFilePath, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);
#endif // _WIN32_WCE

		dc.SelectFont(hFontOld);
	}
	return 0;
}

CPageOne::CPageOne() : m_lpstrFilePath(NULL)
{ }

LRESULT CPageOne::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(m_lpstrFilePath != NULL)	// get and set file properties
	{
		m_filename.Init(GetDlgItem(IDC_FILELOCATION), m_lpstrFilePath);

		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(m_lpstrFilePath, &fd);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			int nSizeK = (int)(fd.nFileSizeLow / 1024);	// assume it not bigger than 2GB
			if(nSizeK == 0 && fd.nFileSizeLow != 0)
				nSizeK = 1;
			TCHAR szBuff[100];
			wsprintf(szBuff, _T("%i KB"), nSizeK);
			SetDlgItemText(IDC_FILESIZE, szBuff);
			SYSTEMTIME st;
			::FileTimeToSystemTime(&fd.ftCreationTime, &st);
			::GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, _T("dddd, MMMM dd',' yyyy',' "), szBuff, sizeof(szBuff) / sizeof(szBuff[0]));
			TCHAR szBuff1[50];
			::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, _T("hh':'mm':'ss tt"), szBuff1, sizeof(szBuff1) / sizeof(szBuff1[0]));
			lstrcat(szBuff, szBuff1);
			SetDlgItemText(IDC_FILEDATE, szBuff);

			szBuff[0] = 0;
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0)
				lstrcat(szBuff, _T("Archive, "));
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
				lstrcat(szBuff, _T("Read-only, "));
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)
				lstrcat(szBuff, _T("Hidden, "));
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0)
				lstrcat(szBuff, _T("System"));
			int nLen = lstrlen(szBuff);
			if(nLen >= 2 && szBuff[nLen - 2] == _T(','))
				szBuff[nLen - 2] = 0;
			SetDlgItemText(IDC_FILEATTRIB, szBuff);

			::FindClose(hFind);
		}
	}
	else
	{
		SetDlgItemText(IDC_FILELOCATION, _T("(Clipboard)"));
		SetDlgItemText(IDC_FILESIZE, _T("N/A"));
		SetDlgItemText(IDC_FILEDATE, _T("N/A"));
		SetDlgItemText(IDC_FILEATTRIB, _T("N/A"));
	}

	return TRUE;
}

CPageTwo::CPageTwo() : pDIB(NULL)
{ }

LRESULT CPageTwo::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Special - remove unused buttons, move Close button, center wizard
	CPropertySheetWindow sheet = GetPropertySheet();
#if !defined(_AYGSHELL_H_) && !defined(__AYGSHELL_H__) // PPC specific
	sheet.CancelToClose();
	RECT rect;
	CButton btnCancel = sheet.GetDlgItem(IDCANCEL);
	btnCancel.GetWindowRect(&rect);
	sheet.ScreenToClient(&rect);
	btnCancel.ShowWindow(SW_HIDE);
	CButton btnClose = sheet.GetDlgItem(IDOK);
	btnClose.SetWindowPos(NULL, &rect, SWP_NOZORDER | SWP_NOSIZE);
	sheet.CenterWindow(GetPropertySheet().GetParent());

	sheet.ModifyStyleEx(WS_EX_CONTEXTHELP, 0);
#endif // (_AYGSHELL_H_) || defined(__AYGSHELL_H__) // PPC specific

	// get and display bitmap prperties
	SetDlgItemText(IDC_TYPE, _T("Windows 3.x Bitmap (BMP)"));
	LPTSTR lpstrCompression = _T("Uncompressed");;

	if (pDIB != NULL)
	{
		SetDlgItemInt(IDC_WIDTH, pDIB->GetWidth());
		SetDlgItemInt(IDC_HEIGHT, pDIB->GetHeight());
		SetDlgItemInt(IDC_HORRES, ::MulDiv(pDIB->m_bmi.bmi.biXPelsPerMeter, 254, 10000));
		SetDlgItemInt(IDC_VERTRES, ::MulDiv(pDIB->m_bmi.bmi.biYPelsPerMeter, 254, 10000));
		SetDlgItemInt(IDC_BITDEPTH, pDIB->GetDepth());

		switch(pDIB->m_bmi.bmi.biCompression)
		{
#ifndef _WIN32_WCE
		case BI_RLE4:
		case BI_RLE8:
			lpstrCompression = _T("RLE");
			break;
#endif // _WIN32_WCE
		case BI_BITFIELDS:
			lpstrCompression = _T("Uncompressed with bitfields");
			break;
		case BI_JPEG:
		case BI_PNG:
			lpstrCompression = _T("Unknown");
			break;
		}
		SetDlgItemText(IDC_COMPRESSION, lpstrCompression);
	}
	return TRUE;
}

CPageThree::CPageThree() : pDIB(NULL)
{
}

LRESULT CPageThree::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CListBox list = GetDlgItem(IDC_PALETTE);
	for (int i = 0; i < 256; ++i)
	{
		TCHAR szText[80];
		wsprintf(szText, "%3u", i);
		list.AddString(szText);
	}
	return TRUE;
}

void OutlineRect(HDC hDC, LPRECT pRect, COLORREF rgb)
{
    RECT rect;
    COLORREF rgbOld = ::SetBkColor(hDC, rgb);
    rect.left = pRect->left;
    rect.right = pRect->right;
    rect.top = pRect->top;
    rect.bottom = rect.top + 1;
    // Draw top
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    // Draw bottom
    rect.bottom = pRect->bottom;
    rect.top = rect.bottom - 1;
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    rect.top = pRect->top;
    rect.bottom = pRect->bottom;
    // Draw left
    rect.left = pRect->left;
    rect.right = rect.left + 1;
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    // Draw right
    rect.right = pRect->right;
    rect.left = rect.right - 1;
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    ::SetBkColor(hDC, rgbOld);
}

COLORREF quad2rgb(LPRGBQUAD pRGB)
{
	return RGB(pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
}

#define RC_TO_COLOR(r,c)    (((c) << 4) + ((r) & 15))
#define INDEX_TO_COLOR(x)	(RC_TO_COLOR(((x) >> 4), ((x) & 15))))

LRESULT CPageThree::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
    HDC hDC = lpDIS->hDC;
    CRect rect = lpDIS->rcItem;
    int id = lpDIS->itemID;
	COLORREF rgb;

    switch (lpDIS->CtlID)
    {
    case IDC_PALETTE:
        if (lpDIS->itemState & ODS_SELECTED)
        {
            OutlineRect(hDC, rect, RGB(0,0,255));
            rect.InflateRect(-1,-1);
        }
		rgb = (pDIB != NULL) ? quad2rgb(pDIB->GetPalette() + INDEX_TO_COLOR(id) : RGB(0,0,0);
		ColorRect(hDC, rect, rgb);
        break;
    default:
        ;
    }
	return 0;
}

LRESULT CPageThree::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPMEASUREITEMSTRUCT lpMIS = (LPMEASUREITEMSTRUCT) lParam;
    int id = lpMIS->CtlID;
    CWindow wnd = GetDlgItem(id);
    CRect rect;
    wnd.GetClientRect(rect);

    lpMIS->itemHeight = rect.Height() / 16;
    lpMIS->itemWidth = rect.Width() / 16;
	return 0;
}

CBmpProperties::CBmpProperties()
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	AddPage(m_page1);
	AddPage(m_page2);
	AddPage(m_page3);
	SetActivePage(1);
	SetTitle(_T("Bitmap Properties"));
}

void CBmpProperties::SetFileInfo(LPCTSTR lpstrFilePath, CDib* pDIB)
{
	m_page1.m_lpstrFilePath = lpstrFilePath;
	m_page2.pDIB = pDIB;
	m_page3.pDIB = pDIB;
}
