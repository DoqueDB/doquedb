// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FullTextImpl.h --
// 
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FULLTEXTIMPL_H
#define __SYDNEY_PLAN_SCALAR_FULLTEXTIMPL_H

#include "Plan/Scalar/FullText.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"


#include "Common/WordData.h"

#include "Exception/NotSupported.h"

#include "Exception/FullTextIndexNeeded.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace FullTextImpl
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
	//	Plan::Scalar::FullTextImpl::Base -- base class for fullText operations
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
			: Super(eType_, cstrName_, cArgument_),
			  m_pData()
		{}
		Base(Tree::Node::Type eType_,
			 const DataType& cDataType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
			: Super(eType_, cDataType_, cstrName_, cArgument_),
			  m_pData()
		{}
		// destructor
		virtual ~Base() {}



	////////////////////////
	// Node::
		virtual const Common::Data* getData() const
		{
			//	Word ------ Operand -- ConstantValue or Variable
			//		|
			//		+------ Option --- Category
			//					|
			//					+----- Scale
			//					|
			//					+----- Language
	
			if (getType() != Tree::Node::Word) _SYDNEY_THROW0(Exception::NotSupported);
			if (m_pData.get() != 0) return m_pData.get();
			

			AUTOPOINTER<Common::WordData> pWordData =
				new Common::WordData(getOperandAt(0)->getOperandAt(0)->getValue());
	
			for (int i = 0; i < static_cast<int>(getOptionSize()); ++i)
			{
				const LogicalFile::TreeNodeInterface* p = getOptionAt(i);
				switch (p->getType())
				{
				case LogicalFile::TreeNodeInterface::Category:
				{
					Common::WordData::Category::Value c
						= Common::WordData::toCategory(p->getValue());
					if (c == Common::WordData::Category::Undefined)
						return false;
					else if (c == Common::WordData::Category::Essential)
						_SYDNEY_THROW0(Exception::NotSupported);

					pWordData->setCategory(c);
					break;
				}
				
				case LogicalFile::TreeNodeInterface::Scale:
				{
					;_SYDNEY_ASSERT(p->getData()->getType() == Common::DataType::Double);
					const Common::DoubleData* pScale = 
						_SYDNEY_DYNAMIC_CAST(const Common::DoubleData*, p->getData());
					pWordData->setScale(pScale->getValue());
					break;
				}
				case LogicalFile::TreeNodeInterface::Language:
				case LogicalFile::TreeNodeInterface::Df:
					_SYDNEY_THROW0(Exception::NotSupported);
				}
			}
	
			m_pData = pWordData.release();
			return m_pData.get();
		}

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

				Interface::IScalar* pOperand;
				if (pFunction_->getType() == Tree::Node::Score
					&& getOperandSize() > 1) {
					pOperand = Scalar::Function::create(cEnvironment_, Tree::Node::List, getArgument(), "SchemaColumnList");
				} else {
					pOperand = getOperandi(0);
				}
				
				return Field::create(cEnvironment_,
									 cArgument.m_eFunction,
									 cArgument.m_cFileSet,
									 cArgument.m_pTable,
									 pFunction_,
									 pOperand);
			}
			return 0;
		}

	protected:
	private:
		mutable Common::ObjectPointer<Common::Data> m_pData;
	////////////////////////
	// Scalar::Function::
	};

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FullTextImpl::Monadic -- function class for fullText operations
	//
	// NOTES
	typedef Base< Tree::Monadic<FullText, Interface::IScalar> > Monadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FullTextImpl::Dyadic -- function class for fullText operations
	//
	// NOTES
	typedef Base< Tree::Dyadic<FullText, Interface::IScalar> > Dyadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FullTextImpl::Nadic -- function class for fullText operations
	//
	// NOTES
	typedef Base< Tree::Nadic<FullText, Interface::IScalar> > Nadic;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FullTextImpl::MonadicWithOption -- function class for fullText operations
	//
	// NOTES
	typedef FunctionImpl::BaseWithOption<
					Tree::MonadicOption<Monadic, Interface::IScalar> > MonadicWithOption;

	////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FullTextImpl::MonadicWithNadicOption -- function class for fullText operations
	//
	// NOTES
	typedef FunctionImpl::BaseWithOption<
					Tree::NadicOption<Monadic, Interface::IScalar> > MonadicWithNadicOption;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FULLTEXTIMPL_H

//
//	Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
