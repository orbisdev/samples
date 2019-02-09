/*
 * GL ES 2.0 liborbisGl sample
 * ----------------------------
 *
 * whole EGL setup/cleanup is internally managed by liborbisGL;
 * main render loop calls 2 functions: draw and update controller;

 * includes playing of .mod files and controller input;
 * main skeleton results in a very basic and clean code.
 */

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
#include <orbisFile.h>
#include <orbisGl.h>
#include <unistd.h>



bool flag=true;

Orbis2dConfig  *conf;
OrbisPadConfig *confPad;

typedef struct OrbisGlobalConf
{
    Orbis2dConfig *conf;
    OrbisPadConfig *confPad;
    OrbisAudioConfig *confAudio;
    OrbisKeyboardConfig *confKeyboard;
    ps4LinkConfiguration *confLink;
    int orbisLinkFlag;
}OrbisGlobalConf;

OrbisGlobalConf *myConf;

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
            pad_special(2);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_DOWN) || orbisPadGetButtonHold(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUG,"Down pressed\n");
            pad_special(3);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_RIGHT) || orbisPadGetButtonHold(ORBISPAD_RIGHT))
        {
            debugNetPrintf(DEBUG,"Right pressed\n");
            pad_special(1);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_LEFT) || orbisPadGetButtonHold(ORBISPAD_LEFT))
        {
            debugNetPrintf(DEBUG,"Left pressed\n");
            pad_special(0);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
        {
            debugNetPrintf(DEBUG,"Triangle pressed exit\n");
            flag=0;
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
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R1))
        {
            debugNetPrintf(DEBUG,"R1 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R2))
        {
            debugNetPrintf(DEBUG,"R2 pressed\n");
        }
    }
}


void finishApp()
{
    orbisAudioFinish();
    orbisKeyboardFinish();
    orbisGlFinish();
    orbisPadFinish();
    orbisFileFinish();
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
            debugNetPrintf(ERROR,"glViewport failed: 0x%08X\n",ret);
            return false;
        }
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue RGBA
        ret=glGetError();
        if(ret)
        {
            debugNetPrintf(ERROR,"glClearColor failed: 0x%08X\n",ret);
            return false;
        }
        return true;
    }
    return false;
}


bool initApp()
{
    int ret;
    sceSystemServiceHideSplashScreen();
    //more library initialiazation here pad,filebroser,audio,keyboard, etc
    //....
    orbisFileInit();
    ret=orbisPadInitWithConf(myConf->confPad);
    if(ret)
    {
        confPad=orbisPadGetConf();
        ret=orbisAudioInitWithConf(myConf->confAudio);
        if(ret==1)
        {
            ret=orbisKeyboardInitWithConf(myConf->confKeyboard);
            if(ret!=1)
                return false;
        }
    }
    else
    {
        return false;
    }
    if(!initAppGl())
        return false;

    return true;
}


/// main rendering loop
int frame = 0;
static bool main_loop(void)
{
    int ret;

    while (flag)
    {
        updateController();

        glClear(GL_COLOR_BUFFER_BIT);
        ret = glGetError();
        if (ret) {
            debugNetPrintf(ERROR,"[ORBIS_GL] glClear failed: 0x%08X\n", ret);
            goto err;
        }

        /// update
        UpdateScene(0.1);

        /// draw
        DrawScene();

        // flip frame
        orbisGlSwapBuffers();
    }
    return true;

err:
    return false;
}



int main(int argc, char *argv[])
{
    int ret;

    uintptr_t intptr=0;
    sscanf(argv[1],"%p",&intptr);
    myConf=(OrbisGlobalConf *)intptr;
    ret=ps4LinkInitWithConf(myConf->confLink);
    if(!ret)
    {
        ps4LinkFinish();
        return 0;
    }
    debugNetPrintf(INFO,"[ORBIS_GL] Hello from GL ES sample with hitodama's sdk and liborbis\n");
    sleep(1);

    // init libraries
    flag=initApp();

    Mod_Init(0);
    ret = Mod_Load("host0:main.mod");
    if(ret)
        Mod_Play();

    orbisAudioResume(0);

    // build shaders, setup initial state, etc.
    InitScene(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);

    // enter main render loop
    if (!main_loop())
    {
        debugNetPrintf(ERROR,"[ORBIS_GL] Main loop stopped.\n");
        goto err;
    }

    err:
    DeleteScene();

    orbisAudioPause(0);
    Mod_End();

    // finish libraries
    finishApp();

    exit(EXIT_SUCCESS);
}
