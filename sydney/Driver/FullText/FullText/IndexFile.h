// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_INDEXFILE_H
#define __SYDNEY_FULLTEXT_INDEXFILE_H

#include "FullText/Module.h"
#include "FullText/FileID.h"

#include "Inverted/SearchCapsule.h"
#include "Inverted/GetLocationCapsule.h"
#include "Inverted/IntermediateFile.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedFile.h"
#include "Inverted/IntermediateFileID.h"
#include "Inverted/IndexFile.h"
#include "Inverted/IndexFileSet.h"
#include "Inverted/OptionDataFile.h"
#include "Inverted/ModInvertedTypes.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Os/Path.h"


#include "ModUnicodeString.h"
#include "ModLanguageSet.h"
#include "ModVector.h"


_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::IndexFile --
//
//	NOTES
//
class IndexFile : public Inverted::IndexFileSet
{
private:

protected:
	// 検索器
	Inverted::SearchCapsule* m_pSearchCapsule;
	// 位置情報検索器を得る
	Inverted::GetLocationCapsule* m_pGetLocationCapsule;
	// オープンオプション
	const LogicalFile::OpenOption* m_pOpenOption;
	// tokenizer
	ModInvertedTokenizer* m_pTokenizer;
	// ファイルID
	FileID& m_cFileID;
	// バッチインサートか
	bool m_bBatch;
public:
	// コンストラクタ(遅延更新用)
	IndexFile(FullText::FileID& cFileID_);
	// コンストラクタ(非遅延更新=ファイルが一つ用)
	IndexFile(FullText::FileID& cFileID_,const Os::Path& cPath_);
	// デストラクタ
	virtual ~IndexFile();

	virtual void create(const Trans::Transaction& cTransaction_);
	virtual void open(const Trans::Transaction& cTransaction_,const LogicalFile::OpenOption& cOption_);
	virtual void close();

	virtual void verify(const Trans::Transaction& cTransaction_,
				Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_)
	{
		getFullInvert()->verify(cTransaction_,uiTreatment_,cProgress_);
	}
	virtual bool isAccessible(bool bForce_ = false)
	{
		return getFullInvert()->isAccessible( bForce_);
	}
	// マウントされているか調べる
	virtual bool isMounted(const Trans::Transaction& trans)
	{
		return getFullInvert()->isMounted(trans);
	}
	// 挿入する
	virtual void insert(const ModUnicodeString& cstrDocument_,
						const ModVector<ModLanguageSet>& vecLanguage_,
						ModUInt32 uiTupleID_,
						ModVector<ModSize>& vecSectionOffset_,
						ModInvertedFeatureList& vecFeature_)
	{
		getFullInvert()->insert(m_pTokenizer,
								cstrDocument_,
								vecLanguage_,
								uiTupleID_,
								vecSectionOffset_,
								vecFeature_);
	}
	// 削除する
	virtual void expunge(const ModUnicodeString& cstrDocument_,
						 const ModVector<ModLanguageSet>& vecLanguage_,
						 ModUInt32 uiTupleID_,
						 const ModVector<ModSize>& vecSectionOffset_)
	{
		getFullInvert()->expunge(m_pTokenizer,
								 cstrDocument_,
								 vecLanguage_,
								 uiTupleID_,
								 vecSectionOffset_);
	}
	// 更新する
	virtual void update(
		ModUInt32 uiTupleID_,
		const ModUnicodeString& cstrKeyDocument_,
		const ModVector<ModLanguageSet>& vecKeyLanguage_,
		const ModVector<ModSize>& vecKeySectionOffset_,
		const ModUnicodeString& cstrValueDocument_,
		const ModVector<ModLanguageSet>& vecValueLanguage_,
		ModVector<ModSize>& vecValueSectionOffset_,
		ModInvertedFeatureList& vecValueFeature_);
	
	// 検索オープンパラメータを得る
	static bool getSearchParameter(
		const FileID& cFileID_,
		const LogicalFile::TreeNodeInterface* pCondition_,
		LogicalFile::OpenOption& cOpenOption_)
	{
		return Inverted::InvertedFile::getSearchParameter(
			pCondition_,
			cFileID_.isLanguage(),
			cFileID_.isScoreField(),
			const_cast<FileID&>(cFileID_).getInverted(),
			cOpenOption_);
	}
	
	// 検索器を得る
	virtual Inverted::SearchCapsule&
	getSearchCapsule(Inverted::OptionDataFile* optionfile_);
	
	// 位置情報検索器を得る
	virtual Inverted::GetLocationCapsule&
	getGetLocationCapsule(Inverted::SearchCapsule* pCapsule_);

	// 検索器をクリアする
	virtual void clearSearchCapsule();
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_INDEXFILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
