// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/BulkImpl.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_BULKIMPL_H
#define __SYDNEY_PLAN_RELATION_BULKIMPL_H

#include "boost/bind.hpp"

#include "Plan/Relation/Bulk.h"
#include "Plan/Relation/Filter.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace BulkImpl
{
	// CLASS
	//	Relation::BulkImpl::Base -- base class of implementation classes for Bulk
	//
	// NOTES
	class Base
		: public Relation::Bulk
	{
	public:
		typedef Relation::Bulk Super;
		typedef Base This;

		// destructor
		virtual ~Base() {}

	///////////////////////
	// Relation::Bulk::
		virtual const Argument& getBulkOption() {return m_cArgument;}

	protected:
		// constructor
		Base()
			: Super(),
			  m_cArgument(0,0,0)
		{}
		Base(const Bulk::Argument& cArgument_)
			: Super(),
			  m_cArgument(cArgument_)
		{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		void setArgument(const Bulk::Argument& cArgument_)
		{
			m_cArgument = cArgument_;
		}
#endif

	private:
		Bulk::Argument m_cArgument;
	};

	// CLASS
	//	Relation::BulkImpl::Input -- implementation class for bulk input
	//
	// NOTES
	class Input
		: public Base
	{
	public:
		typedef Base Super;
		typedef Input This;

		// constructor
		Input(const Super::Argument& cArgument_)
			: Super(cArgument_)
		{}
		// destructor
		~Input() {}

	//////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Interface::IScalar*>& vecScalar_,
								  Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Tree::Node::Type>& vecType_,
									  Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Interface::IScalar* pScalar_,
								   Interface::IScalar* pOperand_);
	};

	// CLASS
	//	Relation::BulkImpl::Output -- implementation class for bulk output
	class Output
		: public Relation::Filter<Base>
	{
	public:
		typedef Relation::Filter<Base> Super;
		typedef Output This;

		// constructor
		Output(const Bulk::Argument& cArgument_,
			   Interface::IRelation* pOperand_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOperand_)
		{
			setArgument(cArgument_);
		}
#else
			: Super(cArgument_, pOperand_)
		{}
#endif
		// destructor
		~Output() {}

	//////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
	protected:
	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Interface::IScalar*>& vecScalar_,
								  Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Tree::Node::Type>& vecType_,
									  Position iPosition_);
	//	virtual void setRetrieved(Opt::Environment& cEnvironment_,
	//							  Position iPosition_);
	//	virtual int addAggregation(Opt::Environment& cEnvironment_,
	//							   Interface::IScalar* pScalar_,
	//							   Interface::IScalar* pOperand_);
	};

} // namespace Impl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_BULKIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
