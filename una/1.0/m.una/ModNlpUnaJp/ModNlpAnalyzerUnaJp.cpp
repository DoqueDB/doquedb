//
// ModNlpAnalyzerUnaJp.cpp - Implementation of Tag and NP module of Unified-Una
// 
// Copyright (c) 2005 - 2010, 2023 Ricoh Company, Ltd.
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

#include "ModAutoMutex.h"
#include "ModCriticalSection.h"
#include "ModUnicodeOstrStream.h"
#include "ModNLPText.h"
#include "ModNLP.h"
#include "UnaDynamicCast.h"
#include "UnaParam.h"
#include "Morph.h"
#include "LibUna/Keyword.h"	
#include "ModNlpUnaJp/Module.h"
#include "ModNlpUnaJp/ModNlpResourceUnaJp.h"
#include "ModNlpUnaJp/ModNlpAnalyzerUnaJp.h"


_UNA_USING
_UNA_UNAJP_USING

namespace {
ModUnicodeString cstrThisAnalyzer("unajp");
const ModSize ulMinLenToRead = 4096;

namespace _Param {
	ModUnicodeString MaxWordLen(Param::MaxWordLen::Key);
	ModUnicodeString Stemming(Param::Stemming::Key);
	ModUnicodeString CompoundDiv(Param::CompoundDiv::Key);
	ModUnicodeString CarriageRet(Param::CarriageRet::Key);
	ModUnicodeString ExpMode(Param::ExpMode::Key);
	ModUnicodeString Space(Param::Space::Key);
	ModUnicodeString DoNorm(Param::DoNorm::Key);
	ModUnicodeString AnaNo(Param::AnaNo::Key);
	ModUnicodeString Disable(Param::Disable::Key);
	ModUnicodeString LengthOfNorm(Param::LengthOfNormalization::Key);
	ModUnicodeString MaxExpTargetStrLen(Param::MaxExpTargetStrLen::Key);
	ModUnicodeString MaxExpPatternNum(Param::MaxExpPatternNum::Key);
#if 0
	ModUnicodeString Keyword(Param::Keyword::Key);
	ModUnicodeString GetOrg(Param::GetOrg::Key);
	ModUnicodeString NormMode(Param::NormMode::Key);
#endif
}
	
}

ModNlpAnalyzerUnaJp::ModNlpAnalyzerUnaJp()
: _pcstrTarget(0),
  _lang(ModLanguageSet()),
  _cAnalyzerX(0),
  _cTerm(0),
  m_ulMaxWordLength(0),
  m_eNormMode(ModNlpNormOnly),
  m_eExpMode(ModNlpExpNoChk),
  m_doNorm(true),
  m_bKeyword(false),
  m_bGetOrg(false),
  m_doCarriageRet(false),
  m_ulLenOfNormalization(0),
  m_ulMaxExpTargetStrLen(40),	// default value of the maximum length of character string for expanding
  m_ulMaxExpPatternNum(40)		// default value of the maximum number of expanded patterns
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::ModNlpAnalyzerUnaJp\n",0);  
	m_doStem = false;
	m_doCompoundDiv = false;
}

ModNlpAnalyzerUnaJp::~ModNlpAnalyzerUnaJp()
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::~ModNlpAnalyzerUnaJp\n",0);  

	this->releaseResource();
}

ModBoolean
ModNlpAnalyzerUnaJp::setResource(
 	ModNlpLocalResource* res_)
{
  DEBUGPRINT("ModNlpAnalyzerUnaJp::setResource\n",0);
  m_resource = res_;//_res = _UNA_DYNAMIC_CAST(ModNlpResourceUnaJp*,res_);
  m_dicSet = ((ModNlpResourceUnaJp*)m_resource)->getDicSet(_lang);
  return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::releaseResource()
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::releaseResource\n",0);  

	delete _cAnalyzerX,_cAnalyzerX=0;
	delete _cTerm,_cTerm=0;
	return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::prepare(
    const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& parametersToModifyAnalyze_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::prepare\n",0);
	bool ret;
	if ((ret = parse(parametersToModifyAnalyze_)) == true)
	{
		reset();
	}
	return ModTrue;
}

bool
ModNlpAnalyzerUnaJp::parse(const Parameters& param_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::parse\n",0);

	bool result = (_cAnalyzerX == 0) ? true : false;
	
	ParameterWrapper param(param_);

	ModSize l = param.getModSize(_Param::MaxWordLen, m_ulMaxWordLength);
	if (l != m_ulMaxWordLength)
	{
		m_ulMaxWordLength = l;
		result = true;
	}

	m_doStem = param.getBoolean(_Param::Stemming, m_doStem);
	m_doCompoundDiv = param.getBoolean(_Param::CompoundDiv, m_doCompoundDiv);
	m_doCarriageRet = param.getBoolean(_Param::CarriageRet, m_doCarriageRet);
	
	if (m_doStem)
	{
		if (m_doCompoundDiv)
		{
			if (m_doCarriageRet)
			{
				m_eNormMode = ModNlpNormRetStemDiv;
			}
			else
			{
				m_eNormMode = ModNlpNormStemDiv;
			}
		}
		else
		{
			if (m_doCarriageRet)
			{
				m_eNormMode = ModNlpNormRetStem;
			}
			else
			{
				m_eNormMode = ModNlpNormStem;
			}
		}
	}
	else
	{
		if (m_doCompoundDiv)
		{
			if (m_doCarriageRet)
			{
				m_eNormMode = ModNlpNormRetDiv;
			}
			else
			{
				m_eNormMode = ModNlpNormDiv;
			}
		}
		else
		{
			if (m_doCarriageRet)
			{
				m_eNormMode = ModNlpNormRet;
			}
			else
			{
				m_eNormMode = ModNlpNormOnly;
			}
		}
	}

	if (param.getString(_Param::ExpMode).getLength() != 0)
	{
		ModSize ulExpMode = ModUnicodeCharTrait::toUInt(
					static_cast<const ModUnicodeChar*>(
						param.getString(_Param::ExpMode)));
		m_eExpMode = static_cast<ModNlpExpMode>(ulExpMode);
	}

	if (param.getString(_Param::Space).getLength() != 0)
	{
		ModSize ulSpace = ModUnicodeCharTrait::toUInt(
			static_cast<const ModUnicodeChar*>(
				param.getString(_Param::Space)));
		m_cMd.space(static_cast<ModNlpAreaTreatment>(ulSpace));
	}

	m_doNorm = param.getBoolean(_Param::DoNorm, m_doNorm);

	if (param.getString(_Param::AnaNo).getLength() != 0)
	{
		_cstrIndicatedAnalyzer = param.getString(_Param::AnaNo);
	}

	if (param.getString(_Param::Disable).getLength() != 0)
	{
		_cstrDisableAnalyzer = param.getString(_Param::Disable);
	}

	m_ulLenOfNormalization = param.getModSize(_Param::LengthOfNorm,
											 m_ulLenOfNormalization);

	// maximum length of character string for synonym expanding function
	if (param.getString(_Param::MaxExpTargetStrLen).getLength() != 0) {
		ModSize len = param.getModSize(_Param::MaxExpTargetStrLen, m_ulMaxExpTargetStrLen);
		if (len != m_ulMaxExpTargetStrLen) {
			m_ulMaxExpTargetStrLen = len;
		}
	}

	// maximum number of expanded strings pattern for synonym expanding function
	ModSize num = param.getModSize(_Param::MaxExpPatternNum, m_ulMaxExpPatternNum);
	if (num != m_ulMaxExpPatternNum) {
		m_ulMaxExpPatternNum = num;
	}

	return result;
}

ModSize 
ModNlpAnalyzerUnaJp::getAbility(const ModLanguageSet& lang)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getAbility\n",0);

	//4	prefered ( will modify all return abilities to 4)
	//3	perfect ability
	//2	percial ability
	//1	no ability and default processing
	//0	no ability and no processing
	ModSize ulAbility = 0;
	if(_cstrIndicatedAnalyzer==cstrThisAnalyzer)
	{
		ulAbility = 4;//prefered
	}
	else if (_cstrDisableAnalyzer!=cstrThisAnalyzer)
	{
		const ModLanguageSet& resourceLang
			= ((ModNlpResourceUnaJp*)m_resource)->getResourceX()->getLanguage();

		if (lang.getSize() == 0)
		{
			ulAbility = 3;
		}
		else if (resourceLang.isContained(ModLanguage::ja))
		{
			// when japanese resource
			if (lang.isContained(ModLanguage::ja))
				ulAbility = 3;
			else
				ulAbility = 1;
		}
		else
		{
			// when foreign resource
			if (lang.isContained(ModLanguage::ja) == ModFalse)
				ulAbility = 2;
			else
				ulAbility = 1;
		}
	}
	return ulAbility;
}

ModBoolean 
ModNlpAnalyzerUnaJp::getWord(
	ModUnicodeString& normalized_, 
	ModUnicodeString* original_, 
	int* pos_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getWord\n",0);
#if 0
	if (_pcstrTarget != 0)
	{
		_cAnalyzerX->set(*_pcstrTarget, m_eNormMode, _lang, m_cMd);
		_pcstrTarget = 0;
	}

	ModBoolean rv = ModFalse;
	
	if (m_bKeyword)
	{
		ModUnicodeChar *ostr;
		ModSize len;
		rv = _cAnalyzerX->getWord(normalized_, ostr, len, *pos_);
		if (rv && original_) original_->allocateCopy(ostr,len);
	}
	else if (!m_bGetOrg )
	{
		rv = _cAnalyzerX->getWord(normalized_, m_doNorm ? ModTrue : ModFalse);
	}
	else
	{
		rv = _cAnalyzerX->getWord(normalized_, *original_, m_doNorm ? ModTrue : ModFalse);
	}
#else
	if (_pcstrTarget != 0)
	{
		_cAnalyzerX->set(*_pcstrTarget, m_eNormMode, _lang, m_cMd);
		_pcstrTarget = 0;
	}

	ModBoolean rv = ModFalse;
	
	if (pos_ != 0)
	{
		ModUnicodeChar *ostr;
		ModSize len;
		rv = _cAnalyzerX->getWord(normalized_, ostr, len, *pos_);
		if (rv && original_) original_->allocateCopy(ostr,len);
	}
	else if (original_ == 0)
	{
		rv = _cAnalyzerX->getWord(normalized_, m_doNorm ? ModTrue : ModFalse);
	}
	else
	{
		rv = _cAnalyzerX->getWord(normalized_, *original_, m_doNorm ? ModTrue : ModFalse);
	}
#endif
	return rv;
}

void
ModNlpAnalyzerUnaJp::reset()
{
	this->releaseResource();
	_cAnalyzerX = new ModNlpAnalyzerX(((ModNlpResourceUnaJp*)m_resource)->getResourceX(),
				((ModNlpResourceUnaJp*)m_resource)->getJapaneseDic(),
				m_ulMaxWordLength);

	// for NP extraction
	termResource = _cAnalyzerX->getTermResource();

	if(termResource != 0){
		_cTerm = new ModTerm(termResource, this);
	}
}

ModBoolean
ModNlpAnalyzerUnaJp::getExpandWords(
    ModVector<ModUnicodeString>& expanded_,
    ModUnicodeString& original_,
    int& pos_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getExpandWords\n",0);

	expanded_.clear();
	original_.clear();
	pos_ =0;

	if (_pcstrTarget != 0)
	{
		_cAnalyzerX->set(*_pcstrTarget, m_eNormMode, _lang, m_cMd);
		_pcstrTarget = 0;
	}

	ModBoolean rv = ModFalse;

	ModUnicodeString* pcstrExpandedWord = 0;
	ModSize ulNum = _cAnalyzerX->getExpandWords(pcstrExpandedWord,original_,pos_,m_eExpMode);
	if (ulNum > 0)
	{
		rv = ModTrue;
	}
	for (ModSize i=0;i<ulNum;++i)
	{
		expanded_.pushBack(pcstrExpandedWord[i]);
	}
	delete [] pcstrExpandedWord,pcstrExpandedWord=0;

	return rv;
}

ModBoolean 
ModNlpAnalyzerUnaJp::getBlock(
		ModVector<ModUnicodeString>& normVector,
  		ModVector<int>& posVector,
  		ModVector<ModUnicodeString>* origVector,
  		ModVector<int>* costVector_,
		ModVector<int>* uposVector_,
		ModBoolean ignore_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getBlock\n",0);
	normVector.clear();
	posVector.clear();

	if (_pcstrTarget != 0)
	{
		_cAnalyzerX->set(*_pcstrTarget, m_eNormMode, _lang, m_cMd);
		_pcstrTarget = 0;
	}

	ModBoolean rv = ModFalse;

	if (origVector == 0)
	{
		rv = _cAnalyzerX->getBlock(normVector, posVector, m_doNorm ? ModTrue : ModFalse);
	}
	else
	{
		origVector->clear();
		rv = _cAnalyzerX->getBlock(normVector, *origVector, posVector, m_doNorm ? ModTrue : ModFalse, costVector_, uposVector_, ignore_);
	}
	return rv;
}

ModBoolean 
ModNlpAnalyzerUnaJp::getBlock(
    ModUnicodeString& result,
	ModUnicodeChar sep1,
	ModUnicodeChar sep2)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getBlock\n",0);
	result.clear();

	if (_pcstrTarget != 0)
	{
		_cAnalyzerX->set(*_pcstrTarget, m_eNormMode, _lang, m_cMd);
		_pcstrTarget = 0;
	}

	ModBoolean rv = ModFalse;
	
	rv = _cAnalyzerX->getBlock(result, sep1, sep2, m_doNorm ? ModTrue : ModFalse);
	return rv;
}

ModBoolean
ModNlpAnalyzerUnaJp::getBlock(
	ModVector<ModUnicodeString>& normVector,
	ModVector<ModUnicodeString>& hinVector)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getBlock\n",0);
	normVector.clear();
	hinVector.clear();

	ModUnicodeString result;
	result.clear();

	if(!getBlock(result,0x23,0x24))
		return ModFalse;

	char sep1[] = "$"; // #
	char sep2[] = "#"; // $

	char *tok1;
	char *tok2;
	const char *s1 = result.getString();
	int s1len = strlen(s1);
	char *ssbase = (char*)(malloc(s1len+1));

	if(ssbase == NULL){
		return ModFalse;
	}

	memset(ssbase,0x00,s1len+1);
	memcpy(ssbase,s1,s1len);

	int pos = 0, pos1 = 0, pos2 = 0;

	// ".....$"までを一区切りとして切り出す
	tok1 = strtok((char*)ssbase, sep1);
	while( tok1 != NULL ){
		// tok1の#で切り出し、一つ目
		tok2 = strtok(tok1, sep2);
		while( tok2 != NULL ){
			if(pos == 1){ /* 表記 */
				normVector.pushBack(tok2);
			}
			if(pos == 2){ /* 品詞名 */
				hinVector.pushBack(tok2);
			}
			tok2 = strtok( NULL, sep2 );  /* 2回目以降 */
			pos ++;
		}
		pos2 ++;
		pos = 0;

		if(pos2 > 0){
			// ".....$"までを一区切りとして切り出す
			if(ssbase != NULL){
				memset(ssbase,0x00,s1len+1);
				memcpy(ssbase,s1,s1len);
				tok1 = strtok((char*)ssbase, sep1);
				for(pos1 = 0; pos1 < pos2; pos1++){
					tok1 = strtok(NULL, sep1);
				}
			}
		}
	}

	if(ssbase != NULL){
		free(ssbase);
	}

	return ModTrue;
}

ModBoolean 
ModNlpAnalyzerUnaJp::getDicName(
		ModVector<ModUnicodeString>& dicNameVector_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getDicName\n",0);
	dicNameVector_.clear();

	return _cAnalyzerX->getDicName(dicNameVector_);
}

ModLanguageCode
ModNlpAnalyzerUnaJp::identify(
  ModSize endPos)
{return ModLanguage::undefined;}

ModBoolean
ModNlpAnalyzerUnaJp::getWholeText(
  ModUnicodeString& wholeText_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getWholeText\n",0);

	wholeText_.clear();

	if (_pcstrTarget == 0)
	{
		return ModFalse;
	}
	
	ModUnicodeString normalized;
	ModUnicodeOstrStream cStream;
	while (getWord(normalized,0,0) == ModTrue)
	{
		cStream << normalized;
	}
	wholeText_ = cStream.getString();

	return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::getNormalizeBuf(
  ModUnicodeString& wholeText_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getWholeText\n",0);

	wholeText_.clear();

	if (_pcstrTarget == 0)
	{
		return ModFalse;
	}
	else
	{
		_cAnalyzerX->getNormalizeBuf(
			*_pcstrTarget, wholeText_, m_ulLenOfNormalization);
		_pcstrTarget = 0;
	}

	return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::getExpandBuf(ModVector<ModUnicodeString>& expanded_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getExpandBuf\n",0);

	expanded_.clear();

	if (_pcstrTarget == 0)
	{
		return ModFalse;
	}
	else
	{
		ModUnicodeString* pcstrExpandedTexts = 0;
		ModSize ulNum = _cAnalyzerX->getExpandBuf(
			*_pcstrTarget,pcstrExpandedTexts,
			m_ulLenOfNormalization,
			m_doNorm ? ModFalse : ModTrue);//m_doNorm is not eExpandOnly_
		if (ulNum == 0) return ModFalse;

		ModSize i = 0;
		for (i = 0;i<ulNum;++i)
		{
			expanded_.pushBack(pcstrExpandedTexts[i]);
		}
		delete [] pcstrExpandedTexts,pcstrExpandedTexts=0;
		_pcstrTarget = 0;
	}

	return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::getExpandStrings(ModSize& expandResult_, ModVector<ModUnicodeString>& expanded_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::getExpandStrings\n",0);

	expandResult_ = ModNlpAnalyzer::ExpandResultNone;
	expanded_.clear();
	/* save target string */
	ModUnicodeString orgTarget;
	ModSize exp_sum = 0;

	if(_pcstrTarget != 0){
		orgTarget = *_pcstrTarget;
		/* The default of the maximum character string length is less than 40. */
		if((*_pcstrTarget).getLength() > m_ulMaxExpTargetStrLen){
			expandResult_ = ModNlpAnalyzer::ExpandResultMaxExpTargetStrLen;
			expanded_.pushBack(orgTarget);
			return ModTrue;
		} else {
			ModNlpNormMode normMode = m_eNormMode;
			covertNormMode(normMode);

			if(_cAnalyzerX->isExpStrStrDicLoad() == ModTrue){	// V125形式+文字列マッチ
				exp_sum = _cAnalyzerX->getExpandStrings(*_pcstrTarget, expanded_, m_ulMaxExpPatternNum);
			} else if(_cAnalyzerX->isExpStrMorDicLoad() == ModTrue){	// V125形式+形態素マッチ
				_cAnalyzerX->set(*_pcstrTarget, normMode, _lang, m_cMd);
				exp_sum = _cAnalyzerX->getExpandStrings(expanded_, m_ulMaxExpPatternNum);
			} else {
				_pcstrTarget = 0;
				ModErrorMessage << "Expanding data is unloaded!" << ModEndl;
				ModThrow(ModModuleStandard, 
						 ModCommonErrorBadArgument,
						 ModErrorLevelError);
			}
			_pcstrTarget = 0;
		}
	}

	if(exp_sum == 0){	 // upper bounds of the number of development patterns
		expandResult_ = ModNlpAnalyzer::ExpandResultMaxExpPatternNum;
		/* The string for expanding is set in the expanded result. */
		expanded_.clear();
		expanded_.pushBack(orgTarget);
		return ModTrue;
	} else if(exp_sum == 1){
		expandResult_ = ModNlpAnalyzer::ExpandResultNone;
		if(_pcstrTarget == 0){
			expanded_.clear();
			expanded_.pushBack(orgTarget);
		}
		return ModTrue;
	} else if(exp_sum < m_ulMaxExpPatternNum){
		expandResult_ = ModNlpAnalyzer::ExpandResultExist;
		return ModTrue;
	}
	return ModFalse;	// unexpected value of exp_sum
}

// FUNCTION
// ModNlpAnalyzerUnaJp::getConcept
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeString& normalized_, 
//		--- one normalized noun phrase
//	ModUnicodeString& original_, 
//		--- one original noun phrase
//	double& npCost_,
//		--- cost of noun phrase
//	ModVector<ModUnicodeString>* normVector_, 
//		--- vector of root form words of one NP
//	ModVector<ModUnicodeString>* origVector_, 
//		--- vector of original form words of one NP
//	ModVector<int>* posVector_
//		--- vector of POS number of one NP
//
// RETURN
//	ModBoolean
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		ModTerm is null.
//
ModBoolean
ModNlpAnalyzerUnaJp::getConcept(ModUnicodeString& normalized_,
								ModUnicodeString& original_,
								double& npCost_,
								ModVector<ModUnicodeString>* normVector_,
								ModVector<ModUnicodeString>* origVector_,
								ModVector<int>* posVector_)
{
	// Generation of term
	if(_cTerm != 0){
		if(_cTerm->getTerm(normalized_, original_, npCost_, normVector_, origVector_, posVector_) == ModFalse)
			return ModFalse;
	} else{
		ModErrorMessage << "ModTerm is null." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
	}
	return ModTrue;
}

ModUnicodeString
ModNlpAnalyzerUnaJp::getResourcePath()
{
  DEBUGPRINT("ModNlpAnalyzerUnaJp::getResourcePath\n",0);  
  return ((ModNlpResourceUnaJp*)m_resource)->getResourcePath();
}

ModBoolean 
ModNlpAnalyzerUnaJp::set(const ModUnicodeString& target_,
						 const ModLanguageSet& lang_)
{
	DEBUGPRINT("ModNlpAnalyzerUnaJp::set\n",1);

	_lang = lang_;
	_pcstrTarget = &target_;

	if(_cTerm != 0)
		_cTerm->resetTerm();

	return ModTrue;
}

ModBoolean
ModNlpAnalyzerUnaJp::isExpStrDataLoad()
{
	return _cAnalyzerX->isExpStrDicLoad();
}

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
//	 ModNlpNormMode&	normMode_	normalize mode
//
// RETURN
//	 none
//
// EXCEPTIONS
//
void
ModNlpAnalyzerUnaJp::covertNormMode(ModNlpNormMode& normMode_)
{
	if(normMode_ == ModNlpNormDiv){
		normMode_ = ModNlpNormOnly;
	} else if(normMode_ == ModNlpNormStemDiv){
		normMode_ = ModNlpNormStem;
	} else if(normMode_ == ModNlpNormRetStemDiv){
		normMode_ = ModNlpNormRetStem;
	} else if(normMode_ == ModNlpNormRetDiv){
		normMode_ = ModNlpNormRet;
	}
}

//
// FUNCTION public
// ModNlpAnalyzerUnaJp::poolTerm -- Extraction of noun phrase
//
// NOTES
//	The noun phrase is extracted from the text 
//	and it registers in the pool.
//
// ARGUMENTS
//	 const ModBoolean	 		useStopDict		whether to use the stop dictionary
//	 const ModSize				maxText			The text maximum length
//												(The number of morphological analysises.
//											 	If it is 0, it is unrestricted).
//	 const ModBoolean	 		useStopTypeDict	whether to use the stop type dictionary
//
// RETURN
//	 none
//
// EXCEPTIONS
//
void
ModNlpAnalyzerUnaJp::poolTerm(
	const ModBoolean				useStopDict,	// whether to use the stop dictionary
	const ModSize					maxText,		// he text maximum length
	const ModBoolean				useStopTypeDict)// whether to use the stop type dictionary
{
	ModTermType type;					// Pattern type
	ModTermType	preType;				// Pattern type of prepositive
	ModTermType	preTypeNum;				// Pattern type of the former clitic of numeral
	ModSize analyzedLength = 0;			// The analyzed number of words

	ModVector<ModUnicodeString> normVector;
	ModVector<ModUnicodeString> origVector;
	ModVector<int>				posVector;
	ModVector<int>				costVector;

	ModVector<ModUnicodeString>::Iterator norm;
	ModVector<ModUnicodeString>::Iterator orig;
	ModVector<int>::Iterator			  pos;
	ModVector<int>::Iterator			  cost;

	preTermNorm.clear();
	preTermOrig.clear();

	// At every an analytical block
	while(this->getBlock(normVector, posVector, &origVector, &costVector, 0, ModFalse)
		  == ModTrue) {
		norm = normVector.begin();
		orig = origVector.begin();
		pos  = posVector.begin();
		cost = costVector.begin();

		// Each retrieval word
		for(; pos != posVector.end();
			++pos, ++norm, ++orig, ++cost, ++analyzedLength) {

			// from the part of speech to the type
			type = (* (termResource->termTypeTable))[*pos];

			// Noun、suffix、慣用数字表現
			if(type == TERM_NOUN_S || type == TERM_SUFFIX || termResource->stopRepreDict->isFound(*norm) == ModTrue) {
				if(preType == TERM_NUMBER){
					if(termNormVector.getSize() == 0) {
						preType = 0;
						continue;
					} else if (preTermNorm.getLength() != 0){
						this->insert(*norm, *orig, *pos, *cost, preType, preTypeNum);
						continue;
					}
				} else if (type == TERM_SUFFIX && preTermNorm.getLength() != 0){
					this->appendInsert(*norm, *orig, *pos, *cost, type, preType);
					continue;
				} else if ((type == TERM_SUFFIX || termResource->stopRepreDict->isFound(*norm) == ModTrue) && preTermNorm.getLength() == 0){
					preType = 0;
					continue;
				} else{
					this->append(*norm, *orig, *pos, *cost, type, preType);
					continue;
				}
			// numeral
			} else if(type == TERM_NUMBER) {
				if(preType == TERM_NUMBER && preTermNorm.getLength() != 0) {
					this->insert(*preNorm, preOrig, prePos, preCost, preType, preTypeNum);
				}
				// 保存用
				preNorm = (*norm).getString();
				preOrig = (*orig).getString();
				prePos = *pos;
				preCost = *cost;
				preTypeNum = preType;
				preType = type;
				continue;
			} else if (preType == TERM_NUMBER && preTermNorm.getLength() != 0){
				this->append(preNorm, preOrig, prePos, preCost, TERM_NUMBER, preType);
			}

			// Prohibition word、delimitation(記号.中点)、symbol
			if(type == TERM_STOP || type == TERM_DELIM || type == TERM_SYMBOL) {
				if(preTermNorm.getLength() != 0){
					if((ModUnicodeCharTrait::find(*norm, UnicodeChar::usAmpersand) != 0) ||
						(ModUnicodeCharTrait::find(*norm, UnicodeChar::usSlash) != 0) ||
						(ModUnicodeCharTrait::find(*norm, UnicodeChar::usHyphen) != 0)){
						this->appendTerm(*norm, *orig);
						this->pushTerm(*norm, *orig, *pos, *cost);
						preType = type;
						continue;
					} else {
						this->insert(*norm, *orig, *pos, *cost, type, preType);
						continue;
					}
				} else {
					preType = 0;
					continue;
				}
			// Prepositive
			} else if(type == TERM_PREFIX) {
				if(preTermNorm.getLength() != 0){
					this->insert(*norm, *orig, *pos, *cost, type, preType);
					continue;
				// Retrieval of prohibition dictionary
				} else {
					if(useStopDict == ModTrue
					&& termResource->stopDict->isFound(*norm) == ModTrue) {
						preType = 0;
						continue;
					}
					this->setTerm(*norm, *orig);
					this->pushTerm(*norm, *orig, *pos, *cost);
					preType = type;
					continue;
				}
			} else {
				this->append(*norm, *orig, *pos, *cost, type, preType);
				continue;
			}
		}

		// When it reaches the maximum analytical length, it ends.
		if(maxText != 0 && analyzedLength > maxText) {
			if(preTermNorm.getLength() != 0) {
				this->insert(*norm, *orig, *pos, *cost, type, preType);
			}
			break;
		}
	}

	// The end is registered
	if(preTermNorm.getLength() != 0){
		if(preType == TERM_NUMBER) {
			this->appendInsert(preNorm, preOrig, prePos, preCost, type, preType);
		} else {
			this->insert(preNorm, preOrig, prePos, preCost, type, preType);
		}
	}
}

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
//	int pos_,
//		I: part of speech
//	int cost_,
//		I: cost of term
//	ModTermType type_,
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
void
ModNlpAnalyzerUnaJp::append(ModUnicodeString norm_,
							ModUnicodeString orig_,
							int pos_,
							int cost_,
							ModTermType type_,
							ModTermType& preType_)
{
	if(_cTerm->useStopDict1 == ModTrue && termResource->stopDict->isFound(norm_) == ModTrue){
		if(preTermNorm.getLength() != 0){
			this->insert(norm_, orig_, pos_, cost_, type_, preType_);
		} else {
			preType_ = 0;
		}
	} else {
		if(preTermNorm.getLength() != 0) {
			this->appendTerm(norm_, orig_);
			this->pushTerm(norm_, orig_, pos_, cost_);
		} else {
			this->setTerm(norm_, orig_);
			this->pushTerm(norm_, orig_, pos_, cost_);
		}
		// For preservation
		preNorm = (norm_).getString();
		preOrig = (orig_).getString();
		prePos = pos_;
		preCost = cost_;
		preType_ = type_;
	}
}

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
//	int pos_,
//		I: part of speech
//	int cost_,
//		I: cost of term
//	ModTermType type_,
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
void
ModNlpAnalyzerUnaJp::appendInsert(ModUnicodeString norm_,
								ModUnicodeString orig_,
								int pos_,
								int cost_,
								ModTermType type_,
								ModTermType& preType_)
{
	if(_cTerm->useStopDict1 == ModTrue && termResource->stopDict->isFound(norm_) == ModTrue){
			this->insert(norm_, orig_, pos_, cost_, type_, preType_);
	} else {
		this->appendTerm(norm_, orig_);
		this->pushTerm(norm_, orig_, pos_, cost_);
		this->insertTerm();
		this->clearTerm();
		preType_ = 0;
	}
}

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
//	int pos_,
//		I: part of speech
//	int cost_,
//		I: cost of term
//	ModTermType type_,
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
void
ModNlpAnalyzerUnaJp::insert(ModUnicodeString norm_,
							ModUnicodeString orig_,
							int pos_,
							int cost_,
							ModTermType type_,
							ModTermType& preType_)
{
	if(termCostVector.getSize() == 1){ // Single word
		// Retrieval of stop type dictionary
		// A single word of "名詞.接尾辞""数詞""接尾辞""形容動詞.一般""接頭辞.一般""名詞.形動" is excluded according to JpExRule.txt.
		if((_cTerm->useStopTypeDict1 == ModTrue && termResource->stopTypeDict->isFound(preType_))
			|| (_cTerm->useStopDict1 == ModTrue && termResource->stopDict->isFound(preTermNorm) == ModTrue)
			|| (preType_ == TERM_UNKNOWN && (preTermNorm.getLength() == 1))
			|| (preType_ == TERM_ALPHABET && (preTermNorm.getLength() == 1))) {
			;
		} else {
			this->insertTerm();
		}
	} else {
		this->insertTerm();
	}

	if(type_ == TERM_PREFIX){
		this->clearTerm();
		this->setTerm(norm_, orig_);
		this->pushTerm(norm_, orig_, pos_, cost_);
		preType_ = type_;
		return;
	}

	this->clearTerm();
	preType_ = 0;
}

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
void
ModNlpAnalyzerUnaJp::setTerm(ModUnicodeString norm_,
					ModUnicodeString orig_)
{
	preTermNorm = norm_;
	preTermOrig = orig_;
}

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
void
ModNlpAnalyzerUnaJp::appendTerm(ModUnicodeString norm_,
							ModUnicodeString orig_)
{
	preTermNorm.append(norm_);
	preTermOrig.append(orig_);
}

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
//		I:part of speech
//	int cost_
//		I:cost of morpheme
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
void
ModNlpAnalyzerUnaJp::pushTerm(ModUnicodeString norm_,
							ModUnicodeString orig_,
							int pos_,
							int cost_)
{
	termNormVector.pushBack(norm_);
	termOrigVector.pushBack(orig_);
	termPosVector.pushBack(pos_);
	termCostVector.pushBack(cost_);
}

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
void
ModNlpAnalyzerUnaJp::clearTerm()
{
	preTermNorm.clear();
	termNormVector.clear();
	preTermOrig.clear();
	termOrigVector.clear();
	termPosVector.clear();
	termCostVector.clear();
}

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
void
ModNlpAnalyzerUnaJp::insertTerm()
{
	ModTermElement term(preTermNorm, preTermOrig);
	term.setNormVector(termNormVector);
	term.setOrigVector(termOrigVector);
	term.setPosVector(termPosVector);
	term.setCostVector(termCostVector);
	_cTerm->insertTerm(term);
}

void
ModNlpAnalyzerUnaJp::
makeMorphArray(ModVector<Morph>& result_,ModUnicodeString &cOrigBuffer_, ModUnicodeString &cNormBuffer_)
{
	ModUnicodeString	cstrNorm;
	ModUnicodeString	cstrOrg;
	Morph::Type::Value	type;

	int orgPos = 0;
	int normPos = 0;
	while ( this->getWord(cstrNorm, &cstrOrg, &type) )
	{

		// get org word
		ModSize orglen = cstrOrg.getLength();

		// get norm word
		ModSize normlen = cstrNorm.getLength();
		cOrigBuffer_ += cstrOrg;
		// とりあえず、各文字列の開始位置を記録しておく
		if ( normlen <= 0 )
		{
			cNormBuffer_ += cstrOrg;
			result_.pushBack( Morph(type,
							reinterpret_cast<ModUnicodeChar*>(orgPos),
							orglen,
							reinterpret_cast<ModUnicodeChar*>(orgPos),
							orglen) );
			normlen = orglen;
		}
		else
		{
			cNormBuffer_ += cstrNorm;
			result_.pushBack( Morph( ((UnaJpDicSet*)m_dicSet)->getHeadNoFromGroup(type), 
							reinterpret_cast<ModUnicodeChar*>(orgPos),
							orglen, 
							reinterpret_cast<ModUnicodeChar*>(normPos),
							normlen) );
		}
		orgPos += orglen;
		normPos += normlen;

	} // end of while
	// 上のループで記録した文字列位置をポインタで置き換えて、終了する
	const ModUnicodeChar* pOrigBuffer = cOrigBuffer_;
	const ModUnicodeChar* pNormBuffer = cNormBuffer_;
	for(ModVector<Morph>::Iterator iter = result_.begin(); iter != result_.end(); ++iter)
	{
		(*iter).setOrg (pOrigBuffer + (unsigned long)((*iter).getOrgPos()));
		(*iter).setNorm(pNormBuffer + (unsigned long)((*iter).getForceNormPos()));
	}
}

//
// Copyright (c) 2005 - 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
