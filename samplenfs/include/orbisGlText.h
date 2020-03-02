
// then we can print them splitted
typedef struct {
    int off;
    int len;
} textline_t;



void orbisGlTextInit(void);
void orbisGlTextSetupBuffer(char *mytext,int x,int y,vec4 color);
void orbisGlTextDraw(textline_t *text_to_draw);