// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModUnaMiddle.h -- UNA ラッパークラスの定義
// 
// Copyright (c) 2000-2007, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModUnaMiddle_H__
#define __ModUnaMiddle_H__

#include "UNA_UNIFY_TAG.h"
#include "ModCommonDLL.h"
#include "ModConfig.h"
#include "ModDefaultManager.h"
#include "ModVector.h"
#include "ModCriticalSection.h"
#include "ModNlpUnaJp/Module.h"

struct unaKApiHandleT;
struct unaKApiDicImgT;
struct unaMorphT;
struct unaBnsT;

_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModUnaMiddle;
class ModUnaMiddleAnalyzer;
class ModUnaResource;
class UnaJpDicSet;

//
// CLASS
// ModUnaMiddleAnalyzer -- UNA 解析器クラス
//
// NOTES
// UNA の解析機能を提供するラッパークラス。
//
class UNA_LOCAL_FUNCTION ModUnaMiddleAnalyzer : public ModDefaultObject
{
private:
	DicSet	*m_dicSet;
public:
	ModUnaMiddleAnalyzer(const ModUnaResource*,
						 DicSet	*dicSet_ = 0,
						 const ModBoolean = ModTrue,
						 const ModBoolean = ModTrue,
						 ModSize MaxWordLen = 0);

	virtual ~ModUnaMiddleAnalyzer();
	
	// NOTES  解析対象テキストを設定する
	// RETURN なし
	void set(
		const ModUnicodeString& target,  // 解析対象テキスト
		const ModSize = 0);	// 解析モード 0:下位構造展開なし/stemmingなし
		                   	//            1:下位構造展開あり/stemmingあり
		                   	//            2:下位構造展開あり/stemmingなし
		                   	//            3:下位構造展開なし/stemmingあり
							//            4: 0 + 改行をまたぐ単語検出あり
		                   	//            5: 1 + 改行をまたぐ単語検出あり
		                   	//            6: 2 + 改行をまたぐ単語検出あり
		                   	//            7: 3 + 改行をまたぐ単語検出あり

	// NOTES  形態素解析結果を取得する
	// RETURN 解析結果が残っていれば ModTrue、そうでなければ ModFalse
	virtual ModBoolean getMorph(
		ModUnicodeString& result,		// 解析結果
		const ModBoolean normalize);	// 正規化指示

	// NOTES  形態素解析結果を取得する
	// RETURN 解析結果が残っていれば ModTrue、そうでなければ ModFalse
	ModBoolean getMorph(
		ModUnicodeString& result,		// 解析結果
		ModUnicodeString& original,		// 元表記
		const ModBoolean normalize);	// 正規化指示

	// NOTES  異表記正規化抜きで形態素解析結果を取得する
	// RETURN 解析結果が残っていれば ModTrue、そうでなければ ModFalse
	ModBoolean get(
		ModUnicodeString& result,		// 解析結果
		const ModBoolean execStdMode);	// 前処理実行指示

	// NOTES  異表記正規化抜きで形態素解析結果を取得する
	// RETURN 解析結果が残っていれば ModTrue、そうでなければ ModFalse
	ModBoolean get(
		ModUnicodeString& result,		// 解析結果
		ModUnicodeString& original,		// 元表記
		const ModBoolean execStdMode);	// 前処理実行指示

	ModBoolean getBlock(ModUnicodeString& result,
		const ModUnicodeChar separator1,
		const ModUnicodeChar separator2);

	// ModTermVersion2
	ModBoolean getBlock(
		ModVector<ModUnicodeString>& formVector,
		ModVector<int>& posVector);

	// ModTermVersion2
	ModBoolean getBlock(	ModVector<ModUnicodeString>& formVector_,
							ModVector<ModUnicodeString>& ostrVector_,
							ModVector<int>& posVector_,
							ModVector<int> *costVector_,
							ModVector<int> *uposVector_);

	// NOTES  形態素解析結果の辞書ベース名
	// RETURN 解析結果が残っていれば ModTrue、そうでなければ ModFalse
	ModBoolean getDicName(ModVector<ModUnicodeString>& dicNameVector_);
	ModVector<ModUnicodeString> dicNameVector;	// 辞書ベース名ベクター

	void clear(const ModBoolean = ModFalse);

	void addDictionary();
	void delDictionary();

	const ModBoolean doStem( const ModSize mode);
	const ModBoolean doSub( const ModSize mode);
	const ModBoolean doCR( const ModSize mode);
    const ModUnicodeChar checkNextChar();	

	void emulateMaxWordOld( const ModBoolean s);
	ModBoolean getRemainLength(ModSize* pulRemainLength_);
protected:
	void analyze(const ModBoolean = ModTrue, const ModBoolean = ModFalse);

	const ModUnaResource* resource;
	unaKApiHandleT* handle;

	const ModUnicodeChar* targetStart;
	const ModUnicodeChar* target;
	ModInt32 targetLength;
	ModSize mode;

	unaMorphT* morphBuffer;				// 形態素解析結果バッファ
	ModInt32 morphNum;
	ModInt32 morphCurrent;

	unaMorphT* subMorphBuffer;			// 形態素解析結果バッファ（下位構造用）
	ModInt32 subMorphNum;
	ModInt32 subMorphCurrent;

	unaBnsT* bunsetsuBuffer;			// 係り受け解析結果バッファ
	ModInt32 bunsetsuNum;
	ModInt32 bunsetsuCurrent;

	ModSize maxWordLen;
	static const ModSize unaLocalKeitaisoSize;
	static const ModSize unaLocalBunsetsuSize;
	
	ModBoolean emulateMwOld;
};


//
// CLASS
// ModUNAResource -- UNA resource class
//
// NOTES
// UNA 解析器が使用する資源を保持する。
//
class UNA_LOCAL_FUNCTION ModUnaResource : public ModDefaultObject
{
	friend class ModUnaMiddleAnalyzer;
	friend class ModUnaAnalyzer;
public:
	ModUnaResource(const ModUnicodeString&,
				   ModBoolean memSwith=ModFalse);

	virtual ~ModUnaResource();

	ModSize optionStatus;		// オプション辞書の状態
	
private:
	void load(ModVector<ModUnicodeString>&, ModVector<ModUnicodeString>&,
			  ModVector<ModUnicodeString>&, ModVector<int>&,
			  ModUnicodeString&, ModUnicodeString&, ModUnicodeString&,
			  ModUnicodeString&, ModUnicodeString&, ModUnicodeString&);
	void free();

	unaKApiDicImgT* dicList;	// UNA形態素解析辞書イメージ表
	int dicCount;				// UNA形態素解析辞書数
	char* connectTbl;			// UNA接続表のイメージ
	char* kakariTbl;			// UNAかかり受け表のイメージ
	char* egtokenTbl;			// UNA英語トークン表のイメージ
	char* unknownTbl;			// UNA未登録語表のイメージ
	char* unknownCost;			// UNA未登録語コストのイメージ
	char* normalTbl;			// 文字列標準化表のイメージ
};

_UNA_UNAJP_END
_UNA_END
#endif // __ModUnaMiddle_H__
//
// Copyright (c) 2000-2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
