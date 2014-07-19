#include <stdlib.h>
#include <string.h>

#include "pair.h"
#include "hash.h"

#define INITIAL_STATES_NUMBER 2048
#define INITIAL_TRANSITIONS_NUMBER 2048
#define GARBAGE_SIZE 1000

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


void initialize()
{
    number_of_states = 0; number_of_transitions = 0;
    memory_for_states = INITIAL_STATES_NUMBER; memory_for_transitions = INITIAL_TRANSITIONS_NUMBER;

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
    }

    for(i = 0; i < GARBAGE_SIZE; i++)
    {
        garbage_states[i] = -1;
        garbage_transitions[i] = -1;
    }

}


void reallocate_memory()
{
    if(number_of_states >= memory_for_states)
    {

        int temp = memory_for_states;
        memory_for_states *= 2;
        first_transition = (int*) realloc(first_transition, memory_for_states * sizeof(int));
        final = (char*) realloc(final, memory_for_states * sizeof(char));

        int i;
        for(i = temp; i < memory_for_states; i++)
        {
            first_transition[i] = -1;
            final[i] = -1;
        }
    }
    else if (number_of_transitions >= memory_for_transitions)
    {
        int temp = memory_for_transitions;
        memory_for_transitions *= 2;
        next_transition = (int*) realloc(next_transition, memory_for_transitions * sizeof(int));
        from = (int*) realloc(from, memory_for_transitions * sizeof(int));
        to = (int*) realloc(to, memory_for_transitions * sizeof(int));
        label = (char*) realloc(label, memory_for_transitions * sizeof(char));
        psi = (char**) realloc(psi, memory_for_transitions * sizeof(char*));
        lambda = (char**) realloc(lambda, memory_for_transitions * sizeof(char*));

        int i;
        for(i = temp; i < memory_for_transitions; i++)
        {
            next_transition[i] = -1;
            from[i] = -1;
            to[i] = -1;
            label[i] = -1;
        }
    }
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
    reallocate_memory();
    if(final[n] == -1)
    {
        final[n] = 0;
        number_of_states++;
    }
}


void add_transition(int from_state, char label_transition, int to_state)
{
    add_state(from_state);
    add_state(to_state);
    int position;
    if(garbage_transition == -1)
        position = number_of_transitions;
    else
    {
        position = garbage_transitions[garbage_transition];
        garbage_transition--;
    }

    reallocate_memory();
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
    }
}


int* path(char* alpha, int* tau_len)
{
    int alpha_len = strlen(alpha);
    int* result = malloc(alpha_len * sizeof(int));
    int result_len = 0;
    int letter; int transition; int state = start;

    int i;
    for(i = 0; i < alpha_len; i++)
    {
        letter = alpha[i];
        transition = first_transition[state];
        while( label[transition] != -1 && transition != -1)
        {
            if ( label[transition] == letter )
            {
                result[result_len] = transition;
                result_len++;
                state = to[transition];
                break;
            }
            transition = next_transition[transition];
        }
    }
    *tau_len = result_len;
    return result;
}


int find_equivalent(int state)
{
    return search(state, h(state));
}


void reduce(char* alpha, int* tau, int tau_len, int l)
{
    int i; int p;
    for(i = 1; tau_len - i == l; i++ )
    {
        p = tau_len - i - 1;
        int tp = tau[p];
        int state = find_equivalent(tp);
        if(state != -1)
        {
            delete_state(tp);
            delete_transition_by_signature(tau[p-1], alpha[p-1]);
        }
        else
            insert(tp, h(tp));
    }
}


int h(int n)
{
    int hash_code;
    int first = first_transition[n];
    if (first == -1) hash_code = 1;
    else { hash_code = ((label[first] * 257) % hash_length) + to[first]; first = next_transition[first]; }
    while(first != -1)
    {
        hash_code = ((hash_code * 257) % hash_length) + label[first];
        hash_code = ((hash_code * 257) % hash_length) + to[first];
        first  = next_transition[first];
    }
    return (hash_code + final[n]) % hash_length;
}


char equal(int n, int m)
{
    char flag = 0;
    if(final[n] == final[m])
        flag = 1;
    if(flag)
    {
        int transition_n = first_transition[n], transition_m = first_transition[m];
        while(label[transition_n] == label[transition_m] && next_transition[transition_n] != -1 && next_transition[transition_m] != -1)
        {
            transition_n = next_transition[transition_n];
            transition_m = next_transition[transition_m];
        }
        if(next_transition[transition_m] != next_transition[transition_n] || label[transition_n] != label[transition_m])
            return 0;
    }
    return flag;
}


void delete_word(char* alpha)
{
    int tau_len;
    int* tau = path(alpha, &tau_len);
    int i;
    for(i = tau_len - 1; i >= 0; i--)
        if(next_transition[first_transition[tau[i]]] != -1)
            delete_state(tau[i]);
}


void build_subseq_trans(pair* dict, int dictionary_size)
{
    initialize(); initialize_hash();
    int j, t;

    for ( j = 0; j < dictionary_size; j++)
    {
        if(j == 0)
        {
            char* alpha = dict[j].first;
            //char* beta = dict[j].second;
            start = 0;
            //lambda[start] = beta;
            for(t = 0; t < strlen(alpha); t++)
            {
                //lambda[t] = '\0';
                add_state(t);
            }
            final[strlen(alpha)] = 1;
            psi[strlen(alpha)] = '\0';
        }

        else
        {
            char* alpha = dict[j].first;
            // char* beta = dict[j].second;
            char* alpha_prim = dict[j - 1].first;
            // char* beta_prim = dict[j - 1].second;

            char alpha_len = strlen(alpha);
            int tau_len;
            int i;

            int* tau = path(alpha, &tau_len);

            reduce(alpha_prim, tau, tau_len, strlen(alpha_prim));

            int* p = malloc((alpha_len - tau_len) * sizeof(int));
            for(i = 0; i < alpha_len - tau_len ; i++)
                p[i] = get_free_state_number();

            int* tau_prim = malloc((alpha_len + 1) * sizeof(int));
            for(i = 0; i < tau_len; i++)
                tau_prim[i] = tau[i];
            for(i = tau_len; i < alpha_len; i++)
                tau_prim[i] = p[i - tau_len];

            for(i = 0; i < alpha_len - tau_len; i++)
                add_state(p[i]);
            final[alpha_len - tau_len - 1] = 1;
            //for(i = tau_len; i < alpha_len; i++)
             //   add_transition(tau_prim[i], alpha[i], tau_prim[i+1]);
            free(p); free(tau_prim);
        }
    }
}
