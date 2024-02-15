// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Spatial.h --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_SPATIAL_H
#define __SYDNEY_PLAN_SCALAR_SPATIAL_H

#include "Plan/Scalar/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace SpatialImpl
{
	// name declaration for local struct
	struct GetIndexArgument;
}

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Spatial -- Interface for spatial
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Spatial
	: public Function
{
public:
	typedef Function Super;
	typedef Spatial This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						Interface::IScalar* pOption_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						Interface::IScalar* pOption_,
						const STRING& cstrName_);

	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						Interface::IScalar* pOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						Interface::IScalar* pOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);

	// destructor
	virtual ~Spatial() {}

/////////////////////////
// Interface::IScalar::

protected:
	// constructor
	Spatial()
		: Super()
	{}
	Spatial(Tree::Node::Type eOperator_,
			 const STRING& cstrName_)
		: Super(eOperator_, cstrName_)
	{}
	Spatial(Tree::Node::Type eOperator_,
			 const DataType& cDataType_,
			 const STRING& cstrName_)
		: Super(eOperator_, cDataType_, cstrName_)
	{}

	// get function type
	Schema::Field::Function::Value getFunctionType();

	// seach for index file for one operand
	bool getIndexFile(Opt::Environment& cEnvironment_,
					  Interface::IScalar* pOperand_,
					  SpatialImpl::GetIndexArgument& cArgument_);

	// create corresponding field
	void createOperandField(Opt::Environment& cEnvironment_,
							Interface::IScalar* pOperand_,
							int iIndex_,
							SpatialImpl::GetIndexArgument& cArgument_);
private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_SPATIAL_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
