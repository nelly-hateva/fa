#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pair.h"
#include "hash.h"

#define INITIAL_DICTIONARY_SIZE 10000
#define INITIAL_STATES_NUMBER 2048
#define INITIAL_TRANSITIONS_NUMBER 2048
#define GARBAGE_SIZE 1000
#define MAX_WORD_SIZE 64
#define MAXIMUM_LINE_SIZE 100
// wc -L filename gives the length of the longest line in a file

extern int find(int, int);

pair* dictionary;
int memory_for_dictionary;
int dictionary_size;

int* garbage_states;
int* garbage_transitions;
int garbage_state;
int garbage_transition;

int* first_transition;
int* next_transition;

char* final;
int* from;
int* to;
char* label;
char** psi;
char** lambda;
int start;

int number_of_states;
int number_of_transitions;
int memory_for_states;
int memory_for_transitions;

int h(int);
void delete_transition(int);
char equal(int, int);


// COMMON STRING OPERATIONS

void longest_common_prefix(char* first_word, char* second_word, char* prefix)
{
    int i = 0;
    int first_word_length = strlen(first_word);
    int second_word_length = strlen(second_word);
    while(i < first_word_length && i < second_word_length && first_word[i] == second_word[i])
        i++;
    strncpy(prefix, first_word, i);
    prefix[i] = '\0';
}


void string_remainder(char* first_word, char* second_word, char* remainder)
{
    int first_word_length = strlen(first_word);
    int difference = strlen(second_word) - first_word_length;
    if(difference > 0)
        memcpy(remainder, second_word + first_word_length, difference);
    remainder[difference] = '\0';
}


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
    psi = (char**) malloc(memory_for_transitions * sizeof(char*));
    lambda = (char**) malloc(memory_for_transitions * sizeof(char*));

    garbage_states = (int*) malloc(GARBAGE_SIZE * sizeof(int));
    garbage_transitions = (int*) malloc(GARBAGE_SIZE * sizeof(int));
    garbage_state = -1;
    garbage_transition = -1;

    dictionary = (pair*) malloc(memory_for_dictionary * sizeof(pair));

    int i;
    for(i = 0; i < memory_for_states; i++)
    {
        first_transition[i] = -1;
        final[i] = -1;
    }

    for(i = 0; i < memory_for_transitions; i++)
    {
        next_transition[i] = -1;
        from[i] = -1;
        to[i] = -1;
        label[i] = -1;
        psi[i] = "";
    }

    for(i = 0; i < GARBAGE_SIZE; i++)
    {
        garbage_states[i] = -1;
        garbage_transitions[i] = -1;
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
    free(psi);
    free(lambda);
    free(garbage_states);
    free(garbage_transitions);
    free(dictionary);
}

void reallocate_memory_for_states()
{
    if(number_of_states >= memory_for_states)
    {
        int old_memory_for_states = memory_for_states;
        memory_for_states *= 2;
        first_transition = (int*) realloc(first_transition, memory_for_states * sizeof(int));
        final = (char*) realloc(final, memory_for_states * sizeof(char));

        int i;
        for(i = old_memory_for_states; i < memory_for_states; i++)
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
        int old_memory_for_transitions = memory_for_transitions;
        memory_for_transitions *= 2;
        next_transition = (int*) realloc(next_transition, memory_for_transitions * sizeof(int));
        from = (int*) realloc(from, memory_for_transitions * sizeof(int));
        to = (int*) realloc(to, memory_for_transitions * sizeof(int));
        label = (char*) realloc(label, memory_for_transitions * sizeof(char));
        psi = (char**) realloc(psi, memory_for_transitions * sizeof(char*));
        lambda = (char**) realloc(lambda, memory_for_transitions * sizeof(char*));

        int i;
        for(i = old_memory_for_transitions; i < memory_for_transitions; i++)
        {
            next_transition[i] = -1;
            from[i] = -1;
            to[i] = -1;
            label[i] = -1;
            psi[i] = "";
        }
    }
}


void reallocate_memory_for_dictionary()
{
    if(dictionary_size >= memory_for_dictionary)
    {
        memory_for_dictionary *= 2;
        dictionary = (pair*) realloc(dictionary, memory_for_dictionary * sizeof(pair));
    }
}


void read_dictionary(char* filename)
{
    FILE *file;
    if( (file = fopen(filename,"r")) == NULL )
    {
        printf("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    char line[MAXIMUM_LINE_SIZE]; const char s[2] = " ";
    char* token; char* first; char* second;

    while ( fgets ( line, sizeof line, file ) != NULL )
    {
        reallocate_memory_for_dictionary();
        line[strlen(line) - 1] = '\0';
        token = strtok(line, s);
        first = token;
        token = strtok(NULL, s);
        second = token;
        pair entry = {.first = strdup(first), .second = strdup(second)};
        dictionary[dictionary_size] = entry;
        dictionary_size++;
    }
    fclose ( file );
}


int get_free_state_number()
{
    if(garbage_state == -1)
        return number_of_states;
    else
    {
        int state = garbage_states[garbage_state];
        garbage_state--;
        return state;
    }
}





void add_state(int n)
{
    reallocate_memory_for_states();
    final[n] = 0;
    number_of_states++;
}

void add_transition(int from_state, char label_transition, int to_state)
{
    if(find(from_state, h(from_state)) != -1)//??
        delete(from_state,h(from_state)); // ???
    int position;
    if(garbage_transition == -1)
        position = number_of_transitions;
    else
    {
        position = garbage_transitions[garbage_transition];
        garbage_transition--;
    }

    reallocate_memory_for_transitions();
    from[position] = from_state;
    to[position] = to_state;
    label[position] = label_transition;

    if(first_transition[from_state] == -1)
        first_transition[from_state] = position;

    int transition = first_transition[from_state];
    int prev = transition;
    while(label[transition] < label_transition && transition != -1)
    {
        prev = transition;
        transition = next_transition[transition];
    }
    if(prev != transition)
    {
        next_transition[prev] = position;
        next_transition[position] = transition;
    }
    else
    {
        first_transition[from_state] = position;
        next_transition[position] = transition;
        next_transition[prev] = -1;
    }
    number_of_transitions++;

}

void add_output(int state, char c, char* string)
{
    int transition = first_transition[state];
    while(transition != -1)
    {
        if(label[transition] == c)
        {
            lambda[transition] = strdup(string);
            return;
        }
        transition = next_transition[transition];
    }
}

void delete_state(int n)
{
    int transition = first_transition[n];
    int next;
    while(transition != -1)
    {
        next = next_transition[transition];
        delete_transition(transition);
        transition = next;
    }
    int i;
    for(i = 0; i < number_of_transitions; i++)
        if(to[i] == n)
            delete_transition(i);

    first_transition[n] = -1;
    final[n] = -1;

    garbage_state++;
    garbage_states[garbage_state] = n;
    number_of_states--;
}


void delete_transition(int n)
{
    int from_state = from[n];
    int transition = first_transition[from_state];
    if(transition == n)
    {
        first_transition[from_state] = next_transition[transition];
        next_transition[transition] = -1;
    }
    else
    {
        int prev = transition;
        while(transition != n)
        {
            prev = transition;
            transition = next_transition[transition];
        }
        next_transition[prev] = next_transition[transition];
    }
    from[n] = -1;
    to[n] = -1;
    label[n] = -1;
    next_transition[n] = -1;

    garbage_transition++;
    garbage_transitions[garbage_transition] = n;
    number_of_transitions--;
}


void delete_transition_by_signature(int from_state, char label_transition)
{
    int transition = first_transition[from_state];
    while(transition != -1)
    {
        if(label[transition] == label_transition)
        {
            delete_transition(transition);
            break;
        }
        transition = next_transition[transition];
    }
}


int delta(int state, char letter)
{
    int transition = first_transition[state];
    while(transition != -1)
    {
        if(label[transition] == letter)
            return to[transition];
        transition = next_transition[transition];
    }
    return -1;
}


int* path(char* alpha, int* tau, int* tau_len)
{
    int alpha_len = strlen(alpha), state = start, i;
    *tau_len = 0;

    tau[*tau_len] = state;
    (*tau_len)++;
    for(i = 0; i < alpha_len; i++)
    {
        state = delta(state, alpha[i]);
        if(state != -1)
        {
            tau[*tau_len] = state;
            (*tau_len)++;
        }
        else
            return tau;
    }
    return tau;
}


char* lambda_transition(int q1, char c, int q2)
{
    int transition = first_transition[q1];
    while(transition != -1)
    {
        if(label[transition] == c && to[transition] == q2)
        {
            return lambda[transition];
        }
        transition = next_transition[transition];
    }
    return strdup("");
}


void output_label(char* word, int position, char* result)
{
    int i;
    int state1 = start, state2;

    for(i = 0; i < MAX_WORD_SIZE; i++)
       result[i] = 0;
    for(i = 0; i <= position; i++)
    {
        state2 = delta(state1, word[i]);
        if(state2 != -1)
        {
            strcat(result, lambda_transition(state1, word[i], state2));
            state1 = state2;
        }
    }
}


int find_equivalent(int state)
{
    return search(state, h(state));
}


void reduce(char* alpha, int length)
{
    int i, p, tp, state, tau_len;
    int tau[MAX_WORD_SIZE];
    path(alpha, tau, &tau_len);
    for(i = 1; tau_len - i >= length; ++i )
    {
        p = tau_len - i;
        tp = tau[p];
        state = find_equivalent(tp);
        if(state != -1)
        {
            delete_state(tp);
            delete_transition_by_signature(tau[p-1], alpha[p-1]);
            add_transition(tau[p-1], alpha[p-1], state);
        }
        else
            insert(tp, h(tp));
    }
}


int h(int n)
{
    int hash_code;
    int transition = first_transition[n];
    if (transition == -1) hash_code = 1;
    else
    {
        hash_code = ((label[transition] * 257) % hash_length) + to[transition];
        transition = next_transition[transition];
    }
    while(transition != -1)
    {
        hash_code = ((hash_code * 257) % hash_length) + label[transition];
        hash_code = ((hash_code * 257) % hash_length) + to[transition];
        hash_code = ((hash_code * 257) % hash_length) + lambda[transition];
        transition  = next_transition[transition];
    }
    return (hash_code + final[n]) % hash_length;
}


char equal(int n, int m)
{
    char flag = 0;
    if(final[n] == final[m] && strcmp(psi[n], psi[m]) == 0)
        flag = 1;
    if(flag)
    {
        int transition_n = first_transition[n], transition_m = first_transition[m];
        while(label[transition_n] == label[transition_m] && lambda[transition_n] == lambda[transition_m] && next_transition[transition_n] != -1 && next_transition[transition_m] != -1)
        {
            transition_n = next_transition[transition_n];
            transition_m = next_transition[transition_m];
        }
        if(next_transition[transition_m] != next_transition[transition_n] || label[transition_n] != label[transition_m] || strcmp(lambda[transition_n], lambda[transition_m]) != 0)
            return 0;
    }
    return flag;
}

void depth_first_search(int state, char* label, char* output_label, FILE* file)
{
    char c; int vertex;
    int label_length = strlen(label), output_label_length = strlen(output_label);
    if(final[state])
    {
        char* new_output_label = malloc(output_label_length + strlen(psi[state]) + 1);
        strcpy(new_output_label, output_label);
        strcat(new_output_label, psi[state]);
        new_output_label[output_label_length + strlen(psi[state]) + 1] = '\0';
        fprintf(file, "%s %s\n", label, new_output_label);
    }

    for(c = 33; c <= 126; c++) {
        vertex = delta(state, c);
        if(vertex != -1)
        {
            char* new_label = malloc(label_length + 2);
            strcpy(new_label, label);
            new_label[label_length] = c;
            new_label[label_length + 1] = '\0';

            char* new_output_label = malloc(output_label_length + strlen(lambda_transition(state, c, vertex)) + 1);
            strcpy(new_output_label, output_label);
            strcat(new_output_label, lambda_transition(state, c, vertex));
            new_output_label[output_label_length + strlen(lambda_transition(state, c, vertex)) + 1] = '\0';
            depth_first_search(vertex, new_label, new_output_label, file);
            free(new_label);
        }
    }
}

void print_transducer()
{
    FILE *file = fopen("dict/transducer", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    depth_first_search(start, "", "", file);
    fclose(file);
}


void build_subseq_trans(char* filename)
{
    allocate_memory(); initialize_hash();
    read_dictionary(filename);

    int i, j, t, c, tau_len, alpha_len;
    char *alpha, *beta, *alpha_prim;
    char dummy[2];

    int tau[MAX_WORD_SIZE];
    int tau_prim[MAX_WORD_SIZE];
    int p[MAX_WORD_SIZE];

    char* output[MAX_WORD_SIZE];
    char* output_labels[MAX_WORD_SIZE];
    char path_label[MAX_WORD_SIZE];
    char prefix[MAX_WORD_SIZE];

    alpha = dictionary[0].first;
    beta = dictionary[0].second;
    alpha_len = strlen(alpha);

    for(t = 0; t <= alpha_len; t++)
        add_state(t);

    start = 0;
    final[alpha_len] = 1;
    psi[alpha_len] = "";

    for(t = 0; t < alpha_len; t++)
        add_transition(t, alpha[t], t + 1);

    add_output(0, alpha[0], beta);
    for(t = 1; t < alpha_len; t++)
        add_output(t, alpha[t], "");

    for ( j = 1; j < dictionary_size; j++ )
    {

        alpha = dictionary[j].first;
        beta = dictionary[j].second;
        alpha_prim = dictionary[j - 1].first;
        alpha_len = strlen(alpha);
        path(alpha, tau, &tau_len);

        reduce(alpha_prim, tau_len);

        // P = < |Q| + 1, ... , |Q| + |apha| - |tau| + 1 >
        for(i = 0; i <= alpha_len - tau_len ; i++)
        {
            p[i] = get_free_state_number();
            add_state(p[i]);
        }

        // tau' = tau . P
        for(i = 0; i < tau_len; i++)
            tau_prim[i] = tau[i];
        for(i = tau_len; i <= alpha_len; i++)
            tau_prim[i] = p[i - tau_len];

        for(i = 0; i < alpha_len; i++)
        {
            output_label(alpha, i, path_label);
            longest_common_prefix(path_label, beta, prefix);
            output[i] = prefix; // Л(i)
            output_labels[i] = path_label;
        }

        final[p[alpha_len - tau_len]] = 1;
        for(i = tau_len - 1; i < alpha_len; i++){
            add_transition(tau_prim[i], alpha[i], tau_prim[i+1]);
        }

        for(i = 0; i < tau_len - 1; i++){
            add_output(tau_prim[i], alpha[i], strrem(output[i], output[i + 1]));
            printf("adding output %d %c %s\n ", tau_prim[i], alpha[i], strrem(output[i], output[i + 1]));
        }
        printf("\n");
        printf("adding output %d %c %s \n", tau_prim[tau_len - 1], alpha[tau_len - 1], strrem(output[tau_len - 1], beta));
        add_output(tau_prim[tau_len - 1], alpha[tau_len - 1], strrem(output[tau_len - 1], beta));

        for(i = tau_len; i < alpha_len; i++){
            add_output(tau_prim[i], alpha[i], "");
            printf("adding output %d %c with epsilon", tau_prim[i], alpha[i]);
        }
        printf("\n");

        for(i = 0; i < tau_len - 1; i++)
            for(c = 33; c <= 126; c++)
            {
                if(c != alpha[i] && delta(tau_prim[i], c) != -1)
                {
                    dummy[0] = c; dummy[1] = '\0';
                    add_output(tau_prim[i], c, dummy);
                    printf("adding output %d %s", tau_prim[i], dummy);
                }
            }

        for(i = 0; i < tau_len; i++)
            if(final[tau[i]])
                psi[tau[i]] = strrem(output[i-1], strcat(output_labels[i], psi[tau[i]]));
        psi[p[alpha_len - tau_len]] = "";

    }
    reduce(alpha, 1);

    print_transducer();
    printf("NUMBER OF STATES %d \n", number_of_states);
    finalize_hash(); free_memory();
}
