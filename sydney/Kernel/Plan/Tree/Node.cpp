// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Node.cpp --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Tree";
}

#include "SyDefault.h"

#include "Plan/Tree/Node.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_TREE_USING

////////////////////////////////////
//	Plan::Tree::Node

// FUNCTION protected
//	Tree::Node::Node -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::Type eType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Node::
Node(Super::Type eType_)
	: Object(), Super(eType_),
	  m_iID(-1)
{}

// FUNCTION protected
//	Tree::Node::Node -- copy constructor
//
// NOTES
//
// ARGUMENTS
//	const Node& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Node::
Node(const Node& cOther_)
	: Object(cOther_), Super(cOther_.getType()),
	  m_iID(-1)
{}

// FUNCTION protected
//	Tree::Node::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Node::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addNode(this);
}

// FUNCTION protected
//	Tree::Node::eraseFromEnvironment -- erase from environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Node::
eraseFromEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.eraseNode(m_iID);
}

//
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
