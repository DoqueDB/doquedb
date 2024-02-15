// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/FullText.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/FullText.h"
#include "Plan/Scalar/Impl/FullTextImpl.h"
#include "Plan/Scalar/Impl/FieldImpl.h"

#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Schema/Column.h"
#include "Schema/Field.h"
#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace
{
	// FUNCTION local
	//	$$$::_addField -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Trans::Transaction& cTrans_
	//	FullTextImpl::GetIndexArgument& cArgument_
	//	Utility::SchemaFileSet& cFileSet_
	//	Schema::Field* pSchemaField_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_addField(Trans::Transaction& cTrans_,
			  FullTextImpl::GetIndexArgument::Map& cMap_,
			  Utility::SchemaFileSet& cFileSet_,
			  Schema::Field* pSchemaField_)
	{
		Schema::File* pSchemaFile = pSchemaField_->getFile(cTrans_);
		cFileSet_.add(pSchemaFile);
		cMap_.insert(pSchemaFile, pSchemaField_);
	}
} // namespace

////////////////////////////////////////
//	Scalar::FullText

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new FullTextImpl::Nadic(eOperator_,
								cstrName_,
								vecOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*
//	Interface::IScalar*>& cOperand_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new FullTextImpl::Dyadic(eOperator_,
								 cstrName_,
								 cOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new FullTextImpl::Monadic(eOperator_,
								  cstrName_,
								  pOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new FullTextImpl::MonadicWithOption(eOperator_,
											cstrName_,
											pOperand_,
											pOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::FullText::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new FullTextImpl::MonadicWithNadicOption(eOperator_,
												 cstrName_,
												 pOperand_,
												 vecOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   vecOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*
//	Interface::IScalar*>& cOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   cOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   pOption_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::FullText::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	FullText*
//
// EXCEPTIONS

//static
FullText*
FullText::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   vecOption_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION protected
//	Scalar::FullText::getFunctionType -- get function type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Field::Function::Value
//
// EXCEPTIONS

Schema::Field::Function::Value
FullText::
getFunctionType()
{
	switch (getType()) {
	case Tree::Node::Score:				return Schema::Field::Function::Score;
	case Tree::Node::Section:			return Schema::Field::Function::Section;
	case Tree::Node::Word:				return Schema::Field::Function::Word;
	case Tree::Node::WordDf:			return Schema::Field::Function::WordDf;
	case Tree::Node::WordScale:			return Schema::Field::Function::WordScale;
	case Tree::Node::Tf:				return Schema::Field::Function::Tf;
	case Tree::Node::ClusterID:			return Schema::Field::Function::ClusterId;
	case Tree::Node::FeatureValue:		return Schema::Field::Function::ClusterWord;
	case Tree::Node::RoughKwicPosition:	return Schema::Field::Function::Kwic;
	case Tree::Node::Existence:			return Schema::Field::Function::Existence;
	}
	return Schema::Field::Function::Undefined;
}

// FUNCTION protected
//	Scalar::FullText::getIndexFile -- seach for index file for one operand
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	FullTextImpl::GetIndexArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FullText::
getIndexFile(Opt::Environment& cEnvironment_,
			 Interface::IScalar* pOperand_,
			 FullTextImpl::GetIndexArgument& cArgument_)
{
	if (cArgument_.m_bResult && pOperand_->isField()) {
		Scalar::Field* pField = pOperand_->getField();
		; _SYDNEY_ASSERT(pField);

		if (pField->isColumn()) {
			Schema::Column* pSchemaColumn = pField->getSchemaColumn();
			; _SYDNEY_ASSERT(pSchemaColumn);

			// get key field corresponding to the column
			ModVector<Schema::Field*> vecIndexField;
			Trans::Transaction& cTrans = cEnvironment_.getTransaction();
			if (pSchemaColumn->getIndexField(cTrans, vecIndexField)) {
				Opt::FilterContainer(vecIndexField,
									 boost::bind(&Schema::Field::hasFunction,
												 _1,
												 cArgument_.m_eFunction,
												 boost::ref(cTrans)));
				if (vecIndexField.isEmpty() == false) {
					cArgument_.m_vecmapField.PUSHBACK(FullTextImpl::GetIndexArgument::Map());
					FullTextImpl::GetIndexArgument::Map& cMap = cArgument_.m_vecmapField.GETBACK();
					Utility::SchemaFileSet cIndexFileSet;
					Opt::ForEach(vecIndexField,
								 boost::bind(_addField,
											 boost::ref(cTrans),
											 boost::ref(cMap),
											 boost::ref(cIndexFileSet),
											 _1));
					; _SYDNEY_ASSERT(cIndexFileSet.isEmpty() == false);

					if (cArgument_.m_pTable == 0) {
						// first call
						cArgument_.m_pTable = pField->getTable();
						cArgument_.m_cIndexFileSet = cIndexFileSet;
						return cArgument_.m_bResult;
					} else {
						// merge
						if (cArgument_.m_pTable == pField->getTable()) {
							cArgument_.m_cIndexFileSet.intersect(cIndexFileSet);
							if (cArgument_.m_cIndexFileSet.isEmpty() == false) {
								return cArgument_.m_bResult;
							}
						}
					}
				}
			}
		}
	}
	return cArgument_.m_bResult = false;
}

// FUNCTION protected
//	Scalar::FullText::createOperandField -- create corresponding field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	int iIndex_
//	FullTextImpl::GetIndexArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FullText::
createOperandField(Opt::Environment& cEnvironment_,
				   Interface::IScalar* pOperand_,
				   int iIndex_,
				   FullTextImpl::GetIndexArgument& cArgument_)
{
	; _SYDNEY_ASSERT(iIndex_ >= 0 && iIndex_ < cArgument_.m_vecmapField.GETSIZE());
	; _SYDNEY_ASSERT(pOperand_->isField());

	Utility::SchemaFileSet::Iterator iterator = cArgument_.m_cIndexFileSet.begin();
	const Utility::SchemaFileSet::Iterator last = cArgument_.m_cIndexFileSet.end();
	for(; iterator != last; ++iterator) {
		Schema::Field* pSchemaField = cArgument_.m_vecmapField[iIndex_][*iterator];
		; _SYDNEY_ASSERT(pSchemaField);

		FieldImpl::FunctionField::createFunctionFile(cEnvironment_,
													 pSchemaField,
													 cArgument_.m_cFileSet,
													 cArgument_.m_pTable,
													 pOperand_);
	}
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
