#include <iostream>
#include <string>
#include <AudioToolbox/AudioToolbox.h>
#include <cmath>
#include <unistd.h>

#include <sid.h>

int tester();

int main(int argc, char* argv[]) {
    std::cout << "reSID minimal CLI test app" << std::endl;
    std::cout << "Arguments (" << argc - 1 << "):" << std::endl;

    for (int i = 1; i < argc; ++i) {
        std::cout << "  argv[" << i << "] = " << argv[i] << std::endl;
    }

    if (argc == 1) {
        std::cout << "\nUsage: ./test_app [args...]" << std::endl;
        std::cout << "Example: ./test_app hello world" << std::endl;
    }

    using namespace reSID;

    printf( "Sid start\n" );
    std::cout << "SID: tone..." << std::endl;

    SID sid;
    sid.reset();
    sid.set_chip_model( MOS8580 );

    sid.set_sampling_parameters(
        985248,                 // PAL C64 clock
        SAMPLE_INTERPOLATE,
        44100                   // audio sample rate
    );

    tester();

    return 0;
}

static const double sampleRate = 44100.0;
static double phase = 0.0;

static OSStatus render(void *inRef,
                       AudioUnitRenderActionFlags *,
                       const AudioTimeStamp *,
                       UInt32,
                       UInt32 inNumberFrames,
                       AudioBufferList *ioData)
{
    float *buffer = (float *)ioData->mBuffers[0].mData;

    double freq = 620.0;
    double phaseInc = 2.0 * M_PI * freq / sampleRate;

    for (UInt32 i = 0; i < inNumberFrames; i++)
    {
        buffer[i] = (float)sin(phase);
        phase += phaseInc;
    }

    return noErr;
}

int tester()
{
    AudioComponentDescription desc = {};
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent comp = AudioComponentFindNext(nullptr, &desc);

    AudioUnit unit;
    AudioComponentInstanceNew(comp, &unit);

    AURenderCallbackStruct callback = {};
    callback.inputProc = render;
    AudioUnitSetProperty(unit,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input,
                         0,
                         &callback,
                         sizeof(callback));

    AudioStreamBasicDescription format = {};
    format.mSampleRate = sampleRate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved;
    format.mChannelsPerFrame = 1;
    format.mFramesPerPacket = 1;
    format.mBitsPerChannel = 32;
    format.mBytesPerFrame = 4;
    format.mBytesPerPacket = 4;

    AudioUnitSetProperty(unit,
                         kAudioUnitProperty_StreamFormat,
                         kAudioUnitScope_Input,
                         0,
                         &format,
                         sizeof(format));

    AudioUnitInitialize(unit);
    AudioOutputUnitStart(unit);

    sleep(2);

    AudioOutputUnitStop(unit);
    AudioUnitUninitialize(unit);
    AudioComponentInstanceDispose(unit);

    return 0;
}