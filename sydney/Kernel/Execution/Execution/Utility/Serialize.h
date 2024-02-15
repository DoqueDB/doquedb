// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Serialize.h --
// 
// Copyright (c) 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_UTILITY_SERIALIZE_H
#define __SYDNEY_EXECUTION_UTILITY_SERIALIZE_H

#include "Execution/Utility/Module.h"
#include "Execution/Declaration.h"

#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

//////////////////////////////
// base functions
//////////////////////////////

// sqldata should serialize Collation separatedly
void SerializeSQLDataType(ModArchive& archiver_,
						  Common::SQLData& cTarget_);
void SerializeSQLDataType(ModArchive& archiver_,
						  VECTOR<Common::SQLData>& vecTarget_);

// array of data
void SerializeVariable(ModArchive& archiver_,
					   VECTOR<Common::Data::Pointer>& vecTarget_);

// space for connection
void SerializeConnection(ModArchive& archiver_,
						 VECTOR<Communication::Connection*>& vecTarget_);

// array and elements correspondence
void SerializeArrayVariable(ModArchive& archiver_,
							MAP<int, VECTOR<int>, LESS<int> >& mapTarget_);

// recover arraydata using elements correspondence
Common::Data::Pointer CreateArrayVariable(const VECTOR<int>& vecID_,
										  VECTOR<Common::Data::Pointer>& vecVariable_);
void AssignArrayVariable(MAP<int, VECTOR<int>, LESS<int> >& mapArray_,
						 VECTOR<Common::Data::Pointer>& vecVariable_);


//////////////////////////////
// template functions
//////////////////////////////

// TEMPLATE FUNCTION public
//	Execution::Utility::Serialize -- serializer
//
// TEMPLATE ARGUMENTS
//	class Object_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Object_*>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Object_>
inline
void
Serialize(ModArchive& archiver_,
		  VECTOR<Object_*>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
			for (int i = 0; i < n; ++i) {
				cOut.writeObject(vecTarget_[i]);
			}
		}
	} else {
		int n;
		archiver_ >> n;
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			vecTarget_.reserve(n);
			Common::InputArchive& cIn = dynamic_cast<Common::InputArchive&>(archiver_);
			for (int i = 0; i < n; ++i) {
				Object_* p = dynamic_cast<Object_*>(cIn.readObject());
				vecTarget_.PUSHBACK(p);
			}
		}
	}
}

// TEMPLATE FUNCTION public
//	Execution::Utility::SerializeObject -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Object_>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Object_>
inline
void
SerializeObject(ModArchive& archiver_,
				VECTOR<Object_>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			for (int i = 0; i < n; ++i) {
				vecTarget_[i].serialize(archiver_);
			}
		}
	} else {
		int n;
		archiver_ >> n;
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			vecTarget_.reserve(n);
			Object_ cObject;
			for (int i = 0; i < n; ++i) {
				cObject.serialize(archiver_);
				vecTarget_.PUSHBACK(cObject);
			}
		}
	}
}

// TEMPLATE FUNCTION public
//	Execution::Utility::SerializeValue -- 
//
// TEMPLATE ARGUMENTS
//	class Value_
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Value_>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Value_>
inline
void
SerializeValue(ModArchive& archiver_,
			   VECTOR<Value_>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			for (int i = 0; i < n; ++i) {
				archiver_ << vecTarget_[i];
			}
		}
	} else {
		int n;
		archiver_ >> n;
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			vecTarget_.reserve(n);
			Value_ cValue;
			for (int i = 0; i < n; ++i) {
				archiver_ >> cValue;
				vecTarget_.PUSHBACK(cValue);
			}
		}
	}
}

// TEMPLATE FUNCTION public
//	Execution::Utility::SerializeMap -- serializer for JobIDConnectionMap
//
// TEMPLATE ARGUMENTS
//	class Map_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	Map& mapTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Map_>
inline
void
SerializeMap(ModArchive& archiver_,
			 Map_& mapTarget_)
{
	if (archiver_.isStore()) {
		int n = mapTarget_.getSize();
		archiver_ << n;
		if (n) {
			typename Map_::Iterator iterator = mapTarget_.begin();
			const typename Map_::Iterator last = mapTarget_.end();
			for (; iterator != last; ++iterator) {
				archiver_((*iterator).first);
				archiver_((*iterator).second);
			}
		}
	} else {
		int n;
		archiver_ >> n;
		mapTarget_.clear();
		if (n) {
			for (int i = 0; i < n; ++i) {
				typename Map_::ValueType cValue;
				archiver_(cValue.first);
				archiver_(cValue.second);
				mapTarget_.insert(cValue);
			}
		}
	}
}

// TEMPLATE FUNCTION public
//	Utility::Serialize::SerializeEnum ---
//
// TEMPLATE ARGUMENTS
//	class Enum_
//	
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	Enum_& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Enum_>
inline
void
SerializeEnum(ModArchive& cArchiver_,
			  Enum_& cTarget_)
{
	int iValue;
	if (cArchiver_.isStore()) {
		iValue = static_cast<int>(cTarget_);
		cArchiver_ << iValue;
	} else {
		cArchiver_ >> iValue;
		cTarget_ = static_cast<Enum_>(iValue);
	}
}


_SYDNEY_EXECUTION_END
_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_UTILITY_SERIALIZE_H

//
//	Copyright (c) 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
