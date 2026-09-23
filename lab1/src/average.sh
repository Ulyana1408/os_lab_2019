#!/bin/bash
count=$#
sum=0
for arg in "$@"
do
    sum=$((sum + arg))
done
average=$((sum / count))
echo "Количество: $count"
echo "Среднее арифметическое: $average"
