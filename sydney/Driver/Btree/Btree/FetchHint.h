// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FetchHint.h -- Fetchヒントクラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_FETCHHINT_H
#define __SYDNEY_BTREE_FETCHHINT_H

#include "Btree/Module.h"
#include "Btree/FileParameter.h"

#include "Common/DataType.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
	class IntegerArrayData;
}

_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::FetchHint -- Fetchヒントクラス
//
//	NOTES
//	Fetchヒントクラス。
//	主にキー値によるFetchの際の検索条件などを保持するクラスである。
//
class FetchHint
{
public:

	// コンストラクタ
	FetchHint();

	// デストラクタ
	virtual ~FetchHint();

	// Fetchヒントを設定する
	void set(const FileParameter&				FileParameter_,
			 const Common::IntegerArrayData&	FetchFieldIndexArray_);

	// Fetchヒントを設定する (for expunge and update)
	void setForUpdate(const FileParameter&	FileParameter_);

	void setUpdateSearchTarget(const bool	UpdateSearchTargetIsKey_);

	// キー値によるFetchの際の検索条件を設定する
	void setCondition(const Common::DataArrayData*	Condition_,
					  const int						StartFieldIndex_ = 0);

	// 固定長フィールド値を比較する
	int compareToFixedCondition(const void*	FieldValue_,
								const int	ArrayIndex_) const;

	//
	// データメンバ
	//

	// Fetch対象フィールド数
	int						m_FieldNumber;

	// 連続したFetch対象キーフィールドのうちで
	// 最後のFetch対象キーフィールドのインデックス
	int						m_LastLinkKeyFieldIndex;

	// Fetch対象フィールドインデックス配列へのポインタ
	int*					m_FieldIndexArray;

	// Fetch対象フィールドインデックス配列の確保サイズ
	ModSize					m_FieldIndexArrayAllocateSize;

	// Fetch対象フィールドタイプ配列へのポインタ
	Common::DataType::Type*	m_FieldTypeArray;

	// Fetch対象フィールドタイプ配列の確保サイズ
	ModSize					m_FieldTypeArrayAllocateSize;

	// Fetch対象フィールド固定長フラグ配列へのポインタ
	bool*					m_IsFixedArray;

	// Fetch対象フィールド固定長フラグ配列の確保サイズ
	ModSize					m_IsFixedArrayAllocateSize;

	// Fetch検索条件配列へのポインタ
	char*					m_ConditionArray;

	// Fetch検索条件配列の確保サイズ
	ModSize					m_ConditionArrayAllocateSize;

	// Fetch検索条件へのオフセット配列へのポインタ
	int*					m_ConditionOffsetArray;

	// Fetch検索条件へのオフセット配列の確保サイズ
	ModSize					m_ConditionOffsetArrayAllocateSize;

	// Fetch検索条件ヌルフラグ配列へのポインタ
	bool*					m_IsNullArray;

	// Fetch検索条件ヌルフラグ配列の確保サイズ
	ModSize					m_IsNullArrayAllocateSize;

	// 比較結果への乗数配列
	int*					m_MultiNumberArray;

	// 比較結果への乗数配列の確保サイズ
	ModSize					m_MultiNumberArrayAllocateSize;

	// キー値によるFetchかどうか
	//     true  : キー値によるFetch
	//     false : オブジェクトIDによるFetch
	bool					m_ByKey;

	// Fetch対象フィールドがキーフィールドのみかどうか
	//     true  : キーフィールドのみ
	//     false : バリューフィールドも含む
	bool					m_OnlyKey;

	// 先頭キーフィールドから連続している
	// Fetch対象キーフィールドから離れた
	// Fetch対象キーフィールドがあるかどうか
	//     true  : 離れたFetch対象キーフィールドがある
	//     false : 連続したFetch対象キーフィールドのみ
	bool					m_ExistSeparateKey;

	bool					m_SetUpdateSearchTarget;

	//
	//	STRUCT
	//	Btree::FetchHint::ConditionType -- Fetch検索条件タイプ構造体
	//
	//	NOTES
	//	Fetch検索条件タイプ構造体。
	//
	struct ConditionType
	{
		//
		//	ENUM
		//	Btree::FetchHint::ConditionType::Value --
		//		Fetch検索条件タイプ
		//
		//	NOTES
		//	Fetch検索条件タイプ。
		//
		enum Value
		{
			Single = 0,	// 単一条件（単一フィールドでのFetch）
			Multi,		// 複合条件（複数フィールドでのFetch）
			Undefined
		};
	};

	// Fetch検索条件タイプ
	ConditionType::Value	m_ConditionType;

	//
	//	STRUCT
	//	Btree::FetchHint::CompareType -- Fetch比較対象構造体
	//
	//	NOTES
	//	Fetch比較対象構造体。
	//
	struct CompareType
	{
		//
		//	ENUM
		//	Btree::FetchHint::CompareType::Value -- Fetch比較対象
		//
		//	NOTES
		//	Fetch比較対象。
		//
		enum Value
		{
			OnlyLinkKey = 0, // 連続したFetch対象キーフィールドのみ比較
			OnlySeparateKey, // 離れたFetch対象キーフィールドのみ比較
			OnlyKey,         // Fetch対象キーフィールドのみ比較
			OnlyValue,       // Fetch対象バリューフィールドのみ比較
			All,             // 全Fetch対象フィールドを比較
			Undefined
		};
	};

private:

	// 確保した各配列を解放する
	void clear(const bool	DoInit_);

	// 各配列へのポインタを設定する
	void setArrayPointer();

	// キー値によるFetchの際の検索条件をチェックする
	void checkConditionByKey(
		const Common::DataArrayData*	Condition_,
		const int						StartFieldIndex_) const;

	//
	// データメンバ
	//

	// ローカル配列の要素数上限
	static const int		LocalElementNumLimit; // = 10

	// ローカルなFetch対象フィールドインデックス配列
	int						m_FieldIndexLocalArray[10];

	// ローカルなFetch対象フィールドタイプ配列
	Common::DataType::Type	m_FieldTypeLocalArray[10];

	// ローカルなFetch対象フィールド固定長フラグ配列
	bool					m_IsFixedLocalArray[10];

	// ローカルなFetch検索条件配列
	// ※ 最も大きな固定長フィールドの値の領域は、
	// 　 DateTimeDataの 4 * 7 バイトなので、
	// 　 その分を確保している。
	char					m_ConditionLocalArray[280]; // 4 * 7 * 10

	// ローカルなFetch検索条件へのオフセット配列
	int						m_ConditionOffsetLocalArray[10];

	// ローカルなFetch検索条件ヌルフラグ配列
	bool					m_IsNullLocalArray[10];

	// ローカルな比較結果への乗数配列
	int						m_MultiNumberLocalArray[10];

}; // end of class Btree::FetchHint

_SYDNEY_BTREE_END
_SYDNEY_END

#endif //__SYDNEY_BTREE_FETCHHINT_H

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
