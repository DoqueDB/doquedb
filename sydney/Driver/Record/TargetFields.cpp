// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TargetFields.cpp -- 選択フィールドのクラス
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/TargetFields.h"
#include "Record/Tools.h"
#include "Record/MetaData.h"

#include "Common/Assert.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING

using namespace Record;

//
//	FUNCTION public
//	Record::TargetFields::TargetFields -- コンストラクタ
//
//	NOTES
//	コンストラクト直後は全てのフィールドが選択された状態(プロジェクションが
//	適用されていない状態)になる。
//
//	ARGUMENTS
//	const ModSize arraySize_
//		確保する配列の要素数(必要な要素数より大きくても構わない)
//	RETURN
//		なし
//	EXCEPTIONS
//		メモリ領域不足	
//
TargetFields::TargetFields(const ModSize ulArraySize_)
	: m_vecPosition()
{
	m_vecPosition.reserve(ulArraySize_);
}

//
//	FUNCTION public
//	Record::TargetFields::~TargetFields -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//	RETURN
//		なし
//	EXCEPTIONS
//
//
TargetFields::~TargetFields()
{
}

//
// アクセッサ
//

//
//	FUNCTION public
//	Record::TargetFields::get -- 選択されたフィールドの番号を返す
//
//	NOTES
//	範囲を越えたアクセスをした場合の動作は不定
//
//	ARGUMENTS
//	const int index_
//		選択されたフィールド番号を格納した配列の添字
//
//	RETURN
//	選択されているフィールド番号
//
//	EXCEPTIONS
//
//
int
TargetFields::get(const int index_) const
{
	return m_vecPosition[index_].first;
}

//	FUNCTION public
//	Record::TargetFields::getIndex -- 選択されたフィールドの番号を返す
//
//	NOTES
//	範囲を越えたアクセスをした場合の動作は不定
//
//	ARGUMENTS
//	const int index_
//		選択されたフィールド番号を格納した配列の添字
//
//	RETURN
//	選択されているフィールド番号
//
//	EXCEPTIONS

int
TargetFields::getIndex(const int index_) const
{
	return m_vecPosition[index_].second;
}

//
//	FUNCTION public
//	Record::TargetFields::getSize -- 要素数を返す
//
//	NOTES
//	選択されたフィールド番号を格納した配列の要素数を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	要素数
//
//	EXCEPTIONS
//
//
int
TargetFields::getSize() const
{
	return m_vecPosition.getSize();
}


//
// マニピュレータ
//


//
//	FUNCTION public
//	Record::TargetFields::addFieldNumber -- プロジェクションによって選択されているフィールド番号を格納
//
//	NOTES
//	プロジェクションによって選択されているフィールド番号を格納する。
//	
// (使用上の注意)
// フィールド番号は昇順に格納していくこと
//
//	ARGUMENTS
//	const int iFieldID_
//		選択されているフィールド番号
//	const int iIndex_
//		選択されているフィールドの順序
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
TargetFields::addFieldNumber(int iFieldID_, int iIndex_)
{
//	; _SYDNEY_ASSERT(m_vecPosition.getSize() == 0
//					 || m_vecPosition.getBack().first < iFieldID_);
	if (!(m_vecPosition.getSize() == 0
					 || m_vecPosition.getBack().first < iFieldID_)) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	int iIndex = (iIndex_ ? iIndex_ : m_vecPosition.getSize());
	m_vecPosition.pushBack(ModPair<int, int>(iFieldID_, iIndex));
}

// TargetFieldsを作る
//static
void
TargetFields::
divide(TargetFields& cDirectTarget_,
	   TargetFields& cVariableTarget_,
	   TargetFields* pTargets_,
	   const MetaData& cMetaData_,
	   bool bIsUpdate_)
{
	// 更新でターゲットの指定がないとき
	// 与えられるデータにはオブジェクトIDが含まれないので
	// Indexから差し引いておく必要がある

	int iOffset = (!pTargets_ && bIsUpdate_) ? 1 : 0;

	TargetIterator cTargetIterator(pTargets_, &cMetaData_);
	do {
		Tools::FieldNum iFieldID = cTargetIterator.getNext();
		if (iFieldID == 0 && iOffset) {
			// iOffsetが1ということはオブジェクトIDは対象にならない
			continue;
		}
		// 可変長か否かで設定する対象が変わる
		if (cMetaData_.isVariable(iFieldID)) {
			cVariableTarget_.addFieldNumber(iFieldID, cTargetIterator.getIndex() - iOffset);
		} else {
			cDirectTarget_.addFieldNumber(iFieldID, cTargetIterator.getIndex() - iOffset);
		}
	} while (cTargetIterator.hasNext());
}

////////////////////
// TargetIterator //
////////////////////

TargetIterator::
TargetIterator(const TargetFields* pTarget_,
			   const MetaData* pMetaData_)
	: m_iTarget(0), m_pTarget(pTarget_)
{
	if (pTarget_) {
		m_funcNext = &TargetIterator::getNextTarget;
		m_funcIndex = &TargetIterator::getNextTargetIndex;
		m_iTargetMax = pTarget_->getSize();
	} else {
		m_funcNext = &TargetIterator::getNextID;
		m_funcIndex = &TargetIterator::getNextIndex;
		m_iTargetMax = pMetaData_->getFieldNumber();
	}
}

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
