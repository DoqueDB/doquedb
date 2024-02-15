// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.cpp -- 
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Admin/LogData.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING
_SYDNEY_ADMIN_LOG_USING

namespace
{
}

//
//	FUNCTION public
//	Admin::Log::ReplicationEndData::ReplicationEndData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Log::ReplicationEndData::ReplicationEndData()
	: Super(Trans::Log::Data::Category::ReplicationEnd),
	  m_ulLastMasterLSN(Trans::Log::IllegalLSN)
{
}

//
//	FUNCTION public
//	Admin::Log::ReplicationEndData::~ReplicationEndData -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Log::ReplicationEndData::~ReplicationEndData()
{
}

//
//	FUNCTION public
//	Admin::Log::ReplicationEndData::serialize -- このクラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& archiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Log::ReplicationEndData::serialize(ModArchive& archiver_)
{
	// まず、親クラスをシリアル化する

	Super::serialize(archiver_);

	// 自分自身をシリアル化する

	if (archiver_.isStore())
	{
		ModSize size = m_vecBeginLSN.getSize();
		archiver_ << size;

		ModVector<Trans::Log::LSN>::Iterator i = m_vecBeginLSN.begin();
		for (; i != m_vecBeginLSN.end(); ++i)
		{
			archiver_ << (*i);
		}

		archiver_ << m_ulLastMasterLSN;
	}
	else
	{
		ModSize size;
		archiver_ >> size;
		m_vecBeginLSN.reserve(size);

		for (ModSize i = 0; i < size; ++i)
		{
			Trans::Log::LSN lsn;
			archiver_ >> lsn;
			m_vecBeginLSN.pushBack(lsn);
		}

		archiver_ >> m_ulLastMasterLSN;
	}
}

//
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
