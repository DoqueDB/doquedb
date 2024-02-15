// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedFile.h --
// 
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MODINVERTEDFILE_H
#define __SYDNEY_INVERTED_MODINVERTEDFILE_H

#include "ModInvertedTypes.h"

class ModInvertedCoder;
class ModInvertedDocumentLengthFile;
class ModInvertedList;
class ModInvertedTokenizer;

//
//	CLASS
//	Inverted::ModInvertedFile --
//
//	NOTES
//	本クラスはFTSの検索処理が必要とするインターフェース
//
class ModInvertedFile
{
public:
	//
	// ModInvertedQuery::getSimpleTokenNode で必要なインタフェース
	//

	// 新しい転置リストを取得(得たポインタは delete すること)
	virtual ModInvertedList* getInvertedList() const = 0;

	// 既存の転置リストをリセット
	virtual ModBoolean getInvertedList(
		const ModUnicodeString&			cstrKey_,
		ModInvertedList&				cInvertedList_,
		const ModInvertedListAccessMode	eAccessMode_) const = 0;

	// 最終文書IDを返す
	virtual ModUInt32 getLastDocumentID() const = 0;

	// 文書長ファイルを取得(得たポインタを delete してはいけない)
	virtual ModInvertedDocumentLengthFile* getDocumentLengthFile() const = 0;

	// 文書頻度を返す
	virtual ModSize	getDocumentFrequency() const = 0;

	// トークナイザを返す
	virtual ModInvertedTokenizer* getTokenizer() const = 0;

	// インデックスのタイプを返す
	// (最近できた ModInvertedDualTokenizer はこれを使うが、
	//	既存のクラスはデータメンバを直接参照している)
	ModInvertedFileIndexingType getIndexingType() const { return indexingType; }

	//
	// ModInvertedTokenizer::tokenize で必要なインタフェース
	//

	// 位置情報の圧縮器を得る
	virtual ModInvertedCoder*
	getLocationCoder(const ModUnicodeString& cstrKey_) const = 0;

	//
	// ModInvertedBooleanResultLeafNode::validate で必要なインタフェース
	// ModInvertedRankingResultLeafNode::validate で必要なインタフェース
	//
	virtual ModInvertedDocumentID getMaxDocumentID() const = 0;
	virtual ModInvertedDocumentID getMinDocumentID() const = 0;

	//
	// 中断要求をチェックするインターフェース
	//
	virtual bool isCancel() const = 0;

	//
	// 特殊なデータメンバ
	//
	// FTS用転置ファイルの検索処理は以下のデータメンバに直接アクセスしてくる。
	//

	// トークナイザのポインタ
	ModInvertedTokenizer*		tokenizer;

	// インデックスのタイプ
	ModInvertedFileIndexingType	indexingType;

	// v1.6.distから追加
	static ModUInt32 debugLevel;
	static ModUInt32 getUInt32FromModParameter(
		const char* key_, const ModUInt32 default_);
};

#endif //__SYDNEY_INVERTED_MODINVERTEDFILE_H

//
//	Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
