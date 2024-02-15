// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ValueExpression.cpp -- ValueExpression
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/Externalizable.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/SQLParserL.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"
#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"
#include "Statement/VariableName.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/FloatData.h"
#include "Common/DoubleData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedInteger64Data.h"

#include "Exception/NumericValueOutOfRange.h"
#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/ValueExpression.h"
#endif

#include "Analysis/Value/ValueExpression.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Operator,
		f_ValueType,
		f_ExpressionType,
		f_Left,
		f_Right,
		f_Option,
		f_Primary,
		f_Function,
		f_Quantifier,
		f_Hint,
		f__end_index
	};

	// Get operator name for creating SQL statement
	struct OperatorTraits {
		enum Position {				 	// what place operator is located
			NoOperator,					//  - operator which is not used
			PrefixOperator,				//  - prefix operator
			PostfixOperator,			//  - postfix operator
			InfixOperator,				//  - infix operator
			LeftIsOperator				//  - left hand operand is teared as operator
		};
		enum Separator {	 			// what character is placed as separator
			None,						//  - nothing
			Space,						//  - space
			Paren,						//  - parenthesis
			Bracket,					//  - brackets
			Comma						//  - comma
		};
		enum Operand {					// what member operands is held
			Primary,					//  - primary
			Left,						//  - left
			Right,						//  - left/right
			List,						//  - list
			NoOperand					//  - no operands
		};
		const char* m_pszOperator;		// operator name
		Position m_ePosition;
		Separator m_eSeparator;
		Separator m_eDelimiter;
		Operand m_eOperand;

		void putPrefixSeparator(ModUnicodeOstrStream& cStream_) const
		{
			switch (m_eSeparator) {
			case OperatorTraits::Space: {cStream_ << ' '; break;}
			case OperatorTraits::Paren: {cStream_ << '('; break;}
			case OperatorTraits::Bracket: {cStream_ << '['; break;}
			default: {break;}
			}
		}
		void putPostfixSeparator(ModUnicodeOstrStream& cStream_) const
		{
			switch (m_eSeparator) {
			case OperatorTraits::Space: {cStream_ << ' '; break;}
			case OperatorTraits::Paren: {cStream_ << ')'; break;}
			case OperatorTraits::Bracket: {cStream_ << ']'; break;}
			default: {break;}
			}
		}
		void putDelimiter(ModUnicodeOstrStream& cStream_) const
		{
			switch (m_eDelimiter) {
			case OperatorTraits::Space: {cStream_ << ' '; break;}
			case OperatorTraits::Paren: {cStream_ << '('; break;}
			case OperatorTraits::Bracket: {cStream_ << '['; break;}
			case OperatorTraits::Comma: {cStream_ << ", "; break;}
			default: {break;}
			}
		}

	} _OperatorTraitsTable[] =
	{
#define nop OperatorTraits::NoOperator
#define pre OperatorTraits::PrefixOperator
#define pst OperatorTraits::PostfixOperator
#define inf OperatorTraits::InfixOperator
#define lop OperatorTraits::LeftIsOperator
#define non OperatorTraits::None
#define spc OperatorTraits::Space
#define prn OperatorTraits::Paren
#define brk OperatorTraits::Bracket
#define cmm OperatorTraits::Comma
#define prm OperatorTraits::Primary
#define lft OperatorTraits::Left
#define rgt OperatorTraits::Right
#define lst OperatorTraits::List
#define noo OperatorTraits::NoOperand
		{0,
		 nop, non, non, noo},
		{0,								//op_List = 1
		 nop, non, non, noo}, 
		{0,								//op_Literal
		 pre, non, non, prm},
		{"null",						//op_Nullobj
		 pre, non, non, noo},
		{0,								//op_Itemref
		 pre, non, non, prm},
		{0,								//op_Rowref
		 nop, non, non, noo},
		{0,								//op_Arrayref
		 lop, brk, non, rgt},
		{"row",							//op_Rowconst
		 pre, prn, cmm, prm},
		{0,								//op_Tblconst
		 pre, non, cmm, prm},
		{"array",						//op_Arrayconst
		 pre, brk, cmm, prm},
		{"?",							//op_Placeholder
		 pre, non, non, prm},
		{0,								//op_ColumnName
		 pre, non, non, prm},
		{" + ",							//op_Add
		 inf, prn, non, rgt},
		{" - ",							//op_Sub
		 inf, prn, non, rgt},
		{" * ",							//op_Mul
		 inf, non, non, rgt},
		{" / ",							//op_Div
		 inf, non, non, rgt},
		{"-",							//op_Neg
		 pre, non, non, lft},
		{"abs",							//op_Abs
		 pre, prn, non, lft},
		{" and ",						//op_And
		 inf, non, non, lst},
		{" or ",						//op_Or
		 inf, non, non, lst},
		{"not",							//op_Not
		 pre, prn, non, lft},
		{" || ",						//op_String_concat
		 inf, non, non, rgt},
		{" = ",							//op_Eq
		 inf, non, non, rgt},
		{" != ",						//op_Ne
		 inf, non, non, rgt},
		{" <= ",						//op_Le
		 inf, non, non, rgt},
		{" < ",							//op_Lt
		 inf, non, non, rgt},
		{" >= ",						//op_Ge
		 inf, non, non, rgt},
		{" > ",							//op_Gt
		 inf, non, non, rgt},
		{" between ",					//op_Between
		 inf, non, spc, rgt},
		{0,								//op_Like
		 pre, non, non, prm},
		{" contains ",					//op_Contains
		 pre, non, non, prm},
		{" is null ",					//op_IsNull
		 pst, non, non, prm},
		{"exists",						//op_Exists
		 pre, prn, non, prm},
		{0,								//op_In
		 pre, non, non, prm},
		{" is not null ",				//op_IsNotNull
		 pst, non, non, prm},
		{0,								//op_Func
		 pre, prn, spc, prm},
		{"wordhead",					//op_WordHead
		 pre, prn, spc, prm},
		{"wordtail",					//op_WordTail
		 pre, prn, spc, prm},
		{"simpleword",					//op_SimpleWord
		 pre, prn, spc, prm},
		{"exactword",					//op_ExactWord
		 pre, prn, spc, prm},
		{"approximateword",				//op_ApproximateWord
		 pre, prn, spc, prm},
		{0,								//op_Similar
		 pre, non, non, prm},
		{0,								//op_GeneratorDefinition
		 pre, non, non, prm},
		{0,								//op_GeneratorOption
		 pre, non, non, prm},
		{"path",						//op_PathName
		 pre, spc, non, prm},
		{"default",						//op_Defaultobj
		 pre, non, non, noo},
		{" not between ",				//op_NotBetween
		 inf, non, spc, rgt},
		{"not like",					//op_NotLike
		 pre, non, non, prm},
		{"not ",						//op_NotIn
		 pre, prn, non, prm},
		{"not similar",					//op_NotSimilar
		 pre, prn, non, prm},
		{0,								// op_RowSubquery
		 nop, prn, non, prm},
		{0,								// op_Case
		 nop, non, non, noo},
		{0,								// op_When
		 nop, non, non, noo},
		{0,								// op_Variable
		 nop, non, non, noo},
		{"mod",							// op_Mod
		 pre, prn, cmm, rgt},
		{"is substring of",				// op_IsSubstringOf
		 inf, prn, non, rgt},
#undef nop
#undef pre
#undef pst
#undef inf
#undef lop
#undef non
#undef spc
#undef prn
#undef brk
#undef cmm
#undef prm
#undef lft
#undef rgt
#undef lst
#undef noo
	};

	int _ExpressionTypeTable[ValueExpression::type_ValueNum][ValueExpression::type_ValueNum] =
	{
#define U ValueExpression::type_Unknown
#define C ValueExpression::type_Constant
#define V ValueExpression::type_Value
#define S ValueExpression::type_SingleColumn
#define M ValueExpression::type_MultipleColumn
#define A ValueExpression::type_Aggregation
		// U  C  V  S  M  A
		{  U, U, U, U, U, A},			// U
		{  U, C, V, S, M, A},			// C
		{  U, V, V, S, M, A},			// V
		{  U, S, S, M, M, A},			// S
		{  U, M, M, M, M, A},			// M
		{  A, A, A, A, A, A}			// A
#undef U
#undef C
#undef V
#undef S
#undef M
#undef A
	};

namespace _Function
{
	// 関数名から関数の種類を引く
	typedef Utility::NameTable::Map Map;
	typedef Utility::NameTable::Entry Entry;

	Map _cMap;

	Entry _functions[] = {
		{ "CARDINALITY",	ValueExpression::func_Cardinality		},
		{ "CHAR_JOIN",		ValueExpression::func_Char_Join			},
		{ "CHAR_LENGTH",	ValueExpression::func_Char_Length		},
		{ "COALESCE",		ValueExpression::func_Coalesce			},
		{ "CLUSTER",		ValueExpression::func_Cluster			},
		{ "FULLTEXT_LENGTH",ValueExpression::func_FullText_Length	},
		{ "OCTET_LENGTH",	ValueExpression::func_Octet_Length		},
		{ "WORD_COUNT",		ValueExpression::func_Word_Count		},
		{ "TF",				ValueExpression::func_Tf				},
		{ "GROUPING_ELEMENT",ValueExpression::func_Grouping_Element	},
		{ "EXISTENCE",		ValueExpression::func_Existence			},
		{ "NEIGHBOR",		ValueExpression::func_Neighbor			},
		{ 0, 0 }
	};

	Map _cClusterMap;

	Entry _clusterMembers[] = {
		{ "ID",				ValueExpression::func_ClusterId			},
		{ "KEYWORD",		ValueExpression::func_ClusterWord		},
		{ 0, 0 }
	};

	Map _cNeighborMap;

	Entry _neighborMembers[] = {
		{ "ID",				ValueExpression::func_NeighborId			},
		{ "DISTANCE",		ValueExpression::func_NeighborDistance		},
		{ 0, 0 }
	};

	struct TypeEntry
	{
		int m_iType;
		bool m_b0;
		bool m_b1;
		bool m_b2;
		bool m_b3;
		bool m_b4;
		bool m_b5;
		bool m_b6;
		bool m_bN;
		bool m_bAgg;
		const char* m_pszName;
		const char* m_pszMemberName;
	} _cTypeTable[ValueExpression::func_ValueNum] = {
#define U ValueExpression::Unknown
#define C ValueExpression::CharString
#define N ValueExpression::Numeric
#define A ValueExpression::Array
#define D ValueExpression::Datetime
	  // type	0arg	1arg	2arg	3arg	4arg	5arg	6arg	Narg	agg		name		member
		{U,		false,	false,	false,	false,	false,	false,	false,	false,	false,	"???",		0},//func_Unknown
		{C,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"user",		0},
		{C,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"session_user",0},
		{C,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"current_user",0},
		{C,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"current_path",0},
		{U,	   	false,	false,	false,	false,	false,	false,	false,	false,	false,	"???",		0},//func_Value
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	 true,	"count",	0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	 true,	"avg",		0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	 true,	"max",		0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	 true,	"min",		0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	 true,	"sum",		0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"char_length",0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"octet_length",0},
		{C,		false,	false,	 true,	 true,	false,	false,	false,	false,	false,	"substring",0},
		{C,		false,	false,	false,	 true,	 true,	false,	false,	false,	false,	"overlay",	0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"word_count",0},
		{A,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"tf",		0},
		{D,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"current_date",0},
		{D,		 true,	false,	false,	false,	false,	false,	false,	false,	false,	"current_timestamp",0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"fulltext_length",0},
		{C,		false,	false,	 true,	false,	false,	false,	false,	false,	false,	"normalize",0},
		{U,		false,	false,	false,	false,	false,	false,	false,	false,	false,	"cluster",	0},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"cluster",	"id"},
		{A,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"cluster",	"keyword"},
		{C,		false,	false,	false,	false,	false,	false,	 true,	false,	false,	"kwic",		0},
		{A,		false,	false,	 true,	false,	false,	false,	false,	false,	false,	"expand_synonym",0},
		{U,		false,	false,	 true,	 true,	 true,	 true,	 true,	false,	false,	"coalesce",	0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"cardinality",0},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"score",	0},
		{A,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"sectionized",0},
		{U,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"word",		0},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"word",		"df"},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"word",		"scale"},
		{C,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"char_join",0},
		{U,		true,	false,	false,	false,	false,	false,	false,	 false,	false,	"grouping_element",0},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"existence",	0},
		{U,		true,	 true,	 true,	 true,	 true,	 true,	 true,	 true,	false,	"???",		0},
		{N,		false,	false,	false,	false,	false,	false,	false,	 true,	false,	"get max",	0},
		{U,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"neighbor",	0},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"neighbor",	"id"},
		{N,		false,	 true,	false,	false,	false,	false,	false,	false,	false,	"neighbor",	"distance"},
#undef U
#undef C
#undef N
#undef A
#undef D
	};

	// 与えられた文字列が対応する関数種別があれば返す
	const Entry* getEntry(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
	{
		return _cMap.getEntry(&_functions[0], pHead_, pTail_);
	}
	// 与えられた文字列が対応するメンバー関数種別があれば返す
	const Entry* getEntry(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
						  const ModUnicodeChar* pMemberHead_, const ModUnicodeChar* pMemberTail_)
	{
		const Entry* pFuncEntry = getEntry(pHead_, pTail_);
		if (pFuncEntry) {
			switch (pFuncEntry->_value) {
			case ValueExpression::func_Cluster:
				{
					return _cClusterMap.getEntry(&_clusterMembers[0], pMemberHead_, pMemberTail_);
				}
			case ValueExpression::func_Neighbor:
				{
					return _cNeighborMap.getEntry(&_neighborMembers[0], pMemberHead_, pMemberTail_);
				}
			default:
				break;
			}
		}
		return 0;
	}

} // namespace _Function

namespace _Externalizable
{
	enum SpecialClass
	{
		SubString = 0,
		Overlay,
		Normalize,
		Kwic,
		ExpandSynonym,
		GeneratorDefinition,
		GeneratorOption,
		SimpleCase,
		SearchedCase,
		SimpleWhen,
		SearchedWhen,
		ValueNum
	};
} // namespace _Externalizable
} // namespace
//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (1)
//
//	NOTES
//		コンストラクタ (1)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
ValueExpression::ValueExpression()
	: Object(ObjectType::ValueExpression, f__end_index)
{
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iOperator_
//		ValueExpression* pLeft_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, ValueExpression* pLeft_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// Left を設定する
	setLeft(pLeft_);
	if(pLeft_){
		// ValueType を設定する
		setValueType(pLeft_->getValueType());
		// ExpressionType を設定する
		setExpressionType(pLeft_->getExpressionType());
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (3)
//
//	NOTES
//		コンストラクタ (3)
//
//	ARGUMENTS
//		int iOperator_
//		ValueExpression* pLeft_
//		ValueExpression* pRight_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, ValueExpression* pLeft_,
								 ValueExpression* pRight_)
	: Object(ObjectType::ValueExpression, f__end_index)
{

	// Operator を設定する
	setOperator(iOperator_);

	// ValueType を設定する
	if(pLeft_->getValueType() == pRight_->getValueType())
	{
		setValueType(pLeft_->getValueType());
	}
	else
	{
		setValueType(Unknown);
	}
	// ExpressionType を設定する
	setExpressionType(_ExpressionTypeTable[pLeft_->getExpressionType()][pRight_->getExpressionType()]);

	switch ( iOperator_ )
	{
	case op_Or:
	case op_And:
		//op_Or, op_And の4つの場合は、構文木が深くならないように
		//operand の取得方法を、getLeft(),getRight() から getOperandList() に変更する。
		if (iOperator_ == pLeft_->getOperator())
		{
			setOperandList(pLeft_->getOperandList());
			pLeft_->setOperandList(0);//pLeft_ のOperandList の所有権を剥奪
			delete pLeft_;
		}
		else
		{
			setOperandList(new ValueExpressionList(pLeft_));
		}
		if (iOperator_ == pRight_->getOperator())
		{
			getOperandList()->merge(*pRight_->getOperandList());//pRight_の OperandList をつなげる。
			delete pRight_;//pRight_の OperandList は空になっている
		}
		else
		{
			getOperandList()->append(pRight_);
		}
		break;
	default:
		// Left を設定する
		setLeft(pLeft_);
		// Right を設定する
		setRight(pRight_);
		break;
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (4)
//
//	NOTES
//		コンストラクタ (4)
//
//	ARGUMENTS
//		int iOperator_
//		ValueExpression* pLeft_
//		ValueExpression* pRight_
//		ValueExpression* pOption_
//		int iValueType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, ValueExpression* pLeft_,
								 ValueExpression* pRight_, ValueExpression* pOption_,
								 int iValueType_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// Left を設定する
	setLeft(pLeft_);
	// Right を設定する
	setRight(pRight_);
	// ValueType を設定する
	setValueType(iValueType_);
	// ExpressionType を設定する
	setExpressionType(_ExpressionTypeTable[pLeft_->getExpressionType()][pRight_->getExpressionType()]);
	if(pOption_){
		// Option を設定する
		setOption(pOption_);
		// ExpressionType を設定する
		setExpressionType(_ExpressionTypeTable[getExpressionType()][pOption_->getExpressionType()]);
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (5)
//
//	NOTES
//		コンストラクタ (5)
//
//	ARGUMENTS
//		int iOperator_
//		ValueExpression* pLeft_
//		ValueExpression* pRight_
//		int iValueType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, ValueExpression* pLeft_,
								 ValueExpression* pRight_, int iValueType_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// Left を設定する
	setLeft(pLeft_);
	// Right を設定する
	setRight(pRight_);
	// ExpressionType を設定する
	setExpressionType(_ExpressionTypeTable[pLeft_->getExpressionType()][pRight_->getExpressionType()]);
	// ValueType を設定する
	setValueType(iValueType_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (6)
//
//	NOTES
//		コンストラクタ (6)
//
//	ARGUMENTS
//		int iOperator_
//		Statement::Object* pPrimary_
//		int iValueType_
//		int iExpressionType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, Object* pPrimary_,
								 int iValueType_, int iExpressionType_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// ValueType を設定する
	setValueType(iValueType_);
	// ExpressionType を設定する
	setExpressionType(iExpressionType_);
	// Primary を設定する
	setPrimary(pPrimary_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (7)
//
//	NOTES
//		コンストラクタ (7)
//
//	ARGUMENTS
//		Literal* pLit_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(Literal* pLit_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(ValueExpression::op_Literal);
	// Primary を設定する
	setPrimary(pLit_);
	// ExpressionType を設定する
	setExpressionType(type_Constant);
	// ValueType を設定する
	switch(pLit_->getToken().getToken()){
	case TOKEN__FLOAT_LITERAL:
	case TOKEN__INTEGER_LITERAL:
			setValueType(Numeric);
			break;
	case TOKEN__DATE:
	case TOKEN__TIMESTAMP:
			setValueType(Datetime);
			break;
	case TOKEN__STRING:
			setValueType(CharString);
			break;
	default:
		setValueType(Unknown);
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::ValueExpression -- コンストラクタ (8)
//
//	NOTES
//		コンストラクタ (8)
//
//	ARGUMENTS
//		int iOperator_
//		int iFunction_
//		int iValueType_
//		int iQuantifier_
//		ValueExpression* pLeft_
//		int iExpressionType_
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression::ValueExpression(int iOperator_, int iFunction_, int iValueType_,
								 int iQuantifier_, ValueExpression* pLeft_,
								 int iExpressionType_,
								 Hint* pHint_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// Function を設定する
	setFunction(iFunction_);
	// ValueType を設定する
	setValueType(iValueType_);
	// Quantifier を設定する
	setQuantifier(iQuantifier_);
	// Left を設定する
	setLeft(pLeft_);
	// ExpressionType を設定する
	setExpressionType(iExpressionType_);
	// Hint を設定する
	setHint(pHint_);
}

// FUNCTION public
//	Statement::ValueExpression::ValueExpression -- コンストラクタ (9)
//
// NOTES
//
// ARGUMENTS
//	int iOperator_
//	int iFunction_
//	int iValueType_
//	ValueExpressionList* pOperand_
//	int iExpressionType_
//	Hint* pHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ValueExpression::
ValueExpression(int iOperator_,
				int iFunction_,
				int iValueType_,
				ValueExpressionList* pOperand_,
				int iExpressionType_,
				Hint* pHint_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	// Operator を設定する
	setOperator(iOperator_);
	// Function を設定する
	setFunction(iFunction_);
	// ValueType を設定する
	setValueType(iValueType_);
	// Primary を設定する
	setPrimary(pOperand_);
	// ExpressionType を設定する
	setExpressionType(iExpressionType_);
	// Hint を設定する
	setHint(pHint_);
}

// FUNCTION public
//	Statement::ValueExpression::ValueExpression -- コンストラクタ (10)
//
// NOTES
//
// ARGUMENTS
//	VariableName* pVarName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ValueExpression::
ValueExpression(VariableName* pVarName_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	setOperator(ValueExpression::op_Variable);
	setPrimary(pVarName_);
	setValueType(ValueExpression::Unknown);
	setExpressionType(ValueExpression::type_Value);
}

// FUNCTION public
//	Statement::ValueExpression::ValueExpression -- コンストラクタ (11)
//
// NOTES
//
// ARGUMENTS
//	Identifier* pFunctionName_
//	ValueExpressionList* pOperand_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ValueExpression::
ValueExpression(Identifier* pFunctionName_, ValueExpressionList* pOperand_)
	: Object(ObjectType::ValueExpression, f__end_index)
{
	setOperator(ValueExpression::op_Func);
	setFunction(ValueExpression::func_Invoke);
	setPrimary(pFunctionName_);
	setOperandList(pOperand_);
	setExpressionType(ValueExpression::type_Value);
}

// FUNCTION public
//	Statement::ValueExpression::create -- 名前から作るコンストラクター
//
// NOTES
//
// ARGUMENTS
//	Identifier* pFunctionName_
//	
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ValueExpression::
create(Identifier* pFunctionName_)
{
	; _SYDNEY_ASSERT(pFunctionName_);
	const _Function::Entry* pEntry = _Function::getEntry(pFunctionName_->getToken().getHead(),
														 pFunctionName_->getToken().getTail());
	if (!pEntry) {
		// invoke function
		return new ValueExpression(_SYDNEY_DYNAMIC_CAST(Identifier*,
														pFunctionName_->copy()),
								   0);
	}

	return create(pEntry->_value, 0);
}

// FUNCTION public
//	Statement::ValueExpression::create -- 
//
// NOTES
//
// ARGUMENTS
//	Identifier* pFunctionName_
//	Identifier* pMemberName_
//	
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ValueExpression::
create(Identifier* pFunctionName_, Identifier* pMemberName_)
{
	; _SYDNEY_ASSERT(pFunctionName_);
	const _Function::Entry* pEntry = _Function::getEntry(pFunctionName_->getToken().getHead(),
														 pFunctionName_->getToken().getTail(),
														 pMemberName_->getToken().getHead(),
														 pMemberName_->getToken().getTail());
	if (!pEntry) {
		ModUnicodeOstrStream cStream;
		ModUnicodeString cName(pFunctionName_->getToken().getHead(), pFunctionName_->getToken().getLength());
		ModUnicodeString cMemberName(pMemberName_->getToken().getHead(), pMemberName_->getToken().getLength());
		cStream << "Unknown function: " << cName << "." << cMemberName;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, cStream.getString());
	}
	return create(pEntry->_value, 0);
}

// FUNCTION public
//	Statement::ValueExpression::create -- 
//
// NOTES
//
// ARGUMENTS
//	Identifier* pFunctionName_
//	ValueExpressionList* pArgs_
//	Hint* pHint_
//	
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ValueExpression::
create(Identifier* pFunctionName_, ValueExpressionList* pArgs_, Hint* pHint_)
{
	; _SYDNEY_ASSERT(pFunctionName_);
	const _Function::Entry* pEntry = _Function::getEntry(pFunctionName_->getToken().getHead(),
														 pFunctionName_->getToken().getTail());
	if (!pEntry) {
		if (pHint_) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		return new ValueExpression(_SYDNEY_DYNAMIC_CAST(Identifier*,
														pFunctionName_->copy()),
								   new ValueExpressionList(*pArgs_));
	}

	return create(pEntry->_value, pArgs_, pHint_);
}

// FUNCTION public
//	Statement::ValueExpression::create -- 
//
// NOTES
//
// ARGUMENTS
//	Identifier* pFunctionName_
//	Identifier* pMemberName_
//	ValueExpressionList* pArgs_
//	Hint* pHint_
//	
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ValueExpression::
create(Identifier* pFunctionName_, Identifier* pMemberName_, ValueExpressionList* pArgs_, Hint* pHint_)
{
	; _SYDNEY_ASSERT(pFunctionName_);
	const _Function::Entry* pEntry = _Function::getEntry(pFunctionName_->getToken().getHead(),
														 pFunctionName_->getToken().getTail(),
														 pMemberName_->getToken().getHead(),
														 pMemberName_->getToken().getTail());
	if (!pEntry) {
		ModUnicodeOstrStream cStream;
		ModUnicodeString cName(pFunctionName_->getToken().getHead(), pFunctionName_->getToken().getLength());
		ModUnicodeString cMemberName(pMemberName_->getToken().getHead(), pMemberName_->getToken().getLength());
		cStream << "Unknown function: " << cName << "." << cMemberName;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, cStream.getString());
	}

	return create(pEntry->_value, pArgs_, pHint_);
}

// FUNCTION public
//	Statement::ValueExpression::create -- 関数を作るコンストラクター
//
// NOTES
//
// ARGUMENTS
//	int iFunction_
//	ValueExpressionList* pArgs_
//	Hint* pHint_
//	
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

//static
ValueExpression*
ValueExpression::
create(int iFunction_, ValueExpressionList* pArgs_, Hint* pHint_)
{
	; _SYDNEY_ASSERT(iFunction_ >= 0);
	; _SYDNEY_ASSERT(iFunction_ < func_ValueNum);

	// check the number of arguments
	int nArgs = pArgs_ ? pArgs_->getCount() : 0;

	// check validity of the number of arguments
	const _Function::TypeEntry& cType = _Function::_cTypeTable[iFunction_];
	if (!cType.m_bN
		&& ((nArgs == 0 && !cType.m_b0)
			|| (nArgs == 1 && !cType.m_b1)
			|| (nArgs == 2 && !cType.m_b2)
			|| (nArgs == 3 && !cType.m_b3)
			|| (nArgs == 4 && !cType.m_b4)
			|| (nArgs == 5 && !cType.m_b5)
			|| (nArgs == 6 && !cType.m_b6))) {
		ModUnicodeOstrStream cStream;
		cStream << "Illegal number of argument for function: " << cType.m_pszName;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, cStream.getString());
	}

	switch (nArgs) {
	case 0:
		{
			return new ValueExpression(op_Func, iFunction_, cType.m_iType, quant_None,
									   0, type_Value,
									   pHint_);
		}
	case 1:
		{
			return new ValueExpression(op_Func, iFunction_, cType.m_iType, quant_None,
									   _SYDNEY_DYNAMIC_CAST(ValueExpression*, pArgs_->getValueExpressionAt(0)->copy()),
									   pArgs_->getValueExpressionAt(0)->getExpressionType(),
									   pHint_);
		}
	default:
		{
			return new ValueExpression(op_Func, iFunction_, cType.m_iType,
									   new ValueExpressionList(*pArgs_),
									   pArgs_->getExpressionType(),
									   pHint_);
		}
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::getOperator -- Operator を得る
//
//	NOTES
//		Operator を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ValueExpression::getOperator() const
{
	return getIntegerElement(f_Operator);
}

//
//	FUNCTION public
//		Statement::ValueExpression::setOperator -- Operator を設定する
//
//	NOTES
//		Operator を設定する
//
//	ARGUMENTS
//		int iOperator_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setOperator(int iOperator_)
{
	setIntegerElement(f_Operator, iOperator_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getValueType -- ValueType を得る
//
//	NOTES
//		ValueType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ValueExpression::getValueType() const
{
	return getIntegerElement(f_ValueType);
}

//
//	FUNCTION public
//		Statement::ValueExpression::setValueType -- ValueType を設定する
//
//	NOTES
//		ValueType を設定する
//
//	ARGUMENTS
//		int iValueType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setValueType(int iValueType_)
{
	setIntegerElement(f_ValueType, iValueType_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getExpressionType -- ExpressionType を得る
//
//	NOTES
//		ExpressionType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ValueExpression::getExpressionType() const
{
	return getIntegerElement(f_ExpressionType);
}

//
//	FUNCTION public
//		Statement::ValueExpression::setExpressionType -- ExpressionType を設定する
//
//	NOTES
//		ExpressionType を設定する
//
//	ARGUMENTS
//		int iExpressionType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setExpressionType(int iExpressionType_)
{
	setIntegerElement(f_ExpressionType, iExpressionType_);
}

// FUNCTION public
//	Statement::ValueExpression::mergeExpressionType -- ExpressionType を合成する
//
// NOTES
//
// ARGUMENTS
//	int iType1_
//	int iType2_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
ValueExpression::
mergeExpressionType(int iType1_, int iType2_)
{
	; _SYDNEY_ASSERT(iType1_ >= 0);
	; _SYDNEY_ASSERT(iType1_ < type_ValueNum);
	; _SYDNEY_ASSERT(iType2_ >= 0);
	; _SYDNEY_ASSERT(iType2_ < type_ValueNum);
	return _ExpressionTypeTable[iType1_][iType2_];
}

// FUNCTION public
//	Statement::ValueExpression::mergeExpressionType -- merge expression type
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pExp1_
//	ValueExpression* pExp2_
//	ValueExpression* pExp3_ = 0
//	ValueExpression* pExp4_ = 0
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
ValueExpression::
mergeExpressionType(ValueExpression* pExp1_,
					ValueExpression* pExp2_,
					ValueExpression* pExp3_ /* = 0 */,
					ValueExpression* pExp4_ /* = 0 */,
					ValueExpression* pExp5_ /* = 0 */,
					ValueExpression* pExp6_ /* = 0 */)
{
	return _ExpressionTypeTable
			[_ExpressionTypeTable
				[pExp1_->getExpressionType()]
				[pExp2_->getExpressionType()]]
			[_ExpressionTypeTable
				[_ExpressionTypeTable
					[pExp3_ ? pExp3_->getExpressionType() : type_Constant]
					[pExp4_ ? pExp4_->getExpressionType() : type_Constant]]
				[_ExpressionTypeTable
					[pExp5_ ? pExp5_->getExpressionType() : type_Constant]
					[pExp6_ ? pExp6_->getExpressionType() : type_Constant]]];
}

//
//	FUNCTION public
//		Statement::ValueExpression::getLeft -- Left を得る
//
//	NOTES
//		Left を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpression::getLeft() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*, getElement(f_Left, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::ValueExpression::setLeft -- Left を設定する
//
//	NOTES
//		Left を設定する
//
//	ARGUMENTS
//		ValueExpression* pLeft_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setLeft(ValueExpression* pLeft_)
{
	setElement(f_Left, pLeft_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getRight -- Right を得る
//
//	NOTES
//		Right を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpression::getRight() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*, getElement(f_Right, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::ValueExpression::setRight -- Right を設定する
//
//	NOTES
//		Right を設定する
//
//	ARGUMENTS
//		ValueExpression* pRight_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setRight(ValueExpression* pRight_)
{
	setElement(f_Right, pRight_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getOperandList -- OperandList を得る
//
//	NOTES
//		OperandList を得る
//		op_Or,op_And の4つの場合は、交換則と結合則が成り立つので
//		構文木が深くならないように、operand を List にして持つ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpressionList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpressionList*
ValueExpression::getOperandList() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpressionList*,
		getElement(f_Left, ObjectType::ValueExpressionList));
}

//
//	FUNCTION public
//		Statement::ValueExpression::setOperandList -- OperandList を設定する
//
//	NOTES
//		OperandList を設定する
//		op_Or,op_And の4つの場合は、交換則と結合則が成り立つので
//		構文木が深くならないように、operand を List にして持つ
//
//	ARGUMENTS
//		ValueExpressionList* pList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setOperandList(ValueExpressionList* pList_)
{
	setElement(f_Left, pList_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getOption -- Option を得る
//
//	NOTES
//		Option を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpression::getOption() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*, getElement(f_Option, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::ValueExpression::setOption -- Option を設定する
//
//	NOTES
//		Option を設定する
//
//	ARGUMENTS
//		ValueExpression* pOption_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setOption(ValueExpression* pOption_)
{
	setElement(f_Option, pOption_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getPrimary -- Primary を得る
//
//	NOTES
//		Primary を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Statement::Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
ValueExpression::getPrimary() const
{
	return m_vecpElements[f_Primary];
}

//
//	FUNCTION public
//		Statement::ValueExpression::setPrimary --  Primary を設定する
//
//	NOTES
//		Primary を設定する
//
//	ARGUMENTS
//		Statement::Object* pPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setPrimary(Object* pPrimary_)
{
	setElement(f_Primary, pPrimary_);
}

// FUNCTION public
//	Statement::ValueExpression::getHint -- Hint を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Hint*
//
// EXCEPTIONS

Hint*
ValueExpression::
getHint() const
{
	// check for instances from older versions
	if (m_vecpElements.getSize() > f_Hint) {
		return _SYDNEY_DYNAMIC_CAST(
					Hint*, getElement(f_Hint, ObjectType::Hint));
	}
	return 0;
}

// FUNCTION public
//	Statement::ValueExpression::setHint -- Hint を設定する
//
// NOTES
//
// ARGUMENTS
//	Hint* pHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ValueExpression::
setHint(Hint* pHint_)
{
	if (pHint_) {
		// check for instances from older versions
		if (m_vecpElements.getSize() > f_Hint) {
			setElement(f_Hint, pHint_);
		} else {
			// Hint is required to set, but the instance is older version
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

//
//	FUNCTION public
//		Statement::ValueExpression::getFunction -- Function を得る
//
//	NOTES
//		Function を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ValueExpression::getFunction() const
{
	return getIntegerElement(f_Function);
}

//
//	FUNCTION public
//		Statement::ValueExpression::setFunction -- Function を設定する
//
//	NOTES
//		Function を設定する
//
//	ARGUMENTS
//		int iFunction_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setFunction(int iFunction_)
{
	setIntegerElement(f_Function, iFunction_);
}

// 関数名を得る
//static
const char*
ValueExpression::
getFunctionName(int iFunction_)
{
	if (iFunction_ >= 0 && iFunction_ < func_ValueNum) {
		return _Function::_cTypeTable[iFunction_].m_pszName;
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//		Statement::ValueExpression::getQuantifier -- Quantifier を得る
//
//	NOTES
//		Quantifier を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ValueExpression::getQuantifier() const
{
	return getIntegerElement(f_Quantifier);
}

//
//	FUNCTION public
//		Statement::ValueExpression::setQuantifier -- Quantifier を設定する
//
//	NOTES
//		Quantifier を設定する
//
//	ARGUMENTS
//		int iQuantifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setQuantifier(int iQuantifier_)
{
	setIntegerElement(f_Quantifier, iQuantifier_);
}

//
//	FUNCTION public
//		Statement::ValueExpression::nullExpression -- null の ValuExpression を作る
//
//	NOTES
//		null の ValuExpression を作る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpression::nullExpression()
{
	return new ValueExpression(op_Nullobj, 0, Null, 1);
}

//
//	FUNCTION public
//		Statement::ValueExpression::defaultExpression -- default の ValuExpression を作る
//
//	NOTES
//		default の ValuExpression を作る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpression::defaultExpression()
{
	return new ValueExpression(op_Defaultobj, 0, Default, 1);
}

//
//	FUNCTION public
//		Statement::ValueExpression::isNumberLiteral -- 数値かどうか検査する
//
//	NOTES
//		数値かどうか検査する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
ValueExpression::isNumberLiteral() const
{
	return getValueType() == Numeric
	    && getOperator() == op_Literal;
}

	
//
//	FUNCTION public
//		Statement::ValueExpression::setSignNegative -- NumberLiteral の符号を反転する
//
//	NOTES
//		NumberLiteral の符号を反転する
//		今まではデータを作ってNegateしていたが
//		高速化のためトークンを前に延ばすのみ
//
//	ARGUMENTS
//		Token* pSign_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpression::setSignNegative(Token* pSign_)
{
	; _SYDNEY_ASSERT(isNumberLiteral());

	Literal* pLiteral = _SYDNEY_DYNAMIC_CAST(Literal*, getPrimary());
	; _SYDNEY_ASSERT( pLiteral );

	// リテラルが指すトークンの範囲に符号を含める
	const Token& cPrevToken = pLiteral->getToken();
	pLiteral->setToken(Token(cPrevToken.getToken(), pSign_->getHead(), cPrevToken.getTail()));
}

// PathNameか
bool
ValueExpression::
isPathName() const
{
	return getOperator() == op_PathName;
}

//
//	FUNCTION public
//		Statement::ValueExpression::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
ValueExpression::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;

	if (getOperator() == op_Func) {
		// function is generated in different way from other value expressions
		; _SYDNEY_ASSERT(getFunction() >= 0 && getFunction() < func_ValueNum);
		const _Function::TypeEntry& cEntry = _Function::_cTypeTable[getFunction()];
		if (getFunction() == func_Invoke) {
			cStream << getPrimary()->toSQLStatement(bForCascade_);
		} else {
			cStream << cEntry.m_pszName;
		}
		if (getFunction() == func_Count && getLeft() == 0) {
			cStream << "(*)";
		} else {
			if (cEntry.m_b1 || cEntry.m_bN) {
				cStream << '(';
				if (getQuantifier() == quant_Distinct) {
					cStream << "distinct ";
				}
				if (ValueExpression* pObject = getLeft()) {
					cStream << pObject->toSQLStatement(bForCascade_);
				} else if (Object* pPrimary = getPrimary()) {
					cStream << pPrimary->toSQLStatement(bForCascade_);
				} else {
					cStream << "(0)";
				}
				cStream << ')';
			}
		}
		if (cEntry.m_pszMemberName) {
			cStream << '.' << cEntry.m_pszMemberName;
		}
		return cStream.getString();
	}

	; _SYDNEY_ASSERT(getOperator() != op_Func);
	const OperatorTraits& cTraits = _OperatorTraitsTable[getOperator()];

	if (cTraits.m_pszOperator
		&& (cTraits.m_ePosition == OperatorTraits::NoOperand
			|| cTraits.m_ePosition == OperatorTraits::PrefixOperator)) {
		cStream << cTraits.m_pszOperator;
	}

	// put separator
	if (cTraits.m_ePosition != OperatorTraits::LeftIsOperator) {
		cTraits.putPrefixSeparator(cStream);
	}

	switch (cTraits.m_eOperand) {
	case OperatorTraits::Primary:
		{
			if (cTraits.m_ePosition != OperatorTraits::NoOperand) {
				if (Object* pObject = getPrimary()) {
					cStream << pObject->toSQLStatement(bForCascade_);
				} else {
					cStream << "(0)";
				}
			}
			break;
		}
	case OperatorTraits::Left:
		{
			if (ValueExpression* pObject = getLeft()) {
				cStream << pObject->toSQLStatement(bForCascade_);
			} else {
				cStream << "(0)";
			}

			// put separator
			if (cTraits.m_ePosition == OperatorTraits::LeftIsOperator) {
				cTraits.putPrefixSeparator(cStream);
			}
			break;
		}
	case OperatorTraits::Right:
		{
			if (ValueExpression* pObject = getLeft()) {
				cStream << pObject->toSQLStatement(bForCascade_);
			} else {
				cStream << "(0)";
			}
			// put separator
			if (cTraits.m_pszOperator
				&& (cTraits.m_ePosition == OperatorTraits::InfixOperator)) {
				cStream << cTraits.m_pszOperator;
			} else if (cTraits.m_ePosition == OperatorTraits::LeftIsOperator) {
				cTraits.putPrefixSeparator(cStream);
			} else {
				cTraits.putDelimiter(cStream);
			}
			if (ValueExpression* pObject = getRight()) {
				cStream << pObject->toSQLStatement(bForCascade_);
			} else {
				cStream << "(0)";
			}
			break;
		}
	case OperatorTraits::List:
		{
			if (ValueExpressionList* pList = getOperandList()) {
				int n = pList->getCount();
				for (int i = 0; i < n; ++i) {
					if (i > 0) {
						if (cTraits.m_pszOperator
							&& (cTraits.m_ePosition == OperatorTraits::InfixOperator)) {
							cStream << cTraits.m_pszOperator;
						} else {
							cTraits.putDelimiter(cStream);
						}
					}
					if (Statement::Object* pObj = pList->getAt(i)) {
						cStream << pObj->toSQLStatement(bForCascade_);
					} else {
						cStream << "(0)";
					}
				}
			} else {
				cStream << "(0)";
			}
			break;
		}
	default:
		break;
	}

	// put separator
	cTraits.putPostfixSeparator(cStream);

	if (cTraits.m_pszOperator
		&& (cTraits.m_ePosition == OperatorTraits::PostfixOperator)) {
		cStream << cTraits.m_pszOperator;
	}

	return cStream.getString();
}

#ifndef SYD_COVERAGE
//
//	FUNCTION 
//		Statement::ValueExpression::printChild -- 下位objectを出力する
//
//	NOTES
//		下位objectを出力する
//
//	ARGUMENTS
//		ModOstrStream& cStream_
//		int iIndent_
//		int iIdx_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
ValueExpression::printChild(ModUnicodeOstrStream& cStream_,
							int iIndent_, int iIdx_) const
{
	cStream_ << '\n';
	Object* pObj = m_vecpElements[iIdx_];
	if (pObj == 0) {
		for (int i=0; i<iIndent_+1; ++i)
			cStream_ << ' ';
		cStream_ << "null";
	} else {
		pObj->toString(cStream_, iIndent_+1);
	}
}

//
//	FUNCTION 
//		Statement::ValueExpression::toString -- Lisp形式で出力する
//
//	NOTES
//		Lisp形式で出力する
//
//	ARGUMENTS
//		ModOstrStream& cStream_
//		int iIndent_ = 0
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
ValueExpression::toString(ModUnicodeOstrStream& cStream_,
						  int iIndent_) const
{
	int i;
	for (i=0; i<iIndent_; ++i)
		cStream_ << ' ';
	
	cStream_ << '(';
	cStream_ << getTypeName(getType());
	cStream_ << ' ';

	bool bLeft = false;
	bool bRight = false;
	bool bOption = false;
	bool bPrimary = false;

	switch(getOperator()) {
	case op_List:			cStream_ << "List"; break;
	case op_Literal:		cStream_ << "Literal"; bPrimary = true; break;
	case op_PathName:		cStream_ << "PathName"; bPrimary = true; break;
	case op_Nullobj:		cStream_ << "Nullobj"; break;
	case op_Defaultobj:		cStream_ << "Defaultobj"; break;
	case op_Itemref:		cStream_ << "Itemref"; bPrimary = true; break;
	case op_Rowref:			cStream_ << "Rowref"; bPrimary = true; break;
	case op_Rowconst:		cStream_ << "Rowconst"; bPrimary = true; break;
	case op_Tblconst:		cStream_ << "Tblconst"; bPrimary = true; break;
	case op_Placeholder:	cStream_ << "Placeholder"; bPrimary = true; break;
	case op_Add:			cStream_ << "+"; bLeft = bRight = true; break;
	case op_Sub:			cStream_ << "-"; bLeft = bRight = true; break;
	case op_Mul:			cStream_ << "*"; bLeft = bRight = true; break;
	case op_Div:			cStream_ << "/"; bLeft = bRight = true; break;
	case op_Neg:			cStream_ << "Neg"; bLeft = true; break;
	case op_Abs:			cStream_ << "Abs"; bLeft = true; break;
	case op_And:			cStream_ << "And"; bLeft = bRight = true; break;
	case op_Or:				cStream_ << "Or" ; bLeft = bRight = true; break;
	case op_Not:			cStream_ << "Not"; bLeft = true; break;
	case op_String_concat:	cStream_ << "String_concat"; bLeft = bRight = true; break;
	case op_Eq:				cStream_ << "=="; bLeft = bRight = true; break;
	case op_Ne:				cStream_ << "!="; bLeft = bRight = true; break;
	case op_Le:				cStream_ << "<="; bLeft = bRight = true; break;
	case op_Lt:				cStream_ << "<"; bLeft = bRight = true; break;
	case op_Ge:				cStream_ << ">="; bLeft = bRight = true; break;
	case op_Gt:				cStream_ << ">"; bLeft = bRight = true; break;
	case op_Between:		cStream_ << "Between"; bLeft = bRight = true; break;
	case op_Like:			cStream_ << "Like"; bPrimary = true; break;
	case op_Similar:		cStream_ << "SimilarTo"; bPrimary = true; break;
	case op_Contains:		cStream_ << "Contains"; bLeft = bRight = true; break;
	case op_IsNull:			cStream_ << "IsNull"; bLeft = true; break;
	case op_IsNotNull:		cStream_ << "IsNotNull"; bLeft = true; break;
	case op_Exists:			cStream_ << "Exists"; bPrimary = true; break;
	case op_In:				cStream_ << "In"; bPrimary = true; break;
	case op_NotBetween:		cStream_ << "Not Between"; bLeft = bRight = true; break;
	case op_NotLike:		cStream_ << "Not Like"; bPrimary = true; break;
	case op_NotIn:			cStream_ << "Not In"; bPrimary = true; break;
	case op_NotSimilar:		cStream_ << "Not Similar"; bPrimary = true; break;
	case op_Func:
		cStream_ << "Func ";
		switch(getFunction()) {
		case func_User:		cStream_ << "User"; break;
		case func_Session_User:		cStream_ << "Session_User"; break;
		case func_Current_User:		cStream_ << "Current_User"; break;		
		case func_Current_Path:		cStream_ << "Current_Path"; break;
#ifdef OBSOLETE
		case func_Value:		cStream_ << "Value"; break;
#endif
		case func_Count:		cStream_ << "Count"; bLeft = true; break;
		case func_Avg:		cStream_ << "Avg"; bLeft = true; break;
		case func_Max:		cStream_ << "Max"; bLeft = true; break;
		case func_Min:		cStream_ << "Min"; bLeft = true; break;
		case func_Sum:		cStream_ << "Sum"; bLeft = true; break;
		case func_Char_Length:	cStream_ << "Char_Length"; bLeft = true; break;
		case func_Octet_Length:	cStream_ << "Octet_Length"; bLeft = true; break;

			//【注意】	SUBSTRING はすべての情報が出力されない
		case func_SubString:	cStream_ << "SubString"; bLeft = true; break;
			//【注意】	OVERLAY はすべての情報が出力されない
		case func_Overlay:	cStream_ << "Overlay"; bLeft = true; break;
			//【注意】	WORDCOUNT はすべての情報が出力されない
		case func_Word_Count:	cStream_ << "Word_Count"; bLeft = true; break;

		case func_Tf:	cStream_ << "Tf"; bLeft = true; break;
		case func_Existence: cStream_ << "Existence"; bLeft = true; break;
		case func_FullText_Length:	cStream_ << "FullText_Length"; bLeft = true; break;
			//【注意】	NORMALIZE はすべての情報が出力されない
		case func_Normalize:	cStream_ << "Normalize"; bLeft = true; break;

		case func_Cluster:	cStream_ << "Cluster"; bLeft = true; break;
		case func_ClusterId:cStream_ << "Cluster.id"; bLeft = true; break;
		case func_ClusterWord:cStream_ << "Cluster.keyword"; bLeft = true; break;
		
		case func_Kwic:cStream_ << "Kwic"; bLeft = true; break;

		case func_Current_Date:		cStream_ << "Current_Date"; break;
		case func_Current_Timestamp:cStream_ << "Current_Timestamp"; break;
			
			//【注意】	EXPAND_SYNONYM はすべての情報が出力されない
		case func_Expand_Synonym:	cStream_ << "Expand_Synonym"; bLeft = true; break;

		case func_Cardinality:		cStream_ << "Cardinality"; bLeft = true;	break;
		case func_Invoke:		cStream_ << "Invoke"; bPrimary = bLeft = true;	break;
		case func_GetMax:		cStream_ << "GetMax"; bPrimary = true;	break;

		case func_Neighbor:	cStream_ << "Neighbor"; bLeft = true; break;
		case func_NeighborId:cStream_ << "Neighbor.id"; bLeft = true; break;
		case func_NeighborDistance:cStream_ << "Neighbor.distance"; bLeft = true; break;

		default:			cStream_ << "func???"; break;
		}
		cStream_ << ' ';
		switch(getQuantifier()) {
		case quant_All: cStream_ << "All"; break;
		case quant_Distinct: cStream_ << "Distinct"; break;
		default: 			 cStream_ << "distinctness???"; break;
		}
	case op_WordHead:				cStream_ << "WordHead"; 	bPrimary = true;	break;
	case op_WordTail:				cStream_ << "WordTail"; 	bPrimary = true;	break;
	case op_SimpleWord:				cStream_ << "SimpleWord"; 	bPrimary = true;	break;
	case op_ExactWord:				cStream_ << "ExactWord";	bPrimary = true;	break;
	case op_ApproximateWord:		cStream_ << "ApproximateWord"; bPrimary = true;	break;
	case op_Mod:			cStream_ << "ModAbs"; bLeft = bRight = true; break;
	case op_IsSubstringOf:			cStream_ << "IsSubstringOf"; bLeft = bRight = true; break;

	default:				cStream_ << "op???"; break;
	}

	cStream_ << "  ";
	switch(getValueType()) {
	case Numeric: cStream_ << "Num"; break;
	case CharString:  cStream_ << "CharStr"; break;
	case Datetime:  cStream_ << "DT"; break;
	case Interval:  cStream_ << "Interval"; break;
	case Enumerate:  cStream_ << "Enum"; break;
	case Boolean:  cStream_ << "Bool"; break;
	case Null:  cStream_ << "nullobj"; break;
	case Default:  cStream_ << "defaultobj"; break;
	case Row:  cStream_ << "Row"; break;
	case Table:  cStream_ << "Table"; break;
	default:  cStream_ << "Type?"; break;
	}
	cStream_ << ' ';

	if (getExpressionType() == type_Constant) {
		cStream_ << "const";
	}

	// メンバの出力
	if (bPrimary)	printChild(cStream_, iIndent_, f_Primary);
	if (bLeft)		printChild(cStream_, iIndent_, f_Left);
	if (bRight)		printChild(cStream_, iIndent_, f_Right);
	if (bOption)	printChild(cStream_, iIndent_, f_Option);

	cStream_ << ')';
}

//
//	FUNCTION public
//		Statement::Object::toString -- 文字列で値を得る(デバッグ用)
//
//	NOTES
//		文字列で値を得る(デバッグ用)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

ModUnicodeString
ValueExpression::toString() const
{
	return toStringDefault(this);
}
#endif

//
//	FUNCTION public
//	Statement::ValueExpression::expandCondition -- And,Orを展開する
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
ValueExpression::expandCondition(int iDepth_)
{
	using namespace Statement;

	// 下位のノードを展開する
	Object::expandCondition(iDepth_);

	if (getOperator() == op_And || getOperator() == op_Or)
		{
			// 下位にANDかORがあったらこのノードへ集める
			mergeOperandList();
		}
}

// FUNCTION public
//	Statement::ValueExpression::setSeparated -- set a value expression as included in parenthesis
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ValueExpression::
setSeparated()
{
	if (getOperator() == op_Literal) {
		Literal* pLiteral = _SYDNEY_DYNAMIC_CAST(Literal*, getPrimary());
		; _SYDNEY_ASSERT(pLiteral);

		pLiteral->setSeparated();
	}
}

// get whether a value expression as included in parenthesis
bool
ValueExpression::
isSeparated() const
{
	if (getOperator() == op_Literal) {
		Literal* pLiteral = _SYDNEY_DYNAMIC_CAST(Literal*, getPrimary());
		; _SYDNEY_ASSERT(pLiteral);

		return pLiteral->isSeparated();
	}
	return true;
}

//
//	FUNCTION private
//	Statement::ValueExpression::mergeOperandList
//		-- 下位に同じオペレータのノードあったら展開する
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
ValueExpression::mergeOperandList()
{
	ValueExpressionList* pOrgList = getOperandList();

	if (int Size = pOrgList->getCount()) {
		// まずオペランドに同じOperatorのものがあるか調べる
		int i = 0;
		do {
			if (pOrgList->getValueExpressionAt(i)->getOperator() == getOperator())
				break;
		} while (++i < Size);

		if (i < Size) {
			// 同じOperatorのものがある -> 展開したオペランドに変更する
			ValueExpressionList* pNewList = new ValueExpressionList;
			//pNewList->setExpressionType(type_Constant);

			i = 0;
			do {
				ValueExpression* pValueExpression = pOrgList->getValueExpressionAt(i);
				if (pValueExpression->getOperator() == getOperator())
					{
						// 同じOperatorが下位にあったので展開する
						ValueExpressionList* pList = pValueExpression->getOperandList();
						pNewList->merge(*pList);
					}
				else
					{
						// その他なのでそのまま代入
						pNewList->append(pValueExpression);
						pOrgList->setAt(i, 0);	// クリア
					}
			} while (++i < Size);

			delete pOrgList;

			// 新しいものを設定する
			setOperandList(pNewList);
		}
	}
}

//
//	FUNCTION public
//	Statement::ValueExpression::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
ValueExpression::copy() const
{
	return new ValueExpression(*this);
}

// FUNCTION public
//	Statement::ValueExpression::getInstance -- 
//
// NOTES
//
// ARGUMENTS
//	int iClassID_
//	
// RETURN
//	Object*
//
// EXCEPTIONS

//static
Object*
ValueExpression::
getInstance(int iClassID_)
{
	if (iClassID_ < Statement::Externalizable::ClassID::ValueExpression) {
		return new ValueExpression;
	} else {
		switch (iClassID_ - Statement::Externalizable::ClassID::ValueExpression) {
		case _Externalizable::SubString:		return new Function::SubString;
		case _Externalizable::Overlay:			return new Function::Overlay;
		case _Externalizable::Normalize:		return new Function::Normalize;
		case _Externalizable::Kwic:				return new Function::Kwic;
		case _Externalizable::ExpandSynonym:	return new Function::ExpandSynonym;
		case _Externalizable::GeneratorDefinition:	return new Generator::Definition;
		case _Externalizable::GeneratorOption:	return new Generator::Option;
		case _Externalizable::SimpleCase:		return new Expression::SimpleCase;
		case _Externalizable::SearchedCase:		return new Expression::SearchedCase;
		case _Externalizable::SimpleWhen:		return new Expression::SimpleWhen;
		case _Externalizable::SearchedWhen:		return new Expression::SearchedWhen;
		default:								return 0;
		}
	}
}

//
//	FUNCTION public
//		Statement::Function::SubString::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
Function::SubString::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "substring(" << getSource()->toSQLStatement(bForCascade_) << " from " << getStartPosition()->toSQLStatement(bForCascade_);
	if (getStringLength())
		cStream << " for " << getStringLength()->toSQLStatement(bForCascade_);
	cStream << ")";

	return cStream.getString();
}

//	FUNCTION public
//	Statement::Function::SubString::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Function::SubString::copy() const
{
	return new SubString(*this);
}

// FUNCTION public
//	Statement::Function::SubString::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::SubString::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::SubString;
}

// FUNCTION public
//	Statement::Function::SubString::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::SubString::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, _start);
	Utility::Serialize::Object(cArchive_, _length);
}

//
//	FUNCTION public
//		Statement::Function::Overlay::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
Function::Overlay::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "overlay(" << getDestination()->toSQLStatement(bForCascade_)
			<< " placing " << getPlacing()->toSQLStatement(bForCascade_)
			<< " from " << getStartPosition()->toSQLStatement(bForCascade_);
	if (getStringLength())
		cStream << " for " << getStringLength()->toSQLStatement(bForCascade_);
	cStream << ")";

	return cStream.getString();
}

//	FUNCTION public
//	Statement::Function::Overlay::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Function::Overlay::copy() const
{
	return new Overlay(*this);
}

// FUNCTION public
//	Statement::Function::Overlay::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::Overlay::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::Overlay;
}

// FUNCTION public
//	Statement::Function::Overlay::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::Overlay::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, _placing);
	Utility::Serialize::Object(cArchive_, _start);
	Utility::Serialize::Object(cArchive_, _length);
}

//
//	FUNCTION public
//		Statement::Function::Normalize::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
Function::Normalize::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "normalize(" << getSource()->toSQLStatement(bForCascade_)
			<< " using " << getParameter()->toSQLStatement(bForCascade_)
			<< ")";

	return cStream.getString();
}

//	FUNCTION public
//	Statement::Function::Normalize::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Function::Normalize::copy() const
{
	return new Normalize(*this);
}

// FUNCTION public
//	Statement::Function::Normalize::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::Normalize::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::Normalize;
}

// FUNCTION public
//	Statement::Function::Normalize::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::Normalize::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, _parameter);
}

//
//	FUNCTION public
//		Statement::Function::Kwic::toSQLStatement -- SQL文を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
//virtual
ModUnicodeString
Function::Kwic::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	
	; _TRMEISTER_ASSERT(getSource() != 0);
	cStream << "kwic(" << getSource()->toSQLStatement(bForCascade_);
	
	; _TRMEISTER_ASSERT(m_pSize != 0);
	cStream << " for " << m_pSize->toSQLStatement(bForCascade_);

	if (m_pStartTag != 0)
		{
			cStream << " enclose with " << m_pStartTag->toSQLStatement(bForCascade_);
		
			if (m_pEndTag != 0)
				{
					cStream << " and " << m_pEndTag->toSQLStatement(bForCascade_);
				}
		}
	
	if (m_pEscape != 0)
		{
			cStream << " escape " << m_pEscape->toSQLStatement(bForCascade_);
		}
	
	if (m_pEllipsis != 0)
		{
			cStream << " ellipsis " << m_pEllipsis->toSQLStatement(bForCascade_);
		}

	cStream << ")";
	
	return cStream.getString();
}

//	FUNCTION public
//	Statement::Function::Kwic::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Function::Kwic::copy() const
{
	return new Kwic(*this);
}

// FUNCTION public
//	Statement::Function::Kwic::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::Kwic::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::Kwic;
}

// FUNCTION public
//	Statement::Function::Kwic::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::Kwic::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, m_pSize);
	Utility::Serialize::Object(cArchive_, m_pStartTag);
	Utility::Serialize::Object(cArchive_, m_pEndTag);
	Utility::Serialize::Object(cArchive_, m_pEscape);
	Utility::Serialize::Object(cArchive_, m_pEllipsis);
}

//
//	FUNCTION public
//		Statement::Function::ExpandSynonym::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
Function::ExpandSynonym::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "expand_synonym(" << getSource()->toSQLStatement(bForCascade_)
			<< " using " << getParameter()->toSQLStatement(bForCascade_)
			<< ")";

	return cStream.getString();
}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Function::ExpandSynonym::copy() const
{
	return new ExpandSynonym(*this);
}

// FUNCTION public
//	Statement::Function::ExpandSynonym::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::ExpandSynonym::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::ExpandSynonym;
}

// FUNCTION public
//	Statement::Function::ExpandSynonym::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::ExpandSynonym::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::Object(cArchive_, _parameter);
}

///////////////////////
// Generator
///////////////////////

namespace
{
	const char* const _pszOptionNameTable[] =
		{
			"start with",	// Start
			"increment by",	// Increment,
			"maxvalue",		// MaxValue,
			"minvalue",		// MinValue,
			"cycle",		// Cycle,
			"get max",		// GetMax,
			0
		};
}

// FUNCTION public
//	Statement::Generator::Definition::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	bool bForCascade_ = false
//	
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Generator::Definition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	if (bForCascade_ == false ) {
		stream << "generated"
			   << (m_eWhen == Always ? " always" : " by default");
		if (m_bIdentity) {
			stream << " as identity";
		}
		ValueExpressionList* pList = getOptions();
		int n = pList->getCount();
		if (n) {
			stream << " (";
			for (int i = 0; i < n; ++i) {
				stream << " " << pList->getAt(i)->toSQLStatement(bForCascade_);
			}
			stream << " )";
		}
	}
	return stream.getString();
}

// FUNCTION public
//	Statement::Generator::Definition::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Generator::Definition::
copy() const
{
	return new Definition(*this);
}

// FUNCTION public
//	Statement::Generator::Definition::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Generator::Definition::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::GeneratorDefinition;
}

// FUNCTION public
//	Statement::Generator::Definition::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::Definition::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_eWhen);
	cArchive_(m_bIdentity);
}

// FUNCTION public
//	Statement::Generator::Option::Option -- Constructor
//
// NOTES
//
// ARGUMENTS
//	Type eType_
//	Literal* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Generator::Option::
Option(Type eType_, Literal* pValue_)
	: Super(op_GeneratorOption, 0),
	  m_eType(eType_)
{
	setPrimary(pValue_);
}

// FUNCTION public
//	Statement::Generator::Option::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	bool bForCascade_ = false
//	
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Generator::Option::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << _pszOptionNameTable[static_cast<int>(m_eType)];
	Literal* pValue = getValue();
	if (pValue) {
		stream << " " << pValue->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

// FUNCTION public
//	Statement::Generator::Option::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Generator::Option::
copy() const
{
	return new Option(*this);
}

// FUNCTION public
//	Statement::Generator::Option::getValue -- Get option value
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Literal*
//
// EXCEPTIONS

Literal*
Generator::Option::
getValue() const
{
	if (getPrimary()) {
		; _SYDNEY_ASSERT(getPrimary()->getType() == ObjectType::Literal);
		return _SYDNEY_DYNAMIC_CAST(Literal*, getPrimary());
	} else {
		return 0;
	}
}

// FUNCTION public
//	Statement::Generator::Option::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Generator::Option::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::GeneratorOption;
}

// FUNCTION public
//	Statement::Generator::Option::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::Option::
serialize(ModArchive& cArchive_)
{
	ValueExpression::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_eType);
}

///////////////////////
// Expression
///////////////////////

///////////////////////////
// Expression::SimpleCase::
///////////////////////////

// FUNCTION public
//	Statement::Expression::SimpleCase::SimpleCase -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pOperand_
//	ValueExpressionList* pWhenList_
//	ValueExpression* pElse_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Expression::SimpleCase::
SimpleCase(ValueExpression* pOperand_,
		   ValueExpressionList* pWhenList_,
		   ValueExpression* pElse_)
	: ValueExpression(op_Case, 0)
{
	setOperandList(pWhenList_);
	setRight(pOperand_);
	setExpressionType(mergeExpressionType(pWhenList_->getExpressionType(),
										  pOperand_->getExpressionType()));
	if (pElse_) {
		setOption(pElse_);
		setExpressionType(mergeExpressionType(getExpressionType(),
											  pElse_->getExpressionType()));
	}
}

// FUNCTION public
//	Statement::Expression::SimpleCase::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

// SQL文で値を得る
//virtual
ModUnicodeString
Expression::SimpleCase::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "case " << getOperand()->toSQLStatement(bForCascade_);
	ValueExpressionList* pWhenList = getWhenList();
	int n = pWhenList->getCount();
	for (int i = 0; i < n; ++i) {
		cStream << " " << pWhenList->getAt(i)->toSQLStatement(bForCascade_);
	}
	if (ValueExpression* pElse = getElse()) {
		cStream << " else " << pElse->toSQLStatement(bForCascade_);
	}
	cStream << " end";
	return cStream.getString();
}

// FUNCTION public
//	Statement::Expression::SimpleCase::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Expression::SimpleCase::
copy() const
{
	return new SimpleCase(*this);
}

// FUNCTION public
//	Statement::Function::SimpleCase::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Expression::SimpleCase::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::SimpleCase;
}

///////////////////////////
// Expression::SearchedCase::
///////////////////////////

// FUNCTION public
//	Statement::Expression::SearchedCase::SearchedCase -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpressionList* pWhenList_
//	ValueExpression* pElse_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Expression::SearchedCase::
SearchedCase(ValueExpressionList* pWhenList_,
			 ValueExpression* pElse_)
	: ValueExpression(op_Case, 0)
{
	setOperandList(pWhenList_);
	setExpressionType(pWhenList_->getExpressionType());
	if (pElse_) {
		setOption(pElse_);
		setExpressionType(mergeExpressionType(getExpressionType(),
											  pElse_->getExpressionType()));
	}
}

// FUNCTION public
//	Statement::Expression::SearchedCase::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

// SQL文で値を得る
//virtual
ModUnicodeString
Expression::SearchedCase::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "case ";
	ValueExpressionList* pWhenList = getWhenList();
	int n = pWhenList->getCount();
	for (int i = 0; i < n; ++i) {
		cStream << " " << pWhenList->getAt(i)->toSQLStatement(bForCascade_);
	}
	if (ValueExpression* pElse = getElse()) {
		cStream << " else " << pElse->toSQLStatement(bForCascade_);
	}
	cStream << " end";
	return cStream.getString();
}

// FUNCTION public
//	Statement::Expression::SearchedCase::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Expression::SearchedCase::
copy() const
{
	return new SearchedCase(*this);
}

// FUNCTION public
//	Statement::Expression::SearchedCase::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Expression::SearchedCase::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::SearchedCase;
}

///////////////////////////
// Expression::SimpleWhen::
///////////////////////////

// FUNCTION public
//	Statement::Expression::SimpleWhen::SimpleWhen -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpressionList* pWhenOperandList_
//	ValueExpression* pResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Expression::SimpleWhen::
SimpleWhen(ValueExpressionList* pWhenOperandList_,
		   ValueExpression* pResult_)
	: ValueExpression(op_When, 0)
{
	setOperandList(pWhenOperandList_);
	setRight(pResult_);
	setExpressionType(mergeExpressionType(pWhenOperandList_->getExpressionType(),
										  pResult_->getExpressionType()));
}

// FUNCTION public
//	Statement::Expression::SimpleWhen::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Expression::SimpleWhen::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "when " << getWhenOperandList()->toSQLStatement(bForCascade_)
			<< " then " << getResult()->toSQLStatement(bForCascade_);
	return cStream.getString();
}

// FUNCTION public
//	Statement::Expression::SimpleWhen::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Expression::SimpleWhen::
copy() const
{
	return new SimpleWhen(*this);
}

// FUNCTION public
//	Statement::Expression::SimpleWhen::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Expression::SimpleWhen::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::SimpleWhen;
}

///////////////////////////
// Expression::SearchedWhen::
///////////////////////////

// FUNCTION public
//	Statement::Expression::SearchedWhen::SearchedWhen -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pWhenCondition_
//	ValueExpression* pResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Expression::SearchedWhen::
SearchedWhen(ValueExpression* pWhenCondition_,
			 ValueExpression* pResult_)
	: ValueExpression(op_When, pWhenCondition_, pResult_)
{}

// FUNCTION public
//	Statement::Expression::SearchedWhen::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Expression::SearchedWhen::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "when " << getWhenCondition()->toSQLStatement(bForCascade_)
			<< " then " << getResult()->toSQLStatement(bForCascade_);
	return cStream.getString();
}

// FUNCTION public
//	Statement::Expression::SearchedWhen::copy -- create clone
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

//virtual
Object*
Expression::SearchedWhen::
copy() const
{
	return new SearchedWhen(*this);
}

// FUNCTION public
//	Statement::Expression::SearchedWhen::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Expression::SearchedWhen::
getClassID() const
{
	return Statement::Externalizable::ClassID::ValueExpression + _Externalizable::SearchedWhen;
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::ValueExpression_Literal _analyzer_literal;
	Analysis::ValueExpression_ItemReference _analyzer_itemref;
	Analysis::ValueExpression_PlaceHolder _analyzer_placeholder;

	Analysis::ValueExpression_Arithmetic_Dyadic _analyzer_arithmetic_dyadic;
	Analysis::ValueExpression_Arithmetic_Monadic _analyzer_arithmetic_monadic;
	Analysis::ValueExpression_Concatenate _analyzer_concatenate;
	Analysis::ValueExpression_CharLength _analyzer_func_charlength;
	Analysis::ValueExpression_SubString _analyzer_func_substring;
	Analysis::ValueExpression_Overlay _analyzer_func_overlay;
	Analysis::ValueExpression_WordCount _analyzer_func_wordcount;
	Analysis::ValueExpression_FullTextLength _analyzer_func_fulltext_length;
	Analysis::ValueExpression_Normalize _analyzer_func_normalize;
	Analysis::ValueExpression_Kwic _analyzer_func_kwic;
	Analysis::ValueExpression_ExpandSynonym _analyzer_func_expand_synonym;

	Analysis::ValueExpression_And _analyzer_and;
	Analysis::ValueExpression_Or _analyzer_or;
	Analysis::ValueExpression_Not _analyzer_not;

	Analysis::ValueExpression_Comparison_Row _analyzer_comparison_row;
	Analysis::ValueExpression_Comparison_Scalar _analyzer_comparison_scalar;
	Analysis::ValueExpression_IsNull _analyzer_isnull;
	Analysis::ValueExpression_Like _analyzer_like;
	Analysis::ValueExpression_Exists _analyzer_exists;
	Analysis::ValueExpression_Between _analyzer_between;
	Analysis::ValueExpression_In _analyzer_in;
	Analysis::ValueExpression_Contains _analyzer_contains;

	Analysis::ValueExpression_TableConstructor _analyzer_tblconst;
	Analysis::ValueExpression_RowConstructor _analyzer_rowconst;
	Analysis::ValueExpression_Null _analyzer_null;
	Analysis::ValueExpression_FullText _analyzer_fulltext;

	Analysis::ValueExpression_ArrayReference _analyzer_arrayref;
	Analysis::ValueExpression_ArrayConstructor _analyzer_arrayconst;

	Analysis::ValueExpression_Function_Set _analyzer_func_set;
#if 0
	Analysis::ValueExpression_OctetLength _analyzer_func_octetlength;
#endif
	Analysis::ValueExpression_Function_Niladic _analyzer_func_niladic;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ValueExpression::
getAnalyzer() const
{
	switch (getOperator()) {
	case op_Literal:
	case op_PathName:
		{
			return &_analyzer_literal;
		}
	case op_Itemref:
		{
			return &_analyzer_itemref;
		}
	case op_Placeholder:
		{
			return &_analyzer_placeholder;
		}
	case op_Add:
	case op_Sub:
	case op_Mul:
	case op_Div:
	case op_Mod:
		{
			return &_analyzer_arithmetic_dyadic;
		}
	case op_Neg:
	case op_Abs:
		{
			return &_analyzer_arithmetic_monadic;
		}
	case op_String_concat:
		{
			return &_analyzer_concatenate;
		}
	case op_And:
		{
			return &_analyzer_and;
		}
	case op_Or:
		{
			return &_analyzer_or;
		}
	case op_Not:
		{
			return &_analyzer_not;
		}
	case op_Eq:
	case op_Ne:
	case op_Le:
	case op_Lt:
	case op_Ge:
	case op_Gt:
		{
			if (getLeft()->getValueType() == ValueExpression::Row)
				return &_analyzer_comparison_row;
			else
				return &_analyzer_comparison_scalar;
		}
	case op_IsNull:
	case op_IsNotNull:
		{
			return &_analyzer_isnull;
		}
	case op_Like:
	case op_NotLike:
	case op_Similar:
	case op_NotSimilar:
		{
			return &_analyzer_like;
		}
	case op_Exists:
		{
			return &_analyzer_exists;
		}
	case op_Tblconst:
		{
			return &_analyzer_tblconst;
		}
	case op_Rowconst:
		{
			return &_analyzer_rowconst;
		}
	case op_Nullobj:
	case op_Defaultobj:
		{
			return &_analyzer_null;
		}
	case op_Arrayref:
		{
			return &_analyzer_arrayref;
		}
	case op_Arrayconst:
		{
			return &_analyzer_arrayconst;
		}

	case op_Func:
		{
			return getAnalyzerForFunc();
		}
	case op_Between:
	case op_NotBetween:
		{
			return &_analyzer_between;
		}
	case op_In:
	case op_NotIn:
		{
			return &_analyzer_in;
		}
	case op_Contains:
		{
			return &_analyzer_contains;
		}
#ifdef OBSOLETE
		// 使用しなくなった
	case op_ColumnName:
		{
			return &_analyzer_arraycolumnname;
		}
#endif
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// 関数のAnalyzerを得る
const Analysis::Analyzer*
ValueExpression::
getAnalyzerForFunc() const
{
	switch (getFunction()) {
	case func_Count:
	case func_Avg:
	case func_Max:
	case func_Min:
	case func_Sum:
		{
			return &_analyzer_func_set;
		}
	case func_Char_Length:
		{
			return &_analyzer_func_charlength;
		}
	case func_SubString:
		{
			return &_analyzer_func_substring;
		}
	case func_Overlay:
		{
			return &_analyzer_func_overlay;
		}
	case func_Normalize:
		{
			return &_analyzer_func_normalize;
		}
#if 0
	case func_Octet_Length:
		{
			return &_analyzer_func_octetlength;
		}
#endif
#if 0
	case func_User:
	case func_Session_User:
	case func_Current_User:
	case func_Current_Path:
#endif
	case func_Current_Date:
	case func_Current_Timestamp:
		{
			return &_analyzer_func_niladic;
		}
#ifdef OBSOLETE
	case func_Value: // domainをサポートしない限り意味がない
		{
			return &_analyzer_func_value;
		}
#endif
	case func_Word_Count:
		{
			return &_analyzer_func_wordcount;
		}
	case func_FullText_Length:
		{
			return &_analyzer_func_fulltext_length;
		}
	case func_Tf:
	case func_ClusterId:
	case func_ClusterWord:
	case func_Score:
	case func_Sectionized:
	case func_Word:
	case func_WordDf:
	case func_WordScale:
	case func_Existence:
		{
			return &_analyzer_fulltext;
		}
	case func_Kwic:
		{
			return &_analyzer_func_kwic;
		}
	case func_Expand_Synonym:
		{
			return &_analyzer_func_expand_synonym;
		}
	case func_Unknown:
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}
#endif

// FUNCTION public
//	Statement::ValueExpression::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
ValueExpression::
getAnalyzer2() const
{
	return Analysis::Value::ValueExpression::create(this);
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
