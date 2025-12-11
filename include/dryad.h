#ifndef DRYAD
#define DRYAD

#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
#include <atomic>
#else
#include <stdatomic.h>
#endif

typedef struct DryadAudioStream DryadAudioStream;

typedef void (*DryadWriteCallback)(float*, DryadAudioStream*);

typedef snd_pcm_t DryadPCM;

struct DryadAudioStream {
    DryadPCM* pcm;
    DryadWriteCallback writeCallback;
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

DryadAudioStream* DryadCreateAudioStream(DryadWriteCallback writeCallback, uint32_t channels, uint32_t sampleRate, uint64_t bufferSize, uint64_t periodSize, void* userData);
void DryadCloseAudioStream(DryadAudioStream* audioStream);

#ifdef __cplusplus
}
#endif

#endif