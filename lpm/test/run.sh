fail=0
pass=0

for testfile in `ls $1`
do
	./cmd < $1/$testfile

	if [ $? -eq 1 ]; then
		echo -e " \033[1;31mFAIL\033[0m $1/$testfile"
		let "fail++"
		# $((fail++))
	else
		echo -e " \033[1;32mPASS\033[0m $1/$testfile"
		let "pass++"
	fi

done

echo -e " \033[1;31m$fail\033[0m/\033[1;32m$pass\033[0m"


