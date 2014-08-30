#include <stdio.h>
#include <stdlib.h>
#include "automaton.h"


int main( int argc, char* argv[] )
{
    if ( argc != 2 )
    {
        printf("Wrong usage. Please specify a file name.\n");
        exit(0);
    }

    create_minimal_transducer_for_given_list(argv[1]);

    return 0;
}
