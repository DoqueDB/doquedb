// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationConcatinate.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/SearchInformationConcatinate.h"

#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::SearchInformationConcatinate::SearchInformationConcatinate
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
SearchInformationConcatinate::
SearchInformationConcatinate(FileID& cFileID_)
	: SearchInformationArray(cFileID_),
	  m_ulTotalDocumentLength(Os::Limits<ModUInt64>::getMax())
{
}

//
//	FUNCTION public
//	FullText2::SearchInformationConcatinate::~SearchInformationConcatinate
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
SearchInformationConcatinate::~SearchInformationConcatinate()
{
}

//
//	FUNCTION public
//	FullText2::SearchInformationConcatinate::SearchInformationConcatinate
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SearchInformationConcatinate& cSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchInformationConcatinate::
SearchInformationConcatinate(const SearchInformationConcatinate& cSrc_)
	: SearchInformationArray(cSrc_),
	  m_ulTotalDocumentLength(cSrc_.m_ulTotalDocumentLength)
{
}

//
//	FUNCTION public
//	FullText2::SearchInformationConcatinate::getDocumentLength
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
SearchInformationConcatinate::getDocumentLength(DocumentID id_,
												ModSize& length_)
{
	length_ = 0;
	ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
	ModVector<SearchInformation*>::Iterator e = m_vecpElement.end();
	for (; i != e; ++i)
	{
		if (*i == 0) continue;
			
		ModSize l = 0;
		if ((*i)->getDocumentLength(id_, l) == false)
		{
			// 存在していない
			return false;
		}
		length_ += l;
	}
	return true;
}

//
//	FUNCTION public
//	FullText2::SearchInformationConcatinate::copy -- コピーを得る
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
SearchInformationConcatinate::copy() const
{
	return new SearchInformationConcatinate(*this);
}

//
//	FUNCTION protected
//	FullText2::SearchInformationConcatinate::geTotalDocumentLengthImpl
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
SearchInformationConcatinate::getTotalDocumentLengthImpl()
{
	if (m_ulTotalDocumentLength == Os::Limits<ModUInt64>::getMax())
	{
		// 初めてなので、計算する

		m_ulTotalDocumentLength = 0;
		ModVector<SearchInformation*>::Iterator i = m_vecpElement.begin();
		ModVector<SearchInformation*>::Iterator e = m_vecpElement.end();
		for (; i != e; ++i)
		{
			if (*i == 0) continue;
			
			m_ulTotalDocumentLength += (*i)->getTotalDocumentLengthImpl();
		}
	}
	
	return m_ulTotalDocumentLength;
}

//
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
