import time
import pypresplash


def main():
    splash = pypresplash.SplashScreen()
    splash.set_font("Verdana", 12)

    success = splash.show(
        "presplash.png", 0, 600, 300, "cover"
    )

    if not success:
        print("Failed to initialize splash screen.")
        return

    stages = [
        "Initializing core subsystems...",
        "Loading graphics engine...",
        "Mounting file system...",
        "Connecting to services...",
        "Finalizing...",
    ]

    for i, stage_msg in enumerate(stages):
        progress = int((i / len(stages)) * 100)

        splash.set_progress(progress, stage_msg, "#0f62fe", "#000000")

        time.sleep(0.8)

    splash.set_progress(100, "Ready!", "#42be65", "#000000")
    time.sleep(0.5)

    splash.hide()

    print("Main application started.")


if __name__ == "__main__":
    main()
