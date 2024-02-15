// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/RowElement.h --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_ROWELEMENT_H
#define __SYDNEY_PLAN_RELATION_ROWELEMENT_H

#include "Plan/Relation/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Tree/Node.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Common
{
	class ColumnMetaData;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

///////////////////////////////////////////////////////////////////////
//	CLASS
//	Plan::Relation::RowElement -- Represents each element of relation row
//
//	NOTES
class RowElement
	: public Tree::Node
{
public:
	typedef RowElement This;
	typedef Tree::Node Super;
	typedef int Position;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pRelation_,
						Position iPosition_,
						Interface::IScalar* pScalar_ = 0);

	// destructor
	~RowElement() {}

	// operator
	bool operator<(const RowElement& cOther_) const;

	// get scalar name
	const STRING& getScalarName(Opt::Environment& cEnvironment_);
	// get scalar interface
	Interface::IScalar* getScalar(Opt::Environment& cEnvironment_);
	// get scalar interface without propagation
	Interface::IScalar* getScalar() {return m_pScalar;}

	// get relation which provides this element
	Interface::IRelation* getRelation() {return m_pRelation;}

	// get position in the relation
	Position getPosition() {return m_iPosition;}	

	// check relation
	bool isElementOf(Interface::IRelation* pRelation_) {return m_pRelation == pRelation_;}

	// set as retrieved
	void retrieve(Opt::Environment& cEnvironment_);

	// set column meta data
	void setMetaData(Opt::Environment& cEnvironment_,
					 Common::ColumnMetaData& cMetaData_);

//////////////////////////////////////////
// Node::Interface::
	virtual ModUnicodeString getValue() const;
	virtual ModSize getOptionSize() const;
	virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const;
	virtual ModSize getOperandSize() const;
	virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const;

protected:
private:
	// constructor
	RowElement(Super::Type eType_,
			   Interface::IRelation* pRelation_,
			   Position iPosition_,
			   Interface::IScalar* pScalar_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

	Interface::IRelation* m_pRelation;	// relation which provides this element
	Position m_iPosition;				// position in the relation
	Interface::IScalar* m_pScalar;		// scalar instance
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_ROWELEMENT_H

//
//	Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
