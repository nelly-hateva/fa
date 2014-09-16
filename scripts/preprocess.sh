#!/bin/bash


gcc -Wall ../src/lib/clean.c -o ../bin/clean

# example line in cmu_dict : JOINING  JH OY1 N IH0 NG
# "/^;;;/d" remove all lines beginning with ;;; e.g. comments
#"s/ \{2,2\}/@/g" replace two spaces (separator) with @
# "s/ //g" remove all spaces
# "s/@/ /g" replace @ with space
# result line : JOINING  JHOY1NIH0NG

sed -e "/^;;;/d" -e "s/ \{2,2\}/@/g" -e "s/ //g" -e "s/@/ /g" < ../src/test/data/cmu_dict > ../src/test/data/dict1
LANG=C sort ../src/test/data/dict1 -o ../src/test/data/dict1
../bin/clean ../src/test/data/dict1 ../src/test/data/temp
mv ../src/test/data/temp ../src/test/data/dict1

# example line in rwb2c.txt : aaaratd,aaarated,aaarated
# result line : aaaratd aaarated

sed -e 's/\([^,]*,[^,]*\),[^,]*/\1/' ../src/test/data/rwb2c.txt | tr ',' ' ' > ../src/test/data/dict2
../bin/clean ../src/test/data/dict2 ../src/test/data/temp
mv ../src/test/data/temp ../src/test/data/dict2

# merge the two dictionaries in one
cat ../src/test/data/dict1 > ../src/test/data/dict3
cat ../src/test/data/dict2 >> ../src/test/data/dict3

LANG=C sort ../src/test/data/dict3 -o ../src/test/data/dict3
../bin/clean ../src/test/data/dict3 ../src/test/data/temp
mv ../src/test/data/temp ../src/test/data/dict3

# to lower case
tr '[:upper:]' '[:lower:]' < ../src/test/data/dict3  | LC_ALL=C sort -o ../src/test/data/dict4
# OR sed -e 's/\(.*\)/\L\1/' ../src/test/data/dict3 | LC_ALL=C sort -o ../src/test/data/dict4
../bin/clean ../src/test/data/dict4 ../src/test/data/temp
mv ../src/test/data/temp ../src/test/data/dict4

# to upper case
tr '[:lower:]' '[:upper:]' < ../src/test/data/dict4 | LANG=C sort -o ../src/test/data/dict5
# OR sed -e 's/\(.*\)/\U\1/' ../src/test/data/dict3 | LC_ALL=C sort -o ../src/test/data/dict5
../bin/clean ../src/test/data/dict4 ../src/test/data/temp
mv ../src/test/data/temp ../src/test/data/dict4

# check for duplicates
# sed 's/ .*//' ../src/test/data/dict4 | uniq | wc
# cat ../src/test/data/dict4 | wc
