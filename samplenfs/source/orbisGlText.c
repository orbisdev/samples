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
typedef struct vertex_t
{
	float x, y, z;		//position
	float s, t;			//texture
	float r, g, b, a;	//color
}vertex_t;


#include "orbisGlText.h"
textline_t   t_credits,
			 t_footer,
			 t_browser; 

GLuint shader;
vertex_buffer_t *buffer;
mat4 model, view, projection;
texture_font_t *font=0;
texture_atlas_t *atlas;
int orbisGlTextEnabled=0;

void orbisGlTextClearBuffer()
{
	vertex_buffer_clear(buffer);
}

void orbisGlTextDraw(textline_t *text_to_draw)
{
//glClearColor( 0.3, 0.1, 0.9, 1 ); // BGRA
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glUseProgram( shader );
    
	glBindTexture( GL_TEXTURE_2D, atlas->id );
	glUniform1i( glGetUniformLocation( shader, "texture" ),0 );
	glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),1, 0, model.data);
	glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),1, 0, view.data);
	glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ),1, 0, projection.data);

	if(0) /* draw whole VBO (storing all added texts) */
    {
       vertex_buffer_render( buffer, GL_TRIANGLES ); // all vbo
    }
    else /* draw a range/selection of indexes (= glyph) */
    {
        vertex_buffer_render_setup( buffer, GL_TRIANGLES ); // start draw

        // follow indexed textline_t
        for(int i = text_to_draw->off;
                i < text_to_draw->off + text_to_draw->len;
                i += 1)
        { // glyph by one (2 triangles: 6 indices, 4 vertices)
            vertex_buffer_render_item ( buffer, i );
        }
        
        vertex_buffer_render_finish( buffer ); // end draw
    }

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void orbisGlTextSetupBuffer(char *mytext,int x,int y,vec4 color)
{
	//if(orbisGlTextEnabled==0){	return;	} ???

	size_t i;
	vec2 ppen={{x,1080-y}};
	vec2 *pen=&ppen;
	float r = color.red, g = color.green, b = color.blue, a = color.alpha;
	for( i=0; i<strlen(mytext); ++i )
	{
		texture_glyph_t *glyph = texture_font_get_glyph( font, mytext + i );
		if( glyph != NULL )
		{
			//debugNetPrintf(3,"%c %d glyph->width %d glyph->advance_x %d\n",mytext[i],i,glyph->width,glyph->advance_x);
            float kerning = 0.0f;
			if( i > 0)
			{
				kerning = texture_glyph_get_kerning( glyph, mytext + i - 1 );
			}
			pen->x += kerning;
			int x0  = (int)( pen->x + glyph->offset_x );
			int y0  = (int)( pen->y + glyph->offset_y );
			int x1  = (int)( x0 + glyph->width );
			int y1  = (int)( y0 - glyph->height );
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			GLuint indices[6] = {0,1,2, 0,2,3};
			vertex_t vertices[4] = {{ x0,y0,0,  s0,t0,  r,g,b,a },
									{ x0,y1,0,  s0,t1,  r,g,b,a },
									{ x1,y1,0,  s1,t1,  r,g,b,a },
									{ x1,y0,0,  s1,t0,  r,g,b,a } };
			vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
			pen->x += glyph->advance_x;
		}
	}
}

void orbisGlTextFinish()
{
	texture_atlas_delete(atlas);
	texture_font_delete(font);
	vertex_buffer_delete(buffer);
	orbisGlDestroyProgram(shader);
	orbisGlTextEnabled=0;
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

extern int _orbisNfs_lastopenFile_size;

void orbisGlTextInit()
{

	size_t h; // text size in pt
	atlas = texture_atlas_new( 512, 512, 1 );
	//const char * filename = "host0:hostapp/system/fonts/Tahoma_bold.ttf";
	//const char * filename = "host0:hostapp/system/fonts/NewYork.ttf";

	const char *filename = "system/fonts/Tahoma_bold.ttf";
          void *ttf      = orbisNfsGetFileContent( filename );

	buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
	debugNetPrintf(DEBUG,"[ORBISGL] %s before texture_font_new \n",__FUNCTION__);

    font = texture_font_new_from_memory(atlas, 22, ttf, _orbisNfs_lastopenFile_size);
	debugNetPrintf(DEBUG,"[ORBISGL] %s after texture_font_new \n",__FUNCTION__);

	// setup vertex positions for all the text we will draw:

	// 1. a title
    orbisGlTextSetupBuffer(filename, 10, 50,
    					  (vec4){{ 1.f, 1.f, 1.f, 1.f }});
//debugNetPrintf(INFO, "%d, %d\n", vector_size( buffer->items ), vector_size( buffer->items ));

    // 2. many lines
    t_credits.off = vector_size( buffer->items );
    creditsDrawText();
	t_credits.len = vector_size( buffer->items ) - t_credits.off;
debugNetPrintf(INFO, "t_credits(%d, %d)\n", t_credits.off, t_credits.len);

	t_footer.off = vector_size( buffer->items );
    footerDrawText();
	t_footer.len = vector_size( buffer->items ) - t_footer.off;
debugNetPrintf(INFO, "t_footer(%d, %d)\n", t_footer.off, t_footer.len);

	t_browser.off = vector_size( buffer->items );
    browserDrawText();
	t_browser.len = vector_size( buffer->items ) - t_browser.off;
debugNetPrintf(INFO, "t_browser(%d, %d)\n", t_browser.off, t_browser.len);

    /* create texture and upload atlas into gpu memory */
    glGenTextures  ( 1, &atlas->id );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_ALPHA, atlas->width, atlas->height,
                                    0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->data );

	shader = orbisGlCreateProgramFromNfs("system/shaders/v3f-t2f-c4f.vert","system/shaders/v3f-t2f-c4f.frag");
	if(!shader)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s program creation failed\n",__FUNCTION__);
		orbisGlTextEnabled=0;
		sleep(2);
	}
    

	mat4_set_identity( &projection );
	mat4_set_identity( &model );
	mat4_set_identity( &view );

	reshape(1920, 1080);
	orbisGlTextEnabled=1;
}

