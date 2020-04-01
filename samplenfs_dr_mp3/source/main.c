#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <systemservice.h>
#include <orbis2d.h>
#include <orbisPad.h>
#include <orbisKeyboard.h>
#include <orbisAudio.h>
#include <ps4link.h>
#include <debugnet.h>
#include <orbisNfs.h>
#include <orbisGl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdarg.h>


OrbisPadConfig *confPad;
bool flag=true;
typedef struct OrbisGlobalConf
{
	Orbis2dConfig *conf;
	OrbisPadConfig *confPad;
	OrbisAudioConfig *confAudio;
	OrbisKeyboardConfig *confKeyboard;
	ps4LinkConfiguration *confLink;
	int orbisLinkFlag;
	debugNetConfiguration *confDebug;
	OrbisNfsConfig *confNfs;
}OrbisGlobalConf;

OrbisGlobalConf *myConf;

int screenStatus;

void updateController()
{
    int ret;
    unsigned int buttons=0;
    ret=orbisPadUpdate();
    if(ret==0)
    {
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUG,"Combo L2R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUG,"Combo L1R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L1|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUG,"Combo L1R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R1) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUG,"Combo L2R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_UP) || orbisPadGetButtonHold(ORBISPAD_UP))
        {
            debugNetPrintf(DEBUG,"Up pressed\n");
            //pad_special(2);
            
        }
        if(orbisPadGetButtonPressed(ORBISPAD_DOWN) || orbisPadGetButtonHold(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUG,"Down pressed\n");
            //pad_special(3);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_RIGHT) || orbisPadGetButtonHold(ORBISPAD_RIGHT))
        {
            debugNetPrintf(DEBUG,"Right pressed\n");
            //pad_special(1);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_LEFT) || orbisPadGetButtonHold(ORBISPAD_LEFT))
        {
            debugNetPrintf(DEBUG,"Left pressed\n");
            //pad_special(0);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
        {
            debugNetPrintf(DEBUG,"Triangle pressed exit\n");
            
        }
        if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
        {
            debugNetPrintf(DEBUG,"Circle pressed\n");
            orbisAudioResume(0);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
        {
            debugNetPrintf(DEBUG,"Cross pressed rand color\n");
            //orbisAudioStop();
        }
        if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
        {
            debugNetPrintf(DEBUG,"Square pressed\n");
            orbisAudioPause(0);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1))
        {
            debugNetPrintf(DEBUG,"L1 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L2))
        {
            debugNetPrintf(DEBUG,"L2 pressed\n");
            flag=0;  // exit app
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R1))
        {
            debugNetPrintf(DEBUG,"R1 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R2))
        {
            debugNetPrintf(DEBUG,"R2 pressed\n");

           	char *t = calloc(256, 1); //selected_entry(t);

           	debugNetPrintf(DEBUG,"t:'%s'\n", t);

            /* dr_mp3_Loop(t); */

            free(t), t = NULL;
        }
    }
}


char path[256];
int notSelected=1;
int comeBack=0;

void finishApp()
{
	//orbisAudioFinish();
	//orbisKeyboardFinish();
	orbisGlFinish();	// needs orbisFile
	orbisPadFinish();
	orbisNfsFinish();
	ps4LinkFinish();
}

static bool initAppGl()
{
	int ret;
	ret=orbisGlInit(ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
	if(ret>0)
	{
		glViewport(0, 0, ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
		ret=glGetError();
		if(ret)
		{
			debugNetPrintf(ERROR,"[NFSSAMPLE] %s glViewport failed: 0x%08X\n",__FUNCTION__,ret);
			return false;
		}
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue RGBA
		ret=glGetError();
		if(ret)
		{
			debugNetPrintf(ERROR,"[NFSSAMPLE] %s glClearColor failed: 0x%08X\n",__FUNCTION__,ret);
			return false;
		}
		return true;
	}
	return false;
}

bool initApp()
{
	int ret;
	//orbisNfsInit(NFSEXPORT);
	//orbisFileInit();
	sceSystemServiceHideSplashScreen();
	
	ret=orbisPadInitWithConf(myConf->confPad);
	if(ret)
	{
		confPad=orbisPadGetConf(); 
	}
	else
	{
		return false;
	}
	if(!initAppGl())
		return false;
	return true;
}

/// for timing, fps
#define WEN  (4096)
unsigned int frame   = 1,
             time_ms = 0;



/// dr_mp3
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define DR_MP3_NO_STDIO
#define DRMP3_HAVE_SSE 1

//static short *pFrames = NULL; // output audio samples buffer, s16le

drmp3_uint8  pData[DRMP3_DATA_CHUNK_SIZE]; // reading buffer from opened file, 16k

drmp3        mp3;      // the main dr_mp3 object
drmp3_config Config;   // and its config
size_t       filesize, // of nfs file
             off;      // offset in filesize

int    main_ch = 0, //dr_mp3_playint_channel
       fill    = 0;
int m_bPlaying = 0,
      numframe = 1;

static short play_buf[(1024 *2) *4], // small fixed buffer to append decoded pcm s16le samples
                  snd[1152 *2];      // small fixed buffer to store a single audio frame of decoded samples

#define BUFSIZE  (1152) // max decoded s16le sample, for a single channel

// now read frames and decode to s16le samples, from callback
static int dr_mp3_decode(void)
{
    if(mp3.pData == NULL) return -1;

    if(mp3.streamCursor <= filesize)  
    {
    //  drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, framesToRead, pFrames);
        int count = drmp3_read_pcm_frames_s16(&mp3, BUFSIZE, &snd[0] /* &snd */ );
        //count = nfs_pread(nfs, fh, off, count, buf2);
        if(count < 0)
        { fprintf(stderr, "Failed to read from file\n"); return 0; }
        else
        if(count == 0) // all samples decoded
        {
           //flush with silence
           memset(&snd, 0, BUFSIZE *2 * sizeof(short)); return -1;
        }
        // sample a texture from
        // static short *pFrames

        off += count;

        /* play audio sample after being decoded */
        if( ! (mp3.streamCursor %1000) )
        {
            fprintf(stderr, "audioout play %4d pFrames: %p, %zub %zu %zu %lu %.3f%%\n", 
                                    count, &snd[0], BUFSIZE *2 * sizeof(short), 
                                    off, filesize, mp3.streamCursor,
                                    (float)((float) mp3.streamCursor / (float)filesize) * 100.f );
        }
    }
    return off;
}

static void audio_PlayCallback(OrbisAudioSample *_buf2, unsigned int length, void *pdata)
{
    //int  handle = orbisAudioGetHandle(main_ch); // unused
    const short *_buf       = (short *)_buf2;
    const int     play_size = length *2 * sizeof(short);

    if(m_bPlaying == 1)
    {
        static int decoded;

        while(fill < (1024 *2))
        {
            //fprintf(stderr, "fill: %4d samples, %4lub, play_buf: %lub\n", fill, fill * sizeof(short), sizeof(play_buf));
            decoded = dr_mp3_decode();

            if (decoded < 1) /* no more samples */
            {
                if( !(numframe %500) ) debugNetPrintf(DEBUG, "Playing, but no audio samples decoded yet...\n");
                memset(&snd[0], 0, sizeof(snd)); // fill with silence
                // trigger next close all audio stuff ?
                m_bPlaying = 0; //flag = 0; // break main loop
            }
            else /* append to play buffer */
            {
                memcpy(&play_buf[fill], &snd[0], 4608);  // we are doing in two step, sort of waste
                fill += 1152 /* decoded samples */ *2 /* channels */;
            }
        }
        if( !(numframe %500) ) debugNetPrintf(DEBUG, "play_buf samples: %d\n", fill);

        /* write to audio device buffers */
        memcpy((void*)_buf, &play_buf[0], play_size); // 1024 samples per channel, = 4096b

        /* move remaining samples to head of buffer */
        memcpy(&play_buf[0], &play_buf[1024 *2], sizeof(play_buf) - play_size);

        fill -= 1024 *2; // we played 1024 samples, per channel

        if(fill < 0)
        { 
            fill = 0;
            if( !(numframe %500) ) debugNetPrintf(DEBUG, "no audio frames to play, fill: %d\n", fill);
            m_bPlaying = 0; flag = 0;
        }
    }
    else // Not Playing, so clear buffer
    {
        if( !(numframe %60) ) { debugNetPrintf(DEBUG, "Inside audio_PlayCallback, not playing m_bPlaying: %d\n", m_bPlaying); }

        /* write silence to audio device buffers */
        memset((void *)_buf, 0, play_size);
        usleep(1000);
    }
    numframe++;
   //sceAudioOutOutput(handle, NULL); // wait for last data, useless
    usleep(1000);
}

int main(int argc, char *argv[])
{
	int ret;
	uintptr_t intptr=0;
	sscanf(argv[1],"%p",&intptr);
	myConf=(OrbisGlobalConf *)intptr;
	
	debugNetInitWithConf(myConf->confDebug);
	orbisNfsInitWithConf(myConf->confNfs);

	debugNetPrintf(INFO,"[NFSSAMPLE] NFS sample for Playstation 4 Retail fucking yeah!!!\n");
	sleep(1);
	// init libraries
	flag=initApp();

#if 1	
    // reinit all audio stuff
    ret=orbisAudioInitWithConf(myConf->confAudio);
    sleep(1);
	orbisAudioFinish();

    debugNetPrintf(INFO,"orbisAudioFinish, now reinit audio... some sleep(s)\n"); sleep(2);

  //ret = orbisAudioInit();                               debugNetPrintf(INFO, "ret: %d\n", ret); //1
    ret = orbisAudioInitChannel(main_ch, 1024, 48000, 1); debugNetPrintf(INFO, "ret: %d\n", ret); //0

    // myConf->confAudio seems there yet

    /* open nfs file, if not give up */
    user_init(); // mount nfs export

    char *filename = "main.mp3";

    if( ! user_open(filename) ) return 0;

    filesize = user_stat();
    printf("%s size:%zu\n", filename, filesize);
    /* setup mp3 decoder config: set output samplerate (for libao), by
       setting it, we trigger the use of internal SampleRateConverter ! */
    Config.outputSampleRate = 48000;

 // if(!drmp3_init_file(&mp3, "/path/to/local/main.mp3", &Config))
    if(!drmp3_init_memory(&mp3, &pData, filesize, &Config))
        { fprintf(stdout, "Failed to open file\n"); return -1; }

#endif
    // attach audio callback
    orbisAudioSetCallback(main_ch, audio_PlayCallback, 0);

    // start audio callback, and play
    orbisAudioResume(0), m_bPlaying = 1;


    /// reset timer
    time_ms = get_time_ms();

	// mp3 audio, from nfs export
    //user_init();   // nfs

    /*ret = dr_mp3_Load("main.mp3");
    if(ret) dr_mp3_Play();

    orbisAudioResume(0); */

    /// enter main render loop
	while(flag)
	{
        updateController();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ret = glGetError();
        if (ret) {
            debugNetPrintf(ERROR,"[ORBIS_GL] glClear failed: 0x%08X\n", ret);
            //goto err;
        }
        /// get timing, fps
        if(frame %WEN == 0)
        {
            unsigned int now = get_time_ms();
            debugNetPrintf(INFO,"frame: %d, took: %ums, fps: %.3f\n", frame, now - time_ms,
                                                     ((double)WEN / (double)(now - time_ms) * 1000.f));
            time_ms = now;
        }
        frame++;

        orbisGlSwapBuffers();  /// flip frame

        sceKernelUsleep(10000);
	}

	//orbisAudioPause(0);
    //orbisAudioStop();
    //dr_mp3_End();
    user_end(); // nfs

    // detach audio callback
    m_bPlaying = 0, orbisAudioSetCallback(main_ch, 0, 0);

    orbisAudioStop();
    orbisAudioFinish();
/*
    ret = avutil_version();
    debugNetPrintf(ERROR,"avutil_version ret:%08X\n", ret);
*/
	finishApp();

	exit(EXIT_SUCCESS);
}