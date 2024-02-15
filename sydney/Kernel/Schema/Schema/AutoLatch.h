// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLatch.h -- オートラッチ関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_SCHEMA_AUTOLATCH_H
#define	__SYDNEY_SCHEMA_AUTOLATCH_H

#include "Schema/Module.h"

#include "Trans/AutoLatch.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Lock
{
	class Name;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class File;

namespace SystemTable
{
	class SystemFile;
}

//	CLASS
//	Schema::AutoLatch -- オートラッチを表すクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class AutoLatch
{
public:
	// コンストラクター
	AutoLatch(Trans::Transaction& trans,
			  const Lock::Name& name, bool force = false);
	AutoLatch(Trans::Transaction& trans, const File& file, bool force = false);
	AutoLatch(Trans::Transaction& trans,
			  const SystemTable::SystemFile& file, bool force = false);

	// ラッチをはずす
	void					unlatch();

private:
	ModAutoPointer<Trans::AutoLatch>	_latch;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_AUTOLATCH_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
