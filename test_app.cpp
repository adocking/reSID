#include <iostream>
#include <string>
#include <AudioToolbox/AudioToolbox.h>
#include <cmath>
#include <unistd.h>
#include <sid.h>

int tester();

std::vector <short> _buffer;

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

    int sampleRate = 44100;

    SID sid;
    sid.reset();
    sid.set_chip_model( MOS8580 );

    sid.set_sampling_parameters(
        985248,                 // PAL C64 clock
        SAMPLE_INTERPOLATE,
        44100                   // audio sample rate
    );

    // frequency (lower for audible envelope)
    uint16_t freq = 6000;
    sid.write(0x00, freq & 0xff);
    sid.write(0x01, freq >> 8);

    // ADSR (slow and obvious)
    sid.write(0x05, 0xeA);
    sid.write(0x06, 0xF0);

    // gate ON (ONLY ONCE)
    sid.write(0x04, 0x20 | 0x01);

    // volume
    sid.write(0x18, 0x0F);

    // correct cycle math
    int offset = 0;
    int failsafe = 100000;
    int seconds = 5;
    int sampleCount = seconds * sampleRate;
    _buffer.resize( sampleCount );

    while ( offset < sampleCount && failsafe -- ) 
    {
        cycle_count clockDelta = (cycle_count)((985248.0 / 44100.0) * 512);
        int out = sid.clock( clockDelta, _buffer.data() + offset, _buffer.size() - offset, 1 );
        offset += out;

        if ( out == 0 && clockDelta == 0 ) { break; }

        printf( "offset: %i\n", offset );
    }

    tester();

    return 0;
}

static const double sampleRate = 44100.0;
static double phase = 0.0;
int _offset = 0;

static OSStatus render(void *inRef,
                       AudioUnitRenderActionFlags *,
                       const AudioTimeStamp *,
                       UInt32,
                       UInt32 inNumberFrames,
                       AudioBufferList *ioData)
{
    float *buffer = (float *)ioData->mBuffers[0].mData;

    for (UInt32 i = 0; i < inNumberFrames; i++)
    {
        buffer[i] = _buffer[(_offset + i) % _buffer.size()] / 32768.f;
    }

    _offset += inNumberFrames;
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

    sleep(5);

    AudioOutputUnitStop(unit);
    AudioUnitUninitialize(unit);
    AudioComponentInstanceDispose(unit);

    return 0;
}