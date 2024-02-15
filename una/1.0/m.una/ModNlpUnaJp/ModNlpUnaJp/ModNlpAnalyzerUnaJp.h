// 
// Copyright (c) 2005, 2010, 2023 Ricoh Company, Ltd.
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
#ifndef __UNA_MODNLPANALYZERUNAJP_H
#define __UNA_MODNLPANALYZERUNAJP_H

#include "ModNLPLocal.h"
#include "ModNLPText.h"
#include "ModCriticalSection.h"
#include "Module.h"
#include "ModNLPX.h"
#include "LibUna/ModTerm.h"

_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModNlpResourceUnaJp;
class ModNlpAnalyzerX;

//
// CLASS
//	ModNlpAnalyzerUnaJp -- Japanese morph analyzer
//
// NOTES
//	This class provides old Japanese UNA function.
//	It's necessary to export for ModNlpAnalyzerUnaJp.
//
class ModNlpAnalyzerUnaJp: public ModNlpLocalAnalyzer
{
public:

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::ModNlpAnalyzerUnaJp -- constructor
//
// NOTES
//	default constructor of ModNlpAnalyzerUnaJp
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
/*!
	Class constructor.
*/
	ModNlpAnalyzerUnaJp();
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::~ModNlpAnalyzerUnaJp -- destructor
//
// NOTES
//	destructor of ModNlpAnalyzerUnaJp
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
/*!
	Class destructor.
*/
	~ModNlpAnalyzerUnaJp();
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::setResource -- set resource
//
// NOTES
//	set resource
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalResource*
//		the address of ModNlpLocalResource
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Set resource for analyzer.
	@param[in]	pointer to instance of ModNlpLocalResource
	\return	whether set successfully
*/
	ModBoolean setResource(ModNlpLocalResource*);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::releaseResource -- decrease reference counter if it is needed
//
// NOTES
//	decrease reference counter if it is needed
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Release resource.
	\return whether resource is released successfully.
*/
	ModBoolean releaseResource();
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::prepare -- set language and parameters
//
// NOTES
//	set language and parameters
//
// ARGUMENTS
//	const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >&
//		I: parameters
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Set language and parameters.
	@param[in]	lang		language setting
	@param[in]	parametersToModifyAnalyze		parameters for processing
	\return whether set successfully.
*/
	ModBoolean prepare( const Parameters& parametersToModifyAnalyze);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::set -- set a text to be analyzed
//
// NOTES
//	set a whole text to be analyzed
//
// ARGUMENTS
//	const ModUnicodeString&
//		I: a text to be analyzed
//	const ModLanguageSet&
//		I: language
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Set target document.
	@param[in]	target	target document for analyzing
	\return whether set successfully.
*/
	ModBoolean set(const ModUnicodeString& target,
				 const ModLanguageSet& lang);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::isExpStrDataLoad
//	-- It is confirmed whether the data of the expanding function set by the parameter is loaded.
//
// NOTES
//	This function is confirmed whether the expanding data set by the parameter was loaded.
//	It is executed before the acquisition function of the expanded strings result
//	is called.
//	If the return value is ModTrue, the acquisition function is called.
//
// ARGUMENTS
//	It is not.
//
// RETURN
//	ModBoolean
//		ModTrue		The data of the expanding function set by the parameter was loaded.
//		ModFalse	The data of the expanding function set by the parameter was not loaded.
//
// EXCEPTIONS
//
	ModBoolean isExpStrDataLoad();

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getAbility -- return ability
//
// NOTES
//	return ability
//
// ARGUMENTS
//	funcName
//		I: Name of function
//
// RETURN
//	ModSize
//		4: prefered ( will modify all return abilities to 4)
//		3: perfect ability
//		2: percial ability
//		1: no ability and default processing
//		0: no ability and no processing
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Get ability of current analyzer.
	@param[in]	nameOfFunc	name of the getXXX functions.
	\return 4  prefered ( will modify all return abilities to 4)
	 return 3  perfect ability
	 return 2  percial ability
	 return 1  no ability and default processing
	 return 0  no ability and no processing
*/
	ModSize getAbility(const ModLanguageSet& lang);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getWord -- get a word from document in turn
//
// NOTES
//	get a word from document in turn
//
// ARGUMENTS
//	ModUnicodeString& normalized
//		O: normalized word
//	ModUnicodeString& original
//		O: original word
//	int& pos
//		O: part of speech
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get a word from document in turn.
	@param[out]	normalized	normalized word
	@param[out]	original	original word
	@param[out]	part of speech
	\return true if more words still remain.
					return false if no more words.
*/
	ModBoolean getWord(ModUnicodeString& normalized, ModUnicodeString* original, int* pos);


//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getExpandWords -- get expanded words from document in turn
//
// NOTES
//	get expanded words from document in turn
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& expanded
//		O: expanded word
//	ModUnicodeString& original
//		O: original word
//	int& pos
//		O: part of speech
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get expanded words from document in turn.
	@param[out]	expanded  expanded word
	@param[out]	original	original word
	@param[out]	pos	whole  part of speech
	\return true if more words still remain,
					return false if there is no more words.
*/
	ModBoolean getExpandWords(ModVector<ModUnicodeString>& expanded, ModUnicodeString& original, int& pos);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getBlock -- get words from document together
//
// NOTES
//	get words from document together
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& normVector
//		O: vector including normalized words
//	ModVector<ModUnicodeString>& origVector
//		O: vector including original words
//	ModVector<int>& posvector
//		O: vector including part of speeches
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get words from document together
	@param[out]	normVector  vector including normalized words
	@param[out]	posvector	vector including part of speeches
	@param[out]	origVector	vector including original words
	@param[out]	costVector	vector including cost
	@param[out]	uposVector	vector including unified part of speech
	@param[out]	ignore		ModTrue: the character string deleted by regularization
									 is disregarded. Default setting.
					  	    ModFalse:it doesn't disregard it.
	\return true if more words still remain,
					return false if there is no more words.
*/
	ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
  							ModVector<int>& posvector,
  							ModVector<ModUnicodeString>* origVector,
  							ModVector<int>* costVector = 0,
							ModVector<int>* uposVector = 0,
							ModBoolean ignore = ModTrue);

	ModBoolean getBlock(ModUnicodeString& result, ModUnicodeChar sep1, ModUnicodeChar sep2);

	ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
										ModVector<ModUnicodeString>& hinVector);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getDicName -- get dicionary base name for each morpheme
//
// NOTES
// 	get dictionary base name for each morpheme which getBlock() returned
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& dicNameVector_
//		O: vector including dictionary base names
//
// RETURN
//	ModBoolean
//		ModTrue:  result found
//		ModFalse: no result found
//
// EXCEPTIONS
//
	ModBoolean getDicName(ModVector<ModUnicodeString>& dicNameVector_);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::identify -- identify in which language the document is written
//
// NOTES
//	****************** Don't use this method cullentry **********************
//
// ARGUMENTS
//	ModSize endPos
//		I: end position of target document to be used to identify
//
// RETURN
//	ModLanguageCode
//		ModLanguageCode::undefined : unable to identify
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	identify in which language the document is written.
	**** Don't use this method cullentry ****
	@param[in] endPos  end position of target document to be used to identify
	\return ModLanguageCode
					ModLanguageCode::undefined : unable to identify
*/
	ModLanguageCode identify(ModSize endPos);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getWholeText -- get whole text
//
// NOTES
//	If NormalizeOnly parameter is specified inparameter,
//	this method returns normalized text of ModNormalizer::normalizeBuf.
//	else if GetOrg parameter is specified in parameter,
//	this method returns original text.
//	else
//	this method returns normalized text created by joining words got from getWord.
//
// ARGUMENTS
//	ModUnicodeString& wholeText
//		O: whole text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get whole text.
	Notes: If NormalizeOnly parameter is specified inparameter,
	this method returns normalized text of ModNormalizer::normalizeBuf.
	else if GetOrg parameter is specified in parameter,
	this method returns original text.
	else this method returns normalized text created by joining words got from getWord.
	@param[out] wholeText  whole text
	\return ModTrue:  succeed in getting text
					ModFalse:  no more text
*/
	ModBoolean getWholeText(ModUnicodeString& wholeText);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getNormalizeBuf -- get normalized text
//
// NOTES
//	get normalized text of ModNormalizer::normalizeBuf.
//
// ARGUMENTS
//  ModUnicodeString& normalized
//		O: normalized text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
	ModBoolean getNormalizeBuf(ModUnicodeString& normalized);
//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getExpandBuf -- get expanded text
//
// NOTES
//	get expanded text of ModNormalizer::expandBuf.
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& expanded
//		O: expanded text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
	ModBoolean getExpandBuf(ModVector<ModUnicodeString>& expanded);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getExpandStrings -- get expanded strings patterns
//
// NOTES
//  This function is used to get expanded strings patterns of ModNlpExpStr::expandStrings
//  only in the ModNlpUnaJp module.
//
// ARGUMENTS
//	ExpandResult& expandResult
//		O:	status of acquisition result of expanded string patterns
//			 0:	no expanded strings patterns
//			 1:	succeed in getting expanded strings patterns
//			 2:	failure of getting expanded strings patterns 
//				by upper bound of length of character string for expanding
//			 3: failure of getting expanded strings patterns 
//				by upper bounds of number of expanded strings patterns
//	ModVector<ModUnicodeString>& expanded
//		O: expanded strings patterns
//
// RETURN
//	ModBoolean
//		ModTrue:	normal termination
//		ModFalse:	something error
//
// EXCEPTIONS
//	exception of lower modules
//
	ModBoolean getExpandStrings(ModSize& expandResult, ModVector<ModUnicodeString>& expanded);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::getConcept -- get noun phrase
//
// NOTES
//	get noun phrase from the text
//
// ARGUMENTS
//	ModUnicodeString& normalized_
//		O: normalized noun phrase
//	ModUnicodeString& original_
//		O: original noun phrase
//	ModVector<ModUnicodeString>& normVector
//		O: vector including normalized words
//	ModVector<ModUnicodeString>& origVector
//		O: vector including original words
//	ModVector<int>& posvector
//		O: vector including part of speeches
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
	ModBoolean getConcept(ModUnicodeString& normalized_,
		ModUnicodeString& original_,
		double& npCost_,
		ModVector<ModUnicodeString>* normVector_=0,
		ModVector<ModUnicodeString>* origVector_=0,
		ModVector<int>* posVector_=0);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::poolTerm -- Extraction of noun phrase
//
// NOTES
//	The noun phrase is extracted from the text 
//	and it registers in the pool.
//
// ARGUMENTS
//	const ModBoolean
//		I: useStopDict		whether to use the stop dictionary
//	const ModSize
//		I: maxText			The text maximum length
//							(The number of morphological analysises.
//							If it is 0, it is unrestricted).
//	const ModBoolean
//		I: useStopTypeDict	whether to use the stop type dictionary
//
// RETURN
//	none
//
// EXCEPTIONS
//
	void poolTerm(const ModBoolean useStopDict,
				const ModSize maxText,
				const ModBoolean useStopTypeDict);

//
// FUNCTION public
//	ModNlpAnalyzerUnaJp::makeMorphArray -- get morpheme array
//
// NOTES
//  This function is used to get morpheme array.
//	It is used only from ModNlpAnalyzer::extractNP.
//
// ARGUMENTS
//	ModVector<Morph>& result_
//		O: morpheme array
//	ModUnicodeString& cOrigBuffer_
//		O: original word
//	ModUnicodeString &cNormBuffer_
//		O: normalized word
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void makeMorphArray(ModVector<Morph>& result_,
						ModUnicodeString &cOrigBuffer_,
						ModUnicodeString &cNormBuffer_);

protected:
//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::parse -- parse parameters about analyzer
//
// NOTES
//	parse parameters about analyzer
//
// ARGUMENTS
//	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >&
//		I: parameters
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	bool parse(const Parameters& param_);
//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::getBaseAbility -- get base ability
//
// NOTES
//	get base ability
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	ModSize getBaseAbility(const ModLanguageSet& lang);
//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::reset -- changes analyzer
//
// NOTES
//	ModNlpAnalyzerX is always re-created in this method.
//	ModNlpResourceX is also re-created if it is necessary.
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void reset();

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::getResourcePath -- return resource path
//
// NOTES
//	return resource path
//
// ARGUMENTS
//	none
//
// RETURN
//	ModUnicodeString
//		resource path
//
// EXCEPTIONS
//	exception of lower modules
//
 	ModUnicodeString getResourcePath() ;

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::set -- set a text to lower analyzer
//
// NOTES
//	set a text to lower analyzer
//
// ARGUMENTS
//	const ModUnicodeString&
//		I: a text
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void set();

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::read -- read the next text
//
// NOTES
//	read the next part of whole text from ModNlpText
//
// ARGUMENTS
//	none
//
// RETURN
//	ModFalse
//		text have been already processed
//	ModTrue
//		there are non-processed text
//
// EXCEPTIONS
//	exception of lower modules
//
	ModBoolean read();

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::append -- append the term
//
// NOTES
//	append the normalized string, original string.
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized term
//	ModUnicodeString orig_
//		I: original term
//	int pos_
//		I: part of speech
//	int cost_
//		I: cost of term
//	ModTermType type_
//		I: type of term
//	ModTermType& preType_
//		I: Type of the former term
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void append(ModUnicodeString norm_,
				ModUnicodeString orig_,
				int pos_,
				int cost_,
				ModTermType type_,
				ModTermType& preType_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::appendInsert -- append and insert the term
//
// NOTES
//	append and insert the normalized string, original string.
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized term
//	ModUnicodeString orig_
//		I: original term
//	int pos_
//		I: part of speech
//	int cost_
//		I: cost of term
//	ModTermType type_
//		I: type of term
//	ModTermType& preType_
//		I: Type of the former term
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void appendInsert(ModUnicodeString norm_,
						ModUnicodeString orig_,
						int pos_,
						int cost_,
						ModTermType type_,
						ModTermType& preType_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::insert -- insert the term
//
// NOTES
//	insert the normalized string, original string.
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized term
//	ModUnicodeString orig_
//		I: original term
//	int pos_
//		I: part of speech
//	int cost_
//		I: cost of term
//	ModTermType type_
//		I: type of term
//	ModTermType& preType_
//		I: Type of the former term
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void insert(ModUnicodeString norm_,
				ModUnicodeString orig_,
				int pos_,
				int cost_,
				ModTermType type_,
				ModTermType& preType_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::setTerm -- set the term
//
// NOTES
//	set the normalized string, original string and part of speech
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized term
//	ModUnicodeString orig_
//		I: original term
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void setTerm(ModUnicodeString norm_,
					ModUnicodeString orig_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::appendTerm -- append the term
//
// NOTES
//	append the normalized string, original string
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized term
//	ModUnicodeString orig_
//		I: original term
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void appendTerm(ModUnicodeString norm_,
					ModUnicodeString orig_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::pushTerm -- push the term in vector
//
// NOTES
//	push the normalized string, original string, part of speech
//	and cost in each vector
//
// ARGUMENTS
//	ModUnicodeString norm_
//		I: normalized morpheme
//	ModUnicodeString orig_
//		I: original morpheme
//	int pos_
//		I: part of speech
//	int cost_
//		I: cost of morpheme
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void pushTerm(ModUnicodeString norm_,
					ModUnicodeString orig_,
					int pos_,
					int cost_);

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::clearTerm -- clear the term
//
// NOTES
//	clear the normalized string, original string and some vector
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void clearTerm();

//
// FUNCTION protected
//	ModNlpAnalyzerUnaJp::insertTerm -- insert the term in MP pool
//
// NOTES
//	ModTermPool::insertTerm is called and the term is inserted in the NP pool
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
	void insertTerm();


private:
//
// FUNCTION private
// ModNlpAnalyzerUnaJp::covertNormMode -- Conversion of normalize mode
//
// NOTES
//	getExpandStrings uses this function to convert normalize mode.  
//	It doesn't depend on a set value of parameter compoundDiv in getExpandStrings
//	and it processes it by setting that doesn't divide the compound word(compoundDiv=false).
//	Therefore, it is necessary to convert the value set by the parameter. 
//
// ARGUMENTS
//	 ModNlpNormMode		normMode_	normalize mode
//
// RETURN
//	 none
//
// EXCEPTIONS
//
	void covertNormMode(ModNlpNormMode &normMode_);


/////////////////////////////////////////
//  Data section
	ModLanguageSet _lang;
	const ModUnicodeString* _pcstrTarget;

	ModNlpAnalyzerX*	_cAnalyzerX;

	// if this parameter is changed, recreate ModNlpAnalyzerX
	ModSize				m_ulMaxWordLength;
	bool				m_doCarriageRet;
	ModNlpExpMode		m_eExpMode;
	ModNlpNormModifier	m_cMd;
	bool				m_doNorm;
	bool				m_bKeyword;
	bool				m_bGetOrg;
	ModNlpNormMode		m_eNormMode;
	ModSize				m_ulLenOfNormalization;
	ModSize				m_ulMaxExpTargetStrLen;
	ModSize				m_ulMaxExpPatternNum;

	ModUnicodeString preNorm;
	ModUnicodeString preOrig;
	int prePos;
	int preCost;

	ModTerm*			_cTerm;
	ModTermResource*	termResource;
	ModUnicodeString	preTermNorm;	// normalized prepositive
	ModUnicodeString	preTermOrig;	// original prepositive
	ModVector<ModUnicodeString>	termNormVector;
	ModVector<ModUnicodeString>	termOrigVector;
	ModVector<int>				termPosVector;
	ModVector<int>				termCostVector;
};
_UNA_UNAJP_END
_UNA_END 
#endif//__UNA_MODNLPANALYZERUNAJP_H

//
// Copyright (c) 2005, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
