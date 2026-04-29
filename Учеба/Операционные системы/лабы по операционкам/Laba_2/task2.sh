#!/usr/bin/env bash

if [ $# -ne 1  ]; then
	echo "Ошибка: требуется ровно 1 аргумент"
	exit 0
fi

if [[ ! "$1" =~ ^[0-9]+$ ]]; then
    echo "Ошибка: $1 не является положительным целым числом"
    exit 1
fi

if [ "$1" -lt 1  ]; then
	echo "Ошибка: число не может быть равно 0"
	exit 1
fi

for ((i=1; i<="$1"; i++));  do
	
if ((i % 2 > 0)); then
		echo "Число - $i"
	fi
done
