// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/SpatialImpl.h --
// 
// Copyright (c) 2013, 2014, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_SPATIALIMPL_H
#define __SYDNEY_PLAN_SCALAR_SPATIALIMPL_H

#include "Plan/Scalar/Spatial.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Exception/NotSupported.h"

#include "Exception/SpatialIndexNeeded.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace SpatialImpl
{
	///////////////////////////
	// STRUCT 
	//	GetIndexArgument --
	//
	// NOTES
	struct GetIndexArgument
	{
		// input values
		Interface::IRelation* m_pRelation;
		Interface::IScalar* m_pFunction;
		Schema::Field::Function::Value m_eFunction;

		// return values
		Utility::SchemaFileSet m_cIndexFileSet;
		Relation::Table* m_pTable;
		Utility::FileSet m_cFileSet;
		bool m_bResult;

		// information cache
		typedef MAP<Schema::File*, Schema::Field*,
					Utility::SchemaFileSet::Comparator> Map;
		VECTOR<Map> m_vecmapField;

		GetIndexArgument(Interface::IRelation* pRelation_,
						 Interface::IScalar* pFunction_,
						 Schema::Field::Function::Value eFunction_)
			: m_pRelation(pRelation_),
			  m_pFunction(pFunction_),
			  m_eFunction(eFunction_),
			  m_cIndexFileSet(),
			  m_pTable(0),
			  m_cFileSet(),
			  m_bResult(true),
			  m_vecmapField()
		{}
	};

	////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::SpatialImpl::Base -- base class for spatial operations
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public FunctionImpl::Base<Handle_>
	{
	public:
		typedef FunctionImpl::Base<Handle_> Super;
		typedef Base<Handle_> This;

		// constructor
		Base(Tree::Node::Type eType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cstrName_, cArgument_)
		{}
		Base(Tree::Node::Type eType_,
			 const DataType& cDataType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cDataType_, cstrName_, cArgument_)
		{}
		// destructor
		virtual ~Base() {}

	////////////////////////
	// Interface::IScalar::
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_)
		{
			if (eFunction_ == Schema::Field::Function::Undefined) {
				GetIndexArgument cArgument(pRelation_,
										   pFunction_,
										   getFunctionType());
				if (false ==
					isAll(boost::bind(&This::getIndexFile,
									  this,
									  boost::ref(cEnvironment_),
									  _1,
									  boost::ref(cArgument)))) {
					// can't find index to process this function
					return 0;
				}
				// create corresponding field for each operands
				foreachOperand_i(boost::bind(&This::createOperandField,
											 this,
											 boost::ref(cEnvironment_),
											 _1,
											 _2,
											 boost::ref(cArgument)));
				return Field::create(cEnvironment_,
									 cArgument.m_eFunction,
									 cArgument.m_cFileSet,
									 cArgument.m_pTable,
									 pFunction_,
									 getOperandi(0));
			}
			return 0;
		}

		using Super::getFunctionType;
		using Super::getOperandi;
		using Super::isAll;
		using Super::foreachOperand_i;



	protected:
	private:
	////////////////////////
	// Scalar::Function::
	};

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::SpatialImpl::Monadic -- function class for spatial operations
	//
	// NOTES
	typedef Base< Tree::Monadic<Spatial, Interface::IScalar> > Monadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::SpatialImpl::Dyadic -- function class for spatial operations
	//
	// NOTES
	typedef Base< Tree::Dyadic<Spatial, Interface::IScalar> > Dyadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::SpatialImpl::Nadic -- function class for spatial operations
	//
	// NOTES
	typedef Base< Tree::Nadic<Spatial, Interface::IScalar> > Nadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::SpatialImpl::MonadicWithOption -- function class for spatial operations
	//
	// NOTES
	typedef FunctionImpl::BaseWithOption<
					Tree::MonadicOption<Monadic, Interface::IScalar> > MonadicWithOption;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::SpatialImpl::NadicWithOption -- function class for spatial operations
	//
	// NOTES
	typedef FunctionImpl::BaseWithOption<
					Tree::MonadicOption<Nadic, Interface::IScalar> > NadicWithOption;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_SPATIALIMPL_H

//
//	Copyright (c) 2013, 2014, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
