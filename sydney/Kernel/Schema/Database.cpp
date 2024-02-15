// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.cpp -- データベース関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/Area.h"
#include "Schema/AreaMap.h"
#include "Schema/AreaContent.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Cascade.h"
#include "Schema/CascadeMap.h"
#include "Schema/Column.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/File.h"
#include "Schema/Function.h"
#include "Schema/FunctionMap.h"
#include "Schema/Hold.h"
#include "Schema/Index.h"
#include "Schema/LogData.h"
#include "Schema/NameParts.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/Object.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/Parameter.h"
#include "Schema/Partition.h"
#include "Schema/PartitionMap.h"
#include "Schema/PathParts.h"
#include "Schema/Privilege.h"
#include "Schema/PrivilegeMap.h"
#include "Schema/Recovery.h"
#include "Schema/Sequence.h"
#include "Schema/SessionID.h"
#include "Schema/SystemDatabase.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Cascade.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/SystemTable_Function.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/SystemTable_Privilege.h"
#include "Schema/Table.h"
#include "Schema/TableMap.h"
#include "Schema/TemporaryDatabase.h"
#include "Schema/Utility.h"
#include "Schema/VirtualTable.h"
#include "Schema/Message_DatabasePathNotExist.h"
#include "Schema/Message_VerifyStarted.h"
#include "Schema/Message_VerifyFinished.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#ifndef SYD_COVERAGE
#include "Admin/Debug.h"
#endif
#include "Admin/Utility.h"

#include "Checkpoint/Database.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Configuration.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/ObjectPointer.h"
#include "Common/StringArrayData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/AreaNotFound.h"
#include "Exception/AlreadyBeginTransaction.h"
#include "Exception/Cancel.h"
#include "Exception/DatabaseAlreadyDefined.h"
#include "Exception/DatabaseCorrupted.h"
#include "Exception/DatabaseNotAvailable.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/InvalidPath.h"
#include "Exception/LockTimeout.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/LogFileCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/RoleNotFound.h"
#include "Exception/SessionBusy.h"
#include "Exception/CanceledBySuperUser.h"
#include "Exception/TemporaryDatabase.h"
#include "Exception/Unexpected.h"

#include "Os/Path.h"

#include "FileCommon/FileOption.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverTable.h"

#include "Server/Manager.h"

#include "Statement/AlterDatabaseAttributeList.h"
#include "Statement/AlterDatabaseAttribute.h"
#include "Statement/AlterDatabaseStatement.h"
#include "Statement/AreaOption.h"
#include "Statement/DatabaseCreateOption.h"
#include "Statement/DatabaseDefinition.h"
#include "Statement/DropDatabaseStatement.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/Literal.h"
#include "Statement/DatabaseCreateOptionList.h"
#include "Statement/DatabasePathElementList.h"
#include "Statement/OptionalAreaParameter.h"
#include "Statement/MoveDatabaseStatement.h"
#include "Statement/MountDatabaseStatement.h"

#include "Lock/Name.h"

#include "Trans/LogFile.h"
#include "Trans/TimeStamp.h"
#include "Trans/Transaction.h"

#include "ModVector.h"
#include "ModAlgorithm.h"
#include "ModArchive.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

#ifdef OBSOLETE // this feature has few effect, so delete this
// スキーマオブジェクトを参照するセッションがなくなったときに
// 圧縮して保持しておく機能を使うか
Common::Configuration::ParameterBoolean _cFreezeEnabled("Schema_FreezeEnabled", false);
#endif

// CacheMapのコンストラクターに与えるパラメーター
ModSize _cacheMapSize = 203; // Default
ModBoolean _cacheMapEnableLink = ModFalse; // Iterationしない

// 各データベースのスーパーユーザモード移行中状態を
// 管理するハッシュマップ(キーはデータベースID)
Database::SuperUserModeNameMap		m_cUserModeNameMap;

// 各データベースのスーパーユーザモード移行中状態を
// 管理するハッシュマップ(キーはデータベース名)
Database::SuperUserModeIdMap		m_cUserModeIdMap;

	
// SuperUserModeMapの排他制御用
Os::CriticalSection m_cUserModeMapLatch;

// スキーマオブジェクトをオブジェクト ID の昇順でソートするための関数

ModBoolean
ascendingSortObject(Object* l, Object* r)
{
	return (l->getID() < r->getID()) ? ModTrue : ModFalse;
}

namespace _Name {

	// 名前の重複を調べる
	bool _checkExistence(Trans::Transaction& cTrans_, const Database* pDatabase_);

} // namespace _Name

namespace _Path {

	// カテゴリーからパス名の定義を得る
	const ModUnicodeString* _getPathDefinition(Database::Path::Category::Value eCategory_,
											   const ModVector<ModUnicodeString>* vecPath_);
	// パスを絶対パスに変更する
	ModUnicodeString _getFullPathName(Database::Path::Category::Value eCategory_,
									  const ModUnicodeString& cstrPath_);
	// カテゴリーから対応するパス名を得る
	ModUnicodeString _getPath(Database::Path::Category::Value eCategory_,
							  const ModUnicodeString* pPath_, const Object::Name& name_);
	ModUnicodeString _getPath(Database::Path::Category::Value eCategory_,
							  const ModUnicodeString* pPath_, const Database* pDatabase_);
	// パス名が同一の場所を指しているか
	bool _isIdentical(const ModUnicodeString& cPath_, const ModUnicodeString& cPath2_);
	// パス名が他のデータベースで使用されていないか
	bool _isUsedInOthers(Trans::Transaction& cTrans_, const ModUnicodeString& cPath_, const Database* pDatabase_);
	// Alterで変更されるパスの存在を調べる
	bool _checkExistence(Trans::Transaction& cTrans_, const Database* pDatabase_,
						 const ModVector<bool>* vecChanged_,
						 const ModVector<ModUnicodeString>* vecPath_,
						 bool bAllowExistence_);
	// MoveDatabaseによって不要になったパス以下を消去する
	void _sweep(Trans::Transaction& cTrans_, const Database* pDatabase_,
				const ModVector<ModUnicodeString>& vecPrevPath_,
				const ModVector<ModUnicodeString>& vecPostPath_);

	// checkRelatedPathでエリアについて調べるための関数
	bool _isRelated(const Os::Path& cPath1_, const Os::Path& cPath2_);
	bool _checkAreaPath(Area* pArea_, const Os::Path& cPath_);

} // namespace _Path

namespace _Privilege
{
	// name of built-in roles
	// [NOTES]
	// built-in roles are defined corresponding to privilege categories

	const ModUnicodeString _cBuiltInRoleName[Common::Privilege::Category::ValueNum] =
	{
		_TRMEISTER_U_STRING("system_operations"),	// System
		_TRMEISTER_U_STRING("database_operations"),	// Database
		_TRMEISTER_U_STRING("data_operations"),		// Data
		_TRMEISTER_U_STRING("reference_operations")	// Reference
	};

} // namespace _Privilege

} // namespace

//////////////////////////////////////////////////////////////////////
// _Name
//////////////////////////////////////////////////////////////////////

//	FUNCTION local
//	$$::_Name::_checkExistence -- createで指定される名前をチェックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database* pDatabase_
//			createしようとしているデータベースオブジェクト
//
//	RETURN
//		true...名前は重複しない
//		false...重複する名前があり、システムパラメーターCancelWhenDuplicatedがtrue
//
//	EXCEPTIONS
//		DatabaseAlreadyDefined
//			重複する名前があり、システムパラメーターCancelWhenDuplicatedがfalse

bool
_Name::_checkExistence(Trans::Transaction& cTrans_, const Database* pDatabase_)
{
	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる

	if (Manager::ObjectName::reserve(pDatabase_) == false) {

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// falseを返し後の処理は何もしない
			SydInfoMessage
				<< "Database definition of the same name in progress("
				<< pDatabase_->getName()
				<< ") canceled"
				<< ModEndl;
			return false;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Database definition of the same name in progress("
				<< pDatabase_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW1(Exception::DatabaseAlreadyDefined, pDatabase_->getName());
		}
	}

	// 同じ名前のデータベースがないか調べ、
	// 同時に既存のデータベースをマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときにcreate中のDatabaseを
	// 読み込んでしまう

	bool bFound = false;
	try {
		bFound = (Database::getID(pDatabase_->getName(), cTrans_) != ObjectID::Invalid);
	} catch (...) {
		Manager::ObjectName::withdraw(pDatabase_);
		_SYDNEY_RETHROW;
	}
	if ( bFound ) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pDatabase_);

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated database definition("
						   << pDatabase_->getName()
						   << ")	canceled"
						   << ModEndl;
			return false;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated database definition("
						   << pDatabase_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW1(Exception::DatabaseAlreadyDefined, pDatabase_->getName());
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// _Path
//////////////////////////////////////////////////////////////////////

//	FUNCTION local
//	$$::_Path::_getPathDefinition -- カテゴリーからパス名指定を得る
//
//	NOTES
//		alterなどでデータベースのメンバーにセットする前に
//		得たい場合もあるのでパス指定配列は引数で与える
//
//	ARGUMENTS
//		Schema::Database::Path::Category::Value eCategory_
//			データベースのパス指定種別
//		const ModVector<ModUnicodeString>* vecPath_
//			パス指定配列
//
//	RETURN
//		指定した種別のパス指定
//
//	EXCEPTIONS

const ModUnicodeString*
_Path::_getPathDefinition(Database::Path::Category::Value eCategory_,
						  const ModVector<ModUnicodeString>* vecPath_)
{
	if (vecPath_) {
		ModSize i = static_cast<ModSize>(eCategory_);
		if (vecPath_->getSize() > i && (*vecPath_)[i].getLength() > 0) {
			return &((*vecPath_)[i]);
		}
	}
	return 0;
}

// FUNCTION local
//	$$$::_Path::_getFullPathName -- パスを絶対パスに変更する
//
// NOTES
//
// ARGUMENTS
//	Database::Path::Category::Value eCategory_
//	const ModUnicodeString& cstrPath_
//	
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
_Path::
_getFullPathName(Database::Path::Category::Value eCategory_,
				 const ModUnicodeString& cstrPath_)
{
	ModUnicodeString cstrResult;
	if (cstrPath_.getLength() > 0) {
		if (ModOsDriver::File::isFullPathName(cstrPath_)) {
			return cstrPath_;
		} else {
			switch (eCategory_) {
			case Database::Path::Category::Data:
			case Database::Path::Category::LogicalLog:
				{
					ModOsDriver::File::getFullPathName(Manager::Configuration::getDefaultAreaPath(),
													   cstrPath_,
													   cstrResult);
					break;
				}
			case Database::Path::Category::System:
				{
					ModOsDriver::File::getFullPathName(Manager::Configuration::getSystemAreaPath(),
													   cstrPath_,
													   cstrResult);
					break;
				}
			}
		}
	}
	return cstrResult;
}

//	FUNCTION local
//	$$::_Path::_getPath -- カテゴリーから対応するパス名を得る
//
//	NOTES
//		alterなどでデータベースのメンバーにセットする前に
//		得たい場合もあるのでパス指定文字列は引数で与える
//
//	ARGUMENTS
//		Schema::Database::Path::Category::Value eCategory_
//			データベースのパス指定種別
//		const ModUnicodeString* pPath_
//			パス指定
//		const Database* pDatabase_
//			データベースオブジェクト
// 		const Object::Name& name_
//			データベース名
//
//	RETURN
//		指定した種別のパス指定
//
//	EXCEPTIONS

inline
ModUnicodeString
_Path::_getPath(Database::Path::Category::Value eCategory_,
				const ModUnicodeString* pPath_, const Object::Name& name_)
{
	; _SYDNEY_ASSERT(name_.getLength());

	return (pPath_ && pPath_->getLength()) ?
		_getFullPathName(eCategory_, *pPath_) :
		static_cast<ModUnicodeString>(Database::getDefaultPath(eCategory_, name_));
}

inline
ModUnicodeString
_Path::_getPath(Database::Path::Category::Value eCategory_,
				const ModUnicodeString* pPath_, const Database* pDatabase_)
{
	; _SYDNEY_ASSERT(pDatabase_);
	return _getPath(eCategory_, pPath_, pDatabase_->getName());
}

//	FUNCTION local
//	$$::_Path::_isIdentical -- パス名が同一の場所を指しているか
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& cPath_
//			パス名
//		const ModUnicodeString& cPath2_
//			調べる対象のパス
//
//	RETURN
//		true .. 同一のパスである
//		false.. 同一のパスでない
//
//	EXCEPTIONS

inline
bool
_Path::_isIdentical(const ModUnicodeString& cPath_, const ModUnicodeString& cPath2_)
{
	return (Os::Path::compare(cPath_, cPath2_)
			== Os::Path::CompareResult::Identical);
}

//	FUNCTION local
//	$$::_Path::_isUsedInOthers -- パス名が他のデータベースやエリアで使用されているか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModUnicodeString& cPath_
//			パス名
//		const Database* pDatabase_
//			対象のデータベース
//
//	RETURN
//		true .. ディレクトリーが他のデータベースやエリアで使用されている
//		false.. ディレクトリーが他のデータベースやエリアで使用されていない
//
//	EXCEPTIONS
bool
_Path::_isUsedInOthers(Trans::Transaction& cTrans_, const ModUnicodeString& cPath_, const Database* pDatabase_)
{
	const ModVector<Database*>& vecDatabase = Manager::ObjectTree::Database::get(cTrans_);
	ModVector<Database*>::ConstIterator iterator = vecDatabase.begin();
	const ModVector<Database*>::ConstIterator& end = vecDatabase.end();
	for (; iterator != end; ++iterator) {
		// 自身とは比較しない
		if (*iterator == pDatabase_) continue;
		// 永続でないデータベースとは比較しない
		if ((*iterator)->getScope() != Object::Scope::Permanent) continue;

		// データベースおよびそれに属するエリアについて調べる
		if ((*iterator)->checkRelatedPath(cTrans_, cPath_))
			return true;
	}
	return false;
}

//	FUNCTION local
//	$$::_Path::_checkExistence -- Alterで変更されるパス名の存在を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Database* pDatabase_
//			データベースオブジェクト
//		const ModVector<bool>* vecChanged_
//			パス変更指定の有無を表すBoolean配列へのポインター
//			0ならすべてのカテゴリーで調べる
//		const ModVector<ModUnicodeString>* vecPath_
//			パス指定配列へのポインター
//		bool bAllowExistence_
//			trueのときすでに存在していてもエラーにしない
//			(redo用)
//
//	RETURN
//		true .. 同じパス名のディレクトリーが存在する
//		false.. 同じパス名のディレクトリーは存在しない
//
//	EXCEPTIONS

bool
_Path::_checkExistence(Trans::Transaction& cTrans_,
					   const Database* pDatabase_,
					   const ModVector<bool>* vecChanged_,
					   const ModVector<ModUnicodeString>* vecPath_,
					   bool bAllowExistence_)
{
	ModVector<ModUnicodeString> vecCheckedPath;
	const int nPathCategory = static_cast<int>(Database::Path::Category::ValueNum);
	vecCheckedPath.reserve(nPathCategory);

	// 調べる対象のパスを列挙する
	int i = 0;
	for (; i < nPathCategory; ++i) {
		// 変更のあるもののみが対象である
		if (!vecChanged_ || (*vecChanged_)[i]) {
			Database::Path::Category::Value eCategory =
				static_cast<Database::Path::Category::Value>(i);
			ModUnicodeString cstrPath =
				_getPath(eCategory, _getPathDefinition(eCategory, vecPath_), pDatabase_);

			if (cstrPath.getLength() > Manager::ObjectPath::getMaxLength()) {
				// パス名が制限長を超えていたらログに吐いて継続
				SydInfoMessage
					<< "Warning: database path length (" << cstrPath.getLength()
					<< ") exceeds the limit(" << Manager::ObjectPath::getMaxLength() << "),"
					<< " creating table with long name may be failed." << ModEndl;
			}

			if (i > 0) {
				int n = vecCheckedPath.getSize();
				int j = 0;
				for (; j < n; ++j) {
					if (_isIdentical(cstrPath, vecCheckedPath[j]))
						// すでにあるパスと同一である
						break;
				}
				if (j < n)
					// すでにあったので次へ
					continue;
			}
			vecCheckedPath.pushBack(cstrPath);
		}
	}

	// 他に作成中や移動中のエリアやデータベースが使用しているパスと重複していないか調べると同時に
	// 使用するパスとして登録する
	if (!Manager::ObjectPath::reserve(pDatabase_, vecCheckedPath)) {
		// 重複していた
		SydInfoMessage << "New database path already used in others." << ModEndl;
		return true;
	}

	int n = vecCheckedPath.getSize();
	i = 0;
	for (; i < n; ++i) {
		Os::Path cPath(vecCheckedPath[i]);
		if (Utility::File::isFound(cPath)) {

			// ディレクトリーが存在する
			// 自身の別のカテゴリーでの変更前ディレクトリーと同じなら
			// そのカテゴリーの移動がない限りエラーにしない
			if (!bAllowExistence_) {
				if (vecChanged_) {
					int iOtherCategory = 0;
					for (; iOtherCategory < nPathCategory; ++iOtherCategory) {

						// 別のカテゴリーでその移動がないなら移動前のパスと同じパスか調べる

						if (!(*vecChanged_)[iOtherCategory]) {
							Database::Path::Category::Value eOtherCategory =
								static_cast<Database::Path::Category::Value>(iOtherCategory);

							// 親子関係ではなく同一パスかで調べる
							// 親子関係の場合、たとえば表のディレクトリーにログが置かれてしまうと
							// 予期しないときに消されてしまう可能性がある

							if (_isIdentical(pDatabase_->getPath(eOtherCategory), cPath)) {
								break;
							}
						}
					}
					// 同一パスがあったか
					if (iOtherCategory < nPathCategory) continue;
					// 同一パスがなかったということは別の用途に使用されているパスである
					SydInfoMessage << "New database path: " << cPath << " already exists." << ModEndl;
					Manager::ObjectPath::withdraw(pDatabase_);
					return true;
				} else {
					// vecChangedが指定されていないことは新規作成を意味する
					// 新規作成においてはファイルが存在したらエラー
					SydInfoMessage << "New database path: " << cPath << " already exists." << ModEndl;
					Manager::ObjectPath::withdraw(pDatabase_);
					return true;
				}
			}
			if (_isUsedInOthers(cTrans_, cPath, pDatabase_)) {
				// 他のデータベースやエリアで使用されている
				SydInfoMessage << "New database path: " << cPath << " already used in others." << ModEndl;
				Manager::ObjectPath::withdraw(pDatabase_);
				return true;
			}
		}
	}

	// 重複はなかった
	return false;
}

//	FUNCTION local
//	$$::_Path::_isRelated -- パス名が一致または親子関係があるか
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath1_
//		const Os::Path& cPath2_
//			調べる対象のパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
bool
_Path::_isRelated(const Os::Path& cPath1_, const Os::Path& cPath2_)
{
	return (cPath1_.compare(cPath2_) != Os::Path::CompareResult::Unrelated);
}

//	FUNCTION local
//	$$::_Path::_sweep -- 移動により不要になったパスを消去する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Database* pDatabase_
//			データベースオブジェクト
//		const ModVector<ModUnicodeString>& vecPrevPath_
//		const ModVector<ModUnicodeString>& vecPostPath_
//			移動前後のパス指定配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Path::_sweep(Trans::Transaction& cTrans_,
			  const Database* pDatabase_,
			  const ModVector<ModUnicodeString>& vecPrevPath_,
			  const ModVector<ModUnicodeString>& vecPostPath_)
{
	for (int i = static_cast<int>(Database::Path::Category::Data); i < Database::Path::Category::ValueNum; ++i) {

		// 変更のあるカテゴリーのみ処理すればよい
		const ModUnicodeString& cstrPrevPath = vecPrevPath_[i];
		const ModUnicodeString& cstrNewPath = vecPostPath_[i];

		if (cstrPrevPath != cstrNewPath) {

			Os::Path cRemovePath =
				_Path::_getPath(static_cast<Database::Path::Category::Value>(i), &cstrPrevPath, pDatabase_);
			// 移動後のパス指定に同じものがあったら削除できない
			bool bFound = false;
			for (int j = static_cast<int>(Database::Path::Category::Data); j < Database::Path::Category::ValueNum; ++j) {

				Os::Path cCheckPath =
					_Path::_getPath(static_cast<Database::Path::Category::Value>(j), &(vecPostPath_[j]), pDatabase_);
				if (i != j && (_Path::_isRelated(cCheckPath, cRemovePath))) {
					bFound = true;
					break;
				}
			}
			if (bFound) continue;

			Utility::File::rmAll(cRemovePath);
		}
	}
}

//	FUNCTION local
//	$$::_Path::_checkAreaPath -- パス名がエリアに使用されていないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area* pArea_
//			調べる対象のエリア
//		const Os::Path& cPath_
//			調べる対象のパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

bool
_Path::_checkAreaPath(Area* pArea_, const Os::Path& cPath_)
{
	; _SYDNEY_ASSERT(pArea_);
	ModSize n = pArea_->getSize();
	for (ModSize i = 0; i < n; ++i) {
		if (_isRelated(cPath_, pArea_->getPath(i)))
			// 親子関係がある
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Database
//////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	Schema::Database::Database --
//		データベースを表すクラスのデフォルトコンストラクター
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

Database::
Database()
	: Object(Object::Category::Database),
	  m_cAttribute(),
	  _sequence(0),
	  _areas(0),
	  _tables(0),
	  _cascades(0),
	  _partitions(0),
	  _functions(0),
	  _privileges(0),
	  _cache(0),
	  m_pPath(0),
	  m_pLogFilePath(0),
	  m_pDataPath(0),
	  m_pSystemTables(0),
	  m_iSystemTableObjectID(ID::Invalid),
	  m_pVirtualTables(0),
	  m_pSnapshot(0),
	  m_iReference(0),
	  m_bDelayedClear(false),
	  m_iFreezedSize(0)
{
}

//	FUNCTION public
//	Schema::Database::Database --
//		データベース定義からデータベースを表すクラスのコンストラクター
//
//	NOTES
//		データベースを表すクラスを生成するだけで、
//		「データベース」表は更新されない
//
//	ARGUMENTS
//		const Statement::DatabaseDefinition&	cStatement_
//			データベース定義のSQL文を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Database::
Database(const Statement::DatabaseDefinition& cStatement_)
	: Object(Category::Database, Scope::Permanent,
			 Status::Unknown,
			 ID::Invalid, ID::Invalid, ID::SystemTable),
	  m_cAttribute(),
	  _sequence(0),
	  _areas(0),
	  _tables(0),
	  _cascades(0),
	  _partitions(0),
	  _functions(0),
	  _privileges(0),
	  _cache(0),
	  m_pPath(0),
	  m_pLogFilePath(0),
	  m_pDataPath(0),
	  m_pSystemTables(0),
	  m_iSystemTableObjectID(ID::Invalid),
	  m_pVirtualTables(0),
	  m_pSnapshot(0),
	  m_iReference(0),
	  m_bDelayedClear(false),
	  m_iFreezedSize(0)
{
	// データベース定義のSQL文からデータベースオブジェクトを作る

	Statement::Identifier* identifier = cStatement_.getDatabaseName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());
	; _SYDNEY_ASSERT(identifier->getIdentifier()->getLength());

	setName(*identifier->getIdentifier());

	// Statement データの抽出
	{
		using namespace Statement;

		// DatabaseCreateOptionList -> DatabasePathElementList への変換
		DatabaseCreateOptionList* pList = cStatement_.getOptionList();
		if ( pList != 0 )
			setCreateOption(*pList);
	}

	if (Statement::Hint* pHint = cStatement_.getHint())
	{
		/* HINTの処理はまだ */
	}
}

//	FUNCTION public
//	Schema::Database::Database --
//		データベース定義のログからデータベースを表すクラスのコンストラクター
//
//	NOTES
//		データベースを表すクラスを生成するだけで、
//		「データベース」表は更新されない
//
//	ARGUMENTS
//		const Schema::LogData&	cLogData_
//			データベース定義に対応するログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Database::
Database(const LogData& cLogData_)
	: Object(Category::Database, Scope::Permanent,
			 Status::Unknown,
			 ID::Invalid, ID::Invalid, ID::SystemTable),
	  m_cAttribute(),
	  _sequence(0),
	  _areas(0),
	  _tables(0),
	  _cascades(0),
	  _partitions(0),
	  _functions(0),
	  _privileges(0),
	  _cache(0),
	  m_pPath(0),
	  m_pLogFilePath(0),
	  m_pDataPath(0),
	  m_pSystemTables(0),
	  m_iSystemTableObjectID(ID::Invalid),
	  m_pVirtualTables(0),
	  m_pSnapshot(0),
	  m_iReference(0),
	  m_bDelayedClear(false),
	  m_iFreezedSize(0)
{
	// データベース定義のログデータからデータベースオブジェクトを作る

	; _SYDNEY_ASSERT(cLogData_.getSubCategory() == LogData::Category::CreateDatabase ||
					 cLogData_.getSubCategory() == LogData::Category::Mount );

	// ログの内容を反映する
	//	 ログ内容:
	//		1．データベース名
	//		2. ID(使用しない)
	//		3．パスリスト
	//		4．属性
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	setName(cLogData_.getString(Log::Name));
	setPath(cLogData_.getStrings(Log::Create::Path));
	if ( !unpackMetaField(cLogData_[Log::Create::Flag].get(), Meta::Database::Flag)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//	FUNCTION public
//	Schema::Database::Database --
//		名称によるデータベースを表すクラスのコンストラクター
//
//	NOTES
//		データベースを表すクラスを生成するだけで、
//		「データベース」表は更新されない
//
//	ARGUMENTS
//		const Schema::Object::Name& cName_
//			定義するデータベースの名称
//		Schema::Object::Scope::Value eScope_ = Scope::Permanent
//			定義するデータベースのスコープ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Database::
Database(const Name& cName_, Scope::Value eScope_)
	: Object(Category::Database, eScope_,
			 Status::Unknown,
			 ID::Invalid, ID::Invalid, ID::SystemTable,
			 cName_),
	  m_cAttribute(),
	  _sequence(0),
	  _areas(0),
	  _tables(0),
	  _cascades(0),
	  _partitions(0),
	  _functions(0),
	  _privileges(0),
	  _cache(0),
	  m_pPath(0),
	  m_pLogFilePath(0),
	  m_pDataPath(0),
	  m_pSystemTables(0),
	  m_iSystemTableObjectID(ID::Invalid),
	  m_pVirtualTables(0),
	  m_pSnapshot(0),
	  m_iReference(0),
	  m_bDelayedClear(false),
	  m_iFreezedSize(0)
{
}

//	FUNCTION public
//	Schema::Database::~Database -- データベースを表すクラスのデストラクター
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

Database::
~Database()
{
	destruct();
}

//	FUNCTION public
//		Schema::Database::getNewInstance -- オブジェクトを新たに取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			元になるデータ
//
//	RETURN
//		新規に作成されたオブジェクト
//
//	EXCEPTIONS

// static
Database*
Database::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Database> pObject = new Database;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION private
//	Schema::Database::destruct --
//		データベースを表すクラスのデストラクター下位関数
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
Database::
destruct()
{
	//まだ下記のアサートが常に正しいようにできていないのでひとまずコメント
	//; _SYDNEY_ASSERT(m_iReference == 0);

	// Area、Table、Privilegeのオブジェクトを破棄する
	clearArea();
	clearTable();
	clearFunction();
	clearPrivilege();

	// システム表を表すオブジェクトを解放する
	clearSystemTables();

	// delete virtual tables
	clearVirtualTables();

	// 表以下のオブジェクトを解放する
	clearCache();

	// 使用しているファイルをdetachする
	detachFiles();

	// パス名指定を解放する
	clearPath();
	clearLogFilePath();
	clearDataPath();

	// 分散用オブジェクトを解放する
	clearCascade();
	clearPartition();

	// キャッシュのクリアが後回しになっていたらエントリーから破棄する
	if (m_bDelayedClear) {
		clearFreezed();
		; _SYDNEY_ASSERT(m_pSnapshot);
		m_pSnapshot->eraseDelayedClear(getID());
		m_bDelayedClear = false;
	}
}

//	FUNCTION private
//	Schema::Database::detachFiles --
//		データベースを表すクラスが使用しているファイルをdetachする
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
Database::
detachFiles()
{
	clearSequence();

	// 表が使っているファイルをdetachする
	if (_tables)
		_tables->apply(ApplyFunction0<Table>(&Table::detachFiles));
}

//	FUNCTION public
//	Schema::Database::create --
//		SQL のデータベース定義からデータベースを実際に定義する
//
//	NOTES
//		更新トランザクション用のキャッシュを使用する
//
//	ARGUMENTS
//		const Statement::DatabaseDefinition&	cStatement_
//			データベース定義のSQL文を表すクラス
//		Trans::Transaction& cTrans_
//			データベース定義を行うトランザクション記述子
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//
//	RETURN
//		定義されたデータベースのスキーマオブジェクト
//
//	EXCEPTIONS

// static
Database*
Database::
create(const Statement::DatabaseDefinition& cStatement_,
	   LogData& cLogData_,
	   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// b.データベースのスキーマオブジェクトを新規作成
	ModAutoPointer<Database> database = new Database(cStatement_);
	; _SYDNEY_ASSERT(database.get());

	// 名前が重複していないか調べる
	if (!_Name::_checkExistence(cTrans_, database.get())) {
		// 重複しており、無視する設定になっているので
		// ここで終わる
		// ★注意★
		// 無視する設定になっていないときは例外が飛んでいる
		return 0;
	}

	// create時からREAD ONLYであるようなデータベースは作れない
	if (database->isReadOnly()) {
		SydInfoMessage << "Creating database with READ ONLY option is not supported." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// IDをふり、状態を変える
	database->Object::create(cTrans_);

	// ログデータを作る
	database->makeLogData(cTrans_, cLogData_);

	return database.release();
}

//	FUNCTION public
//	Schema::Database::create --
//		名称を指定してデータベースを定義する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& cName_
//			定義するデータベースの名前
//		Trans::Transaction& cTrans_
//			データベース定義を行うトランザクション記述子
//
//	RETURN
//		定義されたデータベースのスキーマオブジェクト
//
//	EXCEPTIONS

// static
Database*
Database::
create(const Name& cName_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 与えられたデータベース名から、
	// 定義しようとしているデータベースを表すクラスを生成する

	ModAutoPointer<Database> database = new Database(cName_);
	; _SYDNEY_ASSERT(database.get());

	// 名前が重複していないか調べる
	if (!_Name::_checkExistence(cTrans_, database.get())) {
		// 重複しており、無視する設定になっているので
		// ここで終わる
		// ★注意★
		// 無視する設定になっていないときは例外が飛んでいる
		return 0;
	}

	// IDをふり、状態を変える
	database->Object::create(cTrans_);

	// 生成されたデータベースのスキーマオブジェクトを返す
	return database.release();
}

//	FUNCTION public
//	Schema::Database::create --
//		ログデータからデータベースを定義する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			データベース定義のログデータ
//		Trans::Transaction& cTrans_
//			データベース定義を行うトランザクション記述子
//
//	RETURN
//		定義されたデータベースのスキーマオブジェクト
//
//	EXCEPTIONS

// static
Database*
Database::
create(const LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// b.データベースのスキーマオブジェクトを新規作成
	ModAutoPointer<Database> database = new Database(cLogData_);
	; _SYDNEY_ASSERT(database.get());

	// 名前が重複していないことを確認する
	// ログデータから読むなら重複しているはずがないが
	// 調べるのと同時にloadしておく意味があるのでassertにはできない
	if ( Database::getID(database->getName(), cTrans_) != ObjectID::Invalid ) {
		// 重複しているならログデータがおかしい
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// ALTER DATABASE SET/DROP PATHのUNDOが実行されているときは
	// 最終的なパスをこのデータベースのパスとする
	// IDはログのものを使う
	Object::ID::Value id = getObjectID(cLogData_);
	database->checkUndo(id);

	// IDを新たにふり、状態を変える
	database->Object::create(cTrans_, id);

	// 生成されたデータベースのスキーマオブジェクトを返す
	return database.release();
}

//	FUNCTION public
//	Schema::Database::create --
//		ログデータからデータベースを定義する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			データベース定義のログデータ
//		Trans::Transaction& cTrans_
//			データベース定義を行うトランザクション記述子
//
//	RETURN
//		定義されたデータベースのスキーマオブジェクト
//
//	EXCEPTIONS

// static

Database*
Database::
createForMount(const LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	ModAutoPointer<Database> pDatabase = Database::create(cLogData_, cTrans_);

	// マウントされていない状態にする
	pDatabase->m_cAttribute.m_bUnmounted = true;

	// マウント中であることを登録する
//	Manager::ObjectTree::Database::addMount(cTrans_, pDatabase);

	// ALTER DATABASE SET/DROP PATHのUNDOが実行されているときは
	// 最終的なパスをこのデータベースのパスとする
	// IDはログのものを使う
	ID::Value id = getObjectID(cLogData_);
	pDatabase->checkUndo(id);

	return pDatabase.release();
}

//	FUNCTION public
//	Schema::Database::create -- 自分自身の表すデータベースを実際に定義する
//
//	NOTES
//		永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bAllowExistence_ = false
//			すでに存在していてもエラーにしない
//			(redo用)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
create(Trans::Transaction& cTrans_, bool bAllowExistence_)
{
	// パス名の重複はstatic版のcreateで調査済み
	// ★注意★
	//	Name指定のcreateでは調査していないが、
	//	これらはインストール時、または一時データベース作成時に呼ばれ、
	//	レジストリーの設定がおかしくない限りパス名が重複することはありえない
	//	また、一時データベースは頻繁に作成されるので
	//	速度の面からもName指定のcreateではパス調査を省く

	// データベースに指定されたパスの作成
	// エラー発生時に自動的にmkdirを取り消すためのクラス
	Utility::File::AutoRmDir dataRmDir;
	Utility::File::AutoRmDir logRmDir;
	Utility::File::AutoRmDir systemRmDir;

	dataRmDir.setDir(getPath(Path::Category::Data));
	if (getScope() == Scope::Permanent) {
		logRmDir.setDir(getPath(Path::Category::LogicalLog));
	}
	systemRmDir.setDir(getPath(Path::Category::System));

	// このメソッド内での処理進行状況を表す列挙型
	enum {
		None,
		Area,
		AreaContent,
		Table,
		Column,
		Constraint,
		Index,
		Key,
		File,
		Field,
		Function,
		Privilege,
		Cascade,
		Partition,
		LogFile,
		Sequence,
		ValueNum
	} eStatus = None;

	try {

		if (getScope() == Scope::Permanent) {

			// システム表のうち、このデータベースに関する部分を生成する
			SystemTable::Area(*this).create(cTrans_, bAllowExistence_), eStatus = Area;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Area");
			SystemTable::AreaContent(*this).create(cTrans_, bAllowExistence_), eStatus = AreaContent;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "AreaContent");
			SystemTable::Table(*this).create(cTrans_, bAllowExistence_), eStatus = Table;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Table");
			SystemTable::Column(*this).create(cTrans_, bAllowExistence_), eStatus = Column;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Column");
			SystemTable::Constraint(*this).create(cTrans_, bAllowExistence_), eStatus = Constraint;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Constraint");
			SystemTable::Index(*this).create(cTrans_, bAllowExistence_), eStatus = Index;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Index");
			SystemTable::Key(*this).create(cTrans_, bAllowExistence_), eStatus = Key;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Key");
			SystemTable::File(*this).create(cTrans_, bAllowExistence_), eStatus = File;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "File");
			SystemTable::Field(*this).create(cTrans_, bAllowExistence_), eStatus = Field;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Field");
			SystemTable::Function(*this).create(cTrans_, bAllowExistence_), eStatus = Function;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Function");
			SystemTable::Privilege(*this).create(cTrans_, bAllowExistence_), eStatus = Privilege;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Privilege");
			SystemTable::Cascade(*this).create(cTrans_, bAllowExistence_), eStatus = Cascade;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Cascade");
			SystemTable::Partition(*this).create(cTrans_, bAllowExistence_), eStatus = Partition;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSystemTable", "Partition");

			// このデータベースに関する論理ログを記録するファイルを生成する

			// トランザクション記述子に論理ログを登録する

			cTrans_.setLog(*this);

			// 論理ログファイルを生成する
			if (!bAllowExistence_
				|| !cTrans_.getLog(Trans::Log::File::Category::Database)->isAccessible()) {
				cTrans_.createLog(Trans::Log::File::Category::Database), eStatus = LogFile;
			}
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateLog", "Created");
		}

		// オブジェクトIDを生成するファイルを生成する
		// このファイルは一時データベースに対しても作成する
		{
			getSequence().create(cTrans_, ID::SystemTable, bAllowExistence_), eStatus = Sequence;
			SCHEMA_FAKE_ERROR("Schema::Database", "CreateSequence", "Created");
		}

	} catch (...) {
		switch (eStatus) {
		case Sequence:
			getSequence().drop(cTrans_);
			if (getScope() != Scope::Permanent) {
				// 永続でないデータベースなら以降の後処理は不要
				break;
			}
		case LogFile:
			if (cTrans_.getLog(Trans::Log::File::Category::Database)->isAccessible())
				cTrans_.destroyLog(Trans::Log::File::Category::Database);
		case Partition:
			SystemTable::Partition(*this).drop(cTrans_);
		case Cascade:
			SystemTable::Cascade(*this).drop(cTrans_);
		case Privilege:
			SystemTable::Privilege(*this).drop(cTrans_);
		case Function:
			SystemTable::Function(*this).drop(cTrans_);
		case Field:
			SystemTable::Field(*this).drop(cTrans_);
		case File:
			SystemTable::File(*this).drop(cTrans_);
		case Key:
			SystemTable::Key(*this).drop(cTrans_);
		case Index:
			SystemTable::Index(*this).drop(cTrans_);
		case Constraint:
			SystemTable::Constraint(*this).drop(cTrans_);
		case Column:
			SystemTable::Column(*this).drop(cTrans_);
		case Table:
			SystemTable::Table(*this).drop(cTrans_);
		case AreaContent:
			SystemTable::AreaContent(*this).drop(cTrans_);
		case Area:
			SystemTable::Area(*this).drop(cTrans_);
		case None:
		default:
			break;
		}
		_SYDNEY_RETHROW;
	}

	// 正常終了したのでエラー処理のためのクラスをdisableする
	dataRmDir.disable();
	logRmDir.disable();
	systemRmDir.disable();

	// ★注意★
	// Managerへの登録は永続化後に行う
}

//	FUNCTION public
//	Schema::Database::createForMount -- Mount 操作のための Database オブジェクトを作成する
//
//	NOTES
//		この操作のあと、非 const の Database::create を呼んではいけない
//		（実際にファイルが create されるため）
//
//	ARGUMENTS
//		const Name& cName_
//			データベース名
//
//		const Statement::DatabaseCreateOptionList& stmt_
//			Statement オブジェクト
//
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
Database*
Database::
createForMount(const Name& cName_,
			   const Statement::DatabaseCreateOptionList& cStatement_,
			   Trans::Transaction& cTrans_)
{
	ModAutoPointer<Database> database = new Database(cName_);

	// Statement データの抽出
	{
		using namespace Statement;

		// DatabaseCreateOptionList -> DatabasePathElementList への変換
		database->setCreateOption(cStatement_);
	}

	// パスチェック
	if ( !database->isAccessible() )
	{
		SydInfoMessage << "Can't mount " << cName_ << ". Mount path does not exist." << ModEndl;
		_SYDNEY_THROW1(Exception::InvalidPath, cName_);
	}

	// 名前が重複していないか調べる
	if (!_Name::_checkExistence(cTrans_, database.get())) {
		// 重複しており、無視する設定になっているので
		// ここで終わる
		return 0;
	}

	// unmount されているはず
	database->m_cAttribute.m_bUnmounted = true;

	// IDをふり、状態を変える
	database->Object::create(cTrans_);

	// マウント中であることを登録する
//	Manager::ObjectTree::Database::addMount(cTrans_, database);

	// 生成されたデータベースのスキーマオブジェクトを返す
	return database.release();
}

//	FUNCTION public
//	Schema::Database::getName --
//		データベース抹消のSQL文にしたがってデータベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropDatabaseStatement& cStatement_
//			データベース抹消のSQL文を表すクラス
//
//	RETURN
//		抹消する対象のデータベース名
//
//	EXCEPTIONS

// static
Object::Name
Database::
getName(const Statement::DropDatabaseStatement& cStatement_)
{
	Statement::Identifier* identifier = cStatement_.getDbName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	return *identifier->getIdentifier();
}

//	FUNCTION public
//	Schema::Database::getName --
//		データベース移動のSQL文にしたがってデータベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::MoveDatabaseStatement& cStatement_
//			データベース移動のSQL文を表すクラス
//
//	RETURN
//		抹消する対象のデータベース名
//
//	EXCEPTIONS

// static
Object::Name
Database::
getName(const Statement::MoveDatabaseStatement& cStatement_)
{
	Statement::Identifier* identifier = cStatement_.getDatabaseName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	return *identifier->getIdentifier();
}

//	FUNCTION public
//	Schema::Database::getName --
//		データベース変更のSQL文にしたがってデータベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::AlterDatabaseStatement& cStatement_
//			データベース変更のSQL文を表すクラス
//
//	RETURN
//		抹消する対象のデータベース名
//
//	EXCEPTIONS

// static
Object::Name
Database::
getName(const Statement::AlterDatabaseStatement& cStatement_)
{
	Statement::Identifier* identifier = cStatement_.getDatabaseName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	return *identifier->getIdentifier();
}

//	FUNCTION public
//	Schema::Database::getName --
//		データベースオブジェクトの名称を得る
//
//	NOTES
//		SQL文から名前を得るためのgetNameメソッドで
//		オーバーロードしてしまったので改めて定義する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		対象のデータベース名
//
//	EXCEPTIONS

const Object::Name&
Database::
getName() const
{
	return Object::getName();
}

//	FUNCTION public
//	Schema::Database::getClassID -- このクラスのクラス ID を得る
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
Database::
getClassID() const
{
	return Externalizable::Category::Database +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Database::clear --
//		データベースを表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//		下位オブジェクトのキャッシュからの抹消を行わないので
//		キャッシュに載っているオブジェクトに対してこのメソッドを呼ぶときには
//		あらかじめキャッシュから抹消する処理を行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clear()
{
	destruct();

	// 属性をクリアする	
	m_cAttribute.clear();
}

//	FUNCTION public
//	Schema::Database::isReadOnly --
//		データベースが読み込み属性か問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : データベースは ReadOnly
//			false : データベースは ReadWrite
//
//	EXCEPTIONS
bool
Database::
isReadOnly() const
{
	return m_cAttribute.m_bReadOnly;
}

//	FUNCTION public
//	Schema::Database::setReadOnly --
//		データベース読み込み属性フラグを設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : データベースは ReadOnly
//			false : データベースは ReadWrite
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setReadOnly(bool bReadOnly_)
{
	m_cAttribute.m_bReadOnly = bReadOnly_;
}

//	FUNCTION public
//	Schema::Database::isOnline --
//		データベースがオンライン属性か問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : データベースは Online
//			false : データベースは Offline
//
//	EXCEPTIONS

bool
Database::
isOnline() const
{
	return m_cAttribute.m_bOnline;
}

//	FUNCTION public
//	Schema::Database::setOnline --
//		データベースのオンライン属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : データベースは Online
//			false : データベースは Offline
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setOnline(bool bOnline_)
{
	m_cAttribute.m_bOnline = bOnline_;
}

//	FUNCTION public
//	Schema::Database::isSuperUserMode --
//		データベースがスーパーユーザーモード属性か問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : データベースは スーパーユーザーモード
//			false : データベースは マルチユーザーモード
//
//	EXCEPTIONS

bool
Database::
isSuperUserMode() const
{
	return m_cAttribute.m_bSuperUserMode;
}

//	FUNCTION public
//	Schema::Database::setSuperUserMode --
//		データベースのスーパーユーザーモード属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : データベースは スーパーユーザーモード
//			false : データベースは マルチユーザーモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setSuperUserMode(bool bSuperUserMode_)
{
	m_cAttribute.m_bSuperUserMode = bSuperUserMode_;
}



//	FUNCTION public
//	Schema::Database::isRecoveryFull --
//		データベースの障害回復指定が完全かを問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : データベースは RecoveryFull
//			false : データベースは RecoveryCheckpoint
//
//	EXCEPTIONS

bool
Database::isRecoveryFull() const
{
	return m_cAttribute.m_bRecoveryFull;
}

//	FUNCTION public
//	Schema::Database::setRecoveryFull --
//		データベースの障害回復指定が完全かを指定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : データベースは RecoveryFull
//			false : データベースは RecoveryCheckpoint
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::setRecoveryFull(bool bRecoveryFull_)
{
	m_cAttribute.m_bRecoveryFull = bRecoveryFull_;
}

// FUNCTION public
//	Schema::Database::isSlaveStarted -- SLAVEが開始中かを得る
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
Database::
isSlaveStarted() const
{
	return m_cAttribute.m_bSlaveStarted;
}

// FUNCTION public
//	Schema::Database::setSlaveStarted -- SLAVEが開始中かを設定する
//
// NOTES
//
// ARGUMENTS
//	bool bSlaveStarted_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
setSlaveStarted(bool bSlaveStarted_)
{
	m_cAttribute.m_bSlaveStarted = bSlaveStarted_;
}

// FUNCTION public
//	Schema::Database::isSlave -- SLAVEデータベースか否かを得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//		SLAVEデータベースの場合はtrue、それ以外の場合はfalse

bool
Database::isSlave() const
{
	return (getMasterURL().getLength() != 0) ? true : false;
}

// FUNCTION public
//	Schema::Database::getMasterURL -- MASTER URLを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

const ModUnicodeString&
Database::
getMasterURL() const
{
	return m_cAttribute.m_cstrMasterURL;
}

// FUNCTION public
//	Schema::Database::setMasterURL -- MASTER URLを設定する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrMasterURL_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
setMasterURL(const ModUnicodeString& cstrMasterURL_)
{
	m_cAttribute.m_cstrMasterURL = cstrMasterURL_;
}

//	FUNCTION public
//	Schema::Database::isUnmounted
//		-- データベースが UNMOUNT されたか問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : UNMOUNT された
//				false : UNMOUNT されていない
//
//	EXCEPTIONS

bool
Database::
isUnmounted() const
{
	return m_cAttribute.m_bUnmounted;
}

//	FUNCTION public
//	Schema::Database::isAccessible
//		-- データベースを構成するファイルが作成されているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : ファイルが作成されている
//				false : ファイルが作成されていない
//
//	EXCEPTIONS

bool
Database::
isAccessible() const
{
	// データファイルは作成遅延されているので、論理ログファイルがあるか否かで判断する
	return Utility::File::isFound(getLogFilePath());
}

//	FUNCTION public
//	Schema::Database::setAttritube --
//		データベースの属性を変更する
//
//	NOTES
//		変更により下位オブジェクトにも影響がある場合は
//		下位オブジェクトまでたどって変更する
//
//	ARGUMENTS
//		const Schema::Database::Attribute& cAttribute_
//			変更する属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setAttribute(const Attribute& cAttribute_)
{
	// 属性を設定しなおす
	m_cAttribute = cAttribute_;
}

//	FUNCTION public
//	Schema::Database::propagateAttribute --
//		データベースの属性変更に伴いファイルに関する処理をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		const Schema::Database::Attribute& cAttribute_
//			変更後のデータベースの属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
propagateAttribute(Trans::Transaction& cTrans_, const Attribute& cAttribute_)
{
	// ここで例外が発生すると元のAttributeで再度呼びなおされるので
	// この関数内でエラー処理する必要はない

	// 表以下のオブジェクトに変更を伝える
	loadTable(cTrans_).apply(ApplyFunction2<Table, Trans::Transaction&, const Database::Attribute&>
							 (&Table::propagateDatabaseAttribute, cTrans_, cAttribute_));

	// データベース以下のシステム表を構成するファイルに対しても変更を伝える
	SystemTable::Area(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::AreaContent(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Table(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Column(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Constraint(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Index(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Key(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::File(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Field(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Function(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Privilege(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Cascade(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);
	SystemTable::Partition(*this).propagateDatabaseAttribute(cTrans_, cAttribute_);

	// 論理ログファイルとシーケンスファイルもunmountする
	getSequence().unmount(cTrans_);

	cTrans_.setLog(*this);
	cTrans_.unmountLog(Trans::Log::File::Category::Database);

	// 論理ログファイルおよびシーケンスファイルの属性も変わるため、オブジェクトを一度クリアしておく
	// 次回必要になったときに作成しなおされる
	clearSequence();
}

//	FUNCTION public
//	Schema::Database::drop -- データベースの定義を抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database& cDatabase_
//			定義を抹消するデータベースオブジェクト
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			データベース抹消を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Database::
drop(Database& cDatabase_, LogData& cLogData_, Trans::Transaction& cTrans_)
{
	// データベースオブジェクトに破棄マークをつける
	cDatabase_.drop(cTrans_);
	// ログデータを作成する
	cDatabase_.makeLogData(cTrans_, cLogData_);
}

//	FUNCTION public
//	Schema::Database::drop -- 自分自身を表すデータベースの定義を抹消する
//
//	NOTES
//		永続化するまでは実際には抹消されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース抹消を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	// リカバリー処理のDROPでは実体のファイルの破棄が必要な場合は
	// UNDO処理で破棄されているので下位のオブジェクトにdropを伝播する必要はない

	if (!bRecovery_) {
		// データベースに属する表にすべて破棄マークをつける
		// 表以下のオブジェクトは表の破棄の中で破棄マークがつけられる
		loadTable(cTrans_).apply(ApplyFunction2<Table, Trans::Transaction&, bool>
								 (&Table::drop, cTrans_, bRecovery_));

		// データベースに属するエリアにすべて破棄マークをつける
		loadArea(cTrans_).apply(ApplyFunction3<Area, Trans::Transaction&, bool, bool>
								(&Area::drop, cTrans_, bRecovery_, false /* no check */));

		// データベースに属する関数にすべて破棄マークをつける
		loadFunction(cTrans_).apply(ApplyFunction2<Function, Trans::Transaction&, bool>
									(&Function::drop, cTrans_, bRecovery_));
	}

	// 作成中なら実体を破棄してしまう
	// ★注意★
	// 以前の実装ではMOUNT中でもdestroyを実行していたが
	// MOUNT中は消してはいけないだろう。
	Status::Value eStat = getStatus();
	if (eStat == Status::Created) {

		destroy(cTrans_, true);

	} else {
		// 自身の状態を変える
		Object::drop(bRecovery_);
	}
}

//	FUNCTION public
//	Schema::Database::undoDrop -- 破棄マークをクリアする
//
//	NOTES
//		破棄は論理ログに出力する前なら破棄マークをクリアすることで
//		復旧可能である
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
undoDrop(Trans::Transaction& cTrans_)
{
	// データベースに属する表のすべての破棄マークをクリアする
	// 表以下のオブジェクトは表のクリアの中で破棄マークがクリアされる
	loadTable(cTrans_).apply(ApplyFunction1<Table, Trans::Transaction&>
								 (&Table::undoDrop, cTrans_));

	// データベースに属するエリアのすべての破棄マークをクリアする
	loadArea(cTrans_).apply(ApplyFunction0<Area>(&Area::undoDrop));

	// データベースに属する関数のすべての破棄マークをクリアする
	loadFunction(cTrans_).apply(ApplyFunction0<Function>(&Function::undoDrop));

	// 自身の状態を変える
	Object::undoDrop();
}

//	FUNCTION public
//	Schema::Database::destroy --
//		データベースを構成するファイルとそれを格納するディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに即座に破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
destroy(Trans::Transaction& cTrans_, bool bForce_)
{
	// データベースに属する表を構成するファイルをすべて破棄する
	loadTable(cTrans_).apply(ApplyFunction3<Table, Trans::Transaction&, bool, bool>
							 (&Table::destroy, cTrans_, false /* not destroy area */, bForce_));

	// オブジェクトIDを生成するファイルを破棄する
	getSequence().drop(cTrans_, bForce_);

	// データベースに属するすべてのエリアを削除する
	loadArea(cTrans_).apply(ApplyFunction3<Area, Trans::Transaction&, const ModUnicodeString&, bool>
							(&Area::destroy, cTrans_, ModUnicodeString(), bForce_));

	if (getScope() == Scope::Permanent) {

		// このデータベースに関する論理ログを記録するファイルを破棄する
		// ★注意★
		// データベースがdropされるときは更新操作は行われないので
		// 論理ログファイルの破棄は常に即座に行って構わない
		// また、これより前にsetLogされていなければならない

		// 論理ログファイルを破棄する
		cTrans_.destroyLog(Trans::Log::File::Category::Database);
							// ファイルが存在しなくてもエラーにならないように
							// 実装されていなければならない

		// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
		// 削除する
		// ★注意★
		// 表以下のオブジェクトに対応するシステム表は
		// この処理でファイルごと消去されるので
		// システム表を構成するファイルの中身を
		// いちいち削除して回る必要はない

		SystemTable::Area(*this).drop(cTrans_);
		SystemTable::AreaContent(*this).drop(cTrans_);
		SystemTable::Table(*this).drop(cTrans_);
		SystemTable::Column(*this).drop(cTrans_);
		SystemTable::Constraint(*this).drop(cTrans_);
		SystemTable::Index(*this).drop(cTrans_);
		SystemTable::Key(*this).drop(cTrans_);
		SystemTable::File(*this).drop(cTrans_);
		SystemTable::Field(*this).drop(cTrans_);
		SystemTable::Function(*this).drop(cTrans_);
		SystemTable::Privilege(*this).drop(cTrans_);
		SystemTable::Cascade(*this).drop(cTrans_);
		SystemTable::Partition(*this).drop(cTrans_);
	}

	// データベースに指定されたパスの削除を行う

	{
		Admin::Utility::PathList cPathList;
		for (int i = 0; i < Path::Category::ValueNum; i++) {
			cPathList.addUnique(getPath(static_cast<Path::Category::Value>(i)));
		}
		Admin::Utility::PathList::Iterator iterator = cPathList.begin();
		const Admin::Utility::PathList::Iterator last = cPathList.end();
		for (; iterator != last; ++iterator) {
			Utility::File::rmAll(*(*iterator));
		}
	}
}

//	FUNCTION public
//	Schema::Database::destroy --
//		データベースを構成するあるパス以下のディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::DatabasePath::Category::Value eCategory_
//			破棄するディレクトリーのトップに当たるパスのカテゴリー
//		const ModUnicodeString& cPathPart_
//			破棄するディレクトリーの付加部分
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに即座に破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
destroy(Trans::Transaction& cTrans_,
		Path::Category::Value eCategory_, const ModUnicodeString& cPathPart_, bool bForce_)
{
	Utility::File::rmAll(getID(), getPath(eCategory_).addPart(cPathPart_), bForce_);
}

// FUNCTION public
//	Schema::Database::undoDestroy -- 指定したパス以下のディレクトリーの破棄を取り消す
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Path::Category::Value eCategory_
//	const ModUnicodeString& cPathPart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
undoDestroy(Trans::Transaction& cTrans_,
			Path::Category::Value eCategory_,
			const ModUnicodeString& cPathPart_)
{
	Utility::File::undoRmAll(getPath(eCategory_).addPart(cPathPart_));
}

//	FUNCTION public
//	Schema::Database::mountSystemTable -- Database の SystemTable ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		bool bUndo_ = false
//			trueのときUNDO処理なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
mountSystemTable(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	int iMounted = 0;
	try {
		// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
		// mount する
		for (int i = 0; i < Object::Category::ValueNum; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				if (i == Object::Category::File) {
					SCHEMA_FAKE_ERROR("Schema::Database", "Mount", "SystemTable");
				}

				pSystemFile->mount(cTrans_);
				iMounted = i + 1;
			}
		}

		// オブジェクトID の mount
		getSequence().mount(cTrans_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		for (int i = 0; i < iMounted; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				// mountしてしまった分をunmountする
				pSystemFile->unmount(cTrans_, true /* undo */);
			}
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::mount -- Database の下位ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		bool bUndo_ = false
//			trueのときUNDO処理なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
mount(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	enum {
		None,
		Tables,
		LogFile,
		Flag
	} eStatus = None;
	try {

		// データベースに属する表をすべて mount する
		// 表以下のオブジェクトは表の mount の中で mount される
		loadTable(cTrans_, true /* recovery */).apply(ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
														&Table::mount, cTrans_, getName(), bUndo_),
													  ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
														&Table::unmount, cTrans_, getName(), true /* undo */),
													  bUndo_);
		eStatus = Tables;
		SCHEMA_FAKE_ERROR("Schema::Database", "Mount", "Table");

		// このデータベースに関する論理ログを記録するファイルを mount する
		cTrans_.mountLog(Trans::Log::File::Category::Database);
		eStatus = LogFile;
		SCHEMA_FAKE_ERROR("Schema::Database", "Mount", "Log");

		// UNMOUNT されたフラグを OFF に設定する
		m_cAttribute.m_bUnmounted = false;
		eStatus = Flag;
		SCHEMA_FAKE_ERROR("Schema::Database", "Mount", "Flag");

		if (bUndo_) {
			// 取り消し処理の場合は削除フラグをクリアする
			Object::undoDrop();

		} else {
			// システム表の状態を変える
			SystemTable::setStatus(getDatabaseID(),
								   Object::Category::Database,
								   SystemTable::Status::Dirty);
		}

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Flag:
			m_cAttribute.m_bUnmounted = true;
			// thru.
		case LogFile:
			cTrans_.unmountLog(Trans::Log::File::Category::Database);
			// thru.
		case Tables:
			loadTable(cTrans_, true /* recovery */).apply(ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
														   &Table::unmount, cTrans_, getName(), true /* undo */));
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::unmountSystemTable --
//		システム表を構成するファイルをunmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		bool bUndo_ = false
//			trueのときUNDO処理なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
unmountSystemTable(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースはアンマウントできない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	int iUnmounted = 0;
	try {
		// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
		// unmount する
		for (int i = 0; i < Object::Category::ValueNum; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				pSystemFile->unmount(cTrans_);
				iUnmounted = i + 1;
			}
		}

		// オブジェクトID の unmount
		getSequence().unmount(cTrans_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		for (int i = 0; i < iUnmounted; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				// unmountしてしまった分をmountする
				pSystemFile->mount(cTrans_, true /* undo */);
			}
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::unmount --
//		unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		bool bUndo_ = false
//			trueのときUNDO処理なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
unmount(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースはアンマウントできない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	enum {
		None,
		Tables,
		LogFile,
		SystemTables,
		Flag
	} eStatus = None;

	try {

		// データベースに属する表をすべて unmount する
		// 表以下のオブジェクトは表の unmount の中で unmount される
		loadTable(cTrans_).apply(ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
								   &Table::unmount, cTrans_, getName(), bUndo_),
								 ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
								   &Table::mount, cTrans_, getName(), true /* undo */),
								 bUndo_);
		eStatus = Tables;

		// このデータベースに関する論理ログを記録するファイルを unmount する
		cTrans_.unmountLog(Trans::Log::File::Category::Database);
		eStatus = LogFile;

		// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
		// unmount する
		unmountSystemTable(cTrans_);
		eStatus = SystemTables;

		// UNMOUNT されたフラグを設定する
		m_cAttribute.m_bUnmounted = true;
		eStatus = Flag;

		// 自身の状態を「削除済」にする
		// 「マウント中」のときUNDOで呼ばれると状態がクリアされる
		Object::drop();

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Flag:
			m_cAttribute.m_bUnmounted = false;
			// thru.
		case SystemTables:
			mountSystemTable(cTrans_, true /* undo */);
			// thru.
		case LogFile:
			cTrans_.mountLog(Trans::Log::File::Category::Database);
			// thru.
		case Tables:
			loadTable(cTrans_).apply(ApplyFunction3<Table, Trans::Transaction&, const Name&, bool>(
										  &Table::mount, cTrans_, getName(), true /* undo */));
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::flush --
//		flush する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
flush(Trans::Transaction& cTrans_)
{
	// READ ONLYなデータベースは flush しない
	if (!isReadOnly()) {

		// データベースに属する表を構成するファイルをすべて flush する
		const TableMap& cMap = loadTable(cTrans_);

		// Databaseの参照カウンターが0になることはないので排他制御しなくても
		// TableMapが変化することはないはず
		cMap.apply(ApplyFunction1<Table, Trans::Transaction&>(&Table::flush, cTrans_));
	
		// オブジェクトIDを生成するファイルを flush する
		getSequence().flush(cTrans_);

		if (getScope() == Scope::Permanent) {

			// このデータベースに関する論理ログを記録するファイルを flush する
			cTrans_.flushLog(Trans::Log::File::Category::Database);

			// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
			// flush する

			SystemTable::Area(*this).flush(cTrans_);
			SystemTable::AreaContent(*this).flush(cTrans_);
			SystemTable::Table(*this).flush(cTrans_);
			SystemTable::Column(*this).flush(cTrans_);
			SystemTable::Constraint(*this).flush(cTrans_);
			SystemTable::Index(*this).flush(cTrans_);
			SystemTable::Key(*this).flush(cTrans_);
			SystemTable::File(*this).flush(cTrans_);
			SystemTable::Field(*this).flush(cTrans_);
			SystemTable::Function(*this).flush(cTrans_);
			SystemTable::Privilege(*this).flush(cTrans_);
			SystemTable::Cascade(*this).flush(cTrans_);
			SystemTable::Partition(*this).flush(cTrans_);
		}
	}
}

//	FUNCTION public
//	Schema::Database::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でデータベースを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でデータベースを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、データベースを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でデータベースを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でデータベースを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、データベースが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
sync(Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	if (getScope() == Scope::Permanent && !isReadOnly() && isOnline()) {

		// データベースを構成する表、索引、タプル ID ファイル
		const TableMap& cMap = loadTable(trans);

		// Databaseの参照カウンターが0になることはないので排他制御しなくても
		// TableMapが変化することはないはず

		cMap.apply(ApplyFunction3<Table, Trans::Transaction&, bool&, bool&>(&Table::sync, trans, incomplete, modified));

		// オブジェクト ID  ファイル

		getSequence().sync(trans, incomplete, modified);

		// システム表

		SystemTable::Area(*this).sync(trans, incomplete, modified);
		SystemTable::AreaContent(*this).sync(trans, incomplete, modified);
		SystemTable::Table(*this).sync(trans, incomplete, modified);
		SystemTable::Column(*this).sync(trans, incomplete, modified);
		SystemTable::Constraint(*this).sync(trans, incomplete, modified);
		SystemTable::Index(*this).sync(trans, incomplete, modified);
		SystemTable::Key(*this).sync(trans, incomplete, modified);
		SystemTable::File(*this).sync(trans, incomplete, modified);
		SystemTable::Field(*this).sync(trans, incomplete, modified);
		SystemTable::Function(*this).sync(trans, incomplete, modified);
		SystemTable::Privilege(*this).sync(trans, incomplete, modified);
		SystemTable::Cascade(*this).sync(trans, incomplete, modified);
		SystemTable::Partition(*this).sync(trans, incomplete, modified);
	}
}

//	FUNCTION public
//	Schema::Database::startBackup --バックアップを開始する
//
//	NOTES
//
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		bool bRestorable_ = true
//		bool bUndo_ = false
//
//	RETURN
//
//	EXCEPTIONS

void
Database::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_ /* = true */, bool bUndo_ /* = false */)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースはバックアップできない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	// データベースに属する表を構成するファイルをすべて startBackup する
	const TableMap& cMap = loadTable(cTrans_);

	enum {
		None,
		Tables,
		Sequence
	} eStatus = None;
	int iStarted = 0;

	try {
		// Databaseの参照カウンターが0になることはないので排他制御しなくても
		// TableMapが変化することはないはず
		cMap.apply(ApplyFunction3<Table, Trans::Transaction&, bool, bool>(&Table::startBackup, cTrans_, bRestorable_, bUndo_),
				   ApplyFunction1<Table, Trans::Transaction&>(&Table::endBackup, cTrans_));
		eStatus = Tables;

		// オブジェクトIDを生成するファイルを startBackup する
		getSequence().startBackup(cTrans_, bRestorable_);
		eStatus = Sequence;

		// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
		// startBackup する
		for (int i = 0; i < Object::Category::ValueNum; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				pSystemFile->startBackup(cTrans_, bRestorable_);
				iStarted = i + 1;
			}
		}
	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Sequence:
			{
				for (int i = 0; i < iStarted; ++i) {
					if (i != Object::Category::Database && i != Object::Category::Unknown) {

						// オブジェクトの種類に応じたシステム表オブジェクトを得る
						ModAutoPointer<SystemTable::SystemFile> pSystemFile =
							SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
						; _SYDNEY_ASSERT(pSystemFile.get());

						pSystemFile->endBackup(cTrans_);
					}
				}
				getSequence().endBackup(cTrans_);
			}
			// thru.
		case Tables:
			cMap.apply(ApplyFunction1<Table, Trans::Transaction&>(&Table::endBackup, cTrans_));
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::endBackup -- バックアップを終了する
//
//
//	NOTES
//
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//
//	EXCEPTIONS

void
Database::
endBackup(Trans::Transaction& cTrans_)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースはバックアップできない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	// データベースに属する表を構成するファイルをすべて endBackup する
	const TableMap& cMap = loadTable(cTrans_);

	// エラーが起きたらこのデータベースを使用不可にする
	Common::AutoCaller1<Database, bool> autoDisabler(this, &Database::setAvailability, false);

	// Databaseの参照カウンターが0になることはないので排他制御しなくても
	// TableMapが変化することはないはず
	cMap.apply(ApplyFunction1<Table, Trans::Transaction&>(&Table::endBackup, cTrans_));

	// オブジェクトIDを生成するファイルを endBackup する
	getSequence().endBackup(cTrans_);

	// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
	// endBackup する

	SystemTable::Area(*this).endBackup(cTrans_);
	SystemTable::AreaContent(*this).endBackup(cTrans_);
	SystemTable::Table(*this).endBackup(cTrans_);
	SystemTable::Column(*this).endBackup(cTrans_);
	SystemTable::Constraint(*this).endBackup(cTrans_);
	SystemTable::Index(*this).endBackup(cTrans_);
	SystemTable::Key(*this).endBackup(cTrans_);
	SystemTable::File(*this).endBackup(cTrans_);
	SystemTable::Field(*this).endBackup(cTrans_);
	SystemTable::Function(*this).endBackup(cTrans_);
	SystemTable::Privilege(*this).endBackup(cTrans_);
	SystemTable::Cascade(*this).endBackup(cTrans_);
	SystemTable::Partition(*this).endBackup(cTrans_);

	// 成功したのでエラー処理用のAutoCallerを無効にする
	autoDisabler.release();
}

//	FUNCTION public
//	Schema::Database::recoverSystemTable -- Database の SystemTable を障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
recoverSystemTable(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースは障害回復の対象にはならない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}
#ifndef SYD_COVERAGE
	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Recover [" << getName() << "(System Table)] : " << cPoint_
		<< ModEndl;
#endif
	// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
	// recover する

	SystemTable::Area(*this).recover(cTrans_, cPoint_);
	SystemTable::AreaContent(*this).recover(cTrans_, cPoint_);
	SystemTable::Table(*this).recover(cTrans_, cPoint_);
	SystemTable::Column(*this).recover(cTrans_, cPoint_);
	SystemTable::Constraint(*this).recover(cTrans_, cPoint_);
	SystemTable::Index(*this).recover(cTrans_, cPoint_);
	SystemTable::Key(*this).recover(cTrans_, cPoint_);
	SystemTable::File(*this).recover(cTrans_, cPoint_);
	SystemTable::Field(*this).recover(cTrans_, cPoint_);
	SystemTable::Function(*this).recover(cTrans_, cPoint_);
	SystemTable::Privilege(*this).recover(cTrans_, cPoint_);
	SystemTable::Cascade(*this).recover(cTrans_, cPoint_);
	SystemTable::Partition(*this).recover(cTrans_, cPoint_);

	// オブジェクトIDを生成するファイルを recover する
	getSequence().recover(cTrans_, cPoint_);

}

//	FUNCTION public
//	Schema::Database::recover --
//		データベースを障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//	bool bMount_ = false
//		trueのときMOUNT中の呼び出しなのでSystemTableのrecoverは必要に応じて呼ばれている
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_, bool bMount_ /* = false */)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースは障害回復の対象にはならない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	if (!bMount_) {
		// Mount中の呼び出し出なければシステム表の回復も行う
		recoverSystemTable(cTrans_, cPoint_);
	}
#ifndef SYD_COVERAGE
	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Recover [" << getName() << "(Database File)] : " << cPoint_
		<< ModEndl;
#endif
	// データベースに属する表を構成するファイルをすべて recover する
	loadTable(cTrans_).apply(ApplyFunction3<Table, Trans::Transaction&, const Trans::TimeStamp&, const Name&>
							 (&Table::recover, cTrans_, cPoint_, getName()));
}

//	FUNCTION public
//	Schema::Database::restoreSystemTable -- Database の SystemTable を障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
restoreSystemTable(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースは障害回復の対象にはならない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	// 表、列、制約、索引、キー、ファイル、フィールドのシステム表を
	// restore する

	SystemTable::Area(*this).restore(cTrans_, cPoint_);
	SystemTable::AreaContent(*this).restore(cTrans_, cPoint_);
	SystemTable::Table(*this).restore(cTrans_, cPoint_);
	SystemTable::Column(*this).restore(cTrans_, cPoint_);
	SystemTable::Constraint(*this).restore(cTrans_, cPoint_);
	SystemTable::Index(*this).restore(cTrans_, cPoint_);
	SystemTable::Key(*this).restore(cTrans_, cPoint_);
	SystemTable::File(*this).restore(cTrans_, cPoint_);
	SystemTable::Field(*this).restore(cTrans_, cPoint_);
	SystemTable::Function(*this).restore(cTrans_, cPoint_);
	SystemTable::Privilege(*this).restore(cTrans_, cPoint_);
	SystemTable::Cascade(*this).restore(cTrans_, cPoint_);
	SystemTable::Partition(*this).restore(cTrans_, cPoint_);

	// オブジェクトIDを生成するファイルを restore する
	getSequence().restore(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::Database::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	if (getScope() != Scope::Permanent) {
		// 永続でないデータベースは障害回復の対象にはならない
		_SYDNEY_THROW1(Exception::TemporaryDatabase, getName());
	}

	// データベースに属する表を構成するファイルをすべて restore する
	loadTable(cTrans_).apply(ApplyFunction2<Table, Trans::Transaction&, const Trans::TimeStamp&>
							 (&Table::restore, cTrans_, cPoint_));
}

//	FUNCTION public
//	Schema::Database::alter --
//		データベース定義変更の準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database& cDatabase_
//			対象のデータベースオブジェクト
//		const Statement::MoveDatabaseStatement& cStatement_
//			データベース格納場所変更のSQL文を表すクラス
//		ModVector<ModUnicodeString>& vecPrevPath_
//			ログ出力用の変更前のパスを入れるベクター
//		ModVector<ModUnicodeString>& vecPostPath_
//			ログ出力用の変更後のパスを入れるベクター
//		ModVector<bool>& vecChanged_
//			変更のあったカテゴリー
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//
//	RETURN
//		true ... 移動の必要がある
//		false... 移動の必要はない
//
//	EXCEPTIONS

// static
bool
Database::
alter(Database& cDatabase_,
	  const Statement::MoveDatabaseStatement& cStatement_,
	  ModVector<ModUnicodeString>& vecPrevPath_,
	  ModVector<ModUnicodeString>& vecPostPath_,
	  ModVector<bool>& vecChanged_,
	  LogData& cLogData_,
	  Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// SQL文からパス指定変更指定を取得する
	bool bSet = (cStatement_.getType() == Statement::MoveDatabaseStatement::Set);
										// SetかDropかを表す
	int iChanged = 0;

	// 引数の配列にすべて初期値を入れておく
	const int nPathCategory = static_cast<int>(Path::Category::ValueNum);
	vecPrevPath_.assign(nPathCategory);
	vecPostPath_.assign(nPathCategory);
	vecChanged_.assign(nPathCategory, false);

	const Statement::DatabasePathElementList* pPathList = cStatement_.getPathList();
	; _SYDNEY_ASSERT(pPathList);

	int n = pPathList->getCount();
	for (int i = 0; i < n; i++) {

		// Statementからパス指定部分を取り出す

		const Statement::DatabasePathElement* pElement = pPathList->getPathElementAt(i);
		; _SYDNEY_ASSERT(pElement);

		Statement::Literal* pLiteral = pElement->getPathName();
		; _SYDNEY_ASSERT(pLiteral);

		Common::Data::Pointer pValue = pLiteral->createData();
		Common::StringData* pString =
			_SYDNEY_DYNAMIC_CAST(Common::StringData*, pValue.get());
		; _SYDNEY_ASSERT(pString);

		Path::Category::Value eCategory = Path::Category::ValueNum;

		// パス指定の種別に応じてSchemaでの種別を設定する

		switch (pElement->getPathType()) {
		case Statement::DatabasePathElement::Database:
		{
			eCategory = Path::Category::Data;
			break;
		}
		case Statement::DatabasePathElement::LogicalLog:
		{
			eCategory = Path::Category::LogicalLog;
			break;
		}
		case Statement::DatabasePathElement::System:
		{
			eCategory = Path::Category::System;
			break;
		default:
			; _SYDNEY_ASSERT(false);
			break;
		}
		}

		int iCategory = static_cast<int>(eCategory);

		// 以前の実装ではここで変更のあるカテゴリーについてのみ
		// Vectorにセットしていたが、Undo/Redoの仕様変更により
		// 変更後のパス指定を1つのログで知る必要があるため
		// 変更のあるなしに関係なくVectorにセットすることにする

		// 指定されたカテゴリーの現在の設定
		const ModUnicodeString* pPathDefinition
			= _Path::_getPathDefinition(eCategory, cDatabase_.m_pPath);

		// 変更前のパス指定をセットする
		if (pPathDefinition) {
			vecPrevPath_[iCategory] = *pPathDefinition;
		}
		// 変更後のパス指定をセットする
		if (bSet) {
			vecPostPath_[iCategory] = pString->getValue();
		}
		// 変更があるか調べる
		if (!_Path::_isIdentical(_Path::_getFullPathName(eCategory, vecPrevPath_[iCategory]),
								 _Path::_getFullPathName(eCategory, vecPostPath_[iCategory]))) {
			vecChanged_[iCategory] = true;
			iChanged++;
		}
	}

	if (iChanged == 0) {
		// 移動する必要がなければここで終了
		// ★注意★
		// Reorganizeでは返り値がfalseなら何もしない
		return false;
	}

	// ログデータから移動後のパスを知る必要があるので
	// 変更のなかったところは現在の値を入れておく
	for (int iPathCategory = 0; iPathCategory < nPathCategory; ++iPathCategory) {
		if (!vecChanged_[iPathCategory]) {
			const ModUnicodeString* pPathDefinition =
				_Path::_getPathDefinition(static_cast<Path::Category::Value>(iPathCategory),
										  cDatabase_.m_pPath);
			if (pPathDefinition) {
				vecPostPath_[iPathCategory] = vecPrevPath_[iPathCategory] =
					*pPathDefinition;
			}
		}
	}

	// ログデータを作成する
	cDatabase_.makeLogData(cTrans_, cLogData_);
	// パスリストを追加する
	cLogData_.addStrings(vecPrevPath_);
	cLogData_.addStrings(vecPostPath_);
	; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Move::Num);

	return true;
}

//	FUNCTION public
//	Schema::Database::alter -- データベース定義の変更の準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database& cDatabase_
//			変更対象のデータベースオブジェクト
//		const Statement::AlterDatabaseStatement& cStatement_
//			alter database statement
//		Schema::Database::Attribute&	anteAttr
//			変更前の属性が設定される
//		Schema::Database::Attribute&	postAttr
//			変更後の属性が設定される
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction&	trans
//			データベース定義の変更を行う
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			データベース定義の変更の必要がある
//		false
//			データベース定義の変更の必要はない
//
//	EXCEPTIONS
//		なし

// static
bool
Database::
alter(Database& cDatabase_,
	  const Statement::AlterDatabaseStatement& cStatement_,
	  Attribute& cPrevAttribute_, Attribute& cPostAttribute_,
	  LogData& cLogData_,
	  Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(
		 cTrans_.getCategory() == Trans::Transaction::Category::ReadWrite);

	// Set database attribute before altering
	cPrevAttribute_ = cDatabase_.m_cAttribute;

	// Set database attribute specified by alter statement
	const Boolean::Value iReadWrite =
		cStatement_.getAlterDatabaseOption(Statement::AlterDatabaseAttribute::ReadWrite);
	const Boolean::Value iOnline =
		cStatement_.getAlterDatabaseOption(Statement::AlterDatabaseAttribute::Online);
	const Boolean::Value iRecovery =
		cStatement_.getAlterDatabaseOption(Statement::AlterDatabaseAttribute::RecoveryFull);

	const Boolean::Value iSuperUserMode =
		cStatement_.getAlterDatabaseOption(Statement::AlterDatabaseAttribute::SuperUserMode);

	const Statement::AlterDatabaseStatement::ReplicationType iReplicationType =
		cStatement_.getReplicationType();
	
	cPostAttribute_.m_bReadOnly =
		((iReadWrite == Boolean::Unknown) ?
		 cPrevAttribute_.m_bReadOnly : (iReadWrite == Boolean::False));
	cPostAttribute_.m_bOnline =
		((iOnline == Boolean::Unknown) ?
		 cPrevAttribute_.m_bOnline : (iOnline == Boolean::True));
	cPostAttribute_.m_bRecoveryFull =
		((iRecovery == Boolean::Unknown) ?
		 cPrevAttribute_.m_bRecoveryFull : (iRecovery == Boolean::True));
	
	cPostAttribute_.m_bSuperUserMode =
		((iSuperUserMode == Boolean::Unknown) ?
		 cPrevAttribute_.m_bSuperUserMode : (iSuperUserMode == Boolean::True));

	cPostAttribute_.m_cstrMasterURL =
		((iReplicationType == Statement::AlterDatabaseStatement::SetToMaster) ?
		 ModUnicodeString() : cPrevAttribute_.m_cstrMasterURL);

	cPostAttribute_.m_bSlaveStarted =
		((iReplicationType == Statement::AlterDatabaseStatement::StartSlave) ?
		 true :
		 ((iReplicationType == Statement::AlterDatabaseStatement::StopSlave
		   || iReplicationType == Statement::AlterDatabaseStatement::SetToMaster) ?
		  false :
		  cPrevAttribute_.m_bSlaveStarted));

	if (cPrevAttribute_.m_cstrMasterURL.getLength() == 0
		&& cPostAttribute_.m_bSlaveStarted != cPrevAttribute_.m_bSlaveStarted) {
		// Can't change slave status in master server
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (cPrevAttribute_.m_cstrMasterURL.getLength() != 0)
	{
		// スレーブデータベースの属性は変更できない
		if (iReadWrite != Boolean::Unknown ||
			iOnline != Boolean::Unknown ||
			iRecovery != Boolean::Unknown ||
			iSuperUserMode != Boolean::Unknown)
			
			_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (cPostAttribute_.m_bReadOnly == true &&
		cStatement_.isDiscardLogicalLog() == true)
		// 読み込み専用データベースでは、論理ログの削除はできない
		_SYDNEY_THROW0(Exception::NotSupported);

	// return value is the boolean whether any attributes are changed
	bool bResult
		= !((cPrevAttribute_.m_bReadOnly == cPostAttribute_.m_bReadOnly) &&
			(cPrevAttribute_.m_bOnline == cPostAttribute_.m_bOnline) &&
			(cPrevAttribute_.m_bRecoveryFull == cPostAttribute_.m_bRecoveryFull)&&
			(cPrevAttribute_.m_bSuperUserMode == cPostAttribute_.m_bSuperUserMode)&&
			(cPrevAttribute_.m_bSlaveStarted == cPostAttribute_.m_bSlaveStarted)&&
			(cPrevAttribute_.m_cstrMasterURL == cPostAttribute_.m_cstrMasterURL));
	
	if (bResult) {
		// when altering includes read write -> read only,
		// change the log data category
		if (!cPrevAttribute_.m_bReadOnly && cPostAttribute_.m_bReadOnly) {
			cLogData_.setSubCategory(LogData::Category::AlterDatabase_ReadOnly);
		}
		// when altering includes slave -> master,
		// change the log data category
		if (cPrevAttribute_.m_cstrMasterURL.getLength() != 0 &&
			cPostAttribute_.m_cstrMasterURL.getLength() == 0)
		{
			cLogData_.setSubCategory(
				LogData::Category::AlterDatabase_SetToMaster);
		}
		// create log data
		cDatabase_.makeLogData(cTrans_, cLogData_);
		// add flag value for the attribute after altering
		cLogData_.addUnsignedInteger(cPostAttribute_.getFlag());
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Alter::Num);

		// add MasterURL
		cLogData_.addString(cPostAttribute_.m_cstrMasterURL);
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Alter::Num2);
	}
	return bResult;
}

//	FUNCTION public
//	Schema::Database::alter --
//		データベース定義変更REDOの準備をする
//
//	NOTES
//		redo用
//
//	ARGUMENTS
//		Schema::Database& cDatabase_
//			変更対象のデータベースオブジェクト
//		const Schema::LogData& cLogData_
//			データベース変更のログデータ
//		Schema::Database::Attribute& bPrevAttribute_,
//			変更前の属性
//		Schema::Database::Attribute& bPostAttribute_,
//			変更後の属性
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//
//	RETURN
//		つねにtrue
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログの内容が不正である
//		Exception::DatabaseNotFound
//			指定された名前のデータベースはない

// static
bool
Database::
alter(const LogData& cLogData_,
	  Attribute& cPrevAttribute_,
	  Attribute& cPostAttribute_,
	  Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// ログの内容から属性を得る
	cPostAttribute_.setFlag(cLogData_.getUnsignedInteger(Log::Alter::Flag));

	if (cLogData_.getCount() > Log::Alter::MasterURL) {
		// set master URL
		cPostAttribute_.m_cstrMasterURL = cLogData_.getString(Log::Alter::MasterURL);
	}

	return true;
}

//	FUNCTION public
//	Schema::Database::move --
//		データベースを構成するファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//			変更前のパスが入っているベクター
//		const ModVector<ModUnicodeString>& vecPostPath_
//			変更後のパスが入っているベクター
//		bool bUndo_ = false
//			trueならSystemTable::undoから呼ばれていることを意味する
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
move(Trans::Transaction& cTrans_,
	 ModVector<ModUnicodeString>& vecPrevPath_,
	 ModVector<ModUnicodeString>& vecPostPath_,
	 bool bUndo_, bool bRecovery_)
{
	enum {
		None,
		Moved,
		ValueNum
	} eStatus = None;

	int iCategory = static_cast<int>(Path::Category::Data);

	const int nPathCategory = static_cast<int>(Path::Category::ValueNum);
	try {
		for (; iCategory < nPathCategory; ++iCategory) {

			// 変更のあるカテゴリーのみ処理すればよい
			const ModUnicodeString& cstrPrevPath = vecPrevPath_[iCategory];
			const ModUnicodeString& cstrNewPath = vecPostPath_[iCategory];

			// まずは単純に文字列で比較する
			if (cstrPrevPath != cstrNewPath) {
				Path::Category::Value eCategory = static_cast<Path::Category::Value>(iCategory);

				Os::Path cPrevPath(_Path::_getPath(eCategory, &cstrPrevPath, this));
				Os::Path cNewPath(_Path::_getPath(eCategory, &cstrNewPath, this));

				// 無指定時のパスが得られた状態でちゃんと比較する
				if (!_Path::_isIdentical(cPrevPath, cNewPath)) {

					// パスカテゴリーの値によって処理が異なる
					switch (eCategory) {
					case Path::Category::Data:
						{
							// データを格納するパス指定が変更された
							// → 表、索引を構成するファイルを移動する
							moveData(cTrans_, cPrevPath, cNewPath, bUndo_, bRecovery_);
							break;
						}
					case Path::Category::LogicalLog:
						{
							// 論理ログファイルを格納するパス指定が変更された
							// → 論理ログファイルを移動する
							moveLogicalLog(cTrans_, cPrevPath, cNewPath, bUndo_, bRecovery_);
							break;
						}
					case Path::Category::System:
						{
							// システム表を格納するパス指定が変更された
							// → システム表を移動する
							moveSystem(cTrans_, cPrevPath, cNewPath, bUndo_, bRecovery_);
							break;
						}
					default:
						; _SYDNEY_ASSERT(false);
						break;
					}

					// 移動したら対応するカテゴリーのパス指定を書き換えておく
					setPath(eCategory, cstrNewPath);
				}
			}
		}

		eStatus = Moved;
		SCHEMA_FAKE_ERROR("Schema::Database", "Move", "Moved");

		// カテゴリーの指定によっては呼ばれないFakeErrorがあるのでここでも設定しておく
		// (テストスクリプトが常に失敗することを前提にしている)
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveData", "Moved");
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveData", "MovedFatal");
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveSystem", "ObjectID");
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveSystem", "Moved");
		SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Directory");
		SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "RowID");
		SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Moved");
		SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Removed");
		SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Created");
		SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Moved");
		SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Removed");
		SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "FileIDSet");
		SCHEMA_FAKE_ERROR("Schema::File", "MovePath", "Touched");
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "DirectoryCreated");
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "Record");
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "Index");
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "DirectoryRemoved");
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "SetPath");
		SCHEMA_FAKE_ERROR("Schema::SystemFileSub", "Move", "Moved");
		SCHEMA_FAKE_ERROR("Schema::SystemFileSub", "Move", "SetPath");

		// 移動前のディレクトリーを削除する
		_Path::_sweep(cTrans_, this, vecPrevPath_, vecPostPath_);
		SCHEMA_FAKE_ERROR("Schema::Database", "Move", "Removed");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Moved:
		case None:
		default:
			{
				for (int iErr = static_cast<int>(Path::Category::Data);
					 iErr < iCategory; ++iErr) {
					// 移動してしまったカテゴリーについて元に戻す
					// 変更のあるカテゴリーのみ処理すればよい

					const ModUnicodeString& cstrPrevPath = vecPrevPath_[iErr];
					const ModUnicodeString& cstrNewPath = vecPostPath_[iErr];

					if (cstrPrevPath != cstrNewPath) {
						Path::Category::Value eCategory = static_cast<Path::Category::Value>(iErr);

						Os::Path cPrevPath(_Path::_getPath(eCategory, &cstrPrevPath, this));
						Os::Path cNewPath(_Path::_getPath(eCategory, &cstrNewPath, this));

						if (!_Path::_isIdentical(cPrevPath, cNewPath)) {
							// パスカテゴリーの値によって処理が異なる
							// エラー処理中は引数にUNDO中であることを示すtrueを加える

							switch (eCategory) {
							case Path::Category::Data:
								{
									moveData(cTrans_, cNewPath, cPrevPath, true);
									break;
								}
							case Path::Category::LogicalLog:
								{
									moveLogicalLog(cTrans_, cNewPath, cPrevPath, true);
									break;
								}
							case Path::Category::System:
								{
									moveSystem(cTrans_, cNewPath, cPrevPath, true);
									break;
								}
							default:
								; _SYDNEY_ASSERT(false);
								break;
							}

							// 対応するカテゴリーのパス指定を戻しておく
							setPath(eCategory, cstrPrevPath);
						}
					}
				}
				break;
			}
		}

		// 移動後のディレクトリーを削除する
		_Path::_sweep(cTrans_, this, vecPostPath_, vecPrevPath_);

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Database::undoMove --
//		データベースを構成するファイルの移動を取り消す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//			変更前のパスが入っているベクター
//		const ModVector<ModUnicodeString>& vecPostPath_
//			変更後のパスが入っているベクター
//		bool bTouch_
//			trueの場合Fileが永続化された後なのでtouchする
//			falseの場合Fileが永続化される前なのでuntouchする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
undoMove(Trans::Transaction& cTrans_,
		 const ModVector<ModUnicodeString>& vecPrevPath_,
		 const ModVector<ModUnicodeString>& vecPostPath_)
{
	ModVector<Os::Path> vecRemovePath;
	int iCategory = static_cast<int>(Path::Category::Data);
	for (; iCategory < Path::Category::ValueNum; ++iCategory) {

		const ModUnicodeString& cstrPrevPath = vecPrevPath_[iCategory];
		const ModUnicodeString& cstrPostPath = vecPostPath_[iCategory];
		if (cstrPrevPath != cstrPostPath) {
			Path::Category::Value eCategory = static_cast<Path::Category::Value>(iCategory);

			// 移動してしまったカテゴリーはパス指定が移動後のものにされているので
			// 移動前とパス指定が異なるものを処理する

			const ModUnicodeString* pPathDefinition =
				_Path::_getPathDefinition(eCategory, m_pPath);

			if ((!pPathDefinition && cstrPrevPath.getLength() != 0)
				|| (pPathDefinition && cstrPrevPath != *pPathDefinition)) {

				Os::Path cPrevPath(_Path::_getPath(eCategory, &cstrPrevPath, this));
				Os::Path cPathDefinition(_Path::_getPath(eCategory, pPathDefinition, this));

				if (!_Path::_isIdentical(cPrevPath, cPathDefinition)) {
					// パスカテゴリーの値によって処理が異なる
					switch (eCategory) {
					case Path::Category::Data:
					{
						// データを格納するパス指定が変更された
						// → 表、索引を構成するファイルの移動を取り消す
						moveData(cTrans_, cPathDefinition, cPrevPath, true);
						break;
					}
					case Path::Category::LogicalLog:
					{
						// 論理ログファイルを格納するパス指定が変更された
						// → 論理ログファイルの移動を取り消す
						moveLogicalLog(cTrans_, cPathDefinition, cPrevPath, true);
						break;
					}
					case Path::Category::System:
					{
						// システム表を格納するパス指定が変更された
						// → システム表の移動を取り消す
						moveSystem(cTrans_, cPathDefinition, cPrevPath, true);
						break;
					}
					default:
						; _SYDNEY_ASSERT(false);
						break;
					}
					// 移動先だったディレクトリーを破棄するリストに加える
					ModVector<Os::Path>::Iterator iterator = vecRemovePath.find(cPathDefinition);
					if (iterator == vecRemovePath.end()) {
						vecRemovePath.pushBack(cPathDefinition);
					}
					// 移動したら対応するカテゴリーのパス指定を戻しておく
					setPath(eCategory, cstrPrevPath);
				}
			}
		}
	}

	// 移動先だったディレクトリーを破棄する
	ModVector<Os::Path>::Iterator iterator = vecRemovePath.begin();
	const ModVector<Os::Path>::Iterator& end = vecRemovePath.end();
	for (; iterator != end; ++iterator) {
		Utility::File::rmAll(*iterator);
	}

	// 移動が完了したらデータベースの変更を指定する
	if (getStatus() != Status::Persistent) {
		// UNDO中は永続化状態でなければuntouchする
		untouch();
	} else {
		touch();
	}
}

//	FUNCTION public
//	Schema::Database::get --
//		あるスキーマオブジェクト ID のデータベースを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value		id
//			データベースのスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bForce_ = false
//			trueのとき、Availabilityのチェックをしない
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のデータベースは存在しない
//
//	EXCEPTIONS

// static
Database*
Database::
get(ID::Value id, Trans::Transaction& cTrans_, bool bForce_)
{
	if (id == Schema::Object::ID::Invalid)
		return 0;

	// メタデータベースのIDならメタデータベースを返す
	if (id == ID::SystemTable)
		return SystemDatabase::getInstance(cTrans_);

	Database* pRet = Manager::ObjectTree::Database::get(id, cTrans_);

	// 利用可能かチェックする
	if ( !bForce_ && pRet && !pRet->isAvailable() )
		_SYDNEY_THROW1(Exception::DatabaseNotAvailable, pRet->getName());

	return pRet;
}

//	FUNCTION public
//	Schema::Database::getLocked --
//		ある名前のデータベースを表すクラスを
//		適切なロックをかけながら得る
//
//	NOTES
//		システム表のタプルにロックをかける
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Object::Name&		cName_
//			データベースの名前
//		Lock::Name::Category::Value eManipulate_ = Category::Tuple
//			操作対象の種類
//		Hold::Operation::Value eOperation_ = Operation::ReadWrite
//			データベースに対して行おうとしている処理を表す値
//		bool bForce_ = false
//			trueのとき、Availabilityのチェックをしない
//		Lock::Timeout::Value iTimeout_
//			ロック待ちタイムアウト
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//		0
//			指定された名前のデータベースは存在しない
//
//	EXCEPTIONS

// static
Database*
Database::
getLocked(Trans::Transaction& cTrans_, const Name& cName_,
		  Lock::Name::Category::Value eManipulate_,
		  Hold::Operation::Value eOperation_,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{
	// データベース名からデータベースIDを取得する
	ID::Value nID = Database::getID(cName_, cTrans_, bForce_);
	if ( nID == ObjectID::Invalid ) {
		return 0;
	}
	return getLocked(cTrans_, nID, eManipulate_, eOperation_, bForce_, iTimeout_);
}

//	FUNCTION public
//	Schema::Database::getLocked --
//		ある名前のデータベースを表すクラスを
//		適切なロックをかけながら得る
//
//	NOTES
//		システム表のタプルにロックをかける
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value	iID_
//			データベースのID
//		Lock::Name::Category::Value eManipulate_ = Category::Tuple
//			操作対象の種類
//		Hold::Operation::Value eOperation_ = Operation::ReadWrite
//			データベースに対して行おうとしている処理を表す値
//		bool bForce_ = false
//			trueのとき、Availabilityのチェックをしない
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//		0
//			指定された名前のデータベースは存在しない
//
//	EXCEPTIONS

// static
Database*
Database::
getLocked(Trans::Transaction& cTrans_, ID::Value iID_,
		  Lock::Name::Category::Value eManipulate_,
		  Hold::Operation::Value eOperation_,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{

	Database::checkSuperUserModeState(iID_);
	
	if(iTimeout_ == Lock::Timeout::Unlimited)
	{
		// 無限待ちの場合は100ms間隔で
		// スーパーユーザモード状態と終了状態をチェックする
		// システム表のタプルにロックする
		while (!Manager::SystemTable::hold(cTrans_,
										   Hold::Target::MetaTuple,
										   eManipulate_,
										   eOperation_,
										   iID_,
										   Trans::Log::File::Category::System,
										   0,
										   100))
		{
			Database::checkSuperUserModeState(iID_);
			if(cTrans_.isCanceledStatement())
			{
				_SYDNEY_THROW0(Exception::Cancel);
			}
		}
	}
	else
	{
		if (!Manager::SystemTable::hold(cTrans_,
										   Hold::Target::MetaTuple,
										   eManipulate_,
										   eOperation_,
										   iID_,
										   Trans::Log::File::Category::System,
										   0,
										iTimeout_))
		{
			if(iTimeout_!=Lock::Timeout::Unlimited)
			{
				// ロックがタイムアウトした
				_SYDNEY_THROW0(Exception::LockTimeout);
			}
		}
	}

	// IDからデータベースのスキーマオブジェクトを取得
	Database* pcDatabase = Database::get(iID_, cTrans_, bForce_);
	return pcDatabase;
}

// static
Database*
Database::
getLocked(Trans::Transaction& trans, const Name& name,
		  Lock::Name::Category::Value manipulateDBandTable,
		  Hold::Operation::Value operationDBandTable,
		  Lock::Name::Category::Value manipulateTuple,
		  Hold::Operation::Value operationTuple,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{
	return getLocked(trans, name,
					 manipulateDBandTable, operationDBandTable,
					 manipulateDBandTable, operationDBandTable,
					 manipulateTuple, operationTuple,
					 bForce_,
					 iTimeout_);
}

// static
Database*
Database::
getLocked(Trans::Transaction& trans, ID::Value iID_,
		  Lock::Name::Category::Value manipulateDBandTable,
		  Hold::Operation::Value operationDBandTable,
		  Lock::Name::Category::Value manipulateTuple,
		  Hold::Operation::Value operationTuple,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{
	return getLocked(trans, iID_,
					 manipulateDBandTable, operationDBandTable,
					 manipulateDBandTable, operationDBandTable,
					 manipulateTuple, operationTuple,
					 bForce_,
					 iTimeout_);
}

// static
Database*
Database::
getLocked(Trans::Transaction& trans, const Name& name,
		  Lock::Name::Category::Value manipulateDB,
		  Hold::Operation::Value operationDB,
		  Lock::Name::Category::Value manipulateTable,
		  Hold::Operation::Value operationTable,
		  Lock::Name::Category::Value manipulateTuple,
		  Hold::Operation::Value operationTuple,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{
	// メタデータベースとデータベース表をロックする
	bool bLocked = true;
	if (trans.isNoVersion()) {
		// 版を使うトランザクションは最初にMetaDatabaseをロックしているので後でconvertする
		bLocked = Manager::SystemTable::hold(trans, Hold::Target::MetaDatabase,
											 manipulateDB, operationDB,
											 Object::ID::Invalid,
											 Trans::Log::File::Category::System,
											 0,
											 iTimeout_);
	}
	if (bLocked
		&& Manager::SystemTable::hold(trans, Hold::Target::MetaTable,
									  manipulateTable, operationTable,
									  Object::ID::Invalid,
									  Trans::Log::File::Category::System,
									  0,
									  iTimeout_)) {
		Database* pDatabase =
			getLocked(trans, name, manipulateTuple, operationTuple, bForce_, iTimeout_);

		if (trans.isNoVersion() == false) {
			Manager::SystemTable::convert(trans, Hold::Target::MetaDatabase,
										  Lock::Name::Category::Database,
										  Hold::Operation::ReadOnly,
										  manipulateDB, operationDB);
		}
		return pDatabase;
	}
	// ロック待ちがタイムアウトした
	_SYDNEY_THROW0(Exception::LockTimeout);
}

// static
Database*
Database::
getLocked(Trans::Transaction& trans, ID::Value iID_,
		  Lock::Name::Category::Value manipulateDB,
		  Hold::Operation::Value operationDB,
		  Lock::Name::Category::Value manipulateTable,
		  Hold::Operation::Value operationTable,
		  Lock::Name::Category::Value manipulateTuple,
		  Hold::Operation::Value operationTuple,
		  bool bForce_,
		  Lock::Timeout::Value iTimeout_)
{
	// メタデータベースとデータベース表をロックする

	if (Manager::SystemTable::hold(trans, Hold::Target::MetaDatabase,
								   manipulateDB, operationDB,
								   Object::ID::Invalid,
								   Trans::Log::File::Category::System,
								   0,
								   iTimeout_)
		&& Manager::SystemTable::hold(trans, Hold::Target::MetaTable,
									  manipulateTable, operationTable,
									  Object::ID::Invalid,
									  Trans::Log::File::Category::System,
									  0,
									  iTimeout_)) {
		return getLocked(trans, iID_, manipulateTuple, operationTuple, bForce_, iTimeout_);
	}
	// ロックタイムアウト
	_SYDNEY_THROW0(Exception::LockTimeout);
}

//	FUNCTION public
//	Schema::Database::getID -- ある名前のデータベースのスキーマオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&			name
//			データベースの名前
//			長さが0のときはシステムが定義するデフォルトの名前を用いる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bForce_ = false
//			trueのとき、Availabilityのチェックをしない
//
//	RETURN
//		ID::Invalid 以外の値
//			得られたデータベースのスキーマオブジェクトID
//		ID::Invalid
//			指定された名前のデータベースは存在しない
//
//	EXCEPTIONS

// static
Object::ID::Value
Database::
getID(const Name& name, Trans::Transaction& cTrans_, bool bForce_)
{
	// 今まではnameが長さ0でなければ一時データベースの可能性を考えていたが
	// getTemporaryで得るようにしたのでもはや考えなくてよい。

	// 長さ0ならデフォルトのデータベース名を使う

	Object::ID::Value ret 
		= name.getLength()
			? Manager::ObjectTree::Database::getID(name, cTrans_)
			: Manager::ObjectTree::Database::getID(Name(NameParts::Database::Default), cTrans_);

	// 利用可能かチェックする
	if ( !bForce_
		 && ret != ObjectID::Invalid
		 && !Database::isAvailable(ret) )
		_SYDNEY_THROW1(Exception::DatabaseNotAvailable, name);

	return ret;
}

//	FUNCTION public
//	Schema::Database::getTemporary -- 一時データベースを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Database*
Database::
getTemporary(Trans::Transaction& cTrans_)
{
	return TemporaryDatabase::get(cTrans_);
}

//	FUNCTION public
//	Schema::Database::createTemporary -- 一時データベースを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Database*
Database::
createTemporary(Trans::Transaction& cTrans_)
{
	return TemporaryDatabase::create(cTrans_);
}

//	FUNCTION public
//	Schema::Database::dropTemporary -- 一時データベースを表すクラスを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID iSessionID_
//			セッションID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
dropTemporary(Server::SessionID iSessionID_)
{
	TemporaryDatabase::drop(iSessionID_);
}


//	FUNCTION public
//	Schema::Database::entrySuperUserModeTransitionalState -- スーパーユーザモードへの移行を開始します。 
//
//	NOTES
//
//	ARGUMENTS
//		Object::Name& cName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
enterSuperUserModeTransitionalState(const Object::Name& cName_, const ID::Value iID_)
{
	Os::AutoCriticalSection cAuto(m_cUserModeMapLatch);
	
	SuperUserModeIdMap::Iterator i = m_cUserModeIdMap.find(iID_);
	if (i == m_cUserModeIdMap.end()) {
		m_cUserModeIdMap.insert(iID_, ModThisThread::getThread()->getThreadId());
		m_cUserModeNameMap.insert(cName_, ModThisThread::getThread()->getThreadId());
	} else {
		// 移行状態に入る前にデータベース表のUロックを取得しているため
		// 同じデータベースに対して同時にスーパーユーザーモードに移行することはない
		_SYDNEY_ASSERT(false);
	}

	// マージデーモンを止める
	LogicalFile::FileDriverManager::
		getDriver(LogicalFile::FileDriverID::FullText)->stop();
}


//	FUNCTION public
//	Schema::Database::exitSuperUserModeTransitionalState -- スーパーユーザモードへの移行を完了します。
//
//	NOTES
//
//	ARGUMENTS
//		Object::Name& cName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
exitSuperUserModeTransitionalState(const Object::Name& cName_, const ID::Value iID_)
{
	Os::AutoCriticalSection cAuto(m_cUserModeMapLatch);
	m_cUserModeIdMap.erase(iID_);
	m_cUserModeNameMap.erase(cName_);


	// マージデーモンを再開させる
	LogicalFile::FileDriverManager::getDriver(LogicalFile::FileDriverID::FullText)->start();
}



//	FUNCTION public
//	Schema::Database::checkSuperUserModeState -- スーパーユーザーモードかのチェックを行う
//
//	NOTES
//
//	ARGUMENTS
//		Object::Name& cName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	別スレッドがスーパーユーザーモード移行中の場合:Canceled
// static
void
Database::
checkSuperUserModeState(const ID::Value iID_)
{
	Os::AutoCriticalSection cAuto(m_cUserModeMapLatch);
	
	SuperUserModeIdMap::Iterator i = m_cUserModeIdMap.find(iID_);
	if (i != m_cUserModeIdMap.end()) {
		if((*i).second != ModThisThread::getThread()->getThreadId()) {
			// 別スレッドがスーパーユーザモード移行中
			_SYDNEY_THROW0(Exception::Cancel);
		}
	}
}


//	FUNCTION public
//	Schema::Database::isSuperUserMode -- スーパーユーザーモードかのチェックを行う
//
//	NOTES
//
//	ARGUMENTS
//		Object::Name& cName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
bool
Database::
isSuperUserMode(const Object::Name& cName_)
{
	Os::AutoCriticalSection cAuto(m_cUserModeMapLatch);
	
	SuperUserModeNameMap::Iterator i = m_cUserModeNameMap.find(cName_);
	if(i != m_cUserModeNameMap.end()) {
		return true;
	} else {
		return false;
	}
}


//	FUNCTION private
//	Schema::Database::getLogFilePath --
//		データベースの更新操作を記録する論理ログファイルのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ログファイルのパス名
//
//	EXCEPTIONS

const ModUnicodeString&
Database::
getLogFilePath() const
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!m_pLogFilePath) {
		m_pLogFilePath = new Os::Path(getPath(Path::Category::LogicalLog));
	}
	return *m_pLogFilePath;
}

//	FUNCTION public
//	Schema::Database::getLogFile --
//		データベース用の論理ログファイルの論理ログファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		bool				mounted
//			true
//				マウント済の論理ログファイル記述子を得る
//			false または指定されないとき
//				データベースがマウントされているかに
//				あわせた論理ログファイル記述子を得る
//
//	RETURN
//		得られたオート論理ログファイル記述子
//
//	EXCEPTIONS

Trans::Log::AutoFile
Database::
getLogFile(bool mounted) const
{
	if (getScope() == Scope::Meta)
		return Manager::SystemTable::getLogFile();

	// IDが正しく振られている必要がある
	; _SYDNEY_ASSERT(getID() != ID::Invalid);

	Trans::Log::File::StorageStrategy storageStrategy;
	storageStrategy._mounted = (mounted || !isUnmounted());
	storageStrategy._readOnly = isReadOnly();
	storageStrategy._path = getLogFilePath();
	storageStrategy._truncatable = true;
	storageStrategy._category = Trans::Log::File::Category::Database;
	storageStrategy._recoveryFull = isRecoveryFull();

	return Trans::Log::File::attach(
		storageStrategy, Lock::LogicalLogName(getID()), getName());
}

#ifdef OBSOLETE // Redoの仕様変更によりパス指定で論理ログファイル記述子を得ることはなくなった
//	FUNCTION private
//	Schema::Database::getLogFile --
//		データベース用の論理ログファイルの論理ログファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			論理ログファイルの絶対パス名
//
//	RETURN
//		得られたオート論理ログファイル記述子
//
//	EXCEPTIONS

Trans::Log::AutoFile
Database::
getLogFile(const Os::Path& path) const
{
	if (getScope() == Scope::Meta)
		return Manager::SystemTable::getLogFile();

	Trans::Log::File::StorageStrategy storageStrategy;
	storageStrategy._mounted = !isUnmounted();
	storageStrategy._readOnly = isReadOnly();
	storageStrategy._path = path;
	storageStrategy._truncatable = true;
	storageStrategy._category = Trans::Log::File::Category::Database;
	storageStrategy._recoveryFull = isRecoveryFull();

	return Trans::Log::File::attach(
		storageStrategy, Lock::LogicalLogName(getID()), getName());
}
#endif

//	FUNCTION public
//	Schema::Database::reserve -- データベースの使用を開始する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			使用を開始するセッションのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
reserve(SessionID iSessionID_)
{
	// スナップショットにセッションを登録する
	ObjectSnapshot::reserveDatabase(iSessionID_);
}

//	FUNCTION public
//	Schema::Database::release -- データベースの使用を終了する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			使用を終了したセッションのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
release(SessionID iSessionID_)
{
	// 対応するスナップショットを破棄する
	Manager::ObjectSnapshot::erase(iSessionID_);

	// 一時表の登録を抹消する
	Database::dropTemporary(iSessionID_);

	// スナップショットからセッションの登録を消す
	ObjectSnapshot::eraseReservation(iSessionID_);
}

#ifdef OBSOLETE // データベースオブジェクトを外部がキャッシュすることはない
//	FUNCTION public
//	Schema::Database::isValid -- 陳腐化していないか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			陳腐化していないかを調べるスキーマオブジェクトID
//		Schema::Object::Timestamp iTimestamp_
//			正しいスキーマオブジェクトの値とこの値が異なっていたら
//			陳腐化していると判断する
//		Trans::Transaction& cTrans_
//			陳腐化したかを調べるトランザクション記述子
//
//	RETURN
//		true
//			自分自身の表すスキーマオブジェクトは最新のものである
//		false
//			陳腐化している
//
//	EXCEPTIONS
//		なし

// static
bool
Database::
isValid(ID::Value iID_, Timestamp iTimestamp_, Trans::Transaction& cTrans_)
{
	Database* pDatabase = get(iID_, cTrans_);

	return (pDatabase && pDatabase->getTimestamp() == iTimestamp_);
}
#endif

//	FUNCTION public
//	Schema::Database::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			永続化するオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Database::
doBeforePersist(const Pointer& pDatabase_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::Changed:
	case Status::DeletedInRecovery:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
	{
		// 何もしない
		break;
	}
	case Status::Deleted:
	{
		// データベースを構成するファイルとディレクトリーを削除する
		// ★注意★
		// エラーが起きてもログの内容から再実行できるように
		// ファイルやディレクトリーを実際に「消す」操作は
		// システム表から消す操作を永続化する前に行う

		if ( !pDatabase_->isUnmounted() )
		{
			// UNMOUNT 処理ではないのでファイルを削除する
			pDatabase_->destroy(cTrans_);

		}

		break;
	}
	default:
		break;
	}
}

//	FUNCTION public
//	Schema::Database::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			永続化したオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Database::
doAfterPersist(const Pointer& pDatabase_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_.get());

	ID::Value dbID = pDatabase_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// システムへ生成されるデータベースを表すクラスを追加する

		Manager::ObjectSnapshot::get(cTrans_)->addDatabase(pDatabase_, cTrans_);

		// 利用可能に設定する
		pDatabase_->setAvailability(true);
		break;
	}
	case Status::Changed:
	case Status::CreateCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除なら登録の抹消も行う

		// 状態を「実際に削除された」にする

		pDatabase_->setStatus(Schema::Object::Status::ReallyDeleted);

		// オブジェクトが保持する下位オブジェクトを抹消する
		pDatabase_->reset();

		// データベースに属するシステム表の永続化情報を保持する構造を破棄する
		// ★注意★
		// 破棄するのはこのデータベースに属するシステム表のものなので
		// Database->getIDを使う、dbIDはメタデータベースのIDなので使用しない

		SystemTable::eraseStatus(pDatabase_->getID());

		// データベースが利用不可だったかもしれないので、利用可にしておく
		//
		//【注意】	利用可になっても結局どこからももう参照できない

		pDatabase_->setAvailability(true);

		// データベースの使用を終了する
		pDatabase_->close(true /* volatile */);

		// すべてのスナップショットから登録を抹消する
		Manager::ObjectSnapshot::eraseDatabase(pDatabase_->getID(), cTrans_.getSessionID());

		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Category::Database);
}

//	FUNCTION public
//	Schema::Database::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::DatabasePointer& pDatabase_
//			読み込んだオブジェクト
//		Schema::ObjectSnapshot& cSnapshot_
//			データベースが属するスナップショット
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Database::
doAfterLoad(const Pointer& pDatabase_, ObjectSnapshot& cSnapshot_, Trans::Transaction& cTrans_)
{
	// リカバリー中のUNDO処理において最終的なパスが登録されていたら
	// パス指定を置き換える
	pDatabase_->checkUndo(pDatabase_->getID());

	// システムへ読み出したデータベースを表すクラスを追加する
	// また、マネージャーにこのデータベースを表すクラスを
	// スキーマオブジェクトとして管理させる

	(void) cSnapshot_.addDatabase(pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::Database::reset --
//		データベースが保持する下位オブジェクトを抹消する
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
Database::
reset()
{
	if (_tables)
		resetTable(*this);
	if (_areas)
		resetArea();
	if (_cascades)
		resetCascade();
	if (_partitions)
		resetPartition();
	if (_functions)
		resetFunction();
	if (_privileges)
		resetPrivilege();
}

//	FUNCTION public
//		Schema::Database::getPath --
//			ファイルの格納場所を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ModUnicodeString>& vecPath_
//			パスを格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
getPath(ModVector<ModUnicodeString>& vecPath_) const
{
	; _SYDNEY_ASSERT(vecPath_.isEmpty());
	if (m_pPath) vecPath_ = *m_pPath;
}

//	FUNCTION public
//		Schema::Database::getPath --
//			ファイルの格納場所を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database::Path::Category::Value eCategory_
//			パスの種別
//
//	RETURN
//		ファイルを格納するパス名
//
//	EXCEPTIONS

Os::Path
Database::
getPath(Path::Category::Value eCategory_) const
{
	return getPath(eCategory_, *m_pPath, getName());
}

//	FUNCTION public
//		Schema::Database::getPath --
//			ファイルの格納場所を得る
//
//	NOTES
//		Databaseオブジェクトがない状態でパス指定からパスを得るために使用する
//
//	ARGUMENTS
//		Schema::Database::Path::Category::Value eCategory_
//			パスの種別
//		const ModVector<ModUnicodeString>& vecPathList_
//			パス指定配列
//		const Schema::Object::Name& cName_
//
//	RETURN
//		ファイルを格納するパス名
//
//	EXCEPTIONS

// static
Os::Path
Database::
getPath(Path::Category::Value eCategory_,
		const ModVector<ModUnicodeString>& vecPathList_,
		const Object::Name& cName_)
{
	return _Path::_getPath(eCategory_, _Path::_getPathDefinition(eCategory_, &vecPathList_), cName_);
}

//	FUNCTION public
//	Schema::Database::getDefaultPath --
//		データベースのデフォルトの格納場所の絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database::Path::Category::Value	category
//			絶対パス名を得るデータベースのデフォルトの格納場所の種類
//		Object::Name&		name
//			デフォルトの格納場所の絶対パス名を得るデータベースの名前
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

// static
Os::Path
Database::
getDefaultPath(Path::Category::Value category, const Object::Name& name)
{
	Os::Path	parent;

	switch (category) {
	case Path::Category::Data:
	case Path::Category::LogicalLog:
		parent = Manager::Configuration::getDefaultAreaPath();	break;
	case Path::Category::System:
		parent = Manager::Configuration::getSystemAreaPath();	break;
	default:
		; _SYDNEY_ASSERT(false);
	}

	return parent.addPart(name);
}

//	FUNCTION public
//		Schema::Database::setPath --
//			ファイルを格納するパス名を設定する
//
//	NOTES
//		ファイルを格納するパス名を設定する
//
//	ARGUMENTS
//		const Statement::DatabaseDefinition& stmt
//			Database 定義用 Statement オブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
setPath(const Statement::DatabasePathElementList& cPathList_)
{
	int iCntMax = cPathList_.getCount();
	for ( int iCnt = 0; iCnt < iCntMax; iCnt++ )
	{
		Statement::DatabasePathElement* pPath
			= cPathList_.getPathElementAt(iCnt);
		; _SYDNEY_ASSERT(pPath);

		Statement::Literal* pLiteral = pPath->getPathName();
		; _SYDNEY_ASSERT(pLiteral);

		Common::Data::Pointer pValue = pLiteral->createData();
		Common::StringData* pString
				= _SYDNEY_DYNAMIC_CAST(Common::StringData*, pValue.get());
		; _SYDNEY_ASSERT(pString);

		//パス名を反映
		Path::Category::Value eCategory = Path::Category::ValueNum;
		switch ( pPath->getPathType() )
		{
		case Statement::DatabasePathElement::Database:
		{
			eCategory = Path::Category::Data;
			break;
		}
		case Statement::DatabasePathElement::LogicalLog:
		{
			eCategory = Path::Category::LogicalLog;
			break;
		}
		case Statement::DatabasePathElement::System:
		{
			eCategory = Path::Category::System;
			break;
		}
		default:
			; _SYDNEY_ASSERT(false);
		}
		setPath(eCategory, pString->getValue());
	}
}

//	FUNCTION public
//		Schema::Database::setPath --
//			ファイルを格納するパス名を設定する
//
//	NOTES
//		ファイルを格納するパス名を設定する
//
//	ARGUMENTS
//		Path::Category::Value eCategory_
//			パスの種別
//
//		const ModUnicodeString& cPath_
//			設定パス名
//
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setPath(Path::Category::Value eCategory_, const ModUnicodeString& cPath_)
{
	// 最初の実装では長さ0のときは何もしないようになっていたが、
	// 長さ0の文字列が与えられたときは無指定(=0)を入れるようにする

	ModSize nPos = static_cast<ModSize>(eCategory_);

	if ( m_pPath == 0 )
	{
		resetPath();
		m_pPath->assign(nPos + 1, ModUnicodeString());

	} else {
		if (m_pPath->getSize() <= nPos) {
			m_pPath->reserve(nPos + 1);
			for (ModSize i = m_pPath->getSize(); i <= nPos; ++i) {
				m_pPath->pushBack(ModUnicodeString());
			}
		}
	}

	(*m_pPath)[nPos] = cPath_;

	if (eCategory_ == Path::Category::LogicalLog) {
		// Logのパスを移動したらデフォルトのパス名をクリアしておく
		clearLogFilePath();
	}

	if (eCategory_ == Path::Category::Data) {
		// Dataのパスを移動したらデフォルトのエリアパス名をクリアしておく
		clearDataPath();
	}
}

//	FUNCTION public
//		Schema::Database::setPath --
//			ファイルを格納するパス名を設定する
//
//	NOTES
//		ファイルを格納するパス名を設定する
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>& cPathList_
//			パス名配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setPath(const ModVector<ModUnicodeString>& cPathList_)
{
	if (ModSize n = cPathList_.getSize()) {

		if (n > Path::Category::ValueNum) {
			// 例外送出
			SydInfoMessage
				<< "SetPath failed. Illegal path array."
				<< ModEndl;
			_SYDNEY_THROW1(Exception::InvalidPath, getName());
		}

		// 初期化
		resetPath();
		*m_pPath = cPathList_;

	} else {
		clearPath();
	}

	// ログファイルとデータ用のパス名キャッシュをクリアする
	clearLogFilePath();
	clearDataPath();
}

//	FUNCTION public
//		Schema::Database::resetPath --
//			ファイルを格納するパス名配列を抹消する
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
Database::
resetPath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 初期化
	if ( !m_pPath ) {
		m_pPath = new ModVector<ModUnicodeString>;
	} else {
		m_pPath->clear();
	}
}

//	FUNCTION public
//		Schema::Database::clearPath --
//			ファイルを格納するパス名配列を抹消し、
//			管理用のベクターを破棄する
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
Database::
clearPath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if ( m_pPath ) {
		resetPath();
		delete m_pPath, m_pPath = 0;
	}
}

//	FUNCTION public
//		Schema::Database::clearLogFilePath --
//			ログファイルを格納するパスのキャッシュをクリアする
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
Database::
clearLogFilePath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	delete m_pLogFilePath, m_pLogFilePath = 0;
}

//	FUNCTION public
//		Schema::Database::clearDataPath --
//			データを格納するパスのキャッシュをクリアする
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
Database::
clearDataPath()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	delete m_pDataPath, m_pDataPath = 0;
}

//	FUNCTION public
//		Schema::Database::getSequence --
//			データベースに属するオブジェクトの新しいIDを得るための
//			シーケンスファイルを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		シーケンスファイル
//
//	EXCEPTIONS

Sequence&
Database::
getSequence()
{
	if (!_sequence) {
		Os::Path path = getPath(Path::Category::System);

		// ファイル名を追加する

		path.addPart(PathParts::Sequence::ObjectID);

		// 生成したパス名を使って、タプル ID を生成するための
		// シーケンスを表すクラスを生成する

		_sequence = new Sequence(path, getID(), Sequence::Unsigned::MaxValue,
								 !isUnmounted(), getScope(), isReadOnly());
		; _SYDNEY_ASSERT(_sequence);
	}

	return *_sequence;
}

//	FUNCTION public
//		Schema::Database::clearSequence --
//			データベースに属するオブジェクトの新しいIDを得るための
//			シーケンスファイルを表すオブジェクトを破棄する
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
Database::
clearSequence()
{
	delete _sequence, _sequence = 0;
}

//	FUNCTION public
//		Schema::Database::getDataPath --
//			エリア無指定時にデータ格納に使用するパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		パス名
//
//	EXCEPTIONS

const ModUnicodeString&
Database::
getDataPath()
{
	if (!m_pDataPath) {
		m_pDataPath = new Os::Path(getPath(Path::Category::Data));
	}
	return *m_pDataPath;
}

//	FUNCTION public
//		Schema::Database::getDataPath --
//			エリア無指定時にデータ格納に使用するパス名を得る
//
//	NOTES
//		UNDO時にデータベースのオブジェクトがない状態でパス名を得るのに使用する
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>& cPathList_
//			パス指定
//		const Schema::Object::Name& cName_
//			データベース名
//
//	RETURN
//		パス名
//
//	EXCEPTIONS

// static
Os::Path
Database::
getDataPath(const ModVector<ModUnicodeString>& cPathList_,
			const Name& cName_)
{
	return getPath(Path::Category::Data, cPathList_, cName_);
}

//	FUNCTION public
//		Schema::Database::checkRelatedPath --
//			パスがデータベース内で使用されていないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Os::Path& cPath_
//			調べる対象のパス名
//		const Schema::Area* pOmitArea_ = 0
//			0以外のとき同じエリアは調査の対象から除く
//
//	RETURN
//		true .. パス名と一致するか親子関係にあるパスがすでに使用されている
//		false.. パス名と一致するか親子関係にあるパスは使用されていない
//
//	EXCEPTIONS

bool
Database::
checkRelatedPath(Trans::Transaction& cTrans_,
				 const Os::Path& cPath_,
				 const Area* pOmitArea_ /* = 0 */) const
{
	// すべてのカテゴリーについて親子関係を調べる
	for (int iCategory = static_cast<int>(Database::Path::Category::Data);
		 iCategory < Database::Path::Category::ValueNum; ++iCategory) {
		Database::Path::Category::Value eCategory = static_cast<Database::Path::Category::Value>(iCategory);

		if (_Path::_isRelated(cPath_, getPath(eCategory)))
			// 親子関係がある
			return true;
	}

	// エリアについて調べる
	if (pOmitArea_) {
		if (const_cast<Database*>(this)->loadArea(cTrans_)
			.find(BoolFunction1<Area, const Os::Path&>(_Path::_checkAreaPath, cPath_),
				  BoolFunction1<Area, ID::Value>(AreaMap::omitByID, pOmitArea_->getID()))
			!= 0)
			// 親子関係のあるエリアが見つかった
			return true;
	} else {
		BoolFunction0<Area>::Func func = _Bool::_False<Area>;
		if (const_cast<Database*>(this)->loadArea(cTrans_).find(
				BoolFunction1<Area, const Os::Path&>(
					_Path::_checkAreaPath, cPath_),
					BoolFunction0<Area>(func)) != 0)
			// 親子関係のあるエリアが見つかった
			return true;
	}
	return false;
}

///////////////////////
// Areaに関する処理 //
///////////////////////

//	FUNCTION public
//	Schema::Database::loadArea --
//		データベースに存在するすべてのエリアを表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		データベースに存在するエリアをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const AreaMap&
Database::
loadArea(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_areas) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_areas) {

			// 「エリア」表のうち、このデータベースに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが Mount 中でなく、一時データベースかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetArea();
			else {
				SystemTable::Area(*this).load(cTrans_, bRecovery_);
				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュをクリアする
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_areas);
		}
	}
	return *_areas;
}

//	FUNCTION public
//	Schema::Database::getArea --
//		データベースに存在するすべてのエリアを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		データベースに存在するエリアを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Area*>
Database::
getArea(Trans::Transaction& cTrans_) const
{
	BoolFunction0<Area>::Func func = _Bool::_Deleted;
	return const_cast<Database*>(this)->loadArea(cTrans_).getView(
		getRWLock(), BoolFunction0<Area>(func));
}

//	FUNCTION public
//	Schema::Database::getArea --
//		データベースに存在するエリアのうち、
//		あるスキーマオブジェクト ID のエリアを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	areaID
//			エリアのスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたエリアを格納する領域の先頭アドレス
//		0
//			データベースには指定されたスキーマオブジェクト ID のエリアは存在しない
//
//	EXCEPTIONS

Area*
Database::
getArea(ID::Value areaID, Trans::Transaction& cTrans_) const
{
	if (areaID == ID::Invalid)
		return 0;

	const AreaMap& cMap = const_cast<Database*>(this)->loadArea(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(areaID).get();
}

//	FUNCTION public
//	Schema::Database::getArea --
//		データベースに存在するエリアのうち、ある名前のエリアを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	areaName
//			エリアの名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値return
//			得られたエリアを格納する領域の先頭アドレス
//		0
//			データベースには指定された名前のエリアは存在しない
//
//	EXCEPTIONS

Area*
Database::
getArea(const Name& areaName, Trans::Transaction& cTrans_) const
{
	const AreaMap& cMap = const_cast<Database*>(this)->loadArea(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Area, const Name&>(_Bool::_findByName, areaName));
}

//	FUNCTION public
//	Schema::Database::addArea --
//		データベースを表すクラスのエリアとして、指定されたエリアを表すクラスを追加する
//
//	NOTES
//		「エリア」表は更新されない
//
//	ARGUMENTS
//		const Schema::AreaPointer&		area
//			追加するエリアを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加したエリアを表すクラス
//
//	EXCEPTIONS

Area&
Database::
addArea(const AreaPointer& area, Trans::Transaction& cTrans_)
{
	// 「エリア」表のうち、このデータベースに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたエリアを追加する
	(void) loadArea(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_areas->insert(area);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

	return *area;
}

//	FUNCTION public
//	Schema::Database::eraseArea --
//		データベースを表すクラスからあるエリアを表すクラスの登録を抹消する
//
//	NOTES
//		「エリア」表は更新されない
//		登録を抹消したエリアクラスはdeleteされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	areaID
//			登録を抹消するエリアのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
eraseArea(ID::Value areaID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_areas) {
		(void) _areas->erase(areaID);
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize();
	}

	// エリアはキャッシュに入れていないので
	// 他のもののようにキャッシュから削除する必要はない
}

//	FUNCTION public
//	Schema::Database::resetArea --
//		データベースにはエリアを表すクラスが登録されていないことにする
//
//	NOTES
//		「エリア」表は更新されない
//		エリア以下のオブジェクトをキャッシュから除くことはしない
//		キャッシュのクリアが続くことが分かっているとき専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetArea()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_areas) {

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_areas->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_areas->reset();

	} else {
		// エリアを表すクラスを登録するハッシュマップを生成する

		_areas = new AreaMap;
		; _SYDNEY_ASSERT(_areas);
	}
}

//	FUNCTION public
//	Schema::Database::clearArea --
//		データベースを表すクラスに登録されているエリアを表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「エリア」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clearArea()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_areas) {
		// ベクターに登録されているエリアを表すクラスがあれば、
		// すべて破棄し、ベクターも破棄する
		resetArea();
		delete _areas, _areas = 0;
	}
}

///////////////////////
// Tableに関する処理 //
///////////////////////

//	FUNCTION public
//	Schema::Database::loadTable --
//		データベースに存在するすべての表を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		データベースに存在する表をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const TableMap&
Database::
loadTable(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_tables) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_tables) {
			// 「表」表のうち、このデータベースに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが mount 中でなく、一時データベースかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetTable(*this);
			else {
				SystemTable::Table(*this).load(cTrans_, bRecovery_);
				// Databaseで以下の操作を行うとデッドロックになる可能性があるのでやめる
//				// loadしたらキャッシュサイズを調べる
//				if (Manager::ObjectTree::Database::checkCacheSize()) {
//					// 超えていたらキャッシュをクリアする
//					Manager::ObjectTree::Database::clearCache();
//				}
			}
			; _SYDNEY_ASSERT(_tables);
		}
	}
	return *_tables;
}

//	FUNCTION public
//	Schema::Database::getTable --
//		データベースに存在するすべての表を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bInternal_ = false
//			trueのときUndo情報でDropが記録されていても取得する
//
//	RETURN
//		データベースに存在する表を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Table*>
Database::
getTable(Trans::Transaction& cTrans_, bool bInternal_ /* = false */) const
{
	ModVector<Table*>	v;

	const TableMap& cMap = const_cast<Database*>(this)->loadTable(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction2<Table, const Name&, bool>(TableMap::findValid, getName(), bInternal_));
	return v;
}

//	FUNCTION public
//	Schema::Database::getTable --
//		データベースに存在する表のうち、
//		あるスキーマオブジェクト ID の表を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	tableID
//			表のスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bInternal_ = false
//			trueのときUndo情報でDropが記録されていても取得する
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			データベースには指定されたスキーマオブジェクト ID の表は存在しない
//
//	EXCEPTIONS

Table*
Database::
getTable(ID::Value tableID, Trans::Transaction& cTrans_, bool bInternal_ /* = false */) const
{
	if (tableID == ID::Invalid)
		return 0;

	// IDがシステム表のものと一致するならシステム表を表すオブジェクトを返す
	Object::Category::Value eCategory = Table::getSystemTableCategory(tableID);
	if (eCategory != Object::Category::Unknown
		&& eCategory != Object::Category::Database) {
		return getSystemTable(eCategory, cTrans_);
	}

	const TableMap& cMap = const_cast<Database*>(this)->loadTable(cTrans_);

	AutoRWLock l(getRWLock());
	Table* pTable = cMap.get(tableID).get();

	if (pTable && !TableMap::findValid(pTable, getName(), bInternal_))
		return 0;

	if (pTable && Manager::Configuration::isSchemaPreloaded())
		pTable->doPreLoad(cTrans_, const_cast<Database&>(*this));

	return pTable;
}

//	FUNCTION public
//	Schema::Database::getTable --
//		データベースに存在する表のうち、ある名前の表を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	tableName
//			表の名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bInternal_ = false
//			trueのときUndo情報でDropが記録されていても取得する
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			データベースには指定された名前の表は存在しない
//
//	EXCEPTIONS

Table*
Database::
getTable(const Name& tableName, Trans::Transaction& cTrans_, bool bInternal_ /* = false */) const
{
	// 表名の先頭が'#'なら指定されたデータベースの代わりに
	// 一時データベースを使う

	if (Table::isToBeTemporary(tableName)
		&& (getScope() == Scope::Permanent || getScope() == Scope::Meta))
		return createTemporary(cTrans_)->getTable(tableName, cTrans_);

	// 表名がシステム表と一致するならシステム表を表すオブジェクトを返す
	Object::Category::Value eCategory = Table::getSystemTableCategory(tableName);
	if (eCategory != Object::Category::Unknown
		&& eCategory != Object::Category::Database) {
		return getSystemTable(eCategory, cTrans_);
	}
	// If table name is used in a virtual table, return virtual table object
	VirtualTable::Category::Value eVirtual = VirtualTable::getCategory(tableName);
	if (eVirtual != VirtualTable::Category::Unknown) {
		return getVirtualTable(eVirtual, cTrans_);
	}

	const TableMap& cMap = const_cast<Database*>(this)->loadTable(cTrans_);

	AutoRWLock l(getRWLock());
	Table* pTable = cMap.find(BoolFunction1<Table, const Name&>(_Bool::_findByName, tableName));

	if (pTable && !TableMap::findValid(pTable, getName(), bInternal_))
		return 0;

	if (pTable && Manager::Configuration::isSchemaPreloaded())
		pTable->doPreLoad(cTrans_, const_cast<Database&>(*this));

	return pTable;
}

//	FUNCTION public
//	Schema::Database::getSystemTable --
//		システム表を表す表オブジェクトを得る
//
//	NOTES
//		Fileからも使うのでpublicに昇格
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクト種
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		システム表を表すオブジェクト
//
//	EXCEPTIONS

Table*
Database::
getSystemTable(Object::Category::Value eCategory_,
			   Trans::Transaction& cTrans_) const
{
	initializeSystemTables();

	AutoRWLock l(getRWLock());

	if (!(*m_pSystemTables)[eCategory_]) {
		l.convert(Os::RWLock::Mode::Write);
		if (!(*m_pSystemTables)[eCategory_]) {
			// システム表を表すオブジェクトを作成する
			(*m_pSystemTables)[eCategory_] =
				SystemDatabase::createSystemTable(cTrans_, const_cast<Database&>(*this),
												  eCategory_, m_iSystemTableObjectID);
		}
	}
	return (*m_pSystemTables)[eCategory_];
}

//	FUNCTION public
//	Schema::Database::addTable --
//		データベースを表すクラスの表として、指定された表を表すクラスを追加する
//
//	NOTES
//		「表」表は更新されない
//
//	ARGUMENTS
//		const Schema::Table::Pointer&		table
//			追加する表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加した表を表すクラス
//
//	EXCEPTIONS

Table&
Database::
addTable(const TablePointer& table, Trans::Transaction& cTrans_)
{
	// 「表」表のうち、このデータベースに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた表を追加する

	(void) loadTable(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_tables->insert(table);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

/********* すべての表を一気にPreloadするならここを生かす
	if (Manager::Configuration::isSchemaPreloaded())
		// 表以下のスキーマ情報を読み込む
		table.doPreLoad(cTrans_, *this);
*/
	return *table;
}

//	FUNCTION public
//	Schema::Database::eraseTable --
//		データベースを表すクラスからある表を表すクラスの登録を抹消する
//
//	NOTES
//		「表」表は更新されない
//		登録を抹消した表クラスはdeleteされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iTableID_
//			登録を抹消する表のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
eraseTable(ID::Value iTableID_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_tables) {
		(void) _tables->erase(*this, iTableID_);
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize();
	}

	// 表はキャッシュに入れていないので
	// 他のもののようにキャッシュから削除する必要はない
}

// FUNCTION public
//	Schema::Database::eraseIndex -- 索引の抹消
//
// NOTES
//
// ARGUMENTS
//	ID::Value iTableID_
//	ID::Value iIndexID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
eraseIndex(ID::Value iTableID_, ID::Value iIndexID_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_tables) {
		Table* pTable = _tables->get(iTableID_).get();
		if (pTable) {
			pTable->eraseIndex(*this, iIndexID_);
		}
	}
}

///////////////////////
// Cascadeに関する処理 //
///////////////////////

//	FUNCTION public
//	Schema::Database::loadCascade --
//		データベースに存在するすべての子サーバーを表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		データベースに存在する子サーバーをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const CascadeMap&
Database::
loadCascade(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_cascades) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_cascades) {

			// 「子サーバー」表のうち、このデータベースに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが Mount 中でなく、一時データベースかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetCascade();
			else {
				SystemTable::Cascade(*this).load(cTrans_, bRecovery_);
				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュをクリアする
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_cascades);
		}
	}
	return *_cascades;
}

// FUNCTION public
//	Schema::Database::hasCascade -- 子サーバーの登録があるか
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Database::
hasCascade(Trans::Transaction& cTrans_) const
{
	return !(const_cast<Database*>(this)->loadCascade(cTrans_).isEmpty());
}

//	FUNCTION public
//	Schema::Database::getCascade --
//		データベースに存在するすべての子サーバーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		データベースに存在する子サーバーを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Cascade*>
Database::
getCascade(Trans::Transaction& cTrans_) const
{
	BoolFunction0<Cascade>::Func func = _Bool::_Deleted;
	return const_cast<Database*>(this)->loadCascade(cTrans_).getView(
		getRWLock(), BoolFunction0<Cascade>(func));
}

//	FUNCTION public
//	Schema::Database::getCascade --
//		データベースに存在する子サーバーのうち、
//		あるスキーマオブジェクト ID の子サーバーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	cascadeID
//			子サーバーのスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた子サーバーを格納する領域の先頭アドレス
//		0
//			データベースには指定されたスキーマオブジェクト ID の子サーバーは存在しない
//
//	EXCEPTIONS

Cascade*
Database::
getCascade(ID::Value cascadeID, Trans::Transaction& cTrans_) const
{
	if (cascadeID == ID::Invalid)
		return 0;

	const CascadeMap& cMap = const_cast<Database*>(this)->loadCascade(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(cascadeID).get();
}

//	FUNCTION public
//	Schema::Database::getCascade --
//		データベースに存在する子サーバーのうち、ある名前の子サーバーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	cascadeName
//			子サーバーの名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値return
//			得られた子サーバーを格納する領域の先頭アドレス
//		0
//			データベースには指定された名前の子サーバーは存在しない
//
//	EXCEPTIONS

Cascade*
Database::
getCascade(const Name& cascadeName, Trans::Transaction& cTrans_) const
{
	const CascadeMap& cMap = const_cast<Database*>(this)->loadCascade(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Cascade, const Name&>(_Bool::_findByName, cascadeName));
}

//	FUNCTION public
//	Schema::Database::addCascade --
//		データベースを表すクラスの子サーバーとして、指定された子サーバーを表すクラスを追加する
//
//	NOTES
//		「子サーバー」表は更新されない
//
//	ARGUMENTS
//		const Schema::CascadePointer&		cascade
//			追加する子サーバーを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加した子サーバーを表すクラス
//
//	EXCEPTIONS

Cascade&
Database::
addCascade(const CascadePointer& cascade, Trans::Transaction& cTrans_)
{
	// 「子サーバー」表のうち、このデータベースに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた子サーバーを追加する
	(void) loadCascade(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_cascades->insert(cascade);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

	return *cascade;
}

//	FUNCTION public
//	Schema::Database::eraseCascade --
//		データベースを表すクラスからある子サーバーを表すクラスの登録を抹消する
//
//	NOTES
//		「子サーバー」表は更新されない
//		登録を抹消した子サーバークラスはdeleteされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	cascadeID
//			登録を抹消する子サーバーのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
eraseCascade(ID::Value cascadeID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cascades) {
		(void) _cascades->erase(cascadeID);
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize();
	}

	// 子サーバーはキャッシュに入れていないので
	// 他のもののようにキャッシュから削除する必要はない
}

//	FUNCTION public
//	Schema::Database::resetCascade --
//		データベースには子サーバーを表すクラスが登録されていないことにする
//
//	NOTES
//		「子サーバー」表は更新されない
//		子サーバー以下のオブジェクトをキャッシュから除くことはしない
//		キャッシュのクリアが続くことが分かっているとき専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetCascade()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cascades) {

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_cascades->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_cascades->reset();

	} else {
		// 子サーバーを表すクラスを登録するハッシュマップを生成する

		_cascades = new CascadeMap;
		; _SYDNEY_ASSERT(_cascades);
	}
}

//	FUNCTION public
//	Schema::Database::clearCascade --
//		データベースを表すクラスに登録されている子サーバーを表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「子サーバー」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clearCascade()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cascades) {
		// ベクターに登録されている子サーバーを表すクラスがあれば、
		// すべて破棄し、ベクターも破棄する
		resetCascade();
		delete _cascades, _cascades = 0;
	}
}

///////////////////////
// Partitionに関する処理 //
///////////////////////

//	FUNCTION public
//	Schema::Database::loadPartition --
//		データベースに存在するすべてのルールを表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		データベースに存在するルールをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const PartitionMap&
Database::
loadPartition(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_partitions) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_partitions) {

			// 「ルール」表のうち、このデータベースに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが Mount 中でなく、一時データベースかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetPartition();
			else {
				SystemTable::Partition(*this).load(cTrans_, bRecovery_);
				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュをクリアする
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_partitions);
		}
	}
	return *_partitions;
}

//	FUNCTION public
//	Schema::Database::getPartition --
//		データベースに存在するすべてのルールを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		データベースに存在するルールを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Partition*>
Database::
getPartition(Trans::Transaction& cTrans_) const
{
	BoolFunction0<Partition>::Func func = _Bool::_Deleted;
	return const_cast<Database*>(this)->loadPartition(cTrans_).getView(
		getRWLock(), BoolFunction0<Partition>(func));
}

//	FUNCTION public
//	Schema::Database::getPartition --
//		データベースに存在するルールのうち、
//		あるスキーマオブジェクト ID のルールを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	partitionID
//			ルールのスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたルールを格納する領域の先頭アドレス
//		0
//			データベースには指定されたスキーマオブジェクト ID のルールは存在しない
//
//	EXCEPTIONS

Partition*
Database::
getPartition(ID::Value partitionID, Trans::Transaction& cTrans_) const
{
	if (partitionID == ID::Invalid)
		return 0;

	const PartitionMap& cMap = const_cast<Database*>(this)->loadPartition(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(partitionID).get();
}

//	FUNCTION public
//	Schema::Database::getPartition --
//		データベースに存在するルールのうち、ある名前のルールを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	partitionName
//			ルールの名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値return
//			得られたルールを格納する領域の先頭アドレス
//		0
//			データベースには指定された名前のルールは存在しない
//
//	EXCEPTIONS

Partition*
Database::
getPartition(const Name& partitionName, Trans::Transaction& cTrans_) const
{
	const PartitionMap& cMap = const_cast<Database*>(this)->loadPartition(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Partition, const Name&>(_Bool::_findByName, partitionName));
}

// FUNCTION public
//	Schema::Database::getTablePartition -- 
//
// NOTES
//
// ARGUMENTS
//	ID::Value tableID
//	Trans::Transaction& cTrans_
//	
// RETURN
//	PartitionPointer
//
// EXCEPTIONS

PartitionPointer
Database::
getTablePartition(ID::Value tableID, Trans::Transaction& cTrans_) const
{
	const PartitionMap& cMap = const_cast<Database*>(this)->loadPartition(cTrans_);

	AutoRWLock l(getRWLock());

	PartitionMap::ConstIterator iterator = cMap.begin();
	const PartitionMap::ConstIterator& end = cMap.end();
	for (; iterator != end; ++iterator) {
		const Partition::Pointer& pPartition = PartitionMap::getValue(iterator);
		if (pPartition.get() && pPartition->getTableID() == tableID)
			return pPartition;
	}
	return Partition::Pointer();
}

//	FUNCTION public
//	Schema::Database::addPartition --
//		データベースを表すクラスのルールとして、指定されたルールを表すクラスを追加する
//
//	NOTES
//		「ルール」表は更新されない
//
//	ARGUMENTS
//		const Schema::PartitionPointer&		partition
//			追加するルールを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加したルールを表すクラス
//
//	EXCEPTIONS

Partition&
Database::
addPartition(const PartitionPointer& partition, Trans::Transaction& cTrans_)
{
	// 「ルール」表のうち、このデータベースに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたルールを追加する
	(void) loadPartition(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_partitions->insert(partition);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

	return *partition;
}

//	FUNCTION public
//	Schema::Database::erasePartition --
//		データベースを表すクラスからあるルールを表すクラスの登録を抹消する
//
//	NOTES
//		「ルール」表は更新されない
//		登録を抹消したルールクラスはdeleteされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	partitionID
//			登録を抹消するルールのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
erasePartition(ID::Value partitionID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_partitions) {
		(void) _partitions->erase(partitionID);
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize();
	}

	// ルールはキャッシュに入れていないので
	// 他のもののようにキャッシュから削除する必要はない
}

//	FUNCTION public
//	Schema::Database::resetPartition --
//		データベースにはルールを表すクラスが登録されていないことにする
//
//	NOTES
//		「ルール」表は更新されない
//		ルール以下のオブジェクトをキャッシュから除くことはしない
//		キャッシュのクリアが続くことが分かっているとき専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetPartition()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_partitions) {

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_partitions->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_partitions->reset();

	} else {
		// ルールを表すクラスを登録するハッシュマップを生成する

		_partitions = new PartitionMap;
		; _SYDNEY_ASSERT(_partitions);
	}
}

//	FUNCTION public
//	Schema::Database::clearPartition --
//		データベースを表すクラスに登録されているルールを表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「ルール」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clearPartition()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_partitions) {
		// ベクターに登録されているルールを表すクラスがあれば、
		// すべて破棄し、ベクターも破棄する
		resetPartition();
		delete _partitions, _partitions = 0;
	}
}

///////////////////////
// Functionに関する処理 //
///////////////////////

//	FUNCTION public
//	Schema::Database::loadFunction --
//		データベースに存在するすべての関数を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		データベースに存在する関数をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const FunctionMap&
Database::
loadFunction(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_functions) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_functions) {

			// 「関数」表のうち、このデータベースに関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが Mount 中でなく、一時データベースかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetFunction();
			else {
				SystemTable::Function(*this).load(cTrans_, bRecovery_);
				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュをクリアする
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_functions);
		}
	}
	return *_functions;
}

//	FUNCTION public
//	Schema::Database::getFunction --
//		データベースに存在するすべての関数を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		データベースに存在する関数を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Function*>
Database::
getFunction(Trans::Transaction& cTrans_) const
{
	BoolFunction0<Function>::Func func = _Bool::_Deleted;
	return const_cast<Database*>(this)->loadFunction(cTrans_).getView(
		getRWLock(), BoolFunction0<Function>(func));
}

//	FUNCTION public
//	Schema::Database::getFunction --
//		データベースに存在する関数のうち、
//		あるスキーマオブジェクト ID の関数を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	functionID
//			関数のスキーマオブジェクト ID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた関数を格納する領域の先頭アドレス
//		0
//			データベースには指定されたスキーマオブジェクト ID の関数は存在しない
//
//	EXCEPTIONS

Function*
Database::
getFunction(ID::Value functionID, Trans::Transaction& cTrans_) const
{
	if (functionID == ID::Invalid)
		return 0;

	const FunctionMap& cMap = const_cast<Database*>(this)->loadFunction(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(functionID).get();
}

//	FUNCTION public
//	Schema::Database::getFunction --
//		データベースに存在する関数のうち、ある名前の関数を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	functionName
//			関数の名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値return
//			得られた関数を格納する領域の先頭アドレス
//		0
//			データベースには指定された名前の関数は存在しない
//
//	EXCEPTIONS

Function*
Database::
getFunction(const Name& functionName, Trans::Transaction& cTrans_) const
{
	const FunctionMap& cMap = const_cast<Database*>(this)->loadFunction(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Function, const Name&>(_Bool::_findByName, functionName));
}

//	FUNCTION public
//	Schema::Database::addFunction --
//		データベースを表すクラスの関数として、指定された関数を表すクラスを追加する
//
//	NOTES
//		「関数」表は更新されない
//
//	ARGUMENTS
//		const Schema::FunctionPointer&		function
//			追加する関数を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加した関数を表すクラス
//
//	EXCEPTIONS

Function&
Database::
addFunction(const FunctionPointer& function, Trans::Transaction& cTrans_)
{
	// 「関数」表のうち、このデータベースに関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた関数を追加する
	(void) loadFunction(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_functions->insert(function);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

	return *function;
}

//	FUNCTION public
//	Schema::Database::eraseFunction --
//		データベースを表すクラスからある関数を表すクラスの登録を抹消する
//
//	NOTES
//		「関数」表は更新されない
//		登録を抹消した関数クラスはdeleteされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	functionID
//			登録を抹消する関数のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
eraseFunction(ID::Value functionID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_functions) {
		(void) _functions->erase(functionID);
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize();
	}

	// 関数はキャッシュに入れていないので
	// 他のもののようにキャッシュから削除する必要はない
}

//	FUNCTION public
//	Schema::Database::resetFunction --
//		データベースには関数を表すクラスが登録されていないことにする
//
//	NOTES
//		「関数」表は更新されない
//		関数以下のオブジェクトをキャッシュから除くことはしない
//		キャッシュのクリアが続くことが分かっているとき専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetFunction()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_functions) {

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_functions->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_functions->reset();

	} else {
		// 関数を表すクラスを登録するハッシュマップを生成する

		_functions = new FunctionMap;
		; _SYDNEY_ASSERT(_functions);
	}
}

//	FUNCTION public
//	Schema::Database::clearFunction --
//		データベースを表すクラスに登録されている関数を表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「関数」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clearFunction()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_functions) {
		// ベクターに登録されている関数を表すクラスがあれば、
		// すべて破棄し、ベクターも破棄する
		resetFunction();
		delete _functions, _functions = 0;
	}
}

// FUNCTION public
//	Schema::Database::addPrivilege -- add Privilege object
//
// NOTES
//
// ARGUMENTS
//	const PrivilegePointer& pPrivilege_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege&
//
// EXCEPTIONS

Privilege&
Database::
addPrivilege(const PrivilegePointer& pPrivilege_, Trans::Transaction& cTrans_)
{
	(void) loadPrivilege(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_privileges->insert(pPrivilege_);

	return *pPrivilege_;
}

// FUNCTION public
//	Schema::Database::erasePrivilege -- erase Privilege object
//
// NOTES
//
// ARGUMENTS
//	ID::Value iPrivilegeID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
erasePrivilege(ID::Value iPrivilegeID_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_privileges) {
		(void) _privileges->erase(iPrivilegeID_);
	}
}

//	FUNCTION public
//	Schema::Database::resetTable --
//		データベースには表を表すクラスが登録されていないことにする
//
//	NOTES
//		「表」表は更新されない
//		表以下のオブジェクトをキャッシュから除くことはしない
//		キャッシュのクリアが続くことが分かっているとき専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetTable()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_tables) {

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_tables->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_tables->reset();

	} else {
		// 表を表すクラスを登録するハッシュマップを生成する

		_tables = new TableMap;
		; _SYDNEY_ASSERT(_tables);
	}
}

//	FUNCTION public
//	Schema::Database::resetTable --
//		データベースには表を表すクラスが登録されていないことにする
//
//	NOTES
//		「表」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
resetTable(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_tables) {

		// ベクターに登録されている表を表すクラスがあれば、
		// すべて破棄し、ベクターを空にする
		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_tables->getSize());

		_tables->reset(cDatabase_);

	} else {
		// 表を表すクラスを登録するハッシュマップを生成する

		_tables = new TableMap;
		; _SYDNEY_ASSERT(_tables);
	}
}

//	FUNCTION public
//	Schema::Database::clearTable --
//		データベースを表すクラスに登録されている表を表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「表」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
clearTable()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_tables) {

		// ベクターに登録されている表を表すクラスがあれば、
		// すべて破棄し、ベクターも破棄する

		resetTable();
		delete _tables, _tables = 0;
	}
}

///////////////////////////////
// Privilege related methods //
///////////////////////////////

// FUNCTION public
//	Schema::Database::loadPrivilege -- load privileges
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool bRecovery_ = false
//	
// RETURN
//	const PrivilegeMap&
//
// EXCEPTIONS

const PrivilegeMap&
Database::
loadPrivilege(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_privileges) {

		l.convert(Os::RWLock::Mode::Write);
		// check again after write-lock succeeded
		if (!_privileges) {

			// read privilege system table
			// if the database object is not permanent object or under creation,
			// and mounted and not in recovery, only reset hashmap

			if ( (getScope() != Scope::Permanent
				  || getStatus() == Status::Created)
				 && !isUnmounted() && !bRecovery_)
				resetPrivilege();
			else {
				// load privilege system table
				SystemTable::Privilege(*this).load(cTrans_, bRecovery_);
			}
			; _SYDNEY_ASSERT(_privileges);
		}
	}
	return *_privileges;
}

// FUNCTION public
//	Schema::Database::getPrivilege -- get privilege value for a user
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	int iUserID_
//	Common::Privilege::Value* pResult_
//	int iMaxElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
getPrivilege(Trans::Transaction& cTrans_,
			 int iUserID_,
			 Common::Privilege::Value* pResult_,
			 int iMaxElement_) const
{
	; _SYDNEY_ASSERT(pResult_);
	; _SYDNEY_ASSERT(iMaxElement_ > 0);

	// SystemDatabase allows reference privilege
	if (getID() == ID::SystemTable) {
		if (Common::Privilege::Category::Reference < iMaxElement_) {
			*(pResult_ + Common::Privilege::Category::Reference) = Common::Privilege::All;
		}
		return;
	}

	Privilege* pPrivilege = getPrivilege(cTrans_, iUserID_);

	if (pPrivilege) {
		const ModVector<Common::Privilege::Value>& vecValue = pPrivilege->getValue();
		int n = ModMin(iMaxElement_, static_cast<int>(vecValue.getSize()));
		Common::Privilege::Value* p = pResult_;
		int i = 0;
		for (; i < n; ++i, ++p) {
			*p = vecValue[i];
		}
		for (; i < iMaxElement_; ++i, ++p) {
			*p = Common::Privilege::None;
		}
	} else {
		// clear privilege
		Common::Privilege::Value* p = pResult_;
		for (int i = 0; i < iMaxElement_; ++i, ++p) {
			*p = Common::Privilege::None;
		}
	}
}

// FUNCTION public
//	Schema::Database::getPrivilege -- get privilege object for a user
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	int iUserID_
//	
// RETURN
//	Privilege*
//
// EXCEPTIONS

Privilege*
Database::
getPrivilege(Trans::Transaction& cTrans_, int iUserID_) const
{
	// load privilege system table
	const PrivilegeMap& cMap = const_cast<Database*>(this)->loadPrivilege(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Privilege, int>(PrivilegeMap::findByID,
												   iUserID_));
}

// FUNCTION public
//	Schema::Database::getPrivilege -- get privilege object by ID
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	ID::Value iID_
//	
// RETURN
//	Privilege*
//
// EXCEPTIONS

Privilege*
Database::
getPrivilege(Trans::Transaction& cTrans_, ID::Value iID_) const
{
	// load privilege system table
	const PrivilegeMap& cMap = const_cast<Database*>(this)->loadPrivilege(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(iID_).get();
}

// FUNCTION public
//	Schema::Database::getRolePrivilege -- get privilege value for a role
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const ModUnicodeString& cstrRoleName_
//	ModVector<Common::Privilege::Value>& vecResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Database::
getRolePrivilege(Trans::Transaction& cTrans_,
				 const ModUnicodeString& cstrRoleName_,
				 ModVector<Common::Privilege::Value>& vecResult_)
{
	; _SYDNEY_ASSERT(vecResult_.getSize() == static_cast<ModSize>(Common::Privilege::Category::ValueNum));
	// for now, only built-in roles are available
	for (int i = 0; i < Common::Privilege::Category::ValueNum; ++i) {
		if (ObjectName::equals(cstrRoleName_, _Privilege::_cBuiltInRoleName[i])) {
			// set privilege value
			vecResult_[i] = Common::Privilege::All;
			return;
		}
	}
	// no corresponding role
	_SYDNEY_THROW1(Exception::RoleNotFound, cstrRoleName_);
}

// FUNCTION public
//	Schema::Database::getBuiltInRoleName -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Privilege::Category::Value eCategory_
//	
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

//static
const ModUnicodeString&
Database::
getBuiltInRoleName(Common::Privilege::Category::Value eCategory_)
{
	if (eCategory_ >= 0 && eCategory_ < Common::Privilege::Category::ValueNum) {
		return _Privilege::_cBuiltInRoleName[eCategory_];
	}
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Database::resetPrivilege --
//		Erase all the privilege objects from the database object
//
//	NOTES
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
Database::
resetPrivilege()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_privileges) {

		// Empty the hash map
		_privileges->reset();

	} else {
		// create new hash map
		_privileges = new PrivilegeMap;
		; _SYDNEY_ASSERT(_privileges);
	}
}

//	FUNCTION public
//	Schema::Database::clearPrivilege --
//		Destroy object holding privilege objects
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
Database::
clearPrivilege()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_privileges) {
		// clear privilege objects and delete hashmap
		resetPrivilege();
		delete _privileges, _privileges = 0;
	}
}
////////////////////////////////////

//	FUNCTION public
//	Schema::Database::getCache --
//		キャッシュから指定したオブジェクトIDを持つオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value id
//			このオブジェクトIDを持つオブジェクトを得る
//		Schema::Object::Category::Value category = Unknown
//			取得するオブジェクトの種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Object*
Database::
getCache(ID::Value id, Category::Value category)
{
	if (id == Object::ID::Invalid)
		return 0;

	// このメソッドだけ見ればReadでもいいが、
	// Writeでロックしたメソッドから回りまわって
	// 呼ばれる可能性があるのでWriteモードでロックする
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!_cache)
		return 0;
	; _SYDNEY_ASSERT(_cache);

	CacheMap::Iterator iterator = _cache->find(id);

	Object* object =
		(iterator == _cache->end())
		? static_cast<Object*>(0) : CacheMap::getValue(iterator).get();

	if (object) {
		if (category != Object::Category::Unknown &&
			category != object->getCategory()) {

			// 指定されたオブジェクト ID のオブジェクトを表すクラスが
			// 登録されているが、そのオブジェクトの種別がおかしい
			//
			//【注意】	呼び出し側の実装がおかしい以外に、
			//			このようなことはおきえないはず

			/* 例外発生 */

			; _SYDNEY_ASSERT(false);
		}
	}

	return object;
}

//	FUNCTION
//	Schema::Database::addCache --
//		あるオブジェクトを表すクラスをキャッシュに登録する
//
//	NOTES
//		登録されたオブジェクトを表すクラスは、登録したデータベースの
//		Schema::Database::getCache により、
//		そのオブジェクト ID で探索できる
//
//		既に登録されているオブジェクトを再度登録しても、
//		不整合は生じないし、例外も発生しない
//
//	ARGUMENTS
//		const Schema::Object::Pointer&		object
//			登録するオブジェクトを表すクラス
//
//	RETURN
//		登録したオブジェクトを表すクラス
//
//	EXCEPTIONS

const Object::Pointer&
Database::
addCache(const Object::Pointer& object)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 永続的オブジェクトは永続化された後にキャッシュに入れられる

	; _SYDNEY_ASSERT(object->getScope() != Scope::Permanent
					 || object->getStatus() == Status::Persistent);

	if (!_cache)
		resetCache();
	; _SYDNEY_ASSERT(_cache);

	(void) _cache->insert(object->getID(), object);
	if (!isFreezed() && getScope() == Scope::Permanent)
		Manager::ObjectTree::Database::incrementCacheSize();

	return object;
}

//	FUNCTION
//	Schema::Database::eraseCache --
//		あるオブジェクトを表すクラスの登録をデータベースから抹消する
//
//	NOTES
//		オブジェクトを表すクラスは、
//		Schema::Database::addCache により登録できる
//
//		登録されていないオブジェクトの登録を抹消しようとしても、
//		例外は発生しない
//
//		このメソッドにより抹消したオブジェクトはdeleteまでされる
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			登録を抹消するオブジェクトのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
eraseCache(Object::ID::Value id)
{
	// eraseCacheが呼ばれるときはほとんど_cache!=0なのでいきなりWriteでロックする
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cache) {
		ModSize n = _cache->erase(id);
		if (!isFreezed() && n && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(n);
	}
}

//	FUNCTION
//	Schema::Database::resetCache --
//		キャッシュをクリアする
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
Database::
resetCache()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cache) {

		// オブジェクトを表すクラスのうち、
		// 登録されているものがあれば、破棄する

		if (!isFreezed() && getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_cache->getSize());

#ifdef CHECK_CACHE
		while (!_cache->isEmpty()) {
			// キャッシュに永続化されていないものが残っていないか調べる
			checkCache();

			// ハッシュマップから消せばdeleteされる
			_cache->popFront();
		}
#else
		// ハッシュマップを空にすればdeleteされる
		_cache->clear();
#endif
	} else {
		_cache = new CacheMap(_cacheMapSize, _cacheMapEnableLink);
		; _SYDNEY_ASSERT(_cache);
	}
}

//	FUNCTION
//	Schema::Database::clearCache --
//		キャッシュをクリアし、管理用のベクターを破棄する
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
Database::
clearCache()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_cache) {
		resetCache();
		delete _cache, _cache = 0;
	}
}

#ifdef DEBUG

//	FUNCTION
//	Schema::Database::checkCache --
//		キャッシュの中身がすべてPersistか調べる
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
Database::
checkCache() const
{
#ifdef _CHECK_CACHE
	if (_cache) {
		Object* pObj = _cache->getFront().get();
		if (pObj && pObj->getScope() == Scope::Permanent
			&& pObj->getStatus() != Status::Persistent) {
			SydSchemaErrorMessage << "Object not persistent: category="
								  << pObj->getCategory()
								  << " name="
								  << pObj->getName()
								  << " status="
								  << pObj->getStatus()
								  << ModEndl;
		}
	}
#endif
}

#endif

//	FUNCTION public
//	Schema::Database::checkPath --
//		パスが他のデータベースに使われていないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Database* pDatabase_
//			データベースオブジェクト
//		const ModVector<bool>* vecChanged_ = 0
//			パス変更指定の有無を表すBoolean配列へのポインター
//			0ならすべてのカテゴリーで調べる
//		const ModVector<ModUnicodeString>* vecPath_ = 0
//			パス指定配列へのポインター
//			0ならデータベースオブジェクトのパスが使用される
//		bool bEraseExistence_ = false
//			trueのときすでに存在していたら削除してしまう
//			(redo用)
//
//	RETURN
//		true .. 同じパス名のディレクトリーが存在する
//		false.. 同じパス名のディレクトリーは存在しない
//
//	EXCEPTIONS

bool
Database::
checkPath(Trans::Transaction& cTrans_,
		  const ModVector<bool>* vecChanged_,
		  const ModVector<ModUnicodeString>* vecPath_,
		  bool bEraseExistence_)
{
	return _Path::_checkExistence(cTrans_, this, vecChanged_,
								  (vecPath_) ? vecPath_ : m_pPath,
								  bEraseExistence_);
}

//	FUNCTION public
//	Schema::Database::serialize --
//		データベースを表すクラスのシリアライザー
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
Database::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

	} else {

		// メンバーをすべて初期化しておく

		clear();
	}
}

//	FUNCTION public
//	Schema::Database::verify --
//		データベースの整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
verify(Admin::Verification::Progress& cResult_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_)
{
	const ModUnicodeString cstrPath;	// スキーマでProgressに入れるパスは空文字列

	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	// 呼び出し側で検査の経過が良好であることを保証する必要がある
	; _SYDNEY_ASSERT(cResult_.isGood());

	// 対象の名称を設定する
	cResult_.setSchemaObjectName(getName());

	SydSchemaVerifyMessage
		<< "Verify " << getName()
		<< ModEndl;

	// 途中経過を出す
	_SYDNEY_VERIFY_INFO(cResult_, "", Message::VerifyStarted(getName()), eTreatment_);

	ID::Value iMaxObjectID = ID::Invalid;

	// データベースのシステム表に関する整合性検査を行う
	{
		Admin::Verification::Progress cTmp(cResult_.getConnection());
		SystemTable::Database().verify(cTmp, cTrans_, eTreatment_, iMaxObjectID);
		cResult_ += cTmp;
		if (!bContinue && !cResult_.isGood()) {
			// 経過が良好でなければ終了
			return;
		}
	}
	{
		// データベースIDを格納するシーケンスファイルの整合性検査を行う
		Admin::Verification::Progress cTmp(cResult_.getConnection());
		Manager::ObjectTree::Sequence::get().verify(cTmp, cTrans_, eTreatment_, iMaxObjectID);
		cResult_ += cTmp;
		if (!bContinue && !cResult_.isGood()) {
			// 経過が良好でなければ終了
			return;
		}
	}

	iMaxObjectID = ID::Invalid;

	// データベースに属するすべてのシステム表の整合性検査を行う
	for (int i = 0; i < Object::Category::ValueNum; ++i) {
		if (i != Object::Category::Database && i != Object::Category::Unknown) {

			// 中断のポーリング
			Manager::checkCanceled(cTrans_);

			// オブジェクトの種類に応じたシステム表オブジェクトを得る
			ModAutoPointer<SystemTable::SystemFile> pSystemFile =
				SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
			; _SYDNEY_ASSERT(pSystemFile.get());

			Admin::Verification::Progress cTmp(cResult_.getConnection());
			pSystemFile->verify(cTmp, cTrans_, eTreatment_, iMaxObjectID);
			cResult_ += cTmp;

			if (!bContinue && !cResult_.isGood()) {
				// 経過が良好でなければ終了
				return;
			}
		}
	}

	try {
		// ObjectIDを格納するシーケンスファイルの整合性検査を行う
		Admin::Verification::Progress cTmp(cResult_.getConnection());
		getSequence().verify(cTmp, cTrans_, eTreatment_, iMaxObjectID);
		cResult_ += cTmp;
		SCHEMA_FAKE_ERROR("Schema::Database", "Verify", "Sequence");

		if (!bContinue && !cResult_.isGood()) {
			// 経過が良好でなければ終了
			return;
		}
	} catch (Exception::Object& e) {

		_SYDNEY_VERIFY_ABORTED(cResult_, "", e);
		_SYDNEY_RETHROW;

	} catch (...) {

		Exception::Unexpected e(moduleName, srcFile, __LINE__);
		_SYDNEY_VERIFY_ABORTED(cResult_, "", e);
		_SYDNEY_RETHROW;
	}

	// データベースを構成するファイルが存在するか調べる
	if (!isAccessible()) {
		// ファイルが存在しない少なくとも1つのパスが存在しない
		// →修復不可能

		_SYDNEY_VERIFY_INCONSISTENT(cResult_, cstrPath, Message::DatabasePathNotExist());
	}

	// 途中経過を出す
	_SYDNEY_VERIFY_INFO(cResult_, "", Message::VerifyFinished(getName()), eTreatment_);
}

//	FUNCTION public
//	Schema::Database::makeLogData -- 論理ログデータを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			論理ログデータを生成するトランザクションのトランザクション記述子
//		Schema::LogData&	cLogData_
//			生成された論理ログデータで、呼び出し時には
//			cLogData_.getSubCategory() が設定されている必要がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
makeLogData(Trans::Transaction& trans, LogData& cLogData_) const
{
	// 全ログに共通のデータ
	// 1. データベース名
	// 2. ID
	cLogData_.addString(getName());
	cLogData_.addID(getID());

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateDatabase:
	case LogData::Category::Mount:
	{
		//	データベース作成
		//		3．パスリスト
		//		4．属性
		ModVector<ModUnicodeString> vecPath;
		getPath(vecPath);
		cLogData_.addStrings(vecPath);
		cLogData_.addData(packMetaField(Meta::Database::Flag));

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		break;
	}
	case LogData::Category::DropDatabase:
	{
		// DROP DATABASE

		// データベースの格納場所<UNDO>
		ModVector<ModUnicodeString> vecPath;
		getPath(vecPath);
		cLogData_.addStrings(vecPath);

		// エリアの格納場所<UNDO>
		ModVector<ModUnicodeString> vecAreaPaths; 
		getAreaPath(trans, vecAreaPaths);
		cLogData_.addStrings(vecAreaPaths);

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Drop::Num);
		break;
	}
	case LogData::Category::Unmount:
	{
		//	アンマウント
		//		3．パスリスト
		//		4. 前回のチェックポイント終了時のタイムスタンプ

		ModVector<ModUnicodeString> vecPath;
		getPath(vecPath);
		cLogData_.addStrings(vecPath);

		cLogData_.addUnsignedInteger64(
			Checkpoint::TimeStamp::getMostRecent(getLogFile()->getLockName()));

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Unmount::Num);
		break;

	}
	case LogData::Category::MoveDatabase:
	{
		//	データベース移動
		//		<残りはDatabase::alterの中で設定される>
		break;
	}
	case LogData::Category::AlterDatabase:
	case LogData::Category::AlterDatabase_ReadOnly:
	case LogData::Category::AlterDatabase_SetToMaster:

		//	 データベース変更
		//		3. 前回のチェックポイント終了時のタイムスタンプ

		cLogData_.addUnsignedInteger64(
			Checkpoint::TimeStamp::getMostRecent(getLogFile()->getLockName()));

		//		<残りはDatabase::alterの中で設定される>
		break;

	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//	FUNCTION public
//	Schema::Database::getAreaPath --
//		ログに書くためのパスリストを得る
//
//	NOTES
//		削除マークがついていてもリストされる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		ModVector<ModUnicodeString>& vecAreaPath_
//			返り値のパスリストを格納するVector
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Database::
getAreaPath(Trans::Transaction& cTrans_,
			ModVector<ModUnicodeString>& vecAreaPath_) const
{
	; _SYDNEY_ASSERT(vecAreaPath_.isEmpty());

	// Deletedのものも入れるのでMapから調べる
	// ロックの機構によりこのオブジェクトを触るスレッドは1つであるから
	// 排他制御は不要

	const AreaMap&
		cAreas = const_cast<Database*>(this)->loadArea(cTrans_);

	AutoRWLock l(getRWLock());
	AreaMap::ConstIterator iterator = cAreas.begin();
	const AreaMap::ConstIterator& end = cAreas.end();
	vecAreaPath_.reserve(cAreas.getSize());	// 現在は通常エリアのパスは1つずつなので
											// エリアの数でreserveしてもよい

	for (; iterator != end; ++iterator) {
		Area* pArea = AreaMap::getValue(iterator).get();
		if (pArea) {
			const ModVector<ModUnicodeString>& vecPath = pArea->getPath();
			vecAreaPath_.insert(vecAreaPath_.end(),
								vecPath.begin(), vecPath.end());
		}
	}
}

//	FUNCTION public
//	Schema::Database::getObjectID --
//		
//
//	NOTES
//		redo用
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//		   	ログデータから Schema ID を得る
//
//	RETURN
//		対象の Schmea ID
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータから Schema ID が取得できなかった

// static
ObjectID::Value
Database::
getObjectID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::ID);
}

//	FUNCTION public
//	Schema::Database::getName --
//		データベース抹消、変更、移動のログデータにしたがってデータベース名を得る
//
//	NOTES
//		redo用
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			ログデータ
//
//	RETURN
//		対象のデータベース名
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータからデータベース名が取得できなかった

// static
Object::Name
Database::
getName(const LogData& cLogData_)
{
	return cLogData_.getString(Log::Name);
}

//	FUNCTION public
//	Schema::Database::getPath -- データベースの格納場所の絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData&	data
//			スキーマ操作を表す論理ログデータ
//		int iIndex_
//			ログデータの中でパス指定が格納されている位置
//		ModVector<ModUnicodeString>&	path
//			得られた絶対パス名を要素として格納するベクタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
getPath(const LogData& cLogData_, int iIndex_, ModVector<ModUnicodeString>& vecPath_)
{
	vecPath_ = cLogData_.getStrings(iIndex_);
}

//	FUNCTION public
//	Schema::Database::getAreaPath -- データベースに属するエリアの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData&	data
//			スキーマ操作を表す論理ログデータ
//		ModVector<ModUnicodeString>&	path
//			得られた絶対パス名を要素として格納するベクタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Database::
getAreaPath(const LogData& cLogData_, ModVector<ModUnicodeString>& vecPath_)
{
	vecPath_ = cLogData_.getStrings(Log::Drop::AreaPath);
}

//	FUNCTION public
//	Schema::Database::setAvailability -- データベースが利用可能かを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			利用可能かを設定するデータベースのスキーマオブジェクト識別子
//		bool				v
//			true
//				利用可能にする
//			false
//				利用不可にする
//
//	RETURN
//		true
//			設定前は利用可能だった
//		false
//			設定前は利用不可だった
//
//	EXCEPTIONS

// static
bool
Database::
setAvailability(Object::ID::Value id, bool v)
{
	return Checkpoint::Database::setAvailability(id, v);
}

// オブジェクトメソッド
void
Database::
setAvailability(bool v)
{
	(void) Checkpoint::Database::setAvailability(getID(), v);
}

//	FUNCTION public
//	Schema::Dabase::isAvailable -- データベースが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			利用可能かを調べるデータベースのスキーマオブジェクト識別子
//
//	RETURN
//		true
//			利用可能である
//		false
//			利用不可である
//
//	EXCEPTIONS

// static
bool
Database::
isAvailable(Object::ID::Value id)
{
	return SystemTable::isAvailable() && Checkpoint::Database::isAvailable(id);
}

// オブジェクトメソッド
bool
Database::
isAvailable()
{
	return SystemTable::isAvailable() && Checkpoint::Database::isAvailable(getID());
}

//	FUNCTION protected
//		Schema::Database::setCreateOption
//			構築の為のオプションを設定する
//
//	NOTES
//		PathOption, ReadWriteOption, OnlineOption を設定する
//
//	ARGUMENTS
//		Statement::DatabaseCreateOptionList& cStatement_
//			設定の為の Statement オブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
setCreateOption(const Statement::DatabaseCreateOptionList& cStatement_)
{
	using namespace Statement;

	int iCntMax = cStatement_.getCount();
	DatabaseCreateOption* pcOption = 0;
	for ( int iCnt = 0; iCnt < iCntMax; iCnt++ )
	{
		pcOption = _SYDNEY_DYNAMIC_CAST(DatabaseCreateOption*,
										cStatement_.getAt(iCnt));
		; _SYDNEY_ASSERT(pcOption);

		switch ( pcOption->getOptionType() )
		{
		case DatabaseCreateOption::PathOption:
		{
			DatabasePathElementList* pPathList
				= _SYDNEY_DYNAMIC_CAST(DatabasePathElementList*,
									   pcOption->getOption());
			; _SYDNEY_ASSERT(pPathList);

			// パスの設定
			setPath(*pPathList);

			break;
		}
		case DatabaseCreateOption::ReadWriteOption:
		{
			IntegerValue* pcInteger =
				_SYDNEY_DYNAMIC_CAST(IntegerValue*, pcOption->getOption());
			; _SYDNEY_ASSERT(pcInteger);

			setReadOnly(
				pcInteger->getValue() == DatabaseCreateOption::ReadOnly);

			break;
		}
		case DatabaseCreateOption::OnlineOption:
		{
			IntegerValue* pcInteger =
				_SYDNEY_DYNAMIC_CAST(IntegerValue*, pcOption->getOption());
			; _SYDNEY_ASSERT(pcInteger);

			setOnline(pcInteger->getValue() == DatabaseCreateOption::Online);

			break;
		}
		case DatabaseCreateOption::RecoveryOption:
		{
			IntegerValue* pcInteger =
				_SYDNEY_DYNAMIC_CAST(IntegerValue*, pcOption->getOption());
			; _SYDNEY_ASSERT(pcInteger);

			setRecoveryFull(
				pcInteger->getValue() == DatabaseCreateOption::RecoveryFull);

			break;
		}
		case DatabaseCreateOption::UserModeOption:
		{
			IntegerValue* pcInteger =
				_SYDNEY_DYNAMIC_CAST(IntegerValue*, pcOption->getOption());
			;_SYDNEY_ASSERT(pcInteger);
			
			setSuperUserMode(pcInteger->getValue() == DatabaseCreateOption::SuperUser);
			break;
		}
		}
	}
}

//	FUNCTION private
//	Schema::Database::moveData --
//		データベースを構成するデータファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//		const Schema::Os::Path& cPrevPath_
//			変更前のパス
//		const Schema::Os::Path& cNewPath_
//			変更後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
moveData(Trans::Transaction& cTrans_,
		 const Os::Path& cPrevPath_,
		 const Os::Path& cNewPath_,
		 bool bUndo_,
		 bool bRecovery_)
{
	// エラー処理のためにどこまで進んだかを示す状態変数
	enum {
		None,
		Moved,
		ValueNum
	} eStatus = None;

	// データベースに属する表をすべて得る
	const TableMap& cTables = loadTable(cTrans_);

	// moveData実行中は他のスレッドからTableMapが変更されることはないので
	// 排他制御はいらない

	// エラー処理でどこまで処理が進んだかを調べるため
	// ここでイテレーターを宣言しておく
	TableMap::ConstIterator iterator = cTables.begin();

	try {
		const TableMap::ConstIterator& end = cTables.end();

		for (; iterator != end; ++iterator) {
			const Table::Pointer& pTable = TableMap::getValue(iterator);

			// 表の移動関数を呼ぶ
			pTable->movePath(cTrans_, cPrevPath_, cNewPath_, bUndo_, bRecovery_);
			eStatus = Moved;
		}
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveData", "Moved");
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveData", "MovedFatal");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Moved:
			{
				SCHEMA_FAKE_ERROR("Schema::Database", "MoveData", "MovedFatal");
				// エラーの起きた表までを元に戻す
				TableMap::ConstIterator errIterator = cTables.begin();
				for (; errIterator != iterator; ++errIterator) {
					const Table::Pointer& pTable = TableMap::getValue(errIterator);
					// 古いパスで移動関数を呼ぶ
					pTable->movePath(cTrans_, cNewPath_, cPrevPath_, true);
				}
			}
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Schema::Database::moveLogicalLog --
//		データベースの論理ログファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//		const Schema::Os::Path& cPrevPath_
//			変更前のパス
//		const Schema::Os::Path& cNewPath_
//			変更後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
moveLogicalLog(Trans::Transaction& cTrans_,
			   const Os::Path& cPrevPath_,
			   const Os::Path& cNewPath_,
			   bool bUndo_,
			   bool bRecovery_)
{
	Os::Path cNewPath(cNewPath_);

#ifdef OBSOLETE // Redoの仕様変更によりbRecovery_==trueで呼ばれることはなくなった
	if (bRecovery_) {
		// リカバリー中は移動前と移動後の
		// どちらにファイルがあるか調べる必要がある

		// まず移動前の場所にあるか調べる
		Os::Path cPrevPath(cPrevPath_);

		if (!getLogFile(cPrevPath)->isAccessible()) {
			// 移動前にない -> 移動後にあるか調べる
			if (!getLogFile(cNewPath)->isAccessible()) {
				// 移動後にもない -> データベースが壊れている

				SydSchemaErrorMessage
					<< "Lost logical log file. DB=" << getName()
					<< ModEndl;
				_SYDNEY_THROW0(Exception::LogFileCorrupted);
			}
			// 移動後にあるなら移動の必要はない

			// 新しいパスの論理ログをsetLogしておく
			// ★注意★
			//	redo中はログ出力はされないので、ここでsetLogしなおしても問題ないはず

			cTrans_.setLog(*this);
			return;
		}
	}
#endif

	// 移動する
	cTrans_.renameLog(Trans::Log::File::Category::Database, cNewPath);
}

//	FUNCTION private
//	Schema::Database::moveSystem --
//		データベースのシステム表を移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			データベース変更を行うトランザクション記述子
//		const Schema::Os::Path& cPrevPath_
//			変更前のパス
//		const Schema::Os::Path& cNewPath_
//			変更後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Database::
moveSystem(Trans::Transaction& cTrans_,
		   const Os::Path& cPrevPath_,
		   const Os::Path& cNewPath_,
		   bool bUndo_,
		   bool bRecovery_)
{
	// エラー処理のためにどこまで進んだかを示す状態変数
	enum {
		None,
		ObjectIDMoved,
		Moved,
		ValueNum
	} eStatus = None;

	int i = 0;				// どこまで処理が進んだかを調べるのに使う

	try {
		// オブジェクトIDを生成するシーケンスファイルを移動する
		Os::Path cNewPath(cNewPath_);
		cNewPath.addPart(PathParts::Sequence::ObjectID);

		getSequence().move(cTrans_, cNewPath, false, bRecovery_);
		eStatus = ObjectIDMoved;
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveSystem", "ObjectID");

		// データベースに属するすべてのシステム表を移動する
		for (; i < Object::Category::ValueNum; ++i) {
			if (i != Object::Category::Database && i != Object::Category::Unknown) {

				// オブジェクトの種類に応じたシステム表オブジェクトを得る
				ModAutoPointer<SystemTable::SystemFile> pSystemFile =
					SystemTable::getSystemFile(static_cast<Object::Category::Value>(i), this);
				; _SYDNEY_ASSERT(pSystemFile.get());

				pSystemFile->move(cTrans_, cPrevPath_, cNewPath_, bUndo_, bRecovery_);
				eStatus = Moved;
			}
		}
		SCHEMA_FAKE_ERROR("Schema::Database", "MoveSystem", "Moved");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getID());

		switch (eStatus) {
		case Moved:
			{
				// 移動したシステム表について元に戻す
				for (int iErr = 0; iErr < i; ++iErr) {
					if (iErr != Object::Category::Database && iErr != Object::Category::Unknown) {
						ModAutoPointer<SystemTable::SystemFile> pSystemFile =
							SystemTable::getSystemFile(static_cast<Object::Category::Value>(iErr), this);
						; _SYDNEY_ASSERT(pSystemFile.get());

						pSystemFile->move(cTrans_, cNewPath_, cPrevPath_, true);
					}
				}
				// thru.
			}
		case ObjectIDMoved:
			{
				// 移動したタプルIDを元に戻す
				Os::Path cPrevPath(cPrevPath_);
				cPrevPath.addPart(PathParts::Sequence::ObjectID);
				getSequence().move(cTrans_, cPrevPath, true);
				// thru.
			}
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Schema::Database::initializeSystemTables --
//		システム表を表す表オブジェクトを管理する配列を初期化する
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
Database::
initializeSystemTables() const
{
	AutoRWLock l(getRWLock());
	if (!m_pSystemTables) {
		l.convert(Os::RWLock::Mode::Write);
		if (!m_pSystemTables) {
			m_pSystemTables = new ModVector<Table*>(Object::Category::ValueNum, 0);

			// システム表のスキーマオブジェクトIDは
			// カテゴリーの最大値に対応するものから順に小さくしていく
			m_iSystemTableObjectID = Table::getSystemTableID(Object::Category::ValueNum);
		}
	}
}

//	FUNCTION private
//	Schema::Database::clearSystemTables --
//		システム表を表す表オブジェクトを管理する配列を破棄する
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
Database::
clearSystemTables() const
{
	if (m_pSystemTables) {
		// Tableオブジェクト自体はキャッシュの破棄でdeleteされる
		delete m_pSystemTables, m_pSystemTables = 0;
	}
}

// FUNCTION private
//	Schema::Database::initializeVirtualTables -- prepare vector for virtual tables
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
Database::
initializeVirtualTables() const
{
	AutoRWLock l(getRWLock());
	if (!m_pVirtualTables) {
		l.convert(Os::RWLock::Mode::Write);
		if (!m_pVirtualTables) {
			m_pVirtualTables = new ModVector<TablePointer>(VirtualTable::Category::ValueNum, TablePointer());
		}
	}
}

// FUNCTION private
//	Schema::Database::clearVirtualTables -- clear vector for virtual tables
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
Database::
clearVirtualTables() const
{
	if (m_pVirtualTables) {
		delete m_pVirtualTables, m_pVirtualTables = 0;
	}
}

// FUNCTION private
//	Schema::Database::getVirtualTable -- get virtual table
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

Table*
Database::
getVirtualTable(int iCategory_, Trans::Transaction& cTrans_) const
{
	; _SYDNEY_ASSERT(iCategory_ > 0);
	; _SYDNEY_ASSERT(iCategory_ < VirtualTable::Category::ValueNum);

	initializeVirtualTables();

	AutoRWLock l(getRWLock());

	if ((*m_pVirtualTables)[iCategory_].get() == 0) {
		l.convert(Os::RWLock::Mode::Write);
		if ((*m_pVirtualTables)[iCategory_].get() == 0) {
			(*m_pVirtualTables)[iCategory_] =
				VirtualTable::create(cTrans_, *this,
									 static_cast<VirtualTable::Category::Value>(iCategory_));
		}
	}
	return (*m_pVirtualTables)[iCategory_].get();
}

//	FUNCTION private
//	Schema::Database::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			エリアが属するデータベースオブジェクト
//		Schema::Object::ID::Value iID_
//			検査に使用するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Database::
checkUndo(ID::Value iID_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(iID_, Undo::Type::MoveDatabase)) {
		ModVector<ModUnicodeString> vecPath;
		if (Manager::RecoveryUtility::Path::getUndoDatabasePath(iID_, vecPath)) {
			// Alter後のパスが登録されているときはログデータのパスではなく
			// Alter後のパスを使用する
			setPath(vecPath);
		}
	}
}

//	FUNCTION private
//	Schema::Database::setSnapshot --
//		オブジェクトを格納するスナップショットを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectSnapshot* pSnapshot_
//			設定するスナップショット
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Database::
setSnapshot(ObjectSnapshot* pSnapshot_)
{
	m_pSnapshot = pSnapshot_;
}

//	FUNCTION public
//	Schema::Database::open
//		データベースの使用開始を宣言する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Database::
open()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// m_pSnapshot == 0のときも参照カウンターは変化させる
	if (++m_iReference == 1 && m_bDelayedClear && m_pSnapshot) {
		// 初めてのopenなのでキャッシュのクリアが後回しになっていたら
		// 登録から除く
		m_pSnapshot->eraseDelayedClear(getID());
#ifdef OBSOLETE // this feature has few effect, so delete this
		if (_cFreezeEnabled) {
			// 解凍する
			melt();
		}
#endif
		m_bDelayedClear = false;
	}
}

//	FUNCTION public
//	Schema::Database::close --
//		データベースの使用終了を宣言する
//
//	NOTES
//
//	ARGUMENTS
//		bool bVolatile_ = false
//			trueのときキャッシュをクリアされやすいようにする
//			falseのときキャッシュをクリアされにくいようにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Database::
close(bool bVolatile_ /* = false */)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// m_pSnapshot == 0のときも参照カウンターは変化させる
	if (--m_iReference <= 0 && m_pSnapshot) {
		m_iReference = 0;
		if (!bVolatile_) {
			// volatileでなければ後回しの方を先にクリアする
			if (Manager::ObjectTree::Database::checkCacheSize()) {
				// 超えていたらキャッシュをクリアする
				Manager::ObjectTree::Database::clearCache();
			}
		}
		// データベースが使用不可になっているか、
		// キャッシュの数が制限値を超えていたら自身のキャッシュをクリアする
		if (!isAvailable() || Manager::ObjectTree::Database::checkCacheSize()) {
			abandonCache();
		} else {
			// 超えていなかったので後回しするデータベースとして登録する
#ifdef OBSOLETE // this feature has few effect, so delete this
			if (_cFreezeEnabled) {
				freeze(); // 凍結する
			}
#endif
			m_pSnapshot->addDelayedClear(getID(), bVolatile_);
			m_bDelayedClear = true;
		}
	}
}

//	FUNCTION private
//	Schema::Database::abandonCache --
//		データベースが保持する下位オブジェクトのキャッシュを捨てる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Database::
abandonCache()
{
	if (m_iReference <= 0) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		if (m_iReference <= 0) {
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "Snapshot " << getSnapshotID()
				<< " Database " << getID() << " abandonCache size= area: " << (_areas?_areas->getSize():0)
				<< " table: " << (_tables?_tables->getSize():0)
				<< " function: " << (_functions?_functions->getSize():0)
				<< " privilege: " << (_privileges?_privileges->getSize():0)
				<< " cache: " << (_cache?_cache->getSize():0)
				<< " freezed: " << m_iFreezedSize
				<< ModEndl;
#endif
			clearSequence();
			clearArea();
			clearTable();
			clearFunction();
			clearPrivilege();
			clearSystemTables();
			clearCache();
			// freezeしていたらそれも消す
			clearFreezed();
		}
	}
}

#ifdef DEBUG

//	FUNCTION private
//	Schema::Database::getSnapshotID --
//		データベースが属するスナップショットオブジェクトのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ID
//
//	EXCEPTIONS

unsigned int
Database::
getSnapshotID() const
{
	return m_pSnapshot?m_pSnapshot->getID():0;
}
#endif

//	FUNCTION private
//	Schema::Database::freeze --
//		どこからも参照されていないデータベースオブジェクトの内容を圧縮して保持する
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
Database::
freeze()
{
	// 排他制御は呼び出し側で行う

	// アーカイブするためのオブジェクト
	Utility::BinaryData cArchiver;

	m_iFreezedSize = 0;
	// Areaのfreeze
	{
		ModSize n = (_areas) ? _areas->getSize() : 0;
		cArchiver.out() << n;

		if (n) {
			_areas->writeObject(cArchiver.out());
			m_iFreezedSize += n;
		}
	}

	// Tableのfreeze
	{
		ModSize n = (_tables) ? _tables->getSize() : 0;
		cArchiver.out() << n;

		if (n) {
			_tables->writeObject(cArchiver.out());
			m_iFreezedSize += n;
		}
	}

	// Functionのfreeze
	{
		ModSize n = (_functions) ? _functions->getSize() : 0;
		cArchiver.out() << n;

		if (n) {
			_functions->writeObject(cArchiver.out());
			m_iFreezedSize += n;
		}
	}

	// Privilegeのfreeze
	{
		ModSize n = (_privileges) ? _privileges->getSize() : 0;
		cArchiver.out() << n;

		if (n) {
			_privileges->writeObject(cArchiver.out());
			m_iFreezedSize += n;
		}
	}
	m_iFreezedSize += ((_cache) ? _cache->getSize() : 0);

	// BinaryDataにする
	m_pFreezedData = cArchiver.freeze(true /* compressed */);

	// キャッシュをクリアする
	clearSequence();
	clearArea();
	clearTable();
	clearFunction();
	clearPrivilege();
	clearSystemTables();
	clearCache();
}

//	FUNCTION private
//	Schema::Database::melt --
//		freezeされていたデータベースオブジェクトを参照可能な形に戻す
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
Database::
melt()
{
	// 排他制御は呼び出し側で行う
	if (isFreezed()) {

		// BinaryDataから固めたデータを得る
		Utility::BinaryData cArchiver;
		cArchiver.melt(m_pFreezedData);

		// Areaのmelt
		{
			ModSize n;
			cArchiver.in(this) >> n;
			if (n) {
				resetArea();
				_areas->readObject(cArchiver.in(this), n);
			}
		}
		// Tableのmelt
		{
			ModSize n;
			cArchiver.in(this) >> n;
			if (n) {
				resetTable();
				_tables->readObject(cArchiver.in(this), n);
			}
		}
		// Functionのmelt
		{
			ModSize n;
			cArchiver.in(this) >> n;
			if (n) {
				resetFunction();
				_functions->readObject(cArchiver.in(this), n);
			}
		}
		// Privilegeのmelt
		{
			ModSize n;
			cArchiver.in(this) >> n;
			if (n) {
				resetPrivilege();
				_privileges->readObject(cArchiver.in(this), n);
			}
		}
		m_pFreezedData = Common::Data::Pointer();
		m_iFreezedSize = 0;
	}
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「データベース」表の構造は以下のとおり
// create table Database (
//		ID			id,
//		name		nvarchar,
//		path		<string array>, -- パス名の配列
//		time		timestamp,
//		unsigned int				-- フラグ
//			0:bool	ReadOnly,		-- 読取属性
//			1:bool	Online			-- オンライン属性
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Database>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Database>(Meta::MemberType::_type_, &Database::_get_, &Database::_set_)

	Meta::Definition<Database> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE0(StringArray),	// Path
		_DEFINE0(Timestamp),	// Timestamp
		_DEFINE0(UnsignedInteger), // Flag
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Database::getMetaFieldNumber --
//		スキーマオブジェクトを格納するファイルのフィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

int
Database::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Database::MemberNum);
}

//	FUNCTION public
//	Schema::Database::getMetaFieldDefinition --
//		スキーマオブジェクトを格納するファイルのフィールド定義を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			フィールドのメンバーを識別する番号
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

Meta::MemberType::Value
Database::
getMetaMemberType(int iMemberID_) const
{
	if (iMemberID_ == Meta::Database::MasterURL) {
		return Meta::MemberType::String;
	}
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Database::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するため
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		0以外...正しく変換された
//		0    ...変換に失敗した
//
//	EXCEPTIONS

Common::Data::Pointer
Database::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Database>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::UnsignedInteger:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Database::Flag);
			return pack(m_cAttribute.getFlag());
		}
	case Meta::MemberType::StringArray:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Database::Path);
			ModVector<ModUnicodeString> vecPath;
			getPath(vecPath);
			if (m_cAttribute.m_cstrMasterURL.getLength()) {
				// 下位互換性のためPathArrayに含めて永続化する
				vecPath.pushBack(m_cAttribute.m_cstrMasterURL);
			}
			return pack(vecPath);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Database::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

bool
Database::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Database>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::UnsignedInteger:
		{
			unsigned int iFlag;
			if (unpack(pData_, iFlag)) {
				m_cAttribute.setFlag(iFlag);
				return true;
			}
			break;
		}
	case Meta::MemberType::StringArray:
		{
			ModVector<ModUnicodeString> vecPath;
			if (unpack(pData_, vecPath)) {
				if (vecPath.getSize() > Path::Category::ValueNum) {
					// 下位互換性のためPathArrayに含めて永続化する
					m_cAttribute.m_cstrMasterURL = vecPath.getBack();
					vecPath.popBack();
				}
				setPath(vecPath);
				return true;
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

//	FUNCTION public
//	Schema::Database::isFreezed
//		キャッシュが圧縮されているかを返す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true...圧縮されている
//		false..圧縮されていない
//
//	EXCEPTIONS

bool
Database::
isFreezed() const
{
	return (m_pFreezedData.get() != 0);
}

//	FUNCTION public
//	Schema::Database::clearFreezed
//		圧縮されているキャッシュを解放する
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
Database::
clearFreezed()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	Manager::ObjectTree::Database::decrementCacheSize(m_iFreezedSize);
	m_pFreezedData = Common::Data::Pointer();
	m_iFreezedSize = 0;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
