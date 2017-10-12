// BmpFile.cpp : implementation of the CBmpFile class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BmpFile.h"

CBmpFile::CBmpFile()
{
	memset(&m_bfi, 0, sizeof(m_bfi));
	memset(&m_bmi, 0, sizeof(m_bmi));
	m_hBitmap = NULL;
	m_pBits = NULL;
}

CBmpFile::~CBmpFile()
{
	DeleteObject();
}

BOOL CBmpFile::IsNull()
{
	return (m_bfi.bfType != 'BM') || (m_bmi.biSize == 0);
}

BOOL CBmpFile::DeleteObject()
{
	BOOL bResult = true;
	if (m_hBitmap)
	{
		bResult = ::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_pBits != NULL)
	{
		free(m_pBits);
	}
	return bResult;
}

BOOL CBmpFile::GetSize(SIZE& size)
{
	if (!IsNull())
	{
		size.cx = m_bmi.biWidth;
		size.cy = m_bmi.biHeight;
		return true;
	}
	size.cx = size.cy = 0;
	return false;
}

int CBmpFile::LoadFile(const char* szFilename)
{
	int result = -1;
	FILE* fp = fopen(szFilename, "rb");
	if (fp != NULL)
	{
		/* Determine length of file */
		fseek(fp, 0, SEEK_END);
		size_t length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Read BITMAPFILEHEADER
		size_t bytes = fread(&m_bfi, 1, sizeof(m_bfi), fp);
		if (bytes == sizeof(m_bfi))
		{
			// Read BITMAPINFOHEADER
			bytes = fread(&m_bmi, 1, sizeof(m_bmi), fp);
			if (bytes == sizeof(m_bmi))
			{
				// Read in Palette
				int nColors = m_bmi.biClrUsed;
				bytes = fread(&m_pal, sizeof(RGBQUAD), nColors, fp) * sizeof(RGBQUAD);

				size_t sizeImage = m_bmi.biSizeImage;
				if (sizeImage == 0)
				{
					int bytes = (m_bmi.biBitCount + 7) >> 3;
					int rowWidth = (m_bmi.biWidth + 3) & ~3;
					sizeImage = rowWidth * m_bmi.biHeight * bytes;
				}

				m_pBits = (LPBYTE) malloc(sizeImage);

				bytes = fread(m_pBits, 1, sizeImage, fp);

				result = 0;
			}
			else
			{
				result = -3;
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
