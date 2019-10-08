#!/bin/bash


threads_1=(1 2 4 8 12)
iterations_1=(10 20 40 100 1000 10000 100000)

threads_2=(2 8)
iterations_2=(100 1000 10000 100000)

threads_3=(1)
iterations_3=(1 10 100 1000 10000 100000 1000000)

threads_4=(1 2 4 8 12)
iterations_4=(10 100 1000 10000)

threads_5=(1 2 4 8 12)

list_iterations_1=(10 100 1000 10000 20000)

list_threads_2=(2 4 8 12)
list_iterations_2_2=(1 2 4 8 16 32)

list_iterations_3=(32)

list_threads_4=(1 2 4 8 12 16 24)

echo "================Plot 1==================" >>lab2_add.csv
for i in "${threads_1[@]}"
do
	for j in "${iterations_1[@]}"
	do
		./lab2_add --threads=${i} --iterations=${j} >> lab2_add.csv
		./lab2_add --yield --threads=${i} --iterations=${j} >> lab2_add.csv
	done
done

echo "=====================Plot 2==================" >>lab2_add.csv
for i in "${threads_2[@]}"
do
	for j in "${iterations_2[@]}"
	do
		./lab2_add --yield --threads=${i} --iterations=${j} >> lab2_add.csv
		./lab2_add --threads=${i} --iterations=${j} >> lab2_add.csv
	done
done

echo "=======================Plot3=====================" >>lab2_add.csv
for i in "${threads_3[@]}"
do
	for j in "${iterations_3[@]}"
	do
		./lab2_add --threads=${i} --iterations=${j} >> lab2_add.csv
	done
done


echo "====================Plot 4========================" >>lab2_add.csv
for i in "${threads_4[@]}"
do
	for j in "${iterations_4[@]}"
        do
		./lab2_add --yield --threads=${i} --iterations=${j} >>lab2_add.csv
		./lab2_add --yield --threads=${i} --iterations=${j} --sync=m >>lab2_add.csv
		./lab2_add --yield --threads=${i} --iterations=${j} --sync=c >>lab2_add.csv
		if [ ${j} -ne 10000 ]
		then
			./lab2_add --yield --threads=${i} --iterations=${j} --sync=s >>lab2_add.csv
		fi
	done
done

echo "=====================Plot 5======================" >>lab2_add.csv
for i in "${threads_5[@]}"
do
	./lab2_add --threads=${i} --iterations=10000 >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=m >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=c >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=s >>lab2_add.csv
done

echo "======================Plot 1====================" >>lab2_list.csv
for i in "${list_iterations_1[@]}"
do
	./lab2_list --threads=1 --iterations=${i} >>lab2_list.csv
done

echo "==================Plot 2=======================" >>lab2_list.csv
for i in "${list_threads_2[@]}"
do
	for k in "${list_iterations_2_2[@]}"
	do
		./lab2_list --threads=${i} --iterations=${k} >>lab2_list.csv
		./lab2_list --yield=i --threads=${i} --iterations=${k} >>lab2_list.csv
		./lab2_list --yield=d --threads=${i} --iterations=${k} >>lab2_list.csv
		./lab2_list --yield=il --threads=${i} --iterations=${k} >>lab2_list.csv
		./lab2_list --yield=dl --threads=${i} --iterations=${k} >>lab2_list.csv
	done
done

echo "=========================Plot 3=====================" >>lab2_list.csv
for i in "${list_iterations_3[@]}"
do
	./lab2_list --yield=i --threads=12 --iterations=${i} --sync=m >>lab2_list.csv
	./lab2_list --yield=i --threads=12 --iterations=${i} --sync=s >>lab2_list.csv
	
	./lab2_list --yield=d --threads=12 --iterations=${i} --sync=m >>lab2_list.csv
	./lab2_list --yield=d --threads=12 --iterations=${i} --sync=s >>lab2_list.csv

	./lab2_list --yield=il --threads=12 --iterations=${i} --sync=m >>lab2_list.csv
	./lab2_list --yield=il --threads=12 --iterations=${i} --sync=s >>lab2_list.csv

	./lab2_list --yield=dl --threads=12 --iterations=${i} --sync=m >>lab2_list.csv
	./lab2_list --yield=dl --threads=12 --iterations=${i} --sync=s >>lab2_list.csv
	
	./lab2_list --yield=idl --threads=12 --iterations=${i} --sync=m >>lab2_list.csv
	./lab2_list --yield=idl --threads=12 --iterations=${i} --sync=s >>lab2_list.csv
done

echo "==========================Plot 4=========================" >>lab2_list.csv
for i in "${list_threads_4[@]}"
do
	./lab2_list --threads=${i} --iterations=1000 >>lab2_list.csv
	./lab2_list --threads=${i} --iterations=1000 --sync=m >>lab2_list.csv
	./lab2_list --threads=${i} --iterations=1000 --sync=s >>lab2_list.csv
done
