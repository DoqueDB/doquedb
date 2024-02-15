// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HashKey.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_UTILITY_HASHKEY_H
#define __SYDNEY_EXECUTION_UTILITY_HASHKEY_H

#include "Execution/Utility/Module.h"
#include "Execution/Declaration.h"

#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

class HashKey
	: public Common::Object
{
public:
	HashKey()
		: m_pData()
	{}
	HashKey(const Common::Data::Pointer& pData_)
		: m_pData(pData_)
	{}
	~HashKey() {}

	ModSize hashCode() const
	{return m_pData->hashCode();}

	bool operator==(const HashKey& cOther_) const
	{return m_pData->equals(cOther_.m_pData.get());}

	HashKey copy()
	{
		return HashKey(m_pData->copy());
	}

protected:
private:
	Common::Data::Pointer m_pData;
};

_SYDNEY_EXECUTION_END
_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_UTILITY_HASHKEY_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
