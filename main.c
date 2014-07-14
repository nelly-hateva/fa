#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "automaton.h"

#define MAXIMUM_LINE_SIZE 100
#define DICTIONARY_SIZE 133334

// wc -L filename gives the length of the longest line in a file


int main( int argc, char* argv[] ) {
    if ( argc != 2 )
    {
        printf("Wrong usage. Please specify a file name.\n");
        exit(0);
    }
    FILE *file;
    file = fopen(argv[1],"r");
    if( file == NULL )
    {
        printf("Error while opening the file.\n");
        exit(-1);
    }
    char line [MAXIMUM_LINE_SIZE];
    pair dict[DICTIONARY_SIZE];

    const char s[2] = " ";
    int dictionary_size = 0;
    char* token; char* first; char* second;
    while ( fgets ( line, sizeof line, file ) != NULL )
    {
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        first = token;
        token = strtok(NULL, s);
        second = token;
        pair entry = {.first = strdup(first), .second = strdup(second)};
        dict[dictionary_size] = entry;
        dictionary_size++;
    }
    fclose ( file );

    build_subseq_trans(dict, dictionary_size);

    return 0;
}
