#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <systemservice.h>
#include <orbis2d.h>
#include <orbisPad.h>
#include <orbisKeyboard.h>
#include <orbisAudio.h>
#include <modplayer.h>
#include <ps4link.h>
#include <debugnet.h>
#include <orbisNfs.h>
#include <orbisGl.h>
#include <freetype-gl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdarg.h>

#include "orbisGlTextureManager.h"
#include "browser.h"

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
            switch(screenStatus)
            {
            	case SCREEN_BROWSER: orbisNfsBrowserEntryUp(); selected_entry(NULL); break;
            }
        }
        if(orbisPadGetButtonPressed(ORBISPAD_DOWN) || orbisPadGetButtonHold(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUG,"Down pressed\n");
            //pad_special(3);
            switch(screenStatus)
            {
            	case SCREEN_BROWSER: orbisNfsBrowserEntryDown(); selected_entry(NULL); break;
            }
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
            screenStatus = SCREEN_CREDITS;
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
            screenStatus = SCREEN_BROWSER;
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

           	char *t = calloc(256, 1); selected_entry(t);

           	debugNetPrintf(DEBUG,"t:'%s'\n", t);

            /* dr_mp3_Loop(t); */

            free(t), t = NULL;
        }
    }
}

OrbisGlTextureState *browserTexture=NULL;
OrbisGlTextureState *folderTexture=NULL;
OrbisGlTextureState *fileTexture=NULL;
OrbisGlTextureState *settingsTexture=NULL;
OrbisGlTextureState *creditsTexture=NULL;
OrbisGlProgram *programTexture=NULL;
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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //blue RGBA
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




void textureRegister()
{
	programTexture=orbisGlTextureManagerInit();
	debugNetPrintf(DEBUG,"[NFSSAMPLE] %s linked shader program_id=%d (0x%08x)\n",__FUNCTION__,programTexture->programId, programTexture->programId);

	if(programTexture!=NULL)
	{
		browserTexture=orbisGlInitTexture("browser",BROWSER_BACKGROUND_FILE_PATH,programTexture,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
		folderTexture=orbisGlInitTexture("folder_icon",FOLDER_ICON_PATH,programTexture,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
		fileTexture=orbisGlInitTexture("file_icon",FILE_ICON_PATH,programTexture,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
		//settingsTexture=orbisGlInitTexture("settings",SETTINGS_BACKGROUND_FILE_PATH,programTexture,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
		creditsTexture=orbisGlInitTexture("credits",CREDITS_BACKGROUND_FILE_PATH,programTexture,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);

	}
	//orbisGlTextInit(); <-- twice ?!
	debugNetPrintf(DEBUG,"[NFSSAMPLE] %s registered\n",__FUNCTION__); 
}

#include "orbisGlText.h"
/* basic "group of texts" indexing */
extern textline_t   t_credits, // credits.c
					t_footer,  // credits.c
					t_browser; // browser.c

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
	
    // GLES2 init, defaults
	textureRegister(); // all textures
	orbisGlTextInit(); // all texts
	screenStatus=SCREEN_CREDITS;

    /// reset timer
    time_ms = get_time_ms();

	/* mp3 audio, from nfs export
    user_init();   // nfs

    ret = dr_mp3_Load("main.mp3");
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

        switch(screenStatus)
        {
        	case SCREEN_CREDITS:
        	{
        	  if(creditsTexture) orbisGlDrawTexture(creditsTexture);
        	  orbisGlTextDraw(&t_credits);
        	  orbisGlTextDraw(&t_footer);
        	  break;
        	}
        	case SCREEN_BROWSER:
        	{
        	  if(browserTexture)  orbisGlDrawTexture(browserTexture);
        	  // draw browser
        	  orbisGlTextDraw(&t_browser);
        	  orbisGlTextDraw(&t_footer);
        	  break;
        	}
    	}

        orbisGlSwapBuffers();  /// flip frame

        sceKernelUsleep(10000);
	}

	//orbisAudioPause(0);
    //orbisAudioStop();
    //dr_mp3_End();
    //user_end(); // nfs

	finishApp();

	exit(EXIT_SUCCESS);
}