#!/usr/bin/env bash

echo -e "Эта программа может вывести следующие зарезервированные переменные:\n1. HOME\n2. PWD\n3. PATH\n4. USER\n5. HOSTNAME\n6. RANDOM\n7. Выход из приложения (exit)"

while true; do
	while true; do
		read -r -p "Введите имя переменной без $ для её вывода: " answer
		if [ -z "$answer" ]; then
			echo "Ошибка: Вы не передали имя переменной"
		else
			break
		fi

	done 
	case "$answer" in
		"HOME" | "home")
			echo "Домашняя директория - $HOME"
		;;
		"PWD" | "pwd")
			echo "Текущий рабочий каталог - $PWD"
		;;
		"PATH" | "path")
			ehco "Список каталогов, где Bash хранит исполняемые файлы:"
			echo "$PATH"
		;;
		"USER" | "user") 
			echo "Имя текущего юзера - $USER"
		;;
		"HOSTNAME" | "hostname")
			echo "Имя ПК - $HOSTNAME"
		;;
		"RANDOM" | "random")
			echo "Случайное число от 0 до 32767 - $RANDOM"
		;;
		"EXIT" | "exit")
			echo "Выход из приложения"
			break
		;;
		*)
			echo "Данная переменная не поддерживается"
		;;
	esac

	read -r -p "Хотите выбрать другую переменную? (Yy /Nn)" choice
	if [[ $choice =~ ^[Yy] ]]; then
		echo "Перезапуск приложения"
	else
		echo "Выход из приложения"
		break
	fi

done
