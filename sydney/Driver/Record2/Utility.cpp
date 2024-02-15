// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.cpp -- 
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Record2/Utility.h"

_SYDNEY_USING
_SYDNEY_RECORD2_USING

// static
const Utility::Size
Utility::m_ElementNumArchiveSize = sizeof(Utility::ElementNum);

// static
const Utility::Size
Utility::m_FieldLengthArchiveSize = sizeof(Utility::FieldLength);

// static
const Utility::Size
Utility::m_AreaIDNumArchiveSize = sizeof(Utility::AreaIDNum);

// static
const Utility::AreaSize
Utility::m_AreaSizeArchiveSize = sizeof(Utility::AreaSize);

// static
const Utility::Size
Utility::m_AreaIndexArchiveSize = sizeof(Utility::AreaIndex);

// static
const Utility::Size
Utility::m_AreaOffsetArchiveSize = sizeof(Utility::AreaOffset);

// static
const Utility::Size
Utility::m_TransIDArchiveSize = sizeof(Utility::TransID);

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
