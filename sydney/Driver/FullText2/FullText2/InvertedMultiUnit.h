// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedMultiUnit.h -- 複数の転置ファイルをあらわすクラス
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDMULTIUNIT_H
#define __SYDNEY_FULLTEXT2_INVERTEDMULTIUNIT_H

#include "FullText2/Module.h"
#include "FullText2/InvertedFile.h"
#include "FullText2/InvertedSection.h"

#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedUnit;
class ListManager;

//
//	CLASS
//	FullText2::InvertedMultiUnit
//		-- 複数のInvertedUnitを保持しているクラス
//
//	NOTES
//
class InvertedMultiUnit : public InvertedFile
{
public:
	// コンストラクタ
	InvertedMultiUnit(InvertedSection& cInvertedSection_,
					  const Os::Path& cPath_,
					  bool bBatch_);
	// デストラクタ
	virtual ~InvertedMultiUnit();

	// ファイルを作成する(何もしない)
	void create() {}
	// ファイルを破棄する
	void destroy();
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	
	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const;
	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const;
	
	// InvertedUnitを得る
	InvertedUnit* getUnit(int iUnitNumber_)
		{ return m_vecpUnit[iUnitNumber_]; }
	// 要素数を得る
	ModSize getUnitCount() const
		{ return m_vecpUnit.getSize(); }

	// 検索用のListManagerを得る
	ListManager* getListManager();

	// ページをsaveする
	void saveAllPages();

#ifndef SYD_COVERAGE
	void reportFile(const Trans::Transaction& cTransaction_,
					Buffer::Page::FixMode::Value eFixMode_,
					ModOstream& stream_);
#endif
	
private:
	// ファイルをattachする
	void attach(bool bBatch_);
	// ファイルをdetachする
	void detach();

	// InvertedUnitの配列
	ModVector<InvertedUnit*> m_vecpUnit;

	// 転置ファイルセクション
	InvertedSection& m_cSection;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDMULTIUNIT_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
