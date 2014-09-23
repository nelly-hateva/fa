#!/bin/bash


gcc -Wall ./src/*.c -o ./bin/cmt
time ./bin/cmt $@
