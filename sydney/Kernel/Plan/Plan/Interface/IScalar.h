// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IScalar.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_ISCALAR_H
#define __SYDNEY_PLAN_INTERFACE_ISCALAR_H

#include "Plan/Interface/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Scalar/DataType.h"
#include "Plan/Interface/ISqlNode.h"
#include "Plan/Tree/Node.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Data.h"

#include "Execution/Declaration.h"

#include "Opt/Algorithm.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN

namespace Common
{
	class ColumnMetaData;
}



_SYDNEY_PLAN_BEGIN
namespace Sql
{
	class Query;
}
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Interface::IScalar -- Base class for the classes which represents scalar data information
//
//	NOTES
//		This class is not constructed directly
class IScalar
	: public Tree::Node, public ISqlNode
{
public:
	typedef Tree::Node Super;
	typedef IScalar This;

	// result value of check method
	struct Check
	{
		typedef unsigned int Value;
		enum _Value
		{
			Constant	= 0,			// constant value
			Referred	= 1,			// refered in the target relation
			Preceding	= 1 << 1,		// already obtained by preceding relations
			NotYet		= 1 << 2,		// not obtained yet
			Calculated	= (Referred | Preceding)
		};
		// merge check values
		static void mergeValue(Value* pValue1_, Value eValue2_);

		// check bit
		static bool isOn(Value iValue_, Value iFlag_)
		{return (iValue_ & iFlag_) == iFlag_;}
	};

	// destructor
	virtual ~IScalar() {}

	// accessor
	const Scalar::DataType& getDataType() const;
	This* setNodeType(Super::Type eType_) {setType(eType_); return this;}

	// explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_) = 0;

	// get scalar name
	virtual const STRING& getName() = 0;

	// set expected data type
	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
												const Scalar::DataType& cType_) = 0;

	// create cast node
	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
										   const Scalar::DataType& cToType_,
										   bool bForComparison_,
										   Tree::Node::Type eType_ = Tree::Node::Undefined) = 0;

	// check underlying tables
	virtual Check::Value check(Opt::Environment& cEnvironment_,
							   const Scalar::CheckArgument& cArgument_) = 0;
	virtual bool isRefering(Interface::IRelation* pRelation_) = 0;

	// check used tables
	virtual void getUsedTable(Utility::RelationSet& cResult_) = 0;
	// check used scalars
	virtual void getUsedField(Utility::FieldSet& cResult_) = 0;
	// check unknown keys
	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
							   Predicate::CheckUnknownArgument& cResult_) = 0;

	// is known as null?
	virtual bool isKnownNull(Opt::Environment& cEnvironment_) = 0;
	// is known as not-null?
	virtual bool isKnownNotNull(Opt::Environment& cEnvironment_) = 0;
	// has unassigned parameters?
	virtual bool hasParameter() = 0;
	// is arbitrary element?
	virtual bool isArbitraryElement() = 0;

	// is field scalar?
	virtual bool isField() = 0;
	// get field interface
	virtual Scalar::Field* getField() = 0;
	// has field in the file?
	virtual bool hasField(Interface::IFile* pFile_) = 0;

	// is operation scalar?
	virtual bool isOperation() = 0;

	// is representing same scalar?
	virtual bool isEquivalent(Interface::IScalar* pScalar_) = 0;

	// create new node with option
	virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pOption_) = 0;

	// create virtual column for simple aggregation
	virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
												Interface::IRelation* pRelation_,
												Interface::IScalar* pFunction_,
												Schema::Field::Function::Value eFunction_);
															 
	
	// clear converted virtual column
	virtual void clearConvert(Opt::Environment& cEnvironment_);
	// get position in the relation
	virtual int getPosition(Interface::IRelation* pRelation_);

	// set refered scalars as required
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_) = 0;
	// set refered scalars as retrieved
	virtual void retrieve(Opt::Environment& cEnvironment_) = 0; // in analyzing
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Interface::ICandidate* pCandidate_) = 0; // in optimizing

	virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
									 Plan::Sql::Query* pQueue) = 0; // in optimizing
	
	// set refered scalars as used
	virtual void use(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pCandidate_) = 0;
	// set refered scalars as delayable
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Interface::ICandidate* pCandidate_,
					   Scalar::DelayArgument& cArgument_) = 0;

	// calculate scalar value if can
	virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_) = 0;

	// set column meta data
	virtual void setMetaData(Opt::Environment& cEnvironment_,
							 Common::ColumnMetaData& cMetaData_) = 0;

	// generate variable
	virtual int generate(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_);

	// generate variable from datatype
	virtual int generateFromType(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);

	virtual bool isSubquery() const = 0;

	virtual bool equalsOperand(const Plan::Interface::IScalar* arg) const = 0;	

	// can create equivalent operation
	virtual bool checkOperation(Opt::Environment& cEnvironment_,
								Interface::IScalar* pOperand_);
	// create equivalent operation if available
	virtual PAIR<This*, This*> createOperation(Opt::Environment& cEnvironment_,
											   Interface::IScalar* pOperand_);

protected:
	// costructor
	explicit IScalar(IScalar::Type eType_);
	IScalar(IScalar::Type eType_, const Scalar::DataType& cType_);

	// generate data
	virtual int generateData(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
	// generate main
	virtual int generateThis(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 int iDataID_);

	// get data if cached
	int getNodeVariable(Execution::Interface::IIterator* pIterator_,
						Candidate::AdoptArgument& cArgument_);
	// set data cache
	void setNodeVariable(Execution::Interface::IIterator* pIterator_,
						 Candidate::AdoptArgument& cArgument_,
						 int iDataID_);

	// accessor
	void setDataType(const Scalar::DataType& cType_);

private:
	Scalar::DataType m_cType;
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_ISCALAR_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
