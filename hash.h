#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

extern int hash_length;

void initialize_hash();
void resize_hash();
void insert(int value, int key);
void delete(int value, int key);
char search(int value, int key);

#endif // HASH_H_INCLUDED
