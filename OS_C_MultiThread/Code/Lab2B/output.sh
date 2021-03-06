#!/bin/bash

echo "==========================Plot 1==================== " >>lab2_list.csv
for i in 1 2 4 8 12 16 24
do
	./lab2_list --threads=${i} --iterations=1000 --sync=m >>lab2_list.csv
	./lab2_list --threads=${i} --iterations=1000 --sync=s >>lab2_list.csv
done



echo "==========================Plot 2==================== " >>lab2_2b_list.csv
for i in 1 2 4 8 16 24
do
	./lab2_list --threads=${i} --iterations=1000 --sync=m >>lab2_2b_list.csv
done


echo "=======================Plot 3================================" >>lab2b_list.csv
for i in 1 4 8 12 16
do
	for j in 1 2 4 8 16
	do
		./lab2_list --threads=${i} --iterations=${j} --yield=id --lists=4 >>lab2b_list.csv
	done
	for j in 10 20 40 80
	do
		./lab2_list --threads=${i} --iterations=${j} --yield=id --lists=4 --sync=m >>lab2b_list.csv
		./lab2_list --threads=${i} --iterations=${j} --yield=id --lists=4 --sync=s >>lab2b_list.csv
	done
done

echo "===================== Plot 4 and 5================================" >>lab2b_list.csv
for i in 1 2 4 8 12
do
	for k in 1 4 8 16
	do
		./lab2_list --threads=${i} --iterations=1000 --lists=${k} --sync=m >>lab2b_list.csv
		./lab2_list --threads=${i} --iterations=1000 --lists=${k} --sync=s >>lab2b_list.csv
	done
done

