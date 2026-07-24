import os
import sys
import time
import io
import hashlib
import threading
import ctypes
from ctypes import wintypes
from datetime import datetime
from pathlib import Path
from PIL import Image, ImageGrab, ImageDraw
import win32clipboard
import pystray

# Directory configuration
WORKSPACE_DIR = Path(__file__).parent.resolve()
SAVE_DIR = WORKSPACE_DIR / "saved_images"
LATEST_IMAGE_PATH = SAVE_DIR / "latest_clipboard.png"

# Global state
auto_save_enabled = True
is_running = True
last_image_hash = None
saved_count = 0
is_self_updating_clipboard = False  # Flag to prevent infinite clipboard re-triggering
last_sequence_number = 0
user32 = ctypes.windll.user32


class DROPFILES(ctypes.Structure):
    _fields_ = [
        ('pFiles', wintypes.DWORD),
        ('pt', wintypes.POINT),
        ('fNC', wintypes.BOOL),
        ('fWide', wintypes.BOOL),
    ]


kernel32 = ctypes.windll.kernel32
kernel32.GlobalAlloc.argtypes = [wintypes.UINT, ctypes.c_size_t]
kernel32.GlobalAlloc.restype = wintypes.HGLOBAL
kernel32.GlobalLock.argtypes = [wintypes.HGLOBAL]
kernel32.GlobalLock.restype = ctypes.c_void_p
kernel32.GlobalUnlock.argtypes = [wintypes.HGLOBAL]


def ensure_dirs():
    SAVE_DIR.mkdir(parents=True, exist_ok=True)
    if not LATEST_IMAGE_PATH.exists():
        try:
            placeholder = Image.new('RGBA', (600, 300), color=(30, 41, 59, 255))
            draw = ImageDraw.Draw(placeholder)
            draw.text((40, 140), "Ready! Copy any image to display here.", fill=(255, 255, 255))
            placeholder.save(LATEST_IMAGE_PATH, format="PNG")
        except Exception:
            pass


def generate_tray_icon():
    img = Image.new('RGBA', (64, 64), color=(0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle((4, 4, 60, 60), radius=12, fill=(30, 41, 59), outline=(99, 102, 241), width=3)
    draw.rectangle((16, 20, 48, 44), outline=(255, 255, 255), width=3)
    draw.polygon([(20, 38), (28, 28), (36, 38)], fill=(99, 102, 241))
    draw.ellipse((36, 24, 42, 30), fill=(234, 179, 8))
    return img


def inject_file_drop_to_clipboard(filepath: Path, raw_img: Image.Image):
    """
    Injects CF_HDROP (File Drop) into Windows Clipboard alongside CF_DIB.
    This allows Ctrl+V in OBS, Filmora, Discord, etc. to paste the image file directly!
    """
    global is_self_updating_clipboard
    is_self_updating_clipboard = True
    try:
        abs_path = str(filepath.resolve())
        dropfiles = DROPFILES()
        dropfiles.pFiles = ctypes.sizeof(DROPFILES)
        dropfiles.fWide = True

        encoded_path = abs_path.encode('utf-16le') + b'\x00\x00\x00\x00'
        buf = bytes(dropfiles) + encoded_path

        GMEM_MOVEABLE = 0x0002
        GMEM_ZEROINIT = 0x0040
        hMem = kernel32.GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len(buf))
        pMem = kernel32.GlobalLock(hMem)
        ctypes.memmove(pMem, buf, len(buf))
        kernel32.GlobalUnlock(hMem)

        output = io.BytesIO()
        raw_img.convert('RGB').save(output, 'BMP')
        dib_data = output.getvalue()[14:]
        output.close()

        win32clipboard.OpenClipboard()
        win32clipboard.EmptyClipboard()
        win32clipboard.SetClipboardData(win32clipboard.CF_HDROP, hMem)
        win32clipboard.SetClipboardData(win32clipboard.CF_DIB, dib_data)
        win32clipboard.CloseClipboard()
        global last_sequence_number
        last_sequence_number = user32.GetClipboardSequenceNumber()
    except Exception as e:
        print(f"Error updating clipboard drop format: {e}")
    finally:
        time.sleep(0.3)
        is_self_updating_clipboard = False


def get_clipboard_image():
    global is_self_updating_clipboard
    if is_self_updating_clipboard:
        return None

    try:
        data = ImageGrab.grabclipboard()
        if isinstance(data, Image.Image):
            return data
        elif isinstance(data, list) and len(data) > 0:
            first_path = data[0]
            if isinstance(first_path, str) and os.path.exists(first_path):
                ext = os.path.splitext(first_path)[1].lower()
                if ext in ['.png', '.jpg', '.jpeg', '.webp', '.bmp', '.gif']:
                    return Image.open(first_path)
    except Exception:
        pass
    return None


def compute_image_hash(img: Image.Image) -> str:
    try:
        small = img.copy().resize((100, 100)).convert("RGB")
        return hashlib.md5(small.tobytes()).hexdigest()
    except Exception:
        return ""


def process_clipboard_image(img: Image.Image, icon: pystray.Icon = None):
    global last_image_hash, saved_count

    ensure_dirs()
    img_hash = compute_image_hash(img)
    if not img_hash:
        return

    last_image_hash = img_hash
    try:
        save_img = img.convert("RGBA") if img.mode != "RGBA" else img

        # 1. Overwrite latest_clipboard.png safely
        try:
            save_img.save(LATEST_IMAGE_PATH, format="PNG")
        except Exception:
            try:
                temp_latest = SAVE_DIR / "latest_clipboard.tmp.png"
                save_img.save(temp_latest, format="PNG")
                if temp_latest.exists():
                    os.replace(temp_latest, LATEST_IMAGE_PATH)
            except Exception as err:
                print(f"Warning updating latest_clipboard.png: {err}")

        # 2. Update latest_timestamp.txt to notify OBS script of new image
        try:
            (SAVE_DIR / "latest_timestamp.txt").write_text(str(time.time()), encoding="utf-8")
        except Exception:
            pass

        # 3. Inject CF_HDROP format into Windows Clipboard for direct Ctrl+V pasting!
        inject_file_drop_to_clipboard(LATEST_IMAGE_PATH, save_img)

        saved_count += 1
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Auto-saved #{saved_count}: latest_clipboard.png")
        print(" -> Ready! You can now press Ctrl+V directly in OBS, Filmora, or any app!")

        pass
    except Exception as e:
        print(f"Error processing image: {e}")


def monitor_loop(icon: pystray.Icon):
    ensure_dirs()
    print("=========================================")
    print(" Smart Clipboard Auto-Paster for OBS & Filmora ")
    print("=========================================")
    print(f"[Folder] Images Directory : {SAVE_DIR}")
    print(f"[Target] OBS Source Target: {LATEST_IMAGE_PATH}")
    print("Status: Active. Copy an image in Chrome -> Press Ctrl+V in OBS/Filmora!\n")

    global last_sequence_number
    while is_running:
        if auto_save_enabled:
            current_seq = user32.GetClipboardSequenceNumber()
            if current_seq != last_sequence_number:
                if not is_self_updating_clipboard:
                    last_sequence_number = current_seq
                    img = get_clipboard_image()
                    if img:
                        process_clipboard_image(img, icon)
        time.sleep(0.3)


def is_in_startup():
    startup_dir = Path(os.getenv("APPDATA")) / "Microsoft" / "Windows" / "Start Menu" / "Programs" / "Startup"
    shortcut = startup_dir / "ClipboardImageAutoSaver.vbs"
    return shortcut.exists()


def toggle_startup(icon, item):
    startup_dir = Path(os.getenv("APPDATA")) / "Microsoft" / "Windows" / "Start Menu" / "Programs" / "Startup"
    vbs_path = startup_dir / "ClipboardImageAutoSaver.vbs"

    if vbs_path.exists():
        try:
            vbs_path.unlink()
            if icon:
                icon.notify(title="Clipboard Auto-Paster", message="Removed from Windows Startup")
            print("Removed from Windows Startup")
        except Exception as e:
            print(f"Error removing startup link: {e}")
    else:
        try:
            pythonw = WORKSPACE_DIR / ".venv" / "Scripts" / "pythonw.exe"
            main_script = WORKSPACE_DIR / "main.py"
            vbs_content = f'CreateObject("Wscript.Shell").Run Chr(34) & "{pythonw}" & Chr(34) & " " & Chr(34) & "{main_script}" & Chr(34), 0, False\n'
            vbs_path.write_text(vbs_content, encoding="utf-8")
            if icon:
                icon.notify(title="Clipboard Auto-Paster", message="Added to Windows Startup! Will run automatically on PC boot.")
            print("Added to Windows Startup!")
        except Exception as e:
            print(f"Error adding startup link: {e}")


def open_folder(icon, item):
    ensure_dirs()
    os.startfile(str(SAVE_DIR))


def toggle_autosave(icon, item):
    global auto_save_enabled
    auto_save_enabled = not auto_save_enabled
    state_str = "ENABLED" if auto_save_enabled else "DISABLED"
    try:
        icon.notify(title="Clipboard Auto-Paster", message=f"Auto-Save is now {state_str}")
    except Exception:
        pass


def exit_app(icon, item):
    global is_running
    is_running = False
    icon.visible = False
    icon.stop()


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        try:
            sys.stdout.reconfigure(encoding='utf-8', errors='replace')
        except Exception:
            pass
    ensure_dirs()
    tray_image = generate_tray_icon()

    menu = pystray.Menu(
        pystray.MenuItem("📁 Open Images Folder", open_folder),
        pystray.MenuItem("⚡ Run Automatically on Windows Boot", toggle_startup, checked=lambda item: is_in_startup()),
        pystray.MenuItem("📋 Auto-Save Enabled", toggle_autosave, checked=lambda item: auto_save_enabled),
        pystray.Menu.SEPARATOR,
        pystray.MenuItem("❌ Exit", exit_app)
    )

    icon = pystray.Icon(
        name="ClipboardImageAutoSaver",
        icon=tray_image,
        title="Clipboard Image Auto-Paster (OBS & Filmora)",
        menu=menu
    )

    monitor_thread = threading.Thread(target=monitor_loop, args=(icon,), daemon=True)
    monitor_thread.start()

    icon.run()


if __name__ == "__main__":
    main()
