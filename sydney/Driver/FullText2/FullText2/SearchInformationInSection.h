// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationInSection.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SEARCHINFORMATIONINSECTION_H
#define __SYDNEY_FULLTEXT2_SEARCHINFORMATIONINSECTION_H

#include "FullText2/Module.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/OtherInformationFile.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class FileID;

//
//	CLASS
//	FullText2::SearchInformationInSection -- 
//
//	NOTES
//	転置ファイルセクション用の検索情報クラス
//
class SearchInformationInSection : public SearchInformation
{
public:
	// コンストラクタ
	SearchInformationInSection(FileID& cFileID_,
							   OtherInformationFile* pOtherFile_,
							   ModSize uiExpungeCount_);
	// デストラクタ
	virtual ~SearchInformationInSection();
	// コピーコンストラクタ
	SearchInformationInSection(const SearchInformationInSection& cSrc_);

	// オリジナルの文書長があるか
	bool isOriginalLength()
		{ return m_pOtherFile->isOriginalLength(); }
	// 特徴語があるか
	bool isFeatureSet()
		{ return m_pOtherFile->isFeatureSet(); }

	// 最大文書IDを得る
	DocumentID getMaxDocumentID();
	// 削除文書数を得る
	ModSize getExpungeDocumentCount() { return m_uiExpungeCount; }

	// 文書長を得る
	bool getDocumentLength(DocumentID id_, ModSize& length_);
	// オリジナルの文書長を得る
	bool getOriginalLength(DocumentID id_, ModSize& length_);
	// 挿入したユニット番号を得る
	bool getUnitNumber(DocumentID id_, int& unit_);
	// スコア調整値を得る
	bool getScoreValue(DocumentID id_, double& score_);
	// セションサイズを得る
	bool getSectionSize(DocumentID id_,
						ModVector<ModSize>& vecSectionSize_);
	// 特徴語リストを得る
	bool getFeatureSet(DocumentID id_,
					   FeatureSetPointer& pFeatureSet_);

	// コピーを得る
	SearchInformation* copy() const;

private:
	// 全文書数を得る
	ModSize getDocumentCountImpl();
	// 総文書長を得る
	ModUInt64 getTotalDocumentLengthImpl();
	
	// その他情報ファイル
	OtherInformationFile* m_pOtherFile;
	// その他情報ファイルインスタンスのオーナーか
	bool m_bOwner;

	// 削除数
	ModSize m_uiExpungeCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SEARCHINFORMATIONINSECTION_H

//
//  Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
