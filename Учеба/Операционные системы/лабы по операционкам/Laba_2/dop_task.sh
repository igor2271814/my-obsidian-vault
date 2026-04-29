#!/usr/bin/env bash

read -r -p "Введите адрес директории: " way
if [[ -z $way ]]; then
	echo "Вы не ввели адрес"
	exit 1
fi
if test -d "$way"; then
	echo "Директория $way существует, выполняется переход"
else
	echo "Ошибка: директории $way не существует"
	exit 2
fi
cd "$way" || exit
total_size=0
file_count=0

for file in * ; do
	if [ -f "$file"  ]; then
		size=$(stat -c %s "$file")
		total_size=$((total_size + size))
		file_count=$((file_count + 1))
	fi
done
if [ $file_count -gt 0 ]; then
	avg_size=$((total_size / file_count))
	echo "Файлов в директории: $file_count"
	echo "Общий размер файлов: $total_size байт"
	echo "Средний размер файлов: $avg_size байт"
else
	echo "Файлы не найдены"
fi
