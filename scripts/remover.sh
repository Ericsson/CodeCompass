#!/bin/bash

FILES=$(ls ./*.js)

for FILE in $FILES
do
	if [ -f "$FILE" ]
	then
		sed -i -e "/if (typeof Int64 === 'undefined' && typeof require === 'function') {/,+2 d" "$FILE"
	fi
done
