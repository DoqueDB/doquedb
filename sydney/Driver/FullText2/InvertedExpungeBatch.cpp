// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedExpungeBatch.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
#include "FullText2/InvertedExpungeBatch.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//  FUNCTION public
//  FullText2::InvertedExpungeBatch::InvertedExpungeBatch -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
// 	FullText2::InvertedSection& cSection_
//		転置ファイルセクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedExpungeBatch::InvertedExpungeBatch(InvertedSection& cSection_)
	: InvertedBatch(cSection_)
{
}

//
//  FUNCTION public
//  FullText2::InvertedExpungeBatch::~InvertedExpungeBatch -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedExpungeBatch::~InvertedExpungeBatch()
{
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeBatch::clear -- ファイルの内容をクリアする
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
InvertedExpungeBatch::clear()
{
	m_vecDocumentID.clear();
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeBatch::convertToBigDocumentID
//		-- 小転置の文書IDを大転置の文書IDに変換する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiSmallID_
//		小転置の文書ID
//
//	RETURN
//	FullText2::DocumentID
//		大転置の文書ID
//
//	EXCEPTIONS
//
DocumentID
InvertedExpungeBatch::convertToBigDocumentID(DocumentID uiSmallID_)
{
	DocumentID uiBigID = UndefinedDocumentID;
	if (uiSmallID_ != 0 && (uiSmallID_ - 1) < m_vecDocumentID.getSize())
	{
		uiBigID = m_vecDocumentID[uiSmallID_ - 1];
	}
	return uiBigID;
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeBatch::assignDocumentID
//		-- 小転置の文書IDを確保する
//
//	NOTES
//	この削除用小転置にデータを入力するときには、
//	呼び出し側で、本メソッドを実行し、小転置の文書IDを取得する必要がある
//	ListManager::insert 内で内部的に変換できない理由は以下の通り
//
//	エラー処理で expunge を実行する必要があるが、
//	大転置の文書IDから小転置の文書IDに変換するための
//	ベクターは保持していないので、エラー処理が行えなくなる
//	そのため呼び出し側で小転置の文書IDを保持する必要がある
//
//	ARGUMENT
//	FullText2::DocumentID uiBigDocumentID_
//		大転置の文書ID
//
//	RETURN
//	FullText2::DocumentID
//		新しく採番された小転置の文書ID
//
//	EXCEPTIONS
//
DocumentID
InvertedExpungeBatch::assignDocumentID(DocumentID uiBigDocumentID_)
{
	DocumentID uiSmallID = m_vecDocumentID.getSize();
	m_vecDocumentID.pushBack(uiBigDocumentID_);
	m_pListMap->addListSize(sizeof(DocumentID));
	return (uiSmallID + 1);
}

//
//	FUNCTION public
//	InvertedExpungeBatch::getAll -- すべての文書IDを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<FullText2::DocumentID>& vecID_
//		すべての文書IDがappendされる
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedExpungeBatch::getAll(Common::LargeVector<DocumentID>& vecID_)
{
	vecID_.insert(vecID_.end(),
				  m_vecDocumentID.begin(), m_vecDocumentID.end());
}

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
