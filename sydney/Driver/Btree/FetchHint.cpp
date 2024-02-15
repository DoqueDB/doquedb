// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FetchHint.cpp -- Fetchヒントクラスの実現ファイル
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

#include "Btree/FetchHint.h"
#include "Btree/HintTools.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/IntegerArrayData.h"
#include "Exception/BadArgument.h"
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
//	Btree::FetchHint::LocalElementNumLimit -- ローカル配列の要素数上限
//
//	NOTES
//	ローカル配列の要素数上限。
//
// static
const int
FetchHint::LocalElementNumLimit = 10;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::FetchHint::FetchHint -- コンストラクタ
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
FetchHint::FetchHint()
	: m_FieldNumber(0),
	  m_LastLinkKeyFieldIndex(0),
	  m_FieldIndexArray(0),
	  m_FieldIndexArrayAllocateSize(0),
	  m_FieldTypeArray(0),
	  m_FieldTypeArrayAllocateSize(0),
	  m_IsFixedArray(0),
	  m_IsFixedArrayAllocateSize(0),
	  m_ConditionArray(0),
	  m_ConditionArrayAllocateSize(0),
	  m_ConditionOffsetArray(0),
	  m_ConditionOffsetArrayAllocateSize(0),
	  m_IsNullArray(0),
	  m_IsNullArrayAllocateSize(0),
	  m_MultiNumberArray(0),
	  m_MultiNumberArrayAllocateSize(0),
	  m_ByKey(false),
	  m_OnlyKey(false),
	  m_ExistSeparateKey(false),
	  m_SetUpdateSearchTarget(false),
	  m_ConditionType(ConditionType::Undefined)
{
	//
	// 各ローカル配列の各要素を初期化する
	//

	for (int i = 0; i < FetchHint::LocalElementNumLimit; i++)
	{
		// Fetch対象フィールドインデックス配列
		this->m_FieldIndexLocalArray[i] = 0;

		// Fetch対象フィールドタイプ配列
		this->m_FieldTypeLocalArray[i] = Common::DataType::Undefined;

		// Fetch対象フィールド固定長フラグ配列
		this->m_IsFixedLocalArray[i] = false;

		// Fetch検索条件へのオフセット配列
		this->m_ConditionOffsetLocalArray[i] = 0;

		// Fetch検索条件フルフラグ配列
		this->m_IsNullLocalArray[i] = false;

		// 比較結果への乗数配列
		this->m_MultiNumberLocalArray[i] = 0;
	}

	// Fetch 検索条件配列を初期化する

	(void) Os::Memory::reset(
		m_ConditionLocalArray, sizeof(m_ConditionLocalArray));
}

//
//	FUNCTION public
//	Btree::FetchHint::~FetchHint -- デストラクタ
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
FetchHint::~FetchHint()
{
	// 確保した各配列を解放する
	this->clear(false); // 初期化は不要
}

//
//	FUNCTION public
//	Btree::FetchHint::set -- Fetchヒントを設定する
//
//	NOTES
//	Fetchヒントを設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&		FileParameter_
//		B+木ファイルパラメータへの参照
//	const Common::IntegerArrayData&	FetchFieldIndexArray_
//		Fetch対象フィールドインデックス配列への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FetchHint::set(const FileParameter&				FileParameter_,
			   const Common::IntegerArrayData&	FetchFieldIndexArray_)
{
	// （既に確保してあるかもしれないので）
	// 確保した各配列を解放する
	this->clear(true); // 初期化もする

	// Fetch対象フィールド数を設定する
	this->m_FieldNumber = FetchFieldIndexArray_.getCount();

	// Fetch検索条件タイプを設定する
	this->m_ConditionType =
		(this->m_FieldNumber == 1) ?
			ConditionType::Single : ConditionType::Multi;

	// 各配列へのポインタを設定する
	this->setArrayPointer();

	// B+木ファイルに挿入するオブジェクトは、
	//     ① オブジェクトIDフィールド
	//     ② キーフィールド
	//     ③ バリューフィールド
	// の3種類から構成される。
	// このうち、実際にファイルに記録されるのは、
	//     ② キーフィールド
	//     ③ バリューフィールド
	// である。
	// 変数topValueIndexは、先頭にバリューフィールドの
	// オブジェクト内でのインデックスである。
	// 例えば、
	//        [0]      [1]       [2]       [3]       [4]
	//     ┌───┬────┬────┬────┬────┐
	//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＶＡＬ１│ＶＡＬ２│
	//     └───┴────┴────┴────┴────┘
	// のようなオブジェクトの場合には、
	// 変数topValueIndexは、3となる。
	int	topValueIndex = FileParameter_.m_TopValueFieldIndex;

	// 各フラグを初期化しておく
	this->m_ByKey = true;
	this->m_OnlyKey = true;
	this->m_ExistSeparateKey = false;

	int	offset = 0;

	for (int i = 0; i < this->m_FieldNumber; i++)
	{
		// 各Fetch対象フィールドについてのヒントを設定する…

		// Fetch対象フィールドインデックスを取得する
		int	fieldIndex = FetchFieldIndexArray_.getElement(i);

		// 取得したFetch対象フィールドインデックスを
		// そのまま配列に記録する
		*(this->m_FieldIndexArray + i) = fieldIndex;

		if (i > 0 &&
			fieldIndex > (*(this->m_FieldIndexArray + (i - 1)) + 1) &&
			fieldIndex < topValueIndex)
		{
			// 先頭キーフィールドから連続している
			// Fetch対象キーフィールドから離れた
			// Fetch対象キーフィールドがあった…

			// 例えば、こんなFetchや、
			//                  ↓                  ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＶＡＬ１│
			//     └───┴────┴────┴────┴────┴─
			// こんなFetchではm_ExistSeparateKeyはtrueとなり、
			//                  ↓        ↓                  ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＫＥＹ４│
			//     └───┴────┴────┴────┴────┴─

			// こんなFetchや、
			//                  ↓        ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＶＡＬ１│
			//     └───┴────┴────┴────┴────┴─
			// こんなFetchではfalseとなる。
			//                  ↓        ↓        ↓
			//     ┌───┬────┬────┬────┬────┬─
			//     │ＯＩＤ│ＫＥＹ１│ＫＥＹ２│ＫＥＹ３│ＫＥＹ４│
			//     └───┴────┴────┴────┴────┴─
			// ※ "↓"が、Fetch対象キーフィールド

			this->m_ExistSeparateKey = true;
		}

		if (fieldIndex < topValueIndex &&
			this->m_ExistSeparateKey == false)
		{
			// Fetch対象キーフィールドが
			// 先頭キーフィールドから連続している…

			// 連続したFetch対象キーフィールドのうちで
			// 最後のFetch対象キーフィールドのインデックスを更新する
			this->m_LastLinkKeyFieldIndex = fieldIndex;
		}

		//
		// Fetch検索条件などのエラーチェックは、
		// FetchHintクラスでは行う必要がない。
		//

		if (fieldIndex == 0)
		{
			this->m_ByKey = false;
		}

		if (fieldIndex >= topValueIndex && this->m_OnlyKey)
		{
			this->m_OnlyKey = false;
		}

		// Fetch対象フィールドのフィールドタイプを取得する
		Common::DataType::Type	fieldType =
			*(FileParameter_.m_FieldTypeArray + fieldIndex);

		// 取得したフィールドタイプをそのまま配列に記録する
		*(this->m_FieldTypeArray + i) = fieldType;

		// Fetch対象フィールドは固定長フィールドか？
		bool	isFixed =
			*(FileParameter_.m_IsFixedFieldArray + fieldIndex);

		// 固定長フィールドかどうかを配列に記録する
		*(this->m_IsFixedArray + i) = isFixed;

		// 固定長フィールド配列ならば、
		// Fetch検索条件を配列に記録するので、
		// 配列のどこに記録したかということが必要になる。
		// そのための配列m_ConditionOffsetArrayに、
		// オフセットを記録する。
		*(this->m_ConditionOffsetArray + i) = isFixed ? offset : -1;

		// 次のFetch対象フィールドのためにオフセットを更新する
		// （フィールドタイプにより、適切なサイズ分オフセットを進める。）
		offset += HintTools::getFixedDataSize(fieldType);

		*(this->m_MultiNumberArray + i) =
			*(FileParameter_.m_MultiNumberArray + fieldIndex);
	}
}

//
//	FUNCTION public
//	Btree::FetchHint::setForUpdate --
//		更新系の処理用にFetchヒントを設定する。
//
//	NOTES
//	更新系の処理用にFetchヒントを設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParameter_
//		ファイルパラメータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FetchHint::setForUpdate(const FileParameter&	FileParameter_)
{
	this->clear(true);

	this->m_FieldNumber =
		FileParameter_.m_KeyNum + FileParameter_.m_ValueNum;

	this->m_ConditionType =
		(this->m_FieldNumber == 1) ?
			ConditionType::Single : ConditionType::Multi;

	this->setArrayPointer();

	this->m_LastLinkKeyFieldIndex = FileParameter_.m_KeyNum;

	this->m_ByKey = true;
	this->m_OnlyKey =
		(this->m_FieldNumber == this->m_LastLinkKeyFieldIndex);
	this->m_ExistSeparateKey = false;

	int	offset = 0;

	for (int i = 0; i < this->m_FieldNumber; i++)
	{
		int	fieldIndex = i + 1;

		*(this->m_FieldIndexArray + i) = fieldIndex;

		Common::DataType::Type	fieldType =
			*(FileParameter_.m_FieldTypeArray + fieldIndex);

		*(this->m_FieldTypeArray + i) = fieldType;

		bool	isFixed =
			*(FileParameter_.m_IsFixedFieldArray + fieldIndex);

		*(this->m_IsFixedArray + i) = isFixed;

		*(this->m_ConditionOffsetArray + i) = isFixed ? offset : -1;

		offset += HintTools::getFixedDataSize(fieldType);

		*(this->m_MultiNumberArray + i) =
			*(FileParameter_.m_MultiNumberArray + fieldIndex);
	}

	this->m_SetUpdateSearchTarget = false;
}

//
//	FUNCTION public
//	Btree::FetchHint::setUpdateSaerchTarget --
//		更新の処理用にFetch検索条件を設定する
//
//	NOTES
//	更新系の処理用にFetch検索条件を設定する。
//
//	ARGUMENTS
//	const bool	UpdateSearchTargetIsKey_
//		検索対象がキーフィールドかどうか
//			true  : 検索対象がキーフィールド
//			false : 検索対象がオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FetchHint::setUpdateSearchTarget(const bool	UpdateSearchTargetIsKey_)
{
	if (this->m_SetUpdateSearchTarget)
	{
		return;
	}

	this->m_SetUpdateSearchTarget = true;

	if (UpdateSearchTargetIsKey_)
	{
		this->m_FieldNumber = this->m_LastLinkKeyFieldIndex;

		this->m_OnlyKey = true;
	}
}

//
//	FUNCTION public
//	Btree::FetchHint::setCondition --
//		キー値によるFetchの際の検索条件を設定する
//
//	NOTES
//	キー値によるFetchの際の検索条件を設定する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						StartFieldIndex_ = 0
//		開始フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument（FetchHint::checkConditionByKeyより）
//		不正な検索条件である
//
void
FetchHint::setCondition(
	const Common::DataArrayData*	Condition_,
	const int						StartFieldIndex_ // = 0
	)
{
	// 適切なFetch検索条件が設定されているかをチェックする。
	// 検索条件がおかしければ、
	// checkConditionがBadArgumentを送出する。
	this->checkConditionByKey(Condition_, StartFieldIndex_);

	int	fieldIndex = StartFieldIndex_;

	for (int i = 0; i < this->m_FieldNumber; i++, fieldIndex++)
	{
		// 各Fetch対象フィールドに対する検索条件などを設定する…

		if (*(this->m_IsFixedArray + i))
		{
			// Fetch対象フィールドが固定長フィールド…

			// 固定長フィールドならば、
			// 自身が保持している検索条件のための配列m_ConditionArrayに
			// そのフィールドに対する検索条件を記録しておく。
			// 配列のどこに記録すればよいかも、
			// そのための配列m_ConditionOffsetArrayを参照すれば
			// わかるようになっている。

			; _SYDNEY_ASSERT(*(this->m_ConditionOffsetArray + i) != -1);

			char*	setPos =
				this->m_ConditionArray + *(this->m_ConditionOffsetArray + i);

			Common::Data*	field = Condition_->getElement(fieldIndex).get();

			Common::DataType::Type	fieldType = field->getType();

			; _SYDNEY_ASSERT(fieldType == *(this->m_FieldTypeArray + i) ||
							 field->isNull());

			if (field->isNull())
			{
				// Fetch対象フィールドがヌル値のオブジェクトをFetch…

				*(this->m_IsNullArray + i) = true;
			}
			else
			{
				// Fetch対象フィールドがヌル値以外のオブジェクトをFetch…

				*(this->m_IsNullArray + i) = false;

				// Fetch対象フィールドの検索条件を設定する
				HintTools::setFixedData(fieldType, field, setPos);
			}
		}
		else
		{
			// Fetch対象フィールドが可変長フィールド…

			// 可変長フィールドであっても、
			// 『そのフィールドにヌル値が設定されている
			// 　オブジェクトをFetchしたい。』
			// と言われれば、そのことは配列に記録しておく。

			Common::Data*	field = Condition_->getElement(fieldIndex).get();

			*(this->m_IsNullArray + i) = field->isNull();
		}

	} // end for i
}

//
//	FUNCTION public
//	Btree::FetchHint::compareToFixedCondition --
//		固定長フィールド値を比較する
//
//	NOTES
//	引数FieldValue_が指し示す、固定長フィールド値と、
//	保持してあるフィールドに対する検索条件とを比較し、
//	比較結果を呼び出し側に知らせる。
//
//	ARGUMENTS
//	const void*	FieldValue_
//		固定長フィールド値へのポインタ
//	const int	ArrayIndex_
//		配列インデックス
//		※ Fetch対象フィールドインデックスではない。
//		　 “何番目のFetch対象フィールドか”である。
//
//	RETURN
//	int
//		比較結果
//		= 0	検索条件と等しい
//		> 0	検索条件の方が大きい
//		< 0	検索条件の方が小さい
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
int
FetchHint::compareToFixedCondition(const void*	FieldValue_,
								   const int	ArrayIndex_) const
{
	; _SYDNEY_ASSERT(ArrayIndex_ >= 0 &&
					 ArrayIndex_ < this->m_FieldNumber);

	return
		HintTools::compareToFixedData(
			*(this->m_FieldTypeArray + ArrayIndex_),
			this->m_ConditionArray,
			this->m_ConditionOffsetArray,
			this->m_MultiNumberArray,
			ArrayIndex_,
			FieldValue_);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::FetchHint::clear -- 確保した各配列を解放する
//
//	NOTES
//	自身が保持しているローカルな配列では収まりきれない
//	Fetch検索条件で、ヒープ領域に確保した配列用の領域があれば、
//	その領域を解放する。
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
FetchHint::clear(const bool	DoInit_)
{
	if (this->m_FieldNumber <= FetchHint::LocalElementNumLimit)
	{
		// 自身のローカルは配列で対応していた…

		return;
	}

	// Fetch対象フィールドインデックス配列

	; _SYDNEY_ASSERT(this->m_FieldIndexArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_FieldIndexArray != this->m_FieldIndexLocalArray);

	ModDefaultManager::free(this->m_FieldIndexArray,
							this->m_FieldIndexArrayAllocateSize);

	// Fetch対象フィールドタイプ配列

	; _SYDNEY_ASSERT(this->m_FieldTypeArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_FieldTypeArray != this->m_FieldTypeLocalArray);

	ModDefaultManager::free(this->m_FieldTypeArray,
							this->m_FieldTypeArrayAllocateSize);

	// Fetch対象フィールド固定長フラグ配列

	; _SYDNEY_ASSERT(this->m_IsFixedArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_IsFixedArray != this->m_IsFixedLocalArray);

	ModDefaultManager::free(this->m_IsFixedArray,
							this->m_IsFixedArrayAllocateSize);

	// Fetch検索条件配列

	; _SYDNEY_ASSERT(this->m_ConditionArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_ConditionArray != this->m_ConditionLocalArray);

	ModDefaultManager::free(this->m_ConditionArray,
							this->m_ConditionArrayAllocateSize);

	// Fetch検索条件へのオフセット配列

	; _SYDNEY_ASSERT(this->m_ConditionOffsetArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_ConditionOffsetArray != this->m_ConditionOffsetLocalArray);

	ModDefaultManager::free(this->m_ConditionOffsetArray,
							this->m_ConditionOffsetArrayAllocateSize);

	// Fetch検索条件ヌルフラグ配列

	; _SYDNEY_ASSERT(this->m_IsNullArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(this->m_IsNullArray != this->m_IsNullLocalArray);

	ModDefaultManager::free(this->m_IsNullArray,
							this->m_IsNullArrayAllocateSize);

	// 比較結果への乗数配列

	; _SYDNEY_ASSERT(this->m_MultiNumberArrayAllocateSize > 0);
	; _SYDNEY_ASSERT(
		this->m_MultiNumberArray != this->m_MultiNumberLocalArray);

	ModDefaultManager::free(this->m_MultiNumberArray,
							this->m_MultiNumberArrayAllocateSize);

	if (DoInit_)
	{
		// Fetch対象フィールドインデックス配列
		this->m_FieldIndexArray = 0;
		this->m_FieldIndexArrayAllocateSize = 0;

		// Fetch対象フィールドタイプ配列
		this->m_FieldTypeArray = 0;
		this->m_FieldTypeArrayAllocateSize = 0;

		// Fetch対象フィールド固定長フラグ配列
		this->m_IsFixedArray = 0;
		this->m_IsFixedArrayAllocateSize = 0;

		// Fetch検索条件配列
		this->m_ConditionArray = 0;
		this->m_ConditionArrayAllocateSize = 0;

		// Fetch検索条件へのオフセット配列
		this->m_ConditionOffsetArray = 0;
		this->m_ConditionOffsetArrayAllocateSize = 0;

		// Fetch検索条件ヌルフラグ配列
		this->m_IsNullArray = 0;
		this->m_IsNullArrayAllocateSize = 0;

		// 比較結果への乗数配列
		this->m_MultiNumberArray = 0;
		this->m_MultiNumberArrayAllocateSize = 0;
	}
}

//
//	FUNCTION private
//	Btree::FetchHint::setArrayPointer -- 各配列へのポインタを設定する
//
//	NOTES
//	FetchHintクラスのインスタンス自身は、
//	Fetch対象フィールドが10フィールドまでに対応できる配列は、
//	保持している。
//	もしFetch対象フィールドとして11フィールド以上が
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
FetchHint::setArrayPointer()
{
	; _SYDNEY_ASSERT(this->m_FieldNumber > 0);

	if (this->m_FieldNumber > FetchHint::LocalElementNumLimit)
	{
		// 自身のローカルな配列では対応しきれない…

		// ヒープ領域に各配列用の領域を確保し、
		// ポインタをそれらを指すように設定する。

		// Fetch対象フィールドインデックス配列

		this->m_FieldIndexArrayAllocateSize =
			sizeof(int) * this->m_FieldNumber;

		this->m_FieldIndexArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_FieldIndexArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldIndexArray,
								   this->m_FieldIndexArrayAllocateSize);

		// Fetch対象フィールドタイプ配列

		this->m_FieldTypeArrayAllocateSize =
			sizeof(Common::DataType::Type) * this->m_FieldNumber;

		this->m_FieldTypeArray =
			static_cast<Common::DataType::Type*>(
				ModDefaultManager::allocate(
					this->m_FieldTypeArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldTypeArray,
								   this->m_FieldTypeArrayAllocateSize);

		// Fetch対象フィールド固定長フラグ配列

		this->m_IsFixedArrayAllocateSize =
			sizeof(bool) * this->m_FieldNumber;

		this->m_IsFixedArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsFixedArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_IsFixedArray,
								   this->m_IsFixedArrayAllocateSize);

		// Fetch検索条件配列

		this->m_ConditionArrayAllocateSize =
			sizeof(int) * 7 * this->m_FieldNumber;

		this->m_ConditionArray =
			static_cast<char*>(
				ModDefaultManager::allocate(
					this->m_ConditionArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_ConditionArray,
								   this->m_ConditionArrayAllocateSize);

		// Fetch検索条件へのオフセット配列

		this->m_ConditionOffsetArrayAllocateSize =
			sizeof(int) * this->m_FieldNumber;

		this->m_ConditionOffsetArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_ConditionOffsetArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_ConditionOffsetArray,
			this->m_ConditionOffsetArrayAllocateSize);

		// Fetch検索条件ヌルフラグ配列

		this->m_IsNullArrayAllocateSize =
			sizeof(bool) * this->m_FieldNumber;

		this->m_IsNullArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsNullArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_IsNullArray,
								   this->m_IsNullArrayAllocateSize);

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

		// では、ポインタをローカルな配列を指すように設定する。

		// Fetch対象フィールドインデックス配列
		this->m_FieldIndexArray = this->m_FieldIndexLocalArray;

		// Fetch対象フィールドタイプ配列
		this->m_FieldTypeArray = this->m_FieldTypeLocalArray;

		// Fetch対象フィールド固定長フラグ配列
		this->m_IsFixedArray = this->m_IsFixedLocalArray;

		// Fetch検索条件配列
		this->m_ConditionArray = this->m_ConditionLocalArray;

		// Fetch検索条件へのオフセット配列
		this->m_ConditionOffsetArray = this->m_ConditionOffsetLocalArray;

		// Fetch検索条件ヌルフラグ配列
		this->m_IsNullArray = this->m_IsNullLocalArray;

		// 比較結果への乗数配列
		this->m_MultiNumberArray = this->m_MultiNumberLocalArray;
	}
}

//
//	FUNCTION private
//	Btree::FetchHint::checkConditionByKey --
//		キー値によるFetchの際の検索条件をチェックする。
//
//	NOTES
//	キー値によるFetchの際の検索条件をチェックする。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Condition_
//		Fetch検索条件へのポインタ
//	const int						StartFieldIndex_
//		開始フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な検索条件である
//
void
FetchHint::checkConditionByKey(
	const Common::DataArrayData*	Condition_,
	const int						StartFieldIndex_) const
{
	; _SYDNEY_ASSERT(this->m_FieldNumber > 0);
	; _SYDNEY_ASSERT(this->m_ConditionArray != 0);

	if (Condition_ == 0)
	{
		SydErrorMessage << "empty fetch condition." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	int	fieldNum = Condition_->getCount() - StartFieldIndex_;

	if (fieldNum != this->m_FieldNumber)
	{
		SydErrorMessage << "illegal number of fetch fields." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	int	fieldIndex = StartFieldIndex_;

	for (int i = 0; i < fieldNum; i++, fieldIndex++)
	{
		const Common::Data*	field =
			Condition_->getElement(fieldIndex).get();

		Common::DataType::Type	fieldType = field->getType();

		if (fieldType != *(this->m_FieldTypeArray + i) &&
			!field->isNull())
		{
			SydErrorMessage
				<< "illegal field data type. ("
				<< static_cast<int>(fieldType) << ", "
				<< static_cast<int>(*(this->m_FieldTypeArray + i))
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
