/* audio_shim.c -- miniaudio device + procedural-beep mixer.
 *
 * Phase 8 minimum: produce audible feedback for fire / hit / break
 * events so the game has a soundscape. Real VAG decoding (Phase 8b)
 * will plug into the same mixer.
 *
 * The mixer maintains up to 16 active voices. Each voice is a sine
 * with a frequency + amplitude envelope. Audio_PlaySfx(kind) picks
 * a free voice and seeds the envelope.
 *
 * On --audio-capture <path>: copies the mixer output to a WAV file
 * (RIFF/PCM s16, mono, 22050 Hz). Smoke test verifies non-silent.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(V8_HAVE_MINIAUDIO)
  #define MA_NO_DECODING
  #define MA_NO_ENCODING
  #define MA_NO_GENERATION
  #define MINIAUDIO_IMPLEMENTATION
  #include <miniaudio.h>
#endif

#define SR    22050
#define VOICES 16

typedef struct {
    int    active;
    float  freq;      /* Hz */
    float  amp;       /* current */
    float  decay;     /* per-sample multiplier */
    double phase;     /* radians */
} Voice;

static Voice g_voices[VOICES];

/* Audio-capture support (--audio-capture flag). */
static FILE  *g_cap_fp = NULL;
static int    g_cap_samples = 0;

/* Foreground: pick free voice, set freq + amp + decay. */
static int alloc_voice(float freq, float amp, float halfLifeMs)
{
    for (int i = 0; i < VOICES; i++) {
        if (!g_voices[i].active) {
            g_voices[i].active = 1;
            g_voices[i].freq   = freq;
            g_voices[i].amp    = amp;
            g_voices[i].phase  = 0;
            /* x *= decay  every sample; want amp -> amp/2 in halfLifeMs */
            float halfSamples = (halfLifeMs / 1000.0f) * SR;
            g_voices[i].decay  = powf(0.5f, 1.0f / halfSamples);
            return i;
        }
    }
    return -1;
}

void Audio_PlaySfx(int kind)
{
    switch (kind) {
        case 1: alloc_voice( 800.0f, 0.3f,  60.0f);  break;  /* fire */
        case 2: alloc_voice( 220.0f, 0.5f, 180.0f);  break;  /* hit */
        case 3: alloc_voice( 110.0f, 0.7f, 400.0f);  break;  /* break/explosion */
        case 4: alloc_voice(1200.0f, 0.2f,  30.0f);  break;  /* low fuel beep */
        default:                                            break;
    }
}

/* Mixer callback -- fills `frames` mono s16 samples into `out`. */
static void mix_samples(int16_t *out, int frames)
{
    for (int n = 0; n < frames; n++) {
        float s = 0.0f;
        for (int v = 0; v < VOICES; v++) {
            if (!g_voices[v].active) continue;
            s += g_voices[v].amp * sinf((float)g_voices[v].phase);
            g_voices[v].phase += 2.0 * 3.14159265358979 * g_voices[v].freq / SR;
            if (g_voices[v].phase > 2.0 * 3.14159265358979)
                g_voices[v].phase -= 2.0 * 3.14159265358979;
            g_voices[v].amp *= g_voices[v].decay;
            if (g_voices[v].amp < 0.001f) g_voices[v].active = 0;
        }
        /* Clamp. */
        if (s >  1.0f) s =  1.0f;
        if (s < -1.0f) s = -1.0f;
        out[n] = (int16_t)(s * 30000.0f);
    }
}

#if defined(V8_HAVE_MINIAUDIO)
static ma_device g_dev;
static int       g_dev_ok = 0;

static void ma_callback(ma_device *dev, void *output, const void *input, ma_uint32 frames)
{
    (void)dev; (void)input;
    int16_t *o = (int16_t *)output;
    mix_samples(o, (int)frames);
    if (g_cap_fp) {
        fwrite(o, 2, frames, g_cap_fp);
        g_cap_samples += (int)frames;
    }
}
#endif

void Audio_Init(void)
{
#if defined(V8_HAVE_MINIAUDIO)
    if (g_dev_ok) return;
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format   = ma_format_s16;
    cfg.playback.channels = 1;
    cfg.sampleRate        = SR;
    cfg.dataCallback      = ma_callback;
    if (ma_device_init(NULL, &cfg, &g_dev) != MA_SUCCESS) {
        fprintf(stderr, "v8: audio device init failed (continuing silent)\n");
        return;
    }
    if (ma_device_start(&g_dev) != MA_SUCCESS) {
        fprintf(stderr, "v8: audio device start failed (continuing silent)\n");
        return;
    }
    g_dev_ok = 1;
    fprintf(stderr, "v8: audio device ready (%d Hz mono)\n", SR);
#else
    fprintf(stderr, "v8: audio compiled out\n");
#endif
}

/* Headless audio capture path -- we don't open a device, just run
 * the mixer manually each game tick and dump to WAV. Called from
 * pad_shim.c per frame when --audio-capture is set without an
 * active playback device. */
void Audio_PumpHeadless(int frames_per_tick)
{
    if (!g_cap_fp) return;
    int16_t buf[2048];
    int n = frames_per_tick;
    while (n > 0) {
        int chunk = (n > 2048) ? 2048 : n;
        mix_samples(buf, chunk);
        fwrite(buf, 2, chunk, g_cap_fp);
        g_cap_samples += chunk;
        n -= chunk;
    }
}

/* WAV writer: open + write header (with placeholder sizes); on close
 * rewrite the data chunk size. */
int Audio_CaptureStart(const char *path)
{
    g_cap_fp = fopen(path, "wb");
    if (!g_cap_fp) return -1;
    /* RIFF/WAV PCM header for mono s16 @22050 Hz. Sizes patched at close. */
    uint8_t hdr[44] = {
        'R','I','F','F', 0,0,0,0, 'W','A','V','E',
        'f','m','t',' ', 16,0,0,0, 1,0, 1,0,
        (uint8_t)(SR & 0xff), (uint8_t)((SR>>8)&0xff), (uint8_t)((SR>>16)&0xff), (uint8_t)((SR>>24)&0xff),
        (uint8_t)((SR*2) & 0xff), (uint8_t)(((SR*2)>>8)&0xff),
        (uint8_t)(((SR*2)>>16)&0xff), (uint8_t)(((SR*2)>>24)&0xff),
        2,0, 16,0,
        'd','a','t','a', 0,0,0,0
    };
    fwrite(hdr, 1, sizeof hdr, g_cap_fp);
    g_cap_samples = 0;
    return 0;
}

void Audio_CaptureStop(void)
{
    if (!g_cap_fp) return;
    long data_bytes = g_cap_samples * 2;
    long total_bytes = data_bytes + 36;
    /* Patch RIFF size @4 and data size @40. */
    fseek(g_cap_fp, 4, SEEK_SET);
    uint8_t s[4] = {
        (uint8_t)(total_bytes & 0xff), (uint8_t)((total_bytes>>8) & 0xff),
        (uint8_t)((total_bytes>>16) & 0xff), (uint8_t)((total_bytes>>24) & 0xff),
    };
    fwrite(s, 1, 4, g_cap_fp);
    fseek(g_cap_fp, 40, SEEK_SET);
    s[0] = (uint8_t)(data_bytes & 0xff);
    s[1] = (uint8_t)((data_bytes>>8) & 0xff);
    s[2] = (uint8_t)((data_bytes>>16) & 0xff);
    s[3] = (uint8_t)((data_bytes>>24) & 0xff);
    fwrite(s, 1, 4, g_cap_fp);
    fclose(g_cap_fp);
    g_cap_fp = NULL;
    fprintf(stderr, "v8: audio capture: %d samples, %ld bytes\n",
            g_cap_samples, data_bytes);
}
