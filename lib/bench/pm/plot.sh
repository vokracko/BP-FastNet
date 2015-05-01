make bin  > /dev/null 2>&1
dat_file="./output/output.dat"
printf "Algorithm Aho-Corasick: $input\n" > $dat_file

for input in `ls input`;
do
	printf "benching AC: $input\n"
	res=$(./ac all input/"$input")
	printf "$input\t$res\n" >> $dat_file
done
# gnuplot -e "input=$input" plot.plt
