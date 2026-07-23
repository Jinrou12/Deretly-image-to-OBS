import wave
import math
import struct
import os

def generate_wav(filepath, duration_sec=5.0, sample_rate=48000):
    num_samples = int(duration_sec * sample_rate)
    freqs = [523.25, 659.25, 783.99, 1046.50] # C5, E5, G5, C6 chord progression

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with wave.open(filepath, 'w') as wav_file:
        wav_file.setnchannels(2) # Stereo
        wav_file.setsampwidth(2) # 16-bit PCM
        wav_file.setframerate(sample_rate)

        for i in range(num_samples):
            t = float(i) / sample_rate
            # Harmonic chime generator
            sample = 0.25 * math.sin(2.0 * math.pi * freqs[int(t * 2) % len(freqs)] * t)
            sample += 0.15 * math.sin(2.0 * math.pi * (freqs[int(t * 2) % len(freqs)] / 2) * t)

            # Smooth fade envelope
            envelope = math.sin(math.pi * (i / num_samples))
            sample_val = int(sample * envelope * 32767.0)

            # Pack stereo 16-bit PCM
            data = struct.pack('<hh', sample_val, sample_val)
            wav_file.writeframesraw(data)

    print(f"Successfully generated sample test music: {filepath}")

if __name__ == "__main__":
    target_path = r"C:\Users\Visal\Downloads\safe_music_sample.wav"
    generate_wav(target_path)
