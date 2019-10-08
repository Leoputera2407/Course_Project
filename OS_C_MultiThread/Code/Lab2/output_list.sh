#!/bin/bash

list_iterations_1=(10 100 1000 10000 20000)

list_iterations_3=(32)

list_threads_4=(1 2 4 8 12 16 24)


echo "======================Plot 1====================" >>lab2_list.csv
for i in "${list_iterations_1[@]}"
do
	./lab2_list --threads=1 --iterations=${i} >>lab2_list.csv
done

echo "==================Plot 2=======================" >>lab2_list.csv
for i in 1 10 100 1000
do 
    for j in 2 4 8 12
    do
        ./lab2_list --iterations=$i --threads=$j >>lab2_list.csv
    done
done

for i in 1 2 4 8 16 32
do 
    for j in 2 4 8 12
    do 
        for k in i d il dl
        do
            ./lab2_list --iterations=$i --threads=$j --yield=$k >>lab2_list.csv
        done
    done
done
echo "=========================Plot 3=====================" >>lab2_list.csv
for i in i d il dl
do
	for j in m s
	do
		./lab2_list --yield=${i} --threads=12 --iterations=32 --sync=${j} >>lab2_list.csv
	done
done

echo "==========================Plot 4=========================" >>lab2_list.csv
for i in "${list_threads_4[@]}"
do
	./lab2_list --threads=${i} --iterations=1000 --sync=m >>lab2_list.csv
	./lab2_list --threads=${i} --iterations=1000 --sync=s >>lab2_list.csv
done
