// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationInSection.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/SearchInformationInSection.h"
#include "FullText2/OtherInformationFile.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::SearchInformationInSection
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FileID& cFileID_
//		ファイルID
//	FullText2::OtherInformationFile* pOtherFile_
//		その他情報ファイル
//	ModSize uiExpungeCount_
//		削除文書数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformationInSection::
SearchInformationInSection(FileID& cFileID_, OtherInformationFile* pOtherFile_,
						   ModSize uiExpungeCount_)
	: SearchInformation(cFileID_),
	  m_pOtherFile(pOtherFile_), m_bOwner(false),
	  m_uiExpungeCount(uiExpungeCount_)
{
	// その他情報ファイルのインスタンスのオーナーは InvertedSection である
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::~SearchInformationInSection
//		-- デストラクタ
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
SearchInformationInSection::~SearchInformationInSection()
{
	if (m_bOwner)
	{
		m_pOtherFile->flushAllPages();
		m_pOtherFile->close();
		delete m_pOtherFile;
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::SearchInformationInSection
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SearchInformationInSection& cSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformationInSection::
SearchInformationInSection(const SearchInformationInSection& cSrc_)
	: SearchInformation(cSrc_), m_pOtherFile(0), m_bOwner(true),
	  m_uiExpungeCount(cSrc_.m_uiExpungeCount)
{
	// コピーしたので、インスタンスのオーナーとなる
	
	m_pOtherFile = cSrc_.m_pOtherFile->copy();
}

//
//	FUNCTION public
//	FullText2::SearchInformaionInSection::getMaxDocumentID
//		-- このセクションに登録されている文書の最大の文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	FullText2::DocumentID
//		最大文書ID
//
//	EXCEPTIONS
//
DocumentID
SearchInformationInSection::getMaxDocumentID()
{
	return m_pOtherFile->getLastDocumentID();
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getDocumentLength
//		-- 文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		文書長を得るデータの文書ID
//	ModSize& length_
//		取得した長さ
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getDocumentLength(DocumentID id_, ModSize& length_)
{
	return m_pOtherFile->getDocumentLength(id_, length_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getOriginalLength
//		-- オリジナル文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		オリジナル文書長を得るデータの文書ID
//	ModSize& length_
//		取得した長さ
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getOriginalLength(DocumentID id_, ModSize& length_)
{
	return m_pOtherFile->getOriginalLength(id_, length_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getUnitNumber
//		-- 挿入したユニット番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		挿入したユニット番号を得るデータの文書ID
//	int& unit_
//		ユニット番号
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getUnitNumber(DocumentID id_, int& unit_)
{
	return m_pOtherFile->getUnitNumber(id_, unit_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getScoreValue
//		-- スコア調整値を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		スコア調整値を得るデータの文書ID
//	double& score
//		スコア調整値
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getScoreValue(DocumentID id_, double& score_)
{
	return m_pOtherFile->getScoreData(id_, score_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getSectionSize
//		-- セクションサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		セクションサイズを得るデータの文書ID
//	ModVector<ModSize>& vecSectionSize_
//		セクションサイズ
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getSectionSize(DocumentID id_,
										   ModVector<ModSize>& vecSectionSize_)
{
	return m_pOtherFile->getSectionSize(id_, vecSectionSize_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::getFeatureSet
//		-- 特徴語リストを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID id_
//		特徴語リストを得るデータの文書ID
//	FullText2::FeatureSetPointer& pFeatureSet_
//		特徴語リストへのポインタ
//
//	RETURN
//	bool
//		データが存在しない場合は false を返す
//
//	EXCEPTIONS
//
bool
SearchInformationInSection::getFeatureSet(DocumentID id_,
										  FeatureSetPointer& pFeatureSet_)
{
	return m_pOtherFile->getFeatureSet(id_, pFeatureSet_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationInSection::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SerchInformation*
//		コピーしたオブジェクト
//
//	EXCEPTIONS
//
SearchInformation*
SearchInformationInSection::copy() const
{
	return new SearchInformationInSection(*this);
}

//
//	FUNCTION protected
//	FullText2::SearchInformationInSection::getDocumentCountImpl
//		-- このセクションに登録されている文書数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書数
//
//	EXCEPTIONS
//
ModSize
SearchInformationInSection::getDocumentCountImpl()
{
	return m_pOtherFile->getCount();
}

//
//	FUNCTION protected
//	FullText2::SearchInformationInSection::geTotalDocumentLengthImpl
//		-- このセクションに登録されている文書の総文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//	   	総文書長
//
//	EXCEPTIONS
//
ModUInt64
SearchInformationInSection::getTotalDocumentLengthImpl()
{
	return m_pOtherFile->getTotalDocumentLength();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
