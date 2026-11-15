#include "audio.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

namespace {
    Sound sfxDrop{};
    Sound sfxWin{};
    Sound sfxDraw{};

    Sound generateTone(float startFreq, float endFreq, float durationSec, float volume = 0.95f) {
        const int sampleRate = 44100;
        const int frameCount = (int)(sampleRate * durationSec);
        short* data = (short*)std::malloc(frameCount * sizeof(short));

        float phase = 0.0f;
        for (int i = 0; i < frameCount; i++) {
            float t = (float)i / frameCount;
            float freq = startFreq + (endFreq - startFreq) * t;
            float envelope = (1.0f - t); // Linear decay stays louder longer than quadratic
            phase += (2.0f * PI * freq) / sampleRate;

            // Blend fundamental sine wave with a 2nd harmonic so low notes cut through speakers clearly
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
    SetMasterVolume(1.0f); // Ensure master volume is at 100%

    sfxDrop = generateTone(320.0f, 110.0f, 0.16f, 0.95f);
    sfxWin  = generateTone(440.0f, 880.0f, 0.50f, 0.95f);
    sfxDraw = generateTone(320.0f, 220.0f, 0.40f, 0.90f);

    SetSoundVolume(sfxDrop, 1.0f);
    SetSoundVolume(sfxWin, 1.0f);
    SetSoundVolume(sfxDraw, 1.0f);
}

void playDropSound() { PlaySound(sfxDrop); }
void playWinSound()  { PlaySound(sfxWin); }
void playDrawSound() { PlaySound(sfxDraw); }

void closeAudio() {
    UnloadSound(sfxDrop);
    UnloadSound(sfxWin);
    UnloadSound(sfxDraw);
    CloseAudioDevice();
}