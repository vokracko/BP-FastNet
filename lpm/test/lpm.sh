fail=0
pass=0

for testfile in `ls lpm`
do
	./$1 lpm/$testfile

	if [ $? -eq 0 ]; then
		valgrind ./$1 lpm/$testfile 2>&1 | grep 'no leaks' > /dev/null

		if [ $? -eq 0 ]; then
			echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;32mPASS\033[0m lpm/$testfile"
			let "pass++"
		else
			echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;31mFAIL\033[0m lpm/$testfile"
			let "fail++"
		fi
	else
		echo -e " \033[1;31mFAIL\033[0m lpm/$testfile"
		let "fail++"
	fi

done

echo -e " \033[1;31mFAIL: $fail\033[0m, \033[1;32mPASS: $pass\033[0m"


