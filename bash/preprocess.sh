#!/bin/bash


gcc -Wall ../src/lib/clean.c -o ../bin/clean

# example line in cmu_dict : JOINING  JH OY1 N IH0 NG
# "/^;;;/d" remove all lines beginning with ;;; e.g. comments
#"s/ \{2,2\}/@/g" replace two spaces (separator) with @
# "s/ //g" remove all spaces
# "s/@/ /g" replace @ with space
# result line : JOINING  JHOY1NIH0NG

sed -e "/^;;;/d" -e "s/ \{2,2\}/@/g" -e "s/ //g" -e "s/@/ /g" < ../data/cmu_dict > ../data/dict1
tr '[:upper:]' '[:lower:]' < ../data/dict1  | LC_ALL=C sort -o ../data/dict1
../bin/clean ../data/dict1 ../data/temp
mv ../data/temp ../data/dict1

# example line in rwb2c.txt : aaaratd,aaarated,aaarated
# result line : aaaratd aaarated

sed -e 's/\([^,]*,[^,]*\),[^,]*/\1/' ../data/rwb2c.txt | tr ',' ' ' > ../data/dict2
tr '[:upper:]' '[:lower:]' < ../data/dict2  | LC_ALL=C sort -o ../data/dict2
../bin/clean ../data/dict2 ../data/temp
mv ../data/temp ../data/dict2

tr '\t' ' ' < ../data/geo3b.txt > ../data/dict3
tr '[:upper:]' '[:lower:]' < ../data/dict3  | LC_ALL=C sort -o ../data/dict3
../bin/clean ../data/dict3 ../data/temp
mv ../data/temp ../data/dict3

cat ../data/dict1 > ../data/dict4
cat ../data/dict2 >> ../data/dict4
LANG=C sort ../data/dict4 -o ../data/dict4
../bin/clean ../data/dict4 ../data/temp
mv ../data/temp ../data/dict4

cat ../data/dict1 > ../data/dict5
cat ../data/dict3 >> ../data/dict5
LANG=C sort ../data/dict5 -o ../data/dict5
../bin/clean ../data/dict5 ../data/temp
mv ../data/temp ../data/dict5

cat ../data/dict2 > ../data/dict6
cat ../data/dict3 >> ../data/dict6
LANG=C sort ../data/dict6 -o ../data/dict6
../bin/clean ../data/dict6 ../data/temp
mv ../data/temp ../data/dict6

cat ../data/dict1 > ../data/dict7
cat ../data/dict2 >> ../data/dict7
cat ../data/dict3 >> ../data/dict7
LANG=C sort ../data/dict7 -o ../data/dict7
../bin/clean ../data/dict7 ../data/temp
mv ../data/temp ../data/dict7
