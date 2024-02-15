// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
#ifndef __UNA_MODNLPRESOURCEUNAJP_H
#define __UNA_MODNLPRESOURCEUNAJP_H

#include "LibUna/DicSet.h"
#include "ModNLPLocal.h"
#include "Module.h"
#include "ModNLPX.h"

_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModNlpResourceX;
	struct TypeInfo
	{
		TypeInfo() {}
		TypeInfo(unsigned short code_, const ModUnicodeString& name_)
			: _code(code_), _name(name_) {}
		unsigned short		_code;	// the value of mask
		ModUnicodeString	_name;	// name of partofspeech
	};


class UnaJpDicSet:public DicSet
{
	int				l_TypeNameSize;
	//	TYPEDEF
	//		SameTypeBank -- 同品詞グループリスト
	typedef ModMap<unsigned short, int, ModLess<unsigned short> > SameTypeBank;
	// same part of speech group bank
	ModAutoPointer< SameTypeBank >			l_SameTypeBank;

	ModVector< TypeInfo >		l_TypeNameInfo;
	// l_TypeNameInfo にアクセスする際に使用するlocker
	ModCriticalSection		l_TypeNameInfoCS;

	int getTypeNoFromTypeString(const ModUnicodeString& typeString_, int startpos_ = 0);
	void makeSameTypeList(const ModUnicodeString& typename_);
	const ModVector< TypeInfo >& getTypeInfoVector();
	int getGroupHeadNoFromTypeNo(int startpos_ = 0);
public:
	UnaJpDicSet(const char* rule_,ModLanguageSet lang_);
	unsigned short getHeadNoFromGroup(unsigned short unahin_);
	unsigned short getGroupFromTypeNo(unsigned short unahin_);
	int getTypeCodeFromTypeString(const ModUnicodeString& typename_);
	TagCode getTypeCode(ModUnicodeString& typeString_);
	const char*	getTypeName(const int unahin_){return "";}

	int getTypeCode(int pos);
	void registSameType(const ModUnicodeString& type_);
};

//
// CLASS
//	ModNlpResourceUnaJp -- The resource class used by ModNlpAnalyzerUNAJP
//
// NOTES
//	This resource class used by ModNlpAnalyzerUNAJP
//
class ModNlpResourceUnaJp: public ModNlpLocalResource
{
public:
//
// FUNCTION public
//	ModNlpResourceUnaJp::ModNlpResourceUnaJp -- constructor
//
// NOTES
//	default constructor of ModNlpResourceUnaJp
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
	ModNlpResourceUnaJp();
	
//
// FUNCTION public
//	ModNlpResourceUnaJp::~ModNlpResourceUnaJp -- destructor
//
// NOTES
//	destructor of ModNlpResourceUnaJp
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
	~ModNlpResourceUnaJp();
	
//
// FUNCTION public
//	ModNlpResourceUnaJp::load -- resource file is checked
//
// NOTES
//	set resource path and memory save switch and resource file is checked.
//	Loading resource data is done in activate() methode.
//
// ARGUMENTS
//	const ModUnicodeString& resourcePath, 
//		I: resource path
//	const ModBoolean memorySave
//		I: memory save switch
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Load resources.
	Notes: set resource path and memory save switch and resource file is checked.
	@param[in]	resourcePath	path for the resource
	@param[in]	memorySave		memory save switcher
	\return	Whether load successfully.
*/
	ModBoolean load(const ModUnicodeString& resourcePath, 
					const ModBoolean memorySave);

//
// FUNCTION public
//	ModNlpResourceUnaJp::unload -- resource file is freed
//
// NOTES
//	resource file is freed
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
	Release resources.
	\return	Whether unload successfully.
*/
	ModBoolean unload();

//
// FUNCTION public
//	ModNlpResourceUnaJp::getResourceX -- return ModNlpResourceX
//
// NOTES
//	return ModNlpResourceX
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpResourceX*
//		address of the ModNlpResourceX
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	return ModNlpResourceX
	\return ModNlpResourceX*: address of the ModNlpResourceX
*/
	ModNlpResourceX* getResourceX();

	DicSet *getJapaneseDic();
	DicSet * getDicSet(const ModLanguageSet& lang_){return getJapaneseDic();}
private:
//
// FUNCTION private
//	ModNlpResourceUnaJp::parse -- parse parameters about resource
//
// NOTES
//	parse parameters about resource
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
	void parse(ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& param_);
//
// FUNCTION private
//	ModNlpResourceUnaJp::checkFile -- checks existance of resouce file and decides resource type
//
// NOTES
//	checks existance of resouce file and decides resource type
//
// ARGUMENTS
//	const ModUnicodeString&
//		I: resource file path
//
// RETURN
//	ModSize
//		resource type
//
// EXCEPTIONS
//	exception of lower modules
//
	ModSize checkFile(const ModUnicodeString& resourcePath_);
	
	UnaJpDicSet			JpDicSet;
	ModNlpResourceX*	_cResourceX;
};

_UNA_UNAJP_END
_UNA_END

#endif//__UNA_ModNlpResourceUnaJp_H

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
