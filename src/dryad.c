#include <dryad/dryad.h>

#ifdef DRYAD_ALSA
void* dry_audio_loop(void* args) {
    dry_audio_stream* audioStream = (dry_audio_stream*)args;
    dry_internal* internal = (dry_internal*)audioStream->internal;

    float* outBuffer = malloc(audioStream->periodSize * audioStream->channels * sizeof(float));

    while (atomic_load(&audioStream->active)) {
        snd_pcm_sframes_t avail = snd_pcm_avail_update(internal->pcm);

        if (avail < 0) {
            snd_pcm_prepare(internal->pcm);
            continue;
        }

        if ((uint64_t)avail >= audioStream->periodSize) {
            memset(outBuffer, 0, audioStream->periodSize * audioStream->channels * sizeof(float));

            audioStream->writeCallback(outBuffer, audioStream);

            snd_pcm_sframes_t written = snd_pcm_writei(internal->pcm, outBuffer, audioStream->periodSize);

            if (written < 0) {
                written = snd_pcm_recover(internal->pcm, written, 0);
            }

            continue;
        }
        
        snd_pcm_wait(internal->pcm, (audioStream->periodSize * 1000) / audioStream->sampleRate);
    }

    free(outBuffer);

    return NULL;
}

void* dry_thread_entry(void* args) {
    dry_audio_stream* audioStream = args;

    snd_pcm_t* pcm;
    snd_pcm_hw_params_t* params;

    snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    audioStream->internal->pcm = pcm;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm, params);
    snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(pcm, params, audioStream->channels);
    snd_pcm_hw_params_set_rate(pcm, params, audioStream->sampleRate, 0);
    snd_pcm_hw_params_set_buffer_size_near(pcm, params, &audioStream->bufferSize);
    snd_pcm_hw_params_set_period_size_near(pcm, params, &audioStream->periodSize, NULL);
    snd_pcm_hw_params(pcm, params);
    snd_pcm_prepare(pcm);

    return dry_audio_loop(args);
}

dry_audio_stream* dry_create_audio_stream(dry_write_callback writeCallback, uint32_t channels, uint32_t sampleRate, uint64_t bufferSize, uint64_t periodSize, void* userData) {
    dry_audio_stream* audioStream;

    audioStream = malloc(sizeof(dry_audio_stream));
    audioStream->internal = malloc(sizeof(dry_internal));
    audioStream->writeCallback = writeCallback;

    audioStream->channels = channels;
    audioStream->sampleRate = sampleRate;
    audioStream->bufferSize = bufferSize;
    audioStream->periodSize = periodSize;
    audioStream->userData = userData;
    atomic_store(&audioStream->active, true);

    pthread_create(&audioStream->thread, NULL, dry_thread_entry, audioStream);

    return audioStream;
}

void dry_close_audio_stream(dry_audio_stream* audioStream) {
    atomic_store(&audioStream->active, false);

    snd_pcm_drop(audioStream->internal->pcm);
    pthread_join(audioStream->thread, NULL);

    snd_pcm_close(audioStream->internal->pcm);
    free(audioStream->internal);
    free(audioStream);
}
#endif