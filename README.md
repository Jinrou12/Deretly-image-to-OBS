# Smart Clipboard Auto-Paster for OBS & Filmora

An automatic Windows background service that automatically saves any copied image and updates **OBS Studio**, **Wondershare Filmora**, **Discord**, or **Photoshop** instantly!

---

## 🎥 How to Setup in OBS Studio (Auto-Live Update)

Because OBS Studio doesn't have a native `Ctrl+V` image paste shortcut, this app provides a **Dynamic Live Image Source** that updates automatically on your stream!

1. Open **OBS Studio**.
2. In the **Sources** box at the bottom, click **`+`** -> select **`Image`**.
3. Name it `Clipboard Image` and click **OK**.
4. Click **Browse** and select:
   `D:\Users\.gemini\antigravity\scratch\PC\saved_images\latest_clipboard.png`
5. Check the box **"Unload image when not showing"**.
6. Click **OK**.

✨ **Done!** Now, whenever you right-click & copy any image anywhere on your PC, **it will automatically update live on your OBS screen!**

---

## 🎬 How to Use in Filmora / Discord / Photoshop (`Ctrl + V`)

1. **Copy**: Right-click any image in Chrome/Edge and click **"Copy Image"** (or press `Ctrl+C`).
2. **Paste**: Switch to **Filmora**, **Discord**, or **Photoshop** and press **`Ctrl + V`**.
3. **Done!** The image file pastes directly!

---

## 🚀 Features

- **OBS Live Update**: Automatically keeps `latest_clipboard.png` updated so your OBS scene updates in real-time.
- **Direct `Ctrl + V` Support**: Converts copied web images into native Windows File Format (`CF_HDROP`), enabling direct pasting into Filmora, Discord, Word, etc.
- **Auto-Start with Windows**: Runs silently in the background when your PC boots.
- **Auto-Archive**: Keeps timestamped PNG copies in `saved_images/` for safe keeping.

---

## 🛠️ System Tray Controls

Right-click the icon in your Windows Taskbar System Tray (bottom-right near clock):
- **📁 Open Images Folder**: Opens `saved_images` in File Explorer.
- **⚡ Run Automatically on Windows Boot**: Enable or disable auto-start on PC boot.
- **📋 Auto-Save Enabled**: Pause or resume clipboard monitoring.
- **❌ Exit**: Close the application.
