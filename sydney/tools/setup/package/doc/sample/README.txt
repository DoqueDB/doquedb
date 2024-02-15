青空文庫のデータを用いた DoqueDB のJava および Sqli のサンプルコードをまとめました。
詳しくは https://github.com/doquedb/doquedb/tree/master/docs/howtouse.html
の「使ってみよう」をご確認ください。

構成
data/
    insert.csv
        青空文庫の「公開中 作家別作品一覧拡充版(書誌情報)」
        https://www.aozora.gr.jp/index_pages/list_person_all_extended.zip
        のうち、サンプルに含まれている作品の行のみを抽出し、
        さらに各行末尾(第57カラム)にOUTPUTフォルダ中のテキストファイル名
        (<人物ID>_<作品ID>.txt)を追加したもの

    DATA/*.zip
        insert.csvのテキストファイルURLをダウンロードしたもの

    OUTPUT/*.txt
        DATAのテキストファイルをデモサイトの全文検索用に加工したもの
        加工要領は以下
        ・旧字旧仮名はそのままとする
        ・文字コードはShiftJISとする
        ・改行はLFまたはCR/LFとする(作業環境による)
        ・作品名等の書誌情報はそのまま出力する
        ・テキスト中に現れる記号についての情報は削除する
        ・底本情報とその前の3行開きは削除する
        ・注記、ルビ、ルビ開始位置は削除する
        ・アクセント分解はそのまま残す
        ・外字や特殊記号は「※」だけを残す
        ・くの字点、濁点付きのくの字点はそのまま残す
        ・エスケープされた特殊文字は元の文字に戻す
        ・罫線素片(およびその並び)は全角スペース1字に変換する
        ・上記加工後、連続する空行は1行にまとめる
        加工したテキストは以下のファイルに置く。
        {人物ID(ゼロ埋め6桁)}_{作品ID(ゼロ埋め6桁}.txt

Java/
    ※ javac などでjavaのコードをビルドし、クラスを実行してください。
    ※ クラスファイルの実行時には、-classpath にインストールされた doquedb.jarのパスを指定してください
    ※ doquedb.jar のインストール先は /var/lib/DoqueDB/bin/java/doquedb.jar になります   
    Setup.java
        Java のサンプルコードを実行するための DB の作成をする

    SentenceSearch.java
        類似文書検索のサンプルコード

sqli/
    setup.sh
        sqli のサンプルコードを実行するための DB 
        
    likeSearch.sh
        あいまい検索のサンプルコード
        
    rankSearch.sh
        ランキング検索のサンプルコード

    sentenceSearch.sh
        類似文書検索のサンプルコード
