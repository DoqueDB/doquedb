// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OutputArchive.h -- オブジェクトをポートに書き込む
// 
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_OUTPUTARCHIVE_H
#define __TRMEISTER_COMMON_OUTPUTARCHIVE_H

#include "Common/Object.h"
#include "ModArchive.h"

_TRMEISTER_BEGIN

namespace Common
{
class Externalizable;

//
//	CLASS
//		Common::OutputArchive -- 出力用アーカイブ
//	NOTES
//		ModArchiveを拡張したクラスである。
//		ModArchiveはModDefaultObjectを継承しているので、
//		Common::Objectのサブクラスにすることはできない
//
class SYD_COMMON_FUNCTION OutputArchive : public ModArchive
{
public:
	//コンストラクタ
	OutputArchive(ModSerialIO& cIO_);
	//デストラクタ
	virtual ~OutputArchive();

	//Common::Externalizableの派生クラスを書く
	void writeObject(const Common::Externalizable* pObject_);
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMON_OUTPUTARCHIVE_H

//
//	Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

