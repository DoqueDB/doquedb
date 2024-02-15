// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeNodeInterface.cpp --
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

namespace {
const char moduleName[] = "LogicalFile";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Common/UnicodeString.h"
#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

//
//	FUNCTION public
//		LogicalFile::TreeNodeInterface::getValue -- 文字列で値を得る
//
//	NOTES
//		文字列で値を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ノードの値を表すModString型文字列
//
//	EXCEPTIONS
//		なし
//
ModUnicodeString
TreeNodeInterface::getValue() const
{
	return _TRMEISTER_U_STRING("");
}

// FUNCTION public
//	LogicalFile::TreeNodeInterface::getData -- Common::Dataで値を得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	const Common::Data*
//		VariableやContantValueのとき、対応するCommon::Dataのポインター
//		それ以外のとき0
//
// EXCEPTIONS

//virtual
const Common::Data*
TreeNodeInterface::
getData() const
{
	return 0;
}

//
//	FUNCTION public
//		LogicalFile::TreeNodeInterface::getOptionAt -- オプションを得る
//
//	NOTES
//		指定番めのオプションを得る
//
//	ARGUMENTS
//		iPosition_		オプションの位置
//
//	RETURN
//		オプション
//
//	EXCEPTIONS
//		Exception::NotSupported	関数がoverrideされていない
//
const TreeNodeInterface*
TreeNodeInterface::getOptionAt(ModInt32 iPosition_) const
{
	// this method must be overridden
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//		LogicalFile::TreeNodeInterface::getOperandAt -- オペランドを得る
//
//	NOTES
//		指定番めのオペランドを得る
//
//	ARGUMENTS
//		iPosition_		オペランドの位置
//
//	RETURN
//		オペランド
//
//	EXCEPTIONS
//		Exception::NotSupported	関数がoverrideされていない
//
const TreeNodeInterface*
TreeNodeInterface::getOperandAt(ModInt32 iPosition_) const
{
	// this method must be overridden
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

//
//	FUNCTION public
//		LogicalFile::TreeNodeInterface::getTypeName -- タイプに対応する文字列を得る
//
//	NOTES
//		タイプに対応する文字列を得る
//
//	ARGUMENTS
//		Type eType_
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//		なし
//
ModUnicodeString
TreeNodeInterface::getTypeName(Type eType_)
{
	switch (eType_) {
	case ConstantValue:
		return _TRMEISTER_U_STRING("ConstantValue");
	case Variable:
		return _TRMEISTER_U_STRING("Variable");
	case Column:
		return _TRMEISTER_U_STRING("Column");
	case Table:
		return _TRMEISTER_U_STRING("Table");
	case Index:
		return _TRMEISTER_U_STRING("Index");
	case File:
		return _TRMEISTER_U_STRING("File");
	case Procedure:
		return _TRMEISTER_U_STRING("Procedure");
	case Tuple:
		return _TRMEISTER_U_STRING("Tuple");
	case DerivedColumn:
		return _TRMEISTER_U_STRING("DerivedColumn");
	case SortKey:
		return _TRMEISTER_U_STRING("SortKey");
	case SortDirection:
		return _TRMEISTER_U_STRING("SortDirection");
	case Field:
		return _TRMEISTER_U_STRING("Field");
	case Key:
		return _TRMEISTER_U_STRING("Key");
	case TupleList:
		return _TRMEISTER_U_STRING("TupleList");
	case DerivedTable:
		return _TRMEISTER_U_STRING("DerivedTable");
	case VirtualColumn:
		return _TRMEISTER_U_STRING("VirtualColumn");
	case Hint:
		return _TRMEISTER_U_STRING("Hint");
	case StreamInput:
		return _TRMEISTER_U_STRING("StreamInput");
	case FileInput:
		return _TRMEISTER_U_STRING("FileInput");
	case DataInput:
		return _TRMEISTER_U_STRING("DataInput");
	case Input:
		return _TRMEISTER_U_STRING("Input");
	case Unnest:
		return _TRMEISTER_U_STRING("Unnest");
	case StreamOutput:
		return _TRMEISTER_U_STRING("StreamOutput");
	case FileOutput:
		return _TRMEISTER_U_STRING("FileOutput");
	case PrintOutput:
		return _TRMEISTER_U_STRING("PrintOutput");
	case DataOutput:
		return _TRMEISTER_U_STRING("DataOutput");
	case NullOutput:
		return _TRMEISTER_U_STRING("NullOutput");
	case Output:
		return _TRMEISTER_U_STRING("Output");
	case True:
		return _TRMEISTER_U_STRING("True");
	case False:
		return _TRMEISTER_U_STRING("False");
	case Unknown:
		return _TRMEISTER_U_STRING("Unknown");
	case Equals:
		return _TRMEISTER_U_STRING("Equals");
	case LessThanEquals:
		return _TRMEISTER_U_STRING("LessThanEquals");
	case GreaterThanEquals:
		return _TRMEISTER_U_STRING("GreaterThanEquals");
	case EqualsToNull:
		return _TRMEISTER_U_STRING("IsNull");
	case NotNull:
		return _TRMEISTER_U_STRING("IsNotNull");
	case Between:
		return _TRMEISTER_U_STRING("Between");
	case NotBetween:
		return _TRMEISTER_U_STRING("NotBetween");
	case In:
		return _TRMEISTER_U_STRING("In");
	case NotIn:
		return _TRMEISTER_U_STRING("NotIn");
	case LessThan:
		return _TRMEISTER_U_STRING("LessThan");
	case GreaterThan:
		return _TRMEISTER_U_STRING("GreaterThan");
	case NotEquals:
		return _TRMEISTER_U_STRING("NotEquals");
	case Test:
		return _TRMEISTER_U_STRING("Test");
	case Like:
		return _TRMEISTER_U_STRING("Like");
	case Similar:
		return _TRMEISTER_U_STRING("Similar");
	case NotLike:
		return _TRMEISTER_U_STRING("NotLike");
	case NotSimilar:
		return _TRMEISTER_U_STRING("NotSimilar");
	case And:
		return _TRMEISTER_U_STRING("And");
	case Or:
		return _TRMEISTER_U_STRING("Or");
	case Not:
		return _TRMEISTER_U_STRING("Not");
	case AndNot:
		return _TRMEISTER_U_STRING("AndNot");
	case Exists:
		return _TRMEISTER_U_STRING("Exists");
	case All:
		return _TRMEISTER_U_STRING("All");
	case NotExists:
		return _TRMEISTER_U_STRING("NotExists");
	case RowSubquery:
		return _TRMEISTER_U_STRING("RowSubquery");
	case Any:
		return _TRMEISTER_U_STRING("Any");
	case IsSubstringOf:
		return _TRMEISTER_U_STRING("IsSubstringOf");
	case Selection:
		return _TRMEISTER_U_STRING("Selection");
	case Projection:
		return _TRMEISTER_U_STRING("Projection");
	case Sort:
		return _TRMEISTER_U_STRING("Sort");
	case Unique:
		return _TRMEISTER_U_STRING("Unique");
	case Distinct:
		return _TRMEISTER_U_STRING("Distinct");
	case GroupBy:
		return _TRMEISTER_U_STRING("GroupBy");
	case Lock:
		return _TRMEISTER_U_STRING("Lock");
	case Log:
		return _TRMEISTER_U_STRING("Log");
	case Doubling:
		return _TRMEISTER_U_STRING("Doubling");
	case Aggregation:
		return _TRMEISTER_U_STRING("Aggregation");
	case Limit:
		return _TRMEISTER_U_STRING("Limit");
	case Check:
		return _TRMEISTER_U_STRING("Check");
	case Collecting:
		return _TRMEISTER_U_STRING("Collecting");
	case PartitionBy:
		return _TRMEISTER_U_STRING("PartitionBy");
	case OrderBy:
		return _TRMEISTER_U_STRING("OrderBy");
	case SimpleJoin:
		return _TRMEISTER_U_STRING("SimpleJoin");
	case MergeJoin:
		return _TRMEISTER_U_STRING("MergeJoin");
	case HashJoin:
		return _TRMEISTER_U_STRING("HashJoin");
	case StarJoin:
		return _TRMEISTER_U_STRING("StarJoin");
	case IndexJoin:
		return _TRMEISTER_U_STRING("IndexJoin");
	case LeftOuterJoin:
		return _TRMEISTER_U_STRING("LeftOuterJoin");
	case RightOuterJoin:
		return _TRMEISTER_U_STRING("RightOuterJoin");
	case FullOuterJoin:
		return _TRMEISTER_U_STRING("FullOuterJoin");
	case SimpleSemiJoin:
		return _TRMEISTER_U_STRING("SimpleSemiJoin");
	case MergeSemiJoin:
		return _TRMEISTER_U_STRING("MergeSemiJoin");
	case HashSemiJoin:
		return _TRMEISTER_U_STRING("HashSemiJoin");
	case Intersection:
		return _TRMEISTER_U_STRING("Intersection");
	case Union:
		return _TRMEISTER_U_STRING("Union");
	case Difference:
		return _TRMEISTER_U_STRING("Difference");
	case Product:
		return _TRMEISTER_U_STRING("Product");
	case Fetch:
		return _TRMEISTER_U_STRING("Fetch");
	case Choice:
		return _TRMEISTER_U_STRING("Choice");
	case Copy:
		return _TRMEISTER_U_STRING("Copy");
	case AssignGenerator:
		return _TRMEISTER_U_STRING("AssignGenerator");
	case Add:
		return _TRMEISTER_U_STRING("Add");
	case Subtract:
		return _TRMEISTER_U_STRING("Subtract");
	case Multiply:
		return _TRMEISTER_U_STRING("Multiply");
	case Divide:
		return _TRMEISTER_U_STRING("Divide");
	case Modulus:
		return _TRMEISTER_U_STRING("Modulus");
	case Negative:
		return _TRMEISTER_U_STRING("Negative");
	case StringConcatenate:
		return _TRMEISTER_U_STRING("StringConcatenate");
	case Absolute:
		return _TRMEISTER_U_STRING("Absolute");
	case CharLength:
		return _TRMEISTER_U_STRING("CharLength");
	case SubString:
		return _TRMEISTER_U_STRING("SubString");
	case Overlay:
		return _TRMEISTER_U_STRING("Overlay");
	case Truncate:
		return _TRMEISTER_U_STRING("Truncate");
	case Nop:
		return _TRMEISTER_U_STRING("Nop");
	case Case:
		return _TRMEISTER_U_STRING("Case");
	case NullIf:
		return _TRMEISTER_U_STRING("NullIf");
	case Coalesce:
		return _TRMEISTER_U_STRING("Coalesce");
	case Cast:
		return _TRMEISTER_U_STRING("Cast");
	case WordCount:
		return _TRMEISTER_U_STRING("WordCount");
	case FullTextLength:
		return _TRMEISTER_U_STRING("FullTextLength");
	case Normalize:
		return _TRMEISTER_U_STRING("Normalize");
	case OctetLength:
		return _TRMEISTER_U_STRING("OctetLength");
	case CoalesceDefault:
		return _TRMEISTER_U_STRING("CoalesceDefault");
	case GetMax:
		return _TRMEISTER_U_STRING("GetMax");
	case CurrentDate:
		return _TRMEISTER_U_STRING("CurrentDate");
	case CurrentTime:
		return _TRMEISTER_U_STRING("CurrentTime");
	case CurrentTimestamp:
		return _TRMEISTER_U_STRING("CurrentTimestamp");
	case LocalTime:
		return _TRMEISTER_U_STRING("LocalTime");
	case LocalTimestamp:
		return _TRMEISTER_U_STRING("LocalTimestamp");
	case Count:
		return _TRMEISTER_U_STRING("Count");
	case Avg:
		return _TRMEISTER_U_STRING("Avg");
	case Max:
		return _TRMEISTER_U_STRING("Max");
	case Min:
		return _TRMEISTER_U_STRING("Min");
	case Sum:
		return _TRMEISTER_U_STRING("Sum");
	case ArrayConstructor:
		return _TRMEISTER_U_STRING("ArrayConstructor");
	case ElementReference:
		return _TRMEISTER_U_STRING("ElementReference");
	case Cardinality:
		return _TRMEISTER_U_STRING("Cardinality");
	case CharJoin:
		return _TRMEISTER_U_STRING("CharJoin");
	case Expand:
		return _TRMEISTER_U_STRING("Expand");
	case Freetext:
		return _TRMEISTER_U_STRING("Freetext");
	case GeneratedQuery:
		return _TRMEISTER_U_STRING("GeneratedQuery");
	case Near:
		return _TRMEISTER_U_STRING("Near");
	case RelatedWords:
		return _TRMEISTER_U_STRING("RelatedWords");
	case SelectedWords:
		return _TRMEISTER_U_STRING("SelectedWords");
	case Score:
		return _TRMEISTER_U_STRING("Score");
	case Weight:
		return _TRMEISTER_U_STRING("Weight");
	case Contains:
		return _TRMEISTER_U_STRING("Contains");
	case Language:
		return _TRMEISTER_U_STRING("Language");
	case Extractor:
		return _TRMEISTER_U_STRING("Extractor");
	case Calculator:
		return _TRMEISTER_U_STRING("Calculator");
	case Within:
		return _TRMEISTER_U_STRING("Within");
	case Combiner:
		return _TRMEISTER_U_STRING("Combiner");
	case Head:
		return _TRMEISTER_U_STRING("Head");
	case Tail:
		return _TRMEISTER_U_STRING("Tail");
	case ExactWord:
		return _TRMEISTER_U_STRING("ExactWord");
	case SimpleWord:
		return _TRMEISTER_U_STRING("SimpleWord");
	case AverageLength:
		return _TRMEISTER_U_STRING("AverageLength");
	case Df:
		return _TRMEISTER_U_STRING("Df");
	case Scale:
		return _TRMEISTER_U_STRING("Scale");
	case Upper:
		return _TRMEISTER_U_STRING("Upper");
	case Lower:
		return _TRMEISTER_U_STRING("Lower");
	case Symmetric:
		return _TRMEISTER_U_STRING("Symmetric");
	case Category:
		return _TRMEISTER_U_STRING("Category");
	case Word:
		return _TRMEISTER_U_STRING("Word");
	case WordList:
		return _TRMEISTER_U_STRING("WordList");
	case ScoreFunction:
		return _TRMEISTER_U_STRING("ScoreFunction");
	case AverageWordCount:
		return _TRMEISTER_U_STRING("AverageWordCount");
	case Tf:
		return _TRMEISTER_U_STRING("Tf");
	case String:
		return _TRMEISTER_U_STRING("String");
	case WordHead:
		return _TRMEISTER_U_STRING("WordHead");
	case WordTail:
		return _TRMEISTER_U_STRING("WordTail");
	case ClusteredLimit:
		return _TRMEISTER_U_STRING("ClusteredLimit");
	case Synonym:
		return _TRMEISTER_U_STRING("Synonym");
	case KwicSize:
		return _TRMEISTER_U_STRING("KwicSize");
	case Kwic:
		return _TRMEISTER_U_STRING("Kwic");
	case ExpandSynonym:
		return _TRMEISTER_U_STRING("ExpandSynonym");
	case MatchMode:
		return _TRMEISTER_U_STRING("MatchMode");
	case KwicStartTag:
		return _TRMEISTER_U_STRING("KwicStartTag");
	case KwicEndTag:
		return _TRMEISTER_U_STRING("KwicEndTag");
	case KwicEscape:
		return _TRMEISTER_U_STRING("KwicEscape");
	case KwicEllipsis:
		return _TRMEISTER_U_STRING("KwicEllipsis");
	case ScoreCombiner:
		return _TRMEISTER_U_STRING("ScoreCombiner");
	case ClusteredCombiner:
		return _TRMEISTER_U_STRING("ClusteredCombiner");
	case ClusterID:
		return _TRMEISTER_U_STRING("ClusterID");
	case FeatureValue:
		return _TRMEISTER_U_STRING("FeatureValue");
	case RoughKwicPosition:
		return _TRMEISTER_U_STRING("RoughKwicPosition");
	case Section:
		return _TRMEISTER_U_STRING("Section");
	case AverageCharLength:
		return _TRMEISTER_U_STRING("AverageCharLength");
	case WordDf:
		return _TRMEISTER_U_STRING("WordDf");
	case WordScale:
		return _TRMEISTER_U_STRING("WordScale");
	case Existence:
		return _TRMEISTER_U_STRING("Existence");
	case Append:
		return _TRMEISTER_U_STRING("Append");
	case Replace:
		return _TRMEISTER_U_STRING("Replace");
	case NeighborIn:
		return _TRMEISTER_U_STRING("NeighborIn");
	case NeighborID:
		return _TRMEISTER_U_STRING("NeighborID");
	case NeighborDistance:
		return _TRMEISTER_U_STRING("NeighborDistance");
	case Pair:
		return _TRMEISTER_U_STRING("Pair");
	case List:
		return _TRMEISTER_U_STRING("List");
	case Undefined:
		return _TRMEISTER_U_STRING("Undefined");
	default:
		return _TRMEISTER_U_STRING("[Unknown type of tree node]");
	}
}

//
//	Copyright (c) 1999, 2000, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
