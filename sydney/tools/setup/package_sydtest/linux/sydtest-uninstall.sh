#!/bin/sh

# sydtestを消す


###########
# 変数設定
###########

. ./conf.sh

# root で SydTest を実行することは想定していない。
#installpath=/proj/sydney/work/`whoami`$installpath


#########
# メイン
#########

echo "remove sydtest"
rm -f ${installpath}/bin/sydtest
