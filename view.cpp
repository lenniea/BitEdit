// View.cpp : implementation of the CBitmapView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "view.h"

#define CALC_ZOOM(x,z)	(((z) >= 0) ? (x) << (z) : (x) >> -(z))

CBitmapView::CBitmapView()
{
	m_zoom = 0;
}

BOOL CBitmapView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

int CBitmapView::LoadFile(const char* szFileName)
{
	int result = m_pDoc->LoadFile(szFileName);
	if (result >= 0)
	{
		CSize size(m_pDoc->GetWidth(), m_pDoc->GetHeight());

		SetScrollOffset(0, 0, FALSE);
		SetScrollSize(size);
	}
	return result;
}

CPoint CBitmapView::GetCursor(void)
{
	int x = GetScrollPos(SB_HORZ) + m_cursor.x;
	int y = GetScrollPos(SB_VERT) + m_cursor.y;
	return CPoint(CALC_ZOOM(x, -m_zoom), CALC_ZOOM(y, -m_zoom));
}

int CBitmapView::GetZoomPercent(void)
{
	return CALC_ZOOM(100, m_zoom);
}

CSize CBitmapView::GetZoomSize(void)
{
	int width = m_pDoc->GetWidth();
	int height = m_pDoc->GetHeight();
	CSize sizeImage(CALC_ZOOM(width, m_zoom), CALC_ZOOM(height, m_zoom));
	return sizeImage;
}

void CBitmapView::SetZoom(int zoom)
{
	m_zoom = zoom;
	SetScrollSize(GetZoomSize());
	InvalidateRect(NULL, TRUE);
}

#define ZOOM_MIN	-4		/* 6% */
#define ZOOM_MAX	+4		/* 1600% */

void CBitmapView::ZoomOut2x(void)
{
	if (m_zoom > ZOOM_MIN)
	{
		SetZoom(m_zoom - 1);
	}
}

void CBitmapView::ZoomIn2x(void)
{
	if (m_zoom < ZOOM_MAX)
	{
		SetZoom(m_zoom + 1);
	}
}

typedef struct bmi256
{
	BITMAPINFO bmi;
	RGBQUAD pal[255];
} BMI256;

LRESULT CBitmapView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rect;
	GetClientRect(&rect);
	int x = 0;
	int y = 0;
	CSize size = GetZoomSize();
	if(!m_pDoc->IsNull())
	{
		x = size.cx + 1;
		y = size.cy + 1; 
	}
	CDCHandle dc = (HDC)wParam;
	if(rect.right > m_sizeAll.cx)
	{
		RECT rectRight = rect;
		rectRight.left = x;
		rectRight.bottom = y;
		dc.FillRect(&rectRight, COLOR_WINDOW);
	}
	if(rect.bottom > m_sizeAll.cy)
	{
		RECT rectBottom = rect;
		rectBottom.top = y;
		dc.FillRect(&rectBottom, COLOR_WINDOW);
	}
#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)
	if(!m_pDoc->IsNull())
	{
		dc.MoveTo(size.cx, 0);
		dc.LineTo(size.cx, size.cy);
		dc.LineTo(0, size.cy);
	}
#endif //!defined(_WIN32_WCE) || (_WIN32_WCE >= 400)
	return 0;
}

void CBitmapView::DoPaint(CDCHandle dc)
{
	if(!m_pDoc->IsNull())
	{
#ifdef HAVE_CBITMAP
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBmpOld = dcMem.SelectBitmap(m_bmp);
		CSize size = GetZoomSize();

		dc.StretchBlt(0, 0, size.cx, size.cy, dcMem, 0, 0, m_pDoc->GetWidth(), m_pDoc->GetHeight(), SRCCOPY);
		dcMem.SelectBitmap(hBmpOld);
#else
		CSize size = GetZoomSize();
		CRect rect(CPoint(0,0), size);
		m_pDoc->Draw(dc, rect);
#endif
	}
}

int CBitmapView::OnMouseMove(UINT /*flags*/, CPoint point)
{
	m_cursor = point;
	::PostMessage(GetParent(), WM_ENTERIDLE, 0, NULL);
	return 0;
}

int CBitmapView::OnButtonDown(UINT flags, CPoint point)
{
	m_cursor = point;
	CPoint cursor = GetCursor();
	if (flags & MK_LBUTTON)
		m_pDoc->SetPixel(cursor.x, cursor.y, RGB(0,0,0));
	else if (flags & MK_RBUTTON)
		m_pDoc->SetPixel(cursor.x, cursor.y, RGB(255,255,255));
	return 0;
}

