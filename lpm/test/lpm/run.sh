fail=0
pass=0
tmp=`mktemp`


if [ $# -eq 1 ] && [ $1 = "bspl" ]; then
	STRIDES=1
	ALG=$1
elif [ $# -gt 1 ] && [ $1 != "bspl" ]; then
	ALG=$1
	shift
	STRIDES=$@
fi

for stride in $STRIDES
do
	ALG_UPPERCASE=`echo $ALG | tr [a-z] [A-Z]`
	echo -e "\e[1m======================================================================="
	echo -e "\e[1m$ALG_UPPERCASE, stride = $stride"
	echo -e "\e[1m-----------------------------------------------------------------------"
	make cmd ALG=$ALG STRIDE=$stride > $tmp 2>&1

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

echo -e " \033[1;31mFAIL: $fail\033[0m, \033[1;32mPASS: $pass\033[0m"

if [ $fail -ne 0 ]; then
	exit 1
fi


