if [ $# -ne 4 ]; then
	printf "Run with 4 parameters: ip version, algorithms, ranges and strides\n"
	exit 1
fi

IPv=$1
DEFAULT_RULE=1
ALGS=$2
SIZES=$3
STRIDES=$4

for alg in $ALGS
do
	if [ $alg = "bspl" ]; then
		STRIDES=1;
	else
		STRIDES=$4
	fi

	for stride in $STRIDES
	do

		make bin ALG=$alg STRIDE=$stride IP=$IPv > /dev/null 2>&1
		dat_file="./output/IPv$IPv/$alg-$stride.dat"
		printf "Algorithm\t$alg-$stride\n" > $dat_file

		for size in $SIZES
		do
			printf "benching ALGORITHM: $alg, STRIDE: $stride, SIZE: $size\n"
			res=$(./$alg -v$IPv $DEFAULT_RULE $size)
			printf "$size\t$res\n" >> $dat_file
		done

	done

done

which gnuplot > /dev/null 2>&1
if [ $? -eq 0 ]; then
	gnuplot -e "ipv=$IPv; strides='$STRIDES'" plot.plt
fi
