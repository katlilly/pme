#!/bin/bash

#for i in $(less lists_longer_than_10000.txt)
for i in {1..499690}
do
	 echo $i
	 ./get_optimal_packing_data $i > selectors$i.txt
	 less selectors$i.txt | wc -l
	 uniq selectors$i.txt | wc -l
	 rm selectors$i.txt
done
