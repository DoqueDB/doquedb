// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Local.h -- m.kernel/m.communicationのローカルヘッダーファイル
//			  モジュール内で使用するパラメータ名を定義する。
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_LOCAL_H
#define __TRMEISTER_COMMUNICATION_LOCAL_H

_TRMEISTER_BEGIN

namespace Communication
{

namespace ParameterName
{
//
//	パラメータ名
//

//サーバのポート番号
const char* const RemoteConnectionPortNumber = "Communication_RemoteConnectionPortNumber";

} // namespace ParameterName

} // namespace Communication

_TRMEISTER_END

#endif //__TRMEISTER_COMMUNICATION_LOCAL_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
