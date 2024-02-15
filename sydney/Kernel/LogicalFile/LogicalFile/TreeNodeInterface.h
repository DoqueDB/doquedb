// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeNodeInterface --
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_TREENODEINTERFACE_H
#define __SYDNEY_LOGICALFILE_TREENODEINTERFACE_H

#include "LogicalFile/Module.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}

_SYDNEY_LOGICALFILE_BEGIN

//	CLASS
//	TreeNodeInterface --
//
//	NOTES

class TreeNodeInterface
	: public virtual Common::Object
{
public:
	// ノードタイプ
	enum Type
	{
		// Basic nodes
		ConstantValue =		10000,
		Variable,
		Column,
		Table,
		Index,
		File,
		Procedure,
		Tuple,
		DerivedColumn,
		SortKey,
		SortDirection,
		Field,
		Key,
		TupleList,
		DerivedTable,
		VirtualColumn,
		Hint,

		// Initial Nodes
		StreamInput =		20000,
		FileInput,
		DataInput,
		Input,
		Unnest,

		// Terminal Nodes
		StreamOutput =		21000,
		FileOutput,
		PrintOutput,
		DataOutput,
		NullOutput,
		Output,

		// Predicates
		// ---- Boolean Constant
		PredicatesStart =	30000,
		True,
		False,
		Unknown,

		// ----- Comparison
		// ---------- Comparison With Equals
		Equals =			31000,
		LessThanEquals,
		GreaterThanEquals,
		EqualsToNull,
		NotNull,
		Between,
		NotBetween,
		In,
		NotIn,

		// ---------- Comparison Without Equals
		LessThan =			32000,
		GreaterThan,
		NotEquals,
		Test,

		// ----- String Match
		Like =				33000,
		Similar,
		NotLike,
		NotSimilar,

		// ----- Boolean Operators
		And =				34000,
		Or,
		Not,
		AndNot,

		// ----- Sub Queries
		Exists =			35000,
		All,
		NotExists,
		RowSubquery,
		Any,

		// ----- Array Queries
		IsSubstringOf =		36000,

		// End of Predicates
		PredicatesEnd,

		// Set Operators
		// ----- Unary Set Operators
		Selection =			40000,
		Projection,
		Sort,
		Unique,
		Distinct,
		GroupBy,
		Lock,
		Log,
		Doubling,
		Aggregation,
		Limit,
		Check,
		Collecting,
		PartitionBy,
		OrderBy, // order specification

		// ----- Binary Set Operators
		// ---------- Join Operators
		SimpleJoin =		41000,
		MergeJoin,
		HashJoin,
		StarJoin,
		IndexJoin,

		// ---------- OuterJoin Operators
		LeftOuterJoin =		41200,
		RightOuterJoin,
		FullOuterJoin,

		// ---------- Semi Join Operators
		SimpleSemiJoin =	41500,
		MergeSemiJoin,
		HashSemiJoin,

		// ---------- Set Boolean Operators
		Intersection =		42000,
		Union,
		Difference,
		Product,

		// Special Nodes
		Fetch =				50000,
		Choice,
		Copy,
		AssignGenerator,

		// ----- Arithmetic Operators
		ArithmeticStart =	60000,

		Add,
		Subtract,
		Multiply,
		Divide,
		Modulus,
		Negative,
		StringConcatenate,
		Absolute,
		CharLength,
		SubString,
		Overlay,
		Truncate,
		Nop,
		Case,
		NullIf,
		Coalesce,
		Cast,
		WordCount,
		FullTextLength,
		Normalize,
		OctetLength,
		CoalesceDefault,
		GetMax,

		ArithmeticEnd,

		// DateTime Functions
		DateTimeFunctionStart =	65000,

		CurrentDate,
		CurrentTime,
		CurrentTimestamp,
		LocalTime,
		LocalTimestamp,

		DateTimeFunctionEnd,

		// ----- Set Function Operators
		SetFunctionStart =	70000,

		Count,
		Avg,
		Max,
		Min,
		Sum,
		GroupingElement,
		GroupingWords,
		SetFunctionEnd,

		// ----- Array Operators
		ArrayFunctionStart =	72000,

		ArrayConstructor,
		ElementReference,
		Cardinality,
		CharJoin,

		ArrayFunctionEnd,

		// Inverted Function
		Expand =			75000,
		Freetext,
		GeneratedQuery,
		Near,
		RelatedWords,
		SelectedWords,
		Score,
		Weight,
		Contains,
		Language,
		Extractor,
		Pattern,
		Calculator,
		Within,
		Combiner,
		Head,
		Tail,
		ExactWord,
		SimpleWord,
		AverageLength,
		Df,
		Scale,
		Upper,
		Lower,
		Symmetric,
		Category,
		Word,
		WordList,
		ScoreFunction,
		AverageWordCount,
		Tf,
		String,
		WordHead,
		WordTail,
		ClusteredLimit,
		Synonym,
		KwicSize,
		Kwic,
		ExpandSynonym,
		MatchMode,
		KwicStartTag,
		KwicEndTag,
		KwicEscape,
		KwicEllipsis,
		ScoreCombiner,
		ClusteredCombiner,
		ClusterID,
		FeatureValue,
		RoughKwicPosition,
		Section,
		AverageCharLength,
		WordDf,
		WordScale,
		Existence,
		ScaleParameter,
		WordLimit,
		
		// Lob Function
		Append =			77000,
		Replace,
		
		// Spatial Function
		NeighborIn =		78000,
		NeighborID,
		NeighborDistance,

		// Pure Structural Nodes
		Pair =				90000,
		List,

		Undefined =			99999
	};

	// デストラクタ
	SYD_LOGICALFILE_FUNCTION
	virtual ~TreeNodeInterface();
	// 型を得る
	Type
	getType() const;
	// 文字列で値を得る
	SYD_LOGICALFILE_FUNCTION
	virtual ModUnicodeString getValue() const;
	// Common::Dataで値を得る
	SYD_LOGICALFILE_FUNCTION
	virtual const Common::Data* getData() const;

	// オプションの個数を得る
	virtual ModSize
	getOptionSize() const;
	// 指定番めのオプションを得る(0-base)
	SYD_LOGICALFILE_FUNCTION
	virtual const TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const;

	// オペランドの個数を得る
	virtual ModSize
	getOperandSize() const;
	// 指定番めのオペランドを得る(0-base)
	SYD_LOGICALFILE_FUNCTION
	virtual const TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const;

	// タイプに対応する文字列を得る
	SYD_LOGICALFILE_FUNCTION
	static ModUnicodeString getTypeName(Type eType_);

protected:
	// サブクラス以外がこのクラスを生成することは許さないのでここに置く
	// コンストラクタ
	SYD_LOGICALFILE_FUNCTION
	TreeNodeInterface(Type eType_ = Undefined);

	// 型を設定しなおす
	Type setType(Type eType_);

private:
	// コピーコンストラクタ(宣言のみ)
	TreeNodeInterface(const TreeNodeInterface& cOther_);
	// 代入オペレーター(宣言のみ)
	TreeNodeInterface& operator=(const TreeNodeInterface& cOther_);
	// データ型
	Type m_eType;
};

//	FUNCTION protected
//	LogicalFile::TreeNodeInterface::TreeNodeInterface -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::TreeNodeInterface::Type	eType
//			指定されたとき
//				生成するノードの型
//			指定されないとき
//				LogicalFile::TreeNodeInterface::Undefined が
//				指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TreeNodeInterface::TreeNodeInterface(Type eType_)
	: m_eType(eType_)
{}

//	FUNCTION public
//	LogicalFile::TreeNodeInterface::~TreeNodeInterface -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TreeNodeInterface::~TreeNodeInterface()
{}

//	FUNCTION public
//	LogicalFile::TreeNodeInterface::getType -- 型を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた型
//
//	EXCEPTIONS
//		なし

inline
TreeNodeInterface::Type
TreeNodeInterface::getType() const
{
	return m_eType;
}

//	FUNCTION public
//	LogicalFile::TreeNodeInterface::getOptionSize -- オプションの個数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオプションの個数
//
//	EXCEPTIONS
//		なし

inline
ModSize
TreeNodeInterface::getOptionSize() const
{
	return 0;
}

//	FUNCTION public
//	LogicalFile::TreeNodeInterface::getOperandSize -- オペランドの個数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオペランドの個数
//
//	EXCEPTIONS
//		なし

inline
ModSize
TreeNodeInterface::getOperandSize() const
{
	return 0;
}

//	FUNCTION protected
//	LogicalFile::TreeNodeInterface::setType -- 型を設定しなおす
//
//	NOTES
//
//	ARGUMENTS
//		Type eType_
//
//	RETURN
//		設定した型
//
//	EXCEPTIONS
//		なし

inline
TreeNodeInterface::Type
TreeNodeInterface::setType(Type eType_)
{
	return m_eType = eType_;
}

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif // __SYDNEY_LOGICALFILE_TREENODEINTERFACE_H

//
//	Copyright (c) 1999, 2000, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
