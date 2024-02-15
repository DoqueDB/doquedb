// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/ContainsOperand.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Value";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Value/ContainsOperand.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Value.h"

#include "Statement/ContainsOperand.h"
#include "Statement/ContainsOperandList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsPattern --
	//
	// NOTES
	class ContainsPattern
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsPattern This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsPattern() : Super() {}
		// destructor
		virtual ~ContainsPattern() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsCombinator --
	//
	// NOTES
	class ContainsCombinator
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsCombinator This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsCombinator() : Super() {}
		// destructor
		virtual ~ContainsCombinator() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
		// get node type of generated scalar
		Plan::Tree::Node::Type getNodeType(int iType_) const;
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsFreeText --
	//
	// NOTES
	class ContainsFreeText
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsFreeText This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsFreeText() : Super() {}
		// destructor
		virtual ~ContainsFreeText() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsSpecial --
	//
	// NOTES
	class ContainsSpecial
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsSpecial This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsSpecial() : Super() {}
		// destructor
		virtual ~ContainsSpecial() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
		// get node type of generated scalar
		Plan::Tree::Node::Type getNodeType(int iType_) const;
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsWeight --
	//
	// NOTES
	class ContainsWeight
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsWeight This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsWeight() : Super() {}
		// destructor
		virtual ~ContainsWeight() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsWithin --
	//
	// NOTES
	class ContainsWithin
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsWithin This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsWithin() : Super() {}
		// destructor
		virtual ~ContainsWithin() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsWord --
	//
	// NOTES
	class ContainsWord
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsWord This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsWord() : Super() {}
		// destructor
		virtual ~ContainsWord() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ContainsWordList --
	//
	// NOTES
	class ContainsWordList
		: public Value::ContainsOperand
	{
	public:
		typedef ContainsWordList This;
		typedef Value::ContainsOperand Super;

		// constructor
		ContainsWordList() : Super() {}
		// destructor
		virtual ~ContainsWordList() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES

	const Impl::ContainsPattern _analyzerPattern;
	const Impl::ContainsCombinator _analyzerCombinator;
	const Impl::ContainsFreeText _analyzerFreeText;
	const Impl::ContainsSpecial _analyzerSpecial;
	const Impl::ContainsWeight _analyzerWeight;
	const Impl::ContainsWithin _analyzerWithin;
	const Impl::ContainsWord _analyzerWord;
	const Impl::ContainsWordList _analyzerWordList;
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsPattern

// FUNCTION public
//	Value::Impl::ContainsPattern::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsPattern::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::Pattern* pCP =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::Pattern*, pStatement_);
	; _SYDNEY_ASSERT(pCP);

	Statement::ValueExpression* pPattern = pCP->getPattern();
	Statement::ValueExpression* pLanguage = pCP->getLanguage();

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Pattern,
										  pPattern->getAnalyzer2()->getScalar(cEnvironment_,
																			  pRelation_,
																			  pPattern),
										  pLanguage ?
										  pLanguage->getAnalyzer2()->getScalar(cEnvironment_,
																			   pRelation_,
																			   pLanguage)
										  ->setExpectedType(cEnvironment_,
															Plan::Scalar::DataType::getLanguageType())
										  ->setNodeType(Plan::Tree::Node::Language)
										  : 0,
										  pCP->toSQLStatement());
}

//////////////////////////////////////////////
// Analysis::Value::Impl::ContainsCombinator

// FUNCTION public
//	Value::Impl::ContainsCombinator::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsCombinator::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::LogicalOperation* pCL =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::LogicalOperation*, pStatement_);
	; _SYDNEY_ASSERT(pCL);

	Statement::ContainsOperandList* pList = pCL->getOperandList();
	Statement::ValueExpression* pCombiner = pCL->getCombiner();

	VECTOR<Plan::Interface::IScalar*> vecOperand;

	int n = pList->getCount();
	; _SYDNEY_ASSERT(n);
	vecOperand.reserve(n);

	int i = 0;
	do {
		Statement::ContainsOperand* pOperand = pList->getContainsOperandAt(i);
		; _SYDNEY_ASSERT(pOperand);
		Plan::Interface::IScalar* pOperandScalar =
			pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);
		vecOperand.PUSHBACK(pOperandScalar);
	} while (++i < n);

	Plan::Interface::IScalar* pCombinerScalar = 0;
	if (pCombiner) {
		pCombinerScalar =
			pCombiner->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pCombiner)
			->setNodeType(Plan::Tree::Node::Combiner);
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  getNodeType(pCL->getType()),
										  vecOperand,
										  pCombinerScalar,
										  pCL->toSQLStatement());
}

// FUNCTION private
//	Value::Impl::ContainsCombinator::getNodeType -- 
//
// NOTES
//
// ARGUMENTS
//	int iType_
//	
// RETURN
//	Plan::Tree::Node::Type
//
// EXCEPTIONS

Plan::Tree::Node::Type
Impl::ContainsCombinator::
getNodeType(int iType_) const
{
	switch (iType_) {
#define NODETYPE(x_) \
	case Statement::ContainsOperand::Type::x_: return Plan::Tree::Node::x_
		NODETYPE(And);
		NODETYPE(Or);
		NODETYPE(AndNot);
		NODETYPE(Synonym);
#undef NODETYPE
	default:break;
	}
	; _SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsFreeText

// FUNCTION public
//	Value::Impl::ContainsFreeText::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsFreeText::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::FreeText* pCFT =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::FreeText*, pStatement_);
	; _SYDNEY_ASSERT(pCFT);

	Statement::ValueExpression* pPattern = pCFT->getPattern();
	Statement::ValueExpression* pLanguage = pCFT->getLanguage();
	Statement::ValueExpression* pScaleParam = pCFT->getScaleParameter();
	Statement::ValueExpression* pWordLimit = pCFT->getWordLimit();

	Plan::Interface::IScalar* pPatternScalar =
		pPattern->getAnalyzer2()->getScalar(
			cEnvironment_, pRelation_, pPattern);

	VECTOR<Plan::Interface::IScalar*> vecOption;

	if (pLanguage)
	{
		vecOption.PUSHBACK(pLanguage->getAnalyzer2()
						   ->getScalar(cEnvironment_, pRelation_, pLanguage)
						   ->setExpectedType(
							   cEnvironment_,
							   Plan::Scalar::DataType::getLanguageType())
						   ->setNodeType(Plan::Tree::Node::Language));
	}
	if (pScaleParam)
	{
		vecOption.PUSHBACK(pScaleParam->getAnalyzer2()
						   ->getScalar(cEnvironment_, pRelation_, pScaleParam)
						   ->setExpectedType(
							   cEnvironment_,
							   Plan::Scalar::DataType::getFloatType())
						   ->setNodeType(Plan::Tree::Node::ScaleParameter));
	}
	if (pWordLimit)
	{
		vecOption.PUSHBACK(pWordLimit->getAnalyzer2()
						   ->getScalar(cEnvironment_, pRelation_, pWordLimit)
						   ->setExpectedType(
							   cEnvironment_,
							   Plan::Scalar::DataType::getIntegerType())
						   ->setNodeType(Plan::Tree::Node::WordLimit));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Freetext,
										  pPatternScalar,
										  vecOption,
										  pCFT->toSQLStatement());
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsSpecial

// FUNCTION public
//	Value::Impl::ContainsSpecial::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsSpecial::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::SpecialPattern* pCSP =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::SpecialPattern*, pStatement_);
	; _SYDNEY_ASSERT(pCSP);

	Statement::ContainsOperand* pOperand = pCSP->getOperand();
	Plan::Interface::IScalar* pOperandScalar =
		pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);

	return Plan::Scalar::Function::create(cEnvironment_,
										  getNodeType(pCSP->getType()),
										  pOperandScalar,
										  pCSP->toSQLStatement());
}

// FUNCTION private
//	Value::Impl::ContainsSpecial::getNodeType -- 
//
// NOTES
//
// ARGUMENTS
//	int iType_
//	
// RETURN
//	Plan::Tree::Node::Type
//
// EXCEPTIONS

Plan::Tree::Node::Type
Impl::ContainsSpecial::
getNodeType(int iType_) const
{
	switch (iType_) {
#define NODETYPE(x_) \
	case Statement::ContainsOperand::Type::x_: return Plan::Tree::Node::x_
		NODETYPE(Head);
		NODETYPE(Tail);
		NODETYPE(ExactWord);
		NODETYPE(SimpleWord);
		NODETYPE(String);
		NODETYPE(WordHead);
		NODETYPE(WordTail);
		NODETYPE(ExpandSynonym);
#undef NODETYPE
	default: break;
	}
	; _SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);

}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsWeight

// FUNCTION public
//	Value::Impl::ContainsWeight::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsWeight::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::Weight* pCW =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::Weight*, pStatement_);
	; _SYDNEY_ASSERT(pCW);

	Statement::ContainsOperand* pOperand = pCW->getOperand();
	Statement::ValueExpression* pScale = pCW->getScale();

	Plan::Interface::IScalar* pOperandScalar =
		pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);
	Plan::Interface::IScalar* pScaleScalar = 0;
	if (pScale) {
		pScaleScalar =
			pScale->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pScale)
			->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getFloatType())
			->setNodeType(Plan::Tree::Node::Scale);
	}
	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Weight,
										  pOperandScalar,
										  pScaleScalar,
										  pCW->toSQLStatement());
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsWithin

// FUNCTION public
//	Value::Impl::ContainsWithin::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsWithin::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::Within* pCW =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::Within*, pStatement_);
	; _SYDNEY_ASSERT(pCW);

	Statement::ContainsOperandList* pList = pCW->getOperandList();
	Statement::ValueExpression* pCombiner = pCW->getCombiner();
	Statement::ValueExpression* pLowerDist = pCW->getLowerDist();
	Statement::ValueExpression* pUpperDist = pCW->getUpperDist();

	VECTOR<Plan::Interface::IScalar*> vecOperand;

	int n = pList->getCount();
	; _SYDNEY_ASSERT(n);
	vecOperand.reserve(n);

	int i = 0;
	do {
		Statement::ContainsOperand* pOperand = pList->getContainsOperandAt(i);
		; _SYDNEY_ASSERT(pOperand);
		Plan::Interface::IScalar* pOperandScalar =
			pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);
		vecOperand.PUSHBACK(pOperandScalar);
	} while (++i < n);

	VECTOR<Plan::Interface::IScalar*> vecOption;

	// set symmetric option
	vecOption.PUSHBACK(Plan::Scalar::Value::create(cEnvironment_,
												   Common::Data::Pointer(
														 new Common::IntegerData(
																 (pCW->getSymmetric()
																  != Statement::Contains::Within::Asymmetric)
																 ? 1 : 0)))
					   ->setNodeType(Plan::Tree::Node::Symmetric));

	// set lower option
	if (pLowerDist) {
		vecOption.PUSHBACK(pLowerDist->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLowerDist)
						   ->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getIntegerType())
						   ->setNodeType(Plan::Tree::Node::Lower));
	}
	// set upper option
	if (pUpperDist) {
		vecOption.PUSHBACK(pUpperDist->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pUpperDist)
						   ->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getIntegerType())
						   ->setNodeType(Plan::Tree::Node::Upper));
	}
	// set combiner option
	if (pCombiner) {
		vecOption.PUSHBACK(pCombiner->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pCombiner)
						   ->setNodeType(Plan::Tree::Node::Combiner));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Within,
										  vecOperand,
										  vecOption,
										  pCW->toSQLStatement());
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsWord

// FUNCTION public
//	Value::Impl::ContainsWord::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsWord::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::Word* pCW =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::Word*, pStatement_);
	; _SYDNEY_ASSERT(pCW);

	Statement::ContainsOperand* pPattern = pCW->getPattern();
	Statement::ValueExpression* pCategory = pCW->getCategory();
	Statement::ValueExpression* pScale = pCW->getScale();
	Statement::ValueExpression* pLanguage = pCW->getLanguage();
	Statement::ValueExpression* pDf = pCW->getDf();

	VECTOR<Plan::Interface::IScalar*> vecOption;

	if (pCategory) {
		vecOption.PUSHBACK(pCategory->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pCategory)
						   ->setNodeType(Plan::Tree::Node::Category));
	}
	if (pScale) {
		vecOption.PUSHBACK(pScale->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pScale)
						   ->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getFloatType())
						   ->setNodeType(Plan::Tree::Node::Scale));
	}
	if (pLanguage) {
		vecOption.PUSHBACK(pLanguage->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLanguage)
						   ->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getLanguageType())
						   ->setNodeType(Plan::Tree::Node::Language));
	}
	if (pDf) {
		vecOption.PUSHBACK(pDf->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pDf)
						   ->setExpectedType(cEnvironment_, Plan::Scalar::DataType::getIntegerType())
						   ->setNodeType(Plan::Tree::Node::Df));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Word,
										  pPattern->getAnalyzer2()->getScalar(cEnvironment_,
																			  pRelation_,
																			  pPattern),
										  vecOption,
										  pCW->toSQLStatement());
}

////////////////////////////////////////////
// Analysis::Value::Impl::ContainsWordList

// FUNCTION public
//	Value::Impl::ContainsWordList::getScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::ContainsWordList::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Contains::WordList* pCW =
		_SYDNEY_DYNAMIC_CAST(Statement::Contains::WordList*, pStatement_);
	; _SYDNEY_ASSERT(pCW);

	Statement::ContainsOperandList* pList = pCW->getOperandList();

	VECTOR<Plan::Interface::IScalar*> vecOperand;

	int n = pList->getCount();
	; _SYDNEY_ASSERT(n);
	vecOperand.reserve(n);

	int i = 0;
	do {
		Statement::ContainsOperand* pOperand = pList->getContainsOperandAt(i);
		; _SYDNEY_ASSERT(pOperand);
		Plan::Interface::IScalar* pOperandScalar =
			pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);
		vecOperand.PUSHBACK(pOperandScalar);
	} while (++i < n);

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::WordList,
										  vecOperand,
										  pCW->toSQLStatement());
}

//////////////////////////////
// Predicate::ContainsOperand::

// FUNCTION public
//	Value::ContainsOperand::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ContainsOperand* pStatement_
//	
// RETURN
//	const ContainsOperand*
//
// EXCEPTIONS

//static
const ContainsOperand*
ContainsOperand::
create(const Statement::ContainsOperand* pStatement_)
{
	switch (pStatement_->getType()) {
	case Statement::ContainsOperand::Type::Pattern:
		{
			return &_analyzerPattern;
		}
	case Statement::ContainsOperand::Type::And:
	case Statement::ContainsOperand::Type::Or:
	case Statement::ContainsOperand::Type::AndNot:
	case Statement::ContainsOperand::Type::Synonym:
		{
			return &_analyzerCombinator;
		}
	case Statement::ContainsOperand::Type::FreeText:
		{
			return &_analyzerFreeText;
		}
	case Statement::ContainsOperand::Type::Head:
	case Statement::ContainsOperand::Type::Tail:
	case Statement::ContainsOperand::Type::ExactWord:
	case Statement::ContainsOperand::Type::SimpleWord:
	case Statement::ContainsOperand::Type::String:
	case Statement::ContainsOperand::Type::WordHead:
	case Statement::ContainsOperand::Type::WordTail:
	case Statement::ContainsOperand::Type::ExpandSynonym:
		{
			return &_analyzerSpecial;
		}
	case Statement::ContainsOperand::Type::Weight:
		{
			return &_analyzerWeight;
		}
	case Statement::ContainsOperand::Type::Within:
		{
			return &_analyzerWithin;
		}
	case Statement::ContainsOperand::Type::Word:
		{
			return &_analyzerWord;
		}
	case Statement::ContainsOperand::Type::WordList:
		{
			return &_analyzerWordList;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
