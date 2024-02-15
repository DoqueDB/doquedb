// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOptionAnalyzer.h --
//	Ｂ＋木ファイルオープンオプションのヘッダーファイル
// 
// Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_OPENOPTIONANALYZER_H
#define __SYDNEY_BTREE_OPENOPTIONANALYZER_H

#include "ModString.h"

#include "Common/Object.h"

#include "LogicalFile/TreeNodeInterface.h"
#include "FileCommon/OpenMode.h"

_SYDNEY_BEGIN

namespace Common
{
class IntegerArrayData;
}

namespace LogicalFile
{
class OpenOption;
}

namespace Btree
{
class MultiSearchConditionItem;
class FileParameter;


//
//	CLASS
//	Btree::OpenOptionAnalyzer -- オープンオプション解析器クラス
//
//	NOTES
//	オープンオプション解析器クラス。
//
class OpenOptionAnalyzer : public Common::Object
{
public:

	// 検索オープンパラメータを設定する
	static bool
		getSearchParameter(
			const LogicalFile::TreeNodeInterface*	Condition_,
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_);

	// File::getProjectionParamater, File::getUpdateParameterの
	// 下請け関数
	static bool
		getTargetParameter(
			const Common::IntegerArrayData&		TargetFields_,
			LogicalFile::OpenOption&			OpenOption_,
			const FileCommon::OpenMode::Mode	OpenMode_,
			const FileParameter&				FileParam_);

	// ソート順パラメータを設定する
	static bool
		getSortParameter(
			const Common::IntegerArrayData&	Keys_,
			const Common::IntegerArrayData&	Orders_,
			LogicalFile::OpenOption&		OpenOption_,
			const FileParameter&			FileParam_);

private:
	// Scanモード検索オープンパラメータを設定する
	static bool setScanSearchParameter(LogicalFile::OpenOption&	OpenOption_);

	// Fetchモード検索オープンパラメータを設定する
	static bool setFetchSearchParameter(
		const LogicalFile::TreeNodeInterface*	Condition_,
		LogicalFile::OpenOption&				OpenOption_,
		const FileParameter&					FileParam_);

	// likeでの先頭文字列キーフィールドへの検索のための
	// オープンパラメータを設定する
	static bool
		setLikeSearchParameter(
			const LogicalFile::TreeNodeInterface*	LikeNode_,
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_);

	// Common::BinaryData型のフィールドかどうかをチェックする
	static bool isBinaryDataField(const FileParameter&	FileParam_,
								  const int				FieldIndex_);


	// Fetch対象フィールドのインデックスを設定する
	static bool
		setFetchFieldIndexArray(
			const LogicalFile::TreeNodeInterface*	Condition_,
			Common::IntegerArrayData&				FetchFieldIndexArray_,
			const FileParameter&					FileParam_,
			const LogicalFile::OpenOption&			OpenOption_,
			const bool								FetchOnly_);

	// Search + Fetchモード時のFetch対象フィールドのインデックスを設定する
	static bool
		setFetchFieldIndexArray(
			const LogicalFile::TreeNodeInterface*	FetchedColumns_,
			Common::IntegerArrayData&				FetchFieldIndexArray_,
			const FileParameter&					FileParameter_,
			const LogicalFile::OpenOption&			OpenOption_);

	// ソートキーとFetch対象フィールドの整合性をチェックする
	static void
		checkSortKey(
			const LogicalFile::OpenOption&	OpenOption_,
			bool&							EstablishedSortKey_,
			bool&							SortKeyIsObjectID_);

	// 検索条件とソート順の整合性をチェックする
	static void
		checkSearchField(
			const LogicalFile::OpenOption&	OpenOption_,
			bool&							EstablishedSearchParam_,
			bool&							ObjectIDSearch_);

	// オープンオプションにFetch検索パラメータを設定する
	static void
		setFetchSearchParameter(
			const Common::IntegerArrayData&	FetchFieldIndexArray_,
			LogicalFile::OpenOption&		OpenOption_,
			const bool						FetchOnly_);

	// 単一条件によるSearchモードオープンパラメータを設定する
	static bool
		setSingleSearchParameter(
			const LogicalFile::TreeNodeInterface*	Condition_,
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_);

	// 「キー ＝ ヌル値」のSearchモードオープンパラメータを設定する
	static bool
		setEqualsToNullSearchParameter(
			const LogicalFile::TreeNodeInterface*	Condition_,
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_);

	// 複合検索条件によるSearchモードオープンパラメータを設定する
	static bool
		setMultiSearchParameter(
			const LogicalFile::TreeNodeInterface*	Condition_,
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_,
			MultiSearchConditionItem*				Conditions_,
			int										TargetFieldNum_);

	// オペランド値の文字列表現を返す
	static ModUnicodeString
		getOperandValue(
			const LogicalFile::TreeNodeInterface*		Condition_,
			const LogicalFile::TreeNodeInterface::Type	OperandType_);

	// 比較演算子を示す数値を返す
	static int
		getOperator(
			const LogicalFile::TreeNodeInterface::Type	OperandType_);

	// 範囲指定開始／終了値をチェックする
	static bool
		checkSpanSearchCondition(
			Btree::MultiSearchConditionItem*	Conditions_,
			const int							ConditionIndex_,
			const FileParameter&				FileParam_);

	//検索可能性判定結果
	enum SearchResult {
		 REGULAR
		,ILLEGAL
		,NULLOUT
	};

	// 重複した条件の有無を検査し、条件を設定
	static SearchResult
		checkAndSetNewCondition(
			const LogicalFile::TreeNodeInterface::Type& operandType_
			,const ModUnicodeString& searchData_
			,int& iOperator_
			,ModUnicodeString& cString_);

	// 複合検索条件をチェックする
	static SearchResult
		checkMultiSearchCondition(
			Btree::MultiSearchConditionItem*	Conditions_,
			const int							TargetFieldNum_,
			const LogicalFile::OpenOption&		OpenOption_,
			const FileParameter&				FileParam_);
	static SearchResult
		isContradict(const LogicalFile::TreeNodeInterface::Type& OperandType_, int& iOperator_);

	// setMultiSearchParameter()の内部構成メソッド。複合条件
	static SearchResult
		setMultiSearchParameter_Multi(
			const LogicalFile::TreeNodeInterface*	Condition_,
			const FileParameter&					FileParam_,
			MultiSearchConditionItem*				Conditions_,
			int										TargetFieldNum_);

	// setMultiSearchParameter()の内部構成メソッド。単純条件
	static SearchResult
		setMultiSearchParameter_Single(
			const LogicalFile::TreeNodeInterface*	operand
			,const FileParameter&					FileParam_
			,MultiSearchConditionItem*				Conditions
			,int									TargetFieldNum_);

	// setMultiSearchParameter()の内部構成メソッド。オプション設定
	static void
		setMultiSearchParameter_setOption(
			LogicalFile::OpenOption&				OpenOption_,
			const FileParameter&					FileParam_,
			MultiSearchConditionItem*				Conditions_,
			int										TargetFieldNum_);
	// setMultiSearchParameter()の内部構成メソッド。オプション設定
	static void
		setMultiSearchParameter_setOptionNULL(
			LogicalFile::OpenOption&				OpenOption_);

}; // end of class Btree::OpenOptionAnalyzer

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_OPENOPTIONANALYZER_H

//
//	Copyright (c) 2000,2001,2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
