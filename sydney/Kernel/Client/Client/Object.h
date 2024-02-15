// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h --
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

#ifndef __TRMEISTER_CLIENT_OBJECT_H
#define __TRMEISTER_CLIENT_OBJECT_H

#include "Client/Module.h"
#include "Common/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

//
//	CLASS
//	Client::Object -- クライアントモジュール共通の基底クラス
//
//	NOTES
//
class SYD_FUNCTION Object : public Common::Object
{
public:
	//
	//	STRUCT
	//	Client::Object::Type
	//
	struct Type
	{
		enum Value
		{
			DataSource = 0,
			Port,
			Connection,
			Session,
			ResultSet,
			PrepareStatement,

			ElementSize
		};
	};

	//コンストラクタ
	Object(Type::Value eType_);
	//デストラクタ
	virtual ~Object();

	//型を得る
	Type::Value getType() { return m_eType; }

	//メモリを開放する
	void release();

private:
	//クライアントモジュール内の型
	Type::Value m_eType;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_OBJECT_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
