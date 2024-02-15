// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedCountUnit.h -- 1つの転置ファイルをあらわすクラス
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDCOUNTUNIT_H
#define __SYDNEY_FULLTEXT2_INVERTEDCOUNTUNIT_H

#include "FullText2/Module.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/BtreeFile.h"

#include "Trans/Transaction.h"
#include "Os/Path.h"


_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::InvertedCountUnit
//		-- バキューム機能付き大転置のために転置ファイルユニット
//
//	NOTES
// 	索引単位をキーにバリューに削除数を持つB木を保持している
//
class InvertedCountUnit : public InvertedUnit
{
public:
	// コンストラクタ
	InvertedCountUnit(InvertedSection& cInvertedSection_,
					  const Os::Path& cPath_,
					  bool bBatch_,
					  int iUnitNumber = -1);
	// デストラクタ
	virtual ~InvertedCountUnit();

	// ファイルを作成する
	void create();

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);
	// ファイルの内容をクリアする
	void clear();

	// ページをsaveする
	bool saveAllPages();

	// バキュームが必要かどうか
	bool isNeedVacuum(const ModUnicodeString& cstrKey_,
					  int iNewExpungeCount_);
	// バキュームが必要かどうかのカウントをクリアする
	void clearExpungeCount(const ModUnicodeString& cstrKey_);

protected:
	// ファイルをattachする
	void attachCountFile();
	// ファイルをdetachする
	void detachCountFile();

private:
	// 削除数を保持するB木ファイル
	BtreeFile* m_pCountFile;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDCOUNTUNIT_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
