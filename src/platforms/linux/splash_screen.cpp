#include "splash_linux.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <algorithm>
#include <cairo/cairo-xlib.h>
#include <string>
#include <stdexcept>
#include <chrono>
#include <thread>

struct X11State
{
    Display *display = nullptr;
    Window window = 0;
    cairo_surface_t *x_surface = nullptr;
    cairo_surface_t *image_surface = nullptr;
    int screen = 0;
};

static X11State g_x11;

SplashScreen::SplashScreen() {}

SplashScreen::~SplashScreen()
{
    Hide();
}

bool SplashScreen::LoadSplashImage(const std::string &imagePath, int targetWidth, int targetHeight, const std::string &scaleMode)
{
    cairo_surface_t *original = cairo_image_surface_create_from_png(imagePath.c_str());
    if (cairo_surface_status(original) != CAIRO_STATUS_SUCCESS)
    {
        return false;
    }

    int srcWidth = cairo_image_surface_get_width(original);
    int srcHeight = cairo_image_surface_get_height(original);

    if (targetWidth <= 0 || targetHeight <= 0)
    {
        g_x11.image_surface = original;
        m_width = srcWidth;
        m_height = srcHeight;
        return true;
    }

    m_width = targetWidth;
    m_height = targetHeight;

    g_x11.image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, targetWidth, targetHeight);
    cairo_t *cr = cairo_create(g_x11.image_surface);

    double scaleX = (double)targetWidth / srcWidth;
    double scaleY = (double)targetHeight / srcHeight;
    double offsetX = 0.0;
    double offsetY = 0.0;

    if (scaleMode == "contain")
    {
        double scale = std::min(scaleX, scaleY);
        scaleX = scale;
        scaleY = scale;
        offsetX = (targetWidth - (srcWidth * scale)) / 2.0;
        offsetY = (targetHeight - (srcHeight * scale)) / 2.0;
    }
    else if (scaleMode == "cover")
    {
        double scale = std::max(scaleX, scaleY);
        scaleX = scale;
        scaleY = scale;
        offsetX = (targetWidth - (srcWidth * scale)) / 2.0;
        offsetY = (targetHeight - (srcHeight * scale)) / 2.0;
    }

    cairo_translate(cr, offsetX, offsetY);
    cairo_scale(cr, scaleX, scaleY);

    cairo_set_source_surface(cr, original, 0, 0);

    cairo_pattern_t *pattern = cairo_get_source(cr);
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_BILINEAR);

    cairo_paint(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(original);

    return true;
}

bool SplashScreen::CreateWindowExW()
{
    g_x11.display = XOpenDisplay(nullptr);
    if (!g_x11.display)
        return false;

    g_x11.screen = DefaultScreen(g_x11.display);

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(g_x11.display, g_x11.screen, 32, TrueColor, &vinfo))
    {
        return false;
    }

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(g_x11.display, DefaultRootWindow(g_x11.display), vinfo.visual, AllocNone);
    attr.border_pixel = 0;
    attr.background_pixel = 0;

    int screenWidth = DisplayWidth(g_x11.display, g_x11.screen);
    int screenHeight = DisplayHeight(g_x11.display, g_x11.screen);
    int x = (screenWidth - m_width) / 2;
    int y = (screenHeight - m_height) / 2;

    g_x11.window = XCreateWindow(g_x11.display, DefaultRootWindow(g_x11.display),
                                 x, y, m_width, m_height, 0, vinfo.depth, InputOutput,
                                 vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);

    Atom window_type = XInternAtom(g_x11.display, "_NET_WM_WINDOW_TYPE", False);
    Atom type_splash = XInternAtom(g_x11.display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    XChangeProperty(g_x11.display, g_x11.window, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&type_splash, 1);

    XMapWindow(g_x11.display, g_x11.window);
    XFlush(g_x11.display);

    g_x11.x_surface = cairo_xlib_surface_create(g_x11.display, g_x11.window, vinfo.visual, m_width, m_height);

    return true;
}

void SplashScreen::Render()
{
    if (!g_x11.x_surface)
        return;

    cairo_t *cr = cairo_create(g_x11.x_surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    if (g_x11.image_surface)
    {
        cairo_set_source_surface(cr, g_x11.image_surface, 0, 0);
        cairo_paint(cr);
    }

    if (m_targetProgress >= 0)
    {
        int barHeight = 6;
        int barY = m_height - barHeight;
        int filledWidth = (int)((m_width * m_currentProgress) / 100.0f);

        if (filledWidth > 0)
        {
            cairo_set_source_rgba(cr, m_progR / 255.0, m_progG / 255.0, m_progB / 255.0, 1.0);
            cairo_rectangle(cr, 0, barY, filledWidth, barHeight);
            cairo_fill(cr);
        }

        if (!m_progressMessage.empty())
        {
            std::string displayText = m_progressMessage + " - " + std::to_string((int)m_currentProgress) + "%";

            cairo_select_font_face(cr, m_fontFamily.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, m_fontSize);

            int padding = 15;
            int textY = barY - 15;

            cairo_set_source_rgba(cr, 0, 0, 0, 0.8);
            cairo_move_to(cr, padding + 1, textY + 1);
            cairo_show_text(cr, displayText.c_str());

            cairo_set_source_rgba(cr, m_textR / 255.0, m_textG / 255.0, m_textB / 255.0, 1.0);
            cairo_move_to(cr, padding, textY);
            cairo_show_text(cr, displayText.c_str());
        }
    }

    cairo_surface_flush(g_x11.x_surface);
    cairo_destroy(cr);
    XFlush(g_x11.display);
}

void SplashScreen::SetProgress(int value, const std::string &message, const std::string &hexColor, const std::string &textColor)
{
    if (!g_x11.display)
        return;

    m_targetProgress = value;
    m_progressMessage = message;

    if (m_currentProgress < 0.0f)
        m_currentProgress = 0.0f;

    while (m_currentProgress < (float)m_targetProgress)
    {
        m_currentProgress += ((float)m_targetProgress - m_currentProgress) * 0.15f;
        if ((float)m_targetProgress - m_currentProgress < 0.5f)
        {
            m_currentProgress = (float)m_targetProgress;
        }

        Render();

        while (XPending(g_x11.display))
        {
            XEvent ev;
            XNextEvent(g_x11.display, &ev);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

bool SplashScreen::Show(const std::string &imagePath, int timeoutMs, int width, int height, const std::string &scaleMode)
{
    if (!LoadSplashImage(imagePath, width, height, scaleMode))
        return false;
    if (!CreateWindowExW())
        return false;
    Render();
    return true;
}

void SplashScreen::Hide()
{
    if (g_x11.x_surface)
    {
        cairo_surface_destroy(g_x11.x_surface);
        g_x11.x_surface = nullptr;
    }
    if (g_x11.image_surface)
    {
        cairo_surface_destroy(g_x11.image_surface);
        g_x11.image_surface = nullptr;
    }
    if (g_x11.display && g_x11.window)
    {
        XDestroyWindow(g_x11.display, g_x11.window);
        XCloseDisplay(g_x11.display);
        g_x11.display = nullptr;
    }
}