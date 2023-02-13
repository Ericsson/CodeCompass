#!/bin/bash

FILES_JS=$(ls *.js)
FILES_TS=$(ls *.ts)

for FILE in $FILES_JS
do
	if [ -f "$FILE" ]
	then
		sed -i -e "/if (typeof Int64 === 'undefined' && typeof require === 'function') {/,+2 d" "$FILE"
	fi
done

for FILE in $FILES_TS
do
	if [ -f "$FILE" ]
	then
		sed -i -e "s/import Int64 = require(\"node-int64\");/import Int64 from 'node-int64';/g" "$FILE"
	fi
done