// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:map ts=4 sw=4:
//
// KeyMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_KEY_MAP_H
#define	__SYDNEY_SCHEMA_KEY_MAP_H

#include "Schema/ObjectMap.h"
#include "Schema/Key.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS public
//	Schema::KeyMap --
//
//	NOTES

class KeyMap
	: public ObjectMap<Key, KeyPointer>
{
public:
	KeyMap();

	// KeyMapからオブジェクトを得るのに使用する比較関数
	static bool findByColumnID(Key* pKey_, Object::ID::Value iColumnID_);
	static bool findByPosition(Key* pKey_, Key::Position iPosition_);

	// KeyMapをIterationしながら操作する関数
	static void addColumn(ModVector<Column*>& vecColumns_, Trans::Transaction& cTrans_);
};

//	FUNCTION public
//	Schema::KeyMap::findByColumnID -- 対応する列でオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Key* pKey_
//			比較対象のオブジェクト
//		Schema::Object::ID::Value iColumnID_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
KeyMap::
findByColumnID(Key* pKey_, Object::ID::Value iColumnID_)
{
	return (pKey_->getColumnID() == iColumnID_);
}

//	FUNCTION public
//	Schema::KeyMap::findByPosition -- 位置でオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Key* pKey_
//			比較対象のオブジェクト
//		Key::Position iPosition_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
KeyMap::
findByPosition(Key* pKey_, Key::Position iPosition_)
{
	return (pKey_->getPosition() == iPosition_);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_KEY_MAP_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
