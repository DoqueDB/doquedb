// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnMetaData.cpp -- カラムのメタデータ
// 
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"
#include "Common/ColumnMetaData.h"
#include "Common/ClassID.h"
#include "Os/Limits.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {

//
//	STRUCT
//	_$$::_Item
//
struct _Item
{
	_Item(const char* name_,
		  ColumnMetaData::Type::Value type_,
		  bool flag_,
		  int size_)
		: _name(name_), _type(type_), _flag(flag_), _size(size_) {}
	ModUnicodeString			_name;	// Sydney固有の型名
	ColumnMetaData::Type::Value	_type;	// SQL標準のタイプ
	bool						_flag;	// サイズにSQLDataの値が使えるか
	int							_size;	// 使えない場合のサイズ
};

#define	MAX			Os::Limits<int>::getMax()
#define UNKNOWN		ColumnMetaData::Type::Unknown
#define CHAR		ColumnMetaData::Type::Charactor
#define VARCHAR		ColumnMetaData::Type::CharactorVarying
#define NCHAR		ColumnMetaData::Type::NationalCharactor
#define NVARCHAR	ColumnMetaData::Type::NationalCharactorVarying
#define BINARY		ColumnMetaData::Type::Binary
#define VARBINARY	ColumnMetaData::Type::BinaryVarying
#define CLOB		ColumnMetaData::Type::CharactorLargeObject
#define NCLOB		ColumnMetaData::Type::NationalCharactorLargeObject
#define BLOB		ColumnMetaData::Type::BinaryLargeObject
#define NUMERIC		ColumnMetaData::Type::Numeric
#define SMALLINT	ColumnMetaData::Type::SmallInt
#define INT			ColumnMetaData::Type::Integer
#define BIGINT		ColumnMetaData::Type::BigInt
#define DECIMAL		ColumnMetaData::Type::Decimal
#define FLOAT		ColumnMetaData::Type::Float
#define REAL		ColumnMetaData::Type::Real
#define DOUBLE		ColumnMetaData::Type::DoublePrecision
#define BOOL		ColumnMetaData::Type::Boolean
#define DATE		ColumnMetaData::Type::Date
#define TIME		ColumnMetaData::Type::Time
#define TIMESTAMP	ColumnMetaData::Type::Timestamp
#define ARRAY		ColumnMetaData::Type::Array
#define LANG		ColumnMetaData::Type::Language
#define WORD		ColumnMetaData::Type::Word
	
//
//	LOCAL variable
//
_Item _Table[SQLData::Type::ValueNum][SQLData::Flag::ValueNum] =
{
	// Type::NoType
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Char
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("varchar",	VARCHAR,		true,	MAX),	// OldFixed
		_Item("varchar",	VARCHAR,		true,	MAX),	// OldVariable
		_Item("varchar",	VARCHAR,		false,	MAX),	// Unlimited
		_Item("char",		CHAR,			true,	0),		// Fixed
		_Item("varchar",	VARCHAR,		true,	MAX)	// Variable
	},
	// Type::NChar
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("nvarchar",	NVARCHAR,		true,	MAX),	// OldFixed
		_Item("nvarchar",	NVARCHAR,		true,	MAX),	// OldVariable
		_Item("nvarchar",	NVARCHAR,		false,	MAX),	// Unlimited
		_Item("nchar",		NCHAR,			true,	0),		// Fixed
		_Item("nvarchar",	NVARCHAR,		true,	MAX)	// Variable
	},
	// Type::Int
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("int",		INT,			false,	10),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("int",		INT,			false,	10),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Float
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("float",		DOUBLE,			false,	22),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("float",		DOUBLE,			false,	22),	// Fixed
		_Item("",			DOUBLE,		    false,	0)		// Variable
	},
	// Type::DateTime
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("datetime",	TIMESTAMP,		false,	23),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("datetime",	TIMESTAMP,		false,	23),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::UniqueIdentifier
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("uniqueidentifier",	NVARCHAR,false,	36),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("uniqueidentifier",	NCHAR,	false,	36),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Binary
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("varbinary",	VARBINARY,		true,	MAX),	// OldFixed
		_Item("varbinary",	VARBINARY,		true,	MAX),	// OldVariable
		_Item("varbinary",	VARBINARY,		false,	MAX),	// Unlimited
		_Item("binary",		BINARY,			true,	0),		// Fixed
		_Item("varbinary",	VARBINARY,		true,	MAX)	// Variable
	},
	// Type::Image
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("image",		VARBINARY,		false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::NText
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("ntext",		NVARCHAR,		false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Fulltext
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("fulltext",	NVARCHAR,		false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Language
	{
		_Item("language",	LANG,			false,	8),		// None
		_Item("language",	LANG,			false,	8),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("language",	LANG,			false,	8),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::BLOB
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("blob",		BLOB,			false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::CLOB
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("clob",		CLOB,			false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::NCLOB
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("",			UNKNOWN,		false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("nclob",		NCLOB,			false,	MAX),	// Unlimited
		_Item("",			UNKNOWN,		false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Decimal
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("decimal",	DECIMAL,		true,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("decimal",	DECIMAL,		true,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Date
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("date",		DATE,			false,	10),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("date",		DATE,			false,	10),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Time
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("time",		TIME,			false,	8),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("time",		TIME,			false,	8),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Timestamp
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("timestamp",	TIMESTAMP,		false,	23),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("timestamp",	TIMESTAMP,		false,	23),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::UInt
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("uint",		INT,			false,	10),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("uint",		INT,			false,	10),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::Word
	{
		_Item("word",		WORD,			false,	0),		// None
		_Item("word",		WORD,			false,	0),		// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("word",		WORD,			false,	0),		// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	},
	// Type::BigInt
	{
		_Item("",			UNKNOWN,		false,	0),		// None
		_Item("bigint",		BIGINT,			false,	19),	// OldFixed
		_Item("",			UNKNOWN,		false,	0),		// OldVariable
		_Item("",			UNKNOWN,		false,	0),		// Unlimited
		_Item("bigint",		BIGINT,			false,	19),	// Fixed
		_Item("",			UNKNOWN,		false,	0)		// Variable
	}
};
	
#undef UNKNOWN
#undef CHAR
#undef VARCHAR
#undef NCHAR
#undef NVARCHAR
#undef BINARY
#undef VARBINARY
#undef CLOB
#undef NCLOB
#undef BLOB
#undef NUMERIC
#undef SMALLINT
#undef INT		
#undef BIGINT
#undef DECIMAL
#undef FLOAT
#undef REAL
#undef DOUBLE
#undef BOOL
#undef DATE
#undef TIME
#undef TIMESTAMP
#undef ARRAY
#undef LANG
#undef WORD

DataType::Type _TypeTable[ColumnMetaData::Type::ValueNum] =
{
	DataType::Undefined,				//Unknown
			
	DataType::String,					//Charactor
	DataType::String,					//CharactorVarying
	DataType::String,					//NationalCharactor
	DataType::String,					//NationalCharactorVarying

	DataType::Binary,					//Binary
	DataType::Binary,					//BinaryVarying

	DataType::String,					//CharactorLargeObject
	DataType::String,					//NationalCharactorLargeObject
	DataType::Binary,					//BinaryLargeObject

	DataType::Decimal,					//Numeric
	DataType::Undefined,				//SmallInt
	DataType::Integer,					//Integer
	DataType::Integer64,				//BigInt

	DataType::Decimal,					//Decimal
	DataType::Double,					//Float
	DataType::Double,					//Real
	DataType::Double,					//DoublePrecision

	DataType::Undefined,				//Boolean

	DataType::Date,						//Date
	DataType::Undefined,				//Time
	DataType::DateTime,					//Timestamp

	DataType::Language,					//Language
	DataType::Word						//Word
};

//
//	FUNCTION local
//
int _getPrecision(const SQLData& cSQLData_)
{
	int precision = 0;
	switch (cSQLData_.getType())
	{
	case SQLData::Type::Int:
	case SQLData::Type::UInt:
		precision = 10;
		break;
	case SQLData::Type::Decimal:
		precision = cSQLData_.getLength();
		break;
	case SQLData::Type::BigInt:
		precision = 19;
		break;
	}
	return precision;
}

//
//	FUNCTION local
//
int _getScale(const SQLData& cSQLData_)
{
	int scale = 0;
	switch (cSQLData_.getType())
	{
	case SQLData::Type::Decimal:
		scale = cSQLData_.getScale();
		break;
	}
	return scale;
}

}

//
//	FUNCTION public
//	Common:ColumnMetaData::ColumnMetaData -- コンストラクタ
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
ColumnMetaData::ColumnMetaData()
	: m_eType(Type::Unknown),
	  m_iDisplaySize(0), m_iPrecision(0), m_iScale(0), m_iMaxCardinality(0),
	  m_iFlag(Flag::None)
{
}

//
//	FUNCTION public
//	Common:ColumnMetaData::ColumnMetaData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Common::SQLData& cSQLData_
//		SQLデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ColumnMetaData::ColumnMetaData(const SQLData& cSQLData_)
	: m_iDisplaySize(0), m_iPrecision(0), m_iScale(0), m_iMaxCardinality(0),
	  m_iFlag(Flag::None)
{
	setSQLData(cSQLData_);
}

//
//	FUNCTION public
//	Common::ColumnMetaData::~ColumnMetaData -- デストラクタ
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
ColumnMetaData::~ColumnMetaData()
{
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setSQLData -- SQLデータ型を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::SQLData& cSQLData_
//		SQLデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setSQLData(const SQLData& cSQLData_)
{
	_Item& item = _Table[cSQLData_.getType()][cSQLData_.getFlag()];

	m_eType = item._type;
	m_cTypeName = item._name;
	if (item._flag == true && cSQLData_.getLength() > 0)
		m_iDisplaySize = cSQLData_.getLength();
	else
		m_iDisplaySize = item._size;
	m_iPrecision = _getPrecision(cSQLData_);
	m_iScale = _getScale(cSQLData_);
	m_iMaxCardinality = cSQLData_.getMaxCardinality();

	if (cSQLData_.getType() == SQLData::Type::UInt)
	{
		setUnsigned();
	}

}

//
//	FUNCTION public
//	Common::ColumnMetaData::getDataType -- Commonのデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataType::Type
//		Commonのデータ型
//
//	EXCEPTIONS
//
DataType::Type
ColumnMetaData::getDataType() const
{
	if (isArray())
		return DataType::Array;
		
	return _TypeTable[m_eType];
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setAutoIncrement -- 自動採番かどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bAutoIncrement_ (default true)
//		自動採番の場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setAutoIncrement(bool bAutoIncrement_)
{
	if (bAutoIncrement_)
		m_iFlag |= Flag::AutoIncrement;
	else
		m_iFlag &= ~Flag::AutoIncrement;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setCaseInsensitive
//		-- 大文字小文字が区別されないかどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bCaseInsensitive_ (default true)
//		大文字小文字が区別されない場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setCaseInsensitive(bool bCaseInsensitive_)
{
	if (bCaseInsensitive_)
		m_iFlag |= Flag::CaseInsensitive;
	else
		m_iFlag &= ~Flag::CaseInsensitive;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setUnsigned -- 符号なしかどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bUnsigned_ (default true)
//		符号なしの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setUnsigned(bool bUnsigned_)
{
	if (bUnsigned_)
		m_iFlag |= Flag::Unsigned;
	else
		m_iFlag &= ~Flag::Unsigned;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setNotSearchable -- 検索不可かどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bNotSearchable_ (default true)
//		検索不可の場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setNotSearchable(bool bNotSearchable_)
{
	if (bNotSearchable_)
		m_iFlag |= Flag::NotSearchable;
	else
		m_iFlag &= ~Flag::NotSearchable;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setReadOnly -- 読み込み専用かどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bReadOnly_ (default true)
//		読み込み専用の場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTINS
//
void
ColumnMetaData::setReadOnly(bool bReadOnly_)
{
	if (bReadOnly_)
		m_iFlag |= Flag::ReadOnly;
	else
		m_iFlag &= ~Flag::ReadOnly;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setNotNullable -- NotNullかどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bNotNullable_ (default true)
//		Nullが設定できない場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setNotNullable(bool bNotNullable_)
{
	if (bNotNullable_)
		m_iFlag |= Flag::NotNull;
	else
		m_iFlag &= ~Flag::NotNull;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::setUnique -- ユニークかどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bUnique_
//		ユニークな場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::setUnique(bool bUnique_)
{
	if (bUnique_)
		m_iFlag |= Flag::Unique;
	else
		m_iFlag &= ~Flag::Unique;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::getClassID -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//
int
ColumnMetaData::getClassID() const
{
	return ClassID::ColumnMetaDataClass;
}

//
//	FUNCTION public
//	Common::ColumnMetaData::serialize -- シリアル化を行う
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& archiver_
//		シリアル化するアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ColumnMetaData::serialize(ModArchive& archiver_)
{
	// ColumnMetaDataのserializeは項目が増えてもやり取りできるように、
	// やりとりするデータの前に、やりとりするデータの数を書くようにする
	
	if (archiver_.isStore())
	{
		// 書き込み
		int tmp = m_eType;
		archiver_ << tmp;

		tmp = 6;	// ModUnicodeStringの数
		archiver_ << tmp;
		archiver_ << m_cTypeName;
		archiver_ << m_cColumnName;
		archiver_ << m_cTableName;
		archiver_ << m_cDatabaseName;
		archiver_ << m_cColumnAliasName;
		archiver_ << m_cTableAliasName;

		tmp = 4;	// intの数
		archiver_ << tmp;
		archiver_ << m_iDisplaySize;
		archiver_ << m_iPrecision;
		archiver_ << m_iScale;
		archiver_ << m_iMaxCardinality;

		archiver_ << m_iFlag;
	}
	else
	{
		// 読み込み

		int tmp;
		archiver_ >> tmp;
		m_eType = static_cast<Type::Value>(tmp);

		archiver_ >> tmp;	// ModUnicodeStringの数
		if (tmp) archiver_ >> m_cTypeName, tmp--;
		if (tmp) archiver_ >> m_cColumnName, tmp--;
		if (tmp) archiver_ >> m_cTableName, tmp--;
		if (tmp) archiver_ >> m_cDatabaseName, tmp--;
		if (tmp) archiver_ >> m_cColumnAliasName, tmp--;
		if (tmp) archiver_ >> m_cTableAliasName, tmp--;

		archiver_ >> tmp;	// intの数
		if (tmp) archiver_ >> m_iDisplaySize, tmp--;
		if (tmp) archiver_ >> m_iPrecision, tmp--;
		if (tmp) archiver_ >> m_iScale, tmp--;
		if (tmp) archiver_ >> m_iMaxCardinality, tmp--;

		archiver_ >> m_iFlag;
	}
}

//
//	FUNCTION public
//	Common::ColumnMetaData::toString -- 文字列で表す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//
ModUnicodeString
ColumnMetaData::toString() const
{
	return getColumnAliasName();
}

//
//	Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
