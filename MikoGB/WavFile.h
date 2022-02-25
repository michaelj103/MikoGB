//
//  WavFile.h
//  AudioMaker
//
//  Created by Michael Brandt on 1/12/16.
//  Copyright © 2016 MJB. All rights reserved.
//

#ifndef WavFile_h
#define WavFile_h

//enumerations
typedef enum {
    AttenuationTypeNone = 0,
    AttenuationTypeLinear,
    AttenuationTypeExponential,
} AttenuationType;

//Data types
typedef struct wav_file WAVFile;        //data structure representing a .wav file
typedef struct wav_sound WAVSound;      //data structure representing an abstract "sound"

//file management
WAVFile *wavfile_open(const char *filename);    //TODO: MJB: options
void wavfile_close(WAVFile *file);

//writing data. returns success
int wavfile_writeData(WAVFile *file, void *data, uint32_t length);

//sounds
WAVSound *wavfile_createSound(WAVFile *file, double duration);
void wavsound_setAttenuation(WAVSound *sound, AttenuationType type, double duration);
int wavsound_addTone(WAVSound *sound, double frequency, float volume, double startTime, double duration);
int wavSound_addSample(WAVSound *sound, int16_t sample, int sampleIdx);
int wavfile_finalizeSound(WAVFile *file, WAVSound *sound, double startTime);

#endif /* WavFile_h */
