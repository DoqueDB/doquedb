// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Recovery.h -- 自動リカバリー関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_RECOVERY_H
#define	__SYDNEY_SCHEMA_RECOVERY_H

#include "Schema/Manager.h"
#include "Schema/ObjectID.h"

#include "Common/Privilege.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_SCHEMA_BEGIN

class LogData;
class ObjectName;

namespace Manager
{
	namespace RecoveryUtility
	{
		void terminate();					// 自動リカバリー完了時の後処理を行う

		// マウント時の回復処理のための初期化を行う
		void
		initialize(Schema::Database& database);
		// マウント時の回復処理のための後処理を行う
		void
		terminate(Schema::Database& database);

		namespace Undo
		{
			struct Type
			{
				//	ENUM
				//		Manager::RecoveryList::Undo::Type::Value
				//			-- UNDO 種別
				//	NOTES
				typedef unsigned int Value;
				enum {
					// action
					_Create			= 0x00010000,
					_Drop			= 0x00020000,
					_Alter			= 0x00040000,
					_Move			= 0x00080000,
					_Rename			= 0x00100000,
					_Mount			= 0x00200000,
					_Unmount		= 0x00400000,
					// target
					_Database		= 0x00000001,
					_Area			= 0x00000002,
					_Table			= 0x00000004,
					_Index			= 0x00000008,
					_Column			= 0x00000010,
					_Constraint		= 0x00000020,
					_Privilege		= 0x00000040,
					_Cascade		= 0x00000080,
					_Partition		= 0x00000100,
					_Function		= 0x00000200,

					CreateDatabase	= _Database | _Create,
					MoveDatabase	= _Database | _Move,
					DropDatabase	= _Database | _Drop,
					CreateArea		= _Area | _Create,
					AlterArea		= _Area | _Alter,
					DropArea		= _Area | _Drop,
					CreateTable		= _Table | _Create,
					AlterTable		= _Table | _Alter,
					DropTable		= _Table | _Drop,
					CreateIndex		= _Index | _Create,
					AlterIndex		= _Index | _Alter,
					DropIndex		= _Index | _Drop,
					Mount			= _Database | _Mount,
					Unmount			= _Database | _Unmount,
					RenameTable		= _Table | _Rename,
					RenameIndex		= _Index | _Rename,
					AddColumn		= _Column | _Create,
					AlterColumn		= _Column | _Alter,
					DropColumn		= _Column | _Drop,
					AddConstraint	= _Constraint | _Create,
					DropConstraint	= _Constraint | _Drop,
					CreatePrivilege	= _Privilege | _Create,
					DropPrivilege	= _Privilege | _Drop,
					AlterPrivilege	= _Privilege | _Alter,
					CreateCascade	= _Cascade | _Create,
					DropCascade		= _Cascade | _Drop,
					AlterCascade	= _Cascade | _Alter,
					CreatePartition	= _Partition | _Create,
					DropPartition	= _Partition | _Drop,
					AlterPartition	= _Partition | _Alter,
					CreateFunction	= _Function | _Create,
					DropFunction	= _Function | _Drop,
					AlterFunction	= _Function | _Alter,
					AlterAreaInMount = AlterArea | Mount,
					AlterTableInMount= AlterTable | Mount,
					AlterIndexInMount= AlterIndex | Mount
				};
			};

			void
			enter(ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに対するUNDO操作を登録する
			void
			enter(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに属するオブジェクトのUNDO操作を登録する
			void
			remove(ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに対するUNDO操作の登録を取り消す
			void
			remove(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに属するオブジェクトのUNDO操作の登録を取り消す
			bool
			isEntered(ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに対するUNDO操作が登録されているか問い合わせる
			bool
			isEntered(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Undo::Type::Value type_);
											// データベースに属するオブジェクトのUNDO操作が登録されているか問い合わせる

			// マウント時の回復処理中であるデータベースを登録する
			void
			enterMounting(Schema::Database& database);
			// マウント時の回復処理中であるデータベースの登録を抹消する
			void
			removeMounting(Schema::Database& database);
			// マウント時の回復処理中であるか調べる
			Schema::Database*
			isMounting(const Schema::ObjectName& databaseName);

			bool
			isValidDatabase(ObjectID::Value id_);
											// データベースが外部に見せてよいかを問い合わせる
			bool
			isValidTable(const Schema::ObjectName& databaseName_, ObjectID::Value id_);
											// 表が外部に見せてよいかを問い合わせる
		} // namespace Undo

		namespace Path
		{
			bool
			setUndoDatabasePath(ObjectID::Value id_, const ModVector<ModUnicodeString>& path_);
											// 利用するパスを設定する
			bool
			getUndoDatabasePath(ObjectID::Value id_, ModVector<ModUnicodeString>& str_);
											// 利用するパスを取得する
#ifdef OBSOLETE // 最後にまとめて破棄されて個別に削除されることがないので使用しない
			void
			eraseUndoDatabasePath(ObjectID::Value id_);
											// 利用するパスを削除する
#endif
			bool
			setUndoAreaPath(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const ModVector<ModUnicodeString>& path_);
											// 利用するパスを設定する
			bool
			getUndoAreaPath(const Schema::ObjectName& databaseName_, ObjectID::Value id_, ModVector<ModUnicodeString>& str_);
											// 利用するパスを取得する
			void
			eraseUndoAreaPath(const Schema::ObjectName& databaseName_, ObjectID::Value id_);
											// 利用するパスを削除する
			void
			addUnremovablePath(const ModUnicodeString& path_);
											// リカバリー処理で削除してはいけないパスを加える
			void
			rmAllRemovable(const Os::Path& cPath_, bool bForce_ = true, bool bLookParent_ = true);
											// 削除してはいけないパスを残しながらrmAllする
			bool
			isRemovableAreaPath(const Schema::ObjectName& databaseName_, const Os::Path& cPath_);
											// 破棄しても構わないパスかを得る

			// 最終的なパスとして使用されるものを登録する
			void
			addUsedPath(const ModUnicodeString& cPath_);
			// 最終的なパスとして登録されているかを得る
			bool
			isUsedPath(const Os::Path& cPath_);
		
			// 採用すべき最終的なデータベースパスを得る
			void
			getEffectiveDatabasePath(Trans::Transaction& cTrans_,
									 const LogData& cLogData_, int iIDIndex_, int iPathIndex_,
									 const Schema::ObjectName& cDatabaseName_, ModVector<ModUnicodeString>& vecPath_);
			// DROPのUNDO処理で使用するデータベースパスを得る
			void
			getEffectiveDatabasePathInDrop(Trans::Transaction& cTrans_,
										   const LogData& cLogData_, int iIDIndex_, int iPathIndex_,
										   const Schema::ObjectName& cDatabaseName_, ModVector<ModUnicodeString>& vecPath_);
		} // namespace Path

		namespace ID
		{
			bool
			setUndoAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const ModVector<ObjectID::Value>& area_);
											// 利用するエリアを設定する
			bool
			getUndoAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value id_, ModVector<ObjectID::Value>& area_);
											// 利用するエリアを取得する
			void
			eraseUndoAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value id_);
											// 利用するエリアと登録を消去する
			void
			setUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_, void* pMap_ = 0);
			void
			setUnremovableAreaID(const Schema::ObjectName& databaseName_, const ModVector<ObjectID::Value>& vecID_);
											// パス以下を破棄してはいけないエリアのIDを設定する
			void
			eraseUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_, void* pMap_ = 0);
			void
			eraseUnremovableAreaID(const Schema::ObjectName& databaseName_, const ModVector<ObjectID::Value>& vecID_);
											// パス以下を破棄してはいけないエリアのIDの設定を解除する
			bool
			isUnremovableAreaID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_);
											// パス以下を破棄してはいけないエリアとして登録されているかを調べる

			// 採用すべき最終的なエリア割り当てを得る
			void
			getEffectiveAreaID(const LogData& cLogData_, int iIndex_,
							   ObjectID::Value iObjectID_,
							   const Schema::ObjectName& cDatabaseName_,
							   ModVector<ObjectID::Value>& vecAreaID_);

			// ログに記録されているIDを記録する
			void
			setUsedID(const Schema::ObjectName& databaseName_, ObjectID::Value iID_);

			// ログに記録されているIDの最大値を得る
			ObjectID::Value
			getUsedIDMax(const Schema::ObjectName& databaseName_);

		} // namespace ID

		namespace Name
		{
			// 利用する名前を設定する
			bool
			setUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const Object::Name& name_);

			// 利用する名前を取得する
			bool
			getUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Object::Name& name_);
			// 利用する名前の登録を消去する
			void
			eraseUndoName(const Schema::ObjectName& databaseName_, ObjectID::Value id_);

			// 採用すべき最終的な名前を得る
			void
			getEffectiveName(const LogData& cLogData_, int iIndex_,
							 ObjectID::Value iObjectID_,
							 const Schema::ObjectName& cDatabaseName_,
							 Object::Name& cName_);

			// File names should be registered separatedly from other names.
			// because undoing 'create index' needs file names alterred by succeding 'alter index rename',
			// but log data for 'create index' does not contain the schema object id of the file.
			// ('create index' is already executed in the market, so we can not change the format of the log data.)
			// So, we use the schema object id of the index instead.

			// 利用するファイル名を設定する
			bool
			setUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, const Object::Name& name_);

			// 利用するファイル名を取得する
			bool
			getUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_, Object::Name& name_);
			// 利用するファイル名の登録を消去する
			void
			eraseUndoFileName(const Schema::ObjectName& databaseName_, ObjectID::Value id_);

			// 採用すべき最終的な名前を得る
			void
			getEffectiveFileName(const LogData& cLogData_, int iIndex_,
								 ObjectID::Value iObjectID_,
								 const Schema::ObjectName& cDatabaseName_,
								 Object::Name& cName_);

		} // namespace Name

		namespace PrivilegeValue
		{
			bool setUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_,
							  const ModVector<Common::Privilege::Value>& vecValue_);
			bool getUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_,
							  ModVector<Common::Privilege::Value>& vecValue_);
			void eraseUndoValue(const Schema::ObjectName& cDatabaseName_, ObjectID::Value id_);

			void getEffectiveValue(const LogData& cLogData_, int iIndex_,
								   ObjectID::Value iObjectID_,
								   const Schema::ObjectName& cDatabaseName_,
								   ModVector<Common::Privilege::Value>& vecValue_);
		} // namespace Privilege

	} // namespace RecoveryUtility
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_RECOVERY_H

//
// Copyright (c) 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
