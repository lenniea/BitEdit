// Dib.cpp : implementation of the CDib class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <assert.h>
#include "Dib.h"
#include "resource.h"

void ColorRect(HDC hDC, LPCRECT pRect, COLORREF rgb)
{
    COLORREF rgbOld = ::SetBkColor(hDC, rgb);
    BOOL result = ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    ::SetBkColor(hDC, rgbOld);
}

CDib::CDib()
{
	memset(&m_bmi, 0, sizeof(m_bmi));
	m_hBitmap = NULL;
	m_pBits = NULL;
	m_bModified = FALSE;
}

CDib::~CDib()
{
	DeleteObject();
}

BOOL CDib::IsNull()
{
	return (m_hBitmap == NULL && m_bmi.bmi.biSize == 0);
}

BOOL CDib::SetBitmap(HBITMAP hBitmap)
{
	BOOL bResult = DeleteObject();
	m_hBitmap = hBitmap;
	return bResult;
}

BOOL CDib::DeleteObject()
{
	BOOL bResult = true;
	if (m_hBitmap)
	{
		bResult = ::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_pBits != NULL)
	{
		::GlobalFree(m_pBits);
		m_pBits = NULL;
	}
	return bResult;
}

BOOL CDib::GetSize(SIZE& size)
{
	if (!IsNull())
	{
		size.cx = GetWidth();
		size.cy = GetHeight();
		return true;
	}
	size.cx = size.cy = 0;
	return false;
}

#define DWORD_ALIGN(x)		(((x) + 3) & ~3)

int CDib::GetRowBytes()
{
	int rowBytes = 0;
	int width = GetWidth();

	switch (GetDepth())
	{
	case 1:
		rowBytes = DWORD_ALIGN((width + 7) >> 3);
		break;
	case 4:
		rowBytes = DWORD_ALIGN((width + 1) >> 1);
		break;
	case 8:
		rowBytes = DWORD_ALIGN(width);
		break;
	case 16:
		rowBytes = DWORD_ALIGN(width * 2);
		break;
	case 24:
		rowBytes = ((width * 3) + 3) & ~3;
		break;
	case 32:
		rowBytes = (width * 4);
	default:
		;
	}
	return rowBytes;
}


int CDib::GetColors()
{
	int colors = 0;
	int bits = GetDepth();
	int size = m_bmi.bmi.biSize;
	if (size == sizeof(BITMAPINFOHEADER))
	{
		colors = m_bmi.bmi.biClrUsed;
	}
	switch (bits)
	{
    case 16:
        if (m_bmi.bmi.biCompression == BI_BITFIELDS)
            colors = 3;
        break;
	case 1:
	case 4:
	case 8:
		colors = 1 << bits;
	default:
		;
	}
	return colors;
}

int CDib::GetWidth()
{
	DIBSECTION ds;
	int bytes = ::GetObject(m_hBitmap, sizeof(DIBSECTION), &ds);

	if (bytes != 0)
	{
		return ds.dsBm.bmWidth;
	}

	switch (m_bmi.bmi.biSize)
	{
	case sizeof(BITMAPV4HEADER):
		return m_bmi4.bmi.bV4Width;
	case sizeof(BITMAPINFOHEADER):
		return m_bmi.bmi.biWidth;
	}
	return 1;
}

int CDib::GetHeight()
{
	DIBSECTION ds;

	if (m_hBitmap != NULL && ::GetObject(m_hBitmap, sizeof(DIBSECTION), &ds))
	{
		return ds.dsBm.bmHeight;
	}
	switch (m_bmi.bmi.biSize)
	{
	case sizeof(BITMAPV4HEADER):
		return m_bmi4.bmi.bV4Height;
	case sizeof(BITMAPINFOHEADER):
		return m_bmi.bmi.biHeight;
	}
	return 1;
}

size_t CDib::GetSizeImage()
{
	size_t bytes = 0;

	switch (m_bmi.bmi.biSize)
	{
	case sizeof(BITMAPV4HEADER):
		bytes =  m_bmi4.bmi.bV4SizeImage;
		break;
	default:
		bytes = m_bmi.bmi.biSizeImage;
	}
	if (bytes == 0)
	{
		bytes = GetRowBytes() * GetHeight();
	}
	return bytes;
}

int CDib::GetDepth()
{
	switch (m_bmi.bmi.biSize)
	{
	case sizeof(BITMAPV4HEADER):
		return m_bmi4.bmi.bV4BitCount;
	default:
		return m_bmi.bmi.biBitCount;
	}
	return 0;
}

RGBQUAD* CDib::GetPalette()
{
	switch (m_bmi.bmi.biSize)
	{
	case sizeof(BITMAPV4HEADER):
		return m_bmi4.pal;
	default:
		return m_bmi.pal;
	}
	return NULL;
}

int CDib::LoadBMP(const char* szFilename)
{
	int result = -1;
	FILE* fp = fopen(szFilename, "rb");
	if (fp != NULL)
	{
		/* Determine length of file */
		fseek(fp, 0, SEEK_END);
		size_t nFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Read BITMAPFILEHEADER
		BITMAPFILEHEADER bmfHeader;
		size_t bytes = fread(&bmfHeader, 1, sizeof(bmfHeader), fp);
		if (bytes == sizeof(bmfHeader) && bmfHeader.bfType == 'B' + ('M' << 8))
		{
            size_t nOffset = bytes;

			memset(&m_bmi, 0, sizeof(m_bmi));

			// Read the BITMAPINFO
			bytes = fread(&m_bmi, 1, sizeof(BITMAPINFOHEADER), fp);

            if (m_bmi.bmi.biSize == sizeof(BITMAPINFOHEADER))
            {
                nOffset += bytes;
                int nColors = GetColors();

                if (GetDepth() >= 16 && nColors == 0 && m_bmi.bmi.biCompression == BI_RGB)
                {
                    const RGBQUAD maskRed =   { 255, 0, 0, 0 };
                    const RGBQUAD maskGreen = { 0, 255, 0, 0 };
                    const RGBQUAD maskBlue =  { 0, 0, 255, 0 };
                    m_bmi.pal[0] = maskRed;
                    m_bmi.pal[1] = maskGreen;
                    m_bmi.pal[2] = maskBlue;
                    m_bmi.bmi.biClrUsed = 3;
                }
                else
                {
                    // Read in the color palette
                    nOffset += fread(&m_bmi.pal, 1, sizeof(RGBQUAD) * nColors, fp);
                }
            }
            else
            {
                nOffset += m_bmi.bmi.biSize;
                fseek(fp, nOffset, SEEK_SET);
            }

			// Try to create HBITMAP (for the clipboard)
			m_hBitmap = CreateDIBSection( NULL, (LPBITMAPINFO) &m_bmi.bmi, DIB_RGB_COLORS, (LPVOID*) &m_pBits, 
								NULL, 0L);
			
			DWORD dwSize = nFileLen - nOffset;

			if (m_pBits == NULL)
			{
				m_pBits = (LPBYTE) ::GlobalAlloc(GMEM_FIXED, dwSize);
			}
			
			// Finally read in the bitmap bits
            bytes = fread(m_pBits, 1, dwSize, fp);
			if (bytes == dwSize)
			{
				result = 0;
			}
		}
		else
		{
			result = -2;
		}
		fclose(fp);
	}
	return result;
}

//
//  Silicon Graphics RGB file header
//  (Multi-byte fields in Network Byte Order
//
typedef struct rgb_header
{
    uint16_t MAGIC;         // IRIS image file magic number
    uint8_t  STORAGE;       // Storage format
    uint8_t  BPC;           // Number of bytes per pixel channel 
    uint16_t DIMENSION;     // Number of dimensions
    uint16_t XSIZE;         // X size in pixels 
    uint16_t YSIZE;         // Y size in pixels 
    uint16_t ZSIZE;         // Number of channels
    uint32_t PIXMIN;        // Minimum pixel value
    uint32_t PIXMAX;        // Maximum pixel value
    uint32_t DUMMY;         // Ignored
    uint8_t IMAGENAME[80];  // Image name
    uint32_t COLORMAP;      // Colormap ID
    uint8_t DUMMY2[404];    // Ignored
} RGB_HEADER;

#define RGB_MAGIC   474

#define COLORMAP_NORMAL     0
#define COLORMAP_DITHERED   1
#define COLORMAP_SCREEN     2
#define COLORMAP_SGI        3

void expand_row(uint8_t* optr, uint8_t* iptr, uint32_t depth, uint32_t length)
{
    uint8_t pixel, count;
    
    while(length > 0) {
        --length;
        pixel = *iptr++;
        count = pixel & 0x7f;
        if ( count == 0 )
            break;
        if (pixel & 0x80) {
            while(count-- && length > 0) {
                --length;
                *optr = *iptr++;
                optr += depth;
            }
        } else {
            --length;
            pixel = *iptr++;
            while(count--) {
                *optr = pixel;
                optr += depth;
            }
        }
    }
}

// Factorization to byte reverse SGI offset/length table
void flip_table(uint32_t* table, int count)
{
    while (count > 0)
    {
        *table++ = htonl(*table);
        --count;
    }
}

// Load Silicon Graphics (RGB) file into DIB
int CDib::LoadRGB(const char* szFilename)
{
	int result = -1;
	FILE* fp = fopen(szFilename, "rb");
	if (fp != NULL)
	{
		/* Determine length of file */
		fseek(fp, 0, SEEK_END);
		size_t nFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Read BITMAPFILEHEADER
		RGB_HEADER rgbHeader;
		size_t bytes = fread(&rgbHeader, 1, sizeof(rgbHeader), fp);
		if (bytes == sizeof(rgbHeader) && htons(rgbHeader.MAGIC) == RGB_MAGIC)
		{
            size_t nOffset = bytes;
			memset(&m_bmi, 0, sizeof(m_bmi));
            uint16_t width = htons(rgbHeader.XSIZE);
            uint16_t height = htons(rgbHeader.YSIZE);
            uint16_t depth = htons(rgbHeader.ZSIZE);

            ATLTRACE("x=%u y=%u z=%d\n", width, height, depth);

            m_bmi.bmi.biSize = sizeof(BITMAPINFOHEADER);
            m_bmi.bmi.biPlanes = 1;
            m_bmi.bmi.biWidth = width;
            m_bmi.bmi.biHeight = height;
            m_bmi.bmi.biBitCount = depth * 8;

            int nColors = GetColors();
            int nDepth = GetDepth();
            if( nDepth > 16)
            {
                const RGBQUAD maskRed =   { 255, 0, 0, 0 };
                const RGBQUAD maskGreen = { 0, 255, 0, 0 };
                const RGBQUAD maskBlue =  { 0, 0, 255, 0 };
                m_bmi.pal[0] = maskRed;
                m_bmi.pal[1] = maskGreen;
                m_bmi.pal[2] = maskBlue;
                if (nDepth > 24)
                {
                    const RGBQUAD maskAlpha =  { 0, 0, 0, 255 };
                    m_bmi.pal[3] = maskAlpha;
                    nColors = 4;
                }
                else
                {
                    nColors = 3;
                }
                m_bmi.bmi.biClrUsed = nColors;
            }


			// Try to create HBITMAP (for the clipboard)
			m_hBitmap = CreateDIBSection( NULL, (LPBITMAPINFO) &m_bmi.bmi, DIB_RGB_COLORS, (LPVOID*) &m_pBits, 
								NULL, 0L);
			
            DWORD dwSize = nFileLen - nOffset;

            uint8_t* buffer = (uint8_t*) malloc(dwSize);

            // Finally read in the offset & length tables and RLE/scan data
            bytes = fread(buffer, 1, dwSize, fp);
			if (bytes == dwSize)
			{
                // Read in offset tables
                uint32_t tablen = height * depth;
                uint32_t* starttab = (uint32_t *) buffer;
                uint32_t* lengthtab = (uint32_t *) (buffer + tablen * sizeof(uint32_t));

                flip_table(starttab, tablen);
                flip_table(lengthtab, tablen);

                uint32_t rowWidth = ((width * depth) + 3) & ~3;
                uint8_t* pOut = m_pBits;
                int i,y,z;
                // Decompress RLE bytes or copy row data to bitmap
                for (y = 0; y < height; ++y)
                {
                    for (z = 0; z < depth; ++z)
                    {
                        int index = y + z * height;
                        uint32_t offset = starttab[index];
                        uint32_t length = lengthtab[index];
                        assert(offset >= sizeof(RGB_HEADER));
                        offset -= sizeof(RGB_HEADER);
                        if (rgbHeader.BPC == 1)
                        {
                            ATLTRACE("index=%u offset=%u length=%u\n", index, offset, length);
                            // Convert RGB to BGR order
                            int flip = (z < 3) ? (2 - z) : 3;
                            expand_row(pOut + flip, buffer + offset, depth, length);
                        }
                        else
                        {
                            ATLTRACE("copy row index=%u offset=%u\n", index, offset);
                            int i;
                            for (i = 0; i < width; ++i)
                            {
                                pOut[i * depth + z] = buffer[offset + i];
                            }
                        }
                    }
                    pOut += rowWidth;
                }
			}

            // Free up memory allocated
            free(buffer);
            result = 0;
		}
		else
		{
			result = -2;
		}
		fclose(fp);
	}
	return result;
}

int CDib::LoadFile(const char* szFilename)
{
    const char* pExt = strrchr(szFilename, '.');
    if (pExt != NULL && stricmp(pExt + 1, "rgb") == 0)
        return LoadRGB(szFilename);
    // Default to loading .BMP file
    return LoadBMP(szFilename);
}

int CDib::SaveFile(const char* szFilename)
{
	int result = 0;
	if (result >= 0)
	{
		m_bModified = FALSE;
	}
	return result;
}

BOOL CDib::IsModified()
{
	return m_bModified;
}

COLORREF word2rgb(int word)
{
	int red = (word >> 10) & 0x1F;
	int grn = (word >> 5) & 0x1F;
	int blu = word & 0x1F;
	return RGB(red << 3, grn << 3, blu << 3);
}

#define ARGB(a,r,g,b)   (RGB(r,g,b) | ((a) << 24))

COLORREF CDib::GetPixelInRow(unsigned int x, LPBYTE pRow, int depth)
{
	COLORREF pixel = INVALID_PIXEL;
	LPBYTE pBits;
	RGBQUAD* pRGB;

	switch (depth)
	{
	case 1:
		pBits = pRow + (x >> 3);
		pixel = (*pBits >> (~x & 7)) & 1;
		pRGB = GetPalette() + pixel;
		pixel =  RGB(pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
		break;

	case 4:
		pBits = pRow + (x >> 1);
		pixel = ((x & 1) ? *pBits : (*pBits >> 4)) & 15;
		pRGB = GetPalette() + pixel;
		pixel =  RGB(pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
		break;
	case 8:
		pBits = pRow + x;
		pixel = *pBits;
		pRGB = GetPalette() + pixel;
		pixel =  RGB(pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
		break;
	case 16:
		pBits = pRow + x * 2;
		pixel = pBits[0] + (pBits[1] << 8);
		pixel = word2rgb(pixel);
		break;
	case 24:
		pBits = pRow + x * 3;
		pixel = RGB(pBits[0], pBits[1], pBits[2]);
		break;
	case 32:
		pBits = pRow + x * 4;
		pixel = ARGB(pBits[3], pBits[2], pBits[1], pBits[0]);

	default:
		;
	}
	return pixel;
}

COLORREF CDib::GetPixel(unsigned int x, unsigned int y)
{
	COLORREF pixel = ~0U;
	unsigned int width = GetWidth();
	unsigned int height = GetHeight();
	int format = m_bmi.bmi.biCompression;

	if (x < width && y < height && (format == BI_RGB || format == BI_BITFIELDS))
	{
		int ydib = height - y - 1;
		if (m_pBits != NULL)
		{
			int rowBytes = GetRowBytes();
			LPBYTE pRow = m_pBits + rowBytes * ydib;

			pixel = GetPixelInRow(x, pRow, GetDepth());
		}
		else if (m_hBitmap != NULL)
		{
			BITMAP bm;
			int bytes = GetObject(m_hBitmap, sizeof(BITMAPINFO), &bm);
			if (bytes)
			{
				BITMAPINFO bmi;
				LPBYTE pRow;
				HDC hDC = CreateCompatibleDC(NULL);

				::ZeroMemory(&bmi, sizeof(bmi));
				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biWidth = bm.bmWidth;
				bmi.bmiHeader.biHeight = bm.bmHeight;
				bmi.bmiHeader.biBitCount = bm.bmBitsPixel;
				bmi.bmiHeader.biPlanes = bm.bmPlanes;
				int rowBytes = DWORD_ALIGN(bm.bmWidthBytes);

				pRow = (LPBYTE) ::GlobalAlloc(GMEM_FIXED, rowBytes);
				BOOL bResult = GetDIBits(hDC, m_hBitmap, ydib, 1, pRow, (LPBITMAPINFO) &bmi, DIB_RGB_COLORS);
				if (bResult)
				{
					pixel = GetPixelInRow(x, pRow, bm.bmBitsPixel);
				}
				::GlobalFree(pRow);
				::DeleteDC(hDC);
			}
		}
	}
	return pixel;
}

BOOL CDib::SetPixel(unsigned int x, unsigned int y, COLORREF rgb)
{
    // TODO: write code to modify bitmap
	m_bModified = TRUE;
	return TRUE;
}

#ifndef AC_SRC_ALPHA
    #define AC_SRC_ALPHA 0x01
#endif


BOOL CDib::Draw(HDC hDC, LPCRECT pRect)
{
	int width = pRect->right - pRect->left;
	int height = pRect->bottom - pRect->top;
	BOOL bResult;
	if (m_hBitmap != NULL)
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC(hDC);
		HBITMAP hBmpOld = dcMem.SelectBitmap(m_hBitmap);
        if (GetDepth() >= 32)
        {
            POINT point;
            POINT brushOrg;
            ::GetViewportOrgEx(hDC, &point);
            ::SetBrushOrgEx(hDC, point.x, point.y, &brushOrg);
            // Fill checkboard pattern beneath transparent bitmap
            HINSTANCE hInst = _Module.GetResourceInstance();
            HBITMAP hChecker = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CHECKER));
            HBRUSH hBrush = ::CreatePatternBrush(hChecker);
            HGDIOBJ hBrushOld = ::SelectObject(hDC, hBrush);
            ::Rectangle(hDC, pRect->left, pRect->top, pRect->right, pRect->bottom);
            // Restore brush origin
            ::SetBrushOrgEx(hDC, brushOrg.x, brushOrg.y, NULL);

            ::DeleteObject(hBrush);
            ::SelectObject(hDC, hBrushOld);
//            ::ColorRect(hDC, pRect, RGB(192,192,192));
            const BLENDFUNCTION blend = { AC_SRC_OVER, /*BlendFlags=*/0, /*SrcConstantAlpha=*/255, AC_SRC_ALPHA };
            bResult = ::AlphaBlend(hDC, pRect->left, pRect->top, width, height, dcMem, 0, 0, GetWidth(), GetHeight(), blend);
        }
        else
        {
		    bResult = ::StretchBlt(hDC, pRect->left, pRect->top, width, height, dcMem, 0, 0, GetWidth(), GetHeight(), SRCCOPY);
        }
		dcMem.SelectBitmap(hBmpOld);
	}
	else
	{
		bResult = ::StretchDIBits(hDC, pRect->left, pRect->top, width, height, 0, 0, GetWidth(), GetHeight(), m_pBits, (LPBITMAPINFO) &m_bmi, DIB_RGB_COLORS, SRCCOPY);
	}
	return bResult;
}
