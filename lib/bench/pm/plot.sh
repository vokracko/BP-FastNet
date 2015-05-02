make bin  > /dev/null 2>&1
dat_file="./output/output.dat"
printf "Aho-corasick\n" > $dat_file

for input in `ls input`;
do
	printf "benching AC: $input\n"
	res=$(./ac all input/"$input")
	printf "$input\t$res\n" >> $dat_file
done
gnuplot plot.plt
