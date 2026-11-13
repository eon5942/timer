#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

static const int RATE = 44100;
static const double PI = 3.14159265358979323846;

double parse_duration(const std::string& s) {
    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if (end && *end == '\0') return v * 60.0;  // bare number = minutes

    double total = 0;
    const char* p = s.c_str();
    while (*p) {
        char* e = nullptr;
        double n = std::strtod(p, &e);
        if (e == p) { ++p; continue; }
        switch (*e) {
            case 'h': total += n * 3600; break;
            case 'm': total += n * 60;   break;
            default:  total += n;        break;
        }
        p = (*e) ? e + 1 : e;
    }
    return total;
}

void write_siren(std::FILE* f, double seconds) {
    int n = int(RATE * seconds);
    int chunk = RATE / 4;              // 0.25 s per tone
    static const int FREQ[2] = {660, 880};
    for (int i = 0; i < n; ++i) {
        int freq = FREQ[(i / chunk) & 1];
        short s = short(32000.0 * std::sin(2.0 * PI * freq * i / RATE));
        std::fwrite(&s, sizeof s, 1, f);
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && (std::string(argv[1]) == "--help" ||
                     std::string(argv[1]) == "-h")) {
        std::printf("usage: %s <time>\n\n"
                    "time: bare number = minutes, or combine h/m/s\n\n"
                    "examples:\n"
                    "  %s 5        # 5 minutes\n"
                    "  %s 90s      # 90 seconds\n"
                    "  %s 1h30m    # 1 hour 30 minutes\n",
                    argv[0], argv[0], argv[0], argv[0]);
        return 0;
    }

    double total;
    if (argc > 1) {
        total = parse_duration(argv[1]);
    } else {
        std::string s;
        std::fprintf(stderr, "Timer (e.g. 5, 90s, 1h30m): ");
        std::getline(std::cin, s);
        total = parse_duration(s);
    }
    if (total <= 0) {
        std::fprintf(stderr, "usage: %s <minutes | 90s | 1h30m>\n", argv[0]);
        return 1;
    }

    auto end = std::chrono::steady_clock::now() +
               std::chrono::milliseconds(long(total * 1000));
    while (true) {
        auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end - std::chrono::steady_clock::now()).count();
        if (left <= 0) break;
        int sec = int((left + 999) / 1000);
        std::fprintf(stderr, "\r%02d:%02d", sec / 60, sec % 60);
        std::fflush(stderr);
        std::this_thread::sleep_for(std::chrono::milliseconds(std::min<long>(200, left)));
    }
    std::fprintf(stderr, "\r00:00  TIME'S UP!\n");

    FILE* out = popen(
        "pw-play -a --rate 44100 --channels 1 --format s16 - 2>/dev/null || "
        "paplay --raw --rate 44100 --channels 1 --format s16ne - 2>/dev/null", "w");
    if (out) { write_siren(out, 4.0); pclose(out); }
    else std::fputs("\a\a\a\a\a\a\a\a\a\a", stderr);
    return 0;
}
