// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IDNumber.h -- 論理ファイル共通オプションのヘッダーファイル
// 
// Copyright (c) 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_IDNUMBER_H
#define __SYDNEY_FILECOMMON_IDNUMBER_H

// Common
#include "Common/Common.h"
// FileCommon
#include "FileCommon/FileOption.h"
// Schema
#include "Schema/ObjectID.h"
// Lock
#include "Lock/Name.h"

_SYDNEY_BEGIN

namespace FileCommon
{

	//
	//	CLASS
	//	IDNumber -- 
	//
	//	NOTES
	//	FileID から必要なパラメータを取り出す補助クラス
	//
	class IDNumber
	{
	public:
		IDNumber(const LogicalFile::FileID& cFileID_)
			: m_iDatabaseID( Schema::ObjectID::Invalid )
			, m_iTableID( Schema::ObjectID::Invalid )
			, m_iFileObjectID( Schema::ObjectID::Invalid )
		{
			// IDNumber （を含むパラメータ）がコピーされる場合を考慮すると、
			// cFileID_ の参照を保持して getInteger() 呼び出しを遅延させることはできない。
			int val;
			if ( cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::DatabaseID::Key) ,val ) ) {
				m_iDatabaseID = val;
			}
			if ( cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::TableID::Key) ,val) ) {
				m_iTableID = val;
			}
			if ( cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FileObjectID::Key) ,val) ) {
				m_iFileObjectID = val;
			}
		}

		Lock::FileName getLockName() const
		{
			// Lock::FileName() のポインタ内部で持って、オブジェクト生成をキャッシュする方法もあるが
			// 帰り値最適化がされるなら Lock::FileName() のコンストラクタは小さいので、
			// メモリアローケートの方が不利になる可能性がある。そのまま実体を返すことにする。
			return Lock::FileName(m_iDatabaseID ,m_iTableID ,m_iFileObjectID);
		}

		Lock::FileName getLockName(Schema::ObjectID::Value iDatabaseID
		                          ,Schema::ObjectID::Value iTableID
		                          ,Schema::ObjectID::Value iFileObjectID
		                          ) const
		{
			// Lock::FileName() のポインタ内部で持って、オブジェクト生成をキャッシュする方法もあるが
			// 帰り値最適化がされるなら Lock::FileName() のコンストラクタは小さいので、
			// メモリアローケートの方が不利になる可能性がある。そのまま実体を返すことにする。
			return Lock::FileName(iDatabaseID ,iTableID ,iFileObjectID);
		}

		Schema::ObjectID::Value getDatabaseID()   const { return m_iDatabaseID; }
		Schema::ObjectID::Value getTableID()      const { return m_iTableID; }
		Schema::ObjectID::Value getFileObjectID() const { return m_iFileObjectID; }

	private:
		Schema::ObjectID::Value m_iDatabaseID;
		Schema::ObjectID::Value m_iTableID;
		Schema::ObjectID::Value m_iFileObjectID;
	};

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_IDNUMBER_H

//
//	Copyright (c) 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
