// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.h -- 論理ログデータ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SCHEMA_LOGDATA_H
#define __SYDNEY_SCHEMA_LOGDATA_H

#include "Schema/Module.h"
#include "Schema/Externalizable.h"
#include "Schema/ObjectID.h"

#include "Common/DataArrayData.h"
#include "Trans/LogData.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Transaction
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::LogData -- スキーマ内で使用する論理ログ
//						（ログはトランザクション内でしか書き込めない）
//
//	NOTES
//		このクラスはほとんどエクスポート部分だけなのでクラスごとエクスポートする

class SYD_SCHEMA_FUNCTION LogData
	: public	Trans::Log::ModificationData,	//【注意】	最初に継承すること
	  public	Externalizable
{
public:
	//
	//	TYPEDEF
	//		Schema::LogData::AutoLogData -- 論理ログデータを扱う為の auto-pointer
	//
	//	NOTES
	typedef		ModAutoPointer< Schema::LogData >		AutoLogData;

	//
	//	ENUM
	//		Schema::LogData::Category
	//
	//	NOTES
	//		スキーマ内で扱う論理ログの種別をあらわすクラス
	//
	class Category
	{
	public:
		//
		//	ENUM
		//		Schema::LogData::Category::Value
		//
		//	NOTES
		//		スキーマ内で扱う論理ログの種別をあらわす値の列挙
		//
		enum Value
		{
			Unknown					=	 0,		// 不明

			//--- Mount 操作	---
			Mount					=	10,		// MOUNT
			Unmount,							// UNMOUNT

			//--- Backup 操作	---
			StartBackup,						// START	BACKUP
			EndBackup,							// End		BACKUP

			//--- TimeStamp		---
			TimeStamp				=	30,		// TimeStamp

			//--- Database 操作 ---
			CreateDatabase			=	40,
			AlterDatabase,
			AlterDatabase_ReadOnly,
			MoveDatabase,
			DropDatabase,
			AlterDatabase_SetToMaster,

			//--- Area 操作 ---
			CreateArea				=	60,
			AlterArea,
			DropArea,

			//--- Table 操作 ---
			CreateTable				=	70,
			AlterTable,
			DropTable,
			RenameTable,

			//--- Index 操作 ---
			CreateIndex				=	80,
			AlterIndex,
			DropIndex,
			RenameIndex,

			//--- Column 操作 ---
			AddColumn				=	90,
			AlterColumn,
			DropColumn,

			//--- Constraint 操作 ---
			AddConstraint			=	100,
			DropConstraint,

			//--- Privilege operations ---
			CreatePrivilege			=	110, // GRANT (new privilege)
			DropPrivilege,					 // REVOKE (no privilege)
			AlterPrivilege,					 // GRANT/REVOKE

			//--- Cascade operations ---
			CreateCascade			=	120,
			DropCascade,
			AlterCascade,

			//--- Partition operations ---
			CreatePartition			=	130,
			DropPartition,
			AlterPartition,

			//--- Function operations ---
			CreateFunction			=	140,
			DropFunction,

			NumValue
		};
	};

	LogData(const Schema::LogData::Category::Value eSubCategory_
					= Schema::LogData::Category::Unknown);
	LogData(const Schema::LogData::Category::Value eSubCategory_,
			const Common::DataArrayData& cData_);
												// コンストラクタ

	virtual ~LogData();							// デストラクタ

	void					serialize(ModArchive& archiver_);
												// このクラスをシリアル化する

	int						getClassID() const;	// このクラスのクラス ID を得る

	int						getSubCategory() const;
												// このクラスのサブクラス ID を得る
	void					setSubCategory(Category::Value category);
												// このクラスのサブクラス ID を設定する
	void					addData(const Common::Data::Pointer& pcData_);
												// データを追加する

	Trans::Log::LSN			storeLog(Trans::Transaction& cTrans_,
									 const Trans::Log::File::Category::Value& eCategory_);
												// 論理ログにデータを書き込む

#ifndef SYD_COVERAGE
	ModUnicodeString		toString() const;	// ログ内容を文字列に変換する
#endif

	int						getCount() const;	// ログ内容の件数を取得する

	Common::Data::Pointer
							operator[](const int iIdx_);
	const Common::Data::Pointer
							operator[](int iIdx_) const;
												// データアクセサ

	//--- 以下、論理ログ出力関数(static) ---

#ifdef OBSOLETE // SubCategoryをstore時に指定する機能は使用しない
	static Trans::Log::LSN		storeLog(
									Trans::Transaction& cTrans_,
									const Schema::LogData::Category::Value& eSubCategory_,
									const Trans::Log::File::Category::Value& eCategory_);
	static Trans::Log::LSN		storeLog(
									Trans::Transaction& cTrans_,
									const Schema::LogData::Category::Value& eSubCategory_,
									const Common::DataArrayData& cData_,
									const Trans::Log::File::Category::Value& eCategory_);
#endif
	static Trans::Log::LSN		storeLog(
									Trans::Transaction& cTrans_,
									LogData& cLogData_,
									const Trans::Log::File::Category::Value& eCategory_);
#ifdef OBSOLETE // SubCategoryをstore時に指定する機能は使用しない
	static Trans::Log::LSN		storeLog(
									Trans::Transaction& cTrans_,
									const Schema::LogData::Category::Value& eSubCategory_,
									const Schema::ObjectID& nID_,
									const Trans::Log::File::Category::Value& eCategory_);
	static Trans::Log::LSN		storeLog(
									Trans::Transaction& cTrans_,
									const Schema::LogData::Category::Value& eSubCategory_,
									const Schema::ObjectID& nDbID_,
									const Schema::ObjectID& nID_,
									const Trans::Log::File::Category::Value& eCategory_);
#endif
												// ログを論理ログに書く

	//--- 以下、ログデータ作成および値取得用関数 ---
	// IDの追加
	void						addID(ObjectID::Value iID_);
	// ID配列の追加
	void						addIDs(const ModVector<ObjectID::Value>& vecID_);
	// 文字列の追加
	void						addString(const ModUnicodeString& cString_);
	// 文字列配列の追加
	void						addStrings(const ModVector<ModUnicodeString>& vecString_);
	// unsigned intの追加
	void						addUnsignedInteger(unsigned int iValue_);
	// unsigned int配列の追加
	void						addUnsignedIntegers(const ModVector<unsigned int>& vecValue_);
	// intの追加
	void						addInteger(int iValue_);
	// MotUInt64 の追加
	void						addUnsignedInteger64(ModUInt64 v);
	// NullDataの追加
	void						addNull();
	// IDの取得
	ObjectID::Value				getID(int iIndex_) const;
	// ID配列の取得
	const ModVector<ObjectID::Value>&
								getIDs(int iIndex_) const;
	// 文字列の取得
	const ModUnicodeString&		getString(int iIndex_) const;
	// 文字列配列の取得
	const ModVector<ModUnicodeString>&
								getStrings(int iIndex_) const;
	// unsigned intの取得
	unsigned int				getUnsignedInteger(int iIndex_) const;
	// unsigned int配列の取得
	const ModVector<unsigned int>&
								getUnsignedIntegers(int iIndex_) const;
	// intの取得
	int							getInteger(int iIndex_) const;
	// ModUInt64 の取得
	ModUInt64					getUnsignedInteger64(int index) const;

	// DataArrayDataの取得
	const Common::DataArrayData& getDataArrayData(int iIndex_) const;

	// IDの変換
	static Common::Data::Pointer createID(ObjectID::Value iID_);
	// ID配列の変換
	static Common::Data::Pointer createIDs(const ModVector<ObjectID::Value>& vecID_);
	// 文字列の変換
	static Common::Data::Pointer createString(const ModUnicodeString& cString_);
	// 文字列配列の変換
	static Common::Data::Pointer createStrings(const ModVector<ModUnicodeString>& vecString_);
	// unsigned intの変換
	static Common::Data::Pointer createUnsignedInteger(unsigned int iValue_);
	// unsigned int配列の変換
	static Common::Data::Pointer createUnsignedIntegers(const ModVector<unsigned int>& vecValue_);
	// intの変換
	static Common::Data::Pointer createInteger(int iValue_);
	// ModUInt64 の変換
	static Common::Data::Pointer createUnsignedInteger64(ModUInt64 v);
	// NullDataの変換
	static Common::Data::Pointer createNull();

	// IDの取得
	static ObjectID::Value		getID(const Common::Data::Pointer& pElement_);
	// ID配列の取得
	static const ModVector<ObjectID::Value>&
								getIDs(const Common::Data::Pointer& pElement_);
	// 文字列の取得
	static const ModUnicodeString&	getString(const Common::Data::Pointer& pElement_);
	// 文字列配列の取得
	static const ModVector<ModUnicodeString>&
								getStrings(const Common::Data::Pointer& pElement_);
	// unsigned intの取得
	static unsigned int			getUnsignedInteger(const Common::Data::Pointer& pElement_);
	// unsigned int配列の取得
	static const ModVector<unsigned int>&
								getUnsignedIntegers(const Common::Data::Pointer& pElement_);
	// intの取得
	static int					getInteger(const Common::Data::Pointer& pElement_);
	// ModUInt64 の取得
	static ModUInt64			getUnsignedInteger64(const Common::Data::Pointer& elm);

	// DataArrayDataの取得
	static const Common::DataArrayData&
								getDataArrayData(const Common::Data::Pointer& pElement_);

private:
	Category::Value			m_eSubCategory;		// Schema 内でのカテゴリー

	Common::DataArrayData	m_cDataArray;		// ログ書込み用データ

};

//
//	FUNCTION public
//	Schema::LogData::~LogData -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
inline
LogData::
~LogData()
{
}

//
//	FUNCTION public
//	Recovery::LogData::operator[]
//		-- データアクセサ
//
//	NOTES
//
//	ARGUMENTS
//		int iIdx_
//			配列へのインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
inline
Common::Data::Pointer
LogData::
operator[](int iIdx_)
{
	return m_cDataArray.getElement(iIdx_);
}

inline
const Common::Data::Pointer
LogData::
operator[](int iIdx_) const
{
	return m_cDataArray.getElement(iIdx_);
}

//
//	FUNCTION public
//	Recovery::LogData::getCount
//		-- ログデータの件数を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		件数
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

inline
int
LogData::
getCount() const
{
	return static_cast<int>(m_cDataArray.getCount());
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_LOGDATA_H

//
//	Copyright (c) 2000, 2001, 2002, 2005, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
