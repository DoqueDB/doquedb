## 概要

DoqueDBは、強力な日本語全文検索を特長とするリレーショナルデータベースです。  
この文書の末尾にドキュメントへのリンクがあります。

## 動作環境

DoqueDBは以下の環境でご利用いただけます。
  - OS: Linux (64-bit) \*1
  - アーキテクチャ: x86\_64
  - メモリ: 2GB以上 \*2
  - ディスク容量: 1GB以上 \*3

\*1 動作確認は以下の環境で行っています。
  - RedHat Enterprise Linux 7
  - CentOS 7

\*2 データサイズに応じて必要なメモリは変化します。  
\*3 データベースやログのサイズに応じて必要なディスク容量は変化します。

## 使ってみる

Releasesよりバイナリパッケージを取得してください。バイナリパッケージの  
ファイル名はdoquedb-\<バージョン\>.\<アーキテクチャ\>.tar.gzです。

### インストールとサーバーの起動

インストールはroot権限のあるユーザーで行う必要があります。  
これ以降、rootユーザーで実行することをプロンプト「#」で示します。
```
# tar xvf doquedb-<バージョン>.<アーキテクチャ>.tar.gz
# cd doquedb-<バージョン>.<アーキテクチャ>
# ./install.sh
# ./setup.sh
# cd /var/lib/DoqueDB/bin
# ./doquedb start
```

### サンプルの実行

サンプルの実行は一般ユーザー権限で行えます。  
一般ユーザーで実行することをプロンプト「$」で示します。
```
$ mkdir ~/doquedb
$ cp -rp /var/lib/DoqueDB/doc/sample ~/doquedb
$ cd ~/doquedb/sample/sqli
```

データベースとテーブルを作り、データを登録して索引を作ります。  
その後各種検索を実行します。  
検索の詳細については各シェルスクリプトをご覧ください。
```
$ ./setup.sh
$ ./likeSearch.sh
$ ./rankSearch.sh
$ ./sentenceSearch.sh
```

### サーバーの停止

```
# cd /var/lib/DoqueDB/bin
# ./doquedb stop
```

### アンインストール

```
# cd <パッケージを展開したディレクトリ>
# ./unsetup.sh
# ./uninstall.sh
# rm -fr /var/lib/DoqueDB
```

## DoqueDBをソースコードからビルドする

[BUILDING_PROCEDURE_ja.md](./BUILDING_PROCEDURE_ja.md)を参照してください。

## ドキュメント

以下のドキュメントをご覧いただけます。  
現時点では日本語のドキュメントのみ用意されています。
* [使ってみよう(How to use)](https://doquedb.github.io/doquedb/howtouse.html)
* [ユーザーズマニュアル(Users Manual)](https://doquedb.github.io/doquedb/users.html)
* [ナレッジ(Knowledge)](https://doquedb.github.io/doquedb/knowledge.html)
* [JDBC Driver](https://doquedb.github.io/doquedb-javadoc/javadoc/index.html)
* [Hibernate Dialect](https://doquedb.github.io/doquedb-javadoc/dialect/index.html)

## コミュニティ

ご連絡や不具合報告についてはGitHubのIssuesおよびPull Requestをご利用ください。  
Pull Requestを用いてコードをコントリビュートしていただくにあたっては、  
コントリビューターライセンス契約(個人用:[INDIV\_CLA.txt](https://github.com/doquedb/doquedb/INDIV_CLA.txt), 法人用:[CORP\_CLA.txt](https://github.com/doquedb/doquedb/CORP_CLA.txt))に  
同意していただく必要があります。  
必要事項を記入して、記載のアドレスまでご送付ください。

## ライセンス

DoqueDBのソースコードはApache License 2.0に基づいて公開されています。  
第三者ソフトウェアのライセンスについてはLICENSE/NOTICE.txtをご覧ください。
