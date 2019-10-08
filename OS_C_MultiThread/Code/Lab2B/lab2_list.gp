#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. (optional) average wait time for a lock (ns)
#
# output:
#	lab2b_1.png ... throughput vs number of threads for mutex and spin-lock list operations.
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#	lab2b_3.png ... successful iterations vs threads for each synchronization method.
#	lab2b_4.png ... throughput vs number of threads for mutexes with partitioned lists.
#	lab2b_5.png ... throughput vs number of threads for spin-locks with partitioned lists.


# general plot parameters
set terminal png
set datafile separator ","

#  the total number of operations per second for each synchronization method
set title "List_2B-1: Cost per Operation vs Threads for mutex and spin-lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-s,[0-9]+,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spinlock' with linespoints lc rgb 'blue'


set title "List_2B-2: Time vs Number of Threads for mutex"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
set key left top

plot \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,16,1000,1,|list-none-m,24,1000,1,' lab2_2b_list.csv" using ($2):($7) \
     	title 'completion time' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,16,1000,1,|list-none-m,24,1000,1,' lab2_2b_list.csv" using ($2):($8) \
     	title 'avg wait-for-lock time' with linespoints lc rgb 'blue'


set title "List_2B-3: Iterations that run without failure"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top

plot \
    "< grep -E 'list-id-none,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "red" title "Unprotected", \
    "< grep -E 'list-id-m,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "blue" title "Mutex", \
   "< grep -E 'list-id-s,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "orange" title "Spinlock"


set title "List_2B-4: Throughput of Sublists For Mutex"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9],1000,4,|list-none-m,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-m,[0-9],1000,8,|list-none-m,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -E 'list-none-m,[0-9],1000,16,|list-none-m,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'orange'

set title "lab2b_5: Throughput of Sublist for SpinLock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep -E 'list-none-s,[0-9],1000,1,|list-none-s,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-s,[0-9],1000,4,|list-none-s,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-s,[0-9],1000,8,|list-none-s,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -E 'list-none-s,[0-9],1000,16,|list-none-s,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'orange'

