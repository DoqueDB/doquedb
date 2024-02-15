// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLData.h -- SQLデータ型
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_SQLDATA_H
#define __TRMEISTER_COMMON_SQLDATA_H

#include "Common/Module.h"
#include "Common/Object.h"
#include "Common/Externalizable.h"
#include "Common/Collation.h"
#include "Common/Data.h"
#include "Common/DataType.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	SQLData -- SQLデータ型
//
//	NOTES
//	SQL文に出現するデータ型をそのまま記述したもの。
//	メタデータマネージャーやエグゼキューターで扱うデータ型とは
//	独立した別の概念であるので注意が必要。
//
class SYD_COMMON_FUNCTION SQLData
	: public Object, public Externalizable
{
public:
	//	ENUM
	//  Common::SQLData::Type::Value -- SQLのデータ型
	//
	//  NOTES
	//	スキーマ情報として永続化されているので
	//	ここのエントリーの値を変更してはいけない

	struct Type
	{
		enum Value
		{
			// 指定なし
			NoType = 0,

			// ASCII 固定、可変長文字列
			Char,

			// UNICODE 固定、可変長文字列
			NChar,

			// 整数
			Int,

			// 浮動小数点数
			Float,

			// [SQL Server] 日付と時刻
			DateTime,

			// [SQL Server] GUID
			//
			//【注意】	TRMeister では NChar(36) と同義
			UniqueIdentifier,

			// [SQL Server] 固定、可変長バイナリ
			Binary,

			// [SQL Server] 可変長バイナリ
			//
			//【注意】	SQL Server では SQL99 での BLOB の意味となるが、
			//			TRMeister では Binary と同じ扱いとなる
			Image,

			// [SQL Server] 可変長 UNICODE 文字列
			//
			//【注意】	SQL Server では SQL99 での NCLOB の意味となるが、
			//			TRMeister では NChar と同じ扱いとなる
			NText,

			// [TRMeister]	全文文字列
			//
			//【注意】	TRMeister では NText と同義
			Fulltext,

			// [TRMeister] 言語指定
			Language,

			// 長大可変長バイナリ
			BLOB,

			// 長大 ASCII 可変長文字列
			//
			//【注意】	未実装、予約のため定義しておく
			CLOB,

			// 長大 UNICODE 可変長文字列
			NCLOB,

			// 固定小数点型
			Decimal,

			// 日付型
			Date,

			// 時刻型
			Time,

			// 日時型
			Timestamp,

			// 符号なし整数
			UInt,

			// 全文索引の単語データ
			Word,

			// 倍精度整数
			BigInt,

			ValueNum
		};
	};

	//  ENUM public
	//  Common::SQLDataType::Flag::Value -- データ型の性質を表すenum型
	//
	//  NOTES

	struct Flag
	{
		enum Value
		{
			None = 0,
			OldFixed,					// v14以前の誤った実装によるFixed
			OldVariable,				// v14以前の誤った実装によるVariable
			Unlimited,
			Fixed,						// v15以降の規格に沿ったFixed
			Variable,					// v15以降の規格に沿ったVariable
			ValueNum
		};
	};

	// コンストラクタ (1)
	SQLData();
	// コンストラクタ (2)
	SQLData(Type::Value eType_,
			Flag::Value eFlag_,
			int iLength_,
			int iScale_,
			Collation::Type::Value eCollation_ = Collation::Type::Implicit);
	// コンストラクタ (3)
	SQLData(Type::Value eType_,
			Flag::Value eFlag_,
			int iLength_,
			int iScale_,
			int iMaxCardinality_,
			Collation::Type::Value eCollation_ = Collation::Type::Implicit);
	// 代入オペレーター
	SQLData& operator=(const SQLData& cOther_);
	// 比較演算子
	bool operator<(const SQLData& cOther_) const;

	// 初期化する
	void clear();

	// SQL文の形式で得る
	ModUnicodeString toSQLStatement() const;

	// アクセサ
	
	// Type を得る
	Type::Value getType() const;
	// Type を設定する
	void setType(Type::Value eType_);
	
	// Length を得る
	int getLength() const;
	// Length を設定する
	void setLength(int iLength_);
	
	// Scale を得る
	int getScale() const;
	// Scale を設定する
	void setScale(int iScale_);

	// Flag を得る
	Flag::Value getFlag() const;
	// Flag を設定する
	void setFlag(Flag::Value iFlag_);

	// Array を得る
	int getMaxCardinality() const;
	// Array を設定する
	void setMaxCardinality(int iCardinality_);

	// Arrayか否かを得る
	bool isArrayType() const;

	// Collation を得る
	Collation::Type::Value getCollation() const;
	// Collation を設定する
	void setCollation(Collation::Type::Value eCollation_);

	// クラスIDを得る
	int getClassID() const;
	// シリアル化する
	void serialize(ModArchive& archiver_);

	//////////////////////////////////////////////
	// 配列型のとき要素の型を得る
	SQLData getElementType() const;

	// この型にキャストする
	bool cast(const SQLData& cSourceType_,
			  const Data::Pointer& pData_,
			  const Data::Pointer& pTarget_,
			  bool bForComparison_ = false,
			  bool bNoThrow_ = false) const;

	// 上位互換性のある型を作る
	static bool getCompatibleType(const SQLData& cType1_, const SQLData& cType2_,
								  SQLData& cResult_, bool bForComparison_ = false, Common::DataOperation::Type eOperation = Common::DataOperation::Unknown);
	
	static bool getSQLDataTypeOwnIfDecimal(const SQLData& cTypeOwn_, const SQLData& cTypeDecimal_, SQLData& cResultOwn_);

	// 型に互換性があるかを得る
	static bool isCompatibleType(const SQLData& cType1_, const SQLData& cType2_);

	// castなしに代入可能かを得る
	static bool isAssignable(const SQLData& cType1_, const SQLData& cType2_, Common::DataOperation::Type eOperation_ = Common::DataOperation::Unknown);
	// castなしに比較可能かを得る
	static bool isComparable(const SQLData& cType1_, const SQLData& cType2_);

	// Get maximum value of precision
	static int getMaxPrecision(SQLData::Type::Value eType_);

	static bool checkNumericString(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, bool bNoThrow_);

	// Get target length of string
	bool getCastStringLength(int& nDst_, const SQLData& cSourceType_, const Common::Data& cData_,
							 bool bForComparison_ = false, bool bNoThrow_ = false,
							 bool bTruncateIsNotAllowed_ = false) const;

	// create SQLType object from data type
	static SQLData create(DataType::Type eType_);

private:
	// castの下請け
	bool castScalar(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					bool bForComparison_, bool bNoThrow_) const;
	bool castArray(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
				   bool bForComparison_, bool bNoThrow_) const;

	// 実際にcastするために使用する関数の型
	typedef bool (SQLData::* CastMethod)(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
										 bool bForComparison_, bool bNoThrow_) const;
	// 実際にcastするために使用する関数
	bool castNormal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					bool bForComparison_, bool bNoThrow_) const;
	bool castCannot(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					bool bForComparison_, bool bNoThrow_) const;
	bool castStringString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						  bool bForComparison_, bool bNoThrow_) const;
	bool castStringUnicodeString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
								 bool bForComparison_, bool bNoThrow_) const;
	bool castNumericString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						   bool bForComparison_, bool bNoThrow_) const;

	bool castLanguageString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
							bool bForComparison_, bool bNoThrow_) const;
	bool castBinaryBinary(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
						  bool bForComparison_, bool bNoThrow_) const;
	bool castToString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					  bool bForComparison_, bool bNoThrow_) const;
	bool castToUnicodeString(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
							 bool bForComparison_, bool bNoThrow_) const;
	bool castToBinary(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
					  bool bForComparison_, bool bNoThrow_) const;

	// castStringStringの下請け
	bool castStringString_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
								   bool bForComparison_, bool bNoThrow_,
								   bool bNoTruncate_ = false) const;
	// castStringUnicodeStringの下請け
	bool castStringUnicodeString_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
										  bool bForComparison_, bool bNoThrow_,
										  bool bNoTruncate_ = false) const;
	// castBinaryBinaryの下請け
	bool castBinaryBinary_Internal(const SQLData& cSourceType_, const Data::Pointer& pData_, const Data::Pointer& pTarget_,
								   bool bForComparison_, bool bNoThrow_,
								   bool bNoTruncate_ = false) const;

	Type::Value m_eType;
	Flag::Value m_eFlag;
	int m_iLength;
	int m_iScale;
	int m_iCardinality;

	Collation::Type::Value m_eCollation;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

// operator<< to output column type to messages
ModMessageStream& operator<<(ModMessageStream& cStream_, const _TRMEISTER::Common::SQLData& cType_);
ModOstream& operator<<(ModOstream& cStream_, const _TRMEISTER::Common::SQLData& cType_);

#endif // __TRMEISTER_COMMON_SQLDATA_H

//
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
