//
// ModNlpNpCost.h - header file of ModNlpNpCost class
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

#ifndef __UNA_MODNLPNPCOST_H
#define __UNA_MODNLPNPCOST_H

#include "ModUnicodeString.h"
#include "ModVector.h"
#include "LibUna/Module.h"
#include "LibUna/ModTerm.h"

_UNA_BEGIN

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLASS
// ModNlpNpCost - class for calculating noun phrase cost
//
// NOTES
// 名詞句のコストを算出するためのクラス
//
class UNA_LOCAL_FUNCTION ModNlpNpCost {
public:

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
	ModNlpNpCost();

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
	ModNlpNpCost(ModTerm* modTerm_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpNpCost::~ModNlpNpCost -- destructor
//
// NOTES
//	default destructor of ModNlpNpCost
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
	~ModNlpNpCost();

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	ModNlpNpCost::calculateNpCost -- Calculation of noun phrase cost
//
// NOTES
//	This function is used to calculate cost of noun phrase.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//	ModSize calcCostMode_
//		I:calculation mode of cost. default setting is 1.
//
// RETURN
//	double
//		cost of noun phrase
//
// EXCEPTIONS
//	none
//
	double calculateNpCost(ModVector<int>* cost_, ModSize calcCostMode_ = 1);

private:

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostQJR -- Calculation of noun phrase cost based on method of QJR
//
// NOTES
//	This function is used to calculate noun phrase cost  based on method of QJR.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		noun phrase cost based on method of QJR
//
// EXCEPTIONS
//	none
//
	double calculateNpCostQJR(ModVector<int> cost_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostWordFreq -- Calculation of noun phrase cost based on method of KWordFreq
//
// NOTES
//	This function is used to calculate noun phrase cost  based on method of KWordFreq.
//
// ARGUMENTS
//  ModVector<ModUnicodeString> cost_
//		I:cost of morpheme that composes noun phrase
//
// RETURN
//	double
//		noun phrase cost based on method of KWordFreq
//
// EXCEPTIONS
//	none
//
	double calculateNpCostWordFreq(ModVector<int> cost_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::calculateNpCostMWordFreq -- Calculation of noun phrase cost based on method of MWordFreq
//
// NOTES
//	This function is used to calculate noun phrase cost  based on method of MWordFreq.
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
	double calculateNpCostMWordFreq(ModVector<int> cost_);

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
	double wordFrequencySum(ModVector<int> cost_);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//	ModNlpNpCost::getCostWeightConstant -- get the constant of noun phrases cost
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
//	値が数値でない場合、引数 _option がオプションの個数を超えている場合
//
	double getCostWeightConstant(int weightOption_);

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
//	double d
//		value to be calculated
//
// RETURN
//	double
//		calculated result
//
// EXCEPTIONS
	double log10(double d);

	// ModTerm
	ModTerm* modTerm;
};

_UNA_END

#endif // __ModNlpNpCost_H__

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
