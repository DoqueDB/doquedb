// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- 再構成中のファイルへの論理ログ反映関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_FILE_H
#define	__SYDNEY_ADMIN_FILE_H

#include "Admin/Module.h"

#include "Trans/LogFile.h"

template <class T>
class ModLess;
template <class K, class V, class C>
class ModMap;
template <class T>
class ModVector;

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
	class File;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN
_SYDNEY_ADMIN_REORGANIZATION_BEGIN

//	CLASS
//	Admin::Reorganization::File -- 再構成ファイルへの論理ログの反映を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class File
{
public:
	// 再構成中の索引ファイルへ直前の反映中に実行された操作を反映する
	SYD_ADMIN_FUNCTION
	static Trans::Log::LSN
	reflect(Trans::Transaction& trans,
			const Schema::Database& database, Schema::File& file,
			Trans::Log::LSN startLSN);

private:
	// 直前の反映中に実行されたトランザクションの操作のうち、
	// REDO, UNDO する操作に関連する論理ログのログシーケンス番号を求める
	static Trans::Log::LSN
	findRelatedLSN(Trans::Transaction& trans, Trans::Log::File& logFile,
				   Trans::Log::LSN startLSN,
				   ModMap<Trans::Log::LSN, Trans::Log::LSN,
						  ModLess<Trans::Log::LSN> >& rollbackedLSN,
				   ModVector<Trans::Log::LSN>& ignoredLSN);

	// 再構成中の索引ファイルへの反映処理として、更新操作を UNDO する
	static void
	undo(Trans::Transaction& trans,
		 Schema::File& file, Trans::Log::File& logFile,
		 const ModMap<Trans::Log::LSN, Trans::Log::LSN,
					  ModLess<Trans::Log::LSN> >& rollbackedLSN);
	// 再構成中の索引ファイルへの反映処理として、更新操作を REDO する
	static void
	redo(Trans::Transaction& trans,
		 Schema::File& file, Trans::Log::File& logFile,
		 Trans::Log::LSN startLSN, Trans::Log::LSN lastLSN,
		 const ModMap<Trans::Log::LSN, Trans::Log::LSN,
					  ModLess<Trans::Log::LSN> >& rollbackedLSN,
		 const ModVector<Trans::Log::LSN>& ignoredLSN);
};

_SYDNEY_ADMIN_REORGANIZATION_END
_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_FILE_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
