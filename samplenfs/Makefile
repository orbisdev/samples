ifndef Ps4Sdk
ifdef ps4sdk
Ps4Sdk := $(ps4sdk)
endif
ifdef PS4SDK
Ps4Sdk := $(PS4SDK)
endif
ifndef Ps4Sdk
$(error Neither PS4SDK, Ps4Sdk nor ps4sdk set)
endif
endif

target ?= ps4_elf
TargetFile=homebrew.elf

include $(Ps4Sdk)/make/ps4sdk.mk
LinkerFlags+=-lPs4SystemCall_stub -lorbisNfs -lfreetype-gl -lfreetype -ldebugnet -lps4link  -lelfloader -lorbisKeyboard  -lpng -lz -lorbisGl -lorbisFile -lorbisPad -lorbisAudio -lmod -lorbisFileBrowser -lorbisXbmFont -lsxmlc -lSceNet_stub -lScePigletv2VSH_stub -lSceSystemService_stub -lSceUserService_stub -lScePad_stub -lSceAudioOut_stub -lSceVideoOut_stub -lSceGnmDriver_stub -lSceIme_stub -lSceSysmodule_stub 
#CompilerFlags += -DNDEBUG -I$(Ps4Sdk)/include/nfsc
IncludePath += -I$(Ps4Sdk)/include/freetype2 -I$(Ps4Sdk)/include/freetype-gl
install:
	@cp $(OutPath)/homebrew.elf /usr/local/orbisdev/git/ps4sh/bin
	@echo "Installed!"