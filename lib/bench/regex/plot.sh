dat_file="./output/output.dat"
printf "Regex\t$alg\n" > $dat_file

for alg in $@
do
	make bin ALG=$alg > /dev/null 2>&1

	printf "benching ALGORITHM: $alg\n"
	res=$(./$alg input/web)
	printf "$alg\t$res\n" >> $dat_file
done

# gnuplot -e "ipv=$IPv; strides='$STRIDES'" plot.plt
