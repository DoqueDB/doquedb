#
# Copyright (c) 2023,2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

UNA辞書ビルド環境

■ディレクトリ構成

resourceディレクトリ以下は以下の構成になっています。

  resource
    |- tools
    |    |- src        C++ツールのソース
    |    |- include    C++ツールのヘッダファイル
    |    |- bin        C++ツールのインストール先
    |    |- perl       Perlツール
    |    +- make       Makefile
    |
    |- src-data
    |    |- stem       英語ステマーのソース
    |    |- norm       正規化辞書/異表記展開辞書のソース
    |    +- una        形態素解析辞書のソース
    |
    +- (work)          辞書ビルド用の作業ディレクトリ

■必要なファイルの取得

UNA辞書のビルドには以下のファイルが必要ですが、自由な再配布が
許可されていないため、辞書ビルド環境には同梱されていません。
各自でUnicode.orgから取得し、src-data/normの下に配置してください。

  https://www.unicode.org/Public/1.1-Update/UnicodeData-1.1.5.txt
  https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0201.TXT
  https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0208.TXT
  https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0212.TXT

■unadic(標準辞書, 単一辞書)のビルド

以下の手順を実施します。
ビルドした辞書をDoqueDBから参照するには、後述する
「■unadicのパッケージ作成」を実施してください。

$ mkdir work
$ cd work
$ make -f ../tools/make/make-tools install
$ export LD_LIBRARY_PATH=`pwd`/../tools/bin:$LD_LIBRARY_PATH
$ make -f ../tools/make/make-stem install
$ make -f ../tools/make/make-norm install
$ make -f ../tools/make/make-una install

これでカレントディレクトリにunadicが作られました。
unadicの形態素解析辞書とビルド時の中間ファイルを使って
解析結果とデバッグ情報(-a指定時のみ)を表示してみます。
デバッグ情報の作成には時間がかかります。

$ cat > test.txt
今日もよい１日でしたね。
^D
$ perl ../tools/perl/dounamk -r unadic/una -w uwork test.txt
$ cat test.txt.ana
00001."今日もよい１日でしたね。"
   今日(名詞.副詞;～数詞)
   も(副助詞.モ)
   よい(形容詞.終止連体,ナイ)
   １(数詞)
   日(名詞.接尾辞;数量～)
   でし(助動詞.デス.連用)
   た(助動詞.タ.終止連体)
   ね(終助詞;助動そう～;助詞て～;名詞類～;終助詞～;言い止め＝)
   。(記号.句点)

$ perl ../tools/perl/dounamk -r unadic/una -w uwork -a test.txt
$ cat test.txt.dbg

00001."今日もよい１日でしたね。"

`' [0] +0(0)+0=0 0:0:0:記号.句点
`今' [1] +48(O)+52=100 0:60269:1:名詞.副詞
  `日' [5] +62(h)+5=167 0:120517:1:名詞.接尾辞;数量～
`今' [2] +49(F)+63=112 0:60270:1:接頭辞.一般
`今' [3] +44(O)+111=155 0:60271:1:名詞.固有
`今日' [4] +48(O)+76=124 0:60347:1:名詞.副詞;～数詞
    `も' [6] +58(s)+0=182 0:34790:1:副助詞.モ
---
`' [0] +0(0)+0=0 0:0:0:副助詞.モ
`よ' [1] +196(q)+0=196 0:37111:1:終助詞;助動そう～;助詞て～;名詞類～;命令～
`よ' [2] +65535(X)+7=65542 0:37112:1:補助形容詞.語幹,ヨイ
`よ' [3] +91(O)+23=114 0:37113:1:形容詞.語幹,ナイ
  `い' [7] +65535(X)+72=65721 0:1911:1:動詞.連用,一段
  `い' [8] +65535(X)+53=65702 0:1912:1:補助動詞.連用,一段
`よ' [4] +65535(X)+0=65535 0:37114:1:助動詞.ヨウ.ウ接続
`よい' [5] +65535(X)+7=65542 0:37115:1:補助形容詞.終止連体,ヨイ
`よい' [6] +91(O)+23=114 0:37116:1:形容詞.終止連体,ナイ
    `１' [9] +182(Q)+25=321 2:3:1:数詞
---
`' [0] +0(0)+0=0 0:0:0:数詞
`日' [1] +46(b)+5=51 0:120517:1:名詞.接尾辞;数量～
---
`' [0] +0(0)+0=0 0:0:0:名詞.接尾辞;数量～
`で' [1] +41(t)+26=67 0:22702:1:助動詞.ダ.連用
`で' [2] +40(s)+2=42 0:22703:1:格助詞.デ
  `し' [8] +65535(X)+0=65577 0:14275:1:接続助詞.シ
  `し' [9] +60(Y)+7=109 0:14276:1:動詞.未然連用,サ変;体言動詞化＝
    `た' [13] +78(y)+7=194 0:18512:1:助動詞.タイ.語幹
  `したね' [11] +230(Q)+544=816 2:2:1:未登録語.一般
`で' [3] +65535(X)+47=65582 0:22704:1:接続助詞.テ,濁音
`で' [4] +177(Q)+73=250 0:22705:1:動詞.連用,一段
  `し' [10] +147(q)+0=397 0:14277:1:補助動詞.未然連用,サ変;サ変＝
`で' [5] +65535(X)+30=65565 0:22706:1:補助動詞デ接続.連用,一段
`でし' [6] +71(t)+0=71 0:22848:1:助動詞.デス.連用
    `た' [12] +13(r)+0=84 0:18511:1:助動詞.タ.終止連体
      `ね' [15] +114(r)+0=198 0:26655:1:終助詞;助動そう～;助詞て～;名詞類～;終助詞～;言い止め＝
        `。' [17] +15(k)+0=213 0:154:1:記号.句点
          `' [18] +126(q)+0=339 0:0:1:記号.句点
      `ね' [16] +65535(X)+5=65624 0:26656:1:助動詞.ズ.仮定
    `たね' [14] +65535(X)+73=65679 0:19309:1:名詞.接尾辞
`でしたね' [7] +229(Q)+640=869 2:2:1:未登録語.一般
---
$

■複数辞書のビルド

UNAでは複数の辞書を使った形態素解析が可能です。
複数の辞書は同時にビルドする必要がありますが、
辞書リストに辞書名を記述することにより、
実行時にどの辞書を使うかを選択することができます。
また、各辞書には優先順位を設定でき、
同表記同品詞の語があれば、順位の高い辞書の語が選択されます。
形態素解析辞書ソースには、サンプルとして以下の
拡張辞書ソースと辞書リストデータが用意されています。
  sample-ext1.dic：ひょっこりひょうたん島人物名ほか
  sample-ext2.dic：表記「サンプル」をもつ各品詞の語
  sample-diclist.dat：拡張辞書を含む辞書リスト

unadicを含む複数辞書のビルド手順を以下に示します。

$ (unadicのビルド手順をmake-norm installまで進める)
$ cp -rp unadic unadic-multi
$ make -f ../tools/make/make-una-multi install

これでカレントディレクトリにunadic-multiが作られました。
unadic-multiを使った解析結果は次のようになります。

$ cat > test-ext.txt
サンプらない、サンプります、サンプる。
ドン・ガバチョがサンプくて思いサンプった。
サンプルことがあるサンプルなトラヒゲ。
サンプルサンプルサンプルだ。
^D
$ perl ../tools/perl/dounamk -r unadic-multi/una -w uwork-multi test-ext.txt
$ cat test-ext.txt.ana
00001."サンプらない、サンプります、サンプる。"
   サンプら(動詞.未然,ラ行五段)
   ない(助動詞.ナイ.終止連体)
   、(記号.読点)
   サンプり(動詞.連用,ラ行五段)
   ます(助動詞.マス.終止連体)
   、(記号.読点)
   サンプる(動詞.終止連体,ラ行五段)
   。(記号.句点)

00002."ドン・ガバチョがサンプくて思いサンプった。"
   ドン・ガバチョ(名詞.固有)
   が(格助詞.ガ)
   サンプく(形容詞.未然連用)
   て(接続助詞.テ)
   思い(動詞.連用,ワ行五段)
   サンプっ(補助動詞.音便,ラ行五段)
   た(助動詞.タ.終止連体)
   。(記号.句点)

00003."サンプルことがあるサンプルなトラヒゲ。"
   サンプル(連体詞)
   こと(名詞.形式;詠嘆～)
   が(格助詞.ガ)
   ある(動詞.終止連体,ラ行五段;体言動詞化＝)
   サンプル(形容動詞.一般)
   な(助動詞.ダ.連体)
   トラヒゲ(名詞.固有)
   。(記号.句点)

00004."サンプルサンプルサンプルだ。"
   サンプル(接頭辞.一般)
   サンプル(名詞.一般)
   サンプル(接尾辞)
   だ(助動詞.ダ.終止)
   。(記号.句点)

$

■unadicのパッケージ作成

DoqueDBはビルド時にパッケージディレクトリのunadicを参照します。
ディレクトリはcommon/lib/<OSタイプ>/una/<UNAバージョン>/data/です。
unadicをパッケージディレクトリに配置するには、make-stem、
make-norm、make-unaの実行後に、引き続き以下を実施してください。

$ make -f ../tools/make/make-una package

以上
