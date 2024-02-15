// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IObject.h --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IOBJECT_H
#define __SYDNEY_EXECUTION_INTERFACE_IOBJECT_H

#include "Execution/Interface/Module.h"
#include "Execution/Declaration.h"

#include "Common/Externalizable.h"

#include "Opt/Declaration.h"

#include "ModArchive.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Interface::Object -- Base class for all the classes in program
//
//	NOTES
//		This class is not constructed directly
class IObject
	: public Common::Externalizable
{
public:
	typedef Common::Externalizable Super;
	typedef IObject This;

	// destructor
	virtual ~IObject() {}

	int getID() {return m_iID;}

protected:
	// constructor
	IObject() : Super(), m_iID(-1) {}
	// set ID
	void setID(int iID_)
	{
		m_iID = iID_;
	}

	// serialize object
	void serializeID(ModArchive& archiver_)
	{
		archiver_(m_iID);
	}
private:
	int m_iID;
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IOBJECT_H

//
//	Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
