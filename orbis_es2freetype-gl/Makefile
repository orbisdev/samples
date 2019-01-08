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

###################################

include $(Ps4Sdk)/make/ps4sdk.mk

LinkerFlags+= -lfreetype-gl -lfreetype -ldebugnet -lps4link -lorbisFile -lelfloader -lorbisKeyboard -lorbisGl -lorbisPad -lorbisAudio -lmod -lSceNet_stub -lScePigletv2VSH_stub -lSceSystemService_stub -lSceUserService_stub -lScePad_stub -lSceAudioOut_stub -lSceIme_stub -lSceSysmodule_stub


IncludePath += -I$(Ps4Sdk)/include/freetype2 -I$(Ps4Sdk)/include/freetype-gl





