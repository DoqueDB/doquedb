// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNlpLocal.cpp
// 
// Copyright (c) 2005-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/UnaNameSpace.h"
#include "LibUna/Keyword.h"
#include "LibUna/ModNLPLocal.h"
#include "LibUna/ModNLP.h"


_UNA_USING

// 空文字列
static ModUnicodeString _cNullString;
// true
static ModUnicodeString _cTrue("true");

// As for this function, when only the ModNlpUnaJp module is used,
// and executed, the function of the inherited class is called.
ModBoolean
ModNlpLocalAnalyzer::isExpStrDataLoad()
{
	return ModFalse;
}

// As for this function, when only the ModNlpUnaJp module is used,
// and executed, the function of the inherited class is called.
ModBoolean
ModNlpLocalAnalyzer::getBlock(
	ModVector<ModUnicodeString>& normVector_,
	ModVector<ModUnicodeString>& hinVector_)
{
	return ModFalse;
}

// As for this function, when only the ModNlpUnaJp module is used,
// and executed, the function of the inherited class is called.
ModBoolean
ModNlpLocalAnalyzer::getDicName(
	ModVector<ModUnicodeString>& dicNameVector_)
{
	return ModFalse;
}

ModBoolean
ModNlpLocalAnalyzer::getConcept(
	ModUnicodeString& extractedConcept_,
	ModVector<ModUnicodeString>& normVector_,
	ModVector<ModUnicodeString>& origVector_,
	ModVector<int>& posVector_)
{
	ModSize len;
	// LocalAnalyzerの親クラスをpAnalyzerに設定
	ModNlpAnalyzer *pAnalyzer = this->getParent();
	// 親クラスのgetConceptImp()を呼び出す
	if(pAnalyzer->getConceptImp(extractedConcept_,normVector_,origVector_,posVector_,len)==ModFalse)
		return ModFalse;
	return ModTrue;
}

// As for this function, when only the ModNlpUnaJp module is used,
// and executed, the function of the inherited class is called.
ModBoolean
ModNlpLocalAnalyzer::getExpandStrings(
	ModSize& expandResult,
	ModVector<ModUnicodeString>& expanded)
{
	return ModFalse;
}

//
//	FUNCTION public
//	UNA::ParameterWrapper::ParameterWrapper -- コンストラクタ
//
ParameterWrapper::ParameterWrapper(const ParamMap& cParam_)
	: m_cParam(cParam_)
{
}

//
//	FUNCTION public
//	UNA::ParameterWrapper::~ParameterWrapper -- デストラクタ
ParameterWrapper::~ParameterWrapper()
{}

//
//	FUNCTION public
//	UNA::ParameterWrapper::getString
//		-- 文字列を得る。キーがない場合は空文字列
//
const ModUnicodeString&
ParameterWrapper::getString(const ModUnicodeString& cstrKey_) const
{
	ParamMap::ConstIterator i = m_cParam.find(cstrKey_);
	if (i != m_cParam.end())
		return (*i).second;
	return _cNullString;
}

//
//	FUNCTION public
// 	UNA::ParameterWrapper::getBoolean
//		-- bool値を得る。キーがない場合はdefault値を返す
//
bool
ParameterWrapper::getBoolean(const ModUnicodeString& cstrKey_,
							 bool default_) const
{
	bool r = default_;
	ParamMap::ConstIterator i = m_cParam.find(cstrKey_);
	if (i != m_cParam.end())
	{
		if ((*i).second == _cTrue)
			r = true;
		else
			r = false;
	}
	return r;
}

//
//	FUNCTION public
// 	UNA::ParameterWrapper::getModSize
//		-- ModSize値を得る。キーがない場合はdefault値を返す
//
ModSize
ParameterWrapper::getModSize(const ModUnicodeString& cstrKey_,
							 ModSize default_) const
{
	ModSize r = default_;
	const ModUnicodeString& v = getString(cstrKey_);
	if (v.getLength() != 0)
		r = ModUnicodeCharTrait::toUInt(v);
	return r;
}

//
// Copyright (c) 2005-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
