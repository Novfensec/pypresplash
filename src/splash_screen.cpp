#include "splash_screen.h"
#include <gdiplus.h>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace Gdiplus;

static SplashScreen *g_pSplash = nullptr;

SplashScreen::SplashScreen()
{
    SetProcessDPIAware();
    m_hInstance = GetModuleHandle(nullptr);
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
}

SplashScreen::~SplashScreen()
{
    Hide();
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
    GdiplusShutdown(m_gdiplusToken);
}

bool SplashScreen::CreateWindowExW()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"SplashScreenClass";
    wc.hCursor = LoadCursor(nullptr, IDC_WAIT);

    RegisterClass(&wc);

    DWORD dwExStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    DWORD dwStyle = WS_POPUP;

    m_hWnd = ::CreateWindowEx(
        dwExStyle,
        L"SplashScreenClass",
        L"Splash",
        dwStyle,
        0, 0,
        m_width, m_height,
        nullptr, nullptr, m_hInstance, nullptr);

    if (!m_hWnd)
        return false;

    HRGN hRgn = CreateRoundRectRgn(0, 0, m_width, m_height, 0, 0);
    SetWindowRgn(m_hWnd, hRgn, TRUE);

    CenterWindow();
    g_pSplash = this;
    return true;
}

void SplashScreen::CenterWindow()
{
    RECT rcWork;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

    int screenWidth = rcWork.right - rcWork.left;
    int screenHeight = rcWork.bottom - rcWork.top;

    int x = (screenWidth - m_width) / 2 + rcWork.left;
    int y = (screenHeight - m_height) / 2 + rcWork.top;

    SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, m_width, m_height, SWP_NOZORDER);
}

bool SplashScreen::LoadSplashImage(const std::wstring &imagePath, int targetWidth, int targetHeight, const std::string &scaleMode)
{
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }

    Gdiplus::Bitmap *pBitmap = new Gdiplus::Bitmap(imagePath.c_str());
    if (pBitmap->GetLastStatus() != Gdiplus::Ok)
    {
        delete pBitmap;
        return false;
    }

    if (targetWidth > 0 && targetHeight > 0)
    {
        m_width = targetWidth;
        m_height = targetHeight;

        Gdiplus::Bitmap *pScaled = new Gdiplus::Bitmap(m_width, m_height, PixelFormat32bppARGB);
        Gdiplus::Graphics graphics(pScaled);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        float srcWidth = (float)pBitmap->GetWidth();
        float srcHeight = (float)pBitmap->GetHeight();

        float scaleX = (float)targetWidth / srcWidth;
        float scaleY = (float)targetHeight / srcHeight;

        float drawWidth = (float)targetWidth;
        float drawHeight = (float)targetHeight;
        float drawX = 0.0f;
        float drawY = 0.0f;

        if (scaleMode == "contain")
        {
            float scale = std::min(scaleX, scaleY);
            drawWidth = srcWidth * scale;
            drawHeight = srcHeight * scale;
            drawX = (targetWidth - drawWidth) / 2.0f;
            drawY = (targetHeight - drawHeight) / 2.0f;
        }
        else if (scaleMode == "cover")
        {
            float scale = std::max(scaleX, scaleY);
            drawWidth = srcWidth * scale;
            drawHeight = srcHeight * scale;
            drawX = (targetWidth - drawWidth) / 2.0f;
            drawY = (targetHeight - drawHeight) / 2.0f;
        }

        graphics.DrawImage(pBitmap, drawX, drawY, drawWidth, drawHeight);

        pScaled->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &m_hBitmap);
        delete pScaled;
    }
    else
    {
        m_width = pBitmap->GetWidth();
        m_height = pBitmap->GetHeight();
        pBitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &m_hBitmap);
    }

    delete pBitmap;
    return m_hBitmap != nullptr;
}
void SplashScreen::Render()
{
    if (!m_hWnd || !m_hBitmap)
        return;

    SIZE size = {m_width, m_height};
    RECT rcWindow;
    GetWindowRect(m_hWnd, &rcWindow);
    POINT ptOrigin = {rcWindow.left, rcWindow.top};

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_width;
    bmi.bmiHeader.biHeight = -m_height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pBits = nullptr;
    HBITMAP hbmpMem = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpMem);

    // 1. Draw Background
    HDC hdcImage = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOldImage = (HBITMAP)SelectObject(hdcImage, m_hBitmap);
    BitBlt(hdcMem, 0, 0, m_width, m_height, hdcImage, 0, 0, SRCCOPY);
    SelectObject(hdcImage, hbmpOldImage);
    DeleteDC(hdcImage);

    // 2. Draw UI using GDI+ (Fixes transparency bugs!)
    if (m_targetProgress >= 0)
    {
        Gdiplus::Graphics graphics(hdcMem);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        int barHeight = 6;
        int barX = 0;
        int barY = m_height - barHeight;
        int barWidth = m_width;


        // Progress Bar
        int filledWidth = (int)((barWidth * m_currentProgress) / 100.0f);
        if (filledWidth > 0)
        {
            Gdiplus::SolidBrush progBrush(Gdiplus::Color(255, m_progR, m_progG, m_progB));
            graphics.FillRectangle(&progBrush, barX, barY, filledWidth, barHeight);
        }

        // 3. Draw Text
        if (!m_progressMessage.empty() || m_targetProgress >= 0)
        {
            std::wstring displayText = m_progressMessage;
            if (m_targetProgress >= 0) {
                displayText += L" - " + std::to_wstring((int)m_currentProgress) + L"%";
            }
            
            Gdiplus::FontFamily fontFamily(m_fontFamily.c_str());
            Gdiplus::Font font(&fontFamily, (Gdiplus::REAL)m_fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
            Gdiplus::StringFormat format;
            format.SetAlignment(Gdiplus::StringAlignmentNear);
            format.SetLineAlignment(Gdiplus::StringAlignmentFar);

            int padding = 15;
            Gdiplus::RectF textRect((Gdiplus::REAL)padding, (Gdiplus::REAL)(barY - 40), (Gdiplus::REAL)(m_width - padding * 2), 35.0f);
            
            // Just draw the pure colored text!
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, m_textR, m_textG, m_textB));
            graphics.DrawString(displayText.c_str(), -1, &font, textRect, &format, &textBrush);
        }
    }

    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    POINT ptZero = {0, 0};
    UpdateLayeredWindow(m_hWnd, hdcScreen, &ptOrigin, &size,
                        hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

    SelectObject(hdcMem, hbmpOld);
    DeleteObject(hbmpMem);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

void SplashScreen::SetProgress(int value, const std::wstring &message, const std::string &hexColor, const std::string &textColor)
{
    if (!IsVisible())
        return;

    m_targetProgress = value;
    m_progressMessage = message;

    auto parseHex = [](const std::string &hex, int &r, int &g, int &b)
    {
        std::string h = hex;
        if (!h.empty() && h[0] == '#')
            h = h.substr(1);
        if (h.length() == 6)
        {
            try
            {
                r = std::stoi(h.substr(0, 2), nullptr, 16);
                g = std::stoi(h.substr(2, 2), nullptr, 16);
                b = std::stoi(h.substr(4, 2), nullptr, 16);
            }
            catch (...)
            {
            }
        }
    };

    parseHex(hexColor, m_progR, m_progG, m_progB);
    parseHex(textColor, m_textR, m_textG, m_textB);

    if (m_currentProgress < 0.0f)
    {
        m_currentProgress = 0.0f;
    }

    while (m_currentProgress < (float)m_targetProgress)
    {
        m_currentProgress += ((float)m_targetProgress - m_currentProgress) * 0.15f;

        if ((float)m_targetProgress - m_currentProgress < 0.5f)
        {
            m_currentProgress = (float)m_targetProgress;
        }

        Render();
        Sleep(16);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

LRESULT CALLBACK SplashScreen::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool SplashScreen::Show(const std::wstring &imagePath, int timeoutMs, int width, int height, const std::string &scaleMode)
{
    if (!LoadSplashImage(imagePath, width, height, scaleMode))
        return false;

    if (!CreateWindowExW())
        return false;

    Render();
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    if (timeoutMs > 0)
    {
        SetTimer(m_hWnd, 1, timeoutMs, nullptr);
    }

    return true;
}

void SplashScreen::Hide()
{
    if (m_hWnd && IsWindow(m_hWnd))
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

void SplashScreen::SetFont(const std::wstring &fontFamily, int fontSize)
{
    m_fontFamily = fontFamily;
    m_fontSize = fontSize;
}