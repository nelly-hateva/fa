#include <stdlib.h>

#define INITIAL_HASH_LENGTH 4096
#define INITIAL_HASH_CAPACITY 8192

extern int hash_code(int);
extern char equivalent(int n, int m);

int* table;
int* next;

int hash_size;
int hash_length;
int hash_capacity;

void insert(int, int);


void initialize_hash()
{
    hash_size = 0;
    hash_length = INITIAL_HASH_LENGTH;
    hash_capacity = INITIAL_HASH_CAPACITY;

    table = (int*) malloc(hash_length * sizeof(int));
    next = (int*) malloc(hash_capacity * sizeof(int));

    int i;
    for (i = 0; i < hash_length; ++i)
        table[i] = -1;
    for (i = 0; i < hash_capacity; ++i)
        next[i] = -1;
}


void free_hash()
{
    free(table);
    free(next);
}


void resize_hash()
{
    int element, i, j = 0;
    int *bucket = (int*) malloc(hash_size * sizeof(int));
    int previous_value_hash_size = hash_size;

    for (i = 0; i < hash_length; ++i)
    {
        element = table[i];
        while (element != -1)
        {
            bucket[j] = element;
            ++j;
            element = next[element];
        }
    }

    hash_capacity *= 2;
    hash_length *= 2;
    table = (int*) realloc(table, hash_length * sizeof(int));
    next = (int*) realloc(next, hash_capacity * sizeof(int));

    for (i = 0; i < hash_length; ++i)
        table[i] = -1;
    for (i = 0; i < hash_capacity; ++i)
        next[i] = -1;

    hash_size = 0;
    for (i = 0; i < previous_value_hash_size; ++i)
        insert(bucket[i], hash_code(bucket[i]));
    free(bucket);
}


void delete(int value, int key)
{
    int j = table[key];
    int prev = j;

    if (j == value)
        table[key] = next[j];
    while (j!= -1)
    {
        if (j == value)
        {
            next[prev] = next[j];
            next[j] = -1;
            --hash_size;
            return;
        }
        prev = j;
        j = next[j];
    }
}


int find(int value, int key)
{
    int j = table[key];

    while (j != -1)
    {
        if (j == value)
            return 1;
        j = next[j];
    }

    return -1;
}


void insert(int value, int key)
{
    if (value >= hash_capacity)
        resize_hash();

    if (find(value, key) == -1)
    {
        next[value] = table[key];
        table[key] = value;
        hash_size++;
    }
}


int search(int value, int key)
{
    int j = table[key];

    while (j != -1)
    {
        if (j != value && equivalent(value, j))
            return j;
        j = next[j];
    }

    return -1;
}
