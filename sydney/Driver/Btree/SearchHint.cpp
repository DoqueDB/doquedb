// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchHint.cpp -- 検索ヒントクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Btree/SearchHint.h"
#include "Btree/HintTools.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"
#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"
#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::SearchHint::LocalElementNumLimit -- ローカル配列の要素数上限
//
//	NOTES
//	ローカル配列の要素数上限。
//
// static
const int
SearchHint::LocalElementNumLimit = 10;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::SearchHint::SearchHint -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
SearchHint::SearchHint()
	: m_FieldNumber(0),
	  m_LastKeyFieldIndex(0),
	  m_LastLinkKeyFieldIndex(0),
	  m_FieldIndexArray(0),
	  m_FieldIndexArrayAllocateSize(0),
	  m_FieldTypeArray(0),
	  m_FieldTypeArrayAllocateSize(0),
	  m_IsFixedArray(0),
	  m_IsFixedArrayAllocateSize(0),
	  m_StartConditionArray(0),
	  m_StartConditionArrayAllocateSize(0),
	  m_StartConditionOffsetArray(0),
	  m_StartConditionOffsetArrayAllocateSize(0),
	  m_StartOperatorArray(0),
	  m_StartOperatorArrayAllocateSize(0),
	  m_MultiNumberArray(0),
	  m_MultiNumberArrayAllocateSize(0),
	  m_StopOperator(LogicalFile::TreeNodeInterface::Undefined),
	  m_ByKey(false),
	  m_ExistSeparateKey(false),
	  m_LastStartOperatorIsEquals(false),
	  m_LastIsSpan(false),
	  m_ConditionType(ConditionType::Undefined)
	  ,m_VoidSearch(false)
{
	// 検索終了条件を初期化する

	(void) Os::Memory::reset(m_StopCondition, sizeof(m_StopCondition));

	//
	// 各ローカル配列の各要素を初期化する
	//

	for (int i = 0; i < SearchHint::LocalElementNumLimit; i++)
	{
		// 検索対象フィールドインデックス配列
		this->m_FieldIndexLocalArray[i] = 0;

		// 検索対象フィールドタイプ配列
		this->m_FieldTypeLocalArray[i] = Common::DataType::Undefined;

		// 検索対象フィールド固定長フラグ配列
		this->m_IsFixedLocalArray[i] = false;

		// 検索開始条件へのオフセット配列
		this->m_StartConditionOffsetLocalArray[i] = 0;

		// 検索開始条件の比較演算子配列
		this->m_StartOperatorLocalArray[i] =
			LogicalFile::TreeNodeInterface::Undefined;

		// 比較結果への乗数配列
		this->m_MultiNumberLocalArray[i] = 0;
	}

	// 検索開始条件配列を初期化する

	(void) Os::Memory::reset(
		m_StartConditionLocalArray, sizeof(m_StartConditionLocalArray));
}

//
//	FUNCTION public
//	Btree::SearchHint::~SearchHint -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
SearchHint::~SearchHint()
{
	// 確保した各配列を解放する
	this->clear(false); // 初期化は不要
}

//
//	FUNCTION public
//	Btree::SearchHint::set -- 検索ヒントを設定する
//
//	NOTES
//	検索のためのヒントを設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&						FileParameter_
//		ファイルパラメータへの参照
//	const Btree::OpenParameter::SearchCondition&	SearchCondition_
//		検索条件への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
SearchHint::set(
	const FileParameter&					FileParameter_,
	const OpenParameter::SearchCondition&	SearchCondition_)
{
	// （既に確保してあるかもしれないので）
	// 確保した各配列を解放する
	this->clear(true); // 初期化もする

	// getSearchParameter() の時点で既に検索結果が'φ'だと判断できているか。
	m_VoidSearch = SearchCondition_.m_VoidSearch;
	if (m_VoidSearch) return;// 以後のメンバーは設定しない。
	
	// 検索対象フィールド数を設定する
	this->m_FieldNumber = SearchCondition_.m_cFieldIndexArray.getCount();

	// 検索条件タイプを設定する
	this->m_ConditionType =
		(this->m_FieldNumber == 1) ?
			ConditionType::Single : ConditionType::Multi;

	// 各配列へのポインタを設定する
	this->setArrayPointer();

	// 各フラグを初期化しておく
	this->m_ByKey = true;
	this->m_ExistSeparateKey = false;
	this->m_LastStartOperatorIsEquals = true;

	int	offset = 0;

	for (int i = 0; i < this->m_FieldNumber; i++)
	{
		// 各検索対象フィールドについてのヒントを設定する…

		// 検索対象フィールドインデックスを取得する
		int	fieldIndex = SearchCondition_.m_cFieldIndexArray.getElement(i);

		// 取得した検索対象フィールドインデックスを
		// そのまま配列に記録する
		*(this->m_FieldIndexArray + i) = fieldIndex;

		if (fieldIndex == 0)
		{
			// オブジェクトIDによる検索であった…

			; _SYDNEY_ASSERT(i == 0);

			this->m_ByKey = false;
		}

		if (i > 0 &&
			fieldIndex > (*(this->m_FieldIndexArray + (i - 1)) + 1))
		{
			// 先頭キーフィールドから連続している対象キーフィールドから
			// 離れた対象キーフィールドがあった…

			// 例えば、こんな検索や、
			//                  ↓                  ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＶＡＬ１│
			//     └───┴────┴────┴────┴────┴─
			// こんな検索ではm_ExistSeparateKeyはtrueとなり、
			//                  ↓        ↓                  ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＫＥＹ４│
			//     └───┴────┴────┴────┴────┴─

			// こんな検索や、
			//                  ↓        ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＶＡＬ１│
			//     └───┴────┴────┴────┴────┴─
			// こんな検索ではfalseとなる。
			//                  ↓        ↓        ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＫＥＹ４│
			//     └───┴────┴────┴────┴────┴─
			// ※ "↓"が、Fetch対象キーフィールド

			this->m_ExistSeparateKey = true;
		}
		else
		{
			// 検索対象キーフィールドが
			// 先頭キーフィールドから連続している…

			// 連続した検索対象キーフィールドのうちで
			// 最後の検索対象キーフィールドのインデックスを更新する
			this->m_LastLinkKeyFieldIndex = fieldIndex;
		}

		// 検索対象フィールドのフィールドタイプを取得する
		Common::DataType::Type	fieldType =
			*(FileParameter_.m_FieldTypeArray + fieldIndex);

		// 取得したフィールドタイプをそのまま配列に記録する
		*(this->m_FieldTypeArray + i) = fieldType;

		// 検索対象フィールドは固定長フィールドか？
		bool	isFixed =
			*(FileParameter_.m_IsFixedFieldArray + fieldIndex);

		// 固定長フィールドかどうかを配列に記録する
		*(this->m_IsFixedArray + i) = isFixed;

		// 検索開始条件の比較演算子を取得する
		LogicalFile::TreeNodeInterface::Type	startOpe =
			static_cast<LogicalFile::TreeNodeInterface::Type>(
				SearchCondition_.m_cStartOpeArray.getElement(i));

		// 取得した検索開始条件の比較演算子をそのまま配列に記録する
		*(this->m_StartOperatorArray + i) = startOpe;

		if (startOpe != LogicalFile::TreeNodeInterface::Equals)
		{
			// 取得した検索開始条件の比較演算子がEqualsではなかった…

			// 上のことを示すフラグを設定する
			// （最終検索対象フィールドへの検索開始条件が
			// 　Equalsかどうかを示すフラグをOFFにする。）
			this->m_LastStartOperatorIsEquals = false;
		}

		// 検索開始条件がEqualsToNull以外の場合には、
		// 検索開始条件を設定するため、
		// 配列のオフセットを設定する
		// ※ 検索開始条件を設定するのは、固定長フィールドについてのみ。
		int	setOffset = -1;
		if (isFixed &&
			startOpe != LogicalFile::TreeNodeInterface::EqualsToNull)
		{
			setOffset = offset;
		}

		// 検索開始条件のオフセットを配列に記録する
		*(this->m_StartConditionOffsetArray + i) = setOffset;

		if (setOffset != -1)
		{
			// 検索対象フィールドが固定長フィールドで、
			// しかも、そのフィールドに対する検索開始条件が
			// EqualsToNull以外であった…

			// 検索対象フィールドの検索開始条件を取得する
			Common::Data*	startCondition =
				SearchCondition_.m_cStartArray.getElement(i).get();

			; _SYDNEY_ASSERT(startCondition != 0);

			// 検索開始条件設定先へのポインタを設定する
			char*	setPos = this->m_StartConditionArray + offset;

			// 検索対象フィールドの検索開始条件を設定する
			offset += HintTools::setFixedData(fieldType,
											  startCondition,
											  setPos);
		}

		// 比較結果への乗数を配列に記録する
		*(this->m_MultiNumberArray + i) =
			*(FileParameter_.m_MultiNumberArray + fieldIndex);

	} // end for i

	this->m_LastKeyFieldIndex =
		*(this->m_FieldIndexArray + (this->m_FieldNumber - 1));

	if (SearchCondition_.m_cStopArray.getCount() > 0)
	{
		// （最後の検索対象フィールドへの検索条件が）
		// 範囲指定である…

		// 上のことを示すフラグを設定する
		this->m_LastIsSpan = true;

		// 最終検索対象フィールドのインデックスを求める
		int	last = this->m_FieldNumber - 1;

		// 最終検索対象フィールドのタイプを取得する
		Common::DataType::Type	fieldType =
			*(this->m_FieldTypeArray + last);

		// 最終検索対象フィールドは固定長フィールドか？
		bool	isFixed =
			(FileCommon::DataManager::isVariable(fieldType) == false);

		// 検索終了条件の比較演算子を取得し、設定する
		this->m_StopOperator =
			static_cast<LogicalFile::TreeNodeInterface::Type>(
				SearchCondition_.m_cStopOpeArray.getElement(last));

		if (isFixed)
		{
			// 最終検索対象フィールドが固定長フィールドであった…
			// ※ ここでは、検索開始条件を設定するときのように
			// 　 EqualsToNull以外かどうかというチェックは不要。
			// 　 範囲指定なので、EqualsToNullやEqualsという
			// 　 比較演算子は設定されていないはず。

			// 検索対象フィールドの検索終了条件を取得する
			Common::Data*	stopCondition =
				SearchCondition_.m_cStopArray.getElement(last).get();

			; _SYDNEY_ASSERT(stopCondition != 0);

			// 検索対象フィールドの検索終了条件を設定する
			HintTools::setFixedData(fieldType,
									stopCondition,
									this->m_StopCondition);
		}
	}
	else
	{
		// （最後の検索対象フィールドへの検索条件が）
		// 範囲指定ではない…

		// 上のことを示すフラグを設定する
		this->m_LastIsSpan = false;

		// 検索終了条件の比較演算子にはUndefinedを設定しておく
		this->m_StopOperator = LogicalFile::TreeNodeInterface::Undefined;
	}

	if (*this->m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::Like)
	{
		this->setLikeSearchCondition(SearchCondition_);
	}
}

//
//	FUNCTION public
//	Btree::SearchHint::compareToFixedStartCondition --
//		検索開始条件と検索対象の固定長フィールド値を比較する
//
//	NOTES
//	引数FieldValue_が指し示す領域に記録されている
//	固定長フィールドの値と検索開始条件を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	FieldValue_
//		検索対象となる固定長フィールドの値が
//		記録されている領域へのポインタ
//	const int	ArrayIndex_
//		検索ヒントの各配列のインデックス
//
//	RETURN
//	int
//		< 0 : 検索条件の方がキー値順で前方
//		= 0 : 検索条件とフィールドの値が等しい
//		> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
SearchHint::compareToFixedStartCondition(const void*	FieldValue_,
										 const int		ArrayIndex_) const
{
	; _SYDNEY_ASSERT(ArrayIndex_ >= 0 &&
					 ArrayIndex_ < this->m_FieldNumber);

	return
		HintTools::compareToFixedData(
			*(this->m_FieldTypeArray + ArrayIndex_),
			this->m_StartConditionArray,
			this->m_StartConditionOffsetArray,
			this->m_MultiNumberArray,
			ArrayIndex_,
			FieldValue_);
}

//
//	FUNCTION public
//	Btree::SearchHint::compareToFixedStopCondition --
//		検索終了条件と検索対象の固定長フィールド値を比較する
//
//	NOTES
//		引数FieldValue_が指し示す領域に記録されている
//	固定長フィールドの値と検索終了条件を比較し、
//	比較結果を返す。
//
//	ARGUMENTS
//	const void*	FieldValue_
//		検索対象となる固定長フィールドの値が
//		記録されている領域へのポインタ
//
//	RETURN
//	int
//		< 0 : 検索条件の方がキー値順で前方
//		= 0 : 検索条件とフィールドの値が等しい
//		> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
SearchHint::compareToFixedStopCondition(const void*	FieldValue_) const
{
	; _SYDNEY_ASSERT(this->m_FieldNumber > 0);

	int	lastArrayIndex = this->m_FieldNumber - 1;

	return
		HintTools::compareToFixedData(
			*(this->m_FieldTypeArray + lastArrayIndex),
			this->m_StopCondition,
			0,
			this->m_MultiNumberArray,
			lastArrayIndex,
			FieldValue_);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::SearchHint::setLikeSearchCondition --
//		likeによる文字列検索のためのヒントを設定する
//
//	NOTES
//	likeによる文字列検索のためのヒントを設定する。
//
//	ARGUMENTS
//	const OpenParameter::SearchCondition&	SearchCondition_
//		検索条件への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
SearchHint::setLikeSearchCondition(
	const OpenParameter::SearchCondition&	SearchCondition_)
{
	Common::Data*	conditionData =
		SearchCondition_.m_cStartArray.getElement(0).get();

	; _SYDNEY_ASSERT(conditionData != 0);

	; _SYDNEY_ASSERT(
		conditionData->getType() == Common::DataType::String);

	Common::StringData*	conditionStringData =
		_SYDNEY_DYNAMIC_CAST(Common::StringData*, conditionData);

	this->m_PatternString = conditionStringData->getString();

	this->m_LikeSearchCondition.clear();

	ModSize	numChar = this->m_PatternString.getLength();

	for (ModSize i = 0; i < numChar; i++)
	{
		ModUnicodeChar	patternChar = this->m_PatternString[i];
		if (patternChar == Common::UnicodeChar::usPercent ||
			patternChar == Common::UnicodeChar::usLowLine)
		{
			break;
		}

		if (SearchCondition_.m_SetEscape &&
			patternChar == SearchCondition_.m_Escape)
		{
			i++;

			if (i == numChar)
			{
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}

			patternChar = this->m_PatternString[i];
		}

		this->m_LikeSearchCondition.append(patternChar);
	}
}

//
//	FUNCTION private
//	Btree::SearchHint::clear -- 確保した各配列を解放する
//
//	NOTES
//	自身が保持しているローカルな配列では収まりきれない検索条件で、
//	ヒープ領域に確保した配列用の領域があれば、その領域を解放する。
//
//	ARGUMENTS
//	const bool	DoInit_
//		データメンバの再初期化も行うかどうか
//			true  : 再初期化も行う
//			false : 再初期化は行わない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
SearchHint::clear(const bool	DoInit_)
{
	if (this->m_FieldNumber <= SearchHint::LocalElementNumLimit)
	{
		// 自身のローカルは配列で対応していた…

		return;
	}

	// 検索対象フィールドインデックス配列

	; _SYDNEY_ASSERT(this->m_FieldIndexArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_FieldIndexArray != this->m_FieldIndexLocalArray);

	ModDefaultManager::free(this->m_FieldIndexArray,
							this->m_FieldIndexArrayAllocateSize);

	// 検索対象フィールドタイプ配列

	; _SYDNEY_ASSERT(this->m_FieldTypeArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_FieldTypeArray != this->m_FieldTypeLocalArray);

	ModDefaultManager::free(this->m_FieldTypeArray,
							this->m_FieldTypeArrayAllocateSize);

	// 検索対象フィールド固定長フラグ配列
	; _SYDNEY_ASSERT(this->m_IsFixedArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_IsFixedArray != this->m_IsFixedLocalArray);

	ModDefaultManager::free(this->m_IsFixedArray,
							this->m_IsFixedArrayAllocateSize);

	// 検索開始条件配列

	; _SYDNEY_ASSERT(this->m_StartConditionArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_StartConditionArray != this->m_StartConditionLocalArray);

	ModDefaultManager::free(this->m_StartConditionArray,
							this->m_StartConditionArrayAllocateSize);

	// 検索開始条件へのオフセット配列

	; _SYDNEY_ASSERT(this->m_StartConditionOffsetArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_StartConditionOffsetArray !=
					 this->m_StartConditionOffsetLocalArray);

	ModDefaultManager::free(this->m_StartConditionOffsetArray,
							this->m_StartConditionOffsetArrayAllocateSize);

	// 検索開始条件の比較演算子配列

	; _SYDNEY_ASSERT(this->m_StartOperatorArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_StartOperatorArray != this->m_StartOperatorLocalArray);

	ModDefaultManager::free(this->m_StartOperatorArray,
							this->m_StartOperatorArrayAllocateSize);

	// 比較結果への乗数配列

	; _SYDNEY_ASSERT(this->m_MultiNumberArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_MultiNumberArray != this->m_MultiNumberLocalArray);

	ModDefaultManager::free(this->m_MultiNumberArray,
							this->m_MultiNumberArrayAllocateSize);

	if (DoInit_)
	{
		// 検索対象フィールドインデックス配列
		this->m_FieldIndexArray = 0;
		this->m_FieldIndexArrayAllocateSize = 0;

		// 検索対象フィールドタイプ配列
		this->m_FieldTypeArray = 0;
		this->m_FieldTypeArrayAllocateSize = 0;

		// 検索対象フィールド固定長フラグ配列
		this->m_IsFixedArray = 0;
		this->m_IsFixedArrayAllocateSize = 0;

		// 検索開始条件配列
		this->m_StartConditionArray = 0;
		this->m_StartConditionArrayAllocateSize = 0;

		// 検索開始条件へのオフセット配列
		this->m_StartConditionOffsetArray = 0;
		this->m_StartConditionOffsetArrayAllocateSize = 0;

		// 検索開始条件の比較演算子配列
		this->m_StartOperatorArray = 0;
		this->m_StartOperatorArrayAllocateSize = 0;

		// 比較結果への乗数配列
		this->m_MultiNumberArray = 0;
		this->m_MultiNumberArrayAllocateSize = 0;
	}
}

//
//	FUNCTION private
//	Btree::SearchHint::setArrayPointer -- 各配列へのポインタを設定する
//
//	NOTES
//	SearchHintクラスのインスタンス自身は、
//	検索対象フィールドが10フィールドまでに対応できる配列は、
//	保持している。
//	もし検索対象フィールドとして11フィールド以上が
//	指定されているならば、それらでは対応しきれないので、
//	ヒープ領域に配列のための領域を確保する。
//	このメソッドでは、ローカルな配列またはヒープ領域に確保した配列
//	いずれかへの各ポインタを設定する。
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
void
SearchHint::setArrayPointer()
{
	; _SYDNEY_ASSERT(this->m_FieldNumber > 0);

	if (this->m_FieldNumber > SearchHint::LocalElementNumLimit)
	{
		// 自身のローカルな配列では対応しきれない…

		// ヒープ領域に各配列用の領域を確保し、
		// ポインタをそれらを指すように設定する。

		// 検索対象フィールドインデックス配列

		this->m_FieldIndexArrayAllocateSize =
			sizeof(int) * this->m_FieldNumber;

		this->m_FieldIndexArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_FieldIndexArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldIndexArray,
								   this->m_FieldIndexArrayAllocateSize);

		// 検索対象フィールドタイプ配列

		this->m_FieldTypeArrayAllocateSize =
			sizeof(Common::DataType::Type) * this->m_FieldNumber;

		this->m_FieldTypeArray =
			static_cast<Common::DataType::Type*>(
				ModDefaultManager::allocate(
					this->m_FieldTypeArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldTypeArray,
								   this->m_FieldTypeArrayAllocateSize);

		// 検索対象フィールド固定長フラグ配列

		this->m_IsFixedArrayAllocateSize =
			sizeof(bool) * this->m_FieldNumber;

		this->m_IsFixedArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsFixedArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_IsFixedArray,
								   this->m_IsFixedArrayAllocateSize);

		// 検索開始条件配列

		this->m_StartConditionArrayAllocateSize =
			sizeof(int) * 7 * this->m_FieldNumber;

		this->m_StartConditionArray =
			static_cast<char*>(
				ModDefaultManager::allocate(
					this->m_StartConditionArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_StartConditionArray,
			this->m_StartConditionArrayAllocateSize);

		// 検索開始条件へのオフセット配列

		this->m_StartConditionOffsetArrayAllocateSize =
			sizeof(int) * this->m_FieldNumber;

		this->m_StartConditionOffsetArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_StartConditionOffsetArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_StartConditionOffsetArray,
			this->m_StartConditionOffsetArrayAllocateSize);

		// 検索開始条件の比較演算子配列

		this->m_StartOperatorArrayAllocateSize =
			sizeof(LogicalFile::TreeNodeInterface::Type) *
			this->m_FieldNumber;

		this->m_StartOperatorArray =
			static_cast<LogicalFile::TreeNodeInterface::Type*>(
				ModDefaultManager::allocate(
					this->m_StartOperatorArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_StartOperatorArray,
			this->m_StartOperatorArrayAllocateSize);

		// 比較結果への乗数配列

		this->m_MultiNumberArrayAllocateSize =
			sizeof(int) * this->m_FieldNumber;

		this->m_MultiNumberArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_MultiNumberArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_MultiNumberArray,
								   this->m_MultiNumberArrayAllocateSize);
	}
	else
	{
		// 自身のローカルな配列で対応できる…

		// では、ポインタをローカルは配列を指すように設定する。

		// 検索対象フィールドインデックス配列
		this->m_FieldIndexArray = this->m_FieldIndexLocalArray;

		// 検索対象フィールドタイプ配列
		this->m_FieldTypeArray = this->m_FieldTypeLocalArray;

		// 検索対象フィールド固定長フラグ配列
		this->m_IsFixedArray = this->m_IsFixedLocalArray;

		// 検索開始条件配列
		this->m_StartConditionArray = this->m_StartConditionLocalArray;

		// 検索開始条件へのオフセット配列
		this->m_StartConditionOffsetArray =
			this->m_StartConditionOffsetLocalArray;

		// 検索開始条件の比較演算子配列
		this->m_StartOperatorArray = this->m_StartOperatorLocalArray;

		// 比較結果への乗数配列
		this->m_MultiNumberArray = this->m_MultiNumberLocalArray;
	}
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
