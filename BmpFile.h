#ifndef __BMPFILE__
#define __BMPFILE__

class CBmpFile
{
protected:
	BITMAPFILEHEADER m_bfi;
	BITMAPINFOHEADER m_bmi;
	RGBQUAD m_pal[256];
	LPBYTE m_pBits;
public:
	// Constructor/destructor
	CBmpFile();
	~CBmpFile();

	operator HBITMAP() const { return m_hBitmap; }

	HBITMAP m_hBitmap;

	BOOL IsNull();
	BOOL DeleteObject();
	BOOL GetSize(SIZE& size);

	int LoadFile(const char* szFilename);
};

#endif /* __BMPFILE__ */
