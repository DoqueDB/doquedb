// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FunctionImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FUNCTIONIMPL_H
#define __SYDNEY_PLAN_SCALAR_FUNCTIONIMPL_H

#include "boost/bind.hpp"

#include "Plan/Scalar/Function.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Sql/Argument.h"
#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

#include "Exception/NotCompatible.h"
#include "Exception/NotSupported.h"
#include "Common/Assert.h"
#include "Common/Message.h"

#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"

#include "DExecution/Action/StatementConstruction.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN
namespace {
	enum Operator {
		ExpandSynonym,
		SubString,
		Freetext,
		WordList,
		Sum,
		Count,
		Avg,
		Max,
		Min,
		Absolute,
		Score,
		GetMax,
		Sectionized,
		Modulus,
		Add,
		Subtract,
		Multiply,
		Divide,
		WordHead,
		Within,
		Synonym,
		And,
		Or,
		AndNot,
		Normalize,
		String,
		ExactWord,
		SimpleWord,
		WordTail,
		CharLength,
		OctetLength,
		Distinct,
		StringConcatenate,
		Weight,
		Case,
		Cardinality,
		Overlay,
		Negative,
		Default
	};
	
	const char* const _pszOperatorName[] = 
	{
		" expand_synonym ",
		" substring ",
		" freetext",
		" wordlist",
		" sum",
		" count",
		" avg",
		" max",
		" min",
		" abs",
		" score",
		" get max",
		" sectioniezed",
		" mod",
		" + ",
		" - ",
		" * ",
		" / ",
		" wordhead",
		" within",
		" synonym",
		" & ",
		" | ",
		" - ",
		" normalize",
		" string",
		" exactword",
		" simpleword",
		" wordtail",
		" char_length",
		" octet_length",
		" distinct ",
		" || ",
		" weight",
		" case ",
		" cardinality",
		"overlay",
		"-",
		""
	};
	
	const char* _getOperatorName(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
		case Tree::Node::ExpandSynonym:		return _pszOperatorName[ExpandSynonym];
		case Tree::Node::SubString:			return _pszOperatorName[SubString];
		case Tree::Node::Freetext:			return _pszOperatorName[Freetext];
		case Tree::Node::WordList:			return _pszOperatorName[WordList];
		case Tree::Node::Sum:				return _pszOperatorName[Sum];
		case Tree::Node::Count:				return _pszOperatorName[Count];
		case Tree::Node::Avg:				return _pszOperatorName[Avg];
		case Tree::Node::Max:				return _pszOperatorName[Max];
		case Tree::Node::Min:				return _pszOperatorName[Min];
		case Tree::Node::Absolute:			return _pszOperatorName[Absolute];
		case Tree::Node::Score:				return _pszOperatorName[Score];
		case Tree::Node::GetMax:			return _pszOperatorName[GetMax];
		case Tree::Node::Section:			return _pszOperatorName[Sectionized];
		case Tree::Node::Modulus:			return _pszOperatorName[Modulus];
		case Tree::Node::Add:				return _pszOperatorName[Add];
		case Tree::Node::Subtract:			return _pszOperatorName[Subtract];
		case Tree::Node::Multiply:			return _pszOperatorName[Multiply];
		case Tree::Node::Divide:			return _pszOperatorName[Divide];
		case Tree::Node::WordHead:			return _pszOperatorName[WordHead];
		case Tree::Node::Within:			return _pszOperatorName[Within];
		case Tree::Node::Synonym:			return _pszOperatorName[Synonym];
		case Tree::Node::And:				return _pszOperatorName[And];
		case Tree::Node::Or:				return _pszOperatorName[Or];
		case Tree::Node::AndNot:			return _pszOperatorName[AndNot];
		case Tree::Node::Normalize:			return _pszOperatorName[Normalize];
		case Tree::Node::String:			return _pszOperatorName[String];
		case Tree::Node::ExactWord:			return _pszOperatorName[ExactWord];
		case Tree::Node::SimpleWord:		return _pszOperatorName[SimpleWord];
		case Tree::Node::WordTail:			return _pszOperatorName[WordTail];
		case Tree::Node::CharLength:		return _pszOperatorName[CharLength];
		case Tree::Node::OctetLength:		return _pszOperatorName[OctetLength];
		case Tree::Node::Distinct:			return _pszOperatorName[Distinct];
		case Tree::Node::StringConcatenate:	return _pszOperatorName[StringConcatenate];
		case Tree::Node::Weight:			return _pszOperatorName[Weight];
		case Tree::Node::Case:				return _pszOperatorName[Case];
		case Tree::Node::Cardinality:		return _pszOperatorName[Cardinality];
		case Tree::Node::Overlay:			return _pszOperatorName[Overlay];
		case Tree::Node::Negative:			return _pszOperatorName[Negative];
		default:							;_SYDNEY_ASSERT(false);
		}
		return 0;
	}
}
namespace FunctionImpl
{
	//////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::FunctionImpl::Base
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public Handle_
	{
	public:
		typedef Handle_ Super;
		typedef Base<Handle_> This;
		typedef typename Super::Operand Operand;
		typedef Interface::IScalar::Check Check;
		// following description was failed in linux
		//		typedef Super::Check Check;

		// constructor
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		Base()
			: Super()
		{}
#endif
		Base(Tree::Node::Type eType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument_)
		{
			setArgument(eType_, cstrName_);
		}
#else
			: Super(eType_, cstrName_, cArgument_)
		{}
#endif
		Base(Tree::Node::Type eType_,
			 const DataType& cDataType_,
			 const STRING& cstrName_,
			 typename Handle_::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument_)
		{
			setArgument(eType_, cDataType_, cstrName_);
		}
#else
			: Super(eType_, cDataType_, cstrName_, cArgument_)
		{}
#endif
		// destructor
		virtual ~Base() {}

	/////////////////////////
	// Interface::IScalar::
		virtual typename Check::Value check(Opt::Environment& cEnvironment_,
											const CheckArgument& cArgument_)
		{
			typename Check::Value cValue(Check::Constant);
			foreachOperand(boost::bind(&Interface::IScalar::Check::mergeValue,
									   &cValue,
									   boost::bind(&Handle_::Operand::check,
												   _1,
												   boost::ref(cEnvironment_),
												   boost::cref(cArgument_))));
			return cValue;
		}
		virtual bool isRefering(Interface::IRelation* pRelation_)
		{
			return isAll(boost::bind(&Operand::isRefering,
									 _1,
									 pRelation_));
		}
		virtual void getUsedTable(Utility::RelationSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedTable,
									   _1,
									   boost::ref(cResult_)));
		}
		virtual void getUsedField(Utility::FieldSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedField,
									   _1,
									   boost::ref(cResult_)));
		}
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUnknownKey,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cResult_)));
		}

		virtual bool hasParameter()
		{
			return isAny(boost::bind(&Operand::hasParameter,
									 _1));
		}
		virtual bool hasField(Interface::IFile* pFile_)
		{
			return isAny(boost::bind(&Operand::hasField,
									 _1,
									 pFile_));
		}

		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::use,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_)
		{
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_)));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::use,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
		}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_)
		{
			return isAny(boost::bind(&Operand::delay,
									 _1,
									 boost::ref(cEnvironment_),
									 pCandidate_,
									 boost::ref(cArgument_)));
		}

		virtual void pushNameScope(Plan::Sql::Query* pQuery_)
		{
			foreachOperand(boost::bind(&Operand::pushNameScope,
									   _1,
									   pQuery_));
		}		


		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			switch (getType())
			{
			case Tree::Node::Add:
			case Tree::Node::Subtract:
			case Tree::Node::Multiply:
			case Tree::Node::Divide:
			case Tree::Node::And:
			case Tree::Node::Or:
			case Tree::Node::AndNot:				
				cExec_.append("(");
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(_getOperatorName(getType()));
					}
					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}				
				cExec_.append(")");
				break;

			case Tree::Node::StringConcatenate:
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(_getOperatorName(getType()));
					}
					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				break;
			case Tree::Node::Word:
			case Tree::Node::Pattern:				
				for (int i = 0; i < getSize(); ++i) {
					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				break;
			case Tree::Node::Within:
			case Tree::Node::Synonym:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(" ");
					}

					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				cExec_.append(")");
				break;
			case Tree::Node::Distinct:
			case Tree::Node::Negative:
				cExec_.append(_getOperatorName(getType()));
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(",");
					}

					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				break;
			case Tree::Node::ExpandSynonym:
			case Tree::Node::SubString:		
			case Tree::Node::Freetext:		
			case Tree::Node::WordList:		
			case Tree::Node::Sum:			
			case Tree::Node::Count:			
			case Tree::Node::Avg:			
			case Tree::Node::Max:			
			case Tree::Node::Min:			
			case Tree::Node::Absolute:		
			case Tree::Node::Score:			
			case Tree::Node::GetMax:		
			case Tree::Node::Section:
			case Tree::Node::Modulus:
			case Tree::Node::Normalize:
			case Tree::Node::WordHead:
			case Tree::Node::String:
			case Tree::Node::WordTail:
			case Tree::Node::ExactWord:												
			case Tree::Node::SimpleWord:
			case Tree::Node::OctetLength:
			case Tree::Node::CharLength:
			case Tree::Node::Cardinality:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(",");
					}

					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				cExec_.append(")");
				break;
			default:
				cExec_.append(toSQLStatement(cEnvironment_,
											 cArgument_));
			}
		}
	   
	//	virtual void setMetaData(Opt::Environment& cEnvironment_,
	//							 Common::ColumnMetaData& cMetaData_);
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);

		using Super::getOperandi;
		using Super::getType;
		using Super::getSize;
		using Super::toSQLStatement;
		//		using Super::setDataType;
		using Super::getDataType;
		using Super::mapOperand;
		using Super::foreachOperand;
		using Super::isAll;
		using Super::isAny;

	protected:
		// generate
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_)
		{
			typename Handle_::MapIntResult cOperandData;
			mapOperand(cOperandData,
					   boost::bind(&Operand::generate,
								   _1,
								   boost::ref(cEnvironment_),
								   boost::ref(cProgram_),
								   pIterator_,
								   boost::ref(cArgument_)));
			pIterator_->addCalculation(cProgram_,
									   Execution::Function::Factory::create(
											cProgram_,
											pIterator_,
											getType(),
											cOperandData,
											iDataID_),
									   cArgument_.m_eTarget);
			return iDataID_;
		}

		void setDataType(const Scalar::DataType& cType_) {
			Super::setDataType(cType_);
		}

	private:
		// create data type
		virtual void createDataType(Opt::Environment& cEnvironment_)
		{
			// for throwing exceptions
			const char srcFile[] = __FILE__;
			const char moduleName[] = "Plan::Scalar";

			switch (getType()) {
			case Tree::Node::CharLength:
			case Tree::Node::OctetLength:
			case Tree::Node::FullTextLength:
			case Tree::Node::WordCount:
				{
					// If this result type is changed, logical file I/F has to be changed
					setDataType(DataType::getUnsignedIntegerType());
					break;
				}
			case Tree::Node::Copy:
				{
					// same as operand
					foreachOperand(boost::bind(&This::setDataType,
											   this,
											   boost::bind(&Operand::getDataType,
														   _1)));
					break;
				}
			case Tree::Node::Cast:
				{
					// do nothing
					break;
				}
			case Tree::Node::ExpandSynonym:
				{
					if (getDataType().isNoType()) {
						// treat as scalar string data because it's used as contains operand
						setDataType(DataType::getStringType());
					}
					break;
				}
			case Tree::Node::StringConcatenate:
			case Tree::Node::SubString:
			case Tree::Node::Overlay:
			case Tree::Node::Normalize:
				{
					DataType cResult;
					if (!isAll(boost::bind(&This::getCompatibleType,
										   this,
										   _1,
										   boost::ref(cResult)))) {
						// if any operand has incompatible type, creating fails
						_SYDNEY_THROW0(Exception::NotCompatible);
					}
					// set unlimited
					cResult.setFlag(DataType::Flag::Unlimited);
					cResult.setLength(0);
					setDataType(cResult);
					// set expected type to operands
					mapOperand(boost::bind(&Interface::IScalar::setExpectedType,
										   _1,
										   boost::ref(cEnvironment_),
										   boost::cref(cResult)));
					break;
				}
			case Tree::Node::Cardinality:
				{
					setDataType(DataType::getIntegerType());
					break;
				}
			case Tree::Node::Tf:
			case Tree::Node::Section:
			case Tree::Node::Existence:
				{
					setDataType(DataType::getArrayType(-1,
													   DataType::getUnsignedIntegerType()));
					break;
				}
			case Tree::Node::ClusterID:
			case Tree::Node::WordDf:
			case Tree::Node::RoughKwicPosition:
			case Tree::Node::NeighborID:
				{
					setDataType(DataType::getIntegerType());
					break;
				}
			case Tree::Node::FeatureValue:
				{
					setDataType(DataType::getArrayType(-1,
													   DataType::getWordType()));
					break;
				}
			case Tree::Node::Score:
			case Tree::Node::WordScale:
			case Tree::Node::NeighborDistance:
				{
					setDataType(DataType::getFloatType());
					break;
				}
			case Tree::Node::Word:
				{
					setDataType(DataType::getWordType());
					break;
				}
			case Tree::Node::CharJoin:
				{
					setDataType(DataType::getStringType());
					break;
				}
			default:
				{
					// set compatible type of all operands
					DataType cResult;
					if (!isAll(boost::bind(&This::getCompatibleType,
										   this,
										   _1,
										   boost::ref(cResult)))) {
						// if any operand has incompatible type, creating fails
						_SYDNEY_THROW0(Exception::NotCompatible);
					}
					setDataType(cResult);
					break;
				}
			}

			
		}
	};

	//////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::FunctionImpl::BaseWithOption
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class BaseWithOption
		: public Handle_
	{
	public:
		typedef Handle_ Super;
		typedef BaseWithOption<Handle_> This;
		typedef typename Super::Operand Operand;
		typedef typename Super::Option Option;
		typedef Interface::IScalar::Check Check;
		// following description was failed in linux
		//		typedef typename Super::Check Check;

		// constructor
		BaseWithOption(Tree::Node::Type eType_,
					   const STRING& cstrName_,
					   typename Handle_::Argument cArgument1_,
					   typename Handle_::OptionArgument cArgument2_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument2_)
		{
			setOperand(cArgument1_);
			setArgument(eType_, cstrName_);
		}
#else
			: Super(eType_, cstrName_, cArgument1_, cArgument2_)
		{}
#endif
		BaseWithOption(Tree::Node::Type eType_,
					   const DataType& cDataType_,
					   const STRING& cstrName_,
					   typename Handle_::Argument cArgument1_,
					   typename Handle_::OptionArgument cArgument2_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument2_)
		{
			setOperand(cArgument1_);
			setArgument(eType_, cDataType_, cstrName_);
		}
#else
			: Super(eType_, cDataType_, cstrName_, cArgument1_, cArgument2_)
		{}
#endif

		// destructor
		virtual ~BaseWithOption() {}

	/////////////////////////
	// Interface::IScalar::
		virtual typename Check::Value check(Opt::Environment& cEnvironment_,
											const CheckArgument& cArgument_)
		{
			typename Check::Value cValue(Check::Constant);
			foreachOperand(boost::bind(&Interface::IScalar::Check::mergeValue,
									   &cValue,
									   boost::bind(&Handle_::Operand::check,
												   _1,
												   boost::ref(cEnvironment_),
												   boost::cref(cArgument_))));
			foreachOption(boost::bind(&Interface::IScalar::Check::mergeValue,
									  &cValue,
									  boost::bind(&Handle_::Operand::check,
												  _1,
												  boost::ref(cEnvironment_),
												  boost::cref(cArgument_))));
			return cValue;
		}
		virtual bool isRefering(Interface::IRelation* pRelation_)
		{
			return isAll(boost::bind(&Operand::isRefering,
									 _1,
									 pRelation_))
				&& isAllOption(boost::bind(&Option::isRefering,
										   _1,
										   pRelation_));
		}
		virtual void getUsedTable(Utility::RelationSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedTable,
									   _1,
									   boost::ref(cResult_)));
			foreachOption(boost::bind(&Option::getUsedTable,
									  _1,
									  boost::ref(cResult_)));
		}
		virtual void getUsedField(Utility::FieldSet& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUsedField,
									   _1,
									   boost::ref(cResult_)));
			foreachOption(boost::bind(&Option::getUsedField,
									  _1,
									  boost::ref(cResult_)));
		}
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_)
		{
			foreachOperand(boost::bind(&Operand::getUnknownKey,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cResult_)));
			foreachOption(boost::bind(&Option::getUnknownKey,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cResult_)));
		}
		virtual bool hasParameter()
		{
			return isAny(boost::bind(&Operand::hasParameter,
									 _1))
				|| isAnyOption(boost::bind(&Option::hasParameter,
										   _1));
		}
		virtual bool hasField(Interface::IFile* pFile_)
		{
			return isAny(boost::bind(&Operand::hasField,
									 _1,
									 pFile_))
				|| isAnyOption(boost::bind(&Option::hasField,
										   _1,
										   pFile_));
		}

		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::use,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
			foreachOption(boost::bind(&Option::use,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_)
		{
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_)));
			foreachOption(boost::bind(&Option::retrieve,
									   _1,
									   boost::ref(cEnvironment_)));
		}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::retrieve,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
			foreachOption(boost::bind(&Option::retrieve,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
		{
			foreachOperand(boost::bind(&Operand::use,
									   _1,
									   boost::ref(cEnvironment_),
									   pCandidate_));
			foreachOption(boost::bind(&Option::use,
									  _1,
									  boost::ref(cEnvironment_),
									  pCandidate_));
		}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_)
		{
			return isAny(boost::bind(&Operand::delay,
									 _1,
									 boost::ref(cEnvironment_),
									 pCandidate_,
									 boost::ref(cArgument_)))
				|| isAnyOption(boost::bind(&Option::delay,
										   _1,
										   boost::ref(cEnvironment_),
										   pCandidate_,
										   boost::ref(cArgument_)));
		}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			switch (getType())
			{
			case Tree::Node::Word:
			case Tree::Node::Pattern:
				for (int i = 0; i < getSize(); ++i) {
					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				foreachOption(boost::bind(&Operand::setParameter,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cExec_),
										  boost::ref(cArgument_)));

				break;
			case Tree::Node::Within:
			case Tree::Node::Synonym:
			case Tree::Node::Weight:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(" ");
					}

					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				foreachOption(boost::bind(&Operand::setParameter,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cExec_),
										  boost::ref(cArgument_)));
				cExec_.append(")");
				break;
			

			case Tree::Node::Freetext:		
			case Tree::Node::WordList:		
			case Tree::Node::Section:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				for (int i = 0; i < getSize(); ++i) {
					if (i != 0) {
						cExec_.append(",");
					}

					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cArgument_);
				}
				foreachOption(boost::bind(&Operand::setParameter,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cExec_),
										  boost::ref(cArgument_)));
				cExec_.append(")");
				break;
			case Tree::Node::ExpandSynonym:				
			case Tree::Node::Normalize:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				getOperandi(0)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(" using ");
				foreachOption(boost::bind(&Operand::setParameter,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cExec_),
										  boost::ref(cArgument_)));
				
				cExec_.append(")");
				break;
			case Tree::Node::SubString:
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				getOperandi(0)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(" from ");
				getOptioni(0)->setParameter(cEnvironment_,
											cProgram_,
											pIterator_,
											cExec_,
										    cArgument_);
				if (getOptionSize() > 1) {
					cExec_.append(" for ");
					getOptioni(1)->setParameter(cEnvironment_,
												cProgram_,
												pIterator_,
												cExec_,
												cArgument_);
				}
				cExec_.append(")");
				break;
			case Tree::Node::Case:
			{
				Plan::Sql::QueryArgument cMyArgument(cArgument_);
				cExec_.append(_getOperatorName(getType()));
				for (unsigned int i = 0; i < getOperandSize(); ++i) {
					if (i < getOptionSize()) {
						cExec_.append(" when ");						
						getOptioni(i)->setParameter(cEnvironment_,
													cProgram_,
													pIterator_,
													cExec_,
													cMyArgument);
						cExec_.append(" then ");

					} else {
						cExec_.append(" else ");
					}
					getOperandi(i)->setParameter(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cExec_,
												 cMyArgument);

				}
				cExec_.append(" end ");
				break;
			}
			case Tree::Node::Overlay:
			{
				cExec_.append(_getOperatorName(getType()));
				cExec_.append("(");
				getOperandi(0)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(" placing ");
				getOperandi(1)->setParameter(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cExec_,
											 cArgument_);
				cExec_.append(" from ");
				getOptioni(0)->setParameter(cEnvironment_,
											cProgram_,
											pIterator_,
											cExec_,
										    cArgument_);
				if (getOptionSize() > 1) {
					cExec_.append(" for ");
					getOptioni(1)->setParameter(cEnvironment_,
												cProgram_,
												pIterator_,
												cExec_,
												cArgument_);
				}
				cExec_.append(")");
				break;
			}
			default:
				cExec_.append(toSQLStatement(cEnvironment_,
											 cArgument_));
			}
	   }
	//	virtual void setMetaData(Opt::Environment& cEnvironment_,
	//							 Common::ColumnMetaData& cMetaData_);
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);

		using Super::getType;
		using Super::getSize;
		using Super::getOptioni;
		using Super::getOperandi;
		using Super::foreachOption;
		using Super::getOptionSize;
		using Super::getOperandSize;
		using Super::toSQLStatement;
		using Super::foreachOperand;
		using Super::isAll;
		using Super::isAllOption;
		using Super::isAny;
		using Super::isAnyOption;
		using Super::mapOperand;
		using Super::mapOption;

	protected:
		// generate
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_)
		{
			typename Handle_::MapIntResult cOperandData;
			typename Handle_::OptionMapIntResult cOptionData;
			mapOperand(cOperandData,
					   boost::bind(&Operand::generate,
								   _1,
								   boost::ref(cEnvironment_),
								   boost::ref(cProgram_),
								   pIterator_,
								   boost::ref(cArgument_)));
			mapOption(cOptionData,
					  boost::bind(&Option::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_)));
			pIterator_->addCalculation(cProgram_,
									   Execution::Function::Factory::create(
											cProgram_,
											pIterator_,
											getType(),
											cOperandData,
											cOptionData,
											iDataID_),
									   cArgument_.m_eTarget);
			return iDataID_;
		}
	private:
	};

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::Monadic
	//
	// NOTES
	typedef Base< Tree::Monadic<Function, Interface::IScalar> > Monadic;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::Dyadic
	//
	// NOTES
	typedef Base< Tree::Dyadic<Function, Interface::IScalar> > Dyadic;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::Nadic
	//
	// NOTES
	typedef Base< Tree::Nadic<Function, Interface::IScalar> > Nadic;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::MonadicWithMonadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::MonadicOption<Monadic, Interface::IScalar> > MonadicWithMonadicOption;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::MonadicWithNadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::NadicOption<Monadic, Interface::IScalar> > MonadicWithNadicOption;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::DyadicWithMonadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::MonadicOption<Dyadic, Interface::IScalar> > DyadicWithMonadicOption;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::DyadicWithNadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::NadicOption<Dyadic, Interface::IScalar> > DyadicWithNadicOption;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::NadicWithMonadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::MonadicOption<Nadic, Interface::IScalar> > NadicWithMonadicOption;

	//////////////////////////////////////////////////
	// TYPEDEF
	//	Plan::Scalar::FunctionImpl::NadicWithNadicOption
	//
	// NOTES
	typedef BaseWithOption< Tree::NadicOption<Nadic, Interface::IScalar> > NadicWithNadicOption;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FUNCTIONIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
