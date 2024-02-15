// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.h --
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

#ifndef __SYDNEY_FILECOMMON_OBJECTID_H
#define __SYDNEY_FILECOMMON_OBJECTID_H

#include "ModTypes.h"

#include "Common/Common.h"
#include "Common/ObjectIDData.h"
#include "LogicalFile/ObjectID.h"

_SYDNEY_BEGIN

namespace FileCommon
{

namespace ObjectID
{
	const ModSize		ArchiveSize = LogicalFile::ObjectID::getArchiveSize();
	const ModUInt64		Undefined = LogicalFile::ObjectID::getMaxValue();
}

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_OBJECTID_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
