//
//  WavFile.c
//  AudioMaker
//
//  Created by Michael Brandt on 1/12/16.
//  Copyright © 2016 MJB. All rights reserved.
//

//Description of wav file at:
//http://soundfile.sapp.org/doc/WaveFormat/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "WavFile.h"

#define WAVFILE_SAMPLES_PER_SECOND  44100
#define WAVFILE_BITS_PER_SAMPLE     16
#define WAVFILE_NUM_CHANNELS        1
#define MAX_VOLUME_8                INT8_MAX
#define MAX_VOLUME_16               INT16_MAX
#define MIN_VOLUME_16               INT16_MIN
#define MAX_VOLUME_32               INT32_MAX
#define MAX_VOLUME_64               INT64_MAX

#define MAX(A,B) (A)>(B)?(A):(B)
#define MIN(A,B) (A)<(B)?(A):(B)

//TODO: MJB: store various configuration options here
struct wav_file {
    FILE *stream;
    uint32_t samplesPerSecond;
    uint8_t bitsPerChannel;
    uint8_t numChannels;
};

struct wav_sound {
    double duration;                //duration in seconds
    int sampleCount;                //corresponding sample count
    int sampleBitDepth;             //sample bit depth
    AttenuationType attenuation;    //how added tones attenuate
    double attenuationDuration;     //how long added tones take to attenuate to 0
    void *soundData;                //data in linear PCM. bit depth is sampleBitDepth
};

typedef struct wavfile_header {
    char        riff_tag[4];        //"RIFF" chunk descriptor
    uint32_t    riff_length;        //chunk size: size of entire file in bytes minus riff_tag (4) and riff_length (4)
    char        wave_tag[4];        //Format. must be "WAVE"
    char        fmt_tag[4];         //sub-chunk 1: "fmt "
    uint32_t    fmt_length;         //16 for linear PCM. It's the size of the rest of sub-chunk 1
    uint16_t    audio_format;       //1 for linear PCM
    uint16_t    num_channels;       //number of channels
    uint32_t    sample_rate;        //samples per second (Hz)
    uint32_t    byte_rate;          //== SampleRate * NumChannels * BitsPerSample/8
    uint16_t    block_align;        //== NumChannels * BitsPerSample/8
    uint16_t    bits_per_sample;    //bit depth
    char        data_tag[4];        //sub-chunk 2: "data"
    uint32_t    data_length;        //length of data written in bytes following this header
} WAVHeader;

WAVFile *wavfile_open(const char *filename) {
    WAVHeader header;
    
    int samples_per_second = WAVFILE_SAMPLES_PER_SECOND;
    int bits_per_sample = WAVFILE_BITS_PER_SAMPLE;
    
    strncpy(header.riff_tag, "RIFF", 4);
    strncpy(header.wave_tag, "WAVE", 4);
    strncpy(header.fmt_tag, "fmt ", 4);
    strncpy(header.data_tag, "data", 4);
    
    header.riff_length = 0;     //placeholder. will be rewritten on close
    header.fmt_length = 16;     //constant for WAV + linear PCM
    header.audio_format = 1;    //constant for WAV + linear PCM
    header.num_channels = WAVFILE_NUM_CHANNELS;
    header.sample_rate = samples_per_second;
    header.byte_rate = samples_per_second * (bits_per_sample / 8);
    header.block_align = bits_per_sample / 8;
    header.bits_per_sample = bits_per_sample;
    header.data_length = 0;     //placeholder. will be rewritten on close
    
    FILE *file = fopen(filename, "w+");
    if (!file) {
        return NULL;
    }
    
    fwrite(&header, sizeof(header), 1, file);
    fflush(file);
    
    WAVFile *newFile = calloc(1, sizeof(WAVFile));
    newFile->stream = file;
    
    return newFile;
    
}

int wavfile_writeData(WAVFile *file, void *data, uint32_t length) {
    if (!file) {
        return 0;
    }
    size_t written = fwrite(data, WAVFILE_BITS_PER_SAMPLE / 8, length, file->stream);
    return (written == length);
}

//at completion, write the data length and riff length
void wavfile_close(WAVFile *wavFile) {
    if (!wavFile) {
        return;
    }
    
    FILE *file = wavFile->stream;
    if (!file) {
        return;
    }
    
    //TODO: MJB: track all the info needed in the WAVFile data structure
    long file_length = ftell(file);
    if (file_length > UINT32_MAX) {
        fprintf(stderr, "exceeded maximum data size which is %u\n", UINT32_MAX);
    }
    
    //write length of data, which is the total
    uint32_t data_length = (uint32_t)file_length - sizeof(WAVHeader);
    fseek(file, sizeof(WAVHeader) - sizeof(uint32_t), SEEK_SET);
    fwrite(&data_length, sizeof(data_length), 1, file);
    
    //write riff_length, which is length of the whole file minus 8 bytes
    uint32_t riff_length = (uint32_t)file_length - 8;
    fseek(file, 4, SEEK_SET);   //riff length offset is 4 bytes
    fwrite(&riff_length, sizeof(riff_length), 1, file);
    
    fclose(file);
    free(wavFile);
}

//sounds
WAVSound *wavfile_createSound(WAVFile *file, double duration) {
    WAVSound *newSound = calloc(1, sizeof(WAVSound));
    if (!newSound) {
        return NULL;
    }
    newSound->duration = duration;
    newSound->attenuation = AttenuationTypeNone;
    newSound->attenuationDuration = 0.0;
    //TODO: MJB: get info for the sound from the file
    //for now, assume defaults
    int sampleCount = duration * WAVFILE_SAMPLES_PER_SECOND;
    newSound->sampleCount = sampleCount;
    newSound->sampleBitDepth = WAVFILE_BITS_PER_SAMPLE;
    newSound->soundData = calloc(sampleCount, WAVFILE_BITS_PER_SAMPLE/8);
    if (!newSound->soundData) {
        free(newSound);
        return NULL;
    }
    
    return newSound;
}

void wavfile_destroySound(WAVSound *sound) {
    if (!sound) {
        return;
    }
    
    free(sound->soundData);
    free(sound);
}

int wavfile_finalizeSound(WAVFile *file, WAVSound *sound, double startTime) {
    if (!sound) {
        return 0;
    }
    
    int success = 1;
    
    if (sound->sampleBitDepth != 16) {
        fprintf(stderr, "Currently only 16 bit samples supported\n");
        success = 0;
    }
    
    if (startTime != 0.0) {
        fprintf(stderr, "Currently startTime must be 0.0\n");
        success = 0;
    }
    
    if (success) {
        success = wavfile_writeData(file, sound->soundData, sound->sampleCount);
    }
    
    wavfile_destroySound(sound);
    return success;
}

void _addSample(void *buffer, int idx, int bitDepth, int64_t sample) {
    if (bitDepth == 8) {
        int8_t *data = (int8_t *)buffer;
        data[idx] += (int8_t)sample;
    } else if (bitDepth == 16) {
        int16_t *data = (int16_t *)buffer;
        int64_t curr = (int64_t)data[idx];
        int64_t new = MIN(MAX_VOLUME_16, curr + sample);
        new = MAX(MIN_VOLUME_16, new);
        data[idx] = (int16_t)new;
    } else if (bitDepth == 32) {
        int32_t *data = (int32_t *)buffer;
        data[idx] += (int32_t)sample;
    } else if (bitDepth == 64) {
        int64_t *data = (int64_t *)buffer;
        data[idx] += sample;
    }
}

double _attenuationFactor(WAVSound *sound, double t) {
    AttenuationType type = sound->attenuation;
    double duration = sound->attenuationDuration;
    double factor = 1.0;    //none by default
    if (type == AttenuationTypeLinear) {
        double p = MIN(duration, t);
        factor = 1.0 - (p / duration);
    } else if (type == AttenuationTypeExponential) {
        //roughly ln(32000). When the exponent is more than this, it will be negligible
        static const double target = 10.3;
        double c = target / duration;
        factor = 1.0 / exp(c * t);
    }
    return factor;
}

int wavsound_addTone(WAVSound *sound, double frequency, float volume, double startTime, double duration) {
    if (!sound) {
        return 0;
    }
    
    if (startTime < 0.0 || duration < 0.0 || (startTime + duration) > sound->duration) {
        fprintf(stderr, "Invalid time range (%lf %lf) sound duration %lf\n", startTime, duration, sound->duration);
        return 0;
    }
    
    if (volume < 0.0 || volume > 1.0) {
        fprintf(stderr, "Volume must be from 0.0 to 1.0\n");
        return 0;
    }
    
    if (sound->sampleBitDepth != 16) {
        fprintf(stderr, "Currently only 16 bit samples supported\n");
        return 0;
    }
    
    //TODO: MJB: use file values instead here.
    int startIdx = startTime * WAVFILE_SAMPLES_PER_SECOND;
    int endIdx = (startTime + duration) * WAVFILE_SAMPLES_PER_SECOND;
    int64_t amplitude = MAX_VOLUME_16 * volume;
    void *buffer = sound->soundData;
    for (int i = startIdx; i < endIdx; i++) {
        double t = (double)i / WAVFILE_SAMPLES_PER_SECOND;
        double aFact = _attenuationFactor(sound, t-startTime);
        int64_t value = amplitude * (aFact * sin(frequency * t * 2 * M_PI));
        _addSample(buffer, i, sound->sampleBitDepth, value);
    }
    return 1;
}

int wavSound_addSample(WAVSound *sound, int16_t sample, int sampleIdx) {
    if (!sound) {
        return 0;
    }
    
    if (sound->sampleBitDepth != 16) {
        fprintf(stderr, "Currently only 16 bit samples supported\n");
        return 0;
    }
    
    void *buffer = sound->soundData;
    _addSample(buffer, sampleIdx, sound->sampleBitDepth, sample);
    return 1;
}

void wavsound_setAttenuation(WAVSound *sound, AttenuationType type, double duration) {
    if (!sound) {
        return;
    }
    
    if (duration > 0.0) {
        sound->attenuation = type;
        sound->attenuationDuration = duration;
    }
}

