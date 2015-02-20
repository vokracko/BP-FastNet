if [ $# -ne 1 ]; then
	printf "Run with parameter 4/6\n"
	exit 1
fi

IPv=$1
DEFAULT_RULE=1
ALGS="bspl tbm"
SIZES="1k 10k 100k"

printf "Algorithm"
for alg in $ALGS
do
	printf "\t$alg"
	make bin ALG=$alg STRIDE=2 > /dev/null 2>&1
done
printf "\n"

for size in $SIZES
do
	printf "$size"
	for alg in $ALGS
	do
		res=$(./$alg -v$IPv $DEFAULT_RULE ipv$IPv-$size ipv$IPv-lookup)
		printf "\t$res"
	done
	printf "\n"
done
