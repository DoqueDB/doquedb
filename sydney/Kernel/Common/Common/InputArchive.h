// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InputArchive.h -- 入力用アーカイブ
//					 DataArrayDataで使用するのでCommonに入れる
// 
// Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_INPUTARCHIVE_H
#define __TRMEISTER_COMMON_INPUTARCHIVE_H

#include "Common/Object.h"
#include "ModArchive.h"

_TRMEISTER_BEGIN

namespace Common
{
class Externalizable;

//
//	CLASS
//		Common::InputArchive -- 入力用アーカイブ
//	NOTES
//		ModArchiveを拡張したクラスである。
//		ModArchiveはModDefaultObjectを継承しているので、
//		Common::Objectのサブクラスにすることはできない
//
class SYD_COMMON_FUNCTION InputArchive : public ModArchive
{
public:
	//コンストラクタ
	InputArchive(ModSerialIO& cIO_);
	//デストラクタ
	virtual ~InputArchive();

	//Common::Externalizableの派生クラスを読む
	Externalizable* readObject();
	//Common::Externalizableの派生クラスを読む
	Externalizable* readObject(Externalizable* data_);
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMON_INPUTARCHIVE_H

//
//	Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

