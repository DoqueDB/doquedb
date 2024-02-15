// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileMover.h -- ファイルの移動処理を行うクラス関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_FILEMOVER_H
#define	__SYDNEY_CHECKPOINT_FILEMOVER_H

#ifdef OBSOLETE
#include "Checkpoint/Module.h"

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN

namespace Common
{
	template <class T>
	class ObjectPointer;

	class StringArrayData;
}
namespace LogicalFile
{
	class File;
	class FileDriver;
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

class FileMover
{
	friend class Executor;
public:
	// 論理ファイルをチェックポイント処理時に移動するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans,
		  const LogicalFile::FileDriver& driver,
		  const LogicalFile::File& file,
		  const Common::ObjectPointer<Common::StringArrayData>& areas);
	// 物理ファイルをチェックポイント処理時に移動するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans, const PhysicalFile::File& file,
		  const Version::File::StorageStrategy::Path& path);
	// 論理ログファイルをチェックポイント処理時に改名するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Trans::Transaction& trans,
		  const Trans::Log::File& file, const Os::Path& path);
	// OS ディレクトリをチェックポイント処理時に改名するように依頼する
	SYD_CHECKPOINT_FUNCTION
	static void
	enter(const Os::Path& srcPath, const Os::Path& dstPath);

	// ある論理ファイルをチェックポイント処理時に実際に移動する依頼を取り止める
	SYD_CHECKPOINT_FUNCTION
	static bool
	cancel(const Trans::Transaction& trans, const LogicalFile::File& file);
	// ある物理ファイルをチェックポイント処理時に実際に移動する依頼を取り止める
	SYD_CHECKPOINT_FUNCTION
	static bool
	cancel(const Trans::Transaction& trans, const PhysicalFile::File& file);
	// ある論理ログファイルをチェックポイント処理時に
	// 実際に改名する依頼を取り止める
	SYD_CHECKPOINT_FUNCTION
	static bool
	cancel(const Trans::Transaction& trans, const Trans::Log::File& file);
	// ある OS ディレクトリをチェックポイント処理時に
	// 実際に改名する依頼を取り止める
	SYD_CHECKPOINT_FUNCTION
	static bool
	cancel(const Os::Path& srcPath);

private:
	// 移動が依頼されたオブジェクトを可能な限り実際に破棄する
	static void
	execute(bool force);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END
#endif

#endif	// __SYDNEY_CHECKPOINT_FILEMOVER_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
