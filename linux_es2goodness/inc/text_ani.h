/*
  implement some way to setup different kind of
  animations to use in print text with FreeType

  scope: reusing and sharing resources from the
  already available freetype-gl library

  2020, masterzorag
*/


// shared, from demo-font.c
extern GLuint           shader;           
extern texture_atlas_t *atlas;
extern vertex_buffer_t *buffer;


// -------------------------------------------------------------- effects ---


/* each fx have those states */
enum ani_states
{
    IN,
    OUT,
    DEFAULT,
    CLOSED
};

/* hold the current state values */
typedef struct
{
// GLuint program;
    int   status, // current ani_states
          fcount; // current framecount
    float life;   // total duration in frames
} fx_entry_t;


#define NUM  (3)  // texts we append to shared VBO
static char *text[NUM] = 
{ 
    "text_ani on glsl",
    "level++;",
    "make#liborbis",
};
