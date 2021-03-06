#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define INITIAL_STATES_NUMBER 8192
#define INITIAL_TRANSITIONS_NUMBER 2048
#define FREE_NUMBERS_SIZE 64

#define MAXIMUM_WORD_SIZE 64
#define MAXIMUM_LINE_SIZE 128
// wc -L file - the length of the longest line in the file

#define FIRST_CHAR 33 // ! OR 97 a
#define LAST_CHAR 126 // ~ OR 122 z
#define ALPHABET_SIZE 94 // OR 26

extern int hash_size;

int* free_states_numbers;
int* free_transitions_numbers;
int memory_for_free_states;
int memory_for_free_transitions;
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

int temporary_states[MAXIMUM_WORD_SIZE];
int prefix_states_length;

char* current_output_label;
char* output_labels_new_values[MAXIMUM_WORD_SIZE];
char* output_labels[MAXIMUM_WORD_SIZE];


char label_output[MAXIMUM_WORD_SIZE];
int label_output_length;
char prefix[MAXIMUM_WORD_SIZE];
char rest[MAXIMUM_WORD_SIZE];
char* prefix_states_remainders_previous_values[MAXIMUM_WORD_SIZE][ALPHABET_SIZE] = {{NULL}};
char* prefix_states_output_labels_previous_values[MAXIMUM_WORD_SIZE] = {NULL};

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

    free_state = -1;
    free_transition = -1;

    memory_for_states = INITIAL_STATES_NUMBER;
    memory_for_transitions = INITIAL_TRANSITIONS_NUMBER;
    memory_for_free_states =  FREE_NUMBERS_SIZE;
    memory_for_free_transitions = FREE_NUMBERS_SIZE;

    first_transition = (int*) malloc(memory_for_states * sizeof(int));
    final = (char*) malloc(memory_for_states * sizeof(char));

    next_transition = (int*) malloc(memory_for_transitions * sizeof(int));
    from = (int*) malloc(memory_for_transitions * sizeof(int));
    to = (int*) malloc(memory_for_transitions * sizeof(int));
    label = (char*) malloc(memory_for_transitions * sizeof(char));
    final_state_output = (char**) malloc(memory_for_transitions * sizeof(char*));
    output_transition = (char**) malloc(memory_for_transitions * sizeof(char*));

    free_states_numbers = (int*) malloc(memory_for_free_states * sizeof(int));
    free_transitions_numbers = (int*) malloc(memory_for_free_transitions * sizeof(int));

    int i;
    for (i = 0; i < memory_for_transitions; ++i)
        next_transition[i] = -1;
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
}


void reallocate_memory_for_states()
{
    if (number_of_states >= memory_for_states)
    {
        memory_for_states *= 2;
        first_transition = (int*) realloc(first_transition, memory_for_states * sizeof(int));
        final = (char*) realloc(final, memory_for_states * sizeof(char));
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
            next_transition[i] = -1;
    }
}


void reallocate_memory_for_free_positions()
{
    if (free_state >= memory_for_free_states)
    {
        memory_for_free_states *= 2;
        free_states_numbers = (int*) realloc(free_states_numbers, memory_for_free_states * sizeof(int));
    }
    else if (free_transition >= memory_for_free_transitions)
    {
        memory_for_free_transitions *= 2;
        free_transitions_numbers = (int*) realloc(free_transitions_numbers, memory_for_free_transitions * sizeof(int));
    }
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
    first_transition[state] = -1;
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

    next_transition[position] = first_transition[from_state];
    first_transition[from_state] = position;
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


void remove_transition(int transition)
{
    from[transition] = -1;
    to[transition] = -1;
    label[transition] = -1;
    next_transition[transition] = -1;

    ++free_transition;
    reallocate_memory_for_free_positions();
    free_transitions_numbers[free_transition] = transition;
    --number_of_transitions;
}


void delete_transition(int from_state, char character)
{
    int transition = first_transition[from_state];
    while (transition != -1)
    {
        if (label[transition] == character)
            break;
        transition = next_transition[transition];
    }

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

    remove_transition(transition);
}


void delete_state(int state)
{
    int transition = first_transition[state];
    int next;
    while (transition != -1)
    {
        next = next_transition[transition];
        remove_transition(transition);
        transition = next;
    }
    first_transition[state] = -1;
    final[state] = -1;

    ++free_state;
    reallocate_memory_for_free_positions();
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


void prefix_states_output_labels(char* word, int* temporary_states, char** output_transition_labels, char** output_labels , int length)
{
    int i;
    for (i = 0; i < length;  ++i)
    {
        output_transition_labels[i] = output_transition_label(temporary_states[i], word[i]);
        if (i > 0)
        {
            strcpy(output_labels[i], output_labels[i - 1]);
            strcat(output_labels[i], output_transition_labels[i]);
            output_labels[i][strlen(output_labels[i - 1]) + strlen(output_transition_labels[i])] = '\0';
        }
        else
        {
            strcpy(output_labels[i], output_transition_labels[i]);
            output_labels[i][strlen(output_transition_labels[i])] = '\0';
        }
    }
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


char equivalent(int state1, int state2)
{
    char equivalent = 0;
    if (final[state1] == final[state2] &&
        (final[state1] != 1 || strcmp(final_state_output[state1], final_state_output[state2]) == 0))
        equivalent = 1;

    if (equivalent)
    {
        int transition1 = first_transition[state1], transition2 = first_transition[state2];
        while(transition1 != -1 && transition2 != -1 &&
              label[transition1] == label[transition2] &&
              to[transition1] == to[transition2] &&
              strcmp(output_transition[transition1], output_transition[transition2]) == 0)
        {
            transition1 = next_transition[transition1];
            transition2 = next_transition[transition2];
        }
        if (transition1 != transition2 ||
            (transition1 != -1 && transition2 != -1 && label[transition1] != label[transition2]) ||
            (transition1 != -1 && transition2 != -1 && to[transition1] != to[transition2]) ||
            (transition1 != -1 && transition2 != -1 &&
                strcmp(output_transition[transition1], output_transition[transition2]) != 0))
            return 0;
    }
    return equivalent;
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
            delete_transition(path_states[position - 1], word[position - 1]);
            set_transition(path_states[position - 1], word[position - 1], state);
        }
        else
            insert(current_state, hash_code(current_state));
    }
}


void clone(char* word, int* path_states, int* path_length, int i)
{
    if (i >= *path_length)
        return;
    else
    {
        char* output_label;
        int copy_state = new_state();
        add_state(copy_state);
        int previous_state = path_states[i - 1];
        int current_state = path_states[i];
        final[copy_state] = final[current_state];
        if (final[current_state] == 1)
            final_state_output[copy_state] = strdup(final_state_output[current_state]);

        output_label = output_transition_label(previous_state, word[i - 1]);
        delete_transition(previous_state, word[i - 1]);
        set_transition(previous_state, word[i - 1], copy_state);
        set_output(previous_state, word[i - 1], output_label);

        int transition = first_transition[current_state];
        while (transition != -1)
        {
            output_label = output_transition_label(current_state, label[transition]);
            set_transition(copy_state, label[transition], to[transition]);
            set_output(copy_state, label[transition], output_label);
            transition = next_transition[transition];
        }
        path_states[i] = copy_state;
        clone(word, path_states, path_length, i + 1);
    }
}


void extend(char* word, int* path_states, int* path_length, int* position)
{
    path(word, path_states, path_length);
    int i, j = 1; int minimum = -1;
    int incoming_transitions_count, current_state;

    while (j < *path_length && minimum == -1)
    {
        current_state = path_states[j];
        incoming_transitions_count = 0;
        for (i = 0; i < memory_for_transitions; ++i)
        {
            if (to[i] == current_state)
                ++incoming_transitions_count;
            if (incoming_transitions_count > 1)
            {
                minimum = j;
                break;
            }
        }
        ++j;
    }
    *position = minimum - 1;
    if (minimum != -1)
    {
        delete(path_states[minimum - 1], hash_code(path_states[minimum - 1]));
        return clone(word, path_states, path_length, minimum);
    }
    *position = *path_length;
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

            depth_first_search(vertex, new_label, new_output_label, file);
            free(new_label);
            free(new_output_label);
        }
    }
}


void print_transducer(char* filename, char suffix_filename)
{
    char* result_filename = malloc(strlen(filename) + 2);
    strcpy(result_filename, filename);
    result_filename[strlen(filename)] = suffix_filename;
    result_filename[strlen(filename) + 1] = '\0';

    FILE *file;
    if ((file = fopen(result_filename, "w")) == NULL)
    {
        printf("cmt: Can't open file %s\n", result_filename);
        exit(EXIT_FAILURE);
    }
    depth_first_search(start, "", "", file);
    fclose(file);
    free(result_filename);

    printf("%c NUMBER OF STATES %d\n", suffix_filename, number_of_states);
    printf("%c HASH SIZE %d\n", suffix_filename, hash_size);
    printf("%c NUMBER OF TRANSITIONS %d\n", suffix_filename, number_of_transitions);
}


void delete_word(char* word)
{
    int position;
    int path_states[MAXIMUM_WORD_SIZE];
    int path_states_length;
    extend(word, path_states, &path_states_length, &position);

    int last_deleted = -1;
    if (final[path_states[path_states_length - 1]] == 1 && path_states_length == strlen(word) + 1)
    {
        int j;
        for (j = 0; j < position; ++j)
            delete(path_states[j], hash_code(path_states[j]));
        final[path_states[path_states_length - 1]] = 0;
        int outgoing_transitions, transitionn;
        for (j = path_states_length - 1; j > 0; --j)
        {
            if (final[path_states[j]] != 1)
            {
                outgoing_transitions = 0;
                transitionn = first_transition[path_states[j]];
                while (transitionn != -1)
                {
                    transitionn = next_transition[transitionn];
                    ++outgoing_transitions;
                    if ((j == path_states_length - 1 && outgoing_transitions > 0) || (j != path_states_length - 1 && outgoing_transitions > 1))
                        break;
                }
                if (j == path_states_length - 1 && outgoing_transitions == 0)
                {
                    delete_state(path_states[j]);
                    last_deleted = j;
                }
                else if (outgoing_transitions <= 1 && j != path_states_length - 1)
                {
                    delete_state(path_states[j]);
                    last_deleted = j;
                }
                else
                    break;
            }
            else
                break;
        } 
        if (last_deleted != -1)
            delete_transition(path_states[last_deleted - 1], word[last_deleted - 1]);

        int state;
        int transition;
        int index = 0;
        int t,s;
        int longest=-1;
        char* array[ALPHABET_SIZE] = {NULL};
        int flag;
        int l;
        char prefix[MAXIMUM_WORD_SIZE];
        int pos=0;
        int bulshit = last_deleted - 1;
        char lcp[MAXIMUM_WORD_SIZE];

        if(last_deleted == -1)
            bulshit = path_states_length - 1;
        for (l = bulshit; l > 0; --l)
        {
            index = 0;
            state = path_states[l];
            transition = first_transition[state];
            while (transition != -1)
            {
                array[index] = strdup(output_transition_label(state, label[transition]));
                if (longest == -1)
                    longest = strlen(array[index]);
                else if (strlen(array[index]) > longest)
                    longest = strlen(array[index]);
                ++index;
                transition = next_transition[transition];
            }

            pos = 0;
            for (t = 0; t < longest; ++t)
            {
                flag = 1;
                for (s = 0; s < index - 1; ++s)
                    if (array[s][t] != array[s + 1][t])
                    {
                        flag = 0;
                        break;
                    }
                if (flag == 1)
                {
                    prefix[pos] = array[0][t];
                    ++pos;
                }
                else
                    break;
            }
            prefix[pos] = '\0';

            if(final[state] == 1)
            {
                longest_common_prefix(final_state_output[state], prefix, lcp);
                if (strlen(lcp) > 0)
                {
                    string_remainder(prefix, final_state_output[state], rest);
                    final_state_output[state] = strdup(rest);
                }
                else if (first_transition[state] == -1)
                {
                    set_output(path_states[l-1], word[l-1], final_state_output[state]);
                    final_state_output[state] = "";
                    continue;
                }
                else
                    break;
            }
            transition = first_transition[state];
            while (transition != -1)
            {
                string_remainder(prefix, output_transition_label(state, label[transition]), rest);
                set_output(state, label[transition], rest);
                transition = next_transition[transition];
            }
            char label_output[MAXIMUM_WORD_SIZE];
            strcpy(label_output, output_transition_label(path_states[l - 1], word[l - 1]));
            strcat(label_output, prefix);
            set_output(path_states[l - 1], word[l - 1], label_output);
        }
        reduce(word, 0);
    }
}


void add_word(char* word, char* output)
{
    int i, k;
    char character;
    int word_length = strlen(word);
    for (i = prefix_states_length; i <= word_length; ++i)
    {
        temporary_states[i] = new_state();
        add_state(temporary_states[i]);
    }

    prefix_states_output_labels(word, temporary_states, prefix_states_output_labels_previous_values, output_labels, prefix_states_length);
    for (i = 0; i < prefix_states_length; ++i)
    {
        longest_common_prefix(output_labels[i], output, prefix);
        strcpy(output_labels_new_values[i], prefix);
    }

    final[temporary_states[word_length]] = 1;
    for (i = prefix_states_length - 1; i < word_length; ++i)
        set_transition(temporary_states[i], word[i], temporary_states[i + 1]);

    set_output(temporary_states[0], word[0], output_labels_new_values[0]);
    for (i = 1; i < prefix_states_length - 1; ++i)
    {
        string_remainder(output_labels_new_values[i - 1], output_labels_new_values[i], rest);
        set_output(temporary_states[i], word[i], rest);
    }

    string_remainder(output_labels_new_values[prefix_states_length - 1], output, rest);
    set_output(temporary_states[prefix_states_length - 1], word[prefix_states_length - 1], rest);

    for (i = prefix_states_length; i < word_length; ++i)
        set_output(temporary_states[i], word[i], "");

    for (i = 0; i < prefix_states_length; ++i)
        for (character = FIRST_CHAR; character <= LAST_CHAR; ++character)
        {
            if (character != word[i] && transition(temporary_states[i], character) != -1)
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
                    string_remainder(output_labels_new_values[i - 1], label_output, rest);
                    prefix_states_remainders_previous_values[i][character - FIRST_CHAR] = strdup(rest);
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
        if (final[temporary_states[i]])
        {
            label_output[0] = '\0';
            if (i > 0)
                strcpy(label_output, output_labels[i - 1]);
            strcat(label_output, final_state_output[temporary_states[i]]);
            if (i > 0)
            {
                string_remainder(output_labels_new_values[i - 1], label_output, rest);
                final_state_output[temporary_states[i]] = strdup(rest);
            }
            else
                final_state_output[temporary_states[i]] = strdup(label_output);
        }
    final_state_output[temporary_states[word_length]] = "";
}


void add_words(char* filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAXIMUM_LINE_SIZE]; const char s[2] = " ";
    char* token; char* first; char* second;

    int position;
    while (fgets(line, sizeof line, file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        first = token;
        token = strtok(NULL, s);
        second = token;
            
        extend(first, temporary_states, &prefix_states_length, &position);
        add_word(first, second);
        reduce(first, position);
    }

    fclose(file);
    print_transducer(filename, 'a');
}


void delete_words(char* filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAXIMUM_LINE_SIZE];
    while (fgets(line, sizeof line, file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        delete_word(line);
    }

    fclose(file);
    print_transducer(filename, 'd');
}


void create_minimal_transducer_for_sorted_list(char* filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("cmt: %s: No such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }

    const char s[2] = " ";
    char line[MAXIMUM_LINE_SIZE];
    char* token;

    int i;
    int current_word_length;
    char previous_word[MAXIMUM_WORD_SIZE];
    char *current_word, *current_output;

    fgets(line, sizeof line, file);
    line[strlen(line) - 1] = '\0';
    token = strtok(line, s);
    current_word = token;
    token = strtok(NULL, s);
    current_output = token;
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
    strcpy(previous_word, current_word);

    while (fgets(line, sizeof line, file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        current_word = token;
        token = strtok(NULL, s);
        current_output = token;
        current_word_length = strlen(current_word);
        path(current_word, temporary_states, &prefix_states_length);
        reduce(previous_word, prefix_states_length);
        add_word(current_word, current_output);
        strcpy(previous_word, current_word);
    }
    reduce(current_word, 0);

    fclose(file);
    print_transducer(filename, 'r');
}


void dispatch(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("cmt: No file name given\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        allocate_memory();
        initialize_hash();

        int i;
        for (i = 0; i < MAXIMUM_WORD_SIZE; ++i)
        {
            output_labels[i] = malloc(MAXIMUM_WORD_SIZE * sizeof(char));
            output_labels_new_values[i] = malloc(MAXIMUM_WORD_SIZE * sizeof(char));
        }

        create_minimal_transducer_for_sorted_list(argv[1]);
        if (argc >= 4 && strcmp(argv[2], "--d") == 0)
            delete_words(argv[3]);
        else if (argc >= 6 && strcmp(argv[4], "--d") == 0)
            delete_words(argv[5]);
        if (argc >= 4 && strcmp(argv[2], "--a") == 0)
            add_words(argv[3]);
        else if (argc >= 6 && strcmp(argv[4], "--a") == 0)
            add_words(argv[5]);

        for (i = 0; i < MAXIMUM_WORD_SIZE; ++i)
        {
            free(output_labels[i]);
            free(output_labels_new_values[i]);
        }
        free_hash();
        free_memory();
    }
}
