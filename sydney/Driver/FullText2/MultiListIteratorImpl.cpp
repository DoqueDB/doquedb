// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIteratorImpl.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/MultiListIteratorImpl.h"

#include "FullText2/InvertedSection.h"
#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MultiListIteratorImpl::MultiListIteratorImpl -- コンストラクタ
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
//	なし
//
MultiListIteratorImpl::MultiListIteratorImpl()
	: MultiListIterator()
{
}

//
//	FUNCTION public
//	FullText2::MultiListIteratorImpl::~MultiListIteratorImpl -- デストラクタ
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
//	なし
//
MultiListIteratorImpl::~MultiListIteratorImpl()
{
}

//
//	FUNCTION public
//	FullText2::MultiListIteratorImpl::MultiListIteratorImpl
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MultiListIteratorImpl& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MultiListIteratorImpl::MultiListIteratorImpl(const MultiListIteratorImpl& src_)
	: MultiListIterator(src_)
{
}

//
//	FUNCTION public
//	FullText2::MultiListIteratorImpl::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListIterator*
//		コピー
//
//	EXCEPTIONS
//
ListIterator*
MultiListIteratorImpl::copy() const
{
	return new MultiListIteratorImpl(*this);
}

//
//	FUNCTION public
//	FullText2::MultiListIteratorImpl::getElement
//		-- ある文書IDの文書が格納されている要素の要素番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		探している文書の文書ID
//
// 	RETURN
//	int
//		要素番号。見つからない場合は -1 を返す
//
//	EXCEPTIONS
//
int
MultiListIteratorImpl::getElement(SearchInformation& cSearchInfo_,
								  DocumentID uiDocumentID_)
{
	int unit;
	if (cSearchInfo_.getUnitNumber(uiDocumentID_, unit) == false)
	{
		// 見つからないので、-1 を設定する
		unit = -1;
	}
	return unit;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
