for i in 3[78][0-9][1357].txt 4401.txt
do
	i=${i%.txt}
	echo $i
	perl split.pl $i
	mv ${i}b.txt ${i}.txt
done
