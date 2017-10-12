// View.h : interface of the CBitmapView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VIEW_H__
#define __VIEW_H__

//#define HAVE_CBITMAP

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Dib.h"

#include <atlcrack.h>

#define INVALID_PIXEL		(~0UL)

class CBitmapView : public CScrollWindowImpl<CBitmapView>
{
protected:
	int m_zoom;
	CPoint m_cursor;
public:
#ifndef _WIN32_WCE
	DECLARE_WND_CLASS_EX(NULL, 0, -1)
#else // _WIN32_WCE
	DECLARE_WND_CLASS(NULL)
#endif // _WIN32_WCE

	CDib* m_pDoc;

	CBitmapView();

	BOOL PreTranslateMessage(MSG* pMsg);

	int LoadFile(const char* szFilename);

	int GetZoomPercent(void);
	CPoint GetCursor(void);
	CSize GetZoomSize(void);
	void SetZoom(int zoom);
	void ZoomIn2x(void);
	void ZoomOut2x(void);

	BEGIN_MSG_MAP_EX(CBitmapView)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnButtonDown)
		MSG_WM_RBUTTONDOWN(OnButtonDown)
		CHAIN_MSG_MAP(CScrollWindowImpl<CBitmapView>);
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	int OnMouseMove(UINT flags, CPoint point);
	int OnButtonDown(UINT flags, CPoint point);
	void DoPaint(CDCHandle dc);
};

#endif // __VIEW_H__
