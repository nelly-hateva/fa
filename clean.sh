#!/bin/bash

SOURCE_FILE=dict/dict
DESTINATION_FILE=dict/data

sed -e "/^;;;/d" -e "s/ \{2,2\}/@/g" -e "s/ //g" -e "s/@/ /g" < $SOURCE_FILE > $DESTINATION_FILE

# example line : JOINING  JH OY1 N IH0 NG
# "/^;;;/d" remove all lines beginning with ;;; e.g. comments
#"s/ \{2,2\}/@/g" replace two spaces (separator) with @
# "s/ //g" remove all spaces
# "s/@/ /g" replace @ with space
# result line : JOINING  JHOY1NIH0NG
