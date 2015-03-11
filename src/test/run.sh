fail=0
pass=0
tmp=`mktemp`
ALG=$2

if [ $1 = "lpm" ]; then
	if [ $# -eq 2 ] && [ $2 = "bspl" ]; then
		STRIDES=1
	elif [ $2 != "bspl" ]; then
		shift
		shift
		STRIDES=$@
	fi
else
	STRIDES=1
fi

for stride in $STRIDES
do
	ALG_UPPERCASE=`echo $ALG | tr [a-z] [A-Z]`
	echo -e "\e[1m======================================================================="

	if [ $ALG = "tbm" ]; then
		echo -e "\e[1m $ALG_UPPERCASE, stride = $stride"
	else
		echo -e "\e[1m $ALG_UPPERCASE"
	fi

	echo -e "\e[1m-----------------------------------------------------------------------"
	make bin ALG=$ALG STRIDE=$stride > $tmp 2>&1

	if [ $? -ne 0 ]; then
		cat $tmp
		exit 1
	fi


	for testfile in `ls tests`
	do
		./$ALG tests/$testfile

		if [ $? -eq 0 ]; then
			valgrind ./$ALG tests/$testfile 2> $tmp > /dev/null
			grep 'no leaks' $tmp > /dev/null && grep '0 errors' $tmp > /dev/null

			if [ $? -eq 0 ]; then
				echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;32mPASS\033[0m tests/$testfile"
				let "pass++"
			else
				echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;31mFAIL\033[0m tests/$testfile"
				let "fail++"
			fi
		else
			echo -e " \033[1;31mFAIL\033[0m tests/$testfile"
			let "fail++"
		fi

	done
done

	echo -e "\e[1m======================================================================="
	echo " RESULTS"
if [ $fail -eq 0 ]; then
	echo -e " \033[1;32mPASS: $pass\033[0m"
else
	echo -e " \033[1;31mFAIL: $fail\033[0m, \033[1;32mPASS: $pass\033[0m"
fi

if [ $fail -ne 0 ]; then
	exit 1
fi


