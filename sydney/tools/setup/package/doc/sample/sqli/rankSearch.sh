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
# 実行する前に setup.sh を 実行し DB を立ち上げておくこと
# ランキング検索の例

# データベースの設定
sqli="/var/lib/DoqueDB/bin/sqli"
server="localhost"
port="54321"
database="sampleSqli"
user="root"
passwd="doqadmin"

sqli_cmd="$sqli -remote $server $port -user $user -password $passwd -database $database -code utf-8"

# 1単語で検索
query="select docId, score(content), title, lastName, firstName, kwic(content for 150)
from AozoraBunko
where content contains('難破船')
order by score(content) desc limit 5"
value=`$sqli_cmd -sql $query`

echo "難破船 のランキング検索" ; echo $value | tr '{},' '\n\n '
echo ;

# or で検索
query="select docId, score(content), title, lastName, firstName, kwic(content for 150)
from AozoraBunko
where content contains('難破船'|'無人島'|'太平洋'|'漂流')
order by score(content) desc limit 5"
value=`$sqli_cmd -sql $query`

echo "難破船, 無人島, 太平洋, 漂流 のランキング検索(OR)" ; echo $value | tr '{} ,' '\n\n\n '
echo ;

# and で検索
query="select docId, score(content), title, lastName, firstName, kwic(content for 150)
from AozoraBunko
where content contains('難破船'&'無人島'&'太平洋'&'漂流')
order by score(content) desc limit 5"
value=`$sqli_cmd -sql $query`
echo "難破船, 無人島, 太平洋, 漂流 のランキング検索(AND)" ; echo $value | tr '{} ,' '  \n '
echo ;
