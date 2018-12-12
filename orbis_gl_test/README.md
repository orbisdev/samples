GL ES 2.0 basic sample for PlayStation 4
======================
 
 What does this do?
===================
 
  This is a basic sample using GL ES 2.0 based on sample published by flatz: 
  
  
  https://github.com/flatz/ps4_gl_test  
  
  
  What do you need to compile it?
==================

 1) You will need ps4sdk fork branch firmware505 and liborbis from my own repository

  https://github.com/psxdev/ps4sdk/tree/firmware505
  
  Follow steps in readme
  ```
  git clone https://github.com/orbisdev/liborbis
  
  ```
  
  
 2) Compile sample
  
  
  ```
  git clone https://github.com/orbisdev/samples
  cd samples
  cd orbis_gl_test
  make
  ```
  copy bin/homebrew.elf to ps4sh/bin folder or where you are executing ps4sh binary

 3) You will need ps4sh because all is loaded from host using the last orbislink.pkg look for it or make your own using code published by flatz or me 
  

 4) Before execute homebrew.elf you need ps4sh running on folder where your homebrew.elf was copied.Load orbislink pkg and you will see logs in your ps4sh tool
 
 After you see orbislink splash screen execute connect command on ps4sh
  
 ```
 ./ps4sh
 ps4sh version 1.0
 /Users/bigboss/.ps4shrc: No such file or directory

 log: [HOST][INFO]: [PS4SH] Ready
 log: [PS4][INFO]: debugnet initialized
 log: [PS4][INFO]: ready to have a lot of fun...
 log: [PS4][INFO]: [PS4LINK] Server request thread UID: 0x80F4C9C0
 log: [PS4][INFO]: [PS4LINK] Server command thread UID: 0x80F4CE40
 log: [PS4][DEBUG]: [PS4LINK] Command Thread Started.
 log: [PS4][DEBUG]: [PS4LINK] Created ps4link_requests_sock: 7
 log: [PS4][DEBUG]: [PS4LINK] Created ps4link_commands_sock: 8
 log: [PS4][DEBUG]: [PS4LINK] bind to ps4link_requests_sock done
 log: [PS4][DEBUG]: [PS4LINK] Command listener waiting for commands...
 log: [PS4][DEBUG]: [PS4LINK] Ready for connection 1
 log: [PS4][INFO]: [PS4LINK] Waiting for connection
 
 ps4sh> connect
 log: [HOST][INFO]: [PS4SH] Connecting to fio ps4link ip 192.168.1.17
 log: [HOST][INFO]: [PS4SH] PlayStation is listening at 192.168.1.17
 log: [PS4][DEBUG]: [PS4LINK] Client connected from 192.168.1.3 port: 42436

 log: [PS4][INFO]: [PS4LINK] sock ps4link_fileio set 9 connected 1
 log: [PS4][INFO]: [PS4LINK] Waiting for connection
 log: [PS4][DEBUG]: [ORBISLINK] sandbox KglSbv4wWN
 log: [PS4][DEBUG]: [ORBISLINK]Loading homebrew.elf from host
 log: [PS4][DEBUG]: [ORBISLINK] orbisExecUserElf called
 log: [PS4][DEBUG]: [PS4LINK] file open req (host0:homebrew.elf, 0 0)
 log: [HOST][DEBUG]: [PS4SH] Opening homebrew.elf flags 0
 log: [HOST][DEBUG]: [PS4SH] Open return 7
 log: [PS4][DEBUG]: [PS4LINK] file open reply received (ret 7)
 log: [PS4][DEBUG]: [PS4LINK] file lseek req (fd: 7)
 log: [HOST][DEBUG]: [PS4SH] 604408 result of lseek 0 offset 2 whence
 log: [PS4][DEBUG]: [PS4LINK] ps4link_lseek_file: lseek reply received (ret 604408)
 log: [PS4][DEBUG]: [PS4LINK] file lseek req (fd: 7)
 log: [HOST][DEBUG]: [PS4SH] 0 result of lseek 0 offset 0 whence
 log: [PS4][DEBUG]: [PS4LINK] ps4link_lseek_file: lseek reply received (ret 0)
 log: [PS4][DEBUG]: [ORBISLINK] before orbisSysMmap
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: Reply said there's 604408 bytes to read (wanted 604408)
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 0  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 1  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 2  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 3  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 4  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 5  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 6  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 7  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 8  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 9  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 10  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 11  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 12  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 13  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 14  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 15  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 16  readed 32768
 log: [PS4][DEBUG]: [PS4LINK] ps4link_read_file: chunk 17  readed 47352
 log: [PS4][DEBUG]: [PS4LINK] ps4link_file: file close req (fd: 7)
 log: [PS4][DEBUG]: [PS4LINK] ps4link_close_file: close reply received (ret 0)
 log: [PS4][DEBUG]: [ORBISLINK] orbisExecUserElf ready to run elf
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun malloc for argument
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after malloc for argument
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after ps4MemoryProtectedCreate
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after ps4MemoryProtectedGetWritableAddress writable=200098000
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after ps4MemoryProtectedGetExecutableAddress executable=200098000
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate in segments length=6
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate memcpy 200098040 200004040 336
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy before memcpy
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy after memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate memcpy 200098190 200004190 25
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy before memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate memcpy
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy after memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate memcpy 200098000 200004000 398112
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy before memcpy
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy after memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate memcpy 2002f9320 200065320 135120
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy before memcpy
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy after memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate orbisMemorySet
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemorySet before memset
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemorySet after memset
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate orbisMemorySet
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate before elfLoaderInstantiate memcpy 2002f9320 200065320 256
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy before memcpy
 log: [PS4][DEBUG]: [ORBISLINK] orbisMemoryCopy after memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderInstantiate after elfLoaderInstantiate memcpy
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderLoad  after elfLoaderInstantiate return=0
 log: [PS4][DEBUG]: [ORBISLINK] elfLoaderLoad after elfLoaderRelocate return=0
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after elfLoaderLoad return r=0 readable=200098000 executable=200098000
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after set argument->main 2000ccf80
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserRun after elfDestroyAndFree
 log: [PS4][DEBUG]: [ORBISLINK] New user elf thread UID: 0x80F4D2C0
 log: [PS4][DEBUG]: [ORBISLINK] orbisUserMain Configuration pointer 424500, pointer_conf string 424500
 log: [PS4][INFO]: debugnet already initialized using configuration from ps4link
 log: [PS4][INFO]: debugnet_initialized=1 SocketFD=6 logLevel=3
 log: [PS4][INFO]: ready to have a lot of fun...
 log: [PS4][INFO]: ps4link already initialized using configuration from ps4link
 log: [PS4][INFO]: ps4link_fileio_active=1
 log: [PS4][INFO]: ps4link_cmdsio_active=1
 log: [PS4][INFO]: ps4link_initialized=1
 log: [PS4][INFO]: ps4link_requests_port=18193
 log: [PS4][INFO]: ps4link_commands_port=18194
 log: [PS4][INFO]: ps4link_debug_port=18194
 log: [PS4][INFO]: ps4link_fileio_sock=9
 log: [PS4][INFO]: ps4link_requests_sock=7
 log: [PS4][INFO]: ps4link_commands_sock=8
 log: [PS4][INFO]: [ORBIS_GL] Hello from first gl es sample with hitodama's sdk and liborbis
 log: [PS4][INFO]: [ORBIS_GL] GL_VERSION: OpenGL ES 2.0 Piglet
 log: [PS4][INFO]: [ORBIS_GL] GL_RENDERER: Piglet
 ps4sh>
 ```
 
 5) ready to have a lot of fun :P 
 
  
 6) I wait that this can help to developers really interested in homebrew if you want piracy this is not for you. 


  Change log
===========================
 - 12/12/2018 Initial release
 

  Credits
===========================
  
  this time big thanks to flatz to bring a lot of fun for christmas !!!!
  special thanks to masterzorag and Zer0xFF initial job from them
  
  ![alt text](https://raw.github.com/orbisdev/samples/master/orbis_gl_test/capture.jpg "gl es on PlayStation 4")
  
  
 