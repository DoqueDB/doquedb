# 概要

py-doquedb は、[PEP 249 – Python Database API Specification v2.0](https://peps.python.org/pep-0249/)に準拠した DoqueDB のための Python クライアントライブラリです。

# 動作環境

- Python 3

# はじめに

[Release](https://github.com/DoqueDB/doquedb/releases) よりバイナリパッケージを取得してください。バイナリパッケージのファイル名は py_doquedb-<バージョン>-py3-none-any.whl です。

# インストール

Python3 以降のバージョンをインストールし、以下のコマンドを実行します。
`pip`コマンドはインストール環境によって`pip3`に適宜置き換えてください。

```
$ pip install py_doquedb-<バージョン>-py3-none-any.whl
```

# サンプルコードの実行

py-doquedb の基本的な実装方法はサンプルコード`sample.py`で確認できます。
以下のコマンドを実行して動作確認ができます。

```
$ cd sydney/Python
$ python sample.py
[(1, 'BTC', 10200), (2, 'ETH', 5000), (3, 'XEM', 2500), (4, 'XRP', 1000), (5, 'MONA', 3000), (6, 'XP', 1000)]
```

また、青空文庫のデータを用いた py-doquedb のサンプルコードが `sydney/tools/setup/package/doc/sample/Python` に配置されています。こちらの詳細については「使ってみよう」の [Python でサンプルを実行する](https://doquedb.github.io/doquedb/howtouse.html#sec6) を確認してください。

# 開発環境構築とテスト

## ディレクトリ構成

```
sydney/Python
├── dist            // インストールパッケージを配置
├── docs            // Python クライアントのドキュメント配置
|   └── source      // ドキュメント生成のための設定ファイルとreSTファイルを配置
├── src/doquedb     // Python クライアントのソースコード
├── test            // Python クライアントのテストコード
├── tools           // エラーファイルの自動生成コードなどを配置
├── .gitigonre      // 標準的な gitignore
├── Pipfile         // pipenv で利用するインストールパッケージおよびスクリプトの管理ファイル
├── Pipfile.lock    // pipenv で利用するインストールパッケージの依存関係などを保存したファイル
├── pytest.ini      // test に用いる pytest の設定ファイル
├── README_ja.md    // 日本語のREADMEファイル
├── README.md       // READMEファイル
├── sample.py       // py-doquedb のサンプルコード
├── setup.cfg       // パッケージ作成に使う setuptools の設定ファイル
└── setup.py        // パッケージ作成のためのコード
```

## 仮想環境の構築

pipenv を用いて Python の仮想環境を構築します。pipenv についての詳細は[https://pipenv-ja.readthedocs.io/ja/translate-ja/](https://pipenv-ja.readthedocs.io/ja/translate-ja/)をご確認ください。
py-doquedb は Python3 であれば特にバージョンの指定なく動作しますが、pytest など dev packages でインストールされる依存ライブラリには Python のバージョン指定があるため、インストールパッケージが要求するバージョン以降の Python をインストールしている必要がある点に注意してください。

```
$ pip install pipenv
$ cd sydney/Python
$ pipenv install --dev
```

## エラーファイルの作成

リポジトリにはエラーファイルなど、自動生成されるファイルは含まれていないため、以下のコマンドを実行することでファイルを作成する必要があります。コマンドを実行することで以下のファイルが `sydney/Python/src/exception` の下に作成されます。

- database_exception.py
- errorcode.py
- message_format_eg.py
- message_format_jp.py
- raise_error.py

```
$ pipenv run make_exception
```

## テスト

以下のスクリプトを実行することで pytest が実行されます。

```
$ pipenv run test
```

正常に実行され、テストが全て通った場合、以下のような出力が得られます。テストに失敗した場合は test ファイルの横の実行結果の表示が `.`ではなく`F`となり失敗したテストの詳細が以降に表示されます。

```
$ pipenv run test
============================================== test session starts ===============================================
platform linux -- Python 3.11.7, pytest-8.1.1, pluggy-1.4.0
rootdir: <your rootdir will be shown here>
configfile: pytest.ini
testpaths: ./test
collected 378 items

test/client/test_connection.py ......ssss...                                                               [  3%]
test/client/test_datasource.py ................s..sssss..                                                  [ 10%]
test/client/test_port.py .................                                                                 [ 14%]
test/client/test_resultset.py ........s.....                                                               [ 18%]
test/client/test_session.py ................s.......                                                       [ 24%]
test/common/test_abstracts.py ..........                                                                   [ 27%]
test/common/test_arraydata.py .................                                                            [ 32%]
test/common/test_data.py ...............                                                                   [ 35%]
test/common/test_instance.py ....                                                                          [ 37%]
test/common/test_iostream.py ..........................                                                    [ 43%]
test/common/test_language.py ...................                                                           [ 48%]
test/common/test_scalardata.py .....................ss..................................ss....             [ 65%]
test/common/test_serialdata.py ......................s..                                                   [ 72%]
test/common/test_unicodestr.py ........                                                                    [ 74%]
test/driver/test_connection.py ..................................                                          [ 83%]
test/driver/test_cursor.py ........................ssss..                                                  [ 91%]
test/exception/test_error_message.py .                                                                     [ 91%]
test/exception/test_raise_error.py .                                                                       [ 91%]
test/port/test_connection.py ......                                                                        [ 93%]
test/port/test_constants.py .                                                                              [ 93%]
test/test_doquedb.py ........                                                                              [ 95%]
test/test_scenario.py ................                                                                     [100%]

================================== 357 passed, 21 skipped, 0 warnings in 29.16s ==================================
```

## パッケージング

以下のスクリプトを実行することでパッケージが作成されます。正常に実行されると、`dist/py_doquedb-<version>-py3-none-any.whl`、`build/`以下にビルドファイル、および`src/py_doquedb.egg-info`が作成されます。パッケージに利用されるバージョンなどの設定は`setup.cfg`に記載されていますので、バージョンの変更等があった場合はこちらのファイルを更新してからパッケージを作成してください。

```
pipenv run make_package
```

## ドキュメント作成

以下のスクリプトを実行することで、Sphinx を用いてドキュメントが自動生成されます。`sphinx-apidoc -f -o ./docs/source/ ./src`ではソースコード上に記載された docstring から`docs/source/`の下に reST ファイルを生成します。`sphinx-build -b html docs/source docs/build`では生成した reST ファイルから HTML を`docs/build/`以下に生成します。py-doquedb は docstring のスタイルとして Google スタイルを採用しています。Sphinx の詳細は[公式のドキュメント](https://www.sphinx-doc.org/ja/master/index.html)を確認してください。

```
$ cd sydney/Python
$ pipenv shell
$ pip install -e . # module import エラーを回避するため実行
$ sphinx-apidoc -f -o ./docs/source/ ./src # source配下にreSTファイルを生成
$ sphinx-build -b html docs/source docs/build # build配下にHTMLを生成
```

# ドキュメント

- [Python Driver](https://doquedb.github.io/doquedb-pythondoc/index.html)
