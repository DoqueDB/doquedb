// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:    
//
//	ModNormC.cpp -- C language interface of ModNormalizer
// 
// Copyright (c) 2000-2008, 2023 Ricoh Company, Ltd.
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

#include "ModUnicodeString.h"
#include "ModAutoPointer.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormC.h"
#include "UnaReinterpretCast.h"

_UNA_USING
_UNA_UNAJP_USING

#define PrintAndReset(e) {ModErrorMessage<<e<<ModEndl;ModErrorHandle::reset();}

//
// FUNCTION public
//	modNormInit -- Initializing the Norm module
//
// NOTES
//	Initializing the Norm module
//
// ARGUMENTS
//	const char* dataDirPath
//
// RETURN
//	const char* dataDirPath
//	const int normalizeEnglish
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void*
modNormInit(const char* dataDirPath,
			const int normalizeEnglish)
{
	void*	result = 0;
	try {
		ModNormRule* p = new ModNormRule(ModUnicodeString(dataDirPath),
										 ModBoolean(normalizeEnglish));
		result = p;
	}
	catch (ModException& e) { PrintAndReset(e) };
	return result;
}

//
// FUNCTION public
//	modNormInit2 -- Initializing the Norm module
//
// NOTES
//	Initializing the Norm module
//
// ARGUMENTS
//	const char* ruleDicath
//	...
//	const int normalizeEnglish
//
// RETURN
//	void*
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void*
modNormInit2(const char* ruleDicPath,
			 const char* ruleAppPath,
			 const char* expandDicPath,
			 const char* expandAppPath,
			 const char* connectPath,
			 const char* unknownTablePath,
			 const char* unknownCostPath,
			 const char* normalTablePath,
			 const char* preMapPath,
			 const char* postMapPath,
			 const char* combiMapPath,
			 const int normalizeEnglish)
{
	void*	result = 0;
	try {
		ModNormRule* p = new ModNormRule(ModUnicodeString(ruleDicPath),
										 ModUnicodeString(ruleAppPath),
										 ModUnicodeString(expandDicPath),
										 ModUnicodeString(expandAppPath),
										 ModUnicodeString(connectPath),
										 ModUnicodeString(unknownTablePath),
										 ModUnicodeString(unknownCostPath),
										 ModUnicodeString(normalTablePath),
										 ModUnicodeString(preMapPath),
										 ModUnicodeString(postMapPath),
										 ModUnicodeString(combiMapPath),
										 ModBoolean(normalizeEnglish));
		result = p;
	}
	catch (ModException& e) { PrintAndReset(e) };
	return result;
}

//
// FUNCTION public
//	modNormTerm -- Terminating the Norm module
//
// NOTES
//	Termiating the Norm module
//
// ARGUMENTS
//	void* modNormInit_result
//
// RETURN
//	None
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void
modNormTerm(void* modNormInit_result)
{
	if (modNormInit_result != 0) {
		ModNormRule* p = una_reinterpret_cast<ModNormRule*>(modNormInit_result);
		delete p;
	}
}

//
// FUNCTION public
//	modNormCreateNormalizer -- Creating a normalizer
//
// NOTES
//	Creating a normalizer
//
// ARGUMENTS
//	const void* path1
// 
// RETURN
//	void*
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void*
modNormCreateNormalizer(const void* modNormInit_result)
{
	void* result = 0;
	try {
		ModNormalizer* p = new ModNormalizer(
			una_reinterpret_cast<const ModNormRule*>(modNormInit_result));
		result = p;
	}
	catch (ModException& e) { PrintAndReset(e) };
	return result;
}

//
// FUNCTION public
//	modNormDestroyNormalizer -- Destroying a normalizer
//
// NOTES
//	Destroying a normalizer
//
// ARGUMENTS
//	void*
//
// RETURN
//	None
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void
modNormDestroyNormalizer(void*  normalizer)
{
	ModNormalizer* p = una_reinterpret_cast<ModNormalizer*>(normalizer);
	if (p != 0) {
		delete p;
	}
}

//
// FUNCTION public
//	modNormNormalize -- Normalizing a string
//
// NOTES
//	Normalizing a string
//
// ARGUMENTS
//	void*
//		normalizer
//	const char*
//		string_to_be_normalized
//
// RETURN
//	void*
//		A pointer to a ModUnicodeString
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void*
modNormNormalize(void* normalizer, const char*  str)
{
	void* result = 0;
	ModNormalizer* n = una_reinterpret_cast<ModNormalizer*>(normalizer);
	if (n != 0) {
		try {
			ModUnicodeString	original(str, 0, ModKanjiCode::utf8);
			ModUnicodeString*	resultStr = new ModUnicodeString();
			ModAutoPointer<ModUnicodeString> resultStrAP(resultStr);
			*resultStr = n -> normalizeBuf(original, 0, 0, ModNormalized);
			// Creating UTF-8 string
			resultStr -> getString(ModKanjiCode::utf8);
			resultStrAP.release();
			result = (void*) resultStr;
		}
		catch (ModException& e) { PrintAndReset(e) };
	}
	return result;
}

//
// FUNCTION public
//	modNormDestroyResult -- Destroying the result of normalization
//
// NOTES
//	Destroying the result of normalization
//
// ARGUMENTS
//	void*
//		result_of_modNormNormalize
//
// RETURN
//	None
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void
modNormDestroyResult(void* result)
{
	ModUnicodeString* p = una_reinterpret_cast<ModUnicodeString*>(result);
	if (p != 0) {
		delete p;
	}
}

//
// FUNCTION public
//	modNormGetString -- Getting the normalized string
//
// NOTES
//	Returning the normalized string
//
// ARGUMENTS
//	void*
//		result_of_modNormNormalize
//
// RETURN
//	const char*
//		normalized string (you need not free this string)
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
const char*
modNormGetString(void* resltString)
{
	const char* result = 0;
	ModUnicodeString* p = una_reinterpret_cast<ModUnicodeString*>(resltString);
	if (p != 0) {
		try {
			result = p -> getString(ModKanjiCode::utf8);
		}
		catch (ModException& e) { PrintAndReset(e) };
	}
	return result;
}

//
// FUNCTION public
//	modNormExpand -- Expanding a string
//
// NOTES
//	Expanding a string
//
// ARGUMENTS
//	void*
//		normalizer
//	const char*
//		string to be expanded
//	int*
//		pointer to store the number of expanded string
//
// RETURN
//	void*
//		expanded result
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void*
modNormExpand(void* normalizer, const char* string, int* sizep)
{
	void* result = 0;
	ModNormalizer* n = una_reinterpret_cast<ModNormalizer*>(normalizer);
	if (n != 0 && string != 0) {
		try {
			ModUnicodeString original(string, 0, ModKanjiCode::utf8);
			ModUnicodeString* exp = 0;
			int size;

			size = n -> expandBuf(original, exp, ModNormExpNoChk, 0, 0);
			if (sizep != 0)
				*sizep = size;
			result = (void*) exp;
		}
		catch (ModException& e) { PrintAndReset(e) };
	}
	return result;
}

//
// FUNCTION public
//	modNormDestroyExpanded -- Destroying the expanded result
//
// NOTES
//	Destroying the expanded result
//
// ARGUMENTS
//	void*
//		result_of_expansion
//
// RETURN
//	None
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
void
modNormDestroyExpanded(void* exp)
{
	ModUnicodeString* p = una_reinterpret_cast<ModUnicodeString*>(exp);
	if (p != 0) {
		delete [] p;
	}
}

//
// FUNCTION public
//	modNormGetExpandedString -- Getting an expanded string
//
// NOTES
//	Getting an expanded string
//
// ARGUMENTS
//	void*
//		result_of_expansion
//	int
//		which_string
//
// RETURN
//	an expanded string
//
// EXCEPTIONS
//	None
//
UNA_UNAJP_FUNCTION
const char*
modNormGetExpandedString(void* exp,int n)
{
	const char* result = 0;
	ModUnicodeString* p = una_reinterpret_cast<ModUnicodeString*>(exp);
	if (p != 0) {
		try {
			result = p[n].getString(ModKanjiCode::utf8);
		}
		catch (ModException& e) { PrintAndReset(e) };
	}
	return result;
}

//
// Copyright (c) 2000-2008, 2023 RICOH Company, Ltd.
// All rights reserved.
//
