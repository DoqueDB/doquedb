// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V2Impl/ProgramImpl.h -- プログラム(v2)
// 
// Copyright (c) 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_V2IMPL_PROGRAMIMPL_H
#define __SYDNEY_EXECUTION_V2IMPL_PROGRAMIMPL_H

#include "Execution/V2Impl/Module.h"
#include "Execution/Interface/IProgramBridge.h"
#include "Execution/Interface/IV2Program.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V2IMPL_BEGIN

// CLASS
//	Execution::V2Impl::ProgramImpl -- Program implementation for v2
//
// NOTES
class ProgramImpl
	: public Interface::IProgramBridge
{
public:
	typedef ProgramImpl This;
	typedef Interface::IProgramBridge Super;

	// CLASS
	//	Execution::V2Impl::ProgramImpl::V2Interface -- v2 interface implementation
	//
	// NOTES
	class V2Interface
		: public Interface::IV2Program
	{
	public:
		typedef V2Interface This;
		typedef Interface::IV2Program Super;

		// constructor
		V2Interface()
			: Super(),
			  m_pProgram(0)
		{}
		// destructor
		~V2Interface();

	////////////////////////////
	// Interface::IV2Program
		Interface::IProgram* getProgram();
		void setProgram(Interface::IProgram* pProgram_);
		Interface::IProgram* releaseProgram();
	protected:
	private:
		Interface::IProgram* m_pProgram;
	};

	// constructor
	ProgramImpl()
		: Super(),
		  m_cInterface()
	{}

	// destructor
	~ProgramImpl()
	{}

///////////////////////////////////
// Interface::IProgramBridge
	virtual Version::Value getVersion()
	{
		return Version::V2;
	}
	virtual Interface::IV1Program* getV1Interface();
	virtual Interface::IV2Program* getV2Interface()
	{
		return &m_cInterface;
	}

protected:
private:
	V2Interface m_cInterface;
};

_SYDNEY_EXECUTION_V2IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_V2IMPL_PROGRAMIMPL_H

//
//	Copyright (c) 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
