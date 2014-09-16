#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMUM_WORD_SIZE 64
#define MAXIMUM_LINE_SIZE 128


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("cmt: No file names given\n");
        exit(EXIT_FAILURE);
    }

    FILE *inputfile;
    if ((inputfile = fopen(argv[1], "r")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    FILE *outputfile;
    if ((outputfile = fopen(argv[2], "w")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    char line[MAXIMUM_LINE_SIZE];
    const char s[2] = " ";
    char* token; char* first; char* second;
    char* previous_word = calloc(MAXIMUM_WORD_SIZE, sizeof(char));

    while (fgets(line, sizeof line, inputfile) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        first = token;
        token = strtok(NULL, s);
        second = token;
        if (strcmp(first, previous_word) != 0)
            fprintf(outputfile, "%s %s\n", first, second);
        strcpy(previous_word, first);
    }

    free(previous_word);
    fclose(inputfile);
    fclose(outputfile);
    return 0;
}
