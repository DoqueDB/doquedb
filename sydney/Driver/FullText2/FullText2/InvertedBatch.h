// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedBatch.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDBATCH_H
#define __SYDNEY_FULLTEXT2_INVERTEDBATCH_H

#include "FullText2/Module.h"
#include "FullText2/InvertedUpdateFile.h"
#include "FullText2/BatchListMap.h"

#include "Os/Path.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::InvertedBatch
//		-- バッチインサート用のメモリ上で実装された転置ファイルを表すクラス
//
//	NOTES
//
class InvertedBatch : public InvertedUpdateFile
{
public:
	// コンストラクタ
	InvertedBatch(InvertedSection& cSection_);
	// デストラクタ
	virtual ~InvertedBatch();

	// サイズを得る
	ModUInt64 getSize() { return m_pListMap->getListSize(); }

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_) {}

	// ファイルの内容をクリアする
	void clear();

	// ページをsaveする
	bool saveAllPages() { return true; }

	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const { return true; }
	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const { return false; }

	// 更新用リストマネージャーを得る
	UpdateListManager* getUpdateListManager();

	// 削除対象のIDブロックを削除する
	void expungeIdBlock() {}

	// 削除IDブロックのUndoログをクリアする
	void clearDeleteIdBlockUndoLog() {}
	// 削除対象のIDブロック情報をクリアする
	void clearDeleteIdBlockLog() {}

	//
	//	以下は BatchListManager のためのインターフェース
	//

	// 下限検索する
	BatchListMap::Iterator lowerBound(const ModUnicodeString& cstrKey_)
		{ return m_pListMap->lowerBound(cstrKey_); }
	// 終端を得る
	BatchListMap::Iterator end()
		{ return m_pListMap->end(); }
	// 空の転置リストを登録する
	BatchListMap::Iterator addList(const ModUnicodeString& cstrKey_);
	// 新しい転置リストを得る
	BatchBaseList* makeList(const ModUnicodeString& cstrKey_);

protected:
	// バッチ挿入用データを格納するためのマップ
	BatchListMap* m_pListMap;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDBATCH_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
