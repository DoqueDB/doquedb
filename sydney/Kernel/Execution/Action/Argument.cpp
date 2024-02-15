// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Argument.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Argument.h"
#include "Execution/Externalizable.h"

#include "Common/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_ACTION_USING

////////////////////////////
// Action::Argument::

////////////////////////////
// Action::LockerArgument::

// FUNCTION public
//	Action::LockerArgument::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerArgument::
serialize(ModArchive& archiver_)
{
	if (archiver_.isLoad()) {
		archiver_ >> m_cTableName;
		int iMode;
		archiver_ >> iMode;
		m_eMode = static_cast<Lock::Mode::Value>(iMode);
		int iDuration;
		archiver_ >> iDuration;
		m_eDuration = static_cast<Lock::Duration::Value>(iDuration);

		archiver_ >> m_bIsPrepare;
		archiver_ >> m_bIsUpdate;
		archiver_ >> m_bIsCollection;
		archiver_ >> m_bIsSimple;

	} else {
		archiver_ << m_cTableName;
		int iMode = m_eMode;
		archiver_ << iMode;
		int iDuration = m_eDuration;
		archiver_ << iDuration;

		archiver_ << m_bIsPrepare;
		archiver_ << m_bIsUpdate;
		archiver_ << m_bIsCollection;
		archiver_ << m_bIsSimple;
	}
}

// FUNCTION public
//	Action::LockerArgument::isNeedLock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
LockerArgument::
isNeedLock() const
{
	return m_eMode != Lock::Mode::N;
}

//
//	Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
