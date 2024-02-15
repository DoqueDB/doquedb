// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.cpp --
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Schema/Database.h"
#include "Schema/LogData.h"


#include "Common/IntegerData.h"
#include "Common/NullData.h"
#include "Common/StringArrayData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Exception/LogItemCorrupted.h"
#include "Trans/Transaction.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{

namespace _LogData
{
	// UNDO 可能な操作か
	bool					isUndoable(LogData::Category::Value category);
}

//	FUNCTION
//	$$$::_LogData::isUndoable -- UNDO 可能な操作か
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData::Category::Value	category
//			UNDO 可能か調べる操作
//
//	RETURN
//		true
//			UNDO 可能である
//		false
//			UNDO 不可である
//
//	EXCEPTIONS
//		なし

bool
_LogData::isUndoable(LogData::Category::Value category)
{
	switch (category) {
	case LogData::Category::DropDatabase:
	case LogData::Category::DropTable:
	case LogData::Category::DropIndex:
		return false;
	}
	return true;
}

// 出力演算子
//ModMessageStream& operator<<(ModMessageStream& cStream_,
//							 _SYDNEY::Lock::Mode::Value eValue_);

} // namespace $$

//
//	FUNCTION public
//	Schema::LogData::LogData -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//		const Schema::LogData::Category::Value eSubCategory
//			Schema 内種別
//			（デフォルト：Schema::LogData::Category::Unknown）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

LogData::
LogData(const Schema::LogData::Category::Value eSubCategory_)
	: Trans::Log::ModificationData(Trans::Log::Data::Category::SchemaModify,
								   _LogData::isUndoable(eSubCategory_)),
	  m_eSubCategory(eSubCategory_)
{}

//
//	FUNCTION public
//	Schema::LogData::LogData -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//		const Schema::LogData::Category::Value eSubCategory
//			Schema 内種別
//		const Common::DataArrayData& cData_
//			書き込みデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

LogData::
LogData(const Schema::LogData::Category::Value eSubCategory_,
		const Common::DataArrayData& cData_)
	: Trans::Log::ModificationData(Trans::Log::Data::Category::SchemaModify,
								   _LogData::isUndoable(eSubCategory_)),
	  m_eSubCategory(eSubCategory_), m_cDataArray()
{
	m_cDataArray = cData_;				// ポインターのコピー
}

//
//	FUNCTION public
//	Schema::LogData::serialize
//		-- このクラスをシリアライズする
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive& archiver_
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
void
LogData::
serialize(ModArchive& archiver_)
{
	ModificationData::serialize(archiver_);
	if (archiver_.isStore())
	{
		// 書き込み

		int iTmp = static_cast<int>(m_eSubCategory);
		archiver_ << iTmp;

		m_cDataArray.serialize(archiver_);
	}
	else
	{
		// 読込み
		int iTmp;
		archiver_ >> iTmp;
		m_eSubCategory = static_cast<Category::Value>(iTmp);

		m_cDataArray.serialize(archiver_);
	}
}

//
//	FUNCTION public
//	Schema::LogData::getClassID
//		-- このクラスのクラス ID を得る
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
//
int
LogData::
getClassID() const
{
	return Schema::Externalizable::Category::LogData +
				Common::Externalizable::SchemaClasses;
}

//
//	FUNCTION public static
//	Schema::LogData::getSubCategory
//		-- このクラスのサブカテゴリーを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
int
LogData::
getSubCategory() const
{
	return m_eSubCategory;
}

//
//	FUNCTION public static
//	Schema::LogData::setSubCategory
//		-- このクラスのサブカテゴリーを設定する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
void
LogData::
setSubCategory(Category::Value eCategory)
{
	m_eSubCategory = eCategory;
}

//
//	FUNCTION public static
//	Schema::LogData::addData
//		-- データを追加する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& cData
//			書き込むデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
void
LogData::
addData(const Common::Data::Pointer& pcData_)
{
	m_cDataArray.pushBack(pcData_);
}

//
//	FUNCTION public static
//	Schema::LogData::storeLog
//		-- 論理ログにデータを書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//		const Trans::Log::File::Category::Value& eCategory_
//			論理ログ書き込み対象識別子
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし
//
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	return cTrans_.storeLog(eCategory_, *this);
}

#ifndef SYD_COVERAGE
//	FUNCTION public
//	Schema::LogData::toString -- オブジェクトを表す文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたユニコード文字列
//
//	EXCEPTIONS

ModUnicodeString
LogData::toString() const
{
	ModUnicodeOstrStream	buf;
	buf << moduleName << "::LogData {" << getTimeStamp() << ",";

	switch (m_eSubCategory) {
	case Category::Mount:				buf << "Mount"; 				break;
	case Category::Unmount:				buf << "Unmount";				break;

	case Category::StartBackup:			buf << "StartBackup";			break;
	case Category::EndBackup:			buf << "EndBackup";				break;

	case Category::TimeStamp:			buf << "TimeStamp";				break;

	case Category::CreateDatabase:		buf << "CreateDatabase";		break;
	case Category::AlterDatabase:		buf << "AlterDatabase"; 		break;
	case Category::AlterDatabase_ReadOnly:
									buf << "AlterDatabase_ReadOnly";	break;
	case Category::MoveDatabase:		buf << "MoveDatabase";			break;
	case Category::DropDatabase:		buf << "DropDatabase";			break;
	case Category::AlterDatabase_SetToMaster:
									buf << "AlterDatabase_SetToMaster";	break;

	case Category::CreateArea:			buf << "CreateArea";			break;
	case Category::AlterArea:			buf << "AlterArea";				break;
	case Category::DropArea:			buf << "DropArea";				break;

	case Category::CreateTable:			buf << "CreateTable";			break;
	case Category::AlterTable:			buf << "AlterTable";			break;
	case Category::DropTable:			buf << "DropTable";				break;
	case Category::RenameTable:			buf << "RenameTable";			break;

	case Category::CreateIndex:			buf << "CreateIndex";			break;
	case Category::AlterIndex:			buf << "AlterIndex";			break;
	case Category::DropIndex:			buf << "DropIndex";				break;
	case Category::RenameIndex:			buf << "RenameIndex";			break;

	case Category::AddColumn:			buf << "AddColumn";				break;
	case Category::AlterColumn:			buf << "AlterColumn";			break;
	case Category::DropColumn:			buf << "DropColumn";			break;

	case Category::AddConstraint:		buf << "AddConstraint";			break;
	case Category::DropConstraint:		buf << "DropConstraint";		break;

	case Category::CreatePrivilege:		buf << "CreatePrivilege";		break;
	case Category::DropPrivilege:		buf << "DropPrivilege";			break;
	case Category::AlterPrivilege:		buf << "AlterPrivilege";		break;

	case Category::CreateCascade:		buf << "CreateCascade";			break;
	case Category::DropCascade:			buf << "DropCascade";			break;
	case Category::AlterCascade:		buf << "AlterCascade";			break;

	case Category::CreatePartition:		buf << "CreatePartition";		break;
	case Category::AlterPartition:		buf << "AlterPartition";		break;
	case Category::DropPartition:		buf << "DropPartition";			break;

	case Category::CreateFunction:		buf << "CreateFunction";		break;
	case Category::DropFunction:		buf << "DropFunction";			break;

	default:
	case Category::Unknown:				buf << "Unknown";				break;
	}

	buf << ",{" << m_cDataArray.getElement(0)->toString();

	const int n = m_cDataArray.getCount();
	for (int i = 1; i < n; ++i)
		buf << "," << m_cDataArray.getElement(i)->toString();

	buf << "}}";

	return buf.getString();
}
#endif

//--- 以下、論理ログ出力関数(static) ---

#ifdef OBSOLETE // SubCategoryをstore時に指定する機能は使用しない
//
//	FUNCTION public static
//	Schema::LogData::storeLog
//		-- ログを論理ログに書く
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			書き込み対象トランザクション記述子
//
//		const Schema::LogData::Category::Value& eSubCategory_
//			Schema::LogData の種別
//
//		const Trans::Log::File::Category::Value
//			ログ種別
//			（デフォルト：Trans::Log::File::Category::System）
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし
//

// staitc
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 const Schema::LogData::Category::Value& eSubCategory_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	return LogData(eSubCategory_).storeLog(cTrans_, eCategory_);
}

//
//	FUNCTION public static
//	Schema::LogData::storeLog
//		-- ログを論理ログに書く
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			書き込み対象トランザクション記述子
//
//		const Schema::LogData::Category::Value& eSubCategory_
//			Schema::LogData の種別
//
//		const Common::DataArrayData& cData_
//			書き込むデータ
//
//		const Trans::Log::File::Category::Value
//			ログ種別
//			（デフォルト：Trans::Log::File::Category::System）
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし
//
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 const Schema::LogData::Category::Value& eSubCategory_,
		 const Common::DataArrayData& cData_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	return LogData(eSubCategory_, cData_).storeLog(cTrans_, eCategory_);
}
#endif

//
//	FUNCTION public static
//	Schema::LogData::storeLog
//		-- ログを論理ログに書く
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			書き込み対象トランザクション記述子
//
//		const LogData& cLogData_
//			書き込みログ
//
//		const Trans::Log::File::Category::Value
//			ログ種別
//			（デフォルト：Trans::Log::File::Category::System）
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし
//

// static
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 LogData& cLogData_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	return cLogData_.storeLog(cTrans_, eCategory_);
}


#ifdef OBSOLETE // SubCategoryをstore時に指定する機能は使用しない
//
//	FUNCTION public
//	Schema::LogData::
//		-- ログを論理ログに書く
//
//	NOTES
//		スキーマオブジェクトを書く
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//		const Schema::LogData::Category::Value& eSubCategory_
//			Schema::LogData の種別
//
//		const Object::ID nID_
//			Schema::Object::ID
//
//		const Trans::Log::File::Category::Value& eCategory_
//			ログ種別
//			（デフォルト：Trans::Log::File::Category::System）
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし
//

// static
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 const Schema::LogData::Category::Value& eSubCategory_,
		 const Schema::ObjectID& nID_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	LogData cLog(eSubCategory_);
	cLog.addID(nID_);

	return cLog.storeLog(cTrans_, eCategory_);
}

//
//	FUNCTION public
//	Schema::LogData::
//		-- ログを論理ログに書く
//
//	NOTES
//		スキーマオブジェクトを２つ書く
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//		const Schema::LogData::Category::Value& eSubCategory_
//			サブカテゴリー
//
//		const Schema::ObjectID& nDbID_
//			Database の ID
//
//		const Schema::ObjectID& nID_
//			スキーマオブジェクトの ID
//
//		const Trans::Log::File::Category::Value& eCategory_
//			ログ種別
//			(デフォルト：Trans::Log::File::Category::System)
//
//	RETURN
//		ストアしたログのLSN
//
//	EXCEPTIONS
//		なし

// static
Trans::Log::LSN
LogData::
storeLog(Trans::Transaction& cTrans_,
		 const Schema::LogData::Category::Value& eSubCategory_,
		 const Schema::ObjectID& nDbID_,
		 const Schema::ObjectID& nID_,
		 const Trans::Log::File::Category::Value& eCategory_)
{
	LogData cLog(eSubCategory_);
	cLog.addID(nDbID_);
	cLog.addID(nID_);

	return cLog.storeLog(cTrans_, eCategory_);
}
#endif

//	FUNCTION public
//	Schema::LogData::addID -- IDの追加
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value iID_
//			追加するID
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addID(ObjectID::Value iID_)
{
	addData(createID(iID_));
}

//	FUNCTION public
//	Schema::LogData::addIDs -- ID配列の追加
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ObjectID::Value>& vecID_
//			追加するID配列
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addIDs(const ModVector<ObjectID::Value>& vecID_)
{
	addData(createIDs(vecID_));
}

//	FUNCTION public
//	Schema::LogData::addString -- 文字列の追加
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& cString_
//			追加する文字列
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addString(const ModUnicodeString& cString_)
{
	addData(createString(cString_));
}

//	FUNCTION public
//	Schema::LogData::addStrings -- 文字列配列の追加
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>& vecString_
//			追加する文字列配列
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addStrings(const ModVector<ModUnicodeString>& vecString_)
{
	addData(createStrings(vecString_));
}

//	FUNCTION public
//	Schema::LogData::addUnsignedInteger -- unsigned intの追加
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int iValue_
//			追加するunsigned int
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addUnsignedInteger(unsigned int iValue_)
{
	addData(createUnsignedInteger(iValue_));
}

// FUNCTION public
//	Schema::LogData::addUnsignedIntegers -- unsigned int配列の追加
//
// NOTES
//
// ARGUMENTS
//	const ModVector<unsigned int>& vecValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LogData::
addUnsignedIntegers(const ModVector<unsigned int>& vecValue_)
{
	addData(createUnsignedIntegers(vecValue_));
}

//	FUNCTION public
//	Schema::LogData::addInteger -- intの追加
//
//	NOTES
//
//	ARGUMENTS
//		int iValue_
//			追加するint
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addInteger(int iValue_)
{
	addData(createInteger(iValue_));
}

//	FUNCTION public
//	Schema::LogData::addUnsignedInteger64 -- ModUInt64 の追加
//
//	NOTES
//
//	ARGUMENTS
//		ModUInt64	v
//			追加する ModUInt64
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
LogData::
addUnsignedInteger64(ModUInt64 v)
{
	addData(createUnsignedInteger64(v));
}

//	FUNCTION public
//	Schema::LogData::addNull -- NullDataの追加
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION
void
LogData::
addNull()
{
	addData(createNull());
}

//	FUNCTION public
//	Schema::LogData::getID -- IDの取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			IDが記録されているはずのログデータ上の位置
//
//	RETURN
//		IDの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にIDが記録されていない

ObjectID::Value
LogData::
getID(int iIndex_) const
{
	return getUnsignedInteger(iIndex_);
}

//	FUNCTION public
//	Schema::LogData::getIDs -- 文字列配列の取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			ID配列が記録されているはずのログデータ上の位置
//
//	RETURN
//		const ModVector<ObjectID::Value>&
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にID配列が記録されていない

const ModVector<ObjectID::Value>&
LogData::
getIDs(int iIndex_) const
{
	return getIDs((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::getString -- 文字列の取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			文字列が記録されているはずのログデータ上の位置
//
//	RETURN
//		ログに記録されている文字列
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にIDが記録されていない

const ModUnicodeString&
LogData::
getString(int iIndex_) const
{
	return getString((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::getStrings -- 文字列配列の取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			文字列配列が記録されているはずのログデータ上の位置
//
//	RETURN
//		const ModVector<ModUnicodeString>& vecString_
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置に文字列配列が記録されていない

const ModVector<ModUnicodeString>&
LogData::
getStrings(int iIndex_) const
{
	return getStrings((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::getUnsignedInteger -- unsigned intの取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			unsigned intが記録されているはずのログデータ上の位置
//
//	RETURN
//		unsigned intの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にunsigned intが記録されていない

unsigned int
LogData::
getUnsignedInteger(int iIndex_) const
{
	return getUnsignedInteger((*this)[iIndex_]);
}

// unsigned int配列の取得
const ModVector<unsigned int>&
LogData::
getUnsignedIntegers(int iIndex_) const
{
	return getUnsignedIntegers((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::getInteger -- intの取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			intが記録されているはずのログデータ上の位置
//
//	RETURN
//		intの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にintが記録されていない

int
LogData::
getInteger(int iIndex_) const
{
	return getInteger((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::getUnsignedInteger64 -- ModUInt64 の取得
//
//	NOTES
//
//	ARGUMENTS
//		int		index
//			ModUInt64 が記録されているはずの配列要素の位置
//
//	RETURN
//		得られた ModUInt64
//
//	EXCEPTIONS

ModUInt64
LogData::
getUnsignedInteger64(int index) const
{
	return getUnsignedInteger64((*this)[index]);
}

//	FUNCTION public
//	Schema::LogData::getDataArrayData -- DataArrayDataの取得
//
//	NOTES
//
//	ARGUMENTS
//		int iIndex_
//			DataArrayDataが記録されているはずのログデータ上の位置
//
//	RETURN
//		DataArrayDataへの参照
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定された位置にDataArrayDataが記録されていない

const Common::DataArrayData&
LogData::
getDataArrayData(int iIndex_) const
{
	return getDataArrayData((*this)[iIndex_]);
}

//	FUNCTION public
//	Schema::LogData::createID -- IDの変換
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value iID_
//			変換するID
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createID(ObjectID::Value iID_)
{
	return createUnsignedInteger(iID_);
}

//	FUNCTION public
//	Schema::LogData::createIDs -- ID配列の変換
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ObjectID::Value>& vecID_
//			変換するID配列
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createIDs(const ModVector<ObjectID::Value>& vecID_)
{
	return new Common::UnsignedIntegerArrayData(vecID_);
}

//	FUNCTION public
//	Schema::LogData::createString -- 文字列の変換
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& cString_
//			変換する文字列
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createString(const ModUnicodeString& cString_)
{
	return new Common::StringData(cString_);
}

//	FUNCTION public
//	Schema::LogData::createStrings -- 文字列配列の変換
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>& vecString_
//			変換する文字列配列
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createStrings(const ModVector<ModUnicodeString>& vecString_)
{
	return new Common::StringArrayData(vecString_);
}

//	FUNCTION public
//	Schema::LogData::createUnsignedInteger -- unsigned intの変換
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int iValue_
//			変換するunsigned int
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createUnsignedInteger(unsigned int iValue_)
{
	return new Common::UnsignedIntegerData(iValue_);
}

// FUNCTION public
//	Schema::LogData::createUnsignedIntegers -- unsigned int配列の変換
//
// NOTES
//
// ARGUMENTS
//	const ModVector<unsigned int>& vecValue_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//static
Common::Data::Pointer
LogData::
createUnsignedIntegers(const ModVector<unsigned int>& vecValue_)
{
	return new Common::UnsignedIntegerArrayData(vecValue_);
}

//	FUNCTION public
//	Schema::LogData::createInteger -- intの変換
//
//	NOTES
//
//	ARGUMENTS
//		int iValue_
//			変換するint
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createInteger(int iValue_)
{
	return new Common::IntegerData(iValue_);
}

//	FUNCTION public
//	Schema::LogData::createUnsignedInteger64 -- ModUInt64 の変換
//
//	NOTES
//
//	ARGUMENTS
//		ModUInt64		v
//			変換する ModUInt64
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
Common::Data::Pointer
LogData::
createUnsignedInteger64(ModUInt64 v)
{
	return new Common::UnsignedInteger64Data(v);
}

//	FUNCTION public
//	Schema::LogData::createNull -- NullDataの変換
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION
Common::Data::Pointer
LogData::
createNull()
{
	return Common::NullData::getInstance();
}

//	FUNCTION public
//	Schema::LogData::getID -- IDの取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			IDが記録されているはずのログデータ
//
//	RETURN
//		IDの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにIDが記録されていない

ObjectID::Value
LogData::
getID(const Common::Data::Pointer& pElement_)
{
	return getUnsignedInteger(pElement_);
}

//	FUNCTION public
//	Schema::LogData::getIDs -- 文字列配列の取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			ID配列が記録されているはずのログデータ
//
//	RETURN
//		const ModVector<ObjectID::Value>&
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにID配列が記録されていない

const ModVector<ObjectID::Value>&
LogData::
getIDs(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::Array
		|| pElement_->getElementType() != Common::DataType::UnsignedInteger) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement_).getValue();
}

//	FUNCTION public
//	Schema::LogData::getString -- 文字列の取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			文字列が記録されているはずのログデータ
//
//	RETURN
//		ログに記録されている文字列
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにIDが記録されていない

const ModUnicodeString&
LogData::
getString(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::String) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pElement_).getValue();
}

//	FUNCTION public
//	Schema::LogData::getStrings -- 文字列配列の取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			文字列配列が記録されているはずのログデータ
//
//	RETURN
//		const ModVector<ModUnicodeString>& vecString_
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータに文字列配列が記録されていない

const ModVector<ModUnicodeString>&
LogData::
getStrings(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::Array
		|| pElement_->getElementType() != Common::DataType::String) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::StringArrayData&, *pElement_).getValue();
}

//	FUNCTION public
//	Schema::LogData::getUnsignedInteger -- unsigned intの取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			unsigned intが記録されているはずのログデータ
//
//	RETURN
//		unsigned intの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにunsigned intが記録されていない

unsigned int
LogData::
getUnsignedInteger(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::UnsignedInteger) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *pElement_).getValue();
}

// unsigned int配列の取得
//static
const ModVector<unsigned int>&
LogData::
getUnsignedIntegers(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::Array
		|| pElement_->getElementType() != Common::DataType::UnsignedInteger) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement_).getValue();
}

//	FUNCTION public
//	Schema::LogData::getInteger -- intの取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			intが記録されているはずのログデータ
//
//	RETURN
//		intの値
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにintが記録されていない

int
LogData::
getInteger(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::Integer) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, *pElement_).getValue();
}

//	FUNCTION public
//	Schema::LogData::getUnsignedInteger64 -- ModUInt64 の取得
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data::Pointer&	elm
//			ModUInt64 が記録されているはずの配列要素
//
//	RETURN
//		得られた ModUInt64
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			指定された配列要素には ModUInt64 が記録されていない

// static
ModUInt64
LogData::
getUnsignedInteger64(const Common::Data::Pointer& elm)
{
	if (!elm.get() || elm->getType() != Common::DataType::UnsignedInteger64)
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	return _SYDNEY_DYNAMIC_CAST(
		const Common::UnsignedInteger64Data&, *elm).getValue();
}

//	FUNCTION public
//	Schema::LogData::getDataArrayData -- DataArrayDataの取得
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& pElement_
//			DataArrayDataが記録されているはずのログデータ
//
//	RETURN
//		DataArrayDataへの参照
//
//	EXCEPTION
//		Exception::LogItemCorrupted
//			指定されたデータにDataArrayDataが記録されていない

const Common::DataArrayData&
LogData::
getDataArrayData(const Common::Data::Pointer& pElement_)
{
	if (!pElement_.get()
		|| pElement_->getType() != Common::DataType::Array
		|| pElement_->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pElement_);
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
