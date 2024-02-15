// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.h -- Header file of datatype redefine
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

#ifndef __SYDNEY_RECORD2_UTILITY_H
#define __SYDNEY_RECORD2_UTILITY_H


#include "Record2/ObjectID.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	Record2::Utility -- Utility datatype redefine group
//
//	NOTES
//
//
class Utility
{

public:

	typedef ModSize					Size;

	typedef Size						FieldLength;
	typedef Size						ObjectNum;
	//typedef ModUInt64				ObjectNum;
	typedef int						FieldNum;
	typedef	int							ElementNum;
	typedef unsigned short			AreaIDNum;
	typedef unsigned short			AreaSize;
	typedef unsigned short			AreaIndex;
	typedef unsigned short			AreaOffset;
	typedef ModUInt64					TransID;

	static const Size					m_FieldLengthArchiveSize;
	static const Size					m_ElementNumArchiveSize;
	static const Size					m_AreaIDNumArchiveSize;
	static const AreaSize				m_AreaSizeArchiveSize;
	static const Size					m_AreaIndexArchiveSize;
	static const Size					m_AreaOffsetArchiveSize;
	static const Size					m_TransIDArchiveSize;

};

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_UTILITY_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
