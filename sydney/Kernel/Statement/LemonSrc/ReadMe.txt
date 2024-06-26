■ Lemon 修正時に関するメモ

［始めに］
	このファイルには Statement で Lemon ソース修正時に必要な
	情報が書かれています。

［コンパイル環境］
	  Lemon ソースをコンパイルするには Windows で動作する Lemon.exe が
	必要です。 VSS に登録してあるので各自ダウンロードしてください。
	ダウンロードするファイルは Lemon.exe と lempar.c だけで構いません。

	  ダウンロードしたファイルを適当なフォルダに入れ、Developer Studio
	を起動してください(注１)。

	  次に Developer Studio 上で Lemon.exe をダウンロードしたフォルダに
	パスを設定します。手順は以下の通りです（注２）。

		① ツール(T) － オプション(O) をクリック
		② オプションウィンドウ － ［ディレクトリ］タブ を選択
		③ 表示するディレクトリ(S) で ［実行可能ファイル］を選択。
		④ ディレクトリ(D) 欄に Lemon.exe をダウンロードしたフォルダを追加

	  最後に Perl が動く環境にしてください。

［コンパイル方法］
	  ビルドターゲットを ［Win32 Lemon］に設定し、ビルドしてください。
	lemon ファイルのみをコンパイルします。
	その他のターゲットでは lemon ファイルのビルドは行ないません。

［コンパイルに関する注意］
	  Lemon ソースをコンパイルすると、.c、.h ファイルを出力します。
	Statement では Lemon コンパイル後、Sydney で動作できるように perl で .c を修正し、
	.cpp に出力しています。このため lemon ファイルをコンパイルする時は
	.cpp と .h は VSS よりチェックアウトし、上書きできる状態にしてください。

［コンパイルの流れ］
	  コンパイルの流れを以下に示します。
+--------------------+-----------+-------------+------------------------------+
| action			 | input file| output file | command					  |
+--------------------+-----------+-------------+------------------------------+
| lemon のコンパイル | .lemon	 | .c .h	   | lemon input.lemon			  |
+--------------------+-----------+-------------+------------------------------+
| .c を .cpp に変換	 | .c		 | .cpp		   | perl .\fixlemon			  |
|					 |			 |			   |			input.c input.cpp |
+--------------------+-----------+-------------+------------------------------+
| ファイルのコピー	 | .cpp .h	 | .cpp .h	   | copy input.cpp project_dir	  |
|					 |			 |			   | copy input.h				  |
|					 |			 |			   |	   project_dir\project_dir|
+--------------------+-----------+-------------+------------------------------


注１：
	なぜだか分かりませんが、スペースを含むロングファイルパスでは
	lemon.exe は動作しないようです（Program Files等。Progra~1 は OK）。

注２：
	なぜだか分かりませんが、フルパスで指定しないと 
		Can't open the template file "lempar.c".
	というエラーメッセージが出力されます。
	適宜 Lemon プロジェクトのカスタムビルドの設定を変えてください。

