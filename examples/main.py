import time
import pypresplash


def main():
    splash = pypresplash.SplashScreen()
    splash.set_font("Verdana", 12)

    # use any image format that works
    success = splash.show(
        "bannernfs.png", 0, 720, 405, "cover"
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
        progress = int(((i+1) / len(stages)) * 100)

        splash.set_progress(progress, stage_msg, "#0f62fe", "#ffffff")

        time.sleep(0.8)

    time.sleep(0.5)

    splash.hide()

    print("Main application started.")


if __name__ == "__main__":
    main()
