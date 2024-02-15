// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/Serialize.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Utility";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/StringData.h"

#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

namespace
{
	// FUNCTION local
	//	$$$::_serializeStringData -- stringdata should serialize Collation and EncodingForm separatedly
	//
	// NOTES
	//
	// ARGUMENTS
	//	ModArchive& archiver_
	//	Common::Data::Pointer& cTarget_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	void
	_serializeStringData(ModArchive& archiver_,
						 Common::Data::Pointer& cTarget_)
	{
		Common::StringData* pStringData = _SYDNEY_DYNAMIC_CAST(Common::StringData*, cTarget_.get());
		; _SYDNEY_ASSERT(pStringData);

		if (archiver_.isStore()) {
			int iCollation = static_cast<int>(pStringData->getCollation());
			archiver_ << iCollation;
			int iEncodingForm = static_cast<int>(pStringData->getEncodingForm());
			archiver_ << iEncodingForm;
		} else {
			int iValue;
			archiver_ >> iValue;
			pStringData->setCollation(static_cast<Common::Collation::Type::Value>(iValue));
			archiver_ >> iValue;
			pStringData->setEncodingForm(static_cast<Common::StringData::EncodingForm::Value>(iValue));
		}
	}
}

// FUNCTION public
//	Execution::Utility::SerializeSQLDataType -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	Common::SQLData& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
SerializeSQLDataType(ModArchive& archiver_,
					 Common::SQLData& cTarget_)
{
	if (archiver_.isStore()) {
		cTarget_.serialize(archiver_);
		int iValue = static_cast<int>(cTarget_.getCollation());
		archiver_ << iValue;
	} else {
		cTarget_.serialize(archiver_);
		int iValue;
		archiver_ >> iValue;
		cTarget_.setCollation(static_cast<Common::Collation::Type::Value>(iValue));
	}
}

// FUNCTION public
//	Execution::Utility::SerializeSQLDataType -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Common::SQLData>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
SerializeSQLDataType(ModArchive& archiver_,
					 VECTOR<Common::SQLData>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			for (int i = 0; i < n; ++i) {
				SerializeSQLDataType(archiver_, vecTarget_[i]);
			}
		}
	} else {
		int n;
		archiver_ >> n;
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			vecTarget_.reserve(n);
			Common::SQLData cObject;
			for (int i = 0; i < n; ++i) {
				SerializeSQLDataType(archiver_, cObject);
				vecTarget_.PUSHBACK(cObject);
			}
		}
	}
}

// FUNCTION public
//	Execution::Utility::SerializeVariable -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Common::Data::Pointer>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
SerializeVariable(ModArchive& archiver_,
				  VECTOR<Common::Data::Pointer>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			Common::OutputArchive& cOut = dynamic_cast<Common::OutputArchive&>(archiver_);
			for (int i = 0; i < n; ++i) {
				cOut.writeObject(vecTarget_[i].get());
				if (!vecTarget_[i]->isNull() &&
					vecTarget_[i]->getType() == Common::DataType::String) {
					_serializeStringData(archiver_, vecTarget_[i]);
				}
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
				Common::Data::Pointer p = dynamic_cast<Common::Data*>(cIn.readObject());
				if (p.get() != 0 &&
					p->getType() == Common::DataType::String) {
					_serializeStringData(archiver_, p);
				}
				vecTarget_.PUSHBACK(p);
			}
		}
	}
}

// FUNCTION public
//	Execution::Utility::SerializeConnection -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	VECTOR<Communication::Connection*>& vecTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
SerializeConnection(ModArchive& archiver_,
					VECTOR<Communication::Connection*>& vecTarget_)
{
	if (archiver_.isStore()) {
		int n = vecTarget_.GETSIZE();
		archiver_ << n;
	} else {
		int n;
		archiver_ >> n;
		vecTarget_.erase(vecTarget_.begin(),
						 vecTarget_.end());
		if (n) {
			vecTarget_.assign(n, 0);
		}
	}
}

// FUNCTION public
//	Execution::Utility::SerializeArrayVariable -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	MAP<int, VECTOR<int>, LESS<int> >& mapTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
SerializeArrayVariable(ModArchive& archiver_,
					   MAP<int, VECTOR<int>, LESS<int> >& mapTarget_)
{
	if (archiver_.isStore()) {
		int n = mapTarget_.GETSIZE();
		archiver_ << n;
		if (n) {
			MAP<int, VECTOR<int>, LESS<int> >::ITERATOR iterator = mapTarget_.begin();
			const MAP<int, VECTOR<int>, LESS<int> >::ITERATOR last = mapTarget_.end();
			for (; iterator != last; ++iterator) {
				archiver_ << (*iterator).first;
				SerializeValue(archiver_, (*iterator).second);
			}
		}
	} else {
		int n;
		archiver_ >> n;
		mapTarget_.erase(mapTarget_.begin(),
						 mapTarget_.end());
		if (n) {
			mapTarget_.reserve(n);
			for (int i = 0; i < n; ++i) {
				int iID;
				VECTOR<int> vecID;
				archiver_ >> iID;
				SerializeValue(archiver_, vecID);
				mapTarget_.insert(iID, vecID);
			}
		}
	}
}

// FUNCTION public
//	Execution::Utility::CreateArrayVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecID_
//	VECTOR<Common::Data::Pointer>& vecVariable_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS
Common::Data::Pointer
CreateArrayVariable(const VECTOR<int>& vecID_,
					VECTOR<Common::Data::Pointer>& vecVariable_)
{
	AUTOPOINTER<Common::DataArrayData> pArrayData = new Common::DataArrayData;
	int n = vecID_.GETSIZE();
	if (n > 0) {
		pArrayData->reserve(n);
		int i = 0;
		do {
			pArrayData->pushBack(vecVariable_[vecID_[i]]);
		} while (++i < n);
	}
	return pArrayData.release();
}

// FUNCTION public
//	Execution::Utility::AssignArrayVariable -- 
//
// NOTES
//
// ARGUMENTS
//	MAP<int, VECTOR<int>, LESS<int> >& mapArray_
//	VECTOR<Common::Data::Pointer>& vecVariable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
AssignArrayVariable(MAP<int, VECTOR<int>, LESS<int> >& mapArray_,
					VECTOR<Common::Data::Pointer>& vecVariable_)
{
	int n = mapArray_.GETSIZE();
	if (n) {
		MAP<int, VECTOR<int>, LESS<int> >::ITERATOR iterator = mapArray_.begin();
		const MAP<int, VECTOR<int>, LESS<int> >::ITERATOR last = mapArray_.end();
		for (; iterator != last; ++iterator) {
			vecVariable_[(*iterator).first] = CreateArrayVariable((*iterator).second,
																  vecVariable_);
		}
	}
}

_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
