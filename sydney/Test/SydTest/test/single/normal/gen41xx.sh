#
# 既に作ってある411?.txtから410?.txtを、
# また、415?.txtからおよび41[234]?.txtを生成する。

for i in 415?.txt; do
   j=`expr ${i%.txt} - 20`
   echo $j
   perl -pe 's/sectionized, //; s/, sectionized\([^)]+\)//' $i > $j.txt
done

for i in 41[135]?.txt; do
   j=`expr ${i%.txt} - 10`
   echo $j
   perl -pe 's/delayed, //' $i > $j.txt
done

