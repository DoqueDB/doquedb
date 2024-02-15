// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Fetch.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_TREE_FETCH_H
#define __SYDNEY_PLAN_TREE_FETCH_H

#include "Plan/Tree/Node.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

//	CLASS
//	Plan::Tree::Fetch -- special tree node for getSearchParameter with fetch
//
//	NOTES
class Fetch
	: public Node
{
public:
	typedef Node Super;
	typedef Fetch This;

	// constructor
	Fetch(Node* pNode_)
		: Super(Node::Fetch),
		  m_pNode(pNode_)
	{}

	// destructor
	~Fetch() {}

//////////////////////////////////////////
// Super::
//	Type getType() const;
//	virtual ModUnicodeString getValue() const;
//	virtual const Common::Data* getData() const;
//
	virtual ModSize getOptionSize() const
	{return 2;}
	virtual const Node::Super* getOptionAt(ModInt32 iPosition_) const
	{return this;}
	virtual ModSize getOperandSize() const
	{return m_pNode->getOperandSize() == 0 ? 1 : m_pNode->getOperandSize();}
	virtual const Node::Super* getOperandAt(ModInt32 iPosition_) const
	{return m_pNode->getOperandSize() == 0 ? m_pNode : m_pNode->getOperandAt(iPosition_);}

protected:
private:
	Node* m_pNode;
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_FETCH_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
