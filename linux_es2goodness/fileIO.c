#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


size_t _orbisFile_lastopenFile_size;
// --------------------------------------------------------- buf_from_file ---
unsigned char *orbisFileGetFileContent( const char *filename )
{
    _orbisFile_lastopenFile_size = -1;

    FILE *file = fopen( filename, "rb" );
    if( !file )
        { fprintf( stderr, "Unable to open file \"%s\".\n", filename ); return 0; }

    fseek( file, 0, SEEK_END );
    size_t size = ftell( file );
    fseek(file, 0, SEEK_SET );

    unsigned char * buffer = (unsigned char *) malloc( (size +1) * sizeof( char *) );
    fread( buffer, sizeof(char), size, file );
    buffer[size] = 0;
    _orbisFile_lastopenFile_size = size;
    fclose( file );
    return buffer;
}
