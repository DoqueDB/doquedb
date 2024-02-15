// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InfoFile.cpp -- 全文ファイル情報ファイル
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/IndexFile.h"
#include "FullText/OpenOption.h"
#include "FullText/FieldMask.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/FileID.h"

#include "Trans/Transaction.h"

#include "Schema/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

//
//	FUNCTION public
//	FullText::IndexFile::IndexFile -- コンストラクタ(1)
//
//	NOTES
//	遅延更新用
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFile::IndexFile(FullText::FileID& cFileID_)
	: Inverted::IndexFileSet(cFileID_),
	  m_cFileID(cFileID_), m_bBatch(false)
{
	m_pSearchCapsule = 0;
	m_pGetLocationCapsule = 0;
	m_pTokenizer = 0;
}


//
//	FUNCTION public
//	FullText::IndexFile::IndexFile -- コンストラクタ(2)
//
//	NOTES
//	転置ファイルが１つの場合
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFile::IndexFile(FullText::FileID& cFileID_,
					 const Os::Path& cPath_)
	: Inverted::IndexFileSet(cFileID_.getInverted(), cPath_),
	  m_cFileID(cFileID_), m_bBatch(false)
{
	m_pSearchCapsule = 0;
	m_pGetLocationCapsule = 0;
	m_pTokenizer = 0;
}

//
//	FUNCTION public
//	FullText::IndexFile::~IndexFile -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFile::~IndexFile()
{
	if (m_pTokenizer)
	{
		Inverted::InvertedFile::deleteTokenizer(m_pTokenizer);
		m_pTokenizer = 0;
	}
	delete m_pSearchCapsule, m_pSearchCapsule = 0;
	delete m_pGetLocationCapsule, m_pGetLocationCapsule = 0;
}

//
//	FUNCTION public
//	FullText::IndexFile::create -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
void
IndexFile::create(const Trans::Transaction& cTransaction_)
{
	Inverted::IndexFileSet::create(cTransaction_, m_cFileID);
}

//
//	FUNCTION public
//	FullText::Index::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
///
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::open(const Trans::Transaction& cTransaction_,
				const LogicalFile::OpenOption& cOption_)
{
	// FullTextに移動

	//
	//	【注意】
	//	転置ファイルのオープン・クローズは以下のように行う
	//	1. beginBatchInsert
	//	2. open
	//	...
	//	3. endBatchInsert
	//	4. close
	//
#if 0
	m_bBatch = cOption_.getBoolean(
			_SYDNEY_FULLTEXT_OPEN_PARAMETER_KEY(OpenOption::CreateIndex::Key));
#else
	m_bBatch = cOption_.getInteger(
			_SYDNEY_FULLTEXT_OPEN_PARAMETER_KEY(LogicalFile::OpenOption::KeyNumber::OpenMode)) 
				==  LogicalFile::OpenOption::OpenMode::Batch;
#endif

	Inverted::IndexFileSet::open(cTransaction_, cOption_, m_bBatch);
	m_pOpenOption = &cOption_;
	try
	{
		if (m_pTokenizer == 0)
			m_pTokenizer = getFullInvert()->createTokenizer();
	}
	catch (...)
	{
		Inverted::IndexFileSet::close(m_bBatch);
		_SYDNEY_RETHROW;
	}
}


//
//	FUNCTION public
//	FullText::IndexFile::close -- ファイルをクローズする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::close()
{
	Inverted::IndexFileSet::close(m_bBatch);
}

//
//	FUNCTION public
//	FullText::IndexFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	const ModUnicodeString& cstrKeyDocument_
//		各セクションを連結した文字列(キー)
//	const ModVector<ModLanguageSet>& vecKeyLanguage_
//		各セクションの言語情報(キー)
//	const ModVector<ModSize>& vecKeySectionOffset_
//		セクションオフセット(キー)
//	const ModUnicodeString& cstrValueDocument_
//		各セクションを連結した文字列(バリュー)
//	const ModVector<ModLanguageSet>& vecValueLanguage_
//		各セクションの言語情報(バリュー)
//	ModVector<ModSize>& vecValueSectionOffset_
//		セクションオフセット(バリュー)
//	ModInvertedFeatureList& vecValueFeature_
//		特徴語セット(バリュー)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::update(ModUInt32 uiTupleID_,
				  const ModUnicodeString& cstrKeyDocument_,
				  const ModVector<ModLanguageSet>& vecKeyLanguage_,
				  const ModVector<ModSize>& vecKeySectionOffset_,
				  const ModUnicodeString& cstrValueDocument_,
				  const ModVector<ModLanguageSet>& vecValueLanguage_,
				  ModVector<ModSize>& vecValueSectionOffset_,
				  ModInvertedFeatureList& vecValueFeature_)
{
	// 更新は、削除+挿入
	expunge(cstrKeyDocument_,
			vecKeyLanguage_,
			uiTupleID_,
			vecKeySectionOffset_);

	try
	{
		insert(cstrValueDocument_,
			   vecValueLanguage_,
			   uiTupleID_,
			   vecValueSectionOffset_,
			   vecValueFeature_);
	}
	catch (...)
	{
		try
		{
			// 元に戻す
			ModVector<ModSize> vecSectionOffset = vecKeySectionOffset_;
			insert(cstrKeyDocument_,
				   vecKeyLanguage_,
				   uiTupleID_,
				   vecSectionOffset,
				   vecValueFeature_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText::IndexFile::getSearchCapsule -- 検索器を得る
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::OptionDataFile* file_
//		オプションデータファイル
//
//	RETURN
//	Inverted::SearchCapsule&
//		検索器
//
//	EXCEPTIONS
//
Inverted::SearchCapsule&
IndexFile::getSearchCapsule(Inverted::OptionDataFile* file_)
{
	if (m_pSearchCapsule == 0)
	{
		m_pSearchCapsule = new Inverted::SearchCapsule(*m_pOpenOption,
													   file_,
													   this,
													   m_pTokenizer);
	}
	return *m_pSearchCapsule;
}

//
//	FUNCTION public
//	FullText::DelayIndexFile::getGetLocationCapsule
//		-- 位置情報検索器を得る
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::SearchCapsule* pCapsule_
//		検索カプセル
//
//	RETURN
//	Inverted::GetLocationCapsule&
//		位置情報検索器
//
//	EXCEPTIONS
//
Inverted::GetLocationCapsule&
IndexFile::getGetLocationCapsule(Inverted::SearchCapsule* pCapsule_)
{
	if (m_pGetLocationCapsule == 0)
	{
		m_pGetLocationCapsule = const_cast<Inverted::GetLocationCapsule* >
							(new Inverted::GetLocationCapsule(*pCapsule_));
	}
	return *m_pGetLocationCapsule;
}

//
//	FUNCTION public
//	FullText::IndexFile::clearSearchCapsule -- 検索器をクリアする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::clearSearchCapsule()
{
	delete m_pSearchCapsule, m_pSearchCapsule = 0;
	delete m_pGetLocationCapsule, m_pGetLocationCapsule = 0;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
