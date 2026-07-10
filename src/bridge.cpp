#include <pybind11/pybind11.h>
#include "splash_screen.h"

namespace py = pybind11;

PYBIND11_MODULE(pypresplash, m) {
    m.doc() = "Native Windows GDI+ PreSplash Module";

    py::class_<SplashScreen>(m, "SplashScreen")
        .def(py::init<>())
        .def("show", &SplashScreen::Show, 
             py::arg("image_path"), 
             py::arg("timeout_ms") = 0,
             py::arg("width") = 0,
             py::arg("height") = 0,
             py::arg("scale_mode") = "stretch",
             "Show the splash screen. scale_mode can be 'stretch', 'contain', or 'cover'.")
        .def("hide", &SplashScreen::Hide,
             "Hide and destroy the splash screen window.")
        .def("set_progress", &SplashScreen::SetProgress,
             py::arg("value"), 
             py::arg("message") = L"",
             py::arg("hex_color") = "#00A8FF",
             py::arg("text_color") = "#FFFFFF",
             "Update the progress bar with a target value (0-100), message, bar hex color, and text hex color.")
        .def("set_font", &SplashScreen::SetFont,
             py::arg("family"), 
             py::arg("size") = 14,
             "Set the font family and size for the progress text.")
        .def("is_visible", &SplashScreen::IsVisible,
             "Check if the splash screen is currently active.");
}