// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Default.h -- デフォルト指定関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_DEFAULT_H
#define	__SYDNEY_SCHEMA_DEFAULT_H

#include "Schema/Module.h"
#include "Schema/Externalizable.h"
#include "Schema/ObjectName.h"
#include "Common/Object.h"
#include "Common/Data.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN

namespace Common
{
	class IntegerArrayData;
	class SQLData;
}

namespace Statement
{
	class ValueExpression;
}

_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::Default -- デフォルト指定を表すクラス
//
//	NOTES

class Default
	: public	Common::Object,
	  public	Externalizable
{
public:
	//	CLASS
	//	Schema::Default::Category -- デフォルト指定の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::Default::Category::Value --
		//		デフォルト指定の種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			Constant,							// 定数
			Function,							// 関数
			Identity,							// Generator(Identity Column)
			ValueNum,							// 種別数
			UseOnUpdate = 128,					// UseOnUpdate = trueを示す閾値
			UseAlways = 256						// UseAlways = trueを示す閾値
		};
	};

	Default();
	Default(const Default& src);
												// コンストラクター
	~Default();									// デストラクター

	Default&				operator =(const Default& src);
												// = 演算子

	void					clear();			// メンバーをすべて初期化する

	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// デフォルト指定の種別を得る
	SYD_SCHEMA_FUNCTION
	const Common::Data*		getConstant() const;
												// 定数値を得る

	// 関数の種別を得る
	int						getFunction() const;
	// Identityのオプション指定を得る
	const Common::IntegerArrayData&
							getIdentitySpec() const;

	// 更新でも使うかを得る
	bool					isUseOnUpdate() const;
	// 常に使うかを得る
	bool					isUseAlways() const;
	// DefaultはConstantか
	bool					isConstant() const;
	// DefaultはIdentityか
	bool					isIdentity() const;
	// DefaultはFunctionか
	bool					isFunction() const;

	void					setValue(const ObjectName& cName_,
									 const Statement::ValueExpression& cStatement_,
									 bool bUseOnUpdate_,
									 const Common::SQLData& cType_);
												// ValueExpressionから値をセットする

	// Get as character string
	ModUnicodeString		toString() const;

	SYD_SCHEMA_FUNCTION
	bool					isNull() const;		// DefaultはNullDataか

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る
private:
	void					destruct();			// デストラクター下位関数

	// setValue's utilities
	void setValueConstant(const ObjectName& cName_,
						  const Statement::ValueExpression& cStatement_,
						  bool bUseOnUpdate_,
						  const Common::SQLData& cType_);
	void setValueFunction(const ObjectName& cName_,
						  const Statement::ValueExpression& cStatement_,
						  bool bUseOnUpdate_,
						  const Common::SQLData& cType_);
	void setValueGenerator(const ObjectName& cName_,
						   const Statement::ValueExpression& cStatement_,
						   bool bUseOnUpdate_,
						   const Common::SQLData& cType_);
	void setValueArray(const ObjectName& cName_,
					   const Statement::ValueExpression& cStatement_,
					   bool bUseOnUpdate_,
					   const Common::SQLData& cType_);

	Category::Value			_category;			// デフォルト値の種別
	Common::Data::Pointer	_constant;			// 定数値
	bool					m_bUseOnUpdate;		// 更新時に使うか
	bool					m_bUseAlways;		// Defaultを常に使うか
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_DEFAULT_H

//
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
