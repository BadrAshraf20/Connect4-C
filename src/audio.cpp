#include "audio.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

namespace {
    Sound sfxDrop{};
    Sound sfxWin{};
    Sound sfxDraw{};
    Sound sfxTick{};
    Sound sfxUrgentTick{};

    Sound generateTone(float startFreq, float endFreq, float durationSec, float volume = 0.95f) {
        const int sampleRate = 44100;
        const int frameCount = (int)(sampleRate * durationSec);
        short* data = (short*)std::malloc(frameCount * sizeof(short));

        float phase = 0.0f;
        for (int i = 0; i < frameCount; i++) {
            float t = (float)i / frameCount;
            float freq = startFreq + (endFreq - startFreq) * t;
            float envelope = (1.0f - t);
            phase += (2.0f * PI * freq) / sampleRate;

            float sample = 0.75f * std::sin(phase) + 0.25f * std::sin(phase * 2.0f);
            data[i] = (short)(sample * 32767.0f * volume * envelope);
        }

        Wave wave{};
        wave.frameCount = frameCount;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = data;

        Sound sound = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return sound;
    }
}

void initAudio() {
    InitAudioDevice();
    SetMasterVolume(1.0f);

    sfxDrop       = generateTone(320.0f, 110.0f, 0.16f, 0.95f);
    sfxWin        = generateTone(440.0f, 880.0f, 0.50f, 0.95f);
    sfxDraw       = generateTone(320.0f, 220.0f, 0.40f, 0.90f);
    sfxTick       = generateTone(850.0f, 650.0f, 0.045f, 0.75f); // Crisp stopwatch tick
    sfxUrgentTick = generateTone(1150.0f, 900.0f, 0.055f, 0.85f); // Higher pitch for final second

    SetSoundVolume(sfxDrop, 1.0f);
    SetSoundVolume(sfxWin, 1.0f);
    SetSoundVolume(sfxDraw, 1.0f);
    SetSoundVolume(sfxTick, 0.85f);
    SetSoundVolume(sfxUrgentTick, 0.95f);
}

void playDropSound() { PlaySound(sfxDrop); }
void playWinSound()  { PlaySound(sfxWin); }
void playDrawSound() { PlaySound(sfxDraw); }

void playTickSound(bool urgent) {
    PlaySound(urgent ? sfxUrgentTick : sfxTick);
}

void closeAudio() {
    UnloadSound(sfxDrop);
    UnloadSound(sfxWin);
    UnloadSound(sfxDraw);
    UnloadSound(sfxTick);
    UnloadSound(sfxUrgentTick);
    CloseAudioDevice();
}
