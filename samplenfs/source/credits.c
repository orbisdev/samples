/*
 * liborbis 
 * Copyright (C) 2015,2016,2017,2018 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 * Repository https://github.com/orbisdev/liborbis
 */
#include <orbisPad.h>
#include <debugnet.h>
#include <freetype-gl.h>  // links against libfreetype-gl

#include "orbisGlTextureManager.h"
#include "browser.h"
extern OrbisGlTextureState *creditsTexture;
//extern OrbisGlTextureState *browserTexture;

extern int screenStatus;
void creditsUpdateController()
{
	int ret;
	unsigned int buttons=0;
	ret=orbisPadUpdate();
	if(ret==0)
	{
		
		if(orbisPadGetButtonPressed(ORBISPAD_UP))
		{
			debugNetPrintf(DEBUG,"Up pressed\n");
			screenStatus=SCREEN_BROWSER;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_DOWN))
		{
			debugNetPrintf(DEBUG,"Down pressed\n");
			screenStatus=SCREEN_BROWSER;
		}						
		if(orbisPadGetButtonPressed(ORBISPAD_RIGHT))
		{
			debugNetPrintf(DEBUG,"Right pressed\n");
			screenStatus=SCREEN_BROWSER;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_LEFT))
		{
			debugNetPrintf(DEBUG,"Left pressed\n");
			screenStatus=SCREEN_BROWSER;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
		{
			debugNetPrintf(DEBUG,"Triangle pressed exit\n");
			screenStatus=SCREEN_CREDITS;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
		{
			debugNetPrintf(DEBUG,"Circle pressed come back to browser\n");
			screenStatus=SCREEN_EMU;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
		{
			debugNetPrintf(DEBUG,"Cross pressed\n");
			screenStatus=SCREEN_CREDITS;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
		{
			debugNetPrintf(DEBUG,"Square pressed\n");
			screenStatus=SCREEN_BROWSER;
		}
	}
}
void creditsDrawText()
{
	vec4 color;
	color.r=255.0;
	color.g=255.0;
	color.b=255.0;
	color.a=1.0;
	orbisGlTextClearBuffer();
	orbisGlTextDrawBuffer("Credits",30,52+15,color);
	orbisGlTextDrawBuffer("Back",90-22,1000+34,color);
	orbisGlTextDrawBuffer("Browser",100+90-22+64,1000+34,color);
	orbisGlTextDrawBuffer("Credits",220+90-22+64+79,1000+34,color);
	orbisGlTextDrawBuffer("Select",340+90-22+64+79+79,1000+34,color);
	int i=0;
	orbisGlTextDrawBuffer("libOrbisNfs is a homebrew library based in libnfs and it is part of LIBORBIS for PlayStation 4",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("LIBORBIS was created while i was working in MSX emulator for firmware 1.76 so more than 4 years now",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("It is open source and it can be ported easily to others sdks",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("Source code of LIBORBIS is available at:",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("https://github.com/orbisdev/liborbis",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("This software have been done using the following open source tools:",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- Hitodama's PS4SDK with lasted addons check:",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("https://github.com/psxdev/ps4sdk/tree/firmware505",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- Open Source Toolchain based on LLVM/Clang and gnu binutils compiled on MacOS",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- This sample is available at",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("https://github.com/orbisdev/samples/samplenfs",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- Gimp for all graphic stuff",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("Special thanks goes to:",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- All homebrew developers who are sharing source code because It is the way. For example Ronnie Sahlberg libnfs author",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- Hitodama for his incredible work with PS4SDK the one and only",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- flatz to bring us a lot of fun with gl stuff",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- @frangar, @xerpi @theflow0 and rest of people involved in vitasdk for their incredible work on Vita, it helped me a lot :P",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("Some parts of liborbis are based on their work so forgive me for stealing parts like icons and code from vitashell :P",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- @masterzorag for freetype and orbisfreetype-gl port and all samples for orbisgl and liborbis",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- All people who have been using liborbis all these years",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- All ps3dev and ps2dev old comrades",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- @ZiL0G80 and @notzecoxao for sharing some stuff with me",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- All MSX users, special mention to ASUR MSX members",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("- Hideo Kojima and Sony source of inspiration and dreams and thanks for more than 35 years of playing and coding with Sony devices",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("If you want contact with me you can do at @psxdev on twitter",60,90+i*30+1+67,color);i++;i++;
	orbisGlTextDrawBuffer("Antonio Jose Ramos Marquez aka bigboss",60,90+i*30+1+67,color);i++;
	orbisGlTextDrawBuffer("The best is yet to come... It's time Developers... Assemble",60,90+i*30+1+67,color);i++;i++;
	orbisGlTextDraw();
}
void creditsDraw()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if(creditsTexture!=NULL)
	{
		
       // orbisGlTextureAdd(creditsTexture,0,0,creditsTexture->texture->width,creditsTexture->texture->height);
        
		orbisGlDrawTexture(creditsTexture);
	}

}
void showCredits()
{
	//orbisAudioPause(0);
	screenStatus=SCREEN_CREDITS;
	while(screenStatus==SCREEN_CREDITS)
	{
		
		creditsDraw();
		creditsDrawText();
		creditsUpdateController();	
		orbisGlSwapBuffers();
	}



	//comeBack=0;
	//msx go go
	//displayNumber=0;
}