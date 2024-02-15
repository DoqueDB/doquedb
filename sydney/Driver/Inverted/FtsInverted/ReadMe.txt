
<FtsInv に置いたが、どうしても変更が必要だったこと>

・ModInvertedQueryBaseNode.cpp がコンパイルできなかったので、
　ModInvertedLocationListIterator.h をインクルードするように変更。

・ModInvertedException.h の末尾の数行が邪魔なのでコメントアウト

・ModInvertedFileCapsule.* の不要なコードを削除

・ModInvertedQueryParser.cpp でＭＯＤのテキスト格納クラスを
  必要とする関数 parseAsRegex があったので、関数の中身を
  全てコメントアウトした。

　Sydneyでは正規表現の検索ができないことになっているので
　ここを通ることは無い。。。通ってしまったらアサートが出る。

・ModInvertedQueryParser.cpp からせ正規表現に関する部分を
　コメントアウトしたので以下の1行もコメントアウト。

	#include "ModInvertedRegexLeafNode.h"

・ModInvertedQuery.h の protected メンバを public にした。
　FTS版転置の ModInvertedFile クラスは friend 指定されてたから
　データメンバにアクセスできていた。

　Sydney版転置でもデータメンバにアクセスする必要があるので、
　ひとおもいに public にした。個人的に friend 指定は嫌い。

・[超重要]　ModInvertedNgramTokenizer.cpp の関数
   ModInvertedNgramTokenizer::setCharOffsets でバイトオフセットを文字オフセットに
   変換する処理で「2で割る」という処理を削除した。
　

