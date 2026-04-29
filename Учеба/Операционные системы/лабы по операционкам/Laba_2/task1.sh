#!/usr/bin/env bash

echo "Аргументов принято - $#"
if (($# < 2)); then
	echo "Ошибка: минимум 2 аргумента"
	exit 1
fi

if (($# > 5)); then
	echo "Ошибка: максимум 5 аргументов"
	exit 2
fi

for arg in "$@"; do
	if [[ ! "$arg" =~ ^-?[0-9]+$ ]]; then
		echo "Ошибка: '$arg' не является числом"
		exit 3
	fi
done

sum=0
for arg in "$@"; do
	(( sum += arg ))
done
echo "Сумма: $sum"
exit 0


