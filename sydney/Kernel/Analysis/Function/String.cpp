// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/String.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/String.h"
#include "Analysis/Function/FullText.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"

#include "Exception/KwicWithoutContains.h"
#include "Exception/InvalidFunction.h"
#include "Exception/InvalidKwic.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Scalar/Function.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace StringImpl
{
	// CLASS local
	//	Function::StringImpl::Concatenate -- implementation class for string function analyzer
	//
	// NOTES
	class Concatenate
		: public Function::String
	{
	public:
		typedef Concatenate This;
		typedef Function::String Super;

		// constructor
		Concatenate() : Super() {}
		// destructor
		~Concatenate() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::CharLength -- implementation class for string function analyzer
	//
	// NOTES
	class CharLength
		: public Function::String
	{
	public:
		typedef CharLength This;
		typedef Function::String Super;

		// constructor
		CharLength() : Super() {}
		// destructor
		~CharLength() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::SubString -- implementation class for string function analyzer
	//
	// NOTES
	class SubString
		: public Function::String
	{
	public:
		typedef SubString This;
		typedef Function::String Super;

		// constructor
		SubString() : Super() {}
		// destructor
		~SubString() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::Overlay -- implementation class for string function analyzer
	//
	// NOTES
	class Overlay
		: public Function::String
	{
	public:
		typedef Overlay This;
		typedef Function::String Super;

		// constructor
		Overlay() : Super() {}
		// destructor
		~Overlay() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::Normalize -- implementation class for string function analyzer
	//
	// NOTES
	class Normalize
		: public Function::String
	{
	public:
		typedef Normalize This;
		typedef Function::String Super;

		// constructor
		Normalize() : Super() {}
		// destructor
		~Normalize() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::Kwic -- implementation class for string function analyzer
	//
	// NOTES
	class Kwic
		: public Function::String
	{
	public:
		typedef Kwic This;
		typedef Function::String Super;

		// constructor
		Kwic() : Super() {}
		// destructor
		~Kwic() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::ExpandSynonym -- implementation class for string function analyzer
	//
	// NOTES
	class ExpandSynonym
		: public Function::String
	{
	public:
		typedef ExpandSynonym This;
		typedef Function::String Super;

		// constructor
		ExpandSynonym() : Super() {}
		// destructor
		~ExpandSynonym() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
	// CLASS local
	//	Function::StringImpl::CharJoin -- implementation class for string function analyzer
	//
	// NOTES
	class CharJoin
		: public Function::String
	{
	public:
		typedef CharJoin This;
		typedef Function::String Super;

		// constructor
		CharJoin() : Super() {}
		// destructor
		~CharJoin() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace Impl

namespace
{
	// FUNCTION local
	//	$$$::_getNodeType -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	int iFunc_
	//	
	// RETURN
	//	Plan::Tree::Node::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getNodeType(int iFunc_)
	{
		switch (iFunc_) {
		case Statement::ValueExpression::func_Char_Length:
			{
				return Plan::Tree::Node::CharLength;
			}
		case Statement::ValueExpression::func_Octet_Length:
			{
				return Plan::Tree::Node::OctetLength;
			}
		case Statement::ValueExpression::func_SubString:
			{
				return Plan::Tree::Node::SubString;
			}
		case Statement::ValueExpression::func_Overlay:
			{
				return Plan::Tree::Node::Overlay;
			}
		case Statement::ValueExpression::func_Normalize:
			{
				return Plan::Tree::Node::Normalize;
			}
		case Statement::ValueExpression::func_Kwic:
			{
				return Plan::Tree::Node::Kwic;
			}
		case Statement::ValueExpression::func_Expand_Synonym:
			{
				return Plan::Tree::Node::ExpandSynonym;
			}
		case Statement::ValueExpression::func_Word_Count:
			{
				return Plan::Tree::Node::WordCount;
			}
		case Statement::ValueExpression::func_FullText_Length:
			{
				return Plan::Tree::Node::FullTextLength;
			}
		case Statement::ValueExpression::func_Char_Join:
			{
				return Plan::Tree::Node::CharJoin;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const StringImpl::Concatenate _analyzerconcat;
	const StringImpl::CharLength _analyzerlength;
	const StringImpl::SubString _analyzersubstring;
	const StringImpl::Overlay _analyzeroverlay;
	const StringImpl::Normalize _analyzernormalize;
	const StringImpl::Kwic _analyzerkwic;
	const StringImpl::ExpandSynonym _analyzerexpandsynonym;
	const StringImpl::CharJoin _analyzercharjoin;

} // namespace

//////////////////////////////////////
//	Function::StringImpl::Concatenate

// FUNCTION private
//	Function::StringImpl::Concatenate::createScalar -- 
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

//virtual
Plan::Interface::IScalar*
StringImpl::Concatenate::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpression* pLeft = pVE->getLeft();
	Statement::ValueExpression* pRight = pVE->getRight();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand0 =
		pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);
	Plan::Interface::IScalar* pOperand1 =
		pRight->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pRight);

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::StringConcatenate,
										  MAKEPAIR(pOperand0, pOperand1),
										  pVE->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::CharLength

// FUNCTION private
//	Function::StringImpl::CharLength::createScalar -- 
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

//virtual
Plan::Interface::IScalar*
StringImpl::CharLength::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpression* pLeft = pVE->getLeft();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand =
		pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE->getFunction()),
										  pOperand,
										  pVE->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::SubString

// FUNCTION private
//	Function::StringImpl::SubString::createScalar -- 
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

//virtual
Plan::Interface::IScalar*
StringImpl::SubString::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Function::SubString* pSS =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::SubString*, pStatement_);
	; _SYDNEY_ASSERT(pSS);

	Statement::ValueExpression* pSource = pSS->getSource();
	Statement::ValueExpression* pStart = pSS->getStartPosition();
	Statement::ValueExpression* pLength = pSS->getStringLength();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand =
		pSource->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pSource);
	VECTOR<Plan::Interface::IScalar*> vecOption;
	vecOption.PUSHBACK(pStart->getAnalyzer2()->getScalar(cEnvironment_,
														 pRelation_,
														 pStart)
					   ->setExpectedType(cEnvironment_,
										 Plan::Scalar::DataType::getIntegerType()));
	if (pLength) {
		vecOption.PUSHBACK(pLength->getAnalyzer2()->getScalar(cEnvironment_,
															  pRelation_,
															  pLength)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getIntegerType()));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pSS->getFunction()),
										  pOperand,
										  vecOption,
										  pSS->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::Overlay

// FUNCTION private
//	Function::StringImpl::Overlay::createScalar --
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

//virtual
Plan::Interface::IScalar*
StringImpl::Overlay::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Function::Overlay* pFO =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::Overlay*, pStatement_);
	; _SYDNEY_ASSERT(pFO);

	Statement::ValueExpression* pDest = pFO->getDestination();
	Statement::ValueExpression* pPlacing = pFO->getPlacing();
	Statement::ValueExpression* pStart = pFO->getStartPosition();
	Statement::ValueExpression* pLength = pFO->getStringLength();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand0 =
		pDest->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pDest);
	Plan::Interface::IScalar* pOperand1 =
		pPlacing->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pPlacing);

	VECTOR<Plan::Interface::IScalar*> vecOption;
	vecOption.PUSHBACK(pStart->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pStart)
					   ->setExpectedType(cEnvironment_,
										 Plan::Scalar::DataType::getIntegerType()));
	if (pLength) {
		vecOption.PUSHBACK(pLength->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLength)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getIntegerType()));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pFO->getFunction()),
										  MAKEPAIR(pOperand0, pOperand1),
										  vecOption,
										  pFO->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::Normalize

// FUNCTION private
//	Function::StringImpl::Normalize::createScalar --
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

//virtual
Plan::Interface::IScalar*
StringImpl::Normalize::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Function::Normalize* pFN =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::Normalize*, pStatement_);
	; _SYDNEY_ASSERT(pFN);

	Statement::ValueExpression* pSrc = pFN->getSource();
	Statement::ValueExpression* pParam = pFN->getParameter();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand =
		pSrc->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pSrc)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());
	Plan::Interface::IScalar* pOption =
		pParam->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pParam)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pFN->getFunction()),
										  pOperand,
										  pOption,
										  pFN->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::Kwic

// FUNCTION private
//	Function::StringImpl::Kwic::createScalar --
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

//virtual
Plan::Interface::IScalar*
StringImpl::Kwic::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	if (cEnvironment_.checkStatus(Opt::Environment::Status::SelectList) == false) {
		// kwic can be used only in select list
		_SYDNEY_THROW0(Exception::InvalidKwic);
	}

	Statement::Function::Kwic* pFK =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::Kwic*, pStatement_);
	; _SYDNEY_ASSERT(pFK);

	Statement::ValueExpression* pSrc = pFK->getSource();
	Statement::ValueExpression* pSize = pFK->getSize();
	Statement::ValueExpression* pStartTag = pFK->getStartTag();
	Statement::ValueExpression* pEndTag = pFK->getEndTag();
	Statement::ValueExpression* pEscape = pFK->getEscape();
	Statement::ValueExpression* pEllipsis = pFK->getEllipsis();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);
	
	// create operand scalar
	PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*> cOperand;
	cOperand.first = pSrc->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pSrc);

	// create scalar using fulltext analyzer to get kwic start position
	cOperand.second = FullText::create(pFK)->getScalar(cEnvironment_,
													   pRelation_,
													   pFK);

	if (cOperand.second == 0) {
		// Kwic is used without contains
		_SYDNEY_THROW0(Exception::KwicWithoutContains);
	}

	VECTOR<Plan::Interface::IScalar*> vecOption;
	vecOption.PUSHBACK(pSize->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pSize)
					   ->setExpectedType(cEnvironment_,
										 Plan::Scalar::DataType::getIntegerType())
					   ->setNodeType(Plan::Tree::Node::KwicSize));
	if (pStartTag) {
		vecOption.PUSHBACK(pStartTag->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pStartTag)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getStringType())
						   ->setNodeType(Plan::Tree::Node::KwicStartTag));
		if (pEndTag) {
			vecOption.PUSHBACK(pEndTag->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pEndTag)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::KwicEndTag));
		}
	}
	if (pEscape) {
		vecOption.PUSHBACK(pEscape->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pEscape)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getStringType())
						   ->setNodeType(Plan::Tree::Node::KwicEscape));
	}
	if (pEllipsis) {
		vecOption.PUSHBACK(pEllipsis->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pEllipsis)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getStringType())
						   ->setNodeType(Plan::Tree::Node::KwicEllipsis));
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pFK->getFunction()),
										  cOperand,
										  vecOption,
										  pFK->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::ExpandSynonym

// FUNCTION private
//	Function::StringImpl::ExpandSynonym::createScalar --
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

//virtual
Plan::Interface::IScalar*
StringImpl::ExpandSynonym::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Function::ExpandSynonym* pFN =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::ExpandSynonym*, pStatement_);
	; _SYDNEY_ASSERT(pFN);

	Statement::ValueExpression* pSrc = pFN->getSource();
	Statement::ValueExpression* pParam = pFN->getParameter();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand =
		pSrc->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pSrc)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());
	Plan::Interface::IScalar* pOption =
		pParam->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pParam)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());

	Plan::Scalar::DataType cType;
	if (cEnvironment_.checkStatus(Opt::Environment::Status::ContainsOperand) == false) {
		// set array type
		cType = Plan::Scalar::DataType::getArrayType(-1, Plan::Scalar::DataType::getStringType());
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pFN->getFunction()),
										  pOperand,
										  pOption,
										  cType,
										  pFN->toSQLStatement());
}

//////////////////////////////////////
//	Function::StringImpl::CharJoin

// FUNCTION private
//	Function::StringImpl::CharJoin::createScalar --
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

//virtual
Plan::Interface::IScalar*
StringImpl::CharJoin::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	if (pVE->getPrimary() == 0) {
		// Char_Join needs more than one arguments
		_SYDNEY_THROW1(Exception::InvalidFunction,
					   _TRMEISTER_U_STRING("Char_Join"));
	}

	Statement::ValueExpressionList* pList =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*,
							 pVE->getPrimary());
	;_SYDNEY_ASSERT(pList);

	if (pList->getCount() < 2) {
		// Char_Join needs more than one arguments
		_SYDNEY_THROW1(Exception::InvalidFunction,
					   _TRMEISTER_U_STRING("Char_Join"));
	}

	VECTOR<Plan::Interface::IScalar*> vecOperand;
	pList->getAnalyzer2()->addScalar(cEnvironment_,
									 pRelation_,
									 pList,
									 vecOperand);

	Plan::Interface::IScalar* pSeparator =
		vecOperand[0]->setExpectedType(cEnvironment_,
									   Plan::Scalar::DataType::getStringType());
	vecOperand.POPFRONT();

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE->getFunction()),
										  vecOperand,
										  pSeparator,
										  pVE->toSQLStatement());
}

//////////////////////////////
// Function::String
//////////////////////////////

// FUNCTION public
//	Function::String::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const String*
//
// EXCEPTIONS

//static
const String*
String::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_String_concat:
		{
			return &_analyzerconcat;
		}
	case Statement::ValueExpression::op_Func:
		{
			switch (pStatement_->getFunction()) {
			case Statement::ValueExpression::func_Char_Length:
			case Statement::ValueExpression::func_Word_Count:
			case Statement::ValueExpression::func_FullText_Length:
			//case Statement::ValueExpression::func_Octet_Length: // not supported
				{
					return &_analyzerlength;
				}
			case Statement::ValueExpression::func_SubString:
				{
					return &_analyzersubstring;
				}
			case Statement::ValueExpression::func_Overlay:
				{
					return &_analyzeroverlay;
				}
			case Statement::ValueExpression::func_Normalize:
				{
					return &_analyzernormalize;
				}
			case Statement::ValueExpression::func_Kwic:
				{
					return &_analyzerkwic;
				}
			case Statement::ValueExpression::func_Expand_Synonym:
				{
					return &_analyzerexpandsynonym;
				}
			case Statement::ValueExpression::func_Char_Join:
				{
					return &_analyzercharjoin;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
