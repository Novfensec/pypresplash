#pragma once

#include <string>

class SplashScreen
{
public:
    SplashScreen();
    ~SplashScreen();

    bool Show(const std::string &imagePath, int timeoutMs = 0, int width = 0, int height = 0, const std::string &scaleMode = "stretch");
    void Hide();

    void SetProgress(int value, const std::string &message = "", const std::string &hexColor = "#00A8FF", const std::string &textColor = "#FFFFFF");
    void SetFont(const std::string &fontFamily, int fontSize);

    bool IsVisible() const { return m_isVisible; }

private:
    bool CreateWindowX11();
    bool LoadSplashImage(const std::string &imagePath, int targetWidth, int targetHeight, const std::string &scaleMode);
    void Render();

    bool m_isVisible = false;

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

    std::string m_progressMessage;

    std::string m_fontFamily = "sans-serif";
    int m_fontSize = 14;
};