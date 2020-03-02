/*
 * liborbis 
 * Copyright (C) 2015,2016,2017,2018 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 * Repository https://github.com/orbisdev/liborbis
 */
#include <stdio.h>
#include <string.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#include <debugnet.h>
#include <orbisNfs.h>
#include <fcntl.h>


#include "browser.h"
//int posy=0;
//int posx=0;
vec4 color;
int flagfolder=0;

OrbisNfsBrowserListEntry *currentEntry;

#include "orbisGlTextureManager.h"
extern OrbisGlTextureState *folderTexture, *fileTexture;


void getSizeString(char string[8],char string1[3], uint64_t size) 
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B ", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0) {
		double_size /= 1024.0;
		i++;
	}
	//char string1[16];
	snprintf(string, 8, "%.*f",(i == 0) ? 0 : 2, double_size);
	snprintf(string1, 3, "%s",units[i]);
	//debugNetPrintf(DEBUG,"%s %d\n",string,strlen(string));
	//debugNetPrintf(DEBUG,"%s %d\n",string1,strlen(string1));

	//snprintf(string, 16, "%.*f %s",  2, double_size, units[i]);

}

void selected_entry(char *tmp)
{
	int i = orbisNfsBrowserGetRelPos();
	currentEntry = orbisNfsBrowserListGetNthEntry(i);
	debugNetPrintf(DEBUG,"orbisNfsBrowserGetRelPos: %d, '%s'\n", i, currentEntry->dir->name);
	if(tmp)
		strcpy(tmp, currentEntry->dir->name);
}

#include "orbisGlText.h"

void browserDrawText(void)
{
	int  i=0;
	char dateString[20]; 
	char sizeString[8];
	char unitString[3];
	char mypath[256];

	if(flagfolder==1 && currentEntry)
	{
		if(strcmp(currentEntry->dir->name,".")!=0)
		{
			debugNetPrintf(DEBUG,"go to new directory %s\n",currentEntry->dir->name);
			//char rootpath[256];
			//sprintf(rootpath,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
			//debugNetPrintf(DEBUG,"go to new directory %s\n",rootpath);
			if(strcmp(currentEntry->dir->name,"..")!=0)
			{
				orbisNfsBrowserDirLevelUp(currentEntry->dir->name);
			}
			else
			{
				orbisNfsBrowserDirLevelDown();
			}
			debugNetPrintf(DEBUG,"after orbisNfsBrowserDirLevelUp\n");
		}
		else
		{
			orbisNfsBrowserListRefresh();
		}
		flagfolder=0;
	}
	OrbisNfsBrowserListEntry *entry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos());
	
	/// header
	color = (vec4){{ 1., .6, .1, 1. }};
	char *aux=orbisNfsBrowserGetListPath();
	if(aux[0]=='.' && strlen(aux)<=2)
	{
		snprintf(mypath,256,"%s","nfs:/");
	}
	else
	{
		char *aux1=orbisNfsBrowserGetListPath();
		snprintf(mypath,256,"%s%s","nfs:/",&aux1[2]);
	}
	orbisGlTextSetupBuffer(mypath,30,52+76-30+2,color);

	/// iterate entries	
	while(entry && i<MAX_ENTRIES)
	{
		if(entry->dir->customtype==FILE_TYPE_FOLDER)
		{
			if(folderTexture)
			{
				//orbisGlDraw(programTextureId,folderTextureId,30,90+i*20);
				orbisGlDrawTextureSpecial(folderTexture,30,90+i*30+1+67-30-15);
	
			}
			//sprintf(sizeString,"%s","FOLDER");
			getSizeString(sizeString,unitString,entry->dir->size);
		}
		else
		{
			if(fileTexture)
			{
				//orbisGlDraw(programTextureId,fileTextureId,30,90+i*20+2);	
				orbisGlDrawTextureSpecial(fileTexture,30,90+i*30+1+67-30-15);

			}
			getSizeString(sizeString,unitString,entry->dir->size);
		}
		//debugNetPrintf(DEBUG,("%s %d\n",entry->name,entry->type);

		// this color mark has to be done in render pass!
		if(i==orbisNfsBrowserGetRelPos())
		{
			color = (vec4){{ 1., .2, .9, 1. }}; // selected entry
		}
		else
		{
			color = (vec4){{ 1., 1., 1., 1. }};
		}
		//print_text(50+posx,90+i*20+posy,entry->dir->name);
		orbisGlTextSetupBuffer(entry->dir->name,60,90+i*30+1+67-30,color);
		
		sprintf(dateString,"%02d/%02d/%04d %02d:%02d %s",
		entry->dir->mtime.day,
		entry->dir->mtime.month,
		entry->dir->mtime.year,
		entry->dir->mtime.hour>12?entry->dir->mtime.hour-12:entry->dir->mtime.hour,
		entry->dir->mtime.minute,
		entry->dir->mtime.hour>=12? "PM" : "AM");	
		orbisGlTextSetupBuffer(dateString,740+960+8-15-35,90+i*30+1+67-30,color);
		//orbisGlTextDraw(" hello world",740+960+8-15-35,90+i*30+1+67-30,color);

		orbisGlTextSetupBuffer(unitString,740+960-8-15-35-22*2,90+i*30+1+67-30,color);
		orbisGlTextSetupBuffer(sizeString,740+960-8-15-35-22*7,90+i*30+1+67-30,color);
				
		entry=entry->next;	
		i++;			
			
	}
}
