// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationArray.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/SearchInformationArray.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::SearchInformationArray::SearchInformationArray
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformationArray::
SearchInformationArray(FileID& cFileID_)
	: SearchInformation(cFileID_), m_iNotNull(-1)
{
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::~SearchInformationArray
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
SearchInformationArray::~SearchInformationArray()
{
	ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
	for (; i != m_vecpElement.end(); ++i)
	{
		if (*i) delete *i;
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::SearchInformationArray
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SearchInformationArray& cSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformationArray::
SearchInformationArray(const SearchInformationArray& cSrc_)
	: SearchInformation(cSrc_), m_iNotNull(cSrc_.m_iNotNull)
{
	// メンバーをコピーする
	
	ModVector<SearchInformation*>::ConstIterator i
		= cSrc_.m_vecpElement.begin();
	for (; i != cSrc_.m_vecpElement.end(); ++i)
	{
		if (*i)
		{
			m_vecpElement.pushBack((*i)->copy());
		}
		else
		{
			m_vecpElement.pushBack(0);
		}
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::pushElement
//		-- 各要素用の検索情報クラスを加える
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation* pElement_
//		各要素用の検索情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformationArray::pushElement(SearchInformation* pElement_)
{
	if (m_iNotNull == -1 && pElement_ != 0)
	{
		// null ではない要素を覚えておく
		
		m_iNotNull = static_cast<int>(m_vecpElement.getSize());
	}
	
	m_vecpElement.pushBack(pElement_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getElement
//		-- 各要素用の検索情報クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//
//	RETURN
// 	SearchInformation&
//	   	指定された要素の検索情報クラス
//
//	EXCEPTIONS
//
SearchInformation&
SearchInformationArray::getElement(int n_)
{
	return *m_vecpElement[n_];
}

//
//	FUNCTION public
//	FullText2::SearchInformation::isElementNull
//		-- 各要素用の検索情報クラスがnullかどうかを調べる
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//
//	RETURN
//	bool
//		各要素用の検索情報クラスがnullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SearchInformationArray::isElementNull(int n_)
{
	return (m_vecpElement[n_] == 0) ? true : false;
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::isOriginalLength
//		-- オリジナルの文書長があるか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		オリジナルの文書長がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SearchInformationArray::isOriginalLength()
{
	// オリジナル文書長があるかどうかはすべての要素で同じ
	return getElement(m_iNotNull).isOriginalLength();
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::isFeatureSet
//		-- 特徴語があるか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		特徴語がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SearchInformationArray::isFeatureSet()
{
	// 特徴語があるかどうかはすべての要素で同じ
	return getElement(m_iNotNull).isFeatureSet();
}

//
//	FUNCTION public
//	FullText2::SearchInformaionArray::getMaxDocumentID
//		-- 登録されている文書の最大の文書IDを得る
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
SearchInformationArray::getMaxDocumentID()
{
	// 最大文書IDはすべての要素で同じ
	return getElement(m_iNotNull).getMaxDocumentID();
}

//
//	FUNCTION public
//	FullText2::SearchInformaionArray::getExpungeDocumentCount
//		-- 削除されている文書数を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//		削除文書数
//
//	EXCEPTIONS
//
DocumentID
SearchInformationArray::getExpungeDocumentCount()
{
	// 削除文書数はすべての要素で同じ
	return getElement(m_iNotNull).getExpungeDocumentCount();
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getDocumentLength
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
SearchInformationArray::getDocumentLength(DocumentID id_,
										  ModSize& length_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getOriginalLength
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
SearchInformationArray::getOriginalLength(DocumentID id_,
										  ModSize& length_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getUnitNumber
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
SearchInformationArray::getUnitNumber(DocumentID id_, int& unit_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getScoreValue
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
SearchInformationArray::getScoreValue(DocumentID id_, double& score_)
{
	// スコア調整値はすべての要素で同じ
	return getElement(m_iNotNull).getScoreValue(id_, score_);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getSectionSize
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
SearchInformationArray::
getSectionSize(DocumentID id_, ModVector<ModSize>& vecSectionSize_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::getFeatureSet
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
SearchInformationArray::getFeatureSet(DocumentID id_,
									  FeatureSetPointer& pFeatureSet_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::copy -- コピーを得る
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
SearchInformationArray::copy() const
{
	return new SearchInformationArray(*this);
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::setDocumentFrequency
//		-- DF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//	double dblDocumentFrequency_
//		文書頻度
//	double dblTotalTermFrequency_
//		総文書内頻度
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformationArray::setDocumentFrequency(const ModUnicodeString& tea_,
											 double dblDocumentFrequency_,
											 double dblTotalTermFrequency_)
{
	// 自身に登録する

	SearchInformation::setDocumentFrequency(tea_,
											dblDocumentFrequency_,
											dblTotalTermFrequency_);

	// 子に登録する

	ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
	for (; i != m_vecpElement.end(); ++i)
	{
		if (*i)
		{
			(*i)->setDocumentFrequency(tea_,
									   dblDocumentFrequency_,
									   dblTotalTermFrequency_);
		}
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::setDocumentFrequency
//		-- DF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
// 	FullText2::OperatorTermNode::Frequency& frequency_
//		頻度情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformationArray::
setDocumentFrequency(const ModUnicodeString& tea_,
					 OperatorTermNode::Frequency& frequency_)
{
	// 自身に登録する
	
	double dblDocumentFrequency
		= static_cast<double>(frequency_.m_uiDocumentFrequency);
	double dblTotalTermFrequency
		= static_cast<double>(frequency_.m_ulTotalTermFrequency);

	SearchInformation::setDocumentFrequency(tea_,
											dblDocumentFrequency,
											dblTotalTermFrequency);

	// 子に登録する

	ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
	ModVector<OperatorTermNode::Frequency>::Iterator j
		= frequency_.getChild(*this).begin();
	for (; i != m_vecpElement.end(); ++i, ++j)
	{
		if (*i)
		{
			double dblDocumentFrequency
				= static_cast<double>((*j).m_uiDocumentFrequency);
			double dblTotalTermFrequency
				= static_cast<double>((*j).m_ulTotalTermFrequency);
			
			(*i)->setDocumentFrequency(tea_,
									   dblDocumentFrequency,
									   dblTotalTermFrequency);
		}
	}
}

//
//	FUNCTION public
//	FullText2::SearchInformationArray::setQueryTermFrequency
//		-- 検索文内頻度を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& tea_
//		検索語ノードを表すtea構文
//	double dblQueryTermFrequency_
//		検索文内頻度
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SearchInformationArray::setQueryTermFrequency(const ModUnicodeString& tea_,
											  double dblQueryTermFrequency_)
{
	// 自身に登録する

	SearchInformation::setQueryTermFrequency(tea_,
											 dblQueryTermFrequency_);

	// 子に登録する

	ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
	for (; i != m_vecpElement.end(); ++i)
	{
		if (*i)
		{
			(*i)->setQueryTermFrequency(tea_,
										dblQueryTermFrequency_);
		}
	}
}

//
//	FUNCTION protected
//	FullText2::SearchInformationArray::getDocumentCountImpl
//		-- 登録されている文書数を得る
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
SearchInformationArray::getDocumentCountImpl()
{
	// 登録文書数はすべての要素で同じ
	return getElement(m_iNotNull).getDocumentCount();
}

//
//	FUNCTION protected
//	FullText2::SearchInformationArray::geTotalDocumentLengthImpl
//		-- 登録されている文書の総文書長を得る
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
SearchInformationArray::getTotalDocumentLengthImpl()
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
