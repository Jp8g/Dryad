#include "../include/dryad.h"

void* audioLoop(void* args) {
    DryadAudioStream* audioStream = (DryadAudioStream*)args;
    float* outBuffer = malloc(audioStream->periodSize * audioStream->channels * sizeof(float));

    while (atomic_load(&audioStream->active)) {
        snd_pcm_sframes_t avail = snd_pcm_avail_update((snd_pcm_t*)audioStream->pcm);

        if (avail < 0) {
            snd_pcm_prepare((snd_pcm_t*)audioStream->pcm);
            continue;
        }

        if (avail >= audioStream->periodSize) {
            for (uint32_t i = 0; i < audioStream->periodSize * audioStream->channels; i++) outBuffer[i] = 0.0f;

            audioStream->writeCallback(outBuffer, audioStream);

            snd_pcm_sframes_t written = snd_pcm_writei((snd_pcm_t*)audioStream->pcm, outBuffer, audioStream->periodSize);

            if (written < 0) {
                written = snd_pcm_recover((snd_pcm_t*)audioStream->pcm, written, 0);
            }

            continue;
        }
        
        snd_pcm_wait(audioStream->pcm, 50);
    }

    free(outBuffer);

    return NULL;
}

DryadAudioStream* DryadCreateAudioStream(DryadWriteCallback writeCallback, uint32_t channels, uint32_t sampleRate, uint64_t bufferSize, uint64_t periodSize, void* userData) {
    snd_pcm_t* pcm;
    snd_pcm_hw_params_t* params;
    DryadAudioStream* audioStream;

    snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(pcm, params);
    snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(pcm, params, channels);
    snd_pcm_hw_params_set_rate(pcm, params, sampleRate, 0);
    snd_pcm_hw_params_set_buffer_size_near(pcm, params, &bufferSize);
    snd_pcm_hw_params_set_period_size_near(pcm, params, &periodSize, NULL);
    snd_pcm_hw_params(pcm, params);
    snd_pcm_hw_params_free(params);

    audioStream = malloc(sizeof(DryadAudioStream));

    audioStream->pcm = pcm;
    audioStream->writeCallback = writeCallback;

    audioStream->channels = channels;
    audioStream->sampleRate = sampleRate;
    audioStream->bufferSize = bufferSize;
    audioStream->periodSize = periodSize;
    audioStream->userData = userData;
    atomic_store(&audioStream->active, true);

    pthread_create(&audioStream->thread, NULL, audioLoop, audioStream);

    return audioStream;
}

void DryadCloseAudioStream(DryadAudioStream* audioStream) {
    atomic_store(&audioStream->active, false);

    pthread_join(audioStream->thread, NULL);

    snd_pcm_close(audioStream->pcm);
}