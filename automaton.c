#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pair.h"
#include "hash.h"

#define INITIAL_DICTIONARY_SIZE 10000
#define INITIAL_STATES_NUMBER 2048
#define INITIAL_TRANSITIONS_NUMBER 2048
#define FREE_NUMBERS_SIZE 64

#define MAXIMUM_WORD_SIZE 64
#define MAXIMUM_LINE_SIZE 128
// wc -L file - the length of the longest line in the file

#define FIRST_CHAR 97
#define LAST_CHAR 122
#define ALPHABET_SIZE 26

pair* dictionary;
int memory_for_dictionary;
int dictionary_size;

int* free_states_numbers;
int* free_transitions_numbers;
int free_state;
int free_transition;

int* first_transition;
int* next_transition;

char* final;
int* from;
int* to;
char* label;
char** final_state_output;
char** output_transition;
int start;

int memory_for_states;
int memory_for_transitions;
int number_of_states;
int number_of_transitions;


// COMMON STRING OPERATIONS

void longest_common_prefix(char* first_word, char* second_word, char* prefix)
{
    int i = 0;
    int first_word_length = strlen(first_word);
    int second_word_length = strlen(second_word);

    while (i < first_word_length && i < second_word_length && first_word[i] == second_word[i])
        ++i;
    strncpy(prefix, first_word, i);
    prefix[i] = '\0';
}


void string_remainder(char* first_word, char* second_word, char* remainder)
{
    int first_word_length = strlen(first_word);
    int difference = strlen(second_word) - first_word_length;

    if (difference > 0)
        memcpy(remainder, second_word + first_word_length, difference);
    remainder[difference] = '\0';
}


// MEMORY MANAGEMENT OPERATIONS

void allocate_memory()
{
    number_of_states = 0;
    number_of_transitions = 0;
    dictionary_size = 0;
    memory_for_states = INITIAL_STATES_NUMBER;
    memory_for_transitions = INITIAL_TRANSITIONS_NUMBER;
    memory_for_dictionary = INITIAL_DICTIONARY_SIZE;

    first_transition = (int*) malloc(memory_for_states * sizeof(int));
    final = (char*) malloc(memory_for_states * sizeof(char));

    next_transition = (int*) malloc(memory_for_transitions * sizeof(int));
    from = (int*) malloc(memory_for_transitions * sizeof(int));
    to = (int*) malloc(memory_for_transitions * sizeof(int));
    label = (char*) malloc(memory_for_transitions * sizeof(char));
    final_state_output = (char**) malloc(memory_for_transitions * sizeof(char*));
    output_transition = (char**) malloc(memory_for_transitions * sizeof(char*));

    free_states_numbers = (int*) malloc(FREE_NUMBERS_SIZE * sizeof(int));
    free_transitions_numbers = (int*) malloc(FREE_NUMBERS_SIZE * sizeof(int));
    free_state = -1;
    free_transition = -1;

    dictionary = (pair*) malloc(memory_for_dictionary * sizeof(pair));

    int i;
    for (i = 0; i < memory_for_states; ++i)
    {
        first_transition[i] = -1;
        final[i] = -1;
    }

    for (i = 0; i < memory_for_transitions; ++i)
    {
        next_transition[i] = -1;
        from[i] = -1;
        to[i] = -1;
        label[i] = -1;
        final_state_output[i] = "";
    }

    for (i = 0; i < FREE_NUMBERS_SIZE; ++i)
    {
        free_states_numbers[i] = -1;
        free_transitions_numbers[i] = -1;
    }

}


void free_memory()
{
    free(first_transition);
    free(final);
    free(next_transition);
    free(from);
    free(to);
    free(label);
    free(final_state_output);
    free(output_transition);
    free(free_states_numbers);
    free(free_transitions_numbers);
    free(dictionary);
}


void reallocate_memory_for_states()
{
    if (number_of_states >= memory_for_states)
    {
        int previous_value_memory_for_states = memory_for_states;
        memory_for_states *= 2;
        first_transition = (int*) realloc(first_transition, memory_for_states * sizeof(int));
        final = (char*) realloc(final, memory_for_states * sizeof(char));

        int i;
        for (i = previous_value_memory_for_states; i < memory_for_states; ++i)
        {
            first_transition[i] = -1;
            final[i] = -1;
        }
    }
}


void reallocate_memory_for_transitions()
{
    if (number_of_transitions >= memory_for_transitions)
    {
        int previous_value_memory_for_transitions = memory_for_transitions;
        memory_for_transitions *= 2;
        next_transition = (int*) realloc(next_transition, memory_for_transitions * sizeof(int));
        from = (int*) realloc(from, memory_for_transitions * sizeof(int));
        to = (int*) realloc(to, memory_for_transitions * sizeof(int));
        label = (char*) realloc(label, memory_for_transitions * sizeof(char));
        final_state_output = (char**) realloc(final_state_output, memory_for_transitions * sizeof(char*));
        output_transition = (char**) realloc(output_transition, memory_for_transitions * sizeof(char*));

        int i;
        for (i = previous_value_memory_for_transitions; i < memory_for_transitions; ++i)
        {
            next_transition[i] = -1;
            from[i] = -1;
            to[i] = -1;
            label[i] = -1;
            final_state_output[i] = "";
        }
    }
}


void reallocate_memory_for_dictionary()
{
    if (dictionary_size >= memory_for_dictionary)
    {
        memory_for_dictionary *= 2;
        dictionary = (pair*) realloc(dictionary, memory_for_dictionary * sizeof(pair));
    }
}


// DICTIONARY OPERATIONS

void read_dictionary(char* filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }
    char line[MAXIMUM_LINE_SIZE]; const char s[2] = " ";
    char* token; char* first; char* second;

    while (fgets(line, sizeof line, file) != NULL)
    {
        reallocate_memory_for_dictionary();
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        first = token;
        token = strtok(NULL, s);
        second = token;
        pair entry = {.first = strdup(first), .second = strdup(second)};
        dictionary[dictionary_size] = entry;
        ++dictionary_size;
    }
    fclose(file);
}


// AUTOMATON OPERATIONS

int new_state()
{
    if (free_state == -1)
        return number_of_states;
    else
    {
        int state = free_states_numbers[free_state];
        --free_state;
        return state;
    }
}


void add_state(int state)
{
    reallocate_memory_for_states();
    final[state] = 0;
    ++number_of_states;
}


void set_transition(int from_state, char character, int to_state)
{
    int position;
    if (free_transition == -1)
        position = number_of_transitions;
    else
    {
        position = free_transitions_numbers[free_transition];
        --free_transition;
    }

    reallocate_memory_for_transitions();
    from[position] = from_state;
    to[position] = to_state;
    label[position] = character;

    if (first_transition[from_state] == -1)
        first_transition[from_state] = position;

    int transition = first_transition[from_state];
    int previous = transition;
    while (label[transition] < character && transition != -1)
    {
        previous = transition;
        transition = next_transition[transition];
    }
    if (previous != transition)
    {
        next_transition[previous] = position;
        next_transition[position] = transition;
    }
    else
    {
        first_transition[from_state] = position;
        next_transition[position] = transition;
        next_transition[previous] = -1;
    }
    ++number_of_transitions;
}


void set_output(int state, char character, char* string)
{
    int transition = first_transition[state];
    while (transition != -1)
    {
        if (label[transition] == character)
        {
            output_transition[transition] = strdup(string);
            return;
        }
        transition = next_transition[transition];
    }
}


void delete_transition(int transition)
{
    int from_state = from[transition];
    int outgoing_transition = first_transition[from_state];
    if (outgoing_transition == transition)
    {
        first_transition[from_state] = next_transition[outgoing_transition];
        next_transition[outgoing_transition] = -1;
    }
    else
    {
        int previous_transition = outgoing_transition;
        while (outgoing_transition != transition)
        {
            previous_transition = outgoing_transition;
            outgoing_transition = next_transition[outgoing_transition];
        }
        next_transition[previous_transition] = next_transition[outgoing_transition];
    }
    from[transition] = -1;
    to[transition] = -1;
    label[transition] = -1;
    next_transition[transition] = -1;

    ++free_transition;
    free_transitions_numbers[free_transition] = transition;
    --number_of_transitions;
}


void delete_state(int state)
{
    int transition = first_transition[state];
    int next;
    while (transition != -1)
    {
        next = next_transition[transition];
        delete_transition(transition);
        transition = next;
    }

    int i;
    for (i = 0; i < number_of_transitions; ++i)
        if (to[i] == state)
            delete_transition(i);

    first_transition[state] = -1;
    final[state] = -1;

    ++free_state;
    free_states_numbers[free_state] = state;
    --number_of_states;
}


int transition(int state, char character)
{
    int transition = first_transition[state];
    while (transition != -1)
    {
        if (label[transition] == character)
            return to[transition];
        transition = next_transition[transition];
    }
    return -1;
}


void path(char* word, int* path_states, int* path_states_length)
{
    int word_length = strlen(word), state = start, i;
    *path_states_length = 0;

    path_states[*path_states_length] = state;
    ++(*path_states_length);
    for (i = 0; i < word_length; ++i)
    {
        state = transition(state, word[i]);
        if (state != -1)
        {
            path_states[*path_states_length] = state;
            ++(*path_states_length);
        }
        else
            return;
    }
}


void delete_word(char* word)
{
    int current_state;
    int transition;
    int outgoing_transitions;
    int path_states[MAXIMUM_WORD_SIZE];
    int path_states_size;
    int number_of_outgoing_transitions[MAXIMUM_WORD_SIZE];

    path(word, path_states, &path_states_size);

    int j = 1;
    current_state = path_states[path_states_size - j];
    final[current_state] = -1;
    while (j <= path_states_size)
    {
        current_state = path_states[path_states_size - j];
        outgoing_transitions = 0;
        transition = first_transition[current_state];
        while (transition != -1)
        {
            transition = next_transition[transition];
            ++outgoing_transitions;
            if (j == 1 && outgoing_transitions > 1)
                break;
            else if (outgoing_transitions > 2)
                break;
        }
        number_of_outgoing_transitions[path_states_size - j] = outgoing_transitions;
        ++j;
    }

    for (j = path_states_size - 1; j >= 0; --j)
    {
        if (j == path_states_size - 1 && number_of_outgoing_transitions[j] == 0)
            delete_state(path_states[j]);
        else if (number_of_outgoing_transitions[j] <= 1)
            delete_state(path_states[j]);
        else
            break;
    }
}


char* output_transition_label(int state, char character)
{
    int transition = first_transition[state];
    while (transition != -1)
    {
        if (label[transition] == character)
            return output_transition[transition];
        transition = next_transition[transition];
    }
    return "";
}


void output_label(char* word, int position, char* result)
{
    int i;
    int state1 = start, state2;
    int result_length = 0;
    char* output;

    for (i = 0; i <= position; ++i)
    {
        state2 = transition(state1, word[i]);
        if (state2 != -1)
        {
            output = output_transition_label(state1, word[i]);
            result_length += strlen(output);
            if (i == 0)
                strcpy(result, output);
            else
                strcat(result, output);
            state1 = state2;
        }
        else
            break;
    }
    result[result_length] = '\0';
}


int hash_code(int state)
{
    int code;
    int transition = first_transition[state];
    if (transition == -1)
        code = 1;
    else
    {
        code = ((label[transition] * 257) % hash_length) + to[transition];
        transition = next_transition[transition];
    }
    while (transition != -1)
    {
        code = ((code * 257) % hash_length) + label[transition];
        code = ((code * 257) % hash_length) + to[transition];
        code = ((code * 257) % hash_length) + strlen(output_transition[transition]);
        transition  = next_transition[transition];
    }
    return (code + final[state]) % hash_length;
}


int find_equivalent(int state)
{
    return search(state, hash_code(state));
}


char equal(int state1, int state2)
{
    char equals = 0;
    if (final[state1] == final[state2] && strcmp(final_state_output[state1], final_state_output[state2]) == 0)
        equals = 1;

    if (equals)
    {
        int transition1 = first_transition[state1], transition2 = first_transition[state2];
        while (label[transition1] == label[transition2] && output_transition[transition1] == output_transition[transition2] && next_transition[transition1] != -1 && next_transition[transition2] != -1)
        {
            transition1 = next_transition[transition1];
            transition2 = next_transition[transition2];
        }
        if (next_transition[transition2] != next_transition[transition1] || label[transition1] != label[transition2] || to[transition1] != to[transition2] || strcmp(output_transition[transition1], output_transition[transition2]) != 0)
            return 0;
    }
    return equals;
}


void reduce(char* word, int length)
{
    int i;
    int position;
    int current_state, state;
    int path_states[MAXIMUM_WORD_SIZE];
    int path_states_length;

    path(word, path_states, &path_states_length);
    for (i = 1; path_states_length - i >= length; ++i)
    {
        position = path_states_length - i;
        current_state = path_states[position];
        state = find_equivalent(current_state);
        if (state != -1)
        {
            delete_state(current_state);
            set_transition(path_states[position - 1], word[position - 1], state);
        }
        else
            insert(current_state, hash_code(current_state));
    }
}


void depth_first_search(int state, char* label, char* output_label, FILE* file)
{
    char character;
    int vertex;
    int label_length = strlen(label), output_label_length = strlen(output_label), length;

    if (final[state])
    {
        char* new_output_label = malloc(output_label_length + strlen(final_state_output[state]) + 1);
        strcpy(new_output_label, output_label);
        strcat(new_output_label, final_state_output[state]);
        new_output_label[output_label_length + strlen(final_state_output[state]) + 1] = '\0';
        fprintf(file, "%s %s\n", label, new_output_label);
        free(new_output_label);
    }

    for (character = FIRST_CHAR; character <= LAST_CHAR; ++character) {
        vertex = transition(state, character);
        if (vertex != -1)
        {
            char* new_label = malloc(label_length + 2);
            strcpy(new_label, label);
            new_label[label_length] = character;
            new_label[label_length + 1] = '\0';

            length = output_label_length + strlen(output_transition_label(state, character)) + 1;
            char* new_output_label = malloc(length);
            strcpy(new_output_label, output_label);
            strcat(new_output_label, output_transition_label(state, character));
            new_output_label[length] = '\0';

            depth_first_search(vertex, new_label, new_output_label, file);
            free(new_label);
            free(new_output_label);
        }
    }
}


void print_transducer(char* filename)
{
    FILE *file;
    if ((file = fopen(filename, "w")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }
    depth_first_search(start, "", "", file);
    fclose(file);
}


void create_minimal_transducer_for_given_list(char* inputfile, char* outputfile)
{
    allocate_memory();
    initialize_hash();
    read_dictionary(inputfile);

    int i, j, k;
    int character;
    char *current_word, *current_output, *previous_word;
    char* current_output_label;
    int current_word_length;

    int temporary_states[MAXIMUM_WORD_SIZE];
    int prefix_states[MAXIMUM_WORD_SIZE];
    int prefix_states_length;
    int suffix_states[MAXIMUM_WORD_SIZE];

    char* output_labels_new_values[MAXIMUM_WORD_SIZE];
    char* output_labels[MAXIMUM_WORD_SIZE];

    char label_output[MAXIMUM_WORD_SIZE];
    int label_output_length;
    char prefix[MAXIMUM_WORD_SIZE];
    char remainder[MAXIMUM_WORD_SIZE];
    char* prefix_states_remainders_previous_values[MAXIMUM_WORD_SIZE][ALPHABET_SIZE] = {{NULL}};
    char* prefix_states_output_labels_previous_values[MAXIMUM_WORD_SIZE] = {NULL};

    current_word = dictionary[0].first;
    current_output = dictionary[0].second;
    current_word_length = strlen(current_word);

    for (i = 0; i <= current_word_length; ++i)
        add_state(i);

    start = 0;
    final[current_word_length] = 1;
    final_state_output[current_word_length] = "";

    for (i = 0; i < current_word_length; ++i)
        set_transition(i, current_word[i], i + 1);

    set_output(0, current_word[0], current_output);
    for (i = 1; i < current_word_length; ++i)
        set_output(i, current_word[i], "");

    for (j = 1; j < dictionary_size; ++j)
    {
        previous_word = current_word;
        current_word = dictionary[j].first;
        current_output = dictionary[j].second;
        current_word_length = strlen(current_word);

        path(current_word, prefix_states, &prefix_states_length);
        reduce(previous_word, prefix_states_length);

        for (i = 0; i <= current_word_length - prefix_states_length ; ++i)
        {
            suffix_states[i] = new_state();
            add_state(suffix_states[i]);
        }

        for (i = 0; i < prefix_states_length; ++i)
            temporary_states[i] = prefix_states[i];
        for (i = prefix_states_length; i <= current_word_length; ++i)
            temporary_states[i] = suffix_states[i - prefix_states_length];

        for (i = 0; i < current_word_length; ++i)
        {
            output_label(current_word, i, label_output);
            longest_common_prefix(label_output, current_output, prefix);
            output_labels_new_values[i] = strdup(prefix);
            output_labels[i] = strdup(label_output);
        }

        final[suffix_states[current_word_length - prefix_states_length]] = 1;
        for (i = prefix_states_length - 1; i < current_word_length; ++i)
            set_transition(temporary_states[i], current_word[i], temporary_states[i + 1]);

        for (i = 0; i < prefix_states_length - 1;  ++i)
            prefix_states_output_labels_previous_values[i] = output_transition_label(temporary_states[i], current_word[i]);

        set_output(temporary_states[0], current_word[0], output_labels_new_values[0]);
        for (i = 1; i < prefix_states_length - 1; ++i)
        {
            string_remainder(output_labels_new_values[i - 1], output_labels_new_values[i], remainder);
            set_output(temporary_states[i], current_word[i], remainder);
        }

        string_remainder(output_labels_new_values[prefix_states_length - 1], current_output, remainder);
        set_output(temporary_states[prefix_states_length - 1], current_word[prefix_states_length - 1], remainder);

        for (i = prefix_states_length; i < current_word_length; ++i)
            set_output(temporary_states[i], current_word[i], "");

        for (i = 0; i < prefix_states_length; ++i)
            for (character = FIRST_CHAR; character <= LAST_CHAR; ++character)
            {
                if (character != current_word[i] && transition(temporary_states[i], character) != -1)
                {
                    label_output[0] = '\0'; label_output_length = 0;
                    for (k = 0; k < i + 1; ++k)
                    {
                        if (k == i)
                        {
                            current_output_label = output_transition_label(temporary_states[i], character);
                            strcat(label_output, current_output_label);
                            label_output_length += strlen(current_output_label);
                        }
                        else
                        {
                            strcat(label_output, prefix_states_output_labels_previous_values[k]);
                            label_output_length += strlen(prefix_states_output_labels_previous_values[k]);
                        }
                    }

                    label_output[label_output_length] = '\0';
                    if (i == 0)
                        prefix_states_remainders_previous_values[i][character - FIRST_CHAR] = strdup(label_output);
                    else
                    {
                        string_remainder(output_labels_new_values[i - 1], label_output, remainder);
                        prefix_states_remainders_previous_values[i][character - FIRST_CHAR] = strdup(remainder);
                    }
                }
            }

        for (i = 0; i < prefix_states_length; ++i)
            for (character = FIRST_CHAR; character <= LAST_CHAR; ++character)
            {
                if (prefix_states_remainders_previous_values[i][character - FIRST_CHAR] != NULL)
                {
                    set_output(temporary_states[i], character, prefix_states_remainders_previous_values[i][character - FIRST_CHAR]);
                    free(prefix_states_remainders_previous_values[i][character - FIRST_CHAR]);
                    prefix_states_remainders_previous_values[i][character - FIRST_CHAR] = NULL;
                }
            }

        for (i = 0; i < prefix_states_length; ++i)
            if (final[prefix_states[i]])
            {
                label_output[0] = '\0';
                if (i > 0)
                    strcpy(label_output, output_labels[i-1]);
                strcat(label_output, final_state_output[prefix_states[i]]);
                if (i > 0)
                {
                    string_remainder(output_labels_new_values[i - 1], label_output, remainder);
                    final_state_output[prefix_states[i]] = strdup(remainder);
                }
                else
                    final_state_output[prefix_states[i]] = strdup(label_output);
            }
        final_state_output[suffix_states[current_word_length - prefix_states_length]] = ""; 
    }
    reduce(current_word, 1);

    print_transducer(outputfile);
    printf("NUMBER OF STATES %d\n", number_of_states);
    finalize_hash();
    free_memory();
}
