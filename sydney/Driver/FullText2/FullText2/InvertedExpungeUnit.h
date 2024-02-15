// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedExpungeUnit.h -- 1つの転置ファイルをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDEXPUNGEUNIT_H
#define __SYDNEY_FULLTEXT2_INVERTEDEXPUNGEUNIT_H

#include "FullText2/Module.h"
#include "FullText2/InvertedUnit.h"

#include "Trans/Transaction.h"
#include "Os/Path.h"


_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ExpungeSimpleListManager;
class ExpungeIDVectorFile;

//
//	CLASS
//	FullText2::InvertedExpungeUnit
//		-- 削除用小転置のために転置ファイルユニット
//
//	NOTES
//	通常の転置ファイルユニットに、
//	大転置の文書IDに変換するためのベクターファイルを保持している
//
class InvertedExpungeUnit : public InvertedUnit
{
public:
	// コンストラクタ
	InvertedExpungeUnit(InvertedSection& cInvertedSection_,
						const Os::Path& cPath_,
						bool bBatch_);
	// デストラクタ
	virtual ~InvertedExpungeUnit();

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

	// 大転置の文書IDに変換する
	DocumentID convertToBigDocumentID(DocumentID uiDocumentID_,
									  int& iUnitNumber_);
	// 大転置の文書IDを登録し、小転置の文書IDを得る
	DocumentID assignDocumentID(DocumentID uiBigDocumentID_,
								int iUnitNumber_);
	// 大転置ID <-> 小転置IDのベクターから削除する
	void expungeIDVector(DocumentID uiDocumentID_);

	// すべての文書IDを取得する(appendする)
	void getAll(Common::LargeVector<DocumentID>& vecID_);

protected:
	// ファイルをattachする
	void attachIDVector();
	// ファイルをdetachする
	void detachIDVector();

private:
	// 文書IDを変換するためのベクターファイル
	ExpungeIDVectorFile* m_pIDVectorFile;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDEXPUNGEUNIT_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
