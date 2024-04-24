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
sample.py -- py-doquedbのサンプルコード
事前準備：
    1. DoqueDBをインストールしホスト:localhostポート:54321で立ち上げておく
    2. py-doquedbをインストールしておくこと(手順はREADME.mdを参照)
"""

import doquedb as dq

# サンプルコード実行用のデータベースを作成
# .connect()に必要なパラメータを渡し、Connectionオブジェクトをコンストラクト
conn = dq.connect(host='localhost',
                  port=54321,
                  dbname='DefaultDB',
                  user='root',
                  password='doqadmin',
                  autocommit=True)
# データベース操作のためのCursorオブジェクトをコンストラクト
cur = conn.cursor()
cur.execute("drop database TestPython if exists")
# execute後は必ず結果を取得する
cur.fetchall()
cur.execute("create database TestPython")
cur.fetchall()
# close処理
cur.close()
conn.close()

# 作成したデータベースに再度アクセスしテーブルを作成
conn = dq.connect(host='localhost',
                  port=54321,
                  dbname='TestPython',
                  user='root',
                  password='doqadmin',
                  autocommit=True)
cur = conn.cursor()
# テーブルを作成
cur.execute("drop table Test if exists")
cur.fetchall()
cur.execute("""
            CREATE TABLE Test (
            ID INT NOT NULL,
            name VARCHAR(100),
            price INT
            )
            """)
cur.fetchall()
# close処理
cur.close()
conn.close()

# トランザクション処理のため再度データベースに接続
conn = dq.connect(host='localhost',
                  port=54321,
                  dbname='TestPython',
                  user='root',
                  password='doqadmin',
                  autocommit=False)  # オートコミットを切る
# トランザクションを開始
conn.begin_transaction()
cur = conn.cursor()
# execute でインサートを実行
statement = "INSERT INTO Test VALUES (1, 'BTC', 10200)"
cur.execute(statement)
cur.fetchall()
# executemany でインサート文を複数回実行
statement = "INSERT INTO Test VALUES (?, ?, ?)"
records = [
    (2, 'ETH', 5000),
    (3, 'XEM', 2500),
    (4, 'XRP', 1000),
    (5, 'MONA', 3000),
    (6, 'XP', 1000)
]
cur.executemany(statement, records)
cur.fetchall()
cur.execute("SELECT * FROM Test")
# クエリの結果取得
rows = cur.fetchall()
assert rows is not None
# 結果を表示
print(rows)
conn.commit()
# close処理
cur.close()
conn.close()

# 終了処理
dq.close()
