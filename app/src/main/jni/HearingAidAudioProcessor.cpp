//
// Created by frede on 10.01.2018.
//

#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES.h>
#include <jni.h>
#include <string.h>
#include "HearingAidAudioProcessor.h"
#include <SuperpoweredFrequencyDomain.h>
#include <malloc.h>
#include <SuperpoweredCPU.h>
#include <SuperpoweredSimple.h>

static HearingAidAudioProcessor *jniInstance = NULL;

// static SuperpoweredFrequencyDomain *frequencyDomain;
// static float *magnitudeLeft, *magnitudeRight, *phaseLeft, *phaseRight, *fifoOutput, *inputBufferFloat;
// static int fifoOutputFirstSample, fifoOutputLastSample, stepSize, fifoCapacity;

// #define FFT_LOG_SIZE 11 // 2^11 = 2048

static bool
audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
    return ((HearingAidAudioProcessor *) clientdata)->process(audioIO,
                                                              (unsigned int) numberOfSamples);
}

HearingAidAudioProcessor::HearingAidAudioProcessor(unsigned int samplerate,
                                                   unsigned int buffersize, float *frequencies) {
    /* frequencyDomain = new SuperpoweredFrequencyDomain(
            FFT_LOG_SIZE); // This will do the main "magic".
    stepSize = frequencyDomain->fftSize /
               4; // The default overlap ratio is 4:1, so we will receive this amount of samples from the frequency domain in one step.

    // Frequency domain data goes into these buffers:
    magnitudeLeft = (float *) malloc(frequencyDomain->fftSize * sizeof(float));
    magnitudeRight = (float *) malloc(frequencyDomain->fftSize * sizeof(float));
    phaseLeft = (float *) malloc(frequencyDomain->fftSize * sizeof(float));
    phaseRight = (float *) malloc(frequencyDomain->fftSize * sizeof(float));

    // Time domain result goes into a FIFO (first-in, first-out) buffer
    fifoOutputFirstSample = fifoOutputLastSample = 0;
    fifoCapacity = stepSize *
                   100; // Let's make the fifo's size 100 times more than the step size, so we save memory bandwidth.
    fifoOutput = (float *) malloc(fifoCapacity * sizeof(float) * 2 + 128);

    inputBufferFloat = (float *) malloc(buffersize * sizeof(float) * 2 + 128); */

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true,
                                                 audioProcessing, this,
            // SL_ANDROID_RECORDING_PRESET_GENERIC,
                                                 SL_ANDROID_RECORDING_PRESET_UNPROCESSED,
            // SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION,
                                                 SL_ANDROID_STREAM_MEDIA, 0);

    superpoweredEq = new SuperpoweredNBandEQ(samplerate, frequencies);
    onPlayPause(false);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
bool HearingAidAudioProcessor::process(short int *output, unsigned int numberOfSamples) {
    // return if eq is disabled
    if (!eqEnabled)
        return true;

    float *inputBufferFloat;
    SuperpoweredShortIntToFloat(output, inputBufferFloat,
                                numberOfSamples); // Converting the 16-bit integer samples to 32-bit floating point.
    __android_log_print(ANDROID_LOG_INFO, "HearingAid", "????? Before eq... ?????");
    get_superpowered_eq()->process(inputBufferFloat, inputBufferFloat, numberOfSamples);
    __android_log_print(ANDROID_LOG_INFO, "HearingAid", "????? After eq... ?????");
    SuperpoweredFloatToShortInt(inputBufferFloat, output,
                                numberOfSamples);

    return true;

    /*
    SuperpoweredShortIntToFloat(output, inputBufferFloat,
                                numberOfSamples); // Converting the 16-bit integer samples to 32-bit floating point.frequencyDomain->addInput(inputBufferFloat,
                              numberOfSamples); // Input goes to the frequency domain.

    // In the frequency domain we are working with 1024 magnitudes and phases for every channel (left, right), if the fft size is 2048.
    while (frequencyDomain->timeDomainToFrequencyDomain(magnitudeLeft, magnitudeRight, phaseLeft,
                                                        phaseRight)) {
        // You can work with frequency domain data from this point.

        for (float *i1 = magnitudeLeft; i1 <= magnitudeLeft + 80; i1++) {
            int w = 1;
        }

        for (float *i2 = magnitudeRight; i2 <= magnitudeRight + 80; i2++) {
            int w = 1;
        }*/

    // This is just a quick example: we remove the magnitude of the first 20 bins, meaning total bass cut between 0-430 Hz.
    // memset(magnitudeLeft, 0, 80);
    // memset(magnitudeRight, 0, 80);

    /*
        int floatsToProcess = static_cast<int>(max_frequency / bucket_size);
        for (float *ptr = magnitudeLeft; ptr < magnitudeLeft + floatsToProcess; ptr++) {
            *ptr = *ptr * get_eq_index_for_frequency(magnitudeLeft, ptr);
        }

        for (float *ptr = magnitudeRight; ptr < magnitudeRight + floatsToProcess; ptr++) {
            *ptr = *ptr * get_eq_index_for_frequency(magnitudeRight, ptr);
        }

        // max bin: 3720
        // max eq value: 400
        // max bin in floats: 930

        // We are done working with frequency domain data. Let's go back to the time domain.

        // Check if we have enough room in the fifo buffer for the output. If not, move the existing audio data back to the buffer's beginning.
        if (fifoOutputLastSample + stepSize >=
            fifoCapacity) { // This will be true for every 100th iteration only, so we save precious memory bandwidth.
            int samplesInFifo = fifoOutputLastSample - fifoOutputFirstSample;
            if (samplesInFifo > 0)
                memmove(fifoOutput, fifoOutput + fifoOutputFirstSample * 2,
                        samplesInFifo * sizeof(float) * 2);
            fifoOutputFirstSample = 0;
            fifoOutputLastSample = samplesInFifo;
        };

        // Transforming back to the time domain.
        frequencyDomain->frequencyDomainToTimeDomain(magnitudeLeft, magnitudeRight, phaseLeft,
                                                     phaseRight,
                                                     fifoOutput + fifoOutputLastSample * 2);
        frequencyDomain->advance();
        fifoOutputLastSample += stepSize;
    };

    // If we have enough samples in the fifo output buffer, pass them to the audio output.
    if (fifoOutputLastSample - fifoOutputFirstSample >= numberOfSamples) {
        SuperpoweredFloatToShortInt(fifoOutput + fifoOutputFirstSample * 2, output,
                                    numberOfSamples);
        fifoOutputFirstSample += numberOfSamples;
        return true;
    } else return false; */
}

#pragma clang diagnostic pop

void HearingAidAudioProcessor::onPlayPause(bool play) {
    if (play)
        start();
    else
        stop();
}

void HearingAidAudioProcessor::onForeground() {
    audioSystem->onForeground();
}

void HearingAidAudioProcessor::onBackground() {
    audioSystem->onBackground();
}

void HearingAidAudioProcessor::start() {
    SuperpoweredCPU::setSustainedPerformanceMode(true);
    audioSystem->start();
}

void HearingAidAudioProcessor::stop() {
    SuperpoweredCPU::setSustainedPerformanceMode(false);
    audioSystem->stop();
}

extern "C" JNIEXPORT void
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_HearingAidAudioProcessor(
        JNIEnv *javaEnvironment, jobject __unused obj, jint samplerate,
        jint buffersize,
        jfloatArray frequencies/*, jstring apkPath, jint fileAoffset, jint fileAlength, jint fileBoffset, jint fileBlength*/) {
    // const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);

    jniInstance = new HearingAidAudioProcessor((unsigned int) samplerate,
                                               (unsigned int) buffersize,
                                               javaEnvironment->GetFloatArrayElements(frequencies,
                                                                                      NULL));
    // javaEnvironment->ReleaseStringUTFChars(apkPath, path);
}

extern "C" JNIEXPORT void
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_onPlayPause(
        JNIEnv *__unused javaEnvironment, jobject __unused obj, jboolean play) {
    jniInstance->onPlayPause(play);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_onBackground(JNIEnv *env,
                                                                                 jobject instance) {
    jniInstance->onBackground();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_onForeground(JNIEnv *env,
                                                                                 jobject instance) {
    jniInstance->onForeground();
}

extern "C" JNIEXPORT void
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_eqEnabled(
        JNIEnv *__unused javaEnvironment, jobject __unused obj, jboolean eqEnabled) {
    jniInstance->enableEQ(eqEnabled);
}

extern "C"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
JNIEXPORT void JNICALL
Java_com_github_vatbub_hearingaid_backend_HearingAidPlaybackService_setEQ(JNIEnv *env,
                                                                          jobject instance,
                                                                          jint index,
                                                                          jfloat gainDecibels) {
    jniInstance->get_superpowered_eq()->setBand(index, gainDecibels);
}
#pragma clang diagnostic pop
