//
// ModNlpNpCost.cpp - Implementation of ModNlpNpCost class
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "LibUna/ModNlpNpCost.h"
#include "LibUna/UnicodeChar.h"

_UNA_USING

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpNpCost::ModNlpNpCost -- constructor
//
// NOTES
//	default constructor of ModNlpNpCost
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
ModNlpNpCost::ModNlpNpCost()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpNpCost::ModNlpNpCost -- constructor
//
// NOTES
//	default constructor of ModNlpNpCost
//
// ARGUMENTS
//  ModTerm* modTerm_
//		I:ModTerm pointer
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
ModNlpNpCost::ModNlpNpCost(ModTerm* modTerm_)
:modTerm(modTerm_)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//      ModNlpNpCost::~ModNlpNpCost -- destructor
//
// NOTES
//      default destructor of ModNlpNpCost
//
// ARGUMENTS
//      none
//
// RETURN
//      none
//
// EXCEPTIONS
//      none
//
ModNlpNpCost::~ModNlpNpCost()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpNpCost::calculateNpCost -- Calculation of noun phrase cost
//
// NOTES
//	This function is used to calculate noun phrase cost.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//	ModSize calcCostMode_
//		I:calculation mode of noun phrase cost
//
// RETURN
//	double
//		noun phrase cost
//
// EXCEPTIONS
//		ModCommonErrorBadArgument
//			costVector is null.
//
double
ModNlpNpCost::calculateNpCost(ModVector<int>* cost_, ModSize calcCostMode_)
{
	// parameter.utf8による名詞句コスト算出モードで算出する
	if(cost_) {
		// パラメータの指定により名詞句のコストを算出
		switch (calcCostMode_) {
		case 0:	// QJR
			return calculateNpCostQJR(*cost_);
			break;
		case 1:	// WordFreq:
			return calculateNpCostWordFreq(*cost_);
			break;
		case 2:	// MWordFreq:
			return calculateNpCostMWordFreq(*cost_);
			break;
		default:
			return calculateNpCostWordFreq(*cost_);
			break;
		}
	} else{
		ModErrorMessage << "costVector is null!" << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostQJR -- Calculation of noun phrase cost based on method of QJR
//
// NOTES
//	This function is used to calculate noun phrase cost based on method of QJR.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		cost of noun phrase by QJR method
//
// EXCEPTIONS
//	none
//
double
ModNlpNpCost::calculateNpCostQJR(ModVector<int> cost_)
{
	// それぞれの構成単語に関して出現総計を得る
	double wordFrequencySum = this->wordFrequencySum(cost_);

	// 名詞句の構成数
	ModSize compWordNum = cost_.getSize();

	// QJRによる名詞句コストを算出する
	double npCostQJR = this->getCostWeightConstant(0) * wordFrequencySum / compWordNum +
				 this->getCostWeightConstant(1) * ModOsDriver::Math::log(compWordNum);

	// コストを返す
	return npCostQJR;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostWordFreq -- Calculation of noun phrase cost based on method of WordFreq
//
// NOTES
//	This function is used to calculate noun phrase cost based on method of WordFreq.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		noun phrase cost based on method of WordFreq
//
// EXCEPTIONS
//	none
//
double
ModNlpNpCost::calculateNpCostWordFreq(ModVector<int> cost_)
{
	// それぞれの構成単語に関して出現総計を得る
	double wordFrequencySum = this->wordFrequencySum(cost_);

	// 名詞句の構成数
	ModSize compWordNum = cost_.getSize(); 

	// KWordFreqによる名詞句コストを算出する
	double npCostWordFreq = wordFrequencySum / compWordNum;

	// コストを返す
	return npCostWordFreq;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostMWordFreq -- Calculation of noun phrase cost based on method of MWordFreq
//
// NOTES
//	This function is used to calculate noun phrase cost based on method of MWordFreq.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		noun phrase cost based on method of MWordFreq
//
// EXCEPTIONS
//	none
//
double
ModNlpNpCost::calculateNpCostMWordFreq(ModVector<int> cost_)
{
	// それぞれの構成単語に関して出現総計を得て返す
	double npCostMWordFreq = wordFrequencySum(cost_);
	return npCostMWordFreq;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::wordFrequencySum -- Calculation of the total of the occurrence rate
//
// NOTES
//	This function is used to calculate the total of the occurrence rate 
//	for the composition word of the noun phrase.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		the total of the occurrence rate
//
// EXCEPTIONS
//	none
//
double
ModNlpNpCost::wordFrequencySum(ModVector<int> cost_)
{
	double wordFrequencySum = 0;
	ModVector<int>::Iterator costIte = cost_.begin();
	while(costIte != cost_.end()){
		wordFrequencySum += *costIte;
		*costIte++;
	}

	return wordFrequencySum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::getCostOptionConstant -- get the constant of noun phrases cost
//
// NOTES
//	This function is used to get the constant of noun phrases cost.
//
// ARGUMENTS
//	unsigned int weightOption_
//		Some optional constants are specified
//
// RETURN
//	double
//		the constant of noun phrases cost
//
// EXCEPTIONS
//	値が0か1でない場合
//
double
ModNlpNpCost::getCostWeightConstant(int weightOption_)
{
	double weight;

	if(modTerm){
		if(weightOption_ == 0){
			weight = modTerm->costWeight1;
		} else if(weightOption_ == 1) {
			weight = modTerm->costWeight2;
		} else {
			ModErrorMessage << "Command option is illegal numerical value." << ModEndl;
			ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
		}
	} else {
		weight = 1;
	}

	return weight;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::log10 -- The common logarithm of the argument is returned.
//
// NOTES
//	The common logarithm was made by using the change formula.
//	Because there was no function of the common logarithm
//	though there was a function of the naturalized logarithm in the library of Mod. 
//
// ARGUMENTS
//	double d_
//		value to be calculated
//
// RETURN
//	double
//		calculated result
//
// EXCEPTIONS
double
ModNlpNpCost::log10(double d_)
{
	return ModOsDriver::Math::log(d_) / ModOsDriver::Math::log(10);
}

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
