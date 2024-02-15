#!/bin/bash
# 
# Copyright (c) 2023 Ricoh Company, Ltd.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
# 事前にDoqueDBをインストールし、実行しておくこと
# 青空文庫のデータをもとにDBを立ち上げ、本文の全文索引を作成する

# 絶対パスの取得
dir=`echo $(cd $(dirname $0) && pwd)`

# データベースの設定
sqli="/var/lib/DoqueDB/bin/sqli"
server="localhost"
port="54321"
database="sampleSqli"
user="root"
passwd="doqadmin"

sqli_cmd="$sqli -remote $server $port -user $user -password $passwd -code utf-8"

# データベースの作成
query="create database $database"
value=`$sqli_cmd -sql "$query"`

result=$?
echo $value

# 結果のチェック
if [ $result -eq 0 ]; then
    echo "[INFO] データベースの作成終了"
else
    echo "[ERROR] 予期せぬエラーが発生 異常終了"
    exit 1
fi

# テーブルの作成
query="create table
    AozoraBunko (
        docId int,
        title nvarchar(256),
        lastName nvarchar(128),
        firstName nvarchar(128),
        url varchar(128),
        content ntext,
        primary key(docId)
    )"
value=`$sqli_cmd -database $database -sql $query`

result=$?
echo $value

# 結果のチェック
if [ $result -eq 0 ]; then
    echo "[INFO] テーブルの作成終了"
else
    echo "[ERROR] 予期せぬエラーが発生 異常終了"
    exit 1
fi

# バッチインサートの実施
query="insert into AozoraBunko
    input from path '${dir}/../data/insert.csv'
    hint 'code=\"utf-8\" InputField=(1,2,16,17,51,57)'"
value=`$sqli_cmd -database $database -sql "$query"`

result=$?
echo $value

# 結果のチェック
if [ $result -eq 0 ]; then
    echo "[INFO] バッチインサート終了"
else
    echo "[ERROR] 予期せぬエラーが発生 異常終了"
    exit 1
fi

# 全文索引の作成
query="create fulltext index INDEX1 on AozoraBunko(content)
    hint 'kwic,
    delayed,
    inverted=(normalized=(stemming=false, deletespace=false),
    indexing=dual,
    tokenizer=DUAL:JAP:ALL:1 @NORMRSCID:1 @UNARSCID:1)'"
value=`$sqli_cmd -database $database -sql $query`

result=$?
echo $value

# 結果のチェック
if [ $result -eq 0 ]; then
    echo "[INFO] 全文索引の作成終了"
else
    echo "[ERROR] 予期せぬエラーが発生 異常終了"
    exit 1
fi
