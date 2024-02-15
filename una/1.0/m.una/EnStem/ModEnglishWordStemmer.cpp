// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModEnglishWordStemmer.cpp -- 英単語正規化器の実装
// 
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#include "ModCharString.h"
#include "ModParameter.h"
#include "EnStem/Module.h"
#include "EnStem/ModEnglishWordStemmer.h"

_UNA_USING
_UNA_ENSTEM_USING

//
// FUNCTION public
// ModEnglishWordStemmer::~ModEnglishWordStemmer -- デストラクタ
//
// NOTES
//	ModEnglishWordStemmerのデストラクタ
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
ModEnglishWordStemmer::~ModEnglishWordStemmer()
{
	delete [] dictKey;
	delete [] dictExpand;
	delete [] ruleKey;
	delete [] ruleStem;
	delete [] dictIndex;
	delete [] ruleIndex;
}


//
// FUNCTION
//	ModEnglishWordStemmer::searchDict -- 辞書検索
//
// NOTES
//	辞書を完全一致で検索する。検索アルゴリズムは二分探索。
//
// ARGUMENTS
//	const ModUnicodeString& target
//		検索対象文字列
//
// RETURN
//	DictIndex*
//		対象文字列と完全一致する辞書見出しがある場合は、対応する辞書イ
//		ンデックスのポインタを返す。そうでない場合は 0 を返す。
//
// EXCEPTIONS
//	なし
//

#if defined(CC_SUN4_2)
DictIndex*
#else
ModEnglishWordStemmer::DictIndex*
#endif
ModEnglishWordStemmer::searchDict(const ModUnicodeString& target)
{
	// 見出し先頭文字による検索範囲の限定
    int	count = target[0] - FirstChar;
    int	head  = dictInit[count].headIndex;
    int	tail  = dictInit[count].tailIndex;

	// 先頭文字に対応する見出しがなければ 0 を返す
    if (head < 0) {
#ifdef STEM_DEBUG
		ModDebugMessage << "searchDict: no keys starting with '"
						<< target[0] << "'" << ModEndl;
#endif
		return 0;
    }

	// 検索範囲
    DictIndex*	first 	= dictIndex + head;
    DictIndex*	last  	= dictIndex + tail;

	while (1) {
		int			half	= (int)((last - first) / 2);
		DictIndex*	middle	= first + half;
		char*		key		= dictKey + middle->dictKeyOffset;
		int			cmp		= compare((const ModUnicodeChar*)target,
									  key);
#ifdef STEM_DEBUG
		ModDebugMessage << "searchDict: "
						<< first - dictIndex << " - "
						<< last - dictIndex  << ": "
						<< cmp << " \"" << target << "\" with \""
						<< key << "\" (" << middle - dictIndex << ")"
						<< ModEndl;
#endif

		if (cmp == 0) {			// 完全一致
#ifdef STEM_DEBUG
            ModDebugMessage << "searchDict: matched!" << ModEndl;
#endif
			return middle;	// 比較対象位置を返す
		}
		if (cmp > 0) {		// 対象文字列が大きい
			first = middle + 1;
		} else	{			// 対象文字列が小さい
			last = middle - 1;
		}
		if (first > last) {	// 検索範囲が逆転
			return 0;	// 0 を返す
		}
	}
}


//
// FUNCTION
//	ModEnglishWordStemmer::searchRule -- 規則検索
//
// NOTES
//	規則を語末からの最長一致で検索する。ただし語頭の１文字は検索対象外
//	とする。検索アルゴリズムは二分探索。
//
// ARGUMENTS
//	const ModUnicodeString& target
//		検索対象文字列
//
// RETURN
//	RuleIndex*
//		対象文字列の先頭１文字を除いて、末尾から最長一致する見出しに
//		対応する規則インデックスのポインタを返す。
// 		一致する見出しがない場合は 0 を返す。
//
// EXCEPTIONS
//	なし
//
#if defined(CC_SUN4_2)
RuleIndex*
#else
ModEnglishWordStemmer::RuleIndex*
#endif
ModEnglishWordStemmer::searchRule(const ModUnicodeString& target)
{
	// 見出し先頭文字による検索範囲の限定
    int length	= target.getLength() - 1;	// 先頭１文字分を差し引く
    int count	= target[length] - FirstChar;
	int head	= ruleInit[count].headIndex;
	int tail	= ruleInit[count].tailIndex;

	// 末尾文字に対応する規則がなければ 0 を返す
    if (head < 0) {
#ifdef STEM_DEBUG
		ModDebugMessage << "searchRule: no keys starting with '"
						<< target[length] << "'" << ModEndl;
#endif
		return 0;
    }

	// 対象文字列が見出し長最小値より短い場合は 0 を返す
    int min_len	= ruleInit[count].minKeyLen;
	if (length < min_len) {
#ifdef STEM_DEBUG
		ModDebugMessage << "searchRule: \"" << target
						<< "\" shorter than " << min_len + 1
						<< ModEndl;
#endif
		return 0;
	}

	// 検索範囲
    RuleIndex* first = ruleIndex + head;
    RuleIndex* last  = ruleIndex + tail;

	// 対象文字列の先頭１文字を除いて裏返す
    ModUnicodeChar* reversed = new ModUnicodeChar[length + 1];
    ModUnicodeChar* dummy	 = reversed;
	int				i 		 = 0;
    for (; i < length; dummy ++, i ++) {
        *dummy = target[length - i];
    }
    *dummy = ModUnicodeCharTrait::null();

	// 対象文字列が先頭の見出しより小さい
	if (compare(reversed, ruleKey + first->ruleKeyOffset) < 0) {
#ifdef STEM_DEBUG
        ModDebugMessage << "searchRule: \"" << reversed
						<< "\" smaller than initial key" << ModEndl;
#endif
		delete [] reversed;
        return 0;
	}

	while (1) {
		int 			 half	= (int)((last - first) / 2);
		RuleIndex* 		 middle	= first + half;
		ModUnicodeChar*	 key	= ruleKey + middle->ruleKeyOffset;
		int 			 cmp	= compare(reversed, key);
#ifdef STEM_DEBUG
		ModDebugMessage << "searchRule: "
						<< first - ruleIndex << " - "
						<< last - ruleIndex	 << ": "
						<< cmp << " \"" << reversed << "\" with \""
						<< key << "\" (" << middle - ruleIndex << ")"
						<< ModEndl;
#endif

		if (cmp == 0) {		// 完全一致
#ifdef STEM_DEBUG
			ModDebugMessage << "searchRule: matched!" << ModEndl;
#endif
			delete [] reversed;
			return middle;	// 比較対象位置を返す
		}
		if (cmp > 0) {		// 対象文字列が大きい
			first	= middle + 1;
		} else {			// 対象文字列が小さい
            last = middle - 1;
		}

		if (first > last) {	// 検索範囲が逆転
			// 最後に比較した見出しが対象文字列より大きい
			if (cmp < 0) {
				// １つ前に戻る
				if (middle > ruleIndex + head) -- middle;
				key = ruleKey + middle->ruleKeyOffset;
#ifdef STEM_DEBUG
				ModDebugMessage << "searchRule: comparing with \""
								<< key << "\" ("
								<< middle - ruleIndex << ")" << ModEndl;
#endif
			}
			// 対象文字列と見出しの先頭からの共通文字数を求める
			ModUnicodeChar* l 	 = reversed;
			ModUnicodeChar* r 	 = key;
			int 			comm = 0;
			while (*l != ModUnicodeCharTrait::null() &&
				   *r != ModUnicodeCharTrait::null() &&
				   *l == *r) {
				++ l;
				++ r;
				++ comm;
			}

			// 見出し長最小値より短い
			if (comm < min_len) {
#ifdef STEM_DEBUG
				ModDebugMessage << "searchRule: target shorter than "
								<< min_len << ModEndl;
#endif
				delete [] reversed;
				return 0;	// 0を返す
			}
			// 見出し全体と共通ならマッチしたことになる
			if (comm == ModUnicodeCharTrait::length(key)) {
#ifdef STEM_DEBUG
				ModDebugMessage << "searchRule: matched!" << ModEndl;
#endif
				delete [] reversed;
				return middle;	// 比較対象位置を返す
			}
			// 先頭見出しなら終了
			if (middle == ruleIndex + head) {
#ifdef STEM_DEBUG
				ModDebugMessage << "searchRule: "
								<< "target not matched with initial key"
								<< ModEndl;
#endif
				delete [] reversed;
                return 0;	// 0を返す
			}
			// 対象文字列と検索範囲を新たに設定する
			reversed[comm]  = ModUnicodeCharTrait::null();
			first			= ruleIndex + head;
			last			= middle;
		}
	}
}

//
// FUNCTION
//	ModEnglishWordStemmer::compare -- 文字列比較関数
//
// NOTES
// ASCII 文字列の比較。Unicode 文字列と char 文字列が相互に比較できる
// ように、ModUnicodeCharTrait::compare(), ModCharTrait::compare() を
// 模して作成。同種の文字列同士の比較も可能である。
//
// ARGUMENTS
//	ModUnicodeChar* l / char* l
//		r と比較する ASCII 文字列
//	ModUnicodeChar* r / char* l
//		l と比較する ASCII 文字列
//
// RETURN
//	1
//		最初に異なる文字の ASCII コードが、r より l のものの方が大きい。
//		または r が l の先頭からの部分文字列である。
//	0
//		l と r のすべての文字の ASCII コードが等しい。
//	-1
//		最初に異なる文字の ASCII コードが、l より r のものの方が大きい。
//		または l が r の先頭からの部分文字列である。
//
// EXCEPTIONS
//	なし
//
int
ModEnglishWordStemmer::compare(const ModUnicodeChar* l, const ModUnicodeChar* r)
{
	if (l == r)
		return 0;

    // 両文字列の先頭から１文字ずつ比較
    while (1) {
		if (*l == ModUnicodeCharTrait::null()) {
			// l が終了
			if (*r == ModUnicodeCharTrait::null()) {
				// r も終了なら同一文字列
				return 0;
			}
			return -1;
		}
		if (*r == ModUnicodeCharTrait::null()) {
			// r が終了
			return 1;
		}
		if (*l != *r) {
			// 最初に異なった文字の比較結果で返り値を決める
			return (*l < *r) ? -1 : 1;
		}
		++ l;
		++ r;
	}
}

int
ModEnglishWordStemmer::compare(const ModUnicodeChar* l, const char* r)
{
    // 両文字列の先頭から１文字ずつ比較
    while (1) {
		if (*l == ModUnicodeCharTrait::null()) {
            // l が終了
			if (*r == ModCharTrait::null()) {
				// r も終了なら同一文字列
				return 0;
			}
			return -1;
		}
		if (*r == ModCharTrait::null()) {
            // r が終了
			return 1;
		}
        if (*l != *r) {
            // 最初に異なった文字の比較結果で返り値を決める
            return (*l < *r) ? -1 : 1;
        }
        ++ l;
        ++ r;
	}
}

#ifdef OBSOLETE
int
ModEnglishWordStemmer::compare(const char* l, const ModUnicodeChar* r)
{
    // 両文字列の先頭から１文字ずつ比較
	while (1) {
		if (*l == ModCharTrait::null()) {
            // l が終了
			if (*r == ModUnicodeCharTrait::null()) {
                // r も終了なら同一文字列
				return 0;
			}
			return -1;
		}
		if (*r == ModUnicodeCharTrait::null()) {
            // r が終了
			return 1;
		}
        if (*l != *r) {
            // 最初に異なった文字の比較結果で返り値を決める
            return (*l < *r) ? -1 : 1;
        }
        ++ l;
        ++ r;
    }
}


int
ModEnglishWordStemmer::compare(const char* l, const char* r)
{
	if (l == r)
		return 0;

    // 両文字列の先頭から１文字ずつ比較
    while (1) {
		if (*l == ModCharTrait::null()) {
            // l が終了
			if (*r == ModCharTrait::null()) {
                // r も終了なら同一文字列
				return 0;
			}
			return -1;
		}
		if (*r == ModCharTrait::null()) {
            // r が終了
			return 1;
		}
        if (*l != *r) {

            // 最初に異なった文字の比較結果で返り値を決める
            return (*l < *r) ? -1 : 1;
        }
        ++ l;
        ++ r;
    }
}
#endif

//
// FUNCTION private
//	ModEnglishWordStemmer::verifyTarget -- 対象文字列のチェック
//
// NOTES
//	引数の文字列が処理対象文字列の条件を満たすかチェックする。
//
// ARGUMENTS
//	const ModUnicodeString& t
//		正規化処理の対象文字列
//
// RETURN
// 	ModBoolean
//		引数である文字列が空文字列の場合、または、ASCIIアルファベット
//		小文字以外の文字を含む場合は ModFalse を、そうでなければ
//		ModTrue を返す。
//
// EXCEPTIONS
//	なし
//
ModBoolean
ModEnglishWordStemmer::verifyTarget(const ModUnicodeString& t)
{

	// 空文字列
    if (t.getLength() == 0) {
		return ModFalse;
    }
    ModSize i = 0;
    for (; i < t.getLength(); i ++) {

		// ASCIIアルファベット小文字以外の文字を含む
		if (t[i] < FirstChar || t[i] > LastChar) {
            return ModFalse;
        }
    }
    return ModTrue;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::breakExpand -- 展開語形の分解
//
// NOTES
//	展開語形文字列をフィールド区切り毎に分解し、１語ずつベクターに格納
//	する。
//
// ARGUMENTS
//	char* string
//		対象文字列
//	ModVector<ModUnicodeString>& vector
//		分解結果を格納するベクター
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
void
ModEnglishWordStemmer::breakExpand(char* string,
										   ModVector<ModUnicodeString>& vector)
{

	// 切り出し対象文字列の先頭・末尾位置をセットする
    char* head = string;
    char* tail = string;

	// フィールド区切りか文字列末尾に達するまで末尾位置をシフトする
    while (*tail != ModCharTrait::null()) {
        if (*tail == SepField) {
            ModUnicodeString buffer(head, (ModSize)(tail - head),
									ModKanjiCode::euc);
            vector.pushBack(buffer);

			// フィールド区切りなら先頭位置を次の文字にセットする
            head = tail + 1;
        }
		// 末尾位置を次の文字にセットする（文字列末尾なら null になる）
        ++ tail;
    }
	ModUnicodeString buffer(head, (ModSize)(tail - head),
							ModKanjiCode::euc);
    vector.pushBack(buffer);
	return;
}

//
// ---------------------------------------------
// ModEnglishWordStemmerDataPath関数定義
// ---------------------------------------------

//
// FUNCTION public
// ModEnglishWordStemmerDataPath::ModEnglishWordStemmerDataPath -- コンストラクタ
//
// NOTES
//	ModEnglishWordStemmerDataPath のコンストラクタ
//
// ARGUMENTS
//	const ModCharString& filename
//		ソースファイルへのパスを、以下の順序で１行ずつ記述したファイル
//		へのパス
//			1. 辞書見出し語形ファイル
//			2. 辞書展開語形ファイル
//			3. 辞書インデックスファイル
//			4. 規則見出し語形ファイル
//			5. 規則正規化語形ファイル
//			6. 規則インデックスファイル
//
// RETURN
//	なし
//
// EXCEPTIONS
//	下位からの例外をそのまま返す
//
ModEnglishWordStemmerDataPath::ModEnglishWordStemmerDataPath(
	const ModUnicodeString& filename)
{
    try {
#if defined(V1_6)
        ModFile file(filename, ModFile::readMode);
#else
        ModUnicodeString a(filename);
        ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        ModFile file( b, ModFile::readMode);
#endif
        ModArchive archiver(file, ModArchive::ModeLoadArchive);
		archiver >> *this;
		file.close();
    } catch (ModException& exception) {
		ModErrorMessage << exception << ModEndl;
		ModRethrow(exception);
    }
}

ModEnglishWordStemmerDataPath::ModEnglishWordStemmerDataPath(
	const ModCharString& filename)
{
	ModUnicodeString filenameU(filename.getString());
    try {
#if defined(V1_6)
        ModFile file(filenameU, ModFile::readMode);
#else
        ModUnicodeString a(filenameU);
        ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        ModFile file( b, ModFile::readMode);
#endif
        ModArchive archiver(file, ModArchive::ModeLoadArchive);
		archiver >> *this;
		file.close();
    } catch (ModException& exception) {
#ifdef DEBUG
		ModErrorMessage << exception << ModEndl;
#endif
		ModRethrow(exception);
    }
}

//
// FUNCTION public
//	ModEnglishWordStemmerDataPath::serialize -- シリアライズ
//
// NOTES
//	ModEnglishWordStemmerDataPath のシリアライザ。ロードモード
//	のみを設ける。以下の順でソースファイルへのパスを１行ずつ記述したファ
//	イルを読み込むことができる。
//		1. 辞書見出し語形ファイル
//		2. 辞書展開語形ファイル
//		3. 辞書インデックスファイル
//		4. 規則見出し語形ファイル
//		5. 規則正規化語形ファイル
//		6. 規則インデックスファイル
//
// ARGUMENTS
//	ModArchive& archiver
//		アーカイバ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		指定されたファイル数が規定通り(=6)でない場合は上記例外を返す。
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmerDataPath::serialize(ModArchive& archiver)
{
	ModSize num_file = 0;	// ファイル数
    try {
        if (archiver.isLoad() == ModTrue) {
			ModUnicodeString buffer;		// 文字列バッファ
			char current;				// 入力ストリーム格納用
			while (1) {
				archiver >> current;	// １文字ずつ読み込む
				if (num_file == 6) {
					// ファイル数が規定より多い
					ModErrorMessage << "serialize: too many files"
									<< ModEndl;
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				if (current == '\n') {	// 改行
					switch (num_file) {
					case 0:		// 辞書見出し語形ファイル
						dictKeyPath = buffer;
						break;
					case 1:		// 辞書展開語形ファイル
						dictExpandPath = buffer;
						break;
					case 2:		// 辞書インデックスファイル
						dictIndexPath = buffer;
						break;
					case 3:		// 規則見出し語形ファイル
						ruleKeyPath = buffer;
						break;
					case 4:		// 規則正規化語形ファイル
						ruleStemPath = buffer;
						break;
					case 5:		// 規則インデックスファイル
						ruleIndexPath = buffer;
						break;
					default:
						break;
					}
					buffer.clear();		// バッファをクリア
					++ num_file;		// ファイル数をインクリメント
				} else {
					buffer += current;	// バッファに文字をアペンド
				}
			}
		}
    } catch (ModException& exception) {
		// ファイル末尾
        if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// ファイル数が規定数と異なる
			if (num_file != 6) {
				ModErrorMessage << "serialize: not enough files"
								<< ModEndl;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
            ModErrorHandle::reset();
		} else {
			ModErrorMessage << "serialize: " << exception << ModEndl;
			ModRethrow(exception);
		}
    }
    return;
}


//
// -------------------------------------
// ModEnglishWordStemmer定数定義
// -------------------------------------

//
// CONST private
//	ModEnglishWordStemmer::SepRecord -- レコード区切り文字
//
// NOTES
//	辞書・規則データのソースファイルにおけるレコード区切りとして用いら
//	れる文字。ここでは改行。
//
const char ModEnglishWordStemmer::SepRecord('\n');


//
// CONST private
//	ModEnglishWordStemmer::SepField -- フィールド区切り文字
//
// NOTES
//	辞書・規則データのソースファイルにおけるフィールド区切りとして用い
//	られる文字。ここではスペース。
//
const char ModEnglishWordStemmer::SepField(' ');


//
// CONST private
//	ModEnglishWordStemmer::FirstChar -- アルファベット先頭文字
//
// NOTES
//	ASCII小文字アルファベットの先頭文字 'a'。
//
const char ModEnglishWordStemmer::FirstChar('a');


//
// CONST private
//	ModEnglishWordStemmer::LastChar -- アルファベット末尾文字
//
// NOTES
//	ASCII小文字アルファベットの先頭文字 'z'。
//
const char ModEnglishWordStemmer::LastChar('z');


//
// CONST private
//	ModEnglishWordStemmer::DataVerifier -- データ認識文字列
//
// NOTES
//	辞書、規則データの実行形ファイルを認識するための文字列。アーカイブ
//	ファイルの先頭と末尾に書き込まれる。
//
const char* ModEnglishWordStemmer::DataVerifier
= "This file includes archived data for ModInvertedEnglishWordStemmer!";


//
// -------------------------------------
// ModEnglishWordStemmer関数定義
// -------------------------------------

//
// FUNCTION public
//	ModEnglishWordStemmer::ModEnglishWordStemmer -- コンストラクタ１
//
// NOTES
//	ModEnglishWordStemmerのコンストラクタ。ソースデータから
//	実行形データを作成し、ファイルに書き出す。
//
// ARGUMENTS
//	const ModEnglishWordStemmerDataPath& path
//		ソースファイルパス構造体
//	const ModCharString& filename
//		実行形データ書き出し用ファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		指定されたファイルが空の場合は上記例外を返す。
//	下位からの例外はそのまま返す。
//
ModEnglishWordStemmer::ModEnglishWordStemmer(
	const ModEnglishWordStemmerDataPath& path,
	const ModCharString& filename)
    :
	dictKey(0), dictExpand(0), ruleKey(0), ruleStem(0),
	dictKeyLen(0), dictExpandLen(0), ruleKeyLen(0), ruleStemLen(0),
	dictKeyNum(0), ruleKeyNum(0), dictIndex(0), ruleIndex(0)
{
	setResource(path, ModUnicodeString(filename));
}

ModEnglishWordStemmer::ModEnglishWordStemmer(
	const ModEnglishWordStemmerDataPath& path,
	const ModUnicodeString& filename)
    :
	dictKey(0), dictExpand(0), ruleKey(0), ruleStem(0),
	dictKeyLen(0), dictExpandLen(0), ruleKeyLen(0), ruleStemLen(0),
	dictKeyNum(0), ruleKeyNum(0), dictIndex(0), ruleIndex(0)
{
	setResource(path, filename);
}
void
ModEnglishWordStemmer::setResource(
	const ModEnglishWordStemmerDataPath& path,
	const ModUnicodeString& filename)
{
	try {
		{
			// Dictionary entry word shape
#if defined(V1_6)
			ModFile file(path.dictKeyPath, ModFile::readMode);
#else
			ModUnicodeString a(path.dictKeyPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
			dictKeyLen = (int)(file.getFileSize() - 1);
            if (dictKeyLen < 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.dictKeyPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setDictKey(archiver);
			file.close();
		}
		{
			// 辞書展開語形
#if defined(V1_6)
			ModFile file(path.dictExpandPath, ModFile::readMode);
#else
			ModUnicodeString a(path.dictExpandPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
			dictExpandLen = (int)(file.getFileSize() - 1);
            if (dictExpandLen < 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.dictExpandPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setDictExpand(archiver);
			file.close();
		}
		{
			// 規則見出し語形
#if defined(V1_6)
			ModFile file(path.ruleKeyPath, ModFile::readMode);
#else
			ModUnicodeString a(path.ruleKeyPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
			ruleKeyLen = (int)(file.getFileSize() - 1);
            if (ruleKeyLen < 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.ruleKeyPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setRuleKey(archiver);
			file.close();
		}
		{
			// 規則正規化語形
#if defined(V1_6)
			ModFile file(path.ruleStemPath, ModFile::readMode);
#else
			ModUnicodeString a(path.ruleStemPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
			ruleStemLen = (int)(file.getFileSize() - 1);
            if (ruleStemLen < 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.ruleStemPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setRuleStem(archiver);
			file.close();
		}
		{
			// 辞書インデックス
#if defined(V1_6)
			ModFile file(path.dictIndexPath, ModFile::readMode);
#else
			ModUnicodeString a(path.dictIndexPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
            if (file.getFileSize() == 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.dictIndexPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setDictIndex(archiver);
			file.close();
		}
		{
			// 規則インデックス
#if defined(V1_6)
			ModFile file(path.ruleIndexPath, ModFile::readMode);
#else
			ModUnicodeString a(path.ruleIndexPath);
        		ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        		ModFile file( b, ModFile::readMode);
#endif
            if (file.getFileSize() == 0) {
#ifdef DEBUG
                ModErrorMessage << "empty file: "
								<< path.ruleIndexPath << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
            }
			ModArchive archiver(file, ModArchive::ModeLoadArchive);
			setRuleIndex(archiver);
			file.close();
		}

		// ファイルを write モードでオープンする
#if defined(V1_6)
		ModFile file(filename, ModFile::writeMode);
#else
		ModUnicodeString a(filename);
        	ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        	ModFile file( b, ModFile::writeMode);
#endif
		ModArchive archiver(file, ModArchive::ModeStoreArchive);
		archiver << *this;
		file.close();

	} catch (ModException& exception) {
		delete [] dictKey;
		delete [] dictExpand;
		delete [] ruleKey;
		delete [] ruleStem;
		delete [] dictIndex;
		delete [] ruleIndex;
#ifdef DEBUG
		ModErrorMessage << exception << ModEndl;
#endif
		ModRethrow(exception);
	}
}


//
// FUNCTION public
//	ModEnglishWordStemmer::ModEnglishWordStemmer
//	-- コンストラクタ２
//
// NOTES
//	ModEnglishWordStemmerのコンストラクタ。実行形データを
//	ファイルから読み込む。
//
// ARGUMENTS
//	const ModCharString& filename
//
// RETURN
//	なし
//
// EXCEPTIONS
//	下位からの例外をそのまま返す。
//
ModEnglishWordStemmer::ModEnglishWordStemmer(
	const ModUnicodeString& filename)
    :
	dictKey(0), dictExpand(0), ruleKey(0), ruleStem(0),
	dictKeyLen(0), dictExpandLen(0), ruleKeyLen(0), ruleStemLen(0),
	dictKeyNum(0), ruleKeyNum(0), dictIndex(0), ruleIndex(0)
{
	try {
		// ファイルを read モードでオープンする
#if defined(V1_6)
		ModFile file(filename, ModFile::readMode);
#else
		ModUnicodeString a(filename);
        	ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        	ModFile file( b, ModFile::readMode);
#endif
		ModArchive archiver(file, ModArchive::ModeLoadArchive);
		archiver >> *this;
		file.close();	// ファイルをクローズ
	} catch (ModException& exception) {
		delete [] dictKey;
		delete [] dictExpand;
		delete [] ruleKey;
		delete [] ruleStem;
		delete [] dictIndex;
		delete [] ruleIndex;
		ModRethrow(exception);
	}
}
ModEnglishWordStemmer::ModEnglishWordStemmer(
	const ModCharString& filename)
    :
	dictKey(0), dictExpand(0), ruleKey(0), ruleStem(0),
	dictKeyLen(0), dictExpandLen(0), ruleKeyLen(0), ruleStemLen(0),
	dictKeyNum(0), ruleKeyNum(0), dictIndex(0), ruleIndex(0)
{
	ModUnicodeString filenameU(filename);
	try {
		// ファイルを read モードでオープンする
#if defined(V1_6)
		ModFile file(filenameU, ModFile::readMode);
#else
		ModUnicodeString a(filenameU);
        	ModCharString b(a.getString(ModOs::Process::getEncodingType()));
        	ModFile file( b, ModFile::readMode);
#endif
		ModArchive archiver(file, ModArchive::ModeLoadArchive);
		archiver >> *this;
		file.close();	// ファイルをクローズ
	} catch (ModException& exception) {
		delete [] dictKey;
		delete [] dictExpand;
		delete [] ruleKey;
		delete [] ruleStem;
		delete [] dictIndex;
		delete [] ruleIndex;
#ifdef DEBUG
		ModErrorMessage << exception << ModEndl;
#endif
		ModRethrow(exception);
	}
}

#ifdef OBSOLETE
ModEnglishWordStemmer::ModEnglishWordStemmer()
    :
	dictKey(0), dictExpand(0), ruleKey(0), ruleStem(0),
	dictKeyLen(0), dictExpandLen(0), ruleKeyLen(0), ruleStemLen(0),
	dictKeyNum(0), ruleKeyNum(0), dictIndex(0), ruleIndex(0)
{
	ModParameter p;
	ModUnicodeString filename;
#if defined(V1_6)
	if (p.getUnicodeString(filename, "StemmerDefaultResourcePath") == ModFalse) {
		filename = "stemmer.dat";
	}
#else
	ModCharString cFilename;
	if (p.getString(cFilename, "StemmerDefaultResourcePath") == ModFalse) {
		cFilename = "stemmer.dat";
	}
	filename = cFilename.getString();
#endif
	try {
		// ファイルを read モードでオープンする
#if defined(V1_6)
		ModFile file(filename, ModFile::readMode);
#else
        	ModCharString b(filename.getString(ModOs::Process::getEncodingType()));
        	ModFile file( b, ModFile::readMode);
#endif
		ModArchive archiver(file, ModArchive::ModeLoadArchive);
		archiver >> *this;
		file.close();	// ファイルをクローズ
	} catch (ModException& exception) {
		delete [] dictKey;
		delete [] dictExpand;
		delete [] ruleKey;
		delete [] ruleStem;
		delete [] dictIndex;
		delete [] ruleIndex;
		ModErrorMessage << exception << ModEndl;
		ModRethrow(exception);
	}
}
#endif

//
// FUNCTION public
//	ModEnglishWordStemmer::look -- 辞書引き
//
// NOTES
//	対象文字列が辞書に登録されているかをチェックする。
//
// ARGUMENTS
// 	const ModUnicodeString& target
//		対象文字列
//
// RETURN
//	ModBoolean
//		対象文字列が辞書登録されている場合は ModTrue を、そうでない場
//		合は ModFalse を返す。
//
// EXCEPTIONS
//	下位からの例外をそのまま返す。
//
ModBoolean
ModEnglishWordStemmer::look(const ModUnicodeString& target)
{
    try{
		// 対象外文字列の場合、または、マッチする見出しがなければ
		// ModFalse を返す
		if (verifyTarget(target) == ModFalse) {
#ifdef STEM_DEBUG
			ModDebugMessage << "look: invalid target" << ModEndl;
#endif
            return ModFalse;
		}
		if (searchDict(target) == 0) {
			return ModFalse;
		}
		return ModTrue;
    } catch (ModException& exception) {
#ifdef STEM_DEBUG
		ModDebugMessage << "look: " << exception << ModEndl;
#endif
#ifdef DEBUG
		ModErrorMessage << "look: " << exception << ModEndl;
#endif
        ModRethrow(exception);
    }
}


//
// FUNCTION public
//	ModEnglishWordStemmer::stem -- 正規化
//
// NOTES
//	対象文字列を正規化する。
//
// ARGUMENTS
//	const ModUnicodeString& target
//		対象文字列
//	ModUnicodeString& result
//		結果格納用文字列
//
// RETURN
//	ModBoolean
//		対象文字列と異なる正規化文字列が得られた場合は ModTrue を、そ
//		うでない場合は ModFalse を返す。
//
// EXCEPTIONS
//	下位からの例外をそのまま返す。
//
ModBoolean
ModEnglishWordStemmer::stem(const ModUnicodeString& target,
									ModUnicodeString& result)
{
    try{
		// 対象文字列のチェック
		if (verifyTarget(target) == ModFalse) {
#ifdef STEM_DEBUG
			ModDebugMessage << "stem: invalid target" << ModEndl;
#endif
			// 英字以外を含む場合はターゲットをそのまま返す
			result = target;
			return ModFalse;
		}

		// 辞書検索
		DictIndex* dict = searchDict(target);
		if (dict != 0) {	// マッチする見出しあり
			// 同形ならターゲットを返す
			if (dict->dictKeyOffset == dict->dictStemOffset) {
				result = target;
				return ModFalse;
			}
			// 正規化語形を返す
			result = dictKey + dict->dictStemOffset;
			return ModTrue;
		}

		// 規則検索
		RuleIndex* rule = searchRule(target);
		// マッチする規則がなければターゲットを返す
		if (rule == 0) {
			result = target;
			return ModFalse;
		}
		// 正規化語形に置換される文字数を求める
		ModSize len = target.getLength() -
			ModUnicodeCharTrait::length(ruleKey + rule->ruleKeyOffset);
		// 正規化語形を結果文字列にコピーする
		result = target.copy(0, len) + (ruleStem +
										  rule->ruleStemOffset);
		return ModTrue;

    } catch (ModException& exception) {
#ifdef STEM_DEBUG
		ModDebugMessage << "stem: " << exception << ModEndl;
#endif
#ifdef DEBUG
		ModErrorMessage << "stem: " << exception << ModEndl;
#endif
        ModRethrow(exception);
    }
}


//
// FUNCTION public
//	ModEnglishWordStemmer::expand -- 正規化＆展開
//
// NOTES
//	対象文字列を正規化・展開する。
//
// ARGUMENTS
//	const ModUnicodeString& target
//		対象文字列
//	ModVector <ModUnicodeString>& result
//		結果格納用ベクター
//
// RETURN
//	ModBoolean
//		対象文字列に対して１語以上の展開語形が得られた場合 and/or 対象
//		文字列と異なる正規化文字列が得られた場合は ModTrue を、そうで
//		ない場合は ModFalse を返す。
//
// EXCEPTIONS
//	下位からの例外をそのまま返す。
//
ModBoolean
ModEnglishWordStemmer::expand(const ModUnicodeString& target,
									  ModVector <ModUnicodeString>& result)
{
    try{
		// 対象文字列のチェック
		if (verifyTarget(target) == ModFalse) {
#ifdef STEM_DEBUG
			ModDebugMessage << "expand: invalid target" << ModEndl;
#endif
			// 英字以外を含む場合はターゲットをそのまま返す
			result.pushBack(target);
			return ModFalse;
		}

		// 辞書検索
		DictIndex* dict = searchDict(target);
		if (dict != 0) {	// マッチする見出しあり
			// 展開語形なし
			if (dict->dictExpandOffset < 0) {
				// 見出しと正規化語形が同形ならターゲットを返す
				if (dict->dictKeyOffset == dict->dictStemOffset) {
					result.pushBack(target);
					return ModFalse;
				}
				// 正規化語形を返す
				result.pushBack(dictKey + dict->dictStemOffset);
				return ModTrue;
			}
			// 展開語形をスペース毎に分解してベクターに挿入し ModTrue を返す
			breakExpand(dictExpand + dict->dictExpandOffset, result);
			return ModTrue;
		}

		// 規則検索
		RuleIndex* rule = searchRule(target);

		// マッチする規則がなければ ModFalse を返す。
		if (rule == 0) {
			result.pushBack(target);
			return ModFalse;
		}

		// 正規化語形を求める
		ModSize len =
			target.getLength() -
			ModUnicodeCharTrait::length(ruleKey + rule->ruleKeyOffset);
		ModUnicodeString stemmed =
			target.copy(0, len) + (ruleStem + rule->ruleStemOffset);

		// 正規化語形で再度辞書検索する
		dict = searchDict(stemmed);

		// 以下のいずれかなら正規化語形を返す
		// ・マッチする見出しがない
		// ・展開語形がない
		// ・見出しと正規化語形が異なる（二重に正規化されるのを避けるため）
		if (dict == 0 ||
			dict->dictExpandOffset < 0 ||
			dict->dictKeyOffset != dict->dictStemOffset) {
			result.pushBack(stemmed);
			return ModTrue;
		}
		// 展開語形を分解してベクターに挿入し ModTrue を返す
		breakExpand(dictExpand + dict->dictExpandOffset, result);
		return ModTrue;

    } catch (ModException& exception) {
#ifdef STEM_DEBUG
		ModDebugMessage << "expand: " << exception << ModEndl;
#endif
#ifdef DEBUG
		ModErrorMessage << "expand: " << exception << ModEndl;
#endif
        ModRethrow(exception);
    }
}


//
// FUNCTION private
//	ModEnglishWordStemmer::serialize -- シリアライズ
//
// NOTES
//	辞書・規則データをシリアライズする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		指定されたファイルの先頭と末尾が DataVerifier と一致しない場合は、
//		上記例外を返す。
//	下位からの例外はそのまま返す。
//
void
ModEnglishWordStemmer::serialize(ModArchive& archiver)
{
    try {
		ModSize length = ModCharTrait::length(DataVerifier);

		// データ認識文字列
        if (archiver.isStore() == ModTrue) {
			archiver((void*)DataVerifier, length);
		} else {
			char* buffer = new char[length + 1];
			archiver((void*)buffer, length);
            buffer[length] = ModCharTrait::null();
			if (ModCharTrait::compare(buffer, DataVerifier) != 0) {
#ifdef DEBUG
                ModErrorMessage << "serialize: illegal data file"
                                << ModEndl;
#endif
				delete [] buffer;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			delete [] buffer;
		}

		// 数値メンバ
		archiver(dictKeyLen);
		archiver(dictExpandLen);
		archiver(ruleKeyLen);
		archiver(ruleStemLen);
		archiver(dictKeyNum);
		archiver(ruleKeyNum);

		// 可変長配列メンバの領域を確保する（ロード時）。
		// シリアライザはコンストラクタを通してのみ呼ばれるので、
		// 重複して確保されることはないため、delete はしない。
        if (archiver.isLoad() == ModTrue) {
            dictKey    = new char[dictKeyLen + 1];
            dictExpand = new char[dictExpandLen + 1];

            ruleKey  = new ModUnicodeChar[ruleKeyLen + 1];
            ruleStem = new ModUnicodeChar[ruleStemLen + 1];

            dictIndex = new DictIndex[dictKeyNum];
            ruleIndex = new RuleIndex[ruleKeyNum];
        }

		// 文字列メンバ
		archiver((void*)dictKey, dictKeyLen);
		archiver((void*)dictExpand, dictExpandLen);

		archiver((void*)ruleKey,
				 sizeof(ModUnicodeChar) * ruleKeyLen);
		archiver((void*)ruleStem,
				 sizeof(ModUnicodeChar) * ruleStemLen);

		// 文字列の末尾に空文字を挿入（ロード時）
		if (archiver.isLoad() == ModTrue) {
            dictKey[dictKeyLen]       = ModCharTrait::null();
            dictExpand[dictExpandLen] = ModCharTrait::null();

            ruleKey[ruleKeyLen]   = ModUnicodeCharTrait::null();
            ruleStem[ruleStemLen] = ModUnicodeCharTrait::null();
		}

		// 配列メンバ
		archiver((void*)dictIndex, sizeof(DictIndex) * dictKeyNum);
		archiver((void*)ruleIndex, sizeof(RuleIndex) * ruleKeyNum);

		// 固定長配列メンバ
        archiver((void*)&dictInit, sizeof(InitCharIndex) *
				  ModEnglishWordStemmerInitCharNum);
        archiver((void*)&ruleInit, sizeof(InitCharIndex) *
				  ModEnglishWordStemmerInitCharNum);

		// データ認識文字列
        if (archiver.isStore() == ModTrue) {
			archiver((void*)DataVerifier, length);
		} else {
			char* buffer = new char[length + 1];
			archiver((void*)buffer, length);
            buffer[length] = ModCharTrait::null();
			if (ModCharTrait::compare(buffer, DataVerifier) != 0) {
#ifdef DEBUG
                ModErrorMessage << "serialize: illegal data"
                                << ModEndl;
#endif
				delete [] buffer;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			delete [] buffer;
		}
#ifdef STEM_DEBUG
        ModDebugMessage << "serialize: archive size = "
						<< archiver.getSize() << ModEndl;
#endif

    } catch (ModException& exception) {
		delete [] dictKey;
		delete [] dictExpand;
		delete [] ruleKey;
		delete [] ruleStem;
		delete [] dictIndex;
		delete [] ruleIndex;
#ifdef DEBUG
        ModErrorMessage << "serialize: " << exception << ModEndl;
#endif
        ModRethrow(exception);
    }
    return;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::setDictKey -- 辞書見出し語形の設定
//
// NOTES
//	ソースファイルから辞書見出し語形を読み込み、メンバ変数 dictKey に
//	格納する。更に、先頭文字インデックス配列 dictInit の設定も行なう。
//
// ARGUMENTS
//	const ModCharString& path
//		ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述にレコード区切りと小文字アルファベット
//		  以外の文字が含まれる
//		・ソースファイル末尾が改行でない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setDictKey(ModArchive& archiver)
{
    try {
		// 文字列領域を確保する
		dictKey = new char[dictKeyLen + 1];

		char* dummy = dictKey;	// コピー先ポインタ
		char* init	= dictKey;	// 語頭ポインタ
		char  current;			// 入力ストリーム格納用

		while (1) {
			archiver >> current;		// 1文字ずつ読み込む

			if (current == SepRecord) {	// レコード区切り
				// 語末に空文字を挿入
				*dummy = ModCharTrait::null();
#ifdef STEM_DEBUG
				ModDebugMessage << "setDictKey("
								<< dictKeyNum << "): "
								<< init - dictKey << " "
								<< init << ModEndl;
#endif
				// 先頭文字インデックスを設定
				ModSize count = *init - FirstChar;
				// 先頭位置
				if (dictInit[count].headIndex < 0) {
					dictInit[count].headIndex = dictKeyNum;
				}
				// 末尾位置
				dictInit[count].tailIndex = dictKeyNum;
				// 見出し長最小値
				int length = (int)(dummy - init);
				if (dictInit[count].minKeyLen < 1 ||
					dictInit[count].minKeyLen > length) {
					dictInit[count].minKeyLen = length;
				}
				++ dictKeyNum;	// 語数をインクリメント
				++ dummy;		// コピー先をインクリメント
				init = dummy;	// 語頭をセット

			} else {
				// 小文字アルファベット以外は例外
				if (current < FirstChar || current > LastChar) {
#ifdef DEBUG
					ModErrorMessage << "setDictKey("
									<< dictKeyNum << "): "
									<< "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				*dummy = current;	// 文字をコピー
				++ dummy;			// コピー先をインクリメント
			}
		}

	} catch (ModException& exception) {
		// ファイル末尾
		if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// 文字列が終了していない（ファイル末尾に改行なし）
			if (dictKey[dictKeyLen] != ModCharTrait::null()) {
#ifdef DEBUG
				ModErrorMessage << "setDictKey: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			ModErrorHandle::reset();
		} else {
#ifdef DEBUG
			ModErrorMessage << "setDictKey: " << exception << ModEndl;
#endif
            ModRethrow(exception);
		}
	}
    return;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::setDictExpand -- 辞書展開語形の設定
//
// NOTES
//	ソースファイルから辞書展開語形を読み込み、メンバ変数 dictExpand に
//	格納する。
//
// ARGUMENTS
//	const ModCharString& path
//		ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述にレコード区切り、フィールド区切りと
//		  小文字アルファベット以外の文字が含まれる
//		・ソースファイル末尾が改行でない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setDictExpand(ModArchive& archiver)
{
	ModSize num_word = 0;	// 語数
    try {
		// 文字列領域を確保する
		dictExpand = new char[dictExpandLen + 1];

        char* dummy = dictExpand;	// コピー先ポインタ
        char* init  = dictExpand;	// 語頭ポインタ
        char  current;				// 入力ストリーム格納用

		while (1) {
            archiver >> current;		// 1文字ずつ読み込む

            if (current == SepRecord) {	// レコード区切り
				// 語末に空文字を挿入
                *dummy = ModCharTrait::null();
#ifdef STEM_DEBUG
                ModDebugMessage << "setDictExpand("
								<< num_word << "): "
								<< init - dictExpand << " "
								<< init << ModEndl;
#endif
                ++ num_word;	// 語数をインクリメント
                ++ dummy;		// コピー先をインクリメント
                init = dummy;	// 語頭をセット

			} else if (current == SepField) {	// フィールド区切り
                *dummy = current;	// 文字をコピー
                ++ dummy;			// コピー先をインクリメント

			} else {
				// 小文字アルファベット以外は例外
				if (current < FirstChar || current > LastChar) {
#ifdef DEBUG
					ModErrorMessage << "setDictExpand("
									<< num_word << "): "
									<< "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
                *dummy = current;	// 文字をコピー
                ++ dummy;			// コピー先をインクリメント
			}
		}

	} catch (ModException& exception) {
		// ファイル末尾
		if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// 文字列が終了していない（ファイル末尾に改行なし）
			if (dictExpand[dictExpandLen] != ModCharTrait::null()) {
#ifdef DEBUG
				ModErrorMessage << "setDictExpand: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			ModErrorHandle::reset();
		} else {
#ifdef DEBUG
			ModErrorMessage << "setDictExpand: " << exception << ModEndl;
#endif
            ModRethrow(exception);
		}
	}
    return;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::setRuleKey -- 規則見出し語形の設定
//
// NOTES
//	ソースファイルから規則見出し語形を読み込み、メンバ変数 ruleKey に
//  格納する。更に、先頭文字インデックス配列 ruleInit の設定も行なう。
//
// ARGUMENTS
//	const ModCharString& path
//		ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述にレコード区切りと小文字アルファベット
//		  以外の文字が含まれる
//		・ソースファイル末尾が改行でない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setRuleKey(ModArchive& archiver)
{
    try {
		// 文字列領域を確保する
		ruleKey = new ModUnicodeChar[ruleKeyLen + 1];

		ModUnicodeChar* dummy = ruleKey;	// コピー先ポインタ
		ModUnicodeChar* init  = ruleKey;	// 語頭ポインタ
		char current;						// 入力ストリーム格納用

		while (1) {
			archiver >> current;		// 1文字ずつ読み込む

			if (current == SepRecord) {	// レコード区切り
				// 語末に空文字を挿入
				*dummy = ModUnicodeCharTrait::null();
#ifdef STEM_DEBUG
				ModDebugMessage << "setRuleKey("
								<< ruleKeyNum << "): "
								<< init - ruleKey << " "
								<< init << ModEndl;
#endif
                // 先頭文字インデックスを設定
				ModSize count = *init - FirstChar;
				// 先頭位置
				if (ruleInit[count].headIndex < 0) {
					ruleInit[count].headIndex = ruleKeyNum;
				}
				// 末尾位置
				ruleInit[count].tailIndex = ruleKeyNum;
				// 見出し長最小値
				int length = (int)(dummy - init);
				if (ruleInit[count].minKeyLen < 1 ||
					ruleInit[count].minKeyLen > length) {
					ruleInit[count].minKeyLen = length;
				}
				++ ruleKeyNum;	// 語数をインクリメント
				++ dummy;		// コピー先をインクリメント
				init = dummy;	// 語頭をセット

			} else {
				// 小文字アルファベット以外は例外
				if (current < FirstChar || current > LastChar) {
#ifdef DEBUG
					ModErrorMessage << "setRuleKey("
									<< ruleKeyNum << "): "
									<< "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				*dummy = current;	// 文字をコピー
				++ dummy;			// コピー先をインクリメント
			}
		}

	} catch (ModException& exception) {
		// ファイル末尾
		if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// 文字列が終了していない（ファイル末尾に改行なし）
			if (ruleKey[ruleKeyLen] != ModCharTrait::null()) {
#ifdef DEBUG
				ModErrorMessage << "setRuleKey: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			ModErrorHandle::reset();
		} else {
#ifdef DEBUG
			ModErrorMessage << "setRuleKey: " << exception << ModEndl;
#endif
            ModRethrow(exception);
		}
	}
    return;
}

//
// FUNCTION private
//	ModEnglishWordStemmer::setRuleStem -- 規則正規化語形の設定
//
// NOTES
//	ソースファイルから規則正規化語形を読み込み、メンバ変数 ruleStem に
//  格納する。
//
// ARGUMENTS
//	const ModCharString& path
//		ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述にレコード区切りと小文字アルファベット
//		  以外の文字が含まれる
//		・ソースファイル末尾が改行でない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setRuleStem(ModArchive& archiver)
{
	ModSize num_word = 0;	// 語数
    try {
		// 文字列領域を確保する
		ruleStem = new ModUnicodeChar[ruleStemLen + 1];

		ModUnicodeChar* dummy = ruleStem;	// コピー先ポインタ
		ModUnicodeChar* init  = ruleStem;	// 語頭ポインタ
		char current;						// 入力ストリーム格納用

		while (1) {
			archiver >> current;		// 1文字ずつ読み込む

			if (current == SepRecord) {	// レコード区切り
				// 語末に空文字を挿入
				*dummy = ModUnicodeCharTrait::null();
#ifdef STEM_DEBUG
				ModDebugMessage << "setRuleStem("
								<< num_word << "): "
								<< init - ruleStem << " "
								<< init << ModEndl;
#endif
				++ num_word;	// 語数をインクリメント
				++ dummy;		// コピー先をインクリメント
				init = dummy;	// 語頭をセット
			} else {
				// 小文字アルファベット以外は例外
				if (current < FirstChar || current > LastChar) {
#ifdef DEBUG
					ModErrorMessage << "setRuleStem("
									<< num_word << "): "
									<< "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				*dummy = current;	// 文字をコピー
				++ dummy;			// コピー先をインクリメント
			}
		}
	} catch (ModException& exception) {
		// ファイル末尾
		if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// 文字列が終了していない（ファイル末尾に改行なし）
			if (ruleStem[ruleStemLen] != ModCharTrait::null()) {
#ifdef DEBUG
				ModErrorMessage << "setRuleStem: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			ModErrorHandle::reset();
		} else {
#ifdef DEBUG
			ModErrorMessage << "setRuleStem: " << exception << ModEndl;
#endif
            ModRethrow(exception);
		}
	}
    return;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::setDictIndex -- 辞書インデックスの設定
//
// NOTES
//	ソースファイルから辞書インデックスを読み込み、メンバ変数 dictIndex
//	に格納する。
//
// ARGUMENTS
// const ModCharString& path
//      ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述に、レコード区切り、フィールド区切りと
//		  アラビア数字以外の文字が含まれる
//		・フィールド数が規定数(3)に合わない
//		・インデックス数が見出し語形数と合わない
//		・ファイル末尾が改行でない
//		・オフセット値が記述されていない
//		・オフセット値が最小値より小さい
//		・オフセット値が最大値より大きい
//		・オフセットが語頭位置を指していない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setDictIndex(ModArchive& archiver)
{
	int num_record	= 0;	// レコード数
	int	num_field	= 0;	// フィールド数
    try {
		// 見出し語形数分の領域を確保する
		dictIndex = new DictIndex[dictKeyNum];

		char  			current;	// 入力ストリーム格納用
		ModCharString	buffer;		// 文字列格納用バッファ

		// オフセット値の最大桁数を求める
		ModSize size	= 0;
		ModSize max		= ruleKeyLen;	// 見出しオフセット最大値
		for (; max > 0; size ++) {
			max = max / 10;
		}
		// 最大桁数分の領域を確保
		buffer.reallocate(size);

		while (1) {
			archiver >> current;	// 1文字ずつ読み込む

			if (num_record == dictKeyNum) {
				// インデックス数が見出し数と合わない
#ifdef DEBUG
				ModErrorMessage << "setRuleIndex: "
								<< "illegal number of records "
								<< num_record + 1 << " "
								<< dictKeyNum << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}

			if (current == SepRecord) {  // レコード区切り
				// フィールド数が規定外
				if (num_field != 2) {
#ifdef DEBUG
                    ModErrorMessage << "setDictIndex("
									<< num_record << "): "
									<< "illegal number of fields"
                                    << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
                }
				// バッファが空である
				if (buffer.getLength() == 0) {
#ifdef DEBUG
                    ModErrorMessage << "setDictIndex("
									<< num_record << "): "
									<< "no offset for expand"
                                    << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}

                // 展開語形オフセットを設定
                dictIndex[num_record].dictExpandOffset =
					ModCharTrait::toInt(buffer) - 1;

				// オフセット値が規定範囲外、またはオフセットの位置が
				// 単語先頭でない
				int i = dictIndex[num_record].dictExpandOffset;
				if (i < -1 ||
					i > dictExpandLen ||
					(i > 0 && dictExpand[i - 1] != ModCharTrait::null())) {
#ifdef DEBUG
					ModErrorMessage << "setDictIndex("
									<< num_record << "): "
									<< "illegal offset value for expand: "
                                    << i << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}

#ifdef STEM_DEBUG
				ModDebugMessage << "setDictIndex("
								<< num_record << "): "
								<< dictIndex[num_record].dictKeyOffset
								<< " "
								<< dictIndex[num_record].dictStemOffset
								<< " "
								<< dictIndex[num_record].dictExpandOffset
								<< ModEndl;
#endif
                buffer.clear();     // Clearing the buffer
                num_field = 0;      // Initializing the number of fields
                ++ num_record;

			} else if (current == SepField) {  // フィールド区切り
				if (num_field == 0) {
					// バッファが空である
					if (buffer.getLength() == 0) {
#ifdef DEBUG
						ModErrorMessage << "setDictIndex("
										<< num_record << "): "
										<< "no offset for key"
										<< ModEndl;
#endif
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument, ModErrorLevelError);
					}

					// 見出し語形オフセットを設定
					dictIndex[num_record].dictKeyOffset =
						ModCharTrait::toInt(buffer) - 1;

					// オフセット値が規定範囲外、またはオフセットの位
					// 置が単語先頭でない
					int i = dictIndex[num_record].dictKeyOffset;
					if (i < 0 ||
						i > dictKeyLen ||
						(i > 0 &&
						 dictKey[i - 1] != ModCharTrait::null())) {
#ifdef DEBUG
						ModErrorMessage << "setDictIndex("
										<< num_record << "): "
										<< "illegal offset value for key: "
										<< i << ModEndl;
#endif
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument, ModErrorLevelError);
					}

				} else if (num_field == 1) {
					// バッファが空である
					if (buffer.getLength() == 0) {
#ifdef DEBUG
						ModErrorMessage << "setDictIndex("
										<< num_record << "): "
										<< "no offset for stem"
										<< ModEndl;
#endif
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument, ModErrorLevelError);
					}

                    // 正規化語形オフセットを設定
					dictIndex[num_record].dictStemOffset =
						ModCharTrait::toInt(buffer) - 1;

					// オフセット値が規定範囲外、またはオフセットの位
					// 置が単語先頭でない
					int i = dictIndex[num_record].dictStemOffset;
					if (i < 0 ||
						i > dictKeyLen ||
						(i > 0 &&
						 dictKey[i - 1] != ModCharTrait::null())) {
#ifdef DEBUG
						ModErrorMessage << "setDictIndex("
										<< num_record << "): "
										<< "illegal offset value for stem: "
										<< i << ModEndl;
#endif
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument, ModErrorLevelError);
					}

				} else {
					// フィールド数が規定外
#ifdef DEBUG
					ModErrorMessage << "setDictIndex("
									<< num_record << "): "
									<< "illegal number of fields"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
                buffer.clear(); // バッファをクリア
                ++ num_field;   // フィールド数をインクリメント

			} else {
				// 数字以外は例外
				if (ModCharTrait::isDigit(current) == ModFalse) {
#ifdef DEBUG
					ModErrorMessage << "setDictIndex("
									<< num_record << "): "
									<< "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				buffer += current;	// バッファに追加
			}
		}
    } catch (ModException& exception) {
		// ファイル末尾
        if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// フィールド数が 0 でない（ファイル末尾に改行なし）
			if (num_field > 0) {
#ifdef DEBUG
				ModErrorMessage << "setDictIndex: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
            ModErrorHandle::reset();
        } else {
#ifdef DEBUG
            ModErrorMessage << "setDictIndex: " << exception << ModEndl;
#endif
            ModRethrow(exception);
        }
    }
	if (num_record != dictKeyNum) {
		// インデックス数が見出し数と合わない
#ifdef DEBUG
		ModErrorMessage << "setDictIndex: "
						<< "illegal number of records: "
						<< num_record << " "
						<< dictKeyNum << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
#ifdef STEM_DEBUG
	ModDebugMessage << "dictInit" << ModEndl;
	ModSize i = 0;
	for (; i < ModEnglishWordStemmerInitCharNum; i ++) {
		ModDebugMessage << "\t" << (char)(FirstChar + i) << " "
						<< dictInit[i].headIndex << " "
						<< dictInit[i].tailIndex << " "
						<< dictInit[i].minKeyLen << ModEndl;
	}
#endif
    return;
}


//
// FUNCTION private
//	ModEnglishWordStemmer::setRuleIndex -- 規則インデックスの設定
//
// NOTES
//	ソースファイルから辞書インデックスを読み込み、メンバ変数 dictIndex
//	に格納する。
//
// ARGUMENTS
// const ModCharString& path
//      ソースファイルへのパス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		以下の場合に上記例外を返す。
//		・ソースファイルの記述に、レコードフィールド区切りとアラビア数字
//		  以外の文字を含む
//		・フィールド数が規定数(2)に合わない
//		・インデックス数が見出し語形数と合わない
//		・ファイル末尾が改行でない
//		・オフセット値が記述されていない
//		・オフセット値が最小値より小さい
//		・オフセット値が最大値より大きい
//		・オフセットが語頭位置を指していない
//	下位からの例外は、ModOsErrorEndOfFileの場合はリセットする。
//	それ以外はそのまま返す。
//
void
ModEnglishWordStemmer::setRuleIndex(ModArchive& archiver)
{
	int num_record	= 0;	// レコード数
	int	num_field	= 0;	// フィールド数
    try {
		// 見出し語形数分の領域を確保する
		ruleIndex = new RuleIndex[ruleKeyNum];

		char  			current;	// 入力ストリーム格納用
		ModCharString	buffer;		// 文字列格納用バッファ

		// オフセット値の最大桁数を求める
		ModSize	size	= 0;
		ModSize	max		= ruleKeyLen;
		for (; max > 0; size ++) {
			max = max / 10;
		}
		// 最大桁数分の領域を確保
		buffer.reallocate(size);

		while (1) {
			archiver >> current;	// 1文字ずつ読み込む

			if (num_record == ruleKeyNum) {
				// インデックス数が見出し数と合わない
#ifdef DEBUG
				ModErrorMessage << "setRuleIndex: "
								<< "illegal number of records "
								<< num_record + 1 << " "
								<< ruleKeyNum << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}

			if (current == SepRecord) {  // 改行
				// フィールド数が規定外
				if (num_field != 1) {
#ifdef DEBUG
					ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
                                    << "illegal number of fields"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				// バッファが空である
				if (buffer.getLength() == 0) {
#ifdef DEBUG
                    ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
									<< "no offset for stem"
                                    << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}

				// 正規化語形オフセットを設定
				ruleIndex[num_record].ruleStemOffset =
					ModCharTrait::toInt(buffer) - 1;

				// オフセット値が規定範囲外、またはオフセットの位置が
				// 単語先頭でない
				int i = ruleIndex[num_record].ruleStemOffset;
				if (i < 0 ||
					i > ruleStemLen ||
					(i > 0 &&
					 ruleStem[i - 1] != ModUnicodeCharTrait::null())) {
#ifdef DEBUG
					ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
									<< "illegal offset value for stem: "
									<< i << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
#ifdef STEM_DEBUG
				ModDebugMessage << "setRuleIndex("
								<< num_record << "): "
								<< ruleIndex[num_record].ruleKeyOffset
								<< " "
								<< ruleIndex[num_record].ruleStemOffset
								<< " " << ModEndl;
#endif
				buffer.clear();		// バッファをクリア
				num_field = 0;		// フィールド数を初期化
				++ num_record;

			} else if (current == SepField) {   // フィールド区切り
				// フィールド数が規定外
				if (num_field != 0) {
#ifdef DEBUG
					ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
									<< "illegal number of fields"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				// バッファが空である
				if (buffer.getLength() == 0) {
#ifdef DEBUG
                    ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
									<< "no offset for key"
                                    << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}

				// 見出し語形オフセットを設定
				ruleIndex[num_record].ruleKeyOffset =
					ModCharTrait::toInt(buffer) - 1;

				// オフセット値が規定範囲外、またはオフセットの位置が
				// 単語先頭でない
				int i = ruleIndex[num_record].ruleKeyOffset;
				if (i < 0 ||
					i > ruleKeyLen ||
					(i > 0 &&
					 ruleKey[i - 1] != ModUnicodeCharTrait::null())) {
#ifdef DEBUG
					ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
									<< "illegal offset value for key: "
									<< i << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}

				buffer.clear(); // バッファをクリア
				++ num_field;	// フィールド数をインクリメント

			} else {
                // 数字以外は例外
				if (ModCharTrait::isDigit(current) == ModFalse) {
#ifdef DEBUG
					ModErrorMessage << "setRuleIndex("
									<< num_record << "): "
                                    << "invalid character"
									<< ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
				buffer += current;	// バッファに追加
			}
		}

    } catch (ModException& exception) {
		// ファイル末尾
        if (exception.getErrorNumber() == ModOsErrorEndOfFile) {
			// フィールド数が 0 でない（ファイル末尾に改行なし）
			if (num_field > 0) {
#ifdef DEBUG
				ModErrorMessage << "setRuleIndex: "
								<< "not terminated" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			ModErrorHandle::reset();
        } else {
#ifdef DEBUG
			ModErrorMessage << "setRuleIndex: " << exception << ModEndl;
#endif
			ModRethrow(exception);
		}
    }

	if (num_record != ruleKeyNum) {
		// インデックス数が見出し数と合わない
#ifdef DEBUG
		ModErrorMessage << "setRuleIndex: "
						<< "illegal number of records: "
						<< num_record << " "
						<< ruleKeyNum << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
#ifdef STEM_DEBUG
	ModDebugMessage << "ruleInit" << ModEndl;
	ModSize i = 0;
	for (; i < ModEnglishWordStemmerInitCharNum; i ++) {
		ModDebugMessage << "\t" << (char)(FirstChar + i) << " "
						<< ruleInit[i].headIndex << " "
						<< ruleInit[i].tailIndex << " "
						<< ruleInit[i].minKeyLen << ModEndl;
	}
#endif
	return;
}

//
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
