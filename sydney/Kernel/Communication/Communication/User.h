// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// User.h -- createUser/dropUserに関する定数定義
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_USER_H
#define __TRMEISTER_COMMUNICATION_USER_H

#include "Communication/Module.h"

_TRMEISTER_BEGIN
namespace Communication
{
	namespace User
	{
		// CreateUserに関する定数定義
		struct ID
		{
			//	CONST
			//	Communication::User::ID::Auto -- createUserでUserIDを自動割当することを示す
			//
			//	NOTES

			//	CONST
			//	Communication::User::ID::Min -- createUserでUserIDに指定できる最小値
			//
			//	NOTES

			//	CONST
			//	Communication::User::ID::Max -- createUserでUserIDに指定できる最大値
			//
			//	NOTES
			enum Value
			{
				Auto = -1,
				Min = 1,
				Max = static_cast<int>(0x7fffffff)
			};
		};
		struct DropBehavior
		{
			//	CONST
			//	Communication::User::DropBehavior::Ignore --
			//			dropUserでDB内のPrivilege情報には何もしないことを示す
			//
			//	NOTES

			//	CONST
			//	Communication::User::DropBehavior::Cascade --
			//			dropUserで全DBからRevokeすることを示す
			//
			//	NOTES
			enum Value
			{
				Ignore = 0,
				Cascade = 1
			};
		};

	} // namespace user
} // namespace Communication
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_USER_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
