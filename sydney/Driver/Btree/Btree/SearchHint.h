// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchHint.h -- 検索ヒントクラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_SEARCHHINT_H
#define __SYDNEY_BTREE_SEARCHHINT_H

#include "Common/Common.h"
#include "Common/DataType.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Btree/FileParameter.h"
#include "Btree/OpenParameter.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	CLASS
//	Btree::SearchHint -- 検索ヒントクラス
//
//	NOTES
//	検索ヒントクラス。
//
class SearchHint
{
public:

	// コンストラクタ
	SearchHint();

	// デストラクタ
	virtual ~SearchHint();

	// 検索ヒントを設定する
	void set(
		const FileParameter&					FileParameter_,
		const OpenParameter::SearchCondition&	SearchCondition_);

	// 検索開始条件と検索対象の固定長フィールド値を比較する
	int compareToFixedStartCondition(const void*	FieldValue_,
									 const int		ArrayIndex_) const;

	// 検索終了条件と検索対象の固定長フィールド値を比較する
	int compareToFixedStopCondition(const void*	FieldValue_) const;

	//
	// データメンバ
	//

	// 検索対象フィールド数
	int						m_FieldNumber;

	// 最後の検索対象キーフィールドのインデックス
	int						m_LastKeyFieldIndex;

	// 連続した検索対象フィールドのうちで
	// 最後の検索対象キーフィールドのインデックス
	// （複合条件の場合に有効）
	int						m_LastLinkKeyFieldIndex;

	// 検索対象フィールドインデックス配列へのポインタ
	int*					m_FieldIndexArray;

	// 検索対象フィールドインデックス配列の確保サイズ
	ModSize					m_FieldIndexArrayAllocateSize;

	// 検索対象フィールドタイプ配列へのポインタ
	Common::DataType::Type*	m_FieldTypeArray;

	// 検索対象フィールドタイプ配列の確保サイズ
	ModSize					m_FieldTypeArrayAllocateSize;

	// 検索対象フィールド固定長フラグ配列へのポインタ
	bool*					m_IsFixedArray;

	// 検索対象フィールド固定長フラグ配列の確保サイズ
	ModSize					m_IsFixedArrayAllocateSize;

	// 検索開始条件配列へのポインタ
	char*					m_StartConditionArray;

	// 検索開始条件配列の確保サイズ
	ModSize					m_StartConditionArrayAllocateSize;

	// 検索開始条件へのオフセット配列へのポインタ
	int*					m_StartConditionOffsetArray;

	// 検索開始条件へのオフセット配列の確保サイズ
	ModSize					m_StartConditionOffsetArrayAllocateSize;

	// 検索開始条件の比較演算子配列へのポインタ
	LogicalFile::TreeNodeInterface::Type*
							m_StartOperatorArray;

	// 検索開始条件の比較演算子配列の確保サイズ
	ModSize					m_StartOperatorArrayAllocateSize;

	// 比較結果への乗数配列
	int*					m_MultiNumberArray;

	// 比較結果への乗数配列の確保サイズ
	ModSize					m_MultiNumberArrayAllocateSize;

	// 検索終了条件
	// （範囲指定の場合に有効）
	char					m_StopCondition[28]; // 4 * 7

	// 検索終了条件の比較演算子
	// （範囲指定の場合に有効）
	LogicalFile::TreeNodeInterface::Type
							m_StopOperator;

	// キー値による検索かどうか
	//     true  : キー値による検索
	//     false : オブジェクトIDによる検索
	bool					m_ByKey;

	// 先頭キーフィールドから連続している
	// 検索対象キーフィールドから離れた
	// 検索対象キーフィールドがあるかどうか
	// （複合条件の場合に有効）
	//     true  : 離れた検索対象キーフィールドがある
	//     false : 連続した検索対象キーフィールドのみ
	bool					m_ExistSeparateKey;

	// 最終検索対象キーフィールドへの検索条件がEqualsかどうか
	//     true  : Equals
	//     false : Equals以外
	bool					m_LastStartOperatorIsEquals;

	// 最終検索対象キーフィールドへの検索条件が範囲指定かどうか
	//     true  : 範囲指定
	//     false : 範囲指定ではない
	bool					m_LastIsSpan;

	// 検索結果が'φ'となる問い合わせ
	//     true  : 問い合わせ無効（既に検索結果が'φ'と判明）
	//     false : 問い合わせ有効
	bool					m_VoidSearch;

	//
	//	STRUCT
	//	Btree::SearchHint::ConditionType -- 検索条件タイプ構造体
	//
	//	NOTES
	//	検索条件タイプ構造体。
	//
	struct ConditionType
	{
		//
		//	ENUM
		//	Btree::SearchHint::ConditionType::Value --
		//		検索条件タイプ
		//
		//	NOTES
		//	検索条件タイプ。
		//
		enum Value
		{
			Single = 0, // 単一条件（単一キーフィールドでの検索）
			Multi,      // 複合条件（複数キーフィールドでの検索）
			Undefined
		};
	};

	// 検索条件タイプ
	ConditionType::Value	m_ConditionType;

	//
	//	STRUCT
	//	Btree::SearchHint::CompareType -- 比較対象構造体
	//
	//	NOTES
	//	比較対象構造体。
	//	※ こちらは、“どのフィールドが比較対象か”ということ。
	//
	struct CompareType
	{
		//
		//	ENUM
		//	Btree::SearchHint::CompareType::Value -- 比較対象
		//
		//	NOTES
		//	比較対象。
		//
		enum Value
		{
			All = 0,         // 全ての検索対象キーフィールドを比較
			OnlyLinkKey,     // 連続した検索対象キーフィールドのみ比較
			OnlySeparateKey, // 離れた検索対象キーフィールドのみ比較
			OnlyLastKey,     // 最終検索対象キーフィールドのみ比較
			OnlyNotLastKey,  // 最終検索対象キーフィールド以外を比較
			Undefined
		};
	};

	//
	//	STRUCT
	//	Btree::SearchHint::CompareTarget -- 比較対象構造体
	//
	//	NOTES
	//	比較対象構造体。
	//	※ こちらは、
	//	　 “検索開始条件、検索終了条件、いずれとの比較か”ということ。
	//
	struct CompareTarget
	{
		//
		//	ENUM
		//	Btree::SearchHint::CompareTarget::Value -- 比較対象
		//
		//	NOTES
		//	比較対象。
		//
		enum Value
		{
			Start = 0, // 検索開始条件との比較
			Stop,      // 検索終了条件との比較
			Undefined
		};
	};

	//
	// 以下は、likeによる文字列検索用のメンバ
	//

	// パターン文字列
	ModUnicodeString	m_PatternString;

	// likeでの文字列検索の際の先頭からワイルドカード直前までの文字列
	ModUnicodeString	m_LikeSearchCondition;

private:

	// likeによる文字列検索のためのヒントを設定する
	void
		setLikeSearchCondition(
			const OpenParameter::SearchCondition&	SearchCondition_);

	// 確保した各配列を解放する
	void clear(const bool	DoInit_);

	// 各配列へのポインタを設定する
	void setArrayPointer();

	//
	// データメンバ
	//

	// ローカル配列の要素数上限
	static const int		LocalElementNumLimit; // = 10

	// ローカルな検索対象フィールドインデックス配列
	int						m_FieldIndexLocalArray[10];

	// ローカルな検索対象フィールドタイプ配列
	Common::DataType::Type	m_FieldTypeLocalArray[10];

	// ローカルな検索対象フィールド固定長フラグ配列
	bool					m_IsFixedLocalArray[10];

	// ローカルな検索開始条件配列
	// ※ 最も大きな固定長フィールドの値の領域は、
	// 　 DateTimeDataの 4 * 7 バイトなので、
	// 　 その分を確保している。
	char					m_StartConditionLocalArray[280]; // 4 * 7 * 10

	// ローカルな検索開始条件へのオフセット配列
	int						m_StartConditionOffsetLocalArray[10];

	// ローカルな検索開始条件の比較演算子配列
	LogicalFile::TreeNodeInterface::Type
							m_StartOperatorLocalArray[10];

	// ローカルな比較結果への乗数配列
	int						m_MultiNumberLocalArray[10];

}; // end of class Btree::SearchHint

} // end of namespace Btree

_SYDNEY_END

#endif //__SYDNEY_BTREE_SEARCHHINT_H

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
