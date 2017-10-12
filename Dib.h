#ifndef __DIB_H__
#define __DIB_H__

#ifdef HAVE_STDTYPES
#include <stdtypes.h>
#else
typedef unsigned char uint8_t;
typedef signed char sint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef signed short sint16_t;
typedef short int16_t;
typedef unsigned long uint32_t;
typedef signed long sint32_t;
typedef long int32_t;
#endif

typedef struct
{
	BITMAPINFOHEADER bmi;
	RGBQUAD pal[256];
} BMIV3;

typedef struct
{
	BITMAPV4HEADER bmi;
	RGBQUAD pal[256];
} BMIV4;

#define INVALID_PIXEL		(~0UL)

class CDib
{
protected:
	BOOL m_bModified;
public:
	union {
		BMIV3 m_bmi;
		BMIV4 m_bmi4;
	};
	LPBYTE m_pBits;
public:
	// Constructor/destructor
	CDib();
	~CDib();

	operator HBITMAP() const { return m_hBitmap; }

	HBITMAP m_hBitmap;

	BOOL IsNull();
	BOOL DeleteObject();
	BOOL SetBitmap(HBITMAP hBitmap);
	BOOL GetSize(SIZE& size);
	int GetDepth();
	int GetWidth();
	int GetHeight();
	size_t GetSizeImage();
	int GetRowBytes();
	int GetColors();
	RGBQUAD *GetPalette();
	COLORREF GetPixelInRow(unsigned int x, LPBYTE pRow, int depth);
	COLORREF GetPixel(unsigned int x, unsigned int y);
	BOOL SetPixel(unsigned int x, unsigned int y, COLORREF rgb);

	BOOL Draw(HDC hDC, LPCRECT pRect);

	int LoadBMP(const char* szFilename);
	int LoadRGB(const char* szFilename);
	int LoadFile(const char* szFilename);
	int SaveFile(const char* szFilename);

	BOOL IsModified();
};

void ColorRect(HDC hDC, LPCRECT pRect, COLORREF rgb);

#endif /* __DIB_H__ */
