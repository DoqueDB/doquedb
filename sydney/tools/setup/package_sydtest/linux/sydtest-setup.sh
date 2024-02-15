#!/bin/sh

# sydtest.confを作る。

###########
# 変数設定
###########

. ./conf.sh

# root で SydTest を実行することは想定していない。
#installpath=/proj/sydney/work/`whoami`$installpath

modregfile=mod.conf
sydneyregfile=default.conf
regfile=sydtest.conf

setuptop=../../tools/setup
testtop=../../Test/SydTest/test

#####################
# ファイルの書き込み
#####################

sydtestparam () {
	cat ${modregfile} > ${regfile}
	cat ${sydneyregfile} >> ${regfile}
	cat >> ${regfile} <<_EOF_
#############################################
# for SydTest
SydTest_MessageOutputInfo	"1"
SydTest_MessageOutputDebug	"0"
SydTest_MessageOutputError	"1"
SydTest_MessageDisplayThreshold	500
SydTest_MessageOutputTime	"1"
Common_MessageIncludeDate	FALSE
Common_MessageOutputInfo	"0"
Common_MessageOutputError	"1"
Common_LikeUnaResource		2
Execution_LikeNormalizedString	0

_EOF_
}

#########
# メイン
#########

sydtestparam

if [ ! -f $installpath/etc/$regfile ]; then
    echo "make sydtest.conf"
    install -m 0644 $regfile $installpath/etc
    install -m 0644 $regfile ${testtop}/sydtest_default.conf
fi

if [ -e ../../Test/SydTest/test/restore.sh ]; then
    echo "set execution flag for Test/SydTest/test/restore.sh"
    chmod u+x ../../Test/SydTest/test/restore.sh
else
    echo "You can't examine single/normal/2860.txt,"
    echo "because you don't have Test/SydTest/test/restore.sh"
fi
