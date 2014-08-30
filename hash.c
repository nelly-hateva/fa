#include <stdlib.h>

#define INITIAL_HASH_LENGTH 1024
#define INITIAL_HASH_CAPACITY 2048

extern int h(int);
extern char equal(int n, int m);

int* table;
int* next;

int hash_size;
int hash_length;
int capacity;

void insert(int, int);


void initialize_hash()
{
    hash_size = 0;
    hash_length = INITIAL_HASH_LENGTH;
    capacity = INITIAL_HASH_CAPACITY;

    table = (int*) malloc(hash_length * sizeof(int));
    next = (int*) malloc(capacity * sizeof(int));

    int i;
    for( i = 0 ; i < hash_length; i++ )
        table[i] = -1;
    for( i = 0; i < capacity; i++ )
        next[i] = -1;
}


void finalize_hash()
{
    printf("HASH SIZE %d \n", hash_size);
    free(table);
    free(next);
}


void resize_hash()
{
    int *temp = (int*) malloc(hash_size * sizeof(int));
    int element, i, j = 0;

    for( i = 0 ; i < hash_length; i++ )
    {
        element = table[i];
        while(element != -1)
        {
            temp[j] = element;
            ++j;
            element = next[element];
        }
    }

    capacity *= 2;
    hash_length *= 2;
    table = (int*) realloc(table, hash_length * sizeof(int));
    next = (int*) realloc(next, capacity * sizeof(int));

    for( i = 0 ; i < hash_length; i++ )
        table[i] = -1;
    for( i = 0; i < capacity; i++ )
        next[i] = -1;

    int temp_hash_size = hash_size;
    hash_size = 0;
    for( i = 0; i < temp_hash_size; i++ )
        insert(temp[i], h(temp[i]));
    free(temp);
}


void delete(int value, int key)
{
    int j = table[key];
    int prev = j;
    if (j == value)
        table[key] = next[j];
    while(j!= -1)
    {
        if(j == value)
        {
            next[prev] = next[j];
            next[j] = -1;
            hash_size--;
            return;
        }
        prev = j;
        j = next[j];
    }
}


int find(int value, int key)
{
    int j = table[key];
    while(j != -1)
    {
        if(j  ==  value)
            return 1;
        j = next[j];
    }
    return -1;
}


void insert(int value, int key)
{
    if( value >= capacity )
        resize_hash();

    if( find(value, key) == -1 )
    {
        next[value] = table[key];
        table[key] = value;
        hash_size++;
    }
}


int search(int value, int key)
{
    int j = table[key];
    while(j!= -1)
    {
        if(j != value && equal(value, j))
        {
            if(j < 0)
            {
                printf("%d \n",  j);
                exit(EXIT_FAILURE);
            }
            return j;
        }
        j = next[j];
    }
    return -1;
}
