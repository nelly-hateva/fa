#include <stdio.h>
#include <stdlib.h>
#include "automaton.h"


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("cmt: No file names given\n");
        exit(EXIT_FAILURE);
    }

    create_minimal_transducer_for_given_list(argv[1], argv[2]);

    return 0;
}
