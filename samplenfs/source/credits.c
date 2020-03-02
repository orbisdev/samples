/*
 * liborbis 
 * Copyright (C) 2015,2016,2017,2018 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 * Repository https://github.com/orbisdev/liborbis
 */
#include <orbisPad.h>
#include <debugnet.h>
#include <freetype-gl.h>  // links against libfreetype-gl


#include "orbisGlText.h"
#include "orbisGlTextureManager.h"
#include "browser.h"
extern OrbisGlTextureState *creditsTexture;
//extern OrbisGlTextureState *browserTexture;

extern int screenStatus;

// just called once, we are setting up vertexes positions and textures

vec4 color = {{ 1., 1., 1., 1. }};

void footerDrawText(void)
{
	orbisGlTextSetupBuffer("Credits",30,52+15,color);
	orbisGlTextSetupBuffer("Back",90-22,1000+34,color);
	orbisGlTextSetupBuffer("Browser",100+90-22+64,1000+34,color);
	orbisGlTextSetupBuffer("Credits",220+90-22+64+79,1000+34,color);
	orbisGlTextSetupBuffer("Select",340+90-22+64+79+79,1000+34,color);
}

void creditsDrawText(void)
{
	int i=0;
	orbisGlTextSetupBuffer("libOrbisNfs is a homebrew library based in libnfs and it is part of LIBORBIS for PlayStation 4",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("LIBORBIS was created while i was working in MSX emulator for firmware 1.76 so more than 4 years now",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("It is open source and it can be ported easily to others sdks",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("Source code of LIBORBIS is available at:",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("https://github.com/orbisdev/liborbis",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("This software have been done using the following open source tools:",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- Hitodama's PS4SDK with lasted addons check:",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("https://github.com/psxdev/ps4sdk/tree/firmware505",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- Open Source Toolchain based on LLVM/Clang and gnu binutils compiled on MacOS",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- This sample is available at",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("https://github.com/orbisdev/samples/samplenfs",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- Gimp for all graphic stuff",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("Special thanks goes to:",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- All homebrew developers who are sharing source code because It is the way. For example Ronnie Sahlberg libnfs author",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- Hitodama for his incredible work with PS4SDK the one and only",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- flatz to bring us a lot of fun with gl stuff",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- @frangar, @xerpi @theflow0 and rest of people involved in vitasdk for their incredible work on Vita, it helped me a lot :P",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("Some parts of liborbis are based on their work so forgive me for stealing parts like icons and code from vitashell :P",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- @masterzorag for freetype and orbisfreetype-gl port and all samples for orbisgl and liborbis",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- All people who have been using liborbis all these years",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- All ps3dev and ps2dev old comrades",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- @ZiL0G80 and @notzecoxao for sharing some stuff with me",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- All MSX users, special mention to ASUR MSX members",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("- Hideo Kojima and Sony source of inspiration and dreams and thanks for more than 35 years of playing and coding with Sony devices",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("If you want contact with me you can do at @psxdev on twitter",60,90+i*30+1+67,color);i++;i++;
	orbisGlTextSetupBuffer("Antonio Jose Ramos Marquez aka bigboss",60,90+i*30+1+67,color);i++;
	orbisGlTextSetupBuffer("The best is yet to come... It's time Developers... Assemble",60,90+i*30+1+67,color);i++;i++;

	debugNetPrintf(DEBUG,"creditsDrawText\n");
}
