"""
OBS Music Blocker & Audio Shield - Live Demo & Practice Simulator
Simulates real-time stream audio processing, music spectral detection, and replacement crossfading.
"""

import sys
import time

# Force UTF-8 stdout encoding for Windows console compatibility
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

class DSPDetectorSim:
    def __init__(self, sample_rate=48000, fft_size=1024):
        self.sample_rate = sample_rate
        self.fft_size = fft_size
        self.sensitivity = 0.6
        self.consecutive_detect = 0
        self.is_detected = False

    def process_frame(self, frame_type):
        if frame_type == 'music':
            score = 0.85
        elif frame_type == 'speech':
            score = 0.35
        else:
            score = 0.10

        threshold = 0.65 - (self.sensitivity * 0.35)

        if score >= threshold:
            self.consecutive_detect += 1
        else:
            self.consecutive_detect = max(0, self.consecutive_detect - 1)

        if self.consecutive_detect >= 2:
            self.is_detected = True
        else:
            self.is_detected = False

        return self.is_detected, score

def run_practice_demo():
    print("=" * 72)
    print("      OBS MUSIC BLOCKER & SHIELD - PRACTICE DEMONSTRATION      ")
    print("=" * 72)
    print("Simulating live stream audio feed...\n")

    detector = DSPDetectorSim()

    timeline = [
        (1, "speech", "Streamer talking: 'Welcome to the stream everyone!'"),
        (2, "speech", "Streamer talking: 'Today we are testing our new OBS plugin.'"),
        (3, "music",  "AUDIO: Copyrighted music starts playing in background!"),
        (4, "music",  "AUDIO: Music continuing..."),
        (5, "music",  "AUDIO: Music continuing..."),
        (6, "speech", "AUDIO: Music stopped. Streamer talking: 'Back to the game!'"),
        (7, "speech", "Streamer talking: 'Thanks for watching!'")
    ]

    shield_active = False
    volume_desktop = 100
    volume_custom_music = 0

    for t, audio_type, description in timeline:
        time.sleep(0.4)
        detected, score = detector.process_frame(audio_type)

        if detected and not shield_active:
            shield_active = True
            print(f"\n[!] ALERT: Music Detected! (Confidence: {score*100:.0f}%)")
            print("    -> OBS Shield Engaged: Muting Desktop Audio & Playing Safe Music...\n")
        elif not detected and shield_active:
            shield_active = False
            print(f"\n[+] OK: Music Ended. (Confidence: {score*100:.0f}%)")
            print("    -> OBS Shield Released: Restoring Normal Stream Audio...\n")

        if shield_active:
            volume_desktop = max(0, volume_desktop - 50)
            volume_custom_music = min(100, volume_custom_music + 50)
            status_badge = "[SHIELD ACTIVE]  (Desktop Muted | Playing Safe Music)"
        else:
            volume_desktop = min(100, volume_desktop + 50)
            volume_custom_music = max(0, volume_custom_music - 50)
            status_badge = "[SHIELD IDLE]    (Normal Stream Audio)"

        print(f"[{t:02d}s] {description}")
        print(f"      Status: {status_badge}")
        print(f"      Vol -> Desktop Audio: {volume_desktop:3d}% | Replacement Track: {volume_custom_music:3d}%")
        print("-" * 72)

    print("\n" + "=" * 72)
    print("  DEMO COMPLETE: OBS Music Blocker successfully protected the stream!  ")
    print("=" * 72)

if __name__ == "__main__":
    run_practice_demo()
