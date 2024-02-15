// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/RowElement.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/RowElement.h"

#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

namespace
{
	// empty name
	const STRING _cstrEmptyString;
}

//////////////////////////////////
//	Plan::Relation::RowElement

// FUNCTION public
//	Relation::RowElement::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Position iPosition_
//	Interface::IScalar* pScalar_ = 0
//	
// RETURN
//	RowElement*
//
// EXCEPTIONS

//static
RowElement*
RowElement::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pRelation_,
	   Position iPosition_,
	   Interface::IScalar* pScalar_/*  = 0 */)
{
	; _SYDNEY_ASSERT(pRelation_ || pScalar_);
	Super::Type eType = (pScalar_ ? pScalar_->getType()
						 : pRelation_->getNodeType(cEnvironment_, iPosition_));
	AUTOPOINTER<This> pResult = new This(eType, pRelation_, iPosition_, pScalar_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::RowElement::operator< -- operator
//
// NOTES
//
// ARGUMENTS
//	const RowElement& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
RowElement::
operator<(const RowElement& cOther_) const
{
	return ((m_pRelation != 0 && cOther_.m_pRelation != 0
			 && (*m_pRelation < *cOther_.m_pRelation
				 || (*m_pRelation == *cOther_.m_pRelation
					 && m_iPosition < cOther_.m_iPosition)))
			||
			(m_pRelation == 0
			 && (cOther_.m_pRelation != 0
				 || (cOther_.m_pRelation == 0
					 && *m_pScalar < *cOther_.m_pScalar))));
}

// FUNCTION public
//	Relation::RowElement::getScalarName -- get scalar name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	const STRING&
//
// EXCEPTIONS

const STRING&
RowElement::
getScalarName(Opt::Environment& cEnvironment_)
{
	return m_pRelation
		? m_pRelation->getScalarName(cEnvironment_, m_iPosition)
		: m_pScalar->getName();
}

// FUNCTION public
//	Relation::RowElement::getScalar -- get scalar interface
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
RowElement::
getScalar(Opt::Environment& cEnvironment_)
{
	if (!m_pScalar && m_pRelation) {
		m_pScalar = m_pRelation->getScalar(cEnvironment_, m_iPosition);
	}
	return m_pScalar;
}

// FUNCTION public
//	Relation::RowElement::retrieve -- set as retrieved
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
RowElement::
retrieve(Opt::Environment& cEnvironment_)
{
	if (m_pRelation) {
		m_pRelation->retrieve(cEnvironment_, m_iPosition);
	} else if (m_pScalar) {
		m_pScalar->retrieve(cEnvironment_);
	}
}

// FUNCTION public
//	Relation::RowElement::setMetaData -- set column meta data
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RowElement::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setColumnName(getScalarName(cEnvironment_));
	Interface::IScalar* pScalar = getScalar(cEnvironment_);
	cMetaData_.setSQLData(pScalar->getDataType());
	pScalar->setMetaData(cEnvironment_, cMetaData_);
}

// FUNCTION public
//	Relation::RowElement::getValue -- I/F of tree node interface
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
RowElement::
getValue() const
{
	return m_pScalar ? m_pScalar->getValue()
		: ModUnicodeString();
}

// FUNCTION public
//	Relation::RowElement::getOptionSize -- I/F of tree node interface
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
RowElement::
getOptionSize() const
{
	return m_pScalar ? m_pScalar->getOptionSize()
		: 0;
}

// FUNCTION public
//	Relation::RowElement::getOptionAt -- I/F of tree node interface
//
// NOTES
//
// ARGUMENTS
//	ModInt32 iPosition_
//	
// RETURN
//	const LogicalFile::TreeNodeInterface*
//
// EXCEPTIONS

//virtual
const LogicalFile::TreeNodeInterface*
RowElement::
getOptionAt(ModInt32 iPosition_) const
{
	return m_pScalar ? m_pScalar->getOptionAt(iPosition_)
		: 0;
}

// FUNCTION public
//	Relation::RowElement::getOperandSize -- I/F of tree node interface
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
RowElement::
getOperandSize() const
{
	return m_pScalar ? m_pScalar->getOperandSize()
		: 0;
}

// FUNCTION public
//	Relation::RowElement::getOperandAt -- I/F of tree node interface
//
// NOTES
//
// ARGUMENTS
//	ModInt32 iPosition_
//	
// RETURN
//	const LogicalFile::TreeNodeInterface*
//
// EXCEPTIONS

//virtual
const LogicalFile::TreeNodeInterface*
RowElement::
getOperandAt(ModInt32 iPosition_) const
{
	return m_pScalar ? m_pScalar->getOperandAt(iPosition_)
		: 0;
}

// FUNCTION private
//	Relation::RowElement::RowElement -- constructor
//
// NOTES
//
// ARGUMENTS
//	Super::Type eType_
//	Interface::IRelation* pRelation_
//	Position iPosition_
//	Interface::IScalar* pScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

RowElement::
RowElement(Super::Type eType_,
		   Interface::IRelation* pRelation_,
		   Position iPosition_,
		   Interface::IScalar* pScalar_)
	: Super(eType_),
	  m_pRelation(pRelation_),
	  m_iPosition(iPosition_),
	  m_pScalar(pScalar_)
{}

// FUNCTION private
//	Relation::RowElement::registerToEnvironment -- register to environment
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
RowElement::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addObject(this);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
