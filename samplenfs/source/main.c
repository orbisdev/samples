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


OrbisGlTextureState *browserTexture=NULL;
OrbisGlTextureState *folderTexture=NULL;
OrbisGlTextureState *fileTexture=NULL;
OrbisGlTextureState *settingsTexture=NULL;
OrbisGlTextureState *creditsTexture=NULL;
OrbisGlProgram *programTexture=NULL;
char path[256];
int notSelected=1;
int comeBack=0;
int posy=0;
int posx=0;
vec4 color;
int flagfolder=0;
int screenStatus;

OrbisNfsBrowserListEntry *currentEntry;

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
			orbisNfsBrowserEntryUp();
			currentEntry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());
			if(currentEntry!=NULL)
			{
				debugNetPrintf(INFO,"[NFSSAMPLE] current entry %s\n",currentEntry->dir->name);
			}
		}
		if(orbisPadGetButtonPressed(ORBISPAD_DOWN) || orbisPadGetButtonHold(ORBISPAD_DOWN))
		{
			debugNetPrintf(DEBUG,"Down pressed\n");
			debugNetPrintf(DEBUG," before entry down level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
			
			orbisNfsBrowserEntryDown();
			debugNetPrintf(DEBUG," after entry down level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
			
			currentEntry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());
			if(currentEntry!=NULL)
			{
				debugNetPrintf(INFO,"current entry %s\n",currentEntry->dir->name);
			}
			else
			{
				debugNetPrintf(INFO,"current entry chunga\n");
				
			}
		}
		if(orbisPadGetButtonPressed(ORBISPAD_RIGHT) || orbisPadGetButtonHold(ORBISPAD_RIGHT))
		{
			debugNetPrintf(DEBUG,"Right pressed\n");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_LEFT) || orbisPadGetButtonHold(ORBISPAD_LEFT))
		{
			debugNetPrintf(DEBUG,"Left pressed\n");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
		{
			debugNetPrintf(DEBUG,"Triangle pressed exit\n");
			showCredits();
			orbisGlTextClearBuffer();
			glClear(GL_COLOR_BUFFER_BIT);

			if(browserTexture)
			{
				orbisGlDrawTexture(browserTexture);
			}
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
		{
			debugNetPrintf(DEBUG,"Circle pressed\n");            
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
		{
			debugNetPrintf(DEBUG,"Cross pressed rand color\n");
			notSelected=0;
			debugNetPrintf(DEBUG,"cross level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
			
			//entry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());					
			currentEntry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());

			if(currentEntry!=NULL)
			{
				
				debugNetPrintf(INFO,"cross current entry %s customtype=%d\n",currentEntry->dir->name,currentEntry->dir->customtype);
				
				switch(currentEntry->dir->customtype)
				{	
					case FILE_TYPE_FOLDER:
						notSelected=1;
						//if(strcmp(currentEntry->dir->name, ".") == 0)
						//{
						//	flagfolder=1;
						//}
						//else
						//{
							//if(strcmp(currentEntry->dir->name, ".")!=0)
							//{
								debugNetPrintf(DEBUG,"cross selected folder level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
								
								//orbisFileBrowserDirLevelUp(currentEntry->dir->name);
								//debugNetPrintf(DEBUG,"cross selected folder level=%d base=%d rel=%d\n",orbisFileBrowserGetDirLevel(),orbisFileBrowserGetBasePos(),orbisFileBrowserGetRelPos());
								flagfolder=1;
								//}
						//}
						break;
					case FILE_TYPE_GAME_ROM:
						
						//if(extension==FILE_TYPE_GAME_ROM)
						//{
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							debugNetPrintf(DEBUG,"cross selected entry game %s\n",path);
							
							debugNetPrintf(DEBUG,"change cart\n");
							//actionCartInsertFromHost(path,0);

							//screenStatus=SCREEN_EMU;
							//LoadCart(path,slot,MAP_GUESS);
							//}
						//else
						//{
						//	debugNetPrintf(INFO,"wrong extension choose the right one\n");
							//}
						break;
					case FILE_TYPE_GAME_DSK:
						//if(extension==FILE_TYPE_GAME_DSK)
						//{
						
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							//LoadFileDrive(path,drive);
							debugNetPrintf(DEBUG,"change disk\n");
							//actionDiskInsertFromHost(path,0);
							//screenStatus=SCREEN_EMU;

							//}
						//else
						//{
						//	debugNetPrintf(INFO,"wrong extension choose the right one\n");
							//}
						break;
					case FILE_TYPE_CAS:
					//	if(extension==FILE_TYPE_CAS)
					//	{
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							//ChangeTape(path);
							debugNetPrintf(DEBUG,"change cas\n");
							
					//	}
					//	else
					//	{
					//		debugNetPrintf(INFO,"wrong extension choose the right one\n");
					//	}
						break;
					default:
						debugNetPrintf(DEBUG,"wrong extension come back\n");
						comeBack=1;
						break;
				}
			}

			//orbisAudioStop();
		}
		if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
		{
			debugNetPrintf(DEBUG,"Square pressed\n");
			//orbisAudioPause(0);
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L1))
		{
			debugNetPrintf(DEBUG,"L1 pressed\n");
			debugNetPrintf(3,"%s %d\n",orbisNfsBrowserGetListPath(),strlen(orbisNfsBrowserGetListPath()));

			//whoami();
			//debugNetPrintf(DEBUG,"calling myfuseloader in kernel land %d\n",syscall(11,myfuseloader505));
			// showdir("/data/tmp");
			//             showdir("/data/tmp/app");
			
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L2))
		{
			debugNetPrintf(DEBUG,"L2 pressed\n");
			//showfusedevice("/dev/fuse0");
			//showfusedevice("/data/tmp");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_R1))
		{
			debugNetPrintf(DEBUG,"R1 pressed\n");
			debugNetPrintf(3,"[NFSSAMPLE] %s opening fusenfs.txt from nfs server\n",__FUNCTION__);
			int dfd=orbisNfsOpen("MSX.ROM",O_RDONLY,0);
			debugNetPrintf(3,"[NFSSAMPLE] %s  open return 0x%08X\n",__FUNCTION__,dfd);
			if(dfd>=0)
			{
				int size=orbisNfsLseek(dfd,0,SEEK_END);
				if(size>0)
				{
					debugNetPrintf(3,"[NFSSAMPLE] %s  lseek return 0x%08X\n",__FUNCTION__,size);
					orbisNfsLseek(dfd,0,SEEK_SET);
					debugNetPrintf(3,"[NFSSAMPLE] %s  reading %d bytes\n",__FUNCTION__,size);
					char * buf=malloc(size);
					ret=orbisNfsRead(dfd,buf,size);
					if(ret)
					{
						//debugNetPrintf(3,"[NFSSAMPLE] %s\n",buf);
						debugNetPrintf(3,"[NFSSAMPLE] %d bytes read\n",ret);
						int fd=orbisNfsOpen("MSXC.ROM",O_RDWR|O_CREAT,0644);
						if(fd>=0)
						{
							ret=orbisNfsWrite(fd,buf,size);
							debugNetPrintf(3,"[NFSSAMPLE] %d bytes written\n",ret);
							orbisNfsClose(fd);
						}
					}
				}
				debugNetPrintf(3,"[NFSSAMPLE] %s closing file /hostapp/fusenfs.txt \n",__FUNCTION__);
				orbisNfsClose(dfd);
			}
		}
		if(orbisPadGetButtonPressed(ORBISPAD_R2))
		{
			debugNetPrintf(DEBUG,"R2 pressed\n");
			// debugNetPrintf(DEBUG,"orbisKernelLand return from syscall %d\n",syscall(11,orbisKernelLand));
		}
	}
}

void finishApp()
{
	//orbisAudioFinish();
	//orbisKeyboardFinish();
	orbisGlFinish();
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

void browserDrawText()
{
	vec4 color;
	color.r=255.0;
	color.g=255.0;
	color.b=255.0;
	color.a=1.0;

	orbisGlTextDrawBuffer("Browser",30+posx,52+15+posy,color);
	orbisGlTextDrawBuffer("Back",90-22+posx,1000+34+posy,color);
	orbisGlTextDrawBuffer("Browser",100+90-22+64,1000+34+posy,color);
	orbisGlTextDrawBuffer("Credits",220+90-22+64+79,1000+34+posy,color);
	orbisGlTextDrawBuffer("Select",340+90-22+64+79+79,1000+34+posy,color);

}
void browserDraw()
{
	int i=0;
	uint32_t f1 ,f2;
	char dateString[20]; 
	char sizeString[8];
	char unitString[3];
	char mypath[256];
	orbisGlTextClearBuffer();
	browserDrawText();
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
		if(i==orbisNfsBrowserGetRelPos())
		{
				
			/*f1 = 0xFF24FFBD;
			f2 = 0xFF24FFBD;
			update_gradient(&f1, &f2);*/
			color.r=36.0/255.0;
			color.g=1.0;
			color.b=189.0/255.0;
			color.a=1.0;
								
		}
		else
		{
			/*f1 = 0x80FFFFFF;
			f2 = 0x80FFFFFF;
			update_gradient(&f1, &f2);*/
			color.r=1.0;
			color.g=1.0;
			color.b=1.0;
			color.a=1.0;
		}
		//print_text(50+posx,90+i*20+posy,entry->dir->name);
		orbisGlTextDrawBuffer(entry->dir->name,60,90+i*30+1+67-30,color);
		
		sprintf(dateString,"%02d/%02d/%04d %02d:%02d %s",
		entry->dir->mtime.day,
		entry->dir->mtime.month,
		entry->dir->mtime.year,
		entry->dir->mtime.hour>12?entry->dir->mtime.hour-12:entry->dir->mtime.hour,
		entry->dir->mtime.minute,
		entry->dir->mtime.hour>=12? "PM" : "AM");	
		orbisGlTextDrawBuffer(dateString,740+960+8-15-35,90+i*30+1+67-30,color);
		//orbisGlTextDraw(" hello world",740+960+8-15-35,90+i*30+1+67-30,color);

		orbisGlTextDrawBuffer(unitString,740+960-8-15-35-22*2,90+i*30+1+67-30,color);
		orbisGlTextDrawBuffer(sizeString,740+960-8-15-35-22*7,90+i*30+1+67-30,color);
				
		entry=entry->next;	
		i++;			
			
	}
	//f1 = 0xFF24FFBD,f2 = 0xFF24FFBD;
	//update_gradient(&f1, &f2);
	color.r=1.0;//36.0/255.0;
	color.g=0.1;//1.0;
	color.b=0.1;//189.0/255.0;
	color.a=1.0;
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
	orbisGlTextDrawBuffer(mypath,30,52+76-30+2,color);
	orbisGlTextDraw();
}

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
	orbisGlTextInit();
	debugNetPrintf(DEBUG,"[NFSSAMPLE] %s registered\n",__FUNCTION__);

    
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
	textureRegister();
	while(flag)
	{

		if(browserTexture)
		{
			orbisGlDrawTexture(browserTexture);
		}
		updateController();
		browserDraw();
		orbisGlSwapBuffers();

	}
	finishApp();
	sleep(3);
	exit(EXIT_SUCCESS);
}