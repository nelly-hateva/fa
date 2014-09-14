#!/bin/bash


# example line in cmu_dict : JOINING  JH OY1 N IH0 NG
# "/^;;;/d" remove all lines beginning with ;;; e.g. comments
#"s/ \{2,2\}/@/g" replace two spaces (separator) with @
# "s/ //g" remove all spaces
# "s/@/ /g" replace @ with space
# result line : JOINING  JHOY1NIH0NG
sed -e "/^;;;/d" -e "s/ \{2,2\}/@/g" -e "s/ //g" -e "s/@/ /g" < data/cmu_dict > data/dict1

# example line in rwb2c.txt : aaaratd,aaarated,aaarated
# result line : aaaratd aaarated

sed -e 's/\([^,]*,[^,]*\),[^,]*/\1/' data/rwb2c.txt | tr ',' ' ' > data/dict2

# merge the two dictionaries in one
cat data/dict1 > data/merged
cat data/dict2 >> data/merged

LANG=C sort data/merged -o data/dict3
rm -f data/merged

# to lower case
tr '[:upper:]' '[:lower:]' < data/dict3  | LC_ALL=C sort -o data/dict4
# OR sed -e 's/\(.*\)/\L\1/' data/dict3 | LC_ALL=C sort -o data/dict4

# check for duplicates
# sed 's/ .*//' data/dict4 | uniq | wc
# cat data/dict4 | wc

# to upper case
tr '[:lower:]' '[:upper:]' < data/dict4 | LANG=C sort -o data/dict5
# OR sed -e 's/\(.*\)/\U\1/' data/dict3 | LC_ALL=C sort -o data/dict5
