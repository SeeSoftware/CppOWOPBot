#include "ScreenCaptureHelper.h"

sf::Image ScreenCaptureHelper::hwndToImage(HWND hwnd, const sf::Vector2i imageSize)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height = imageSize.y, width = imageSize.x, srcheight, srcwidth;
	HBITMAP hbwindow;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, HALFTONE);//COLORONCOLOR);

	sf::Vector2i windowSize = getWindowSize(hwnd);

	srcheight = windowSize.y;
	srcwidth = windowSize.x;


	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	
	uint8_t* buffer = new uint8_t[(size_t)imageSize.x * (size_t)imageSize.y * 4];
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	for (size_t i = 0; i < (size_t)imageSize.x * (size_t)imageSize.y * 4; i += 4) //fix pixels
	{
		uint8_t tmp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i+2] = tmp;
		buffer[i + 3] = 255;
	}
		

	sf::Image src;
	src.create(imageSize.x, imageSize.y, (sf::Uint8*)buffer);
	
	// avoid memory leak
	delete[] buffer;

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}

sf::Vector2i ScreenCaptureHelper::getWindowSize(HWND hwnd)
{
	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);
	return sf::Vector2i(windowsize.right, windowsize.bottom);
}
