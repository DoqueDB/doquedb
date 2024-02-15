// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/Declare.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Query/Declare.h"
#include "Analysis/Query/Utility.h"

#include "Common/Assert.h"


#include "Exception/OuterReferenceNotAllowed.h"
#include "Exception/PrepareFailed.h"

#include "Opt/Environment.h"

#include "Statement/DeclareStatement.h"


_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::DeclareImpl -- implementation class of Declare
	//
	// NOTES
	class DeclareImpl
		: public Query::Declare
	{
	public:
		typedef DeclareImpl This;
		typedef Query::Declare Super;

		// constructor
		DeclareImpl() {}
		// destructor
		virtual ~DeclareImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::DeclareImpl _analyzer;

} //namespace

////////////////////////////////
// Query::Impl::DeclareImpl
////////////////////////////////

// FUNCTION public
//	Query::Impl::DeclareImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::DeclareImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::PrepareFailed);
}

//////////////////////////////////
// Query::Declare
//////////////////////////////////

// FUNCTION public
//	Query::Declare::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::Declare* pStatement_
//	
// RETURN
//	const Declare*
//
// EXCEPTIONS

//static
const Declare*
Declare::
create(const Statement::DeclareStatement* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
