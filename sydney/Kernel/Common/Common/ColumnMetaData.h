// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnMetaData.h -- カラムのメタデータ
// 
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_COLUMNMETADATA_H
#define __TRMEISTER_COMMON_COLUMNMETADATA_H

#include "Common/Module.h"
#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"
#include "Common/SQLData.h"
#include "Common/DataType.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::ColumnMetaData
//
//	NOTES
//	カラムのメタデータをあらわすクラス
//	このColumnMetaDataの集合がResultSetMetaDataとなる
//
class SYD_COMMON_FUNCTION ColumnMetaData
	: public ExecutableObject, public Externalizable
{
public:
	//
	//	ENUM
	//	Common::ColumnMetaData::Type::Value -- カラムの型を表す列挙子
	//
	//	NOTES
	//	Common::DataTypeはSQLの型とあっていない。
	//	この型はアプリケーションにそのまま渡るので、
	//	追加する場合は、必ず最後に追加するとこ。
	//
	struct Type
	{
		enum Value
		{
			Unknown = 0,
			
			// Charactor String Type
			Charactor,
			CharactorVarying,
			NationalCharactor,
			NationalCharactorVarying,

			// Binary String Type
			Binary,
			BinaryVarying,

			// Large Object Type
			CharactorLargeObject,
			NationalCharactorLargeObject,
			BinaryLargeObject,

			// Integer Numeric Type
			Numeric,
			SmallInt,
			Integer,
			BigInt,

			// Decimal Numeric Type
			Decimal,
			Float,
			Real,
			DoublePrecision,

			// Boolean Type
			Boolean,

			// Time Type
			Date,
			Time,
			Timestamp,

			// Other Type (TRMeister独自の型)
			Language,
			Word,

			ValueNum
		};
	};

	// コンストラクタ
	ColumnMetaData();
	// コンストラクタ
	ColumnMetaData(const SQLData& cSQLData_);
	// デストラクタ
	virtual ~ColumnMetaData();

	// SQLDataを設定する
	void setSQLData(const SQLData& cSQLData_);

	// SQL標準のデータ型
	Type::Value getType() const { return m_eType; }
	void setType(Type::Value eType_) { m_eType = eType_; }

	// Commonのデータ型を得る
	DataType::Type getDataType() const;

	// TRMeister固有のデータ型名
	const ModUnicodeString& getTypeName() const { return m_cTypeName; }
	void setTypeName(const ModUnicodeString& cTypeName_)
		{ m_cTypeName = cTypeName_; }

	// カラム名
	const ModUnicodeString& getColumnName() const
		{ return m_cColumnName; }
	void setColumnName(const ModUnicodeString& cColumnName_)
		{ m_cColumnName = cColumnName_; }

	// テーブル名
	const ModUnicodeString& getTableName() const
		{ return m_cTableName; }
	void setTableName(const ModUnicodeString& cTableName_)
		{ m_cTableName = cTableName_; }

	// データベース名
	const ModUnicodeString& getDatabaseName() const
		{ return m_cDatabaseName; }
	void setDatabaseName(const ModUnicodeString& cDatabaseName_)
		{ m_cDatabaseName = cDatabaseName_; }

	// カラム別名
	const ModUnicodeString& getColumnAliasName() const
		{ return m_cColumnAliasName; }
	void setColumnAliasName(const ModUnicodeString& cColumnAliasName_)
		{ m_cColumnAliasName = cColumnAliasName_; }

	// テーブル別名
	const ModUnicodeString& getTableAliasName() const
		{ return m_cTableAliasName; }
	void setTableAliasName(const ModUnicodeString& cTableAliasName_)
		{ m_cTableAliasName = cTableAliasName_; }

	// 最大表示サイズ(文字数)
	int getDisplaySize() const { return m_iDisplaySize; }
	void setDisplaySize(int iDisplaySize_) { m_iDisplaySize = iDisplaySize_; }

	// 10進桁数
	int getPrecision() const { return m_iPrecision; }
	void setPrecision(int iPrecision_) { m_iPrecision = iPrecision_; }

	// 小数点以下桁数
	int getScale() const { return m_iScale; }
	void setScale(int iScale_) { m_iScale = iScale_; }

	// 配列要素数
	int getMaxCardinality() const { return m_iMaxCardinality; }
	void setMaxCardinality(int iMaxCardinality_);

	// 自動採番かどうか
	bool isAutoIncrement() const { return m_iFlag & Flag::AutoIncrement; }
	void setAutoIncrement(bool bAutoIncrement_ = true);

	// 大文字小文字が区別されるないかどうか
	bool isCaseInsensitive() const { return m_iFlag & Flag::CaseInsensitive; }
	void setCaseInsensitive(bool bCaseInsensitive_ = true);

	// 符号なしかどうか
	bool isUnsigned() const { return m_iFlag & Flag::Unsigned; }
	void setUnsigned(bool bUnsigned_ = true);

	// 検索不可かどうか
	bool isNotSearchable() const { return m_iFlag & Flag::NotSearchable; }
	void setNotSearchable(bool bNotSearchable_ = true);

	// 読み込み専用かどうか
	bool isReadOnly() const { return m_iFlag & Flag::ReadOnly; }
	void setReadOnly(bool bReadOnly_ = true);

	// NULLをセットできないかどうか
	bool isNotNullable() const { return m_iFlag & Flag::NotNull; }
	void setNotNullable(bool bNotNullable_ = true);

	// ユニークかどうか
	bool isUnique() const { return m_iFlag & Flag::Unique; }
	void setUnique(bool bUnique_ = true);

	// 配列かどうか
	bool isArray() const { return (m_iMaxCardinality != 0); }

	// クラスIDを得る
	int getClassID() const;
	// シリアル化
	void serialize(ModArchive& archiver_);

	// 文字列で表す
	ModUnicodeString toString() const;

private:
	//
	//	ENUM
	//	Common::ColumnMetaData::Flag::Value -- 属性を表す列挙子
	//
	struct Flag
	{
		typedef int Value;
		
		enum
		{
			None			= 0,

			AutoIncrement	= (1 << 0),		// 自動採番
			CaseInsensitive	= (1 << 1),		// 大文字小文字が区別されない
			Unsigned		= (1 << 2),		// 符号なし
			NotSearchable	= (1 << 3),		// 検索不可
			ReadOnly		= (1 << 4),		// 読み出し専用
			NotNull			= (1 << 5),		// NULL不許可
			Unique			= (1 << 6)		// ユニーク
		};
	};
	
	// SQL標準のデータ型
	Type::Value m_eType;
	
	// TRMeister固有のデータ型名
	ModUnicodeString m_cTypeName;
	// カラム名
	ModUnicodeString m_cColumnName;
	// テーブル名
	ModUnicodeString m_cTableName;
	// データベース名
	ModUnicodeString m_cDatabaseName;
	// カラム別名
	ModUnicodeString m_cColumnAliasName;
	// テーブル別名
	ModUnicodeString m_cTableAliasName;

	// 最大表示サイズ
	int m_iDisplaySize;
	// 10進桁数
	int m_iPrecision;
	// 小数点以下桁数
	int m_iScale;
	// 配列要素数
	int m_iMaxCardinality;

	// 属性
	Flag::Value m_iFlag;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_COLUMNMETADATA_H

//
//	Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
