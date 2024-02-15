#!/bin/sh

# SydTest.sh から sydtestを作る


###########
# 変数設定
###########

. ./conf.sh

# root で SydTest を実行することは想定していない。
#installpath=/proj/sydney/work/`whoami`

sed -e "s!%INSTALL_PATH%!$installpath!g" < SydTest.sh > sydtest
chmod 0755 sydtest
install -m 0755 sydtest $installpath/bin

echo "copy conf.sh to Test/SydTest/test"
cp -f conf.sh ../../Test/SydTest/test
