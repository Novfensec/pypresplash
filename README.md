# PyPreSplash

[![Python 3.11+](https://img.shields.io/badge/python-3.11+-blue.svg)](https://www.python.org/downloads/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)]()

**PyPreSplash** is a cross-platform splash screen module with smooth animations and transparency. 

Designed specifically for heavy GUI frameworks (like PyQt, Kivy, or PySide) and data science tools, it provides immediate visual feedback to the user while Python quietly initializes your heavy libraries in the background.

## The Problem It Solves
Python applications often suffer from a noticeable startup delay. When a user launches an app, the OS waits for the Python interpreter to load massive libraries into memory before rendering the first window. Without immediate feedback, users often assume the application failed to launch.

PyPreSplash solves this by completely bypassing Python's GUI event loop. It uses **native OS APIs** (Win32/GDI+ on Windows, X11/Cairo on Linux) via C++ bindings to instantly paint a hardware-accelerated splash screen to the monitor the millisecond your script is executed.

## Features
* **Zero Python GUI Dependencies:** Runs entirely on native C++ windowing APIs. No need to load Tkinter, Kivy, or PyQt just to show a loading screen.
* **True Alpha Transparency:** Fully supports PNGs with transparent backgrounds.
* **Asynchronous Rendering:** The progress bar smoothly glides to its target using a native C++ micro-loop and double-buffering, ensuring 60FPS animations without blocking the main Python thread.
* **CSS-Style Image Scaling:** Supports `contain`, `cover`, and `stretch` scaling modes to perfectly fit your artwork inside custom window bounds.
* **Dynamic Hex Styling:** Update progress bar and text colors on the fly using standard Hex codes (e.g., `#0f62fe`).

## Compatibility Matrix

| Operating System | Native Windowing API | Graphics / Rendering API |
| :--- | :--- | :--- |
| **Windows 10 / 11** | Win32 API | GDI+ |
| **Linux (Debian/Ubuntu)** | X11 | Cairo |
| **macOS** | *Coming Soon* | *Coming Soon* |

## Installation & Build Instructions

PyPreSplash is built using C++17, `pybind11`, and `scikit-build-core`.

### 1. Prerequisites
Depending on your host operating system, you will need the following development tools to compile the C++ source:

* **Windows:** Visual Studio / MSVC Compiler, CMake
    ```powershell
    winget install -e --id Microsoft.VisualStudio.BuildTools --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    winget install Kitware.CMake
    ```

* **Linux (Ubuntu/Debian):** X11 and Cairo development headers, CMake, and the standard compiler toolchains
    ```bash
    sudo apt-get update
    sudo apt-get install libx11-dev libcairo2-dev cmake build-essential
    ```

### 2. Standard Installation

- Install from pypi:

```bash
pip install pypresplash
```

- Install from source:

```bash
pip install https://github.com/novfensec/pypresplash/archive/main.zip
```

### 3. Building Distributable Wheels

To compile a `.whl` file that you can distribute to end-users:

```bash
git clone 
pip install build
python -m build --wheel
```

*The compiled wheel will be available in the `dist/` directory.*

## Quick Start

Initialize PyPreSplash at the **very top** of your entry script, before importing your main GUI frameworks.

```python
import time
import pypresplash

def main():
    splash = pypresplash.SplashScreen()
    splash.set_font("Segoe UI", 12) 

    success = splash.show("presplash.png", 0, 720, 405, "contain")

    if not success:
        print("Failed to initialize native splash screen.")
        return

    stages = [
        "Initializing core subsystems...",
        "Loading graphics engine...",
        "Mounting file system...",
        "Connecting to services..."
    ]

    for i, stage_msg in enumerate(stages):
        progress = int((i / len(stages)) * 100)

        splash.set_progress(progress, stage_msg, "#0f62fe", "#FFFFFF")
        time.sleep(0.8)

    splash.set_progress(100, "Ready!", "#0f62fe", "#FFFFFF")
    time.sleep(0.5)

    splash.hide()

    print("Main application GUI rendering started.")

if __name__ == "__main__":
    main()
```

## API Reference

### `SplashScreen()`

Initializes the PyPreSplash object and allocates native windowing resources.

### `.show(image_path: str, timeout_ms: int = 0, width: int = 0, height: int = 0, scale_mode: str = "stretch") -> bool`

Creates and displays the borderless splash screen window.

* **`image_path`**: Absolute or relative path to your `.png` file.
* **`timeout_ms`**: Optional auto-close timer in milliseconds (0 keeps it open indefinitely).
* **`width` / `height**`: Target window dimensions in pixels. If `0`, it defaults to the exact dimensions of the image.
* **`scale_mode`**:
* `"stretch"`: Forces the image to exactly match the target width/height.
* `"contain"`: Scales the image to fit entirely inside the width/height while maintaining aspect ratio. Unused space remains transparent.
* `"cover"`: Scales the image to fill the entire width/height, cropping edges if necessary.

### `.set_progress(value: int, message: str = "", hex_color: str = "#00A8FF", text_color: str = "#FFFFFF")`

Asynchronously animates the progress bar to a new value.

* **`value`**: Integer between 0 and 100.
* **`message`**: Status text displayed just above the loading bar.
* **`hex_color`**: Hex string for the loading bar.
* **`text_color`**: Hex string for the status text.

### `.set_font(family: str, size: int = 14)`

Configures the text rendering engine. Must be called *before* `.set_progress`.

* **`family`**: Name of an installed system font (e.g., `"Segoe UI"` on Windows, `"sans-serif"` on Linux).
* **`size`**: Font size in points.

### `.hide()`

Destroys the native window and safely releases the graphics buffer from memory.

### `.is_visible() -> bool`

Returns `True` if the splash screen is currently active and rendered on screen.


## License
This project is licensed under the MIT License. See the [LICENSE](https://github.com/Novfensec/pypresplash/tree/main/LICENSE) file for details.
