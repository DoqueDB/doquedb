// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ValueExpression -- ValueExpression
// 
// Copyright (c) 1999, 2000, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_VALUEEXPRESSION_H
#define __SYDNEY_STATEMENT_VALUEEXPRESSION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Hint;
class Identifier;
class Literal;
class Token;
class ValueExpressionList;
class VariableName;

//	CLASS
//		ValueExpression -- ValueExpression
//
//	NOTES

class SYD_STATEMENT_FUNCTION ValueExpression : public Statement::Object
{
public:
	// コンストラクタ (1)
	ValueExpression();
	// コンストラクタ (2)
	ValueExpression(int iOperator_, ValueExpression* pLeft_);
	// コンストラクタ (3)
	ValueExpression(int iOperator_, ValueExpression* pLeft_, ValueExpression* pRight_);
	// コンストラクタ (4)
	ValueExpression(int iOperator_, ValueExpression* pLeft_, ValueExpression* pRight_, ValueExpression* pOption_, int iValueType_);
	// コンストラクタ (5)
	ValueExpression(int iOperator_, ValueExpression* pLeft_, ValueExpression* pRight_, int iValueType_);
	// コンストラクタ (6)
	ValueExpression(int iOperator_, Statement::Object* pPrimary_, int iValueType_, int iExpressionType_);
	// コンストラクタ (7)
	ValueExpression(Literal* pLit_);
	// コンストラクタ (8)
	ValueExpression(int iOperator_, int iFunction_, int iValueType_, int iQuantifier_, ValueExpression* pLeft_,
					int iExpressionType_,
					Hint* pHint_ = 0);
	// コンストラクタ (9)
	ValueExpression(int iOperator_, int iFunction_, int iValueType_, ValueExpressionList* pOperand_,
					int iExpressionType_,
					Hint* pHint_ = 0);
	// コンストラクタ (10)
	ValueExpression(VariableName* varName);
	// コンストラクタ (11)
	ValueExpression(Identifier* pFunctionName_, ValueExpressionList* pOperand_);

	// 名前から作るコンストラクター
	static ValueExpression* create(Identifier* pFunctionName_);
	static ValueExpression* create(Identifier* pFunctionName_, Identifier* pMemberName_);
	static ValueExpression* create(Identifier* pFunctionName_, ValueExpressionList* pArgs_, Hint* pHint_);
	static ValueExpression* create(Identifier* pFunctionName_, Identifier* pMemberName_, ValueExpressionList* pArgs_, Hint* pHint_);

	// 関数を作るコンストラクター
	static ValueExpression* create(int iFunction_, ValueExpressionList* pArgs_, Hint* pHint_ = 0);

	enum FunctionType {
		func_Unknown = 0,
		func_User,
		func_Session_User,
		func_Current_User,
		func_Current_Path,
		func_Value,
		func_Count,
		func_Avg,
		func_Max,
		func_Min,
		func_Sum,
		func_Char_Length,
		func_Octet_Length,
		func_SubString,
		func_Overlay,
		func_Word_Count,
		func_Tf,
		func_Current_Date,
		func_Current_Timestamp,
		func_FullText_Length,
		func_Normalize,
		func_Cluster,
		func_ClusterId,
		func_ClusterWord,
		func_Kwic,
		func_Expand_Synonym,
		func_Coalesce,
		func_Cardinality,
		func_Score,
		func_Sectionized,
		func_Word,
		func_WordDf,
		func_WordScale,
		func_Char_Join,
		func_Grouping_Element,		
		func_Existence,
		func_Invoke,		
		func_GetMax,
		func_Neighbor,
		func_NeighborId,
		func_NeighborDistance,
		func_ValueNum,
	};
	enum Quantifier {
		quant_None = 0,
		quant_All,
		quant_Distinct
	};

	// アクセサ
	// Operator を得る
	int getOperator() const;
	// Operator を設定する
	void setOperator(int iOperator_);
	//
	//	Enum global
	//	Operator -- Operatorの値
	//
	//	NOTES
	//	Operatorの値
	//
	enum Operator {
		op_List = 1,
		op_Literal,
		op_Nullobj,
		op_Itemref,
		op_Rowref,
		op_Arrayref,
		op_Rowconst,
		op_Tblconst,
		op_Arrayconst,
		op_Placeholder,
		op_ColumnName,
		op_Add,
		op_Sub,
		op_Mul,
		op_Div,
		op_Neg,
		op_Abs,
		op_And,
		op_Or,
		op_Not,
		op_String_concat,
		op_Eq,
		op_Ne,
		op_Le,
		op_Lt,
		op_Ge,
		op_Gt,
		op_Between,
		op_Like,
		op_Contains,
		op_IsNull,
		op_Exists,
		op_In,
		op_IsNotNull,
		op_Func,
		op_WordHead,
		op_WordTail,
		op_SimpleWord,
		op_ExactWord,
		op_ApproximateWord,
		op_Similar,
		op_GeneratorDefinition,
		op_GeneratorOption,
		op_PathName,
		op_Defaultobj,
		op_NotBetween,
		op_NotLike,
		op_NotIn,
		op_NotSimilar,
		op_RowSubquery,
		op_Case,
		op_When,
		op_Variable,
		op_Mod,
		op_IsSubstringOf
	};
	// ValueType を得る
	int getValueType() const;
	// ValueType を設定する
	void setValueType(int iValueType_);
	//
	//	Enum global
	//	ValueType -- ValueTypeの値
	//
	//	NOTES
	//	ValueTypeの値
	//
	enum ValueType {
		Unknown = 0,
		Numeric,
		CharString,
		BinaryString,
		Datetime,
		Interval,
		Enumerate,
		Boolean,
		Null,
		Row,
		Table,
		Array,
		Default
	};

	// ExpressionType を得る
	int getExpressionType() const;
	// ExpressionType を設定する
	void setExpressionType(int iType_);

	// ExpressionType を合成する
	static int mergeExpressionType(int iType1_, int iType2_);
	static int mergeExpressionType(ValueExpression* pExp1_,
								   ValueExpression* pExp2_,
								   ValueExpression* pExp3_ = 0,
								   ValueExpression* pExp4_ = 0,
								   ValueExpression* pExp5_ = 0,
								   ValueExpression* pExp6_ = 0);

	//	ENUM public
	//	ValueExpression::ExpressionType -- type of value expression
	//
	//	NOTES
	enum ExpressionType {
		type_Unknown = 0,				// Unknown type
		type_Constant,					// Constant value
		type_Value,						// Variable value
		type_SingleColumn,				// Value constructed by one column
		type_MultipleColumn,			// Value constructed by more than one columns
		type_Aggregation,				// Value includes aggregation
		type_ValueNum
	};

	// Left を得る
	ValueExpression* getLeft() const;
	// Left を設定する
	void setLeft(ValueExpression* pLeft_);

	// Right を得る
	ValueExpression* getRight() const;
	// Right を設定する
	void setRight(ValueExpression* pRight_);

	// OperandList を得る
	ValueExpressionList* getOperandList() const;
	// OperandList を設定する
	void setOperandList(ValueExpressionList* pList_);

	// Option を得る
	ValueExpression* getOption() const;
	// Option を設定する
	void setOption(ValueExpression* pOption_);

	// Primary を得る
	Statement::Object* getPrimary() const;
	// Primary を設定する
	void setPrimary(Statement::Object* pPrimary_);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// Function を得る
	int getFunction() const;
	// Function を設定する
	void setFunction(int iFunction_);

	// 関数名を得る
	static const char* getFunctionName(int iFunction_);

	// Quantifier を得る
	int getQuantifier() const;
	// Quantifier を設定する
	void setQuantifier(int iQuantifier_);

	// null の ValuExpression を作る
	static ValueExpression* nullExpression();
	// default の ValuExpression を作る
	static ValueExpression* defaultExpression();

	//数値かどうか検査する
	bool isNumberLiteral() const;
	//Lieral の符号を反転する
	void setSignNegative(Token* pSign_);

	// PathNameか
	bool isPathName() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#ifndef SYD_COVERAGE
	// 文字列化
	ModUnicodeString toString() const;
	// Lisp形式で出力する
	void toString(ModUnicodeOstrStream& cStream_, int iIndent_ = 0) const;
#endif

	// And, Or を展開する
	void expandCondition(int iDepth_ = 0);

	// set a value expression as included in parenthesis
	void setSeparated();
	// get whether a value expression as included in parenthesis
	bool isSeparated() const;

	//自身をコピーする
	Object* copy() const;
	static Object* getInstance(int iClassID_);

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	ValueExpression& operator=(const ValueExpression& cOther_);

	// 下位に同じオペレータがあった場合に展開する
	void mergeOperandList();
#ifndef SYD_COVERAGE
	// 下位objectを出力する
	void printChild(ModUnicodeOstrStream& cStream_, int iIndent_, int iIdx_) const;
#endif

	// 関数のAnalyzerを得る
	const Analysis::Analyzer* getAnalyzerForFunc() const;
};

namespace Function
{

//	CLASS
//	Statement::Function::SubString -- 関数 SUBSTRING を表す
//
//	NOTES

class SubString
	: public	ValueExpression
{
public:
	//constructor
	SubString()
		: ValueExpression()
	{
		setOperator(op_Func);
		setFunction(func_SubString);
	}
	// コンストラクター
	SubString(ValueExpression* source,
			  ValueExpression* start, ValueExpression* length = 0);
	// コピーコンストラクター
	SubString(const SubString& v);
	// デストラクタ
	virtual ~SubString();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 取得先を得る
	ValueExpression*
	getSource() const;
	// 処理の開始位置を得る
	ValueExpression*
	getStartPosition() const;
	// 取り出す長さを得る
	ValueExpression*
	getStringLength() const;

	// 自分自身をコピーする
	SYD_STATEMENT_FUNCTION
	virtual Object*
	copy() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 処理の開始位置
	ValueExpression*		_start;
	// 取り出す長さ
	ValueExpression*		_length;
};

//	CLASS
//	Statement::Function::Overlay -- 関数 OVERLAY を表す
//
//	NOTES

class Overlay
	: public	ValueExpression
{
public:
	//constructor
	Overlay()
		: ValueExpression()
	{
		setOperator(op_Func);
		setFunction(func_Overlay);
	}
	// コンストラクター
	Overlay(ValueExpression* dest, ValueExpression* placing,
			ValueExpression* start, ValueExpression* length = 0);
	// コピーコンストラクター
	Overlay(const Overlay& v);
	// デストラクタ
	virtual ~Overlay();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 上書き先を得る
	ValueExpression*
	getDestination() const;
	// 上書き内容を得る
	ValueExpression*
	getPlacing() const;
	// 処理の開始位置を得る
	ValueExpression*
	getStartPosition() const;
	// 上書きされる部分の長さを得る
	ValueExpression*
	getStringLength() const;

	// 自分自身をコピーする
	SYD_STATEMENT_FUNCTION
	virtual Object*
	copy() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 上書き内容
	ValueExpression*		_placing;
	// 処理の開始位置
	ValueExpression*		_start;
	// 上書きされる部分の長さ
	ValueExpression*		_length;
};

//	CLASS
//	Statement::Function::Normalize -- 関数 NORMALIZE を表す
//
//	NOTES

class Normalize
	: public	ValueExpression
{
public:
	//constructor
	Normalize()
		: ValueExpression()
	{
		setOperator(op_Func);
		setFunction(func_Normalize);
	}
	// コンストラクター
	Normalize(ValueExpression* source, ValueExpression* parameter);
	// コピーコンストラクター
	Normalize(const Normalize& v);
	// デストラクタ
	virtual ~Normalize();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 入力を得る
	ValueExpression*
	getSource() const;
	// パラメーターを得る
	ValueExpression*
	getParameter() const;

	// 自分自身をコピーする
	SYD_STATEMENT_FUNCTION
	virtual Object*
	copy() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// パラメーター
	ValueExpression*		_parameter;
};

//
//	CLASS
//	Statement::Function::Kwic -- 関数 KWIC を表す
//
//	NOTES
//
class Kwic
	: public	ValueExpression
{
public:
	//constructor
	Kwic()
		: ValueExpression()
	{
		setOperator(op_Func);
		setFunction(func_Kwic);
	}
	// コンストラクター
	Kwic(ValueExpression* pSource_,
		 ValueExpression* pSize_,
		 ValueExpression* pStartTag_,
		 ValueExpression* pEndTag_,
		 ValueExpression* pEscape_,
		 ValueExpression* pEllipsis_);
	// コピーコンストラクター
	Kwic(const Kwic& v);
	// デストラクタ
	virtual ~Kwic();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 取得先を得る
	ValueExpression*
	getSource() const;
	// サイズを得る
	ValueExpression*
	getSize() const;
	// 開始タグを得る
	ValueExpression*
	getStartTag() const;
	// 終了タグを得る
	ValueExpression*
	getEndTag() const;
	// エスケープ指定を得る
	ValueExpression*
	getEscape() const;
	// 省略記号を得る
	ValueExpression*
	getEllipsis() const;
	
	// 自分自身をコピーする
	SYD_STATEMENT_FUNCTION
	virtual Object*
	copy() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// サイズ
	ValueExpression* m_pSize;
	// 開始タグ
	ValueExpression* m_pStartTag;
	// 終了タグ
	ValueExpression* m_pEndTag;
	// エスケープ指定
	ValueExpression* m_pEscape;
	// 省略記号
	ValueExpression* m_pEllipsis;
};

//	CLASS
//	Statement::Function::ExpandSynonym -- 関数 EXPAND SYNONYM を表す
//
//	NOTES

class ExpandSynonym
	: public	ValueExpression
{
public:
	//constructor
	ExpandSynonym()
		: ValueExpression()
	{
		setOperator(op_Func);
		setFunction(func_Expand_Synonym);
	}
	// コンストラクター
	ExpandSynonym(ValueExpression* source, ValueExpression* parameter);
	// コピーコンストラクター
	ExpandSynonym(const ExpandSynonym& v);
	// デストラクタ
	virtual ~ExpandSynonym();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 入力を得る
	ValueExpression*
	getSource() const;
	// パラメーターを得る
	ValueExpression*
	getParameter() const;

	// 自分自身をコピーする
	SYD_STATEMENT_FUNCTION
	virtual Object*
	copy() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// パラメーター
	ValueExpression*		_parameter;
};

//	FUNCTION public
//	Statement::Function::SubString::SubString -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	source
//			取得先を得るための式
//		Statement::ValueExpression*	start
//			処理の開始位置を得るための式
//		Statement::ValueExpression*	length
//			0 以外の値
//				取り出す長さを得るための式
//			0 または指定されないとき
//				取得先の末尾まで取り出すことを表す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
SubString::SubString(ValueExpression* source,
					 ValueExpression* start, ValueExpression* length)
	: ValueExpression(op_Func, func_SubString, Unknown, quant_None, source,
					  ValueExpression::mergeExpressionType(source, start, length)),
	  _start(start),
	  _length(length)
{}

//	FUNCTION public
//	Statement::Function::SubString::SubString -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Function::SubString&	v
//			コピー元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
SubString::SubString(const SubString& v)
	: ValueExpression(v),
	  _start((v.getStartPosition()) ?
			 static_cast<ValueExpression*>(v.getStartPosition()->copy()) : 0),
	  _length((v.getStringLength()) ?
			  static_cast<ValueExpression*>(v.getStringLength()->copy()) : 0)
{}

//	FUNCTION public
//	Statement::Function::SubString::~SubString -- デストラクター
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

inline
SubString::~SubString()
{
	delete _start, _start = 0;
	delete _length, _length = 0;
}

//	FUNCTION public
//	Statement::Function::SubString::getSource -- 取得先を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得先を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
SubString::getSource() const
{
	return getLeft();
}

//	FUNCTION public
//	Statement::Function::SubString::getStartPosition -- 処理の開始位置を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		処理の開始位置を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
SubString::getStartPosition() const
{
	return _start;
}

//	FUNCTION public
//	Statement::Function::SubString::getStringLength -- 取り出す長さを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取り出す長さを得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
SubString::getStringLength() const
{
	return _length;
}

//	FUNCTION public
//	Statement::Function::Overlay::Overlay -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	dest
//			上書き先を得るための式
//		Statement::ValueExpression*	placing
//			上書き内容を得るための式
//		Statement::ValueExpression*	start
//			処理の開始位置を得るための式
//		Statement::ValueExpression*	length
//			0 以外の値
//				上書きされる部分の長さを得るための式
//			0 または指定されないとき
//				上書き内容と同じ長さの部分を上書きすることを表す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Overlay::Overlay(ValueExpression* dest, ValueExpression* placing,
				 ValueExpression* start, ValueExpression* length)
	: ValueExpression(op_Func, func_Overlay, Unknown, quant_None, dest,
					  ValueExpression::mergeExpressionType(dest, placing, start, length)),
	  _placing(placing),
	  _start(start),
	  _length(length)
{}

//	FUNCTION public
//	Statement::Function::Overlay::Overlay -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Function::Overlay&	v
//			コピー元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Overlay::Overlay(const Overlay& v)
	: ValueExpression(v),
	  _placing((v.getPlacing()) ?
			   static_cast<ValueExpression*>(v.getPlacing()->copy()) : 0),
	  _start((v.getStartPosition()) ?
			 static_cast<ValueExpression*>(v.getStartPosition()->copy()) : 0),
	  _length((v.getStringLength()) ?
			  static_cast<ValueExpression*>(v.getStringLength()->copy()) : 0)
{}

//	FUNCTION public
//	Statement::Function::Overlay::~Overlay -- デストラクター
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

inline
Overlay::~Overlay()
{
	delete _placing, _placing = 0;
	delete _start, _start = 0;
	delete _length, _length = 0;
}

//	FUNCTION public
//	Statement::Function::Overlay::getDestination -- 上書き先を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		上書き先を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Overlay::getDestination() const
{
	return getLeft();
}

//	FUNCTION public
//	Statement::Function::Overlay::getPlacing -- 上書き内容を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		上書き内容を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Overlay::getPlacing() const
{
	return _placing;
}

//	FUNCTION public
//	Statement::Function::Overlay::getStartPosition -- 処理の開始位置を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		処理の開始位置を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Overlay::getStartPosition() const
{
	return _start;
}

//	FUNCTION public
//	Statement::Function::Overlay::getStringLength --
//		上書きされる部分の長さを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		上書きされる部分の長さを得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Overlay::getStringLength() const
{
	return _length;
}

//	FUNCTION public
//	Statement::Function::Normalize::Normalize -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	source
//			入力を得るための式
//		Statement::ValueExpression*	param
//			パラメーターを得るための式
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Normalize::Normalize(ValueExpression* source, ValueExpression* parameter)
	: ValueExpression(op_Func, func_Normalize, Unknown, quant_None, source,
					  ValueExpression::mergeExpressionType(source, parameter)),
	  _parameter(parameter)
{}

//	FUNCTION public
//	Statement::Function::Normalize::Normalize -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Function::Normalize&	v
//			コピー元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Normalize::Normalize(const Normalize& v)
	: ValueExpression(v),
	  _parameter((v.getParameter()) ?
			   static_cast<ValueExpression*>(v.getParameter()->copy()) : 0)
{}

//	FUNCTION public
//	Statement::Function::Normalize::~Normalize -- デストラクター
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

inline
Normalize::~Normalize()
{
	delete _parameter, _parameter = 0;
}

//	FUNCTION public
//	Statement::Function::Normalize::getSource -- 入力を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		入力を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Normalize::getSource() const
{
	return getLeft();
}

//	FUNCTION public
//	Statement::Function::Normalize::getParameter -- パラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		パラメーターを得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
Normalize::getParameter() const
{
	return _parameter;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::Kwic -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
Kwic::Kwic(ValueExpression* pSource_,
		   ValueExpression* pSize_,
		   ValueExpression* pStartTag_,
		   ValueExpression* pEndTag_,
		   ValueExpression* pEscape_,
		   ValueExpression* pEllipsis_)
	: ValueExpression(
		op_Func, func_Kwic, Unknown, quant_None, pSource_,
		ValueExpression::mergeExpressionType(
			pSource_, pSize_, pStartTag_, pEndTag_, pEscape_, pEllipsis_)),
	  m_pSize(pSize_),
	  m_pStartTag(pStartTag_),
	  m_pEndTag(pEndTag_),
	  m_pEscape(pEscape_),
	  m_pEllipsis(pEllipsis_)
{}

//
//	FUNCTION public
//	Statement::Function::Kwic::Kwic -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
Kwic::Kwic(const Kwic& v)
	: ValueExpression(v),
	  m_pSize((v.getSize()) ?
			  static_cast<ValueExpression*>(v.getSize()->copy()) : 0),
	  m_pStartTag((v.getStartTag()) ?
			  static_cast<ValueExpression*>(v.getStartTag()->copy()) : 0),
	  m_pEndTag((v.getEndTag()) ?
			  static_cast<ValueExpression*>(v.getEndTag()->copy()) : 0),
	  m_pEscape((v.getEscape()) ?
			  static_cast<ValueExpression*>(v.getEscape()->copy()) : 0),
	  m_pEllipsis((v.getEllipsis()) ?
			  static_cast<ValueExpression*>(v.getEllipsis()->copy()) : 0)
{}

//
//	FUNCTION public
//	Statement::Function::Kwic::~Kwic -- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
Kwic::~Kwic()
{
	delete m_pSize, m_pSize = 0;
	delete m_pStartTag, m_pStartTag = 0;
	delete m_pEndTag, m_pEndTag = 0;
	delete m_pEscape, m_pEscape = 0;
	delete m_pEllipsis, m_pEllipsis = 0;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getSource -- 入力を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getSource() const
{
	return getLeft();
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getSize() const
{
	return m_pSize;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getStartTag -- 開始タグを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getStartTag() const
{
	return m_pStartTag;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getEndTag -- 終了タグを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getEndTag() const
{
	return m_pEndTag;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getEscape -- エスケープ指定を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getEscape() const
{
	return m_pEscape;
}

//
//	FUNCTION public
//	Statement::Function::Kwic::getEllipsis -- 省略記号を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ValueExpression*
Kwic::getEllipsis() const
{
	return m_pEllipsis;
}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::ExpandSynonym -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	source
//			入力を得るための式
//		Statement::ValueExpression*	param
//			パラメーターを得るための式
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ExpandSynonym::ExpandSynonym(ValueExpression* source,
							 ValueExpression* parameter)
	: ValueExpression(op_Func, func_Expand_Synonym, Unknown, quant_None, source,
					  ValueExpression::mergeExpressionType(source, parameter)),
	  _parameter(parameter)
{}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::ExpandSynonym -- コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Function::ExpandSynonym&	v
//			コピー元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ExpandSynonym::ExpandSynonym(const ExpandSynonym& v)
	: ValueExpression(v),
	  _parameter((v.getParameter()) ?
			   static_cast<ValueExpression*>(v.getParameter()->copy()) : 0)
{}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::~ExpandSynonym -- デストラクター
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

inline
ExpandSynonym::~ExpandSynonym()
{
	delete _parameter, _parameter = 0;
}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::getSource -- 入力を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		入力を得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
ExpandSynonym::getSource() const
{
	return getLeft();
}

//	FUNCTION public
//	Statement::Function::ExpandSynonym::getParameter -- パラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		パラメーターを得るための式
//
//	EXCEPTIONS
//		なし

inline
ValueExpression*
ExpandSynonym::getParameter() const
{
	return _parameter;
}

} // namespace Function

namespace Generator
{
	//	CLASS
	//	Statemente::Generator::Definition -- <generator clause>
	//
	//	NOTES

	class Definition
		: public ValueExpression
	{
	public:
		typedef ValueExpression Super;

		// Constant value denoting when generated
		enum When {
			Always = 0,
			ByDefault,
		};

		//constructor
		Definition()
			: Super(op_GeneratorDefinition, 0),
			  m_eWhen(Always), m_bIdentity(false)
		{}
		// Constructor
		Definition(When eWhen_, ValueExpressionList* pOptions_, bool bIdentity_)
			: Super(op_GeneratorDefinition, 0),
			  m_eWhen(eWhen_), m_bIdentity(bIdentity_)
		{
			setOperandList(pOptions_);
		}
		// Copy constructor
		Definition(const Definition& cOther_)
			: Super(cOther_),
			  m_eWhen(cOther_.m_eWhen),
			  m_bIdentity(cOther_.m_bIdentity)
		{}
		// Destructor
		virtual ~Definition() {}

		// Get when generated
		When getWhen() const {return m_eWhen;}
		// Get options
		ValueExpressionList* getOptions() const {return getOperandList();}
		// Get whether identity
		bool isIdentity() const {return m_bIdentity;}

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
		virtual void serialize(ModArchive& cArchive_);

	protected:
	private:
		//copy will be never called
		Definition& operator=(const Definition& cOther_);

		When m_eWhen;					// when generated
		bool m_bIdentity;				// identity or not
	};

	//	CLASS
	//	Statemente::Generator::Option -- <generator option>
	//
	//	NOTES

	class Option
		: public ValueExpression
	{
	public:
		typedef ValueExpression Super;

		// Constant value denoting option types
		enum Type {
			Start = 0,
			Increment,
			MaxValue,
			MinValue,
			Cycle,
			GetMax,
			TypeNum
		};

		//constructor
		Option()
			: Super(op_GeneratorOption, 0),
			  m_eType(TypeNum)
		{}
		// Constructor
		Option(Type eType_, Literal* pValue_);
		// Copy constructor
		Option(const Option& cOther_)
			: Super(cOther_),
			  m_eType(cOther_.m_eType)
		{}
		// Destructor
		virtual ~Option() {}

		// Get option type
		Type getType() const {return m_eType;}
		// Get option value
		Literal* getValue() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
		virtual void serialize(ModArchive& cArchive_);

	protected:
	private:
		//copy will be never called
		Option& operator=(const Option& cOther_);

		Type m_eType;					// option type
	};

} // namespace Generator

namespace Expression
{
	class SimpleCase : public ValueExpression
	{
	public:
		//constructor
		SimpleCase()
			: ValueExpression(op_Case, 0)
		{}
		SimpleCase(ValueExpression* pOperand_,
				   ValueExpressionList* pWhenList_,
				   ValueExpression* pElse_);
		SimpleCase(const SimpleCase& cOther_);
		~SimpleCase();

		ValueExpression* getOperand() const;
		ValueExpressionList* getWhenList() const;
		ValueExpression* getElse() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
	//	virtual void serialize(ModArchive& cArchive_);
	protected:
	private:
	};
	class SearchedCase : public ValueExpression
	{
	public:
		//constructor
		SearchedCase()
			: ValueExpression(op_Case, 0)
		{}
		SearchedCase(ValueExpressionList* pWhenList_,
					 ValueExpression* pElse_);
		SearchedCase(const SearchedCase& cOther_);
		~SearchedCase();

		ValueExpressionList* getWhenList() const;
		ValueExpression* getElse() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
	//	virtual void serialize(ModArchive& cArchive_);
	protected:
	private:
	};
	class SimpleWhen : public ValueExpression
	{
	public:
		//constructor
		SimpleWhen()
			: ValueExpression(op_When, 0)
		{}
		SimpleWhen(ValueExpressionList* pWhenOperandList_,
				   ValueExpression* pResult_);
		SimpleWhen(const SimpleWhen& cOther_);
		~SimpleWhen();

		ValueExpressionList* getWhenOperandList() const;
		ValueExpression* getResult() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
	//	virtual void serialize(ModArchive& cArchive_);
	protected:
	private:
	};
	class SearchedWhen : public ValueExpression
	{
	public:
		//constructor
		SearchedWhen()
			: ValueExpression(op_When, 0)
		{}
		SearchedWhen(ValueExpression* pWhenCondition_,
					 ValueExpression* pResult_);
		SearchedWhen(const SearchedWhen& cOther_);
		~SearchedWhen();

		ValueExpression* getWhenCondition() const;
		ValueExpression* getResult() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		// create clone
		virtual Object* copy() const;

	/////////////////////////////
	// Common::Externalizable::
		virtual int getClassID() const;

	/////////////////////////////
	// ModSerializable::
	//	virtual void serialize(ModArchive& cArchive_);
	protected:
	private:
	};

} // namespace Expression

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
//	const SimpleCase& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

inline
Expression::SimpleCase::
SimpleCase(const SimpleCase& cOther_)
	: ValueExpression(cOther_)
{}

// FUNCTION public
//	Statement::Expression::SimpleCase::~SimpleCase -- 
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

inline
Expression::SimpleCase::
~SimpleCase()
{}

// FUNCTION public
//	Statement::Expression::SimpleCase::getOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SimpleCase::
getOperand() const
{
	return getRight();
}

// FUNCTION public
//	Statement::Expression::SimpleCase::getWhenList -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpressionList*
//
// EXCEPTIONS

inline
ValueExpressionList*
Expression::SimpleCase::
getWhenList() const
{
	return getOperandList();
}

// FUNCTION public
//	Statement::Expression::SimpleCase::getElse -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SimpleCase::
getElse() const
{
	return getOption();
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
//	const SeachedCase& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

inline
Expression::SearchedCase::
SearchedCase(const SearchedCase& cOther_)
	: ValueExpression(cOther_)
{}

// FUNCTION public
//	Statement::Expression::SearchedCase::~SearchedCase -- 
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

inline
Expression::SearchedCase::
~SearchedCase()
{}

// FUNCTION public
//	Statement::Expression::SearchedCase::getWhenList -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpressionList*
//
// EXCEPTIONS

inline
ValueExpressionList*
Expression::SearchedCase::
getWhenList() const
{
	return getOperandList();
}

// FUNCTION public
//	Statement::Expression::SearchedCase::getElse -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SearchedCase::
getElse() const
{
	return getOption();
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
//	const SimpleWhen& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

inline
Expression::SimpleWhen::
SimpleWhen(const SimpleWhen& cOther_)
	: ValueExpression(cOther_)
{}

// FUNCTION public
//	Statement::Expression::SimpleWhen::~SimpleWhen -- 
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

inline
Expression::SimpleWhen::
~SimpleWhen()
{}

// FUNCTION public
//	Statement::Expression::SimpleWhen::getWhenOperandList -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpressionList*
//
// EXCEPTIONS

inline
ValueExpressionList*
Expression::SimpleWhen::
getWhenOperandList() const
{
	return getOperandList();
}

// FUNCTION public
//	Statement::Expression::SimpleWhen::getResult -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SimpleWhen::
getResult() const
{
	return getRight();
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
//	const SearchedWhen& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

inline
Expression::SearchedWhen::
SearchedWhen(const SearchedWhen& cOther_)
	: ValueExpression(cOther_)
{}

// FUNCTION public
//	Statement::Expression::SearchedWhen::~SearchedWhen -- 
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

inline
Expression::SearchedWhen::
~SearchedWhen()
{}

// FUNCTION public
//	Statement::Expression::SearchedWhen::getWhenCondition -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SearchedWhen::
getWhenCondition() const
{
	return getLeft();
}

// FUNCTION public
//	Statement::Expression::SearchedWhen::getResult -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

inline
ValueExpression*
Expression::SearchedWhen::
getResult() const
{
	return getRight();
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_VALUEEXPRESSION_H

//
// Copyright (c) 1999, 2000, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
