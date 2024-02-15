// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Algorithm.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_UTILITY_ALGORITHM_H
#define __SYDNEY_PLAN_UTILITY_ALGORITHM_H

#include "Plan/Utility/Module.h"
#include "Plan/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

//////////////////////
// Arguments
//////////////////////

// TEMPLATE CLASS
//	Plan::Utility::StorageN -- structure with N members
//
// TEMPLATE ARGUMENTS
//	class A1_
//	...
//	class AN_
//
// NOTES
template <class A1_>
struct Storage1
{
	typedef A1_ Arg1;

	Storage1(A1_ arg1_) : m_arg1(arg1_) {}

	A1_ m_arg1;
};
template <class A1_, class A2_>
struct Storage2
	: public Storage1<A1_>
{
	typedef A2_ Arg2;

	Storage2(A1_ arg1_, A2_ arg2_)
		: Storage1<A1_>(arg1_), m_arg2(arg2_) {}

	A2_ m_arg2;
};
template <class A1_, class A2_, class A3_>
struct Storage3
	: public Storage2<A1_, A2_>
{
	typedef A3_ Arg3;

	Storage3(A1_ arg1_, A2_ arg2_, A3_ arg3_)
		: Storage2<A1_, A2_>(arg1_, arg2_), m_arg3(arg3_) {}

	A3_ m_arg3;
};
template <class A1_, class A2_, class A3_, class A4_>
struct Storage4
	: public Storage3<A1_, A2_, A3_>
{
	typedef A4_ Arg4;

	Storage4(A1_ arg1_, A2_ arg2_, A3_ arg3_, A4_ arg4_)
		: Storage3<A1_, A2_, A3_>(arg1_, arg2_, arg3_), m_arg4(arg4_) {}

	A4_ m_arg4;
};
template <class A1_, class A2_, class A3_, class A4_, class A5_>
struct Storage5
	: public Storage4<A1_, A2_, A3_, A4_>
{
	typedef A5_ Arg5;

	Storage5(A1_ arg1_, A2_ arg2_, A3_ arg3_, A4_ arg4_, A5_ arg5_)
		: Storage4<A1_, A2_, A3_, A4_>(arg1_, arg2_, arg3_, arg4_), m_arg5(arg5_) {}

	A5_ m_arg5;
};

//////////////////////
// Comparators
//////////////////////

// TEMPLATE CLASS
//	Plan::Utility::ReferingLess -- apply operator< to objects refered by pointers
//
// TEMPLATE ARGUMENTS
//	class ValueType_
//
// NOTES
template <class ValueType_>
class ReferingLess
{
public:
	// for non-const pointers
	bool operator() (ValueType_* pValue1_, ValueType_* pValue2_) const
	{
		return (!pValue1_ && pValue2_)
			|| (pValue1_ && pValue2_ && *pValue1_ < *pValue2_) ? true : false;
	}
	// for non-const references
	bool operator() (ValueType_& cValue1_, ValueType_& cValue2_) const
	{
		return (cValue1_ < cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (ValueType_* pValue1_, ValueType_& cValue2_) const
	{
		return !pValue1_
			|| (pValue1_ && *pValue1_ < cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (ValueType_& cValue1_, ValueType_* pValue2_) const
	{
		return (pValue2_ && cValue1_ < *pValue2_) ? true : false;
	}
	// for const pointers
	bool operator() (const ValueType_* pValue1_, const ValueType_* pValue2_) const
	{
		return (!pValue1_ && pValue2_)
			|| (pValue1_ && pValue2_ && *pValue1_ < *pValue2_) ? true : false;
	}
	// for const references
	bool operator() (const ValueType_& cValue1_, const ValueType_& cValue2_) const
	{
		return (cValue1_ < cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (const ValueType_* pValue1_, const ValueType_& cValue2_) const
	{
		return !pValue1_
			|| (pValue1_ && *pValue1_ < cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (const ValueType_& cValue1_, const ValueType_* pValue2_) const
	{
		return (pValue2_ && cValue1_ < *pValue2_) ? true : false;
	}
};

// TEMPLATE CLASS
//	Plan::Utility::IDLess -- apply operator< to objects' id obtained by getID method
//
// TEMPLATE ARGUMENTS
//	class ValueType_
//
// NOTES
template <class ValueType_>
class IDLess
{
public:
	// for non-const pointers
	bool operator() (ValueType_* pValue1_, ValueType_* pValue2_) const
	{
		return ((*pValue1_).getID() < (*pValue2_).getID()) ? true : false;
	}
	// for non-const references
	bool operator() (ValueType_& cValue1_, ValueType_& cValue2_) const
	{
		return (cValue1_.getID() < cValue2_.getID()) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (ValueType_* pValue1_, ValueType_& cValue2_) const
	{
		return ((*pValue1_).getID() < cValue2_.getID()) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (ValueType_& cValue1_, ValueType_* pValue2_) const
	{
		return (cValue1_.getID() < (*pValue2_).getID()) ? true : false;
	}
	// for const pointers
	bool operator() (const ValueType_* pValue1_, const ValueType_* pValue2_) const
	{
		return ((*pValue1_).getID() < (*pValue2_).getID()) ? true : false;
	}
	// for const references
	bool operator() (const ValueType_& cValue1_, const ValueType_& cValue2_) const
	{
		return (cValue1_.getID() < cValue2_.getID()) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (const ValueType_* pValue1_, const ValueType_& cValue2_) const
	{
		return ((*pValue1_).getID() < cValue2_.getID()) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (const ValueType_& cValue1_, const ValueType_* pValue2_) const
	{
		return (cValue1_.getID() < (*pValue2_).getID()) ? true : false;
	}
};

// TEMPLATE CLASS
//	Plan::Utility::SchemaLess -- apply lessThan to schema objects
//
// TEMPLATE ARGUMENTS
//	class ValueType_
//
// NOTES
template <class ValueType_>
class SchemaLess
{
public:
	// for non-const pointers
	bool operator() (ValueType_* pValue1_, ValueType_* pValue2_) const
	{
		return (*this)(*pValue1_, *pValue2_);
	}
	// for non-const references
	bool operator() (ValueType_& cValue1_, ValueType_& cValue2_) const
	{
		return cValue1_.isLessThan(cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (ValueType_* pValue1_, ValueType_& cValue2_) const
	{
		return (*this)(*pValue1_, cValue2_);
	}
	// for ObjectSet::find
	bool operator() (ValueType_& cValue1_, ValueType_* pValue2_) const
	{
		return (*this)(cValue1_, *pValue2_);
	}
	// for const pointers
	bool operator() (const ValueType_* pValue1_, const ValueType_* pValue2_) const
	{
		return (*this)(*pValue1_, *pValue2_);
	}
	// for const references
	bool operator() (const ValueType_& cValue1_, const ValueType_& cValue2_) const
	{
		return cValue1_.isLessThan(cValue2_) ? true : false;
	}
	// for ObjectSet::find
	bool operator() (const ValueType_* pValue1_, const ValueType_& cValue2_) const
	{
		return (*this)(*pValue1_, cValue2_);
	}
	// for ObjectSet::find
	bool operator() (const ValueType_& cValue1_, const ValueType_* pValue2_) const
	{
		return (*this)(cValue1_, *pValue2_);
	}
};

_SYDNEY_PLAN_END
_SYDNEY_PLAN_UTILITY_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_UTILITY_ALGORITHM_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
