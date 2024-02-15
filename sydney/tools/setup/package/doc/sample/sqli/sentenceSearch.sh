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
# 自然文検索の例

# データベースの設定
sqli="/var/lib/DoqueDB/bin/sqli"
server="localhost"
port="54321"
database="sampleSqli"
user="root"
passwd="doqadmin"

sqli_cmd="$sqli -remote $server $port -user $user -password $passwd -database $database -code utf-8"

# 山月記を検索
search_word="秀才であったが現状に満足できず、羞恥心のあまり、ある時、虎になってしまう。
ある日、偶然、昔の友人と再会し、虎になった経緯を話す。"
query="select docId, score(content), title, lastName, firstName, kwic(content for 150)
from AozoraBunko
where content contains freetext('$search_word')
order by score(content)
desc limit 15"
value=`$sqli_cmd -sql $query`

echo "自然文：" $search_word ; echo $value | tr '{},' '\n\n '
echo ;

# 白雪姫を検索
search_word="小人と暮らすお姫さまが悪いおばあさんに毒リンゴを食べさせられる話"
query="select docId, score(content), title, lastName, firstName, kwic(content for 150)
from AozoraBunko
where content contains freetext('$search_word')
order by score(content)
desc limit 15"
value=`$sqli_cmd -sql $query`
echo "自然文：" $search_word ; echo $value | tr '{},' '\n\n '
echo ;
