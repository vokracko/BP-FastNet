fail=0
pass=0

for testfile in `ls $1`
do
	./cmd < $1/$testfile

	if [ $? -eq 0 ]; then
		valgrind ./cmd < $1/$testfile 2>&1 | grep 'no leaks' > /dev/null

		if [ $? -eq 0 ]; then
			echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;32mPASS\033[0m $1/$testfile"
			let "pass++"
		else
			echo -e " \033[1;32mPASS\033[0m, valgrind: \033[1;31mFAIL\033[0m $1/$testfile"
			let "fail++"
		fi
	else
		echo -e " \033[1;31mFAIL\033[0m $1/$testfile"
		let "fail++"
	fi

done

echo -e " \033[1;31mFAIL: $fail\033[0m, \033[1;32mPASS: $pass\033[0m"


