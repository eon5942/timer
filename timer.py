#!/usr/bin/env python3
import math, os, re, struct, subprocess, sys, tempfile, time, wave

SAMPLE_RATE = 44100

def parse_duration(s):
    s = s.strip().lower()
    if re.fullmatch(r"\d+(?:\.\d+)?", s):  # bare number = minutes
        return float(s) * 60
    total = 0.0
    for num, unit in re.findall(r"(\d+(?:\.\d+)?)([hms])", s):
        n = float(num)
        total += n * (3600 if unit == "h" else 60 if unit == "m" else 1)
    return total

def make_siren(path, seconds=3.0):
    with wave.open(path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SAMPLE_RATE)
        frames = bytearray()
        n = int(SAMPLE_RATE * seconds)
        for i in range(n):
            t = i / SAMPLE_RATE
            freq = 660 if (i // (SAMPLE_RATE // 4)) % 2 == 0 else 880
            sample = int(32000 * math.sin(2 * math.pi * freq * t))
            frames += struct.pack("<h", sample)
        w.writeframes(bytes(frames))

def play(path):
    if sys.platform == "darwin":
        return subprocess.run(["afplay", path])
    if sys.platform == "win32":
        import winsound
        return winsound.PlaySound(path, winsound.SND_FILENAME)
    for cmd in ("pw-play", "paplay", "aplay"):
        exe = subprocess.run(["sh", "-c", f"command -v {cmd}"], capture_output=True).stdout.strip()
        if exe:
            return subprocess.run([exe.decode(), path])
    return subprocess.run(["sh", "-c", "printf '\\a\\a\\a\\a\\a\\a\\a\\a\\a\\a'"])

def main():
    if len(sys.argv) < 2:
        s = input("Timer (e.g. 5, 90s, 1h30m): ")
    else:
        s = sys.argv[1]
    total = parse_duration(s)
    if total <= 0:
        print(f"usage: {sys.argv[0]} <minutes | 90s | 1h30m>")
        sys.exit(1)

    tmp = os.path.join(tempfile.gettempdir(), "timer_beep.wav")
    make_siren(tmp)

    end = time.monotonic() + total
    while True:
        left = end - time.monotonic()
        if left <= 0:
            break
        m, sec = divmod(int(left) + 1, 60)
        print(f"\r{m:02d}:{sec:02d}", end="", flush=True)
        time.sleep(min(1, left))

    print("\r00:00  TIME'S UP!")
    play(tmp)
    os.remove(tmp)

if __name__ == "__main__":
    main()
