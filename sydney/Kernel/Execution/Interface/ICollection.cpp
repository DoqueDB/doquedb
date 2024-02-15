// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/Collection.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Interface";
}

#include "SyDefault.h"

#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Explain.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_INTERFACE_USING

//////////////////////////////////////////
// Execution::Interface::ICollection

//
//	FUNCTION public
//	Interface::ICollection::Put::getLastPosition
//		-- get the last position of putting data
//
//	NOTES
//
//	ARGUMENTS
//	nothing
//
//	RETURN
//	int
//		last potision
//
//	EXCEPTIONS
//
int
ICollection::Put::getLastPosition()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Interface::ICollection::Get::getData
//		-- get data from collection (random access interfece)
//
//	NOTES
//
//	ARGUMENTS
// 	Interface::IProgram& cProgram_
//	Common::Data* pData_
//	int iPosition_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
ICollection::Get::getData(Interface::IProgram& cProgram_,
						  Common::Data* pData_,
						  int iPosition_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Interface::ICollection::Get::get
//		-- get data from collection (random access interfece)
//
//	NOTES
//
//	ARGUMENTS
// 	Interface::IProgram& cProgram_
//	Common::Externalizable* pObject_
//	int iPosition_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
ICollection::Get::get(Interface::IProgram& cProgram_,
					  Common::Externalizable* pObject_,
					  int iPosition_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::ICollection::isEmptyGrouping -- is empty grouping collection?
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

//virtual
bool
ICollection::
isEmptyGrouping()
{
	// default: empty grouping
	return true;
}

// FUNCTION protected
//	Interface::ICollection::registerToProgram -- register to program
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ICollection::
registerToProgram(Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerCollection method.
	setID(cProgram_.registerCollection(this));
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
