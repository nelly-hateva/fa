#!/bin/bash


gcc -Wall ./src/*.c -o ./bin/cmt
time ./bin/cmt $1 $2
diff $1 $2
