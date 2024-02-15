// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Recovery.cpp -- 自動リカバリー関連の関数定義
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyTypeName.h"

#include "Schema/Manager.h"
#include "Schema/Database.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/ObjectID.h"
#include "Schema/Privilege.h"
#include "Schema/Recovery.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"

#include "Exception/LogItemCorrupted.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "ModAutoPointer.h"
#include "ModMap.h"
#include "ModPair.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

#ifdef PURIFY
#define SCHEMA_USE_STATIC_AUTOPOINTER
#endif

#ifdef SCHEMA_USE_STATIC_AUTOPOINTER
#define _AutoPointer(__type__)	ModAutoPointer<__type__ >
#define _IsValidPointer(__pointer__)	(__pointer__.get() != 0)
#define _Free(__pointer__)	{__pointer__ = 0;}
#else
#define _AutoPointer(__type__)	__type__*
#define _IsValidPointer(__pointer__)	(__pointer__ != 0)
#define _Free(__pointer__)	{delete __pointer__; __pointer__ = 0;}
#endif

//	VARIABLE
//	_vecMountingDatabases -- マウント後のリカバリー処理をしているデータベース
//
//	NOTES
//		UNDO処理においてリカバリー処理がマウント後のものか自動リカバリーかで
//		処理内容が異なる場合があるためこれらを区別するために使用する

ModVector<Database*> _vecMountingDatabases;

//
//	TYPEDEF
//		UndoObjectMap -- UNDOした操作を覚えておくためのマップ
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の
//		マップを使用するためにマップの配列が用意される
typedef ModMap< Object::ID::Value,
				Manager::RecoveryUtility::Undo::Type::Value,
				ModLess<Object::ID::Value> >	UndoObjectMap;
typedef ModVector<ModPair<Object::Name, UndoObjectMap> >	UndoObjectMapVector;

_AutoPointer(UndoObjectMap)			_undoObjectMap = 0;
_AutoPointer(UndoObjectMapVector)	_undoObjectMapVector = 0;

struct UndoDatabaseInfo
{
	Object::ID::Value	_usedIDMax;

};

//
//	TYPEDEF
//		UndoPathMap -- UNDOにより実際には移動しなかったパスを覚えておくためのマップ
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の
//		マップを使用するためにマップの配列が用意される
typedef ModMap< Object::ID::Value, ModVector<ModUnicodeString>, ModLess<Object::ID::Value> > UndoPathMap;
typedef ModVector<ModPair<Object::Name, UndoPathMap> > UndoPathMapVector;
_AutoPointer(UndoPathMap)		_undoDatabasePathMap = 0;
_AutoPointer(UndoPathMapVector)	_undoAreaPathMapVector = 0;

//
//	TYPEDEF
//		UndoAreaMap -- UNDOにより実際には変更しなかったエリアIDを覚えておくためのマップ
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の
//		マップを使用するためにマップの配列が用意される
typedef ModMap< Object::ID::Value, ModVector<ObjectID::Value>, ModLess<Object::ID::Value> > UndoAreaMap;
typedef ModVector<ModPair<Object::Name, UndoAreaMap> > UndoAreaMapVector;
_AutoPointer(UndoAreaMapVector)	_undoAreaMapVector = 0;

//
//	TYPEDEF
//		UndoNameMap -- UNDOにより実際には変更しなかった名前を覚えておくためのマップ
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の
//		マップを使用するためにマップの配列が用意される
typedef ModMap< Object::ID::Value, Object::Name, ModLess<Object::ID::Value> > UndoNameMap;
typedef ModVector<ModPair<Object::Name, UndoNameMap> > UndoNameMapVector;
_AutoPointer(UndoNameMapVector)	_undoNameMapVector = 0;
_AutoPointer(UndoNameMapVector)	_undoFileNameMapVector = 0;

//
//	TYPEDEF
//		UnremovableAreaMap -- パス以下を破棄してはいけないエリアIDを覚えておくためのマップ
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の
//		マップを使用するためにマップの配列が用意される
typedef ModMap< Object::ID::Value, int, ModLess<Object::ID::Value> > UnremovableAreaMap;
typedef ModVector<ModPair<Object::Name, UnremovableAreaMap> > UnremovableAreaMapVector;
_AutoPointer(UnremovableAreaMapVector)	_unremovableAreaMapVector = 0;

//
//	TYPEDEF
//		UndoPrivilegeMap -- Map to record final privilege which changed by grant/revoke
//
//	NOTES
typedef ModMap< Object::ID::Value, ModVector<Common::Privilege::Value>, ModLess<Object::ID::Value> > UndoPrivilegeMap;
typedef ModVector<ModPair<Object::Name, UndoPrivilegeMap> > UndoPrivilegeMapVector;
_AutoPointer(UndoPrivilegeMapVector)	_undoPrivilegeMapVector = 0;
_AutoPointer(UndoPrivilegeMapVector)	_undoFilePrivilegeMapVector = 0;

//
//	TYPEDEF
//		UsedIDMax -- ログに記録されているIDの最大値
//
//	NOTES
//		データベースに属するオブジェクトはデータベースごとに別々の値を覚える

typedef Object::ID::Value UsedIDMax;
typedef ModVector<ModPair<Object::Name, UsedIDMax> > UsedIDMaxVector;
_AutoPointer(UsedIDMaxVector)	_usedIDMaxVector = 0;

//	VARIABLE
//	$$$::_exceptPathList -- rmAllの対象から除外するパスのリスト
//
//	NOTES
ModVector<ModUnicodeString> _exceptPathList;

//	VARIABLE
//	$$$::_usedPathList -- 使用するパスのリスト
//
//	NOTES
ModVector<ModUnicodeString> _usedPathList;

//	VARIABLE
//	$$$::_criticalSection -- このファイルで使用する変数を保護する
//
//	NOTES
Os::CriticalSection _criticalSection;

//	TEMPLATE FUNCTION local
//	$$::_findMapVector --
//
//	TEMPLATE ARGUMENTS
//		class _MAPVECTOR_
//			処理対象のMapVectorの型
//		class _MAP_
//			処理対象のMapの型
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& databaseName_
//			初期化したいマップのデータベース名
//		_AutoPointer(_MAPVECTOR)& map_
//			処理対象のMapVector
//		bool& bNew_
//			引数で渡されたときにtrueならば、存在しないときに新たに追加する
//				このとき返り値としてすでに存在していたらfalse
//				新たにMapが追加されたらtrueが入れられる
//			引数で渡されたときにfalseならば、存在しないときに何もしない
//
//	RETURN
//
//	EXCEPTIONS

template <class _MAPVECTOR_, class _MAP_>
_MAP_*
_findMapVector(const Object::Name& databaseName_, _AutoPointer(_MAPVECTOR_)& map_, bool& bNew_)
{
	if (_IsValidPointer(map_)) {
		typename _MAPVECTOR_::Iterator iterator = map_->begin();
		const typename _MAPVECTOR_::Iterator& end = map_->end();
		for (; iterator != end; ++iterator) {
			if ((*iterator).first == databaseName_) {
				bNew_ = false;
				if (iterator != map_->begin()) {
					// 続いて同じIDで調べることが予想されるので先頭と交換する
					ModPair<Object::Name, _MAP_> tmp = *(map_->begin());
					(*map_->begin()) = *iterator;
					*iterator = tmp;
					iterator = map_->begin();
				}
				return &(*iterator).second;
			}
		}
	} else if (bNew_) {
		map_ = new _MAPVECTOR_;
	}

	// ここまで来るときは見つからなかったとき
	if (bNew_) {
		// 続いて同じIDで調べることが予想されるので先頭に追加する
		map_->pushFront(ModPair<Object::Name, _MAP_>(databaseName_, _MAP_()));
		bNew_ = true;
		return &(map_->getFront().second);
	}
	// 作るように指示されていないときは0を返す
	return 0;
}

//	TEMPLATE FUNCTION local
//	$$::_eraseMapVector --
//
//	TEMPLATE ARGUMENTS
//		class _MAPVECTOR_
//			処理対象のMapVectorの型
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& databaseName
//			破棄したいマップのデータベース名
//		_AutoPointer(_MAPVECTOR)& map_
//			処理対象のMapVector
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _MAPVECTOR_>
void
_eraseMapVector(const Object::Name& databaseName_, _AutoPointer(_MAPVECTOR_)& map_)
{
	if (_IsValidPointer(map_)) {
		typename _MAPVECTOR_::Iterator iterator = map_->begin();
		const typename _MAPVECTOR_::Iterator& end = map_->end();
		for (; iterator != end; ++iterator) {
			if ((*iterator).first == databaseName_) {
				map_->erase(iterator);
				break;
			}
		}
		if (map_->getSize() == 0) {
			_Free(map_);
		}
	}
}

} // namespace

///////////////////////////////////////
//	Schema::Manager::RecoveryUtility //
///////////////////////////////////////

//	FUNCTION
//	Schema::Manager::RecoveryUtility::terminate
//		-- リカバリー用に使用したマップ等を開放する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::
terminate()
{
	_Free(_usedIDMaxVector);
	_Free(_unremovableAreaMapVector);
	_Free(_undoPrivilegeMapVector);
	_Free(_undoNameMapVector);
	_Free(_undoAreaMapVector);
	_Free(_undoDatabasePathMap);
	_Free(_undoAreaPathMapVector);
	_Free(_undoObjectMap);
	_Free(_undoObjectMapVector);
	_exceptPathList.clear();
	_usedPathList.clear();
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::initialize --
//		マウント時の回復処理のための初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::
initialize(Schema::Database& database)
{
	Undo::enterMounting(database);
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::terminate --
//		マウント時の回復処理のための終了処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::
terminate(Schema::Database& database)
{
	_eraseMapVector<UsedIDMaxVector>(database.getName(), _usedIDMaxVector);
	_eraseMapVector<UnremovableAreaMapVector>(
		database.getName(), _unremovableAreaMapVector);
	_eraseMapVector<UndoPrivilegeMapVector>(database.getName(), _undoPrivilegeMapVector);
	_eraseMapVector<UndoNameMapVector>(database.getName(), _undoNameMapVector);
	_eraseMapVector<UndoAreaMapVector>(database.getName(), _undoAreaMapVector);
	_eraseMapVector<UndoPathMapVector>(
		database.getName(), _undoAreaPathMapVector);
	_eraseMapVector<UndoObjectMapVector>(
		database.getName(), _undoObjectMapVector);

	Undo::removeMounting(database);
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::enter
//		-- Undo 中の Schema オブジェクトを登録する
//
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Undo::
enter(ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( !_IsValidPointer(_undoObjectMap) )
		_undoObjectMap = new UndoObjectMap;

	UndoObjectMap::Iterator it = _undoObjectMap->find(id_);

	if ( it == _undoObjectMap->end() ) {
		// 新規に登録
		_undoObjectMap->insert(id_, type_);
	} else {
#ifdef DEBUG
		// CREATEやDROPなら同じ内容で二度登録されることはない
		switch (type_) {
		case Undo::Type::CreateDatabase:
		case Undo::Type::DropDatabase:
		case Undo::Type::Mount:
		case Undo::Type::Unmount:
			; _SYDNEY_ASSERT(((*it).second & type_) != type_);
		}
#endif
		(*it).second |= type_;
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::enter
//		-- Undo 中の Schema オブジェクトを登録する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Undo::
enter(const Object::Name& databaseName_, ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UndoObjectMap* map = _findMapVector<UndoObjectMapVector, UndoObjectMap>(databaseName_, _undoObjectMapVector, bNew);

	if (bNew) {
		// 新規に登録
		map->insert(id_, type_);
	} else {
		UndoObjectMap::Iterator it = map->find(id_);
		if ( it == map->end() ) {
			// 新規に登録
			map->insert(id_, type_);
		} else {
#ifdef DEBUG
			// CREATEやDROPなら同じ内容で二度登録されることはない
			switch (type_) {
			case Undo::Type::CreateArea:
			case Undo::Type::DropArea:
			case Undo::Type::CreateTable:
			case Undo::Type::DropTable:
			case Undo::Type::CreateIndex:
			case Undo::Type::DropIndex:
				; _SYDNEY_ASSERT(((*it).second & type_) != type_);
			}
#endif
			(*it).second |= type_;
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::remove
//		-- Undo 中の Schema オブジェクトを解除する
//
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::Undo::
remove(ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( _IsValidPointer(_undoObjectMap) )
	{
		Os::AutoCriticalSection m(_criticalSection);

		UndoObjectMap::Iterator it = _undoObjectMap->find(id_);
		if ( it != _undoObjectMap->end() ) {

			// ビットを落とす
			(*it).second &= ~type_;

			if ((*it).second == 0) {
				// すべてのビットが落ちたら削除する
				_undoObjectMap->erase(it);

				if ( !_undoObjectMap->getSize() )
					// 何も登録されていないならマップを削除
					_Free(_undoObjectMap);
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::remove
//		-- Undo 中の Schema オブジェクトを解除する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::Undo::
remove(const Object::Name& databaseName_, ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoObjectMap* map = _findMapVector<UndoObjectMapVector, UndoObjectMap>(databaseName_, _undoObjectMapVector, bNew);

	if ( map )
	{
		Os::AutoCriticalSection m(_criticalSection);

		UndoObjectMap::Iterator it = map->find(id_);
		if ( it != map->end() ) {

			// ビットを落とす
			(*it).second &= ~type_;

			if ((*it).second == 0) {
				// すべてのビットが落ちたら削除する
				map->erase(it);
				if (map->getSize() == 0) {
					_eraseMapVector<UndoObjectMapVector>(databaseName_, _undoObjectMapVector);
				}
			}
		}
	}
}
	
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::isEntered
//		-- Undo 中の Schema オブジェクトがあるか問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
bool
Manager::RecoveryUtility::Undo::
isEntered(ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( _IsValidPointer(_undoObjectMap) )
	{
		UndoObjectMap::Iterator it  = _undoObjectMap->find(id_);
		if ( it != _undoObjectMap->end() )
			return (((*it).second & type_) == type_);
	}
	return false;
}
	
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::isEntered
//		-- Undo 中の Schema オブジェクトがあるか問い合わせる
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			Schema オブジェクトID
//		Name::Undo::Type::Value type_
//			Undo 種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
bool
Manager::RecoveryUtility::Undo::
isEntered(const Object::Name& databaseName_, ObjectID::Value id_, Undo::Type::Value type_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoObjectMap* map = _findMapVector<UndoObjectMapVector, UndoObjectMap>(databaseName_, _undoObjectMapVector, bNew);

	if ( map )
	{
		UndoObjectMap::Iterator it  = map->find(id_);
		if ( it != map->end() )
			return (((*it).second & type_) == type_ );
	}
	return false;
}
	
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::enterMounting --
//		マウント時の回復処理中であるデータベースを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Undo::
enterMounting(Schema::Database& database)
{
	Os::AutoCriticalSection m(_criticalSection);

	_vecMountingDatabases.pushBack(&database);
}
	
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::removeMounting --
//		マウント時の回復処理中であるデータベースの登録を抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			マウントするデータベースを表すデータベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Undo::
removeMounting(Schema::Database& database)
{
	Os::AutoCriticalSection m(_criticalSection);

	ModVector<Database*>::Iterator ite = _vecMountingDatabases.find(&database);

	if (ite != _vecMountingDatabases.end())
		_vecMountingDatabases.erase(ite);
}
	
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Undo::isMounting --
//		マウント時の回復処理中であるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Object::Name& databaseName
//			調べるデータベースの名前
//
//	RETURN
//		0 以外の値
//			指定された名前のデータベースはマウント時の回復処理中であり、
//			そのデータベースオブジェクトを格納する領域の先頭アドレス
//		0
//			指定された名前のデータベースはマウント字の回復処理中でない
//
//	EXCEPTIONS

Schema::Database*
Manager::RecoveryUtility::Undo::
isMounting(const Object::Name& databaseName)
{
	Os::AutoCriticalSection m(_criticalSection);

	ModVector<Database*>::Iterator			ite(_vecMountingDatabases.begin());
	const ModVector<Database*>::Iterator&	end = _vecMountingDatabases.end();

	for (; ite != end; ++ite) {
		Database* database = *ite;
		; _SYDNEY_ASSERT(database);
		if (database->getName() == databaseName)
			return database;
	}

	return 0;
}

//	FUNCTION public
//	Schema::Manager::RecoveryUtility::Undo::isValidDatabase -- 外部に見せてよいデータベースかを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			データベースオブジェクトのID
//
//	RETURN
//		bool	true  : 外部に見せてよい
//				false : 外部に見せてはいけない
//
//	EXCEPTIONS

bool
Manager::RecoveryUtility::Undo::
isValidDatabase(Schema::Object::ID::Value iID_)
{
	return (!Undo::isEntered(iID_, Undo::Type::DropDatabase)
			&& !Undo::isEntered(iID_, Undo::Type::Unmount));
}

//	FUNCTION public
//	Schema::Manager::RecoveryUtility::Undo::isValidTable -- 外部に見せてよい表かを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::ObjectName& cDatabaseName_
//			データベース名
//		Schema::Object::ID::Value iID_
//			表ID
//
//	RETURN
//		bool	true  : 外部に見せてよい
//				false : 外部に見せてはいけない
//
//	EXCEPTIONS

bool
Manager::RecoveryUtility::Undo::
isValidTable(const Schema::ObjectName& cDatabaseName_, Schema::Object::ID::Value iID_)
{
	return (!Undo::isEntered(cDatabaseName_, iID_, Undo::Type::DropTable));
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::setUndoDatabasePath
//		-- 利用するパスを設定する
// 
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			Schema ID
//		const ModVector<ModUnicodeString>& path_
//			利用される（外部に公開される）パス配列
//
//	RETURN
//		true .. 新規に登録された
//		false.. すでに登録されていた
//
//	EXCEPTIONS

bool
Manager::RecoveryUtility::Path::
setUndoDatabasePath(ObjectID::Value id_, const ModVector<ModUnicodeString>& path_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( !_IsValidPointer(_undoDatabasePathMap) ) {
		_undoDatabasePathMap = new UndoPathMap;
	}

	// 既に登録されているかチェック
	UndoPathMap::Iterator it = _undoDatabasePathMap->find(id_);
	if ( it == _undoDatabasePathMap->end() ) {

		// 新規に登録
		_undoDatabasePathMap->insert(id_, path_);
		return true;
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::getUndoDatabasePath
//		-- 利用するパスを取得する
//
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			
//		bool bDelete_ = false
//
//		const ModVector<ModUnicodeString>& path_
//			利用すべきパス
//
//	RETURN
//		bool
//
//	EXCEPTIONS

bool 
Manager::RecoveryUtility::Path::
getUndoDatabasePath(ObjectID::Value id_, ModVector<ModUnicodeString>& path_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( _IsValidPointer(_undoDatabasePathMap) ) {

		// 既に登録されているかチェック
		UndoPathMap::Iterator it = _undoDatabasePathMap->find(id_);

		if ( it != _undoDatabasePathMap->end() )
		{
			path_ = (*it).second;
			return true;
		}
	}
	return false;
}

#ifdef OBSOLETE // 最後にまとめて破棄されて個別に削除されることがないので使用しない
//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::eraseUndoDatabasePath
//		-- 利用するパスを削除する
//
//	NOTES
//
//	ARGUMENTS
//		ObjectID::Value id_
//			削除対象の Schema ID
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Path::
eraseUndoDatabasePath(ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	if ( _IsValidPointer(_undoDatabasePathMap) ) {

		// 格納パスが 0 なら削除
		_undoDatabasePathMap->erase(id_);
		
		// 登録件数が 0 なら本体を削除
		if ( _undoDatabasePathMap->getSize() <= 0 ) {
			_Free(_undoDatabasePathMap);
		}
	}
}
#endif

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::setUndoAreaPath
//		-- 利用するパスを設定する
// 
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			Schema ID
//		const ModVector<ModUnicodeString>& path_
//			利用される（外部に公開される）パス配列
//
//	RETURN
//		true .. 新規に登録された
//		false.. すでに登録されていた
//
//	EXCEPTIONS

bool
Manager::RecoveryUtility::Path::
setUndoAreaPath(const Object::Name& databaseName_, ObjectID::Value id_, const ModVector<ModUnicodeString>& path_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UndoPathMap* map = _findMapVector<UndoPathMapVector, UndoPathMap>(databaseName_, _undoAreaPathMapVector, bNew);

	if (bNew || map->find(id_) == map->end() ) {
		// 新規に登録
		map->insert(id_, path_);
		return true;
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::getUndoAreaPath
//		-- 利用するパスを取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			
//		bool bDelete_ = false
//
//		const ModVector<ModUnicodeString>& path_
//			利用すべきパス
//
//	RETURN
//		bool
//
//	EXCEPTIONS

bool 
Manager::RecoveryUtility::Path::
getUndoAreaPath(const Object::Name& databaseName_, ObjectID::Value id_, ModVector<ModUnicodeString>& path_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoPathMap* map = _findMapVector<UndoPathMapVector, UndoPathMap>(databaseName_, _undoAreaPathMapVector, bNew);

	if ( map ) {

		// 既に登録されているかチェック
		UndoPathMap::Iterator it = map->find(id_);

		if ( it != map->end() )
		{
			path_ = (*it).second;
			return true;
		}
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::eraseUndoAreaPath
//		-- 利用するパスを削除する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			削除対象の Schema ID
//
//	RETURN
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Path::
eraseUndoAreaPath(const Object::Name& databaseName_, ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoPathMap* map = _findMapVector<UndoPathMapVector, UndoPathMap>(databaseName_, _undoAreaPathMapVector, bNew);

	if ( map ) {
		map->erase(id_);
		// 登録件数が 0 ならMAPを削除
		if ( map->getSize() <= 0 ) {
			_eraseMapVector<UndoPathMapVector>(databaseName_, _undoAreaPathMapVector);
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::addUnremovablePath
//		-- 削除してはいけないパスをリストに加える
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeString& path_
//			パスリストに加えるパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::Path::
addUnremovablePath(const ModUnicodeString& path_)
{
	Os::AutoCriticalSection m(_criticalSection);
	_exceptPathList.pushBack(path_);
}

//	FUNCTION public
//	Schema::Utility::File::rmAllRemovable --
//		除外リストにあるパスを除いてディレクトリーの中身まで含めてすべて削除する
//
//	NOTES
//		ディレクトリーが存在しないときは何もしない
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			削除するディレクトリーのフルパス名
//		bool bForce_ = true
//			true  : すぐ削除する
//			false : Checkpoint モジュールに削除依頼する
//		bool bLookParent_ = true
//			true  : 親ディレクトリーが除外リストにあったときも削除しない
//			false : 自身が除外リストにあったときのみ削除する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::ModLibraryError
//			ModのrmAllが例外を投げた
void
Manager::RecoveryUtility::Path::
rmAllRemovable(const Os::Path& cPath_, bool bForce_, bool bLookParent_)
{
	Os::AutoCriticalSection m(_criticalSection);

	// 指定されたパスと関係のある除外リスト中のパスを集める
	ModVector<ModUnicodeString*> vecpExceptPath;

	ModSize n = _exceptPathList.getSize();
	for (ModSize i = 0; i < n; ++i) {
		// 指定したパスと除外パスリストにあるパスとの関係性を調べる
		switch (cPath_.compare(_exceptPathList[i])) {
		case Os::Path::CompareResult::Identical:
			// 同一なら何もしない
			return;
		case Os::Path::CompareResult::Child:
			// 除外するパスが親ならフラグにより動作が変わる
			if (bLookParent_)
				// 親の指定を継承するなら何もしない
				return;
			else
				// 親は無関係とするなら他のパスについて調べる
				break;
		case Os::Path::CompareResult::Unrelated:
			// 無関係なら除外リストのほかのパスについて調べる
			break;
		case Os::Path::CompareResult::Parent:
			// 除外するパスが子にあるので除外するパスに加える
			vecpExceptPath.pushBack(&_exceptPathList[i]);
			break;
		}
	}
	// リスト中にあるパスを除いてすべて破棄する
	Utility::File::rmAllExcept(cPath_, vecpExceptPath, bForce_);
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::isRemovableAreaPath
//		-- 破棄しても構わないパスかを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		const Os::Path& cPath_
//			破棄しようとしているパス
//
//	RETURN
//		破棄してもよいパスならtrue
//
//	EXCEPTIONS
bool
Manager::RecoveryUtility::Path::
isRemovableAreaPath(const Schema::ObjectName& databaseName_, const Os::Path& cPath_)
{
	Os::AutoCriticalSection m(_criticalSection);
	
	// 参照回数が1以上であるエリアのIDを調べる

	bool bNew = false; // なければ何もしないことを指示する
	UnremovableAreaMap* map = _findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	if (map) {

		// 登録されているエリアのIDごとにパスを調べる
		UnremovableAreaMap::Iterator iterator = map->begin();
		const UnremovableAreaMap::Iterator& end = map->end();
		for (; iterator != end; ++iterator) {
			if ((*iterator).second > 0) {
				// 参照回数が1以上なら登録されているパスを調べる
				ModVector<ModUnicodeString> vecPath;
				if (!Path::getUndoAreaPath(databaseName_, (*iterator).first, vecPath)) {
					// 登録されていないならALTER AREAされていないということなので
					// チェックするパスと同じパスであるはずがない
				} else {
					// 登録されているパスと比較する
					ModSize n = vecPath.getSize();
					for (ModSize i = 0; i < n; ++i) {
						if (cPath_.compare(vecPath[i]) != Os::Path::CompareResult::Unrelated) {
							// 同一、または親子関係にあるので破棄できない
							return false;
						}
					}
				}
			}
		}
	}
	// チェックにかからなければ破棄してよい
	return true;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::setUndoAreaID
//		-- 利用する Area を設定する
// 
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			Schema ID
//		const ModVector<ObjectID::Value>& area_
//			利用される（外部に公開される）area 配列
//
//	RETURN
//		true .. 新規に登録された
//		false.. すでに登録されていた
//
//	EXCEPTIONS

bool
Manager::RecoveryUtility::ID::
setUndoAreaID(const Object::Name& databaseName_, ObjectID::Value id_, const ModVector<ObjectID::Value>& area_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UndoAreaMap* map = _findMapVector<UndoAreaMapVector, UndoAreaMap>(databaseName_, _undoAreaMapVector, bNew);

	// 既に登録されているかチェック
	// 新たに作られた場合は調べる必要はない
	if (bNew || map->find(id_) == map->end()) {
		// 新規に登録
		map->insert(id_, area_);
		return true;
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::getUndoAreaID
//		-- 利用する Area を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			
//		const ModVector<ObjectID::Value>& area_
//			利用すべき Area
//
//	RETURN
//		bool
//
//	EXCEPTIONS

bool 
Manager::RecoveryUtility::ID::
getUndoAreaID(const Object::Name& databaseName_, ObjectID::Value id_, ModVector<ObjectID::Value>& area_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoAreaMap* map = _findMapVector<UndoAreaMapVector, UndoAreaMap>(databaseName_, _undoAreaMapVector, bNew);

	if ( map ) {

		// 既に登録されているかチェック
		UndoAreaMap::Iterator it = map->find(id_);

		if ( it != map->end() )
		{
			area_ = (*it).second;
			return true;
		}
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::eraseUndoAreaID
//		-- 利用する Area の登録を消去する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		ObjectID::Value id_
//			消去するオブジェクトのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::ID::
eraseUndoAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoAreaMap* map = _findMapVector<UndoAreaMapVector, UndoAreaMap>(databaseName_, _undoAreaMapVector, bNew);

	if (map) {
		map->erase(id_);
		// 登録件数が 0 ならMAPを削除
		if ( map->getSize() <= 0 ) {
			_eraseMapVector<UndoAreaMapVector>(databaseName_, _undoAreaMapVector);
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::setUnremovableAreaID
//		-- パス以下を破棄してはいけないエリアのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		Schema::ObjectID::Value iID_
//			削除対象の Schema ID
//		void* pMap_ = 0
//			UnremovableMapへのポインター
//			0ならここで検索する
//			
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::ID::
setUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_, void* pMap_ /* = 0 */)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = (pMap_ == 0); // なければ作ることを指示する
	UnremovableAreaMap* map =
		pMap_ ? syd_reinterpret_cast<UnremovableAreaMap*>(pMap_)
		: _findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	if (iID_ != ObjectID::Invalid) {
		// 既に登録されているかチェック
		// 新たに作られた場合は調べる必要はない
		UnremovableAreaMap::Iterator iterator;
		if (bNew || (iterator = map->find(iID_)) == map->end()) {
			// 新規に登録
			map->insert(iID_, 1);
		} else {
			// 値をインクリメント
			++((*iterator).second);
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::setUnremovableAreaID
//		-- パス以下を破棄してはいけないエリアのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		const ModVector<ObjectID::Value>& vecID_
//			削除対象の Schema ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::ID::
setUnremovableAreaID(const Schema::ObjectName& databaseName_, const ModVector<ObjectID::Value>& vecID_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UnremovableAreaMap* map = _findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	ModSize n = vecID_.getSize();
	for (ModSize i = 0; i < n; ++i) {
		setUnremovableAreaID(databaseName_, vecID_[i], map);
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::eraseUnremovableAreaID
//		-- パス以下を破棄してはいけないエリアのIDの設定を解除する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		Schema::ObjectID::Value iID_
//			削除対象の Schema ID
//		void* pMap_ = 0
//			UnremovableMapへのポインター
//			0ならここで検索する
//			
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::ID::
eraseUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_, void* pMap_ /* = 0 */)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UnremovableAreaMap* map =
		pMap_ ? syd_reinterpret_cast<UnremovableAreaMap*>(pMap_)
		: _findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	if (map) {
		if (iID_ != ObjectID::Invalid) {
			// 既に登録されているかチェック
			UnremovableAreaMap::Iterator iterator = map->find(iID_);
			if (iterator != map->end()) {
				// 値をデクリメントして0になったらエントリーを消す
				if ((--((*iterator).second)) <= 0) {
					map->erase(iID_);
					if (!pMap_ && map->getSize() == 0) {
						// さらに登録件数が0になったらMAPを削除
						_eraseMapVector<UnremovableAreaMapVector>(databaseName_, _unremovableAreaMapVector);
					}
				}
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::eraseUnremovableAreaID
//		-- パス以下を破棄してはいけないエリアのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		const ModVector<ObjectID::Value>& vecID_
//			削除対象の Schema ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::ID::
eraseUnremovableAreaID(const Schema::ObjectName& databaseName_, const ModVector<ObjectID::Value>& vecID_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UnremovableAreaMap* map = _findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	if (map) {
		ModSize n = vecID_.getSize();
		for (ModSize i = 0; i < n; ++i) {
			eraseUnremovableAreaID(databaseName_, vecID_[i], map);
		}
		// 登録件数が0になったらMAPを削除
		if (map->getSize() == 0) {
			_eraseMapVector<UnremovableAreaMapVector>(databaseName_, _unremovableAreaMapVector);
		}
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::isUnremovableAreaID
//		-- パス以下を破棄してはいけないエリアのIDかを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		const ModVector<ObjectID::Value>& vecID_
//			調査対象の Schema ID
//
//	RETURN
//		true .. 破棄してはいけないエリアとして登録されている
//		false.. 破棄してはいけないエリアとして登録されていない
//
//	EXCEPTIONS
bool
Manager::RecoveryUtility::ID::
isUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UnremovableAreaMap* map =
		_findMapVector<UnremovableAreaMapVector, UnremovableAreaMap>(databaseName_, _unremovableAreaMapVector, bNew);

	if (map) {
		// 既に登録されているかチェック
		UnremovableAreaMap::Iterator iterator = map->find(iID_);
		if (iterator != map->end()) {
			// 値が1以上であれば破棄してはいけない
			return ((*iterator).second > 0);
		}
	}
	// 登録されていない
	return false;
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::Path::addUsedPath -- 最終的なパスとして使用されるものを登録する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::RecoveryUtility::Path::
addUsedPath(const ModUnicodeString& cPath_)
{
	Os::AutoCriticalSection m(_criticalSection);
	_usedPathList.pushBack(cPath_);
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::Path::isUsedPath -- 最終的なパスとして登録されているかを得る
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Manager::RecoveryUtility::Path::
isUsedPath(const Os::Path& cPath_)
{
	Os::AutoCriticalSection m(_criticalSection);

	ModSize n = _usedPathList.getSize();
	for (ModSize i = 0; i < n; ++i) {
		// 指定したパスと使用パスリストにあるパスとの関係性を調べる
		switch (cPath_.compare(_usedPathList[i])) {
		case Os::Path::CompareResult::Identical:
			// 同一なら使用されている
			return true;
		case Os::Path::CompareResult::Child:
			// 使用パスが親なら使用されている
			return true;
		case Os::Path::CompareResult::Unrelated:
			// 無関係
			break;
		case Os::Path::CompareResult::Parent:
			// 使用パスが子にある
			break;
		}
	}
	return false;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::Path::getEffectiveDatabasePath --
//		Undo/Redo処理で使用するデータベースパス指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		int iIDIndex_
//			データベースIDを記録してあるログデータ上の位置
//		int iPathIndex_
//			データベースパスを記録してあるログデータ上の位置
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		ModVector<ModUnicodeString>& vecPath_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::Path::
getEffectiveDatabasePath(Trans::Transaction& cTrans_,
						 const LogData& cLogData_,
						 int iIDIndex_, int iPathIndex_,
						 const Object::Name& cDatabaseName_,
						 ModVector<ModUnicodeString>& vecPath_)
{
	// ログ出力時点のデータベースIDを得る

	Object::ID::Value databaseID = cLogData_.getID(iIDIndex_);

	// ALTER DATABASEがUNDOされるなどした場合、最終的なパスが登録されている

	if (!Path::getUndoDatabasePath(databaseID, vecPath_))

		// 登録されていない場合

		getEffectiveDatabasePathInDrop(cTrans_, cLogData_, iIDIndex_,
									   iPathIndex_, cDatabaseName_, vecPath_);
}

//	FUNCTION public
//	Schema::Manager::RecoveryUtility::Path::getEffectiveDatabasePathInDrop --
//		DropのUndo処理で使用するデータベースパス指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		int iIDIndex_
//			データベースIDを記録してあるログデータ上の位置
//		int iPathIndex_
//			データベースパスを記録してあるログデータ上の位置
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		ModVector<ModUnicodeString>& vecPath_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::Path::
getEffectiveDatabasePathInDrop(Trans::Transaction& cTrans_,
							   const LogData& cLogData_,
							   int iIDIndex_, int iPathIndex_,
							   const Object::Name& cDatabaseName_,
							   ModVector<ModUnicodeString>& vecPath_)
{
	if (Database* pcDatabase = Undo::isMounting(cDatabaseName_))

		// マウント時の回復処理中のデータベースである

		pcDatabase->getPath(vecPath_);
	else

		// 自動リカバリ中ならば論理ログに
		// 記録されているパス名を使用する

		vecPath_ = cLogData_.getStrings(iPathIndex_);
}

//	FUNCTION public
//	Schema::Manager::RecoveryUtility::ID::getEffectiveAreaID --
//		Undo/Redo処理で使用するエリアID割り当て指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表操作のログデータ
//		int iIndex_
//			エリアID割り当てが記録されているログデータ上の位置
//		Schema::Object::ID::Value iObjectID_
//			ログに記録されている表または索引のID
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		ModVector<Schema::Object::ID::Value>& vecAreaID_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::RecoveryUtility::ID::
getEffectiveAreaID(const LogData& cLogData_, int iIndex_,
				   Object::ID::Value iObjectID_,
				   const Object::Name& cDatabaseName_,
				   ModVector<Object::ID::Value>& vecAreaID_)
{
	// ALTER TABLEがUNDOされるなどした場合、最終的なエリアIDが登録されている
	if ( !getUndoAreaID(cDatabaseName_, iObjectID_, vecAreaID_) && iIndex_ >= 0  ) {

		// 最終的なエリアIDが登録されていなければログのエリアIDを使用する
		vecAreaID_ = cLogData_.getIDs(iIndex_);
	}
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::setUsedID
//		-- ログに記録されているIDを記録する
// 
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//		Schema::ObjectID::Value id_
//			ログに記録されていたSchema ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::RecoveryUtility::ID::
setUsedID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UsedIDMax* max = _findMapVector<UsedIDMaxVector, UsedIDMax>(databaseName_, _usedIDMaxVector, bNew);

	if (*max < iID_) *max = iID_;
}

//	FUNCTION
//	Schema::Manager::RecoveryUtility::ID::getUsedIDMax
//		-- ログに記録されているIDの最大値を得る
// 
//	NOTES
//
//	ARGUMENTS
//		const Object::Name& databaseName_
//			オブジェクトが属するデータベース名
//
//	RETURN
//		登録されているIDの最大値
//
//	EXCEPTIONS

ObjectID::Value
Manager::RecoveryUtility::ID::
getUsedIDMax(const Schema::ObjectName& databaseName_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UsedIDMax* max = _findMapVector<UsedIDMaxVector, UsedIDMax>(databaseName_, _usedIDMaxVector, bNew);

	return (max) ? *max : 0;
}

// 利用する名前を設定する
bool
Manager::RecoveryUtility::Name::
setUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const Object::Name& name_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoNameMapVector, bNew);

	// 既に登録されているかチェック
	// 新たに作られた場合は調べる必要はない
	if (bNew || map->find(id_) == map->end()) {
		// 新規に登録
		map->insert(id_, name_);
		return true;
	}
	return false;
}

// 利用する名前を取得する
bool
Manager::RecoveryUtility::Name::
getUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Object::Name& name_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoNameMapVector, bNew);

	if ( map ) {

		// 既に登録されているかチェック
		UndoNameMap::Iterator it = map->find(id_);

		if ( it != map->end() )
		{
			name_ = (*it).second;
			return true;
		}
	}
	return false;
}

// 利用する名前の登録を消去する
void
Manager::RecoveryUtility::Name::
eraseUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoNameMapVector, bNew);

	if (map) {
		map->erase(id_);
		// 登録件数が 0 ならMAPを削除
		if ( map->getSize() <= 0 ) {
			_eraseMapVector<UndoNameMapVector>(databaseName_, _undoNameMapVector);
		}
	}
}

// 採用すべき最終的な名前を得る
void
Manager::RecoveryUtility::Name::
getEffectiveName(const LogData& cLogData_, int iIndex_,
				 ObjectID::Value iObjectID_,
				 const Schema::ObjectName& cDatabaseName_,
				 Object::Name& cName_)
{
	// ALTER TABLEがUNDOされるなどした場合、最終的な名前が登録されている
	if ( !getUndoName(cDatabaseName_, iObjectID_, cName_) && iIndex_ >= 0 ) {

		// 最終的な名前が登録されていなければログの名前を使用する
		cName_ = cLogData_.getString(iIndex_);
	}
}

// 利用する名前を設定する
bool
Manager::RecoveryUtility::Name::
setUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const Object::Name& name_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // なければ作ることを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoFileNameMapVector, bNew);

	// 既に登録されているかチェック
	// 新たに作られた場合は調べる必要はない
	if (bNew || map->find(id_) == map->end()) {
		// 新規に登録
		map->insert(id_, name_);
		return true;
	}
	return false;
}

// 利用する名前を取得する
bool
Manager::RecoveryUtility::Name::
getUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Object::Name& name_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoFileNameMapVector, bNew);

	if ( map ) {

		// 既に登録されているかチェック
		UndoNameMap::Iterator it = map->find(id_);

		if ( it != map->end() )
		{
			name_ = (*it).second;
			return true;
		}
	}
	return false;
}

// 利用する名前の登録を消去する
void
Manager::RecoveryUtility::Name::
eraseUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // なければ何もしないことを指示する
	UndoNameMap* map = _findMapVector<UndoNameMapVector, UndoNameMap>(databaseName_, _undoFileNameMapVector, bNew);

	if (map) {
		map->erase(id_);
		// 登録件数が 0 ならMAPを削除
		if ( map->getSize() <= 0 ) {
			_eraseMapVector<UndoNameMapVector>(databaseName_, _undoFileNameMapVector);
		}
	}
}

// 採用すべき最終的な名前を得る
void
Manager::RecoveryUtility::Name::
getEffectiveFileName(const LogData& cLogData_, int iIndex_,
					 ObjectID::Value iObjectID_,
					 const Schema::ObjectName& cDatabaseName_,
					 Object::Name& cName_)
{
	// ALTER TABLEがUNDOされるなどした場合、最終的な名前が登録されている
	if ( !getUndoFileName(cDatabaseName_, iObjectID_, cName_) && iIndex_ >= 0 ) {

		// 最終的な名前が登録されていなければログの名前を使用する
		cName_ = cLogData_.getString(iIndex_);
	}
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::PrivilegeValue::setUndoValue -- set effective privilege value
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cDatabaseName_
//	ObjectID::Value id_
//	const ModVector<Common::Privilege::Value>& vecValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Manager::RecoveryUtility::PrivilegeValue::
setUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_,
			 const ModVector<Common::Privilege::Value>& vecValue_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = true; // create if not exist
	UndoPrivilegeMap* map = _findMapVector<UndoPrivilegeMapVector, UndoPrivilegeMap>(
								 cDatabaseName_, _undoPrivilegeMapVector, bNew);

	// Check whether id is already registered
	if (bNew || map->find(id_) == map->end()) {
		// register
		map->insert(id_, vecValue_);
		return true;
	}
	return false;
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::PrivilegeValue::getUndoValue -- get effective privilege value
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cDatabaseName_
//	ObjectID::Value id_
//	ModVector<Common::Privilege::Value>& vecValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Manager::RecoveryUtility::PrivilegeValue::
getUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_,
			 ModVector<Common::Privilege::Value>& vecValue_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // do nothing if not exist
	UndoPrivilegeMap* map = _findMapVector<UndoPrivilegeMapVector, UndoPrivilegeMap>(
								 cDatabaseName_, _undoPrivilegeMapVector, bNew);
	if (map) {

		// check whether id is registered
		UndoPrivilegeMap::Iterator iterator = map->find(id_);

		if (iterator != map->end()) {
			vecValue_ = (*iterator).second;
			return true;
		}
	}
	return false;
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::PrivilegeValue::eraseUndoValue -- erase undo information for privilege
//
// NOTES
//
// ARGUMENTS
//	const Schema::ObjectName& cDatabaseName_
//	ObjectID::Value id_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::RecoveryUtility::PrivilegeValue::
eraseUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_)
{
	Os::AutoCriticalSection m(_criticalSection);

	bool bNew = false; // do nothing if not exist
	UndoPrivilegeMap* map = _findMapVector<UndoPrivilegeMapVector, UndoPrivilegeMap>(
									 cDatabaseName_, _undoPrivilegeMapVector, bNew);

	if (map) {
		map->erase(id_);
		// If registered value become nothing, delete map itself
		if (map->getSize() <= 0) {
			_eraseMapVector<UndoPrivilegeMapVector>(cDatabaseName_, _undoPrivilegeMapVector);
		}
	}
}

// FUNCTION public
//	Schema::Manager::RecoveryUtility::PrivilegeValue::getEffectiveValue -- get final value for a privilege object
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	int iIndex_
//	ObjectID::Value iObjectID_
//	const Schema::ObjectName& cDatabaseName_
//	ModVector<Common::Privilege::Value>& vecValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::RecoveryUtility::PrivilegeValue::
getEffectiveValue(const LogData& cLogData_, int iIndex_,
				  ObjectID::Value iObjectID_,
				  const Schema::ObjectName& cDatabaseName_,
				  ModVector<Common::Privilege::Value>& vecValue_)
{
	if (!getUndoValue(cDatabaseName_, iObjectID_, vecValue_)) {
		vecValue_ = Privilege::getValue(cLogData_, iIndex_);
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
