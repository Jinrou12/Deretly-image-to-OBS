import os
import sys
from pathlib import Path

WORKSPACE_DIR = Path(__file__).parent.resolve()
SAVE_DIR = WORKSPACE_DIR / "saved_images"
LATEST_IMAGE_PATH = SAVE_DIR / "latest_clipboard.png"

def check():
    print("=== OBS Clipboard Auto-Saver Status Check ===")
    print(f"Workspace Directory: {WORKSPACE_DIR}")
    print(f"Saved Images Dir   : {SAVE_DIR} (Exists: {SAVE_DIR.exists()})")
    print(f"Latest Image Path  : {LATEST_IMAGE_PATH} (Exists: {LATEST_IMAGE_PATH.exists()})")
    
    if SAVE_DIR.exists():
        files = list(SAVE_DIR.glob("*"))
        print(f"Found {len(files)} files in saved_images:")
        for f in files:
            print(f" - {f.name} ({f.stat().st_size} bytes)")
    
if __name__ == "__main__":
    check()
