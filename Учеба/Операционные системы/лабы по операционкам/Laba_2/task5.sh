#!/usr/bin/env bash

echo "Загрузка..."
sleep 1
echo "Загрузка..."
sleep 1
echo "Загрузка..."
sleep 1
echo "Мини-калькулятор"

while true; do
	read -r -p "Введите первое число: " num1
	if [[ -z $num1 || ! $num1 =~ ^-?[0-9]+([.][0-9]+)?$ ]]; then
		echo "Ошибка: Вы не ввели первое число или это не число"
	else
		break
	fi
done
while true; do
	read -r -p "ВВедите второе число: " num2
	if [[ -z $num2 || ! $num2 =~ ^-?[0-9]+([.][0-9]+)?$ ]]; then
		echo "Ошибка: Вы не ввели второе число или это не число"
	else
		break
	fi
done
echo -e "Выберите арифметическую операцию: \n1. +\n2. -\n3. *\n4. /"

read -r -p "Ваш выбор: " answer

case "$answer" in
	"+" | "1")
		echo "Ответ: $(echo "$num1 + $num2" | bc -l)"
	;;
	"-" | "2")
		echo "Ответ: $(echo "$num1 - $num2" | bc -l)"
	;;
	"*" | "3")
		echo "Ответ: $(echo "$num1 * $num2" | bc -l)"
	;;
	"/" | "4")
		if [[ $num2 == 0 ]]; then
			echo "На ноль делить нельзя"
		else
			echo "Ответ: $(echo "$num1 / $num2" | bc -l)"
		fi
	;;
	*) 
		echo "Такой операции нет"
	;;
esac
