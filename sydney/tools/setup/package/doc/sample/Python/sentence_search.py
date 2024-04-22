#!/bin/python3
# Copyright (c) 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
sentence_search.py -- 自然文検索の例
事前準備：
    1. DoqueDBをインストールし、実行しておくこと
    2. py-doquedbをインストールしておくこと(手順はsydney/Python/README.mdを参照)
    3. ./setup.py(Python/setup.pyではない)を実行し、青空文庫のDBを立ち上げておくこと
"""
import doquedb as dq

# 白雪姫を検索する
try:
    conn = dq.connect(host='localhost',
                      port=54321,
                      dbname='samplePython',
                      user='root',
                      password='doqadmin',
                      autocommit=True)
    cur = conn.cursor()
    searchword = "小人と暮らすお姫さまが悪いおばあさんに毒リンゴを食べさせられる話"
    cur.execute(f"""
                select docId,
                       score(content),
                       title,
                       lastName,
                       firstName,
                       kwic(content for 150)
                from AozoraBunko where content contains
                    freetext('{searchword}')
                order by score(content)
                desc limit 5
                """)
    # 検索結果を取得してprintする
    rows = cur.fetchall()
    assert rows
    print(*rows, sep='\n')
except dq.Error as e:
    print("error:" + e)
finally:
    cur.close()
    conn.close()

dq.close()
