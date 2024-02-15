// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.cpp --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "LogicalFile/LogData.h"
#include "LogicalFile/Externalizable.h"

#include "ModArchive.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

//
//  FUNCTION public
//  LogicalFile::LogData::LogData -- コンストラクタ(1)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
LogData::LogData()
	: Super()
{
}

//
//  FUNCTION public
//  LogicalFile::LogData::LogData -- コンストラクタ(2)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
LogData::LogData(const Type::Value eType_)
    : Super(Trans::Log::Data::Category::DriverModify),
	  m_eType(eType_)
{
}

//
//  FUNCTION public
//  LogicalFile::LogData::~LogData -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
LogData::~LogData()
{
}

//  FUNCTION public
//  LogicalFile::LogData::serialize
//      -- このクラスをシリアル化する
//
//  NOTES
//
//  ARGUMENTS
//
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LogData::serialize(ModArchive& archiver_)
{
	// まず、親クラスをシリアル化する
	Super::serialize(archiver_);

	// 自分自身をシリアル化する
	
	if (archiver_.isStore())
	{
		int tmp = m_eType;
		archiver_ << tmp;
	}
	else
	{
		int tmp;
		archiver_ >> tmp;
		m_eType = static_cast<Type::Value>(tmp);
	}
}

//
//	FUNCTION public
//	LogicalFile::FullTextMergeLog::FullTextMegeLog
//		-- コンストラクタ
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
FullTextMergeLog::FullTextMergeLog()
	: LogData(Type::FullTextMerge)
{
}

//
//	FUNCTION public
//	LogicalFile::FullTextMergeLog::~FullTextMergeLog
//		-- デストラクタ
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
FullTextMergeLog::~FullTextMergeLog()
{
}


//
//  FUNCTION public
//  LogicalFile::FullTextMergeLog::getClassID
//      -- このクラスのクラス ID を得る
//
//  NOTES
//
//  ARGUMENTS
//	なし
//
//  RETURN
//	int
//		クラスID
//
//  EXCEPTIONS
//
int 
FullTextMergeLog::getClassID() const
{
	return LogicalFile::Externalizable::Category::FullTextMergeLog +
			Common::Externalizable::LogicalFileClasses;
}

//
//	FUNCTION public
//	LogicalFile::KdTreeMergeLog::KdTreeMegeLog
//		-- コンストラクタ
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
KdTreeMergeLog::KdTreeMergeLog()
	: LogData(Type::KdTreeMerge)
{
}

//
//	FUNCTION public
//	LogicalFile::KdTreeMergeLog::~KdTreeMergeLog
//		-- デストラクタ
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
KdTreeMergeLog::~KdTreeMergeLog()
{
}


//
//  FUNCTION public
//  LogicalFile::KdTreeMergeLog::getClassID
//      -- このクラスのクラス ID を得る
//
//  NOTES
//
//  ARGUMENTS
//	なし
//
//  RETURN
//	int
//		クラスID
//
//  EXCEPTIONS
//
int 
KdTreeMergeLog::getClassID() const
{
	return LogicalFile::Externalizable::Category::KdTreeMergeLog +
			Common::Externalizable::LogicalFileClasses;
}

//
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
