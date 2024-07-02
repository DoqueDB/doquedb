■概要

DoqueDBは、強力な日本語全文検索を特長とするリレーショナルデータベースです。
この文書の末尾にドキュメントへのリンクがあります。

■動作環境

DoqueDBは以下の環境でご利用いただけます。
  ・OS: Linux (64bit) *1
  ・アーキテクチャ: x86_64
  ・メモリ: 2GB以上 *2
  ・ディスク容量: 1GB以上 *3

*1 動作確認は以下の環境で行っています。
  ・RedHat Enterprise Linux 7
  ・CentOS 7

*2 データサイズに応じて必要なメモリは変化します。
*3 データベースやログのサイズに応じて必要なディスク容量は変化します。

■使ってみる

(1) インストールとサーバーの起動

インストールはroot権限のあるユーザーで行う必要があります。
これ以降、rootユーザーで実行することをプロンプト「#」で示します。

  # tar xvf doquedb-<バージョン>.tar.gz
  # cd doquedb-<バージョン>
  # ./install.sh
  # ./setup.sh
  # cd /var/lib/DoqueDB/bin
  # ./doquedb start

(2) サンプルの実行

サンプルの実行は一般ユーザー権限で行えます。
一般ユーザーで実行することをプロンプト「$」で示します。

  $ mkdir ~/doquedb
  $ cp -rp /var/lib/DoqueDB/doc/sample ~/doquedb
  $ cd ~/doquedb/sample/sqli

データベースとテーブルを作り、データを登録して索引を作ります。
その後各種検索を実行します。
検索の詳細については各シェルスクリプトをご覧ください。

  $ ./setup.sh
  $ ./likeSearch.sh
  $ ./rankSearch.sh
  $ ./sentenceSearch.sh

(3)  サーバーの停止

  # cd /var/lib/DoqueDB/bin
  # ./doquedb stop

(4) アンインストール

  # cd <パッケージを展開したディレクトリ>
  # ./unsetup.sh
  # ./uninstall.sh
  # rm -fr /var/lib/DoqueDB

■ドキュメント

以下のドキュメントをご覧いただけます。
現時点では日本語のドキュメントのみ用意されています。

  ・使ってみよう https://github.com/doquedb/doquedb/tree/master/docs/howtouse.html
  ・ユーザーズマニュアル https://github.com/doquedb/doquedb/tree/master/docs/users.html
  ・ナレッジ https://github.com/doquedb/doquedb/tree/master/docs/knowledge.html
  ・JDBC Driver https://github.com/doquedb/doquedb/tree/master/docs/javadoc/index.html
  ・Hibernate Dialect https://github.com/doquedb/doquedb/tree/master/docs/dialect/index.html

■コミュニティ

ご連絡や不具合報告についてはGitHubのIssuesおよびPull Requestをご利用ください。

■ライセンス

DoqueDBのソースコードはApache License 2.0に基づいて公開されています。
第三者ソフトウェアのライセンスについてはLICENSES/NOTICE.txtをご覧ください。
