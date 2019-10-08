#!/bin/bash


threads_1=(1 2 4 8 12)
iterations_1=(10 20 40 100 1000 10000 100000)

threads_2=(2 8)
iterations_2=(100 1000 10000 100000)

threads_3=(1)
iterations_3=(1 10 100 1000 10000 100000 1000000)

threads_4=( 2 4 8 10 12)

threads_5=(1 2 4 8 12)
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
		./lab2_add --yield --threads=${i} --iterations=10000 --sync=m >>lab2_add.csv
		./lab2_add --yield --threads=${i} --iterations=10000 --sync=c >>lab2_add.csv
		
		./lab2_add --yield --threads=${i} --iterations=1000 --sync=s >>lab2_add.csv
		
done

echo "=====================Plot 5======================" >>lab2_add.csv
for i in "${threads_5[@]}"
do
	./lab2_add --threads=${i} --iterations=10000 >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=m >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=c >>lab2_add.csv
	./lab2_add --threads=${i} --iterations=10000 --sync=s >>lab2_add.csv
done
