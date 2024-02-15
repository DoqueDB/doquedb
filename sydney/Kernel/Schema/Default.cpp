// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Default.cpp -- デフォルト値関連の関数定義
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Default.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/InputArchive.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"

#include "Exception/InvalidDefault.h"
#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"

#include "Os/Limits.h"

#include "Statement/Literal.h"
#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	Common::Data::Pointer
	_convertLiteralToData(const ObjectName& cName_,
						  const Statement::ValueExpression& cValueExpression_,
						  const Common::SQLData& cType_)
	{
		if (cValueExpression_.getOperator() == Statement::ValueExpression::op_Nullobj) {
			return Common::NullData::getInstance();
		}
		const Statement::Literal* pLiteral =
			_SYDNEY_DYNAMIC_CAST(const Statement::Literal*, cValueExpression_.getPrimary());
		if (!pLiteral) {
			_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
		}

		Common::Data::Pointer pData = pLiteral->createData();
		if (!pData.get()) {
			// Data can't be created -> illegal default
			_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
		}
		; _SYDNEY_ASSERT(pData.get());
		if (!pData->isNull()) {
			// 得られた値の互換性をチェックする
			Common::Data::Pointer pCastData;
			Common::SQLData cDataSQLType;
			if (!pData->getSQLType(cDataSQLType)
				|| (!Common::SQLData::isAssignable(cDataSQLType, cType_)
					&& !cType_.cast(cDataSQLType, pData,
									pCastData = Common::DataInstance::create(cType_),
									false/* not comparison */, true /* no throw */))) {
				// SQLData型が得られなかったか、代入可能でなかった
				_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
			}
			// castした結果代入可能になったらそれを使う
			// BinaryData uses specified data for ColumnMetaData
			if (pCastData.get()
				&& (pCastData->getType() != Common::DataType::Binary)) {
				pData = pCastData;
			}
		}
		return pData;
	}

	// Constant table for generator options
	struct {
		const char* m_pszName;
		bool m_bUseValue;
		int m_iDefaultValue;
	} _GeneratorOptions[Statement::Generator::Option::TypeNum] =
	{
			{"start with",		true,	0},
			{"increment by",	true,	1},
			{"maxvalue",		true,	Os::Limits<int>::getMax()},
			{"minvalue",		true,	0},
			{"cycle",			false,	0},
			{"get max",			false,	0}
	};
}

//	FUNCTION public
//	Schema::Default::Default -- 
//		デフォルト指定を表すクラスのデフォルトコンストラクター
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

Default::
Default()
	: _category(Category::Unknown),
	  _constant(),
	  m_bUseOnUpdate(false),
	  m_bUseAlways(false)
{ }

//	FUNCTION public
//	Schema::Default::Default --
//		デフォルト指定を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Default&	src
//			自分自身へ代入するデフォルト指定
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Default::
Default(const Schema::Default& src)
	: _category(Category::Unknown),
	  _constant(),
	  m_bUseOnUpdate(false),
	  m_bUseAlways(false)
{
	*this = src;
}

//	FUNCTION public
//	Schema::Default::~Default --
//		デフォルト指定を表すクラスのデストラクター
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

Default::
~Default()
{
	destruct();
}

//	FUNCTION private
//	Schema::Default::destruct --
//		デフォルト指定を表すクラスのデストラクター下位関数
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

void
Default::
destruct()
{
}

//	FUNCTION public
//	Schema::Default::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Default&	src
//			自分自身へ代入するデフォルト指定
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

Default&
Default::
operator =(const Default& src)
{
	if (this != &src) {
		destruct();

		_category = src._category;
		if (src._constant.get())
			_constant = src._constant->copy();
		m_bUseOnUpdate = src.m_bUseOnUpdate;
		m_bUseAlways = src.m_bUseAlways;
	}
	return *this;
}

//	FUNCTION public
//	Schema::Default::getCategory -- デフォルト指定の種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデフォルト指定の種別
//
//	EXCEPTIONS
//		なし

Default::Category::Value
Default::
getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Schema::Default::getConstant --
//		デフォルト指定で指定されたデフォルト値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られたデフォルト値を格納する領域の先頭アドレス
//		0
//			デフォルト指定として、定数値が指定されていない

const Common::Data*
Default::
getConstant() const
{
	return (_category == Category::Constant) ? _constant.get() : 0;
}

// FUNCTION public
//	Schema::Default::getFunction -- 関数の種別を得る
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

int
Default::
getFunction() const
{
	; _SYDNEY_ASSERT(_category == Category::Function);
	; _SYDNEY_ASSERT(_constant->getType() == Common::DataType::Integer);
	const Common::IntegerData& cValue =
		_SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, *_constant);
	return cValue.getValue();
}

// FUNCTION public
//	Schema::Default::getIdentitySpec -- Identityのオプション指定を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::IntegerArrayData&
//
// EXCEPTIONS

const Common::IntegerArrayData&
Default::
getIdentitySpec() const
{
	; _SYDNEY_ASSERT(_category == Category::Identity);
	; _SYDNEY_ASSERT(_constant->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(_constant->getElementType() == Common::DataType::Integer);

	return _SYDNEY_DYNAMIC_CAST(const Common::IntegerArrayData&, *_constant);
}

// FUNCTION public
//	Schema::Default::isUseOnUpdate -- 更新でも使うかを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Default::
isUseOnUpdate() const
{
	return m_bUseOnUpdate;
}

// FUNCTION public
//	Schema::Default::isUseAlways -- 常に使うかを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Default::
isUseAlways() const
{
	return m_bUseAlways;
}

// FUNCTION public
//	Schema::Default::isConstant -- DefaultはConstantか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Default::
isConstant() const
{
	return _category == Category::Constant;
}

// FUNCTION public
//	Schema::Default::isIdentity -- DefaultはIdentityか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Default::
isIdentity() const
{
	return _category == Category::Identity;
}

// FUNCTION public
//	Schema::Default::isFunction -- DefaultはFunctionか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Default::
isFunction() const
{
	return _category == Category::Function;
}

// FUNCTION public
//	Schema::Default::setValue -- ValueExpressionから値をセットする
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cName_
//	const Statement::ValueExpression& cStatement_
//	bool bUseOnUpdate_
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Default::
setValue(const Schema::ObjectName& cName_,
		 const Statement::ValueExpression& cStatement_,
		 bool bUseOnUpdate_, const Common::SQLData& cType_)
{
	switch (cStatement_.getOperator()) {
	case Statement::ValueExpression::op_Nullobj:
	case Statement::ValueExpression::op_Literal:
		{
			setValueConstant(cName_, cStatement_, bUseOnUpdate_, cType_);
			break;
		}
	case Statement::ValueExpression::op_Func:
		{
			setValueFunction(cName_, cStatement_, bUseOnUpdate_, cType_);
			break;
		}
	case Statement::ValueExpression::op_GeneratorDefinition:
		{
			setValueGenerator(cName_, cStatement_, bUseOnUpdate_, cType_);
			break;
		}
	case Statement::ValueExpression::op_Arrayconst:
		{
			setValueArray(cName_, cStatement_, bUseOnUpdate_, cType_);
			break;
		}
	default:
		{
			// Invalid value expression is specified for default clause
			ModUnicodeOstrStream stream;
			stream << "ColumnDefinition: invalid default value.";
			_SYDNEY_THROW1(Exception::SQLSyntaxError, stream.getString());
		}
	}
}

//	FUNCTION public
//	Schema::Default::isNull -- 
//		デフォルトがNullDataであるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		デフォルトがNullDataの場合 true、NullDataでない場合 false
//
//	EXCEPTIONS

bool
Default::
isNull() const
{
	return (_category == Category::Unknown) || (_constant->isNull());
}

//	FUNCTION public
//	Schema::Default::serialize -- 
//		デフォルト指定を表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Default::
serialize(ModArchive& archiver)
{
	if (archiver.isStore()) {

		// デフォルト指定の種別
		{
		int tmp = _category;
		if (m_bUseOnUpdate) tmp += Category::UseOnUpdate;
		if (m_bUseAlways) tmp += Category::UseAlways;
		archiver << tmp;
		}
		if (_category != Category::Unknown) {

			// デフォルトがNULLでない

			Common::OutputArchive& out =
				dynamic_cast<Common::OutputArchive&>(archiver);

			; _SYDNEY_ASSERT(_constant.get());
			out.writeObject(_constant.get());
		}
	} else {

		// デフォルト指定の種別
		{
		int tmp;
		archiver >> tmp;
		if (m_bUseAlways = (tmp >= Category::UseAlways))
			tmp -= Category::UseAlways;
		if (m_bUseOnUpdate = (tmp >= Category::UseOnUpdate))
			tmp -= Category::UseOnUpdate;
		_category = static_cast<Category::Value>(tmp);
		}
		if (_category != Category::Unknown) {

			// デフォルトがNULLでない

			Common::InputArchive& in =
				dynamic_cast<Common::InputArchive&>(archiver);

			_constant = dynamic_cast<Common::Data*>(in.readObject());
		}
	}
}

//	FUNCTION public
//	Schema::Default::getClassID -- このクラスのクラス ID を得る
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

int
Default::
getClassID() const
{
	return Externalizable::Category::Default +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Default::clear --
//		デフォルト指定を表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Default::
clear()
{
	_category = Category::Unknown;

	destruct();
}

// setValue's utilities

// FUNCTION public
//	Schema::Default::setValueConstant -- set default value from a literal
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cName_
//	const Statement::ValueExpression& cStatement_
//	bool bUseOnUpdate_
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Default::
setValueConstant(const Schema::ObjectName& cName_,
				 const Statement::ValueExpression& cStatement_,
				 bool bUseOnUpdate_,
				 const Common::SQLData& cType_)
{
	// Get data from Literal
	_constant = _convertLiteralToData(cName_, cStatement_, cType_);

	// Constant
	_category = _constant->isNull() ? Category::Unknown : Category::Constant;

	// UseOnUpdateは無視する
	if (bUseOnUpdate_) {
		SydInfoMessage << "ColumnDefinition: default has USING ON UPDATE specification but ignored."
					   << ModEndl;
	}
}

// FUNCTION public
//	Schema::Default::setValueFunction -- set default value from a function
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cName_
//	const Statement::ValueExpression& cStatement_
//	bool bUseOnUpdate_
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Default::
setValueFunction(const Schema::ObjectName& cName_,
				 const Statement::ValueExpression& cStatement_,
				 bool bUseOnUpdate_,
				 const Common::SQLData& cType_)
{
	// 関数種別から値をセットする
	switch (cStatement_.getFunction()) {
	case Statement::ValueExpression::func_Current_Timestamp:
		{
			// 型の互換性をチェックする
			Common::SQLData cDataSQLType;
			(void) Common::Data::getSQLType(Common::DataType::DateTime, cDataSQLType);
			if (!Common::SQLData::isAssignable(cDataSQLType, cType_)) {
				// 列の型がこの関数の値を代入可能ではなかった
				// (Functionの場合はCastが必要ならInvalidDefaultにする)
				_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
			}
			// 関数としてセットする
			_category = Category::Function;
			_constant = new Common::IntegerData(cStatement_.getFunction());
			m_bUseOnUpdate = bUseOnUpdate_;
			break;
		}
	default:
		{
			// Invalid value expression is specified for default clause
			ModUnicodeOstrStream stream;
			stream << "ColumnDefinition: invalid function in default clause.";
			_SYDNEY_THROW1(Exception::SQLSyntaxError, stream.getString());
		}
	}
}

// FUNCTION public
//	Schema::Default::setValueGenerator -- set default value from generator specification
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cName_
//	const Statement::ValueExpression& cStatement_
//	bool bUseOnUpdate_
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Default::
setValueGenerator(const Schema::ObjectName& cName_,
				  const Statement::ValueExpression& cStatement_,
				  bool bUseOnUpdate_,
				  const Common::SQLData& cType_)
{
	// UseOnUpdateはtrueになることはない
	; _SYDNEY_ASSERT(!bUseOnUpdate_);

	// 型の互換性をチェックする
	Common::SQLData cDataSQLType;
	(void) Common::Data::getSQLType(Common::DataType::Integer, cDataSQLType);
	if (!Common::SQLData::isAssignable(cDataSQLType, cType_)) {
		// 列の型がこの関数の値を代入可能ではなかった
		// (Generatorの場合はCastが必要ならInvalidDefaultにする)
		_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
	}

	// Generator optionの種別ごとに値を保持する
	ModVector<int> vecOptionValue(Statement::Generator::Option::TypeNum);
	// 値を設定したかを記録すると同時にデフォルト値を入れておく
	bool vecOptionSet[Statement::Generator::Option::TypeNum];
	for (int i = 0; i < Statement::Generator::Option::TypeNum; ++i) {
		vecOptionSet[i] = false;
		vecOptionValue[i] = _GeneratorOptions[i].m_iDefaultValue;
	}

	// Generator::Definitionにキャストする
	const Statement::Generator::Definition& cDefinition =
		_SYDNEY_DYNAMIC_CAST(const Statement::Generator::Definition&, cStatement_);

	// Identity Columnしかサポートしない
	if (!cDefinition.isIdentity()) {
		SydInfoMessage << "ColumnDefinition: generator should be defined as identity column." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// GENERATED ALWAYSか
	if (cDefinition.getWhen() == Statement::Generator::Definition::Always) {
		m_bUseAlways = true;
	}

	// オプションをセットする
	Statement::ValueExpressionList* pOptions = cDefinition.getOptions();
	if (pOptions) {
		int iCount = pOptions->getCount();
		for (int i = 0; i < iCount; ++i) {
			Statement::ValueExpression* pVE = pOptions->getValueExpressionAt(i);
			; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_GeneratorOption);

			Statement::Generator::Option* pOption =
				_SYDNEY_DYNAMIC_CAST(Statement::Generator::Option*, pVE);

			int iType = pOption->getType();
			if (vecOptionSet[iType]) {
				// 同じ種別のオプションは2つ以上あってはいけない
				ModUnicodeOstrStream stream;
				stream << "ColumnDefinition: duplicate generator options.";
				_SYDNEY_THROW1(Exception::SQLSyntaxError, stream.getString());
			}
			vecOptionSet[iType] = true;

			if (pOption->getValue()) {
				// オプションの値をセットする
				Common::Data::Pointer pIntValue =
					pOption->getValue()->createData(Common::DataType::Integer, true /* for assign */);
				; _SYDNEY_ASSERT(!pIntValue->isNull());
				; _SYDNEY_ASSERT(pIntValue->getType() == Common::DataType::Integer);
				vecOptionValue[iType] =
					_SYDNEY_DYNAMIC_CAST(Common::IntegerData&, *pIntValue).getValue();
			} else {
				// フラグを表すオプションなので1をセットする
				; _SYDNEY_ASSERT(iType == Statement::Generator::Option::Cycle
								 || iType == Statement::Generator::Option::GetMax);
				vecOptionValue[iType] = 1;
			}
		}

		// オプションを検査する
		int iIncrement = vecOptionValue[Statement::Generator::Option::Increment];
		int iStart = vecOptionValue[Statement::Generator::Option::Start];
		int iMinValue = vecOptionValue[Statement::Generator::Option::MinValue];
		int iMaxValue = vecOptionValue[Statement::Generator::Option::MaxValue];

		// ALWAYSとGET MAXが同時に指定されていたら警告メッセージ
		if (m_bUseAlways && (vecOptionValue[Statement::Generator::Option::GetMax] == 1)) {
			SydInfoMessage << "ColumnDefinition: GET MAX is ignored when it is defined with GENERATED ALWAYS."
						   << ModEndl;
		}

		// 未指定時のデフォルト値をセットする
		if (!vecOptionSet[Statement::Generator::Option::Start]) {
			// Startが指定されていないときはIncrementの正負に応じてMaxやMinを使う
			iStart = vecOptionValue[Statement::Generator::Option::Start] = (iIncrement > 0) ? iMinValue : iMaxValue;
		}
		if (iIncrement > 0 && !vecOptionSet[Statement::Generator::Option::MinValue]) {
			// Ascending sequenceでMINVALUEが指定されていないときはStartと0の小さいほうを使う
			iMinValue = vecOptionValue[Statement::Generator::Option::MinValue] = ModMin(iStart, 0);
		}

		// オプションの整合性を検査する
		// 1. Increment != 0
		// 2. MinValue < MaxValue
		// 3. MinValue <= Start <= MaxValue
		if ((iIncrement == 0)
			||
			(iMinValue >= iMaxValue)
			||
			((iMinValue > iStart) || (iStart > iMaxValue))) {
			ModUnicodeOstrStream stream;
			stream << "ColumnDefinition: illegal options for generator.";
			_SYDNEY_THROW1(Exception::SQLSyntaxError, stream.getString());
		}
	}

	// 結果をセットする
	_category = Category::Identity;
	_constant = new Common::IntegerArrayData(vecOptionValue);
}

// FUNCTION public
//	Schema::Default::setValueArray -- set default value from array constructor
//
// NOTES
//
// ARGUMENTS
//	const ObjectName& cName_
//	const Statement::ValueExpression& cStatement_
//	bool bUseOnUpdate_
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Default::
setValueArray(const ObjectName& cName_,
			  const Statement::ValueExpression& cStatement_,
			  bool bUseOnUpdate_,
			  const Common::SQLData& cType_)
{
	if (!cType_.isArrayType()) {
		_SYDNEY_THROW1(Exception::InvalidDefault, cName_);
	}
		
	// get the elements of array constructor
	Statement::ValueExpressionList* pList =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, cStatement_.getPrimary());

	ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData;
	if (pList) {
		int n = pList->getCount();
		pData->reserve(n);
		Common::SQLData cElementType = cType_.getElementType();

		for (int i = 0; i < n; ++i) {
			pData->pushBack(_convertLiteralToData(cName_, *pList->getValueExpressionAt(i), cElementType));
		}
	}
	_constant = pData.release();
	_category = Category::Constant;

	// UseOnUpdateは無視する
	if (bUseOnUpdate_) {
		SydInfoMessage << "ColumnDefinition: default has USING ON UPDATE specification but ignored."
					   << ModEndl;
	}
}

// FUNCTION public
//	Schema::Default::toString -- Get as character string
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

ModUnicodeString
Default::
toString() const
{
	ModUnicodeOstrStream stream;
	if (isFunction()) {
		// function default
		stream << Statement::ValueExpression::getFunctionName(getFunction());
	} else if (isIdentity()) {
		// identity column
		stream << "generated " << (isUseAlways() ? "always" : "by default")
			   << " as identity";

		const Common::IntegerArrayData& cIdentitySpec = getIdentitySpec();
		ModUnicodeOstrStream options;
		int n = cIdentitySpec.getCount();
		for (int i = 0; i < n; ++i) {
			if (cIdentitySpec.getElement(i) != _GeneratorOptions[i].m_iDefaultValue) {
				if (!options.isEmpty()) {
					options << ' ';
				}
				options << _GeneratorOptions[i].m_pszName;
				if (_GeneratorOptions[i].m_bUseValue) {
					options << ' ' << cIdentitySpec.getElement(i);
				}
			}
		}
		if (!options.isEmpty()) {
			stream << " (" << options.getString() << ")";
		}
	} else {
		// normal constant default
		; _SYDNEY_ASSERT(getConstant());
		stream << getConstant()->toString();
	}
	if (isUseOnUpdate()) {
		; _SYDNEY_ASSERT(isFunction() || isIdentity());
		stream << " using on update";
	}
	return stream.getString();
}

//
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
