// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionMasterID.cpp -- マスタIDを管理する
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Communication/ConnectionMasterID.h"

_TRMEISTER_USING
using namespace Communication;

//
//	FUNCTION public static
//	Communication::ConnectionMasterID::isNormal
//										-- 正しいものかどうかチェックする
//
//	NOTES
//	マスターIDが正しいものかどうかチェックする
//
//	ARGUMENTS
//	int iID_
//		マスターID
//
//	RETURN
//	正しいものの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
ConnectionMasterID::isNormal(int iID_)
{
	return iID_ >= Minimum && iID_ < Maximum;
}

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
