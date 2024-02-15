// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionSlaveID.cpp -- スレーブIDを管理する
// 
// Copyright (c) 1999, 2001, 2005, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Communication/ConnectionSlaveID.h"

#include "Common/Message.h"
#include "Os/AutoCriticalSection.h"

_TRMEISTER_USING

using namespace Communication;

//
//	VARIABLE private
//	Communication::ConnectionSlaveID::m_iLastID -- 次のスレーブID
//
//	NOTES
//	次のスレーブID
//
int Communication::ConnectionSlaveID::m_iLastID
							= Communication::ConnectionSlaveID::Minimum;

//
//	VARIABLE private
//	Communication::ConnectionSlaveID::m_cCriticalSection
//								-- 排他制御用のクリティカルセクション
//
//	NOTES
//	排他制御用のクリティカルセクション
//
Os::CriticalSection Communication::ConnectionSlaveID::m_cCriticalSection;

//
//	FUNCTION public static
//	Communication::ConnectionSlaveID::allocateID -- 新たなスレーブIDを得る
//
//	NOTES
//	新たなスレーブIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スレーブID。オーバーフローした場合は ConnectionSlaveID::Undefined を
//		返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ConnectionSlaveID::allocateID()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (m_iLastID == Maximum)
	{
		//オーバーフロー
		SydErrorMessage << "Slave ID overflow." << ModEndl;
		return Undefined;
	}
	int iID = m_iLastID++;
	return iID;
}

//
//	FUNCTION public static
//	Communication::ConnectionSlaveID::isNormal
//										-- 正しいものかどうかチェックする
//
//	NOTES
//	スレーブIDが正しいものかどうかチェックする
//
//	ARGUMENTS
//	int iID_
//		スレーブID
//
//	RETURN
//	正しいものの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
ConnectionSlaveID::isNormal(int iID_)
{
	return iID_ >= Minimum && iID_ < Maximum;
}

//
//	Copyright (c) 1999, 2001, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
