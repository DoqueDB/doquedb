// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/ICollection.h --
// 
// Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_ICOLLECTION_H
#define __SYDNEY_EXECUTION_INTERFACE_ICOLLECTION_H

#include "Execution/Interface/IObject.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
	class Externalizable;
}
namespace Communication
{
	class Connection;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Interface::Collection -- Base class for the classes which represents collection
//
//	NOTES
//		This class is not constructed directly
class ICollection
	: public IObject
{
public:
	typedef IObject Super;
	typedef ICollection This;

	// CLASS
	//	ICollection::Put -- put interface
	//
	// NOTES
	class Put
	{
	public:
		// destructor
		virtual ~Put() {}

		// do after putting data
		virtual void finish(Interface::IProgram& cProgram_) = 0;
		// terminate
		virtual void terminate(Interface::IProgram& cProgram_) = 0;
		// put data to collection
		virtual bool putData(Interface::IProgram& cProgram_,
							 const Common::Data* pData_) = 0;
		virtual bool put(Interface::IProgram& cProgram_,
						 const Common::Externalizable* pObject_) = 0;
		// get the last position
		virtual int getLastPosition();
		// shift collection if necessary
		virtual void shift(Interface::IProgram& cProgram_) {/* default: do nothing */}
		// flush output
		virtual void flush() {/* default: do nothing */}
	protected:
		Put() {}
	private:
	};

	// CLASS
	//	ICollection::Get -- get interface
	//
	// NOTES
	class Get
	{
	public:
		// destructor
		virtual ~Get() {}

		// do after getting data
		virtual void finish(Interface::IProgram& cProgram_) = 0;
		// terminate
		virtual void terminate(Interface::IProgram& cProgram_) = 0;
		// get data from collection
		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::Data* pData_) = 0;
		virtual bool get(Interface::IProgram& cProgram_,
						 Common::Externalizable* pObject_) = 0;
		// get data from collection (random access interface)
		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::Data* pData_, int iPosition_);
		virtual bool get(Interface::IProgram& cProgram_,
						 Common::Externalizable* pObject_, int iPosition_);
		// reset iteration
		virtual void reset() = 0;
	protected:
		Get() {}
	private:
	};

	// destructor
	virtual ~ICollection() {}

	// output explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_) = 0;

	// initialize
	virtual void initialize(Interface::IProgram& cProgram_) = 0;
	virtual void terminate(Interface::IProgram& cProgram_) = 0;

	// clear contents
	virtual void clear() = 0;
	// accessor
	virtual bool isEmpty() = 0;
	// is empty grouping collection?
	virtual bool isEmptyGrouping();
	// is
	virtual bool isGetNextOperand() {return true;}
	// get put interface
	virtual Put* getPutInterface() = 0;
	// get put interface
	virtual Get* getGetInterface() = 0;

protected:
	// constructor
	ICollection() : Super() {}
	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_ICOLLECTION_H

//
//	Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
