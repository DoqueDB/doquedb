＜環境設定＞
export LANG=ja_JP.utf8

export JAVA_HOME=/usr/j2se8

export DISPLAY=:0.0  //preparedstatementのエラー回避用


＜テストについて＞

接続先の変更はTestBase.javaでする。

PreparedStatementTest.java
　f_nclob, f_blob型は処理の中でバグを引き起こすので、おかしい挙動をするものはコメントアウトしている。
　test_cancel()はいつでも成功するとは限りません（ps.cancel()の仕様？）。java6では失敗が多く、java8では成功している。　

DatabaseMetaDataTest.java
　test_0332()のエラーは以下を確認する。
　　・mkdir先の権限があるか
　　・サービスが動いているホストで実行しているか
