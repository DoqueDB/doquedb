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
setup.py -- 青空文庫のデータをもとにDBを立ち上げ、本文の全文索引を作成する
事前準備：
    1. DoqueDBをインストールし、実行しておくこと
    2. py-doquedbをインストールしておくこと(手順はsydney/Python/README.mdを参照)
"""

import os
import doquedb as dq

# カレントディレクトリの取得
path = os.getcwd()

# 1. サンプルコード実行用のデータベースを作成する
# 接続情報を与え、データベースに接続する
try:
    conn = dq.connect(host='localhost',
                      port=54321,
                      dbname='DefaultDB',
                      user='root',
                      password='doqadmin',
                      autocommit=True)  # スキーマ操作のため明示的なトランザクションは利用できない
    # データベース操作のためのCursorオブジェクトを作り、データベースを作成する
    print("データベースの作成を開始")
    cur = conn.cursor()
    cur.execute("create database samplePython")
    rows = cur.fetchall()
    print(rows)
except dq.Error as e:
    print("error: " + e)
finally:
    cur.close()
    conn.close()

# 2. 作成したデータベースに再度アクセスしテーブルを作成
try:
    conn = dq.connect(host='localhost',
                      port=54321,
                      dbname='samplePython',
                      user='root',
                      password='doqadmin',
                      autocommit=True)  # スキーマ操作のため明示的なトランザクションは利用できない
    cur = conn.cursor()
    # テーブルを作成
    print("テーブルの作成を開始")
    rs = cur.execute("""
                CREATE TABLE AozoraBunko (
                docId int,
                title nvarchar(256),
                lastName nvarchar(128),
                firstName nvarchar(128),
                url varchar(128),
                content ntext,
                primary key(docId))
                """)
    rows = cur.fetchall()
    print(rows)
except dq.Error as e:
    print("error:" + e)
finally:
    cur.close()
    conn.close()

# 3. トランザクション処理のため再度データベースに接続しデータを登録する
try:
    conn = dq.connect(host='localhost',
                      port=54321,
                      dbname='samplePython',
                      user='root',
                      password='doqadmin',
                      autocommit=False)  # オートコミットを切る
    # トランザクションを開始
    conn.begin_transaction()
    cur = conn.cursor()
    # バッチインサートを実行
    print("バッチインサートを開始")
    statement = f"""
    insert into AozoraBunko
    input from path '{path}/../data/insert.csv'
    hint 'code="utf-8" InputField=(1,2,16,17,51,57)'
    """
    cur.execute(statement)
    rows = cur.fetchall()
    print(rows)
    conn.commit()
except dq.Error as e:
    print("error: " + e)
    conn.rollback()
    cur.close()
    conn.close()

# 4. 全文索引の作成のため再度データベースに接続し、索引を登録する
try:
    conn = dq.connect(host='localhost',
                      port=54321,
                      dbname='samplePython',
                      user='root',
                      password='doqadmin',
                      autocommit=True)  # スキーマ操作のため明示的なトランザクションは利用できない
    cur = conn.cursor()
    # 全文索引の作成
    print("全文索引の作成を開始")
    statement = """
    create fulltext index INDEX1 on AozoraBunko(content)
    hint 'kwic,
          delayed,
          inverted=(normalized=(stemming=false, deletespace=false),
          indexing=dual,
          tokenizer=DUAL:JAP:ALL:1 @NORMRSCID:1 @UNARSCID:1)'
    """
    cur.execute(statement)
    rows = cur.fetchall()
    print(rows)
except dq.Error as e:
    print("error: " + e)
    cur.close()
    conn.close()

# 終了処理
dq.close()
