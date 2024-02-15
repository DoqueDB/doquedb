#
# 既に作ってある140?.txtから141?.txtおよび142?.txtを生成する。
for i in 0 2 4 6 8; do
    perl -pe 's/indexing=word/indexing=dual/' 140$i.txt >141$i.txt
    perl -pe 's/indexing=word/indexing=ngram/' 140$i.txt >142$i.txt
done