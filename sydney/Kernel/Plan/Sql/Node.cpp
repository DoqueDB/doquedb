// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Projection.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Sql";
}

#include "SyDefault.h"

#include "Plan/Sql/Node.h"

#include "Plan/Sql/Impl/NodeImpl.h"
#include "Common/Assert.h"
#include "Exception/NotSupported.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN



/////////////////////////////////////
// Relation::SimpleNode

// FUNCTION public
//	Sql::Node::createSimpleNode -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	STRING cstrNode_
//	
// RETURN
//	Node*
//
// EXCEPTIONS

//static
Node*
Node::
createSimpleNode(Opt::Environment& cEnvironment_,
	   const STRING& cstrParam_)
{
	AUTOPOINTER<Node> pResult = new NodeImpl::SimpleStringNode(cstrParam_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Sql::Node::createSimpleNode -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iDataID_
//	
// RETURN
//	Node*
//
// EXCEPTIONS

//static
Node*
Node::
createSimpleNode(Opt::Environment& cEnvironment_,
				 int iDataID_)
{
	AUTOPOINTER<Node> pResult = new NodeImpl::SimpleDataNode(iDataID_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}



// FUNCTION public
//	Sql::Node::createArrayNode -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Node*
//
// EXCEPTIONS

//static
Node*
Node::
createArrayNode(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<Node> pResult = new NodeImpl::ArrayNode();
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}



// FUNCTION public
//	Sql::Node::createArrayPlaceHolderNode -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iDataID_
//	
// RETURN
//	Node*
//
// EXCEPTIONS

//static
Node*
Node::
createArrayPlaceHolderNode(Opt::Environment& cEnvironment_, int iDataID_)
{
	AUTOPOINTER<Node> pResult = new NodeImpl::ArrayPlaceHolderNode(iDataID_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Sql::Node::appendNode
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::ISqlNode* pNode_
//	
// RETURN
//	
//
// EXCEPTIONS
void
Node::
appendNode(Plan::Interface::ISqlNode* pNode_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Sql::Node::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	
//
// EXCEPTIONS
STRING
Node::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}
	

// FUNCTION private
//	Sql::Node::registerToEnvironment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	
//
// EXCEPTIONS
void
Node::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addObject(this);
}
	
	
_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

