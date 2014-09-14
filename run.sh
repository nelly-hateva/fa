#!/bin/bash


gcc -Wall *.c -o cmt
time ./cmt $1 $2 > $3
