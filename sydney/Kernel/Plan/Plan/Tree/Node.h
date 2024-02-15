// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Node.h --
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

#ifndef __SYDNEY_PLAN_TREE_NODE_H
#define __SYDNEY_PLAN_TREE_NODE_H

#include "Plan/Tree/Module.h"
#include "Plan/Tree/Declaration.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

//	CLASS
//	Plan::Tree::Node -- base class for plan tree node
//
//	NOTES
//	This class can not be constructed directly
class Node
	: public LogicalFile::TreeNodeInterface
{
public:
	typedef LogicalFile::TreeNodeInterface Super;
	typedef Node This;

	// destructor
	virtual ~Node() {}

	// operator
	bool operator==(const This& cOther_) const
	{return m_iID == cOther_.m_iID;}
	bool operator<(const This& cOther_) const
	{return m_iID < cOther_.m_iID;}

	// accessor
	virtual int getID() const
	{return m_iID;}

//////////////////////////////////////////
// Super::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//	virtual const Common::Data* getData() const;
//
//	virtual ModSize getOptionSize() const;
//	virtual const Super* getOptionAt(ModInt32 iPosition_) const;
//
//	virtual ModSize getOperandSize() const;
//	virtual const Super* getOperandAt(ModInt32 iPosition_) const;

protected:
	// constructor
	Node(Super::Type eType_ = Super::Undefined);
	Node(const Node& cOther_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
	// erase from environment
	void eraseFromEnvironment(Opt::Environment& cEnvironment_);

private:
	// don't copy
	This& operator=(const This& cOther_);

	int m_iID;
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_NODE_H

//
//	Copyright (c) 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
