// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IRelation.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_IRELATION_H
#define __SYDNEY_PLAN_INTERFACE_IRELATION_H

#include "Plan/Interface/Module.h"
#include "Plan/Tree/Node.h"
#include "Plan/Utility/ObjectSet.h"

#include "Plan/Declaration.h"
#include "Plan/Sql/Query.h"

#include "Execution/Declaration.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"



_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Interface::IRelation -- Base class for the classes which represents relation data information
//
//	NOTES
//		This class is not constructed directly
class IRelation
	: public Tree::Node
{
public:
	typedef Tree::Node Super;
	typedef IRelation This;

	typedef int Position;
	typedef Position Size;
	typedef unsigned int InquiryResult;

	// destructor
	static void erase(Opt::Environment& cEnvironment_,
					  This* pThis_);

	// result row spec
	Relation::RowInfo* getRowInfo(Opt::Environment& cEnvironment_);
	// key row spec
	Relation::RowInfo* getKeyInfo(Opt::Environment& cEnvironment_);

	// set result row spec
	void setRowInfo(Opt::Environment& cEnvironment_,
					Relation::RowInfo* pRowInfo_);

	// degree of the relation
	Size getDegree(Opt::Environment& cEnvironment_);
	// max of position
	Position getMaxPosition(Opt::Environment& cEnvironment_);

	// cardinality of the relation if available
	virtual Size getCardinality(Opt::Environment& cEnvironment_);
	// get i-th row if available
	virtual bool getRow(Opt::Environment& cEnvironment_,
						int iPosition_,
						VECTOR<Interface::IScalar*>& vecResult_);

	// get relation name
	virtual const STRING& getCorrelationName(Opt::Environment& cEnvironment_);

	// set derived table and column name
	virtual void setCorrelationName(Opt::Environment& cEnvironment_,
									const STRING& cstrTableName_,
									const VECTOR<STRING>& vecColumnName_);

	// get scalar name by position
	const STRING& getScalarName(Opt::Environment& cEnvironment_,
								Position iPosition_);
	// get scalar interface by position
	Interface::IScalar* getScalar(Opt::Environment& cEnvironment_,
								  Position iPosition_);
	// get node type of each scalar by position
	IRelation::Type getNodeType(Opt::Environment& cEnvironment_,
								Position iPosition_);

	// set scalar as retrieved
	void retrieve(Opt::Environment& cEnvironment_,
				  Position iPosition_);
	// set scalar as aggregation
	int aggregate(Opt::Environment& cEnvironment_,
				  Interface::IScalar* pScalar_,
				  Interface::IScalar* pOperand_);

	// estimate relation count in unwind result
	virtual int estimateUnwind(Opt::Environment& cEnvironment_);
	// estimate relation count in unnion rewrite
	virtual int estimateUnion(Opt::Environment& cEnvironment_);

	// unwind subquery
	virtual PAIR<Interface::IRelation*, Interface::IPredicate*>
					unwind(Opt::Environment& cEnvironment_);

	// rewrite by predicate
	virtual PAIR<Interface::IRelation*, Interface::IPredicate*>
					rewrite(Opt::Environment& cEnvironment_,
							Interface::IPredicate* pPredicate_,
							Predicate::RewriteArgument& cArgument_);

	// create access plan candidate
	virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
													AccessPlan::Source& cPlanSource_) = 0;

	// inquiry about relation's attributes
	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
								  const Relation::InquiryArgument& cArgument_) = 0;

	// require scalar used in relation (for subquery)
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_ = 0) = 0;

	// get used tables
	virtual void getUsedTable(Opt::Environment& cEnvironment_,
							  Utility::RelationSet& cResult_) = 0;

	virtual bool isGrouping() { return m_bGrouping; }

	virtual void setGrouping() { m_bGrouping = true; }

	virtual bool isOutputToVariable() { return m_bOutputVariable; }

	virtual const STRING& getOutputVariableName() { return m_cstrOutputVariableName; }

	virtual void addOutputVariableName(Opt::Environment& cEnvironment_,
									   const STRING& cstrName)
	{
		m_bOutputVariable = true;
		m_cstrOutputVariableName = cstrName;
	}
	
	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
	

	

protected:
	// costructor
	explicit IRelation(IRelation::Type eType_);
	// destructor
	virtual ~IRelation() {}

private:
	// set result row spec
	virtual Relation::RowInfo* createRowInfo(Opt::Environment& cEnvironment_) = 0;
	// set key spec
	virtual Relation::RowInfo* createKeyInfo(Opt::Environment& cEnvironment_) = 0;
	// set degree of the relation
	virtual int setDegree(Opt::Environment& cEnvironment_) = 0;
	// set max position of the relation
	virtual int setMaxPosition(Opt::Environment& cEnvironment_) = 0;
	// set scalar names
	virtual void createScalarName(Opt::Environment& cEnvironment_,
								  VECTOR<STRING>& vecName_,
								  Position iPosition_) = 0;
	// set scalar interface
	virtual void createScalar(Opt::Environment& cEnvironment_,
							  VECTOR<Interface::IScalar*>& vecScalar_,
							  Position iPosition_) = 0;
	// set scalar node type
	virtual void createScalarType(Opt::Environment& cEnvironment_,
								  VECTOR<Node::Type>& vecType_,
								  Position iPosition_) = 0;
	// set retrieved flag
	virtual void setRetrieved(Opt::Environment& cEnvironment_,
							  Position iPosition_) = 0;
	// add aggregation
	virtual int addAggregation(Opt::Environment& cEnvironment_,
							   Interface::IScalar* pScalar_,
							   Interface::IScalar* pOperand_) = 0;


									 

	Relation::RowInfo* m_pRowInfo;
	Relation::RowInfo* m_pKeyInfo;
	Size m_iDegree;
	Position m_iMaxPosition;
	STRING m_cstrCorrelationName;
	STRING m_cstrOutputVariableName;
	bool m_bOutputVariable;
	VECTOR<STRING> m_vecName;
	VECTOR<Interface::IScalar*> m_vecScalar;
	VECTOR<Node::Type> m_vecType;
	bool m_bGrouping;
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_IRELATION_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
