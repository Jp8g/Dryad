#ifndef DRYAD
#define DRYAD

#define _GNU_SOURCE 1

#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
#include <atomic>
#else
#include <stdatomic.h>
#endif

typedef struct dry_audio_stream dry_audio_stream;

typedef void (*dry_write_callback)(float*, dry_audio_stream*);

typedef snd_pcm_t dry_pcm;

struct dry_audio_stream {
    dry_pcm* pcm;
    dry_write_callback writeCallback;
    uint64_t bufferSize;
    uint64_t periodSize;
    void* userData;
    pthread_t thread;

    uint32_t channels;
    uint32_t sampleRate;

    #ifdef __cplusplus
    std::atomic<bool> active;
    #else
    _Atomic bool active;
    #endif
};

#ifdef __cplusplus
extern "C" {
#endif

dry_audio_stream* dry_create_audio_stream(dry_write_callback writeCallback, uint32_t channels, uint32_t sampleRate, uint64_t bufferSize, uint64_t periodSize, void* userData);
void DryadCloseAudioStream(dry_audio_stream* audioStream);

#ifdef __cplusplus
}
#endif

#endif