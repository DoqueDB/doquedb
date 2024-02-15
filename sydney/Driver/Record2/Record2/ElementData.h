// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- LogicalFile call it to get the instance of LogicalInterface
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_ELEMENTDATA_H
#define __SYDNEY_RECORD2_ELEMENTDATA_H

#include "Record2/VariableData.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	Record2::ElementData -- 
//		ElementData
//	NOTES
// If one field is string array, we use this class to access it.
//
class ElementData : public VariableData
{
public:
	
	//constructor
	ElementData();

	//destructor
	~ElementData();

    //get element value by index
    void getValueBySubscript();

    //set value by index
    void setValueBySubscript();

};

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_ELEMENTDATA_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
