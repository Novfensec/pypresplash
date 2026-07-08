# PyPreSplash

A lightning-fast, native splash screen module for Python applications. 

### Why does this exist?
Python applications (especially those using heavy libraries or frameworks like PyQt/Kivy) take a few seconds to start up. Without a splash screen, users might click your app icon, see nothing happen for 4 seconds, assume it's broken, and click it three more times. 

**PyPreSplash** pops up *instantly* using the native Windows API. It gives your users immediate visual feedback (a logo and a progress bar) while Python quietly finishes doing the heavy lifting in the background.

## Features
* **Zero Heavy Dependencies:** Uses native Win32/GDI+ APIs. No need to load Tkinter, PyQt, or Kivy just to show a loading screen.
* **True Transparency:** Fully supports PNGs with transparent backgrounds. No ugly black boxes around your logo.
* **Smooth 60FPS Animations:** The progress bar smoothly glides to its target using a C++ micro-loop, ensuring the window stays responsive.
* **Minimalist UI:** The progress bar draws directly over your image edge-to-edge for a modern look.
* **Dynamic Styling:** Change progress bar and text colors on the fly using standard Hex codes (e.g., `#0f62fe`).
* **Custom Typography:** Use any system font (e.g., `"Segoe UI"`, `"Consolas"`) with hardware-accelerated ClearType rendering.

## Installation

This project is built using C++17, Pybind11, and `scikit-build-core`. 

**Prerequisites:**
* Windows 10 or 11
* Python 3.8+
* A C++ Compiler (Visual Studio / MSVC)

### Install for Development
To install the module directly into your current Python environment:
```bash
pip install .

```

### Build a Distributable Wheel

To compile a `.whl` file that you can distribute to end-users (so they don't need a C++ compiler installed to use your app):

```bash
pip install build
python -m build --wheel

```

The compiled wheel will be located in the `dist/` folder.

## Quick Start

Here is a complete example of how to mask a heavy application load.

```python
import time
import pypresplash

def main():
    splash = pypresplash.SplashScreen()

    splash.set_font("Segoe UI", 12) 

    success = splash.show("presplash.png", 0, 600, 600, "contain")
    
    if not success:
        print("Failed to load splash screen.")
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

    splash.set_progress(100, "Ready!", "#42be65", "#FFFFFF")
    time.sleep(0.5)

    splash.hide()
    
    print("Main application started.")

if __name__ == "__main__":
    main()

```

## API Reference

### `SplashScreen()`

Initializes the splash screen object and starts the GDI+ subsystem.

### `.show(image_path, timeout_ms=0, width=0, height=0, scale_mode="stretch") -> bool`

Creates and displays the splash screen window.

* **`image_path`**: Path to your `.png`, `.jpg`, or `.bmp` file.
* **`timeout_ms`**: Optional auto-close timer in milliseconds (0 keeps it open indefinitely).
* **`width` / `height**`: Target window dimensions. If `0`, it defaults to the exact size of the image file.
* **`scale_mode`**:
* `"stretch"` (Default): Forces the image to exactly match the target width/height.
* `"contain"`: Scales the image to fit entirely inside the width/height while maintaining aspect ratio. Unused space is transparent.
* `"cover"`: Scales the image to fill the entire width/height, cropping edges if necessary.



### `.set_progress(value, message="", hex_color="#00A8FF", text_color="#FFFFFF")`

Smoothly animates the progress bar to a new value.

* **`value`**: Integer between 0 and 100.
* **`message`**: Status text displayed just above the loading bar.
* **`hex_color`**: Hex string for the loading bar (e.g., `"#FF5733"`).
* **`text_color`**: Hex string for the status text.

### `.set_font(family, size=14)`

Configures the text rendering engine. Must be called *before* `.set_progress`.

* **`family`**: Name of an installed system font (e.g., `"Segoe UI"`, `"Consolas"`).
* **`size`**: Font size in points. (Recommended: 10 to 13).

### `.hide()`

Destroys the window and safely releases GDI+ resources.

### `.is_visible() -> bool`

Returns `True` if the splash screen is currently active and rendered on screen.
