// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Cast.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Cast.h"
#include "Execution/Function/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataInstance.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CastImpl -- implementation classes of Cast
	//
	// NOTES
	class CastImpl
		: public Function::Cast
	{
	public:
		typedef CastImpl This;
		typedef Function::Cast Super;

		CastImpl()
			: Super(),
			  m_cSourceType(),
			  m_cType(),
			  m_cSourceData(),
			  m_cData(),
			  m_bForComparison(false),
			  m_bNoThrow(false)
		{}
		CastImpl(const Common::SQLData& cSourceType_,
				 const Common::SQLData& cType_,
				 int iSourceID_,
				 int iDataID_,
				 bool bForComparison_,
				 bool bNoThrow_)
			: Super(),
			  m_cSourceType(cSourceType_),
			  m_cType(cType_),
			  m_cSourceData(iSourceID_),
			  m_cData(iDataID_),
			  m_bForComparison(bForComparison_),
			  m_bNoThrow(bNoThrow_)
		{}

		~CastImpl() {}

	///////////////////////////
	// Function::Cast::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void accumulate(Interface::IProgram& cProgram_,
	//							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::Data* getSourceData() {return m_cSourceData.getData();}
		Common::Data* getData() {return m_cData.get();}

	private:
		Common::SQLData m_cSourceType;
		Common::SQLData m_cType;
		Action::DataHolder m_cSourceData;
		Action::DataHolder m_cData;
		bool m_bForComparison;
		bool m_bNoThrow;
	};
}

///////////////////////////////////////////////
// Execution::Function::Impl::CastImpl

// FUNCTION public
//	Function::Impl::CastImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::CastImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put("cast(");
	OSTRSTREAM stream;
	stream << m_cType;
	cExplain_.put(stream.getString());
	cExplain_.put(") ");
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cSourceData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::CastImpl::initialize -- 
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

//virtual
void
Impl::CastImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_cSourceData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CastImpl::terminate -- 
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

//virtual
void
Impl::CastImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		m_cData.terminate(cProgram_);
		m_cSourceData.terminate(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CastImpl::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Impl::CastImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		getData()->setSQLType(m_cType);

		if (m_cType.cast(m_cSourceType,
						 Common::Data::Pointer(getSourceData()),
						 Common::Data::Pointer(static_cast<const Common::Data*>(getData())),
						 m_bForComparison,
						 m_bNoThrow) == false) {
			// if cast is failed without throwing, set null
			getData()->setNull();
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CastImpl::finish -- 
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

//virtual
void
Impl::CastImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::Impl::CastImpl::reset -- 
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

//virtual
void
Impl::CastImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::Impl::CastImpl::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Impl::CastImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Cast);
}

// FUNCTION public
//	Function::Impl::CastImpl::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CastImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeSQLDataType(archiver_, m_cSourceType);
	Utility::SerializeSQLDataType(archiver_, m_cType);
	m_cSourceData.serialize(archiver_);
	m_cData.serialize(archiver_);
	archiver_(m_bForComparison);
	archiver_(m_bNoThrow);
}

/////////////////////////////////
// Function::Cast::

// FUNCTION public
//	Function::Cast::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Common::SQLData& cSourceType_
//	const Common::SQLData& cTargetType_
//	int iSourceID_
//	int iDataID_
//	bool bForComparison_ = false
//	bool bNoThrow_ = false
//	
// RETURN
//	Cast*
//
// EXCEPTIONS

//static
Cast*
Cast::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Common::SQLData& cSourceType_,
	   const Common::SQLData& cTargetType_,
	   int iSourceID_,
	   int iDataID_,
	   bool bForComparison_ /* = false */,
	   bool bNoThrow_ /* = false */)
{
	AUTOPOINTER<This> pResult = new Impl::CastImpl(cSourceType_,
												   cTargetType_,
												   iSourceID_,
												   iDataID_,
												   bForComparison_,
												   bNoThrow_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Cast::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Cast*
//
// EXCEPTIONS

//static
Cast*
Cast::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Cast);
	return new Impl::CastImpl;
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
