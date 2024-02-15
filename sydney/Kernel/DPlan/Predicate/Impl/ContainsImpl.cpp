// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ContainsImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Predicate::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Common/Message.h"

#include "DPlan/Predicate/Impl/ContainsImpl.h"
#include "DPlan/Candidate/WordExtraction.h"
#include "DPlan/Candidate/Grouping.h"
#include "DPlan/Candidate/Table.h"
#include "DPlan/Scalar/Aggregation.h"
#include "DPlan/Scalar/Field.h"



#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Sort.h"
#include "Plan/File/KwicOption.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/Key.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Aggregation.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Sql/Argument.h"
#include "Plan/Sql/Node.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"
#include "Common/Data.h"

#include "DExecution/Action/Fulltext.h"
#include "DExecution/Action/StatementConstruction.h"
#include "DExecution/Collection/WordExtraction.h"
#include "DExecution/Collection/Sifter.h"
#include "DExecution/Iterator/Expand.h"
#include "DExecution/Predicate/HasNextCandidate.h"

#include "Exception/FullTextIndexNeeded.h"
#include "Exception/NotSupported.h"
#include "Exception/PrepareFailed.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Store.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Operator/Clear.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Output.h"
#include "Execution/Operator/SetNull.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"




#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Key.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " contains ";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::ContainsImpl

// FUNCTION public
//	Predicate::Impl::ContainsImpl::createKwicOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pKwicSize_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
createKwicOption(Opt::Environment& cEnvironment_,
				 Plan::Interface::IScalar* pKwicSize_)
{
	if (m_pKwicOption == 0) {
		// in preparation, kwicoption with placeholder can't be adopted here
		if (cEnvironment_.isPrepare()
			&& pKwicSize_->hasParameter()) {
			_SYDNEY_THROW0(Exception::PrepareFailed);
		}
		m_pKwicOption = Plan::File::KwicOption::create(cEnvironment_);
		m_pKwicOption->setKwicSize(pKwicSize_);
	}	
}
// FUNCTION public
//	Predicate::Impl::ContainsImpl::createDistributePlan-- 重要語句抽出するCandidateを生成する
// 	
//		
// NOTES
//	
//	
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
// 	Interface::ICandidate* pOperand_
//
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
Impl::ContainsImpl::
createDistributePlan(Opt::Environment& cEnvironment_,
					 Plan::Interface::ICandidate* pOperand_,
					 Plan::Utility::FieldSet& cFieldSet_)
{
	Plan::Utility::FieldSet::ConstIterator ite = cFieldSet_.begin();
	for (;ite != cFieldSet_.end(); ++ite) {
		if ((*ite)->isFunction()
			&& (*ite)->getFunction()->getOperandSize() == 1
			&& (*ite)->getFunction()->getOperandAt(0) == getOperand0()) {

			if ((*ite)->getFunction()->getType() == Plan::Tree::Node::Word) {
				return DPlan::Candidate::WordExtraction::create(cEnvironment_,
																pOperand_,
																this,
																false);
			} else if ((*ite)->getFunction()->getType() == Plan::Tree::Node::Score) {
				return DPlan::Candidate::WordExtraction::create(cEnvironment_,
																pOperand_,
																this,
																true);
																
			}
		}
	}
	
	return pOperand_;
}




// FUNCTION public
//	Predicate::Impl::ContainsImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand0()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName);
	cExplain_.popNoNewLine();
	getOperand1()->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::ContainsImpl::createSQL -- 
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::ContainsImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	cStream << _pszOperatorName;
	cStream << getOperand1()->toSQLStatement(cEnvironment_, cArgument_);
	if (m_pExpand) {
		cStream << "expand (from (";
		cStream << m_pExpand->generateSQL(cEnvironment_)->toSQLStatement(cEnvironment_, cArgument_);
		cStream << "))";
	}
	if (m_pRankFrom) {
		cStream << "rank from (";
		cStream << m_pRankFrom->generateSQL(cEnvironment_)->toSQLStatement(cEnvironment_, cArgument_);
		cStream << ")";
	}
	return cStream.getString();
}



// FUNCTION public
//	Predicate::Impl::ContainsImpl::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand0()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
	cExec_.append(_pszOperatorName);

	
	if (m_pAdoptResult) {
		if (m_iOutData == -1) { // ノードが共有された場合複数回呼ばれるため。
			const_cast<ContainsImpl*>(this)->m_iOutData =
				cProgram_.addVariable(Common::DataInstance::create(Common::DataType::String));
			VECTOR<int> vecData;
			vecData.pushBack(m_iOutData);
			m_pAdoptResult->addAction(cProgram_,
									  _ACTION_ARGUMENT1(OutData,
														cProgram_.addVariable(vecData)));
			Execution::Interface::IAction* pAction = 
				Execution::Operator::Iterate::Once::create(cProgram_,
														   pIterator_,
														   m_pAdoptResult->getID());
			cExec_.append(m_iOutData, pAction);
		} else {
			cExec_.append(m_iOutData, 0);
		}
	} else {
		getOperand1()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
		if (getExpand()) {
			cExec_.append( " expand (from (");
			getExpand()->generateSQL(cEnvironment_)->setParameter(cEnvironment_,
																  cProgram_,
																  pIterator_,
																  cExec_,
																  cArgument_);
			cExec_.append( ")) ");
		}
	}
	
	if (m_pRankFrom) {
		cExec_.append( "rank from (");
		cExec_.append(m_pRankFrom->generateSQL(cEnvironment_)->toSQLStatement(cEnvironment_, cArgument_));
		cExec_.append( ")");
	}

	for (int i = 0; i < Super::getOptionSize(); ++i) {
		const Plan::Interface::IScalar* pOption =
			_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*, Super::getOptionAt(i));
		const_cast<Plan::Interface::IScalar*>(pOption)->setParameter(cEnvironment_,
																	 cProgram_,
																	 pIterator_,
																	 cExec_,
																	 cArgument_);
	}
}

// FUNCTION public
//	Predicate::Impl::ContainsImpl::getOptionSize -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
Impl::ContainsImpl::
getOptionSize() const
{
	return Super::getOptionSize()
		+ (m_pKwicOption ? 1 : 0)
		+ (m_pExpand ? 1 : 0);

	// m_pRankFrom is not needed for option
 }
 
// FUNCTION public
//	Predicate::Impl::ContainsImpl::getOptionAt -- 
//
// NOTES
//
// ARGUMENTS
//	ModInt32 iPosition_
//	
// RETURN
//	const Tree::Node::Super*
//
// EXCEPTIONS

//virtual
const Plan::Tree::Node::Super*
Impl::ContainsImpl::
getOptionAt(ModInt32 iPosition_) const
{
	ModInt32 n = Super::getOptionSize();
	if (iPosition_ < n) {
		return Super::getOptionAt(iPosition_);
	}
	if (m_pKwicOption) ++n;
	if (iPosition_ < n) {
		return syd_reinterpret_cast<const Plan::Tree::Node::Super*>(m_pKwicOption->getKwicSize());
	}
	if (m_pExpand) ++n;
	if (iPosition_ < n) {
		return syd_reinterpret_cast<const Plan::Tree::Node::Super*>(m_pExpand);
	}

	// m_pRankFrom is not needed for option
	return 0;
}





// FUNCTION public
//	Predicate::Impl::ContainsImpl::getFulltextIndex -- 全文索引ファイルを取得する
//
// NOTES
//	左辺のカラムに関連付けられた全文索引ファイルのファイルIDを取得する.
//	
// ARGUMENTS
//	Opt::Environment cEnvironment_
//	
// RETURN
//	LogicalFile::FileID&
//
// EXCEPTIONS
//	FullTextIndexNeeded 全文索引ファイルが存在しない場合
//
//virtual
Schema::File*
Impl::ContainsImpl::
getFulltextIndex(Opt::Environment& cEnvironment_)
{
	if (getOperand0()->isField()
		&& getOperand0()->getField()->isColumn()) {
		
		ModVector<Schema::Field*> cvecField;
		getOperand0()->getField()->getSchemaColumn()->
			getIndexField(cEnvironment_.getTransaction(), cvecField);
		
		ModVector<Schema::Field*>::ConstIterator ite = cvecField.begin();
		ModVector<Schema::Field*>::ConstIterator end = cvecField.end();
		Schema::File* pResult;
		for (; ite != end; ite++) {
			pResult = (*ite)->getFile(cEnvironment_.getTransaction());
			if (pResult->getCategory() == Schema::File::Category::FullText) {
				break;
			}
		}
		if (ite == end) 
			_SYDNEY_THROW0(Exception::FullTextIndexNeeded);
		
		return pResult;
	} else {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}


// FUNCTION public
//	Predicate::Impl::ContainsImpl::getColumn -- 条件が指定されているカラムを返す
//
// NOTES
//	
//	
// ARGUMENTS
//	
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS
//	
//
//virtual
Plan::Interface::IScalar*
Impl::ContainsImpl::
getColumn()
{
	return getOperand0();

}



// FUNCTION public
//	Candidate::WordExtraction::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
Impl::ContainsImpl::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	if (m_pAdoptResult) return m_pAdoptResult;
	
	// start upで文書の総件数を求める
	Plan::Order::Specification* pSortSpecification =
		Plan::Order::Specification::create(cEnvironment_, VECTOR<Plan::Order::Key*>(), false);
	Plan::Interface::IScalar* pOperand = getOperand0();
	

	OSTRSTREAM cCountStream;
	Plan::Sql::QueryArgument cQueryArg;
	cCountStream << "count(" << pOperand->toSQLStatement(cEnvironment_, cQueryArg) << ")";
	cCountStream << "count(*)";
	Plan::Interface::IScalar* pCount =
		Plan::Scalar::Function::create(cEnvironment_,
									   Plan::Tree::Node::Count,
									   pOperand,
									   cCountStream.getString());
	cCountStream.clear();
	cCountStream << "fulltext_length(" << pOperand->toSQLStatement(cEnvironment_, cQueryArg) << ")";
	Plan::Interface::IScalar* pLength =
		Plan::Scalar::Function::create(cEnvironment_,
									   Plan::Tree::Node::FullTextLength,
									   pOperand,
									   cCountStream.getString());
	cCountStream.clear();
	cCountStream << "avg(" << pLength->toSQLStatement(cEnvironment_, cQueryArg) << ")";
	Plan::Interface::IScalar* pAvg =
		Plan::Scalar::Function::create(cEnvironment_,
									   Plan::Tree::Node::Avg,
									   pLength,
									   cCountStream.getString());

	VECTOR<Plan::Interface::IScalar*> vecAggregation;
	vecAggregation.pushBack(pCount);
	vecAggregation.pushBack(pAvg);

	Plan::Utility::ScalarSet cAggregationOperand;
	const Plan::Interface::IScalar* pLengthOperand =
		_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*,
							 pAvg->getOperandAt(0)->getOperandAt(0));
	cAggregationOperand.add(const_cast<Plan::Interface::IScalar*>(pLengthOperand));
	
	Plan::Utility::RelationSet cRelationSet;
	pOperand->getUsedTable(cRelationSet);
	if (cRelationSet.getSize() != 1)
		_SYDNEY_THROW0(Exception::NotSupported);
	

	int iCountID = -1;
	int iAvgID = -1;
	Execution::Interface::IIterator* pCountIterator = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
		Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
		Plan::Interface::ICandidate* pTable =
			DPlan::Candidate::Table::Distribute::Retrieve::create(cEnvironment_, *cRelationSet.begin());
		Plan::Interface::ICandidate* pGrouping =
			DPlan::Candidate::Grouping::Simple::create(cEnvironment_,
													   vecAggregation,
													   cAggregationOperand,
													   pTable);

		cMyArgument.m_pQuery = pGrouping->generateSQL(cEnvironment_);
		pCount->retrieveFromCascade(cEnvironment_, cMyArgument.m_pQuery);
		pAvg->retrieveFromCascade(cEnvironment_, cMyArgument.m_pQuery);
		cMyArgument.m_pInput = 0;
		cMyArgument.m_pCascade = 0;
		pCountIterator = pGrouping->adopt(cEnvironment_, cProgram_, cMyArgument);
		iCountID = pCount->generate(cEnvironment_, cProgram_, pCountIterator, cMyArgument);
		iAvgID = pAvg->generate(cEnvironment_, cProgram_, pCountIterator, cMyArgument);
	}

	
	// pResultはExpandIteratorをcreateする
	DExecution::Action::Fulltext* pFulltext =
		DExecution::Action::Fulltext::create(cEnvironment_,
											 cProgram_,
											 getFulltextIndex(cEnvironment_)->getFileID(),
											 getOperand1());

	// Limitをセットする
	if (cArgument_.m_cLimit.isSpecified()) {
			Common::IntegerArrayData cValue;
		cArgument_.m_cLimit.getValue(cEnvironment_, cValue);
		pFulltext->setLimit(cValue.getElement(0));
	}	


	// extractorとExpandLimitをセットする
	for (int i = 0; i < static_cast<int>(getOptionSize()); ++i) {
		switch (getOptionAt(i)->getType()) {
		case Plan::Tree::Node::Extractor:
			pFulltext->setExtractor(getOptionAt(i)->getValue());
			break;
		case Plan::Tree::Node::Expand:
			for (int j = 0; j < static_cast<int>(getOptionAt(i)->getOptionSize()); ++j) {
				const LogicalFile::TreeNodeInterface* pOption = getOptionAt(i)->getOptionAt(j);
				if (pOption->getType() == Plan::Tree::Node::Limit) {
					pFulltext->setRelatedLimit(pOption->getData()->getUnsignedInt());
					break;
				}
			}
			break;
		case Plan::Tree::Node::Calculator:
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	
	
	// expand文を投げて結果から重要語句抽出する
	// 現状ここでのoperand1はFreeTextのみ．
	// score計算のためのDFは別途検討予定
	DExecution::Collection::WordExtraction* pExtraction =
		DExecution::Collection::WordExtraction::create(cProgram_,
													   pFulltext->getID());

	Execution::Interface::IIterator* pResult =
		DExecution::Iterator::Expand::create(cProgram_,
											 pExtraction->getID(),
											 pFulltext->getID(),
											 iCountID,
											 iAvgID,
											 getExpand() == 0);

	pResult->addCalculation(cProgram_,
							Execution::Operator::Iterate::All::create(cProgram_,
																	  pResult,
																	  pCountIterator->getID(),
																	  true /* no undone */),
							Execution::Action::Argument::Target::StartUp);

	pResult = adoptExpand(cEnvironment_, cProgram_, cArgument_, pResult);
	
	

	// 抽出した単語をセットする領域を生成する
	int iTermDataID =
		cProgram_.addVariable(Common::DataInstance::create(Common::DataType::String));

	VECTOR<int> vecData;
	vecData.pushBack(iTermDataID);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 cProgram_.addVariable(vecData)));
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));
	vecData.clear();
	if (getOperand1()->getType() == Plan::Tree::Node::Freetext
		|| getOperand1()->getType() == Plan::Tree::Node::WordList) {
		pCountIterator = addDFCountIteratorByWordList(cEnvironment_,
													  cProgram_,
													  cArgument_,
													  pResult,
													  iTermDataID,
													  pFulltext->getID(),
													  vecData);
	} else {
		pCountIterator = addDFCountIterator(cEnvironment_,
											cProgram_,
											cArgument_,
											pResult,
											iTermDataID,
											pFulltext->getID(),
											vecData);
	}

	// DFとWordをCollectionにためて、pFulltextから重要語句抽出をするFilterをかぶせる
	// "ふるい"にかけて重要語句を取捨選択する
	Execution::Interface::ICollection* pSifter =
		DExecution::Collection::Sifter::create(cProgram_, pFulltext->getID());
	int iPutDataID = cProgram_.addVariable(vecData);
	pCountIterator->addCalculation(cProgram_,
								   Execution::Operator::Output::create(cProgram_,
																	   pCountIterator,
																	   pSifter->getID(),
																	   iPutDataID));

	m_pAdoptResult = Execution::Iterator::Input::create(cProgram_,
														pSifter->getID());
	m_pAdoptResult->addCalculation(cProgram_,
								   Execution::Operator::Iterate::All::create(cProgram_,
																			 m_pAdoptResult,
																			 pResult->getID()),
								   Execution::Action::Argument::Target::StartUp);

	return m_pAdoptResult;
}


// FUNCTION private
//	Predicate::Impl::ContainsImpl::adoptExpand -- 
//
// NOTES
//	Expandの実行結果をpIterator_のInputにセットする
// 
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
Impl::ContainsImpl::
adoptExpand(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Plan::Candidate::AdoptArgument& cArgument_,
			Execution::Interface::IIterator* pIterator_)
{
	// Expand文の実行結果をCollection::WordExtractionのInputにセットする

	if (getExpand() != 0) {
		Plan::AccessPlan::Source cPlanSource;
		Plan::Interface::ICandidate* pCandidate =
			getExpand()->createAccessPlan(cEnvironment_, cPlanSource);
		Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
		cMyArgument.m_pInput = 0;
		cMyArgument.m_pCascade = 0;
		cMyArgument.m_pQuery = pCandidate->generateSQL(cEnvironment_);
		Execution::Interface::IIterator* pExpandIterator =
			pCandidate->adopt(cEnvironment_, cProgram_, cMyArgument);
		
		
		int iExpandDataID =
			getExpand()->getRowInfo(cEnvironment_)->generate(cEnvironment_,
															 cProgram_,
															 pExpandIterator,
															 cMyArgument);
		Execution::Interface::ICollection* pCollection =
			Execution::Collection::Store::create(cProgram_);
		pExpandIterator->addCalculation(cProgram_,
										Execution::Operator::Output::create(cProgram_,
																			pExpandIterator,
																			pCollection->getID(),
																			iExpandDataID));
		Execution::Interface::IIterator* pInput =
			Execution::Iterator::Input::create(cProgram_);
		
		pInput->addCalculation(cProgram_,
							   Execution::Operator::Iterate::All::create(cProgram_,
																		 pInput,
																		 pExpandIterator->getID(),
																		 true),
							   Plan::Candidate::AdoptArgument::Target::StartUp);
		
		int iPutDataID = getExpand()->
			getRowInfo(cEnvironment_)->generateFromType(cEnvironment_,
														cProgram_,
														pInput,
														cMyArgument);
		pInput->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pCollection->getID(),
											iPutDataID));
		
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT(Input,
											   pInput->getID(),
											   iPutDataID));
	}
	return pIterator_;
}

// FUNCTION private
//	Predicate::Impl::ContainsImpl::adoptExpand -- 
//
// NOTES
//	Expandの実行結果をpIterator_のInputにセットする
// 
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_,
//	Plan::Candidate::AdoptArgument& cArgument_,
//	Execution::Interface::IIterator* pIterator_,
//	int iTermDataID_,
//	int iFulltextID_,
//	VECTOR<int>& vecData
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
Impl::ContainsImpl::
addDFCountIterator(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Plan::Candidate::AdoptArgument& cArgument_,
				   Execution::Interface::IIterator* pIterator_,
				   int iTermDataID_,
				   int iFulltextID_,
				   VECTOR<int>& vecData)
{
	VECTOR<Plan::Interface::IScalar*> vecAggregation;
	Plan::Interface::IScalar* pCountOperand =
		Plan::Scalar::Value::create(cEnvironment_,
									new Common::StringData(STRING("*")));

	// Plan::Scalar::Functionを使用すると分散判定されないので、直接DPlanを呼ぶ
	Plan::Interface::IScalar* pCount =
		DPlan::Scalar::Aggregation::create(cEnvironment_,
										   Plan::Tree::Node::Count,
										   pCountOperand,
										   "");
	vecAggregation.pushBack(pCount);
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	Plan::Utility::RelationSet cRelationSet;
	getOperand0()->getUsedTable(cRelationSet);
	Plan::Interface::ICandidate* pTable =
		DPlan::Candidate::Table::Distribute::Retrieve::create(cEnvironment_, *cRelationSet.begin());

	cMyArgument.m_pInput = 0;
	cMyArgument.m_pCascade = 0;
	Plan::Utility::ScalarSet cAggregationOperand;
	Plan::Interface::ICandidate* pGrouping =
		DPlan::Candidate::Grouping::Simple::create(cEnvironment_,
												   vecAggregation,
												   cAggregationOperand,
												   pTable);
	Plan::Sql::Query* pQuery = pGrouping->generateSQL(cEnvironment_);
	pCount->retrieveFromCascade(cEnvironment_, pQuery);
	OSTRSTREAM cStream;
	cStream << getOperand0()->getName() << " contains ";
	STRING cSql(cStream.getString());
	pQuery->setPredicate(Plan::Interface::ISqlNode::createSimpleNode(cEnvironment_, cSql, iTermDataID_));
	pQuery->setIterable();

	// SQL文を生成するアクションを追加するIteratorを指定
	cArgument_.m_pInput = pIterator_;
	cArgument_.m_pCascade = 0;
	// SQL文をConcatinateするGeneratorを生成する
	pQuery->addSqlGenerator(cEnvironment_, cProgram_, pIterator_, true);
	
	int iCheckID =
		DExecution::Predicate::HasNextCandidate::create(cProgram_, pIterator_, iFulltextID_)->getID();
	// 最後の単語の抽出が完了時にまとめてSQL文を送る.
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  iCheckID,
											  Execution::Action::Argument::Target::Execution));
	
	cMyArgument.m_pQuery = pQuery;
	cMyArgument.m_bConcatinateIterable = true;
	Execution::Interface::IIterator* pCountIterator =
		pGrouping->adopt(cEnvironment_, cProgram_, cMyArgument);

	pIterator_->addCalculation(cProgram_,
						   Execution::Operator::Iterate::RuntimeStartup::create(cProgram_,
																				pIterator_,
																				pCountIterator->getID()));
	pIterator_->addCalculation(cProgram_,
							Execution::Operator::SetNull::create(cProgram_, pIterator_, pQuery->getSqlID(cProgram_)));
														 
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  Execution::Action::Argument::Target::Execution));
	
	int iCountID = pCount->generate(cEnvironment_,
									cProgram_,
									pCountIterator,
									cArgument_);
	vecData.pushBack(iCountID);
	return pCountIterator;
}

// FUNCTION private
//	Predicate::Impl::ContainsImpl::adoptExpand -- 
//
// NOTES
//	Expandの実行結果をpIterator_のInputにセットする
// 
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_,
//	Plan::Candidate::AdoptArgument& cArgument_,
//	Execution::Interface::IIterator* pIterator_,
//	int iTermDataID_,
//	int iFulltextID_,
//	VECTOR<int>& vecData
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual

Execution::Interface::IIterator*
Impl::ContainsImpl::
addDFCountIteratorByWordList(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Plan::Candidate::AdoptArgument& cArgument_,
							 Execution::Interface::IIterator* pIterator_,
							 int iTermDataID_,
							 int iFulltextID_,
							 VECTOR<int>& vecData)
{
	OSTRSTREAM cStream;
	cStream << "word(" << getOperand0()->getName() << ")";
	VECTOR<Plan::Interface::IScalar*> vecAggregation;
	Plan::Interface::IScalar* pFunction =
		Plan::Scalar::Function::create(cEnvironment_,
									   Plan::Tree::Node::Word,
									   getOperand0(),
									   cStream.getString());
	cStream.clear();
	Plan::Utility::RelationSet cRelationSet;
	getOperand0()->getUsedTable(cRelationSet);
	if (cRelationSet.getSize() != 1
		|| (*cRelationSet.begin())->getType() != Plan::Tree::Node::Table ) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Plan::Interface::IScalar* pWord =
		Scalar::Field::create(cEnvironment_,
							  _SYDNEY_DYNAMIC_CAST(Plan::Relation::Table*, *cRelationSet.begin()),
							  pFunction);
	Plan::Interface::IScalar* pWordAggregation = 
		Scalar::Aggregation::create(cEnvironment_,
									Plan::Tree::Node::Word,
									pWord,
									"Word");
	int iWordListDataID =
		cProgram_.addVariable(Common::DataInstance::create(Common::DataType::Array));
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Output::Array::create(cProgram_,
																		  pIterator_,
																		  iWordListDataID,
																		  iTermDataID_));
	
	vecAggregation.pushBack(pWordAggregation);
	Plan::Order::Key* pKey =
		Plan::Order::Key::create(cEnvironment_,
								 pWord,
								 Plan::Order::Direction::Ascending);
	
	Plan::Order::Specification* pSortSpecification =
		Plan::Order::Specification::create(cEnvironment_, pKey);
	Plan::Interface::ICandidate* pTable =
		DPlan::Candidate::Table::Distribute::Retrieve::create(cEnvironment_, *cRelationSet.begin());
	Plan::Interface::ICandidate* pSort =
		Plan::Candidate::Sort::create(cEnvironment_, pSortSpecification, pTable);
	
	Plan::Utility::ScalarSet cAggregationOperand;
	Plan::Interface::ICandidate* pGrouping =
		DPlan::Candidate::Grouping::Normal::create(cEnvironment_,
												   pSortSpecification,
												   vecAggregation,
												   cAggregationOperand,
												   pSort);

	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pInput = 0;
	cMyArgument.m_pCascade = 0;
	Plan::Sql::Query* pQuery = pGrouping->generateSQL(cEnvironment_);
	pWordAggregation->retrieveFromCascade(cEnvironment_, pQuery);
	
	
	Plan::Sql::Node* pPredicate = Plan::Sql::Node::createArrayNode(cEnvironment_);

	cStream << getOperand0()->getName() << " contains wordlist (";
	pPredicate->appendNode(Plan::Sql::Node::createSimpleNode(cEnvironment_, STRING(cStream.getString())));
	pPredicate->appendNode(Plan::Sql::Node::createArrayPlaceHolderNode(cEnvironment_, iWordListDataID));
	pPredicate->appendNode(Plan::Sql::Node::createSimpleNode(cEnvironment_, ")"));
	pQuery->setPredicate(pPredicate);
		
	int iCheckID =
		DExecution::Predicate::HasNextCandidate::create(cProgram_, pIterator_, iFulltextID_)->getID();
	// 最後の単語の抽出が完了時にまとめてSQL文を送る.
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  iCheckID,
											  Execution::Action::Argument::Target::Execution));
	
	cMyArgument.m_pQuery = pQuery;
	Execution::Interface::IIterator* pCountIterator =
		pGrouping->adopt(cEnvironment_, cProgram_, cMyArgument);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Iterate::RuntimeStartup::create(cProgram_,
																					pIterator_,
																					pCountIterator->getID()));
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Clear::Array::create(cProgram_,
																		 pIterator_,
																		 pQuery->getSqlID(cProgram_)));

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Clear::create(cProgram_,
																  pIterator_,
																  iWordListDataID));

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  Execution::Action::Argument::Target::Execution));
	
	int iWordID = pWordAggregation->generate(cEnvironment_,
											 cProgram_,
											 pCountIterator,
											 cArgument_);
	vecData.pushBack(iWordID);
	return pCountIterator;
}





// FUNCTION private
//	Predicate::Impl::ContainsImpl::isNeedWordExtraction -- 
//
// NOTES
//	score,wordが必要な場合は、単語抽出の必要がある
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
Impl::ContainsImpl::
isNeedWordExtraction(const Plan::Utility::FieldSet& cFieldSet_)
{
	Plan::Utility::FieldSet::ConstIterator ite = cFieldSet_.begin();
	for (;ite != cFieldSet_.end(); ++ite) {
		if ((*ite)->isFunction()
			&& ((*ite)->getFunction()->getType() == Plan::Tree::Node::Word
				|| (*ite)->getFunction()->getType() == Plan::Tree::Node::Score)) {
			return true;
		}
	}
	return false;
}



 // FUNCTION private
//	Predicate::Impl::ContainsImpl::addToEnvironment -- 
 //
 // NOTES
 //
 // ARGUMENTS
 //	Opt::Environment& cEnvironment_

 // EXCEPTIONS

//virtual
void
Impl::ContainsImpl::
addToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addContains(getOperand0(), this);
}
 

_SYDNEY_DPLAN_PREDICATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
