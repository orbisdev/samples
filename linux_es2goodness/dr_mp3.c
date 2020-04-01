/*
    dr_mp3 example

	2020, masterzorag

    - reading from nfs export (-lnfs)
    - stock dr_mp3 memory operations expects whole song is malloc'd:
      added streaming data in chunked reads using a fixed buffer
    - output to default sound device (-lao)
*/

#include "defines.h"


#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define DR_MP3_NO_STDIO
#define DRMP3_HAVE_SSE 1


#ifdef _LIBAO_
#include <ao/ao.h>
ao_device 		*device;
ao_sample_format format;
int 			 default_driver;
#endif


// output audio samples buffer, s16le
static short *pFrames = NULL;

// reading buffer from opened file, 16k
drmp3_uint8 pData[DRMP3_DATA_CHUNK_SIZE];

drmp3        mp3;    // the main dr_mp3 object
drmp3_config Config; // and its config

size_t filesize;     // of nfs file

int main (int argc, char **argv)
{
    user_init(); // mount nfs export

    char *filename = argv[1];

#ifdef _LIBAO_
    // -- Setup for default audio driver --
    ao_initialize();
    default_driver     = ao_default_driver_id();
    format.bits        = 16;
    format.channels    = 2;
    format.rate        = 44100; // pulseaudio default
    format.byte_format = AO_FMT_LITTLE;
    // -- Open driver --
    device = ao_open_live(default_driver, &format, NULL );
    if (device == NULL) { fprintf(stderr, "Error opening device.\n"); exit (1); }    
#endif

    /* open nfs file, if not give up */
    if( ! user_open(filename) ) return 0;

    filesize = user_stat();
    printf("%s size:%zu\n", filename, filesize);
    
    /* setup mp3 decoder config: set output samplerate ( for pulseaudio )
       setting it, we trigger the use of internal SampleRateConverter ! */
    Config.outputSampleRate = format.rate;

    // drmp3_bool32 drmp3_init_memory(drmp3* pMP3, const void* pData, size_t dataSize, const drmp3_config* pConfig)
    if(!drmp3_init_memory(&mp3, &pData, filesize, &Config))
 // if(!drmp3_init_file(&mp3, "/Archive/PS4-work/OrbisLink/ps4sh/bin/main2.mp3", &Config))
    {
        fprintf(stdout, "Failed to open file\n"); return 0;
    }
#if 0
    drmp3_uint64 totalMP3FrameCount, totalPCMFrameCount;// = 124416000;
    ret = drmp3_get_mp3_and_pcm_frame_count(&mp3, &totalMP3FrameCount, &totalPCMFrameCount);
    printf("totalMP3FrameCount:\t%llu\ntotalPCMFrameCount:\t%llu\n", totalMP3FrameCount, totalPCMFrameCount);       
#endif

    // The drmp3 object is transparent so you can get access to the channel count and sample rate like so:
    printf("%zu, %zu, pcmFramesRemainingInMP3Frame:%u/%u, channels %u, sampleRate: %u\n", 
            mp3.dataSize,
            mp3.dataCapacity,
            mp3.pcmFramesConsumedInMP3Frame,
            mp3.pcmFramesRemainingInMP3Frame, 
            mp3.channels,
            mp3.sampleRate);

// but after a drmp3_get_mp3_and_pcm_frame_count() call, we need to reset !
// so, end clean all and re-init:
/*    
    drmp3_uninit(&mp3);
    user_close();
    memset(pData, 0, sizeof(pData));

    user_open(filename);
    drmp3_init_memory(&mp3, &pData, filesize, NULL);
    
//  clear decoded audio output samples:
//  default max 1152 * 2 channels * sizeof(short)
    //memset(snd, 0, sizeof(snd));
*/

/// instead, we will rely on mp3.streamCursor / filesize !

    #define BUFSIZE  (1152)  // max decoded s16le sample, for a single channel

    if(!pFrames)
        pFrames = calloc(BUFSIZE *2, sizeof(short));

    long long off = 0, count; // track read frames

    while(mp3.streamCursor <= filesize) // eq. until EOF
    {
        count = drmp3_read_pcm_frames_s16(&mp3, BUFSIZE, pFrames /* &snd */ );

        if(count < 0)
        { fprintf(stderr, "Failed to read from file\n"); return 0; }
        else
    	if(count == 0) // all samples decoded
        {
            fprintf(stderr, "End of file\n");
            memset(pFrames, 0, BUFSIZE *2 * sizeof(short)); // silence
            break;
        }
        off += count;

        /* play audio sample after being decoded */
        ao_play(device, (char *)&pFrames[0], BUFSIZE *2 * sizeof(short));
    }
    free(pFrames), pFrames = NULL;

    // -- libao end --
    ao_close(device);
    ao_shutdown();

    drmp3_uninit(&mp3);

    user_end(); // nfs umount export
}
