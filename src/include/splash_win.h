#pragma once

#include <windows.h>
#include <string>

class SplashScreen
{
public:
    SplashScreen();
    ~SplashScreen();

    bool Show(const std::wstring &imagePath, int timeoutMs = 0, int width = 0, int height = 0, const std::string &scaleMode = "stretch");
    void Hide();

    void SetProgress(int value, const std::wstring &message = L"", const std::string &hexColor = "#00A8FF", const std::string &textColor = "#FFFFFF");
    void SetFont(const std::wstring &fontFamily, int fontSize);

    bool IsVisible() const { return m_hWnd != nullptr; }

private:
    bool CreateWindowExW();
    void CenterWindow();
    bool LoadSplashImage(const std::wstring &imagePath, int targetWidth, int targetHeight, const std::string &scaleMode);
    void Render();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    HBITMAP m_hBitmap = nullptr;
    ULONG_PTR m_gdiplusToken = 0;

    int m_width = 0;
    int m_height = 0;

    float m_currentProgress = -1.0f;
    int m_targetProgress = -1;
    int m_progR = 0;
    int m_progG = 168;
    int m_progB = 255;

    int m_textR = 255;
    int m_textG = 255;
    int m_textB = 255;
    std::wstring m_progressMessage;
    std::wstring m_fontFamily = L"Segoe UI";
    int m_fontSize = 14;
};