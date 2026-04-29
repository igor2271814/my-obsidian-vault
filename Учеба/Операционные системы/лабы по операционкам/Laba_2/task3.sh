#!/usr/bin/env bash

if [ $# -eq 0  ]; then
	echo "Ошибка: вы не передали имя файла"
	echo "Использование: $0 <имя_файла>"
	exit 1
fi

if [ -f "$1" ]; then
	echo "Файл '$1'  существует"

	if [ -r "$1"  ]; then
		echo "Файл '$1' доступен для чтения"
	else
		echo "Но у вас нет прав на его чтение"
	fi
else
	echo "Файл $1 не найден"
	read -r -p "Создать файл с таким имененм? (Y/N y/n)" answer
	
	if [[ $answer =~ ^[Yy]  ]]; then
		touch "$1"
		echo "Файл '$1' успешно создан!"
	else
		echo "Создание отменено"
		exit 0
	fi
fi
