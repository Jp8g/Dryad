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

typedef struct dry_internal {

#ifdef DRYAD_ALSA
    snd_pcm_t* pcm;
#endif

} dry_internal;

struct dry_audio_stream {
    dry_internal* internal;
    
    pthread_t thread;
    dry_write_callback writeCallback;
    void* userData;

    uint64_t bufferSize;
    uint64_t periodSize;
    uint32_t sampleRate;
    uint32_t channels;

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
void dry_close_audio_stream(dry_audio_stream* audioStream);

#ifdef __cplusplus
}
#endif

#endif