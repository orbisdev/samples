# linux_es2goodness, a GLES2 playground


So you'd like to:
- draw some Freetype texts
- in OpenGL ES 2.0 (GLES2)
- in your EGL display (X11, KMS, etc.)
- on linux, etc.
- w/o any additional libs like GLUT, GLEW, etc. (portability first)
- and extend adding some other scene?

This is a demo application I'm using to test and learn GLES2 code, or travel the marvelous world of shaders!

This demo is currently tested on linux on Mesa lib (x86/x86_64) and ps4 orbis on Piglet;

App should compile and run on macos too, just write the corresponding X11 (windowing system) part;


### refs:

- _to flood with_


### requirements:

- lib FreeType
- lib FreeType-gl
- libX11, libEGL, libGLESv2 (Mesa)

### notes:

_Since GL code is the same, I'm building libfreetype-gl from orbisdev/liborbis/portlibs !_


### features:

- egl.c: setup EGL display, create a X11 window
- demo-font.c: setup shaders to draw sample texts with FreeType
- text_ani.c: setup shaders to draw sample texts with FreeType, to implement simple effects
- to extend

### what really needs?

```bash
linux_es2goodness # readelf -d egl_demo | grep NEEDED
 0x0000000000000001 (NEEDED)             Shared library: [libX11.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libEGL.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libGLESv2.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libfreetype.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
```

### make liborbis

```bash
linux_es2goodness # make -f text_ani.makefile && ./egl_demo_text_ani

linux_es2goodness # make -f sprite.makefile && ./egl_demo_sprite
```


![a](https://user-images.githubusercontent.com/8250079/75284913-ee12fa00-5815-11ea-8e8e-6c92cd18269f.png)
![a](https://user-images.githubusercontent.com/8250079/75290411-8ca45880-5820-11ea-9f33-56bfada9d4b9.png)

