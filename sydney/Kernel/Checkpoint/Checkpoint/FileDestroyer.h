// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDestoryer.h -- ファイルの破棄処理を行うクラス関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_FILEDESTROYER_H
#define	__SYDNEY_CHECKPOINT_FILEDESTROYER_H

#include "Checkpoint/Module.h"
#include "Schema/ObjectID.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}
namespace LogicalFile
{
	class File;
	class FileDriver;
}
namespace PhysicalFile
{
	class File;
}
namespace Trans
{
	namespace Log
	{
		class File;
	}
	class Transaction;
}

_SYDNEY_CHECKPOINT_BEGIN

class FileDestroyer
{
	friend class Executor;
public:
	// 論理ファイルをチェックポイント処理時に破棄するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans,
		  Schema::ObjectID::Value dbID,
		  const LogicalFile::FileDriver& driver,
		  const LogicalFile::File& file);
#ifdef OBSOLETE
	// 物理ファイルをチェックポイント処理時に破棄するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans, const PhysicalFile::File& file);
	// 論理ログファイルをチェックポイント処理時に破棄するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans, const Trans::Log::File& file);
#endif
	// OS ディレクトリをチェックポイント処理時に破棄するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(Schema::ObjectID::Value dbID,
		  const Os::Path& path, bool onlyDir);

	// 論理ファイルの破棄依頼を取り消す
	SYD_CHECKPOINT_FUNCTION
	static void
	erase(const Trans::Transaction& trans,
		  const LogicalFile::FileDriver& driver,
		  const LogicalFile::File& file);
	// OS ディレクトリの破棄依頼を取り消す
	SYD_CHECKPOINT_FUNCTION
	static void
	erase(const Os::Path& path);

private:
	// 破棄が依頼されたオブジェクトを可能な限り実際に破棄する
	static void
	execute(bool force);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_FILEDESTROYER_H

//
// Copyright (c) 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
