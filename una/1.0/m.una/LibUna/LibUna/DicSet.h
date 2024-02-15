//
// DicSet.h -
// 
// Copyright (c) 2004-2010, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_DicSet__H
#define __UNA_DicSet__H
#include "UnaDLL.h"
#include "ModLanguageSet.h"
#include "ModUnicodeString.h"
#include "UnicodeChar.h"
#include "Morph.h"
#include "UNA_UNIFY_TAG.h"
#include "DoubleArray.h"


namespace UNA {

class UNA_LOCAL_FUNCTION TagConvTable
{
	// automaton machine
	int			m_base;
	TagTable	*m_table;
	DoubleArray<TagCode> m;

public:
	TagConvTable(TagTable *table_,int base_)
	{
		char name[128];
		m_table = table_;
		m_base  = base_;
		for( TagTable *e = table_ ; e->name ; e++)
		{
			strcpy(name,e->name);
			strcat(name,"#");
			TagCode code = TagCode((int)(base_ + (e - table_)),e->id);
			m.insert((UNA_CHAR *)(name),code);
		}
	}
	TagCode getTypeCode(unsigned char* tag_)
	{
		TagCode code;
		char name[128];
		strcpy((char*)name,(const char*)tag_);
		strcat(name,"#");
		if(m.search((UNA_CHAR *)(name),code))
		{
			return code;
		}
		else
			return TagCode();
	}

	const char* getTypeName(const int unahin_)
	{
		return m_table[unahin_- m_base].name;
	}

	const int getTypeId(const int unahin_)
	{
		return m_table[unahin_- m_base].id;
	}
};

class DicSet
{
	ModLanguageSet m_lang;
	const char *m_rule;
	TagConvTable *tagConvTable;

public:
	DicSet(const char *rule_,ModLanguageSet &lang_):m_rule(rule_),m_lang(lang_),tagConvTable(0)
	{
	}

	~DicSet(){ delete tagConvTable;}

	void build(TagTable *table_,int base_)
	{
		if(tagConvTable == 0)
			tagConvTable = new TagConvTable(table_,base_);
	}

	virtual TagCode getTypeCode(unsigned char* tag_)
	{
		return tagConvTable->getTypeCode(tag_);
	}

	virtual TagCode getTypeCode(ModUnicodeString& tag_)
	{
		return getTypeCode((unsigned char*)tag_.getString());
	}

	virtual int getTypeCode(int pos_)
	{
		return UNA_OTHER;
	}

	virtual const char*	getTypeName(const int unahin_)
	{
		return tagConvTable->getTypeName(unahin_);
	}

	virtual const int getTypeId(const int unahin_)
	{
		return tagConvTable->getTypeId(unahin_);
	}

	virtual void getTypeCodeString(const ModUnicodeString& src_, ModUnicodeString& dst_)
	{
		// It keeps looking at src_ one by one, the place of part of speech name to letter converts
		const ModUnicodeChar* tgt = src_;
		const ModUnicodeChar* srcBeg = tgt;
		const ModUnicodeChar* srcEnd = tgt + src_.getLength();

		for( ; tgt < srcEnd; ++tgt ) {

			if ( *tgt == UnicodeChar::usWquate) {

				// start of PartOfSpeech
				const ModUnicodeChar* typeName = tgt + 1;

				// It finishes to read part of speech name
				while ( *(++tgt) != UnicodeChar::usWquate) {

					if ( tgt >= srcEnd ) {
						ModErrorMessage << "out of rule command" << ModEndl;
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument,
								 ModErrorLevelError);
					}
				}
				// Part of speech number is obtained from part of speech name
				ModUnicodeString tag = ModUnicodeString(typeName, (ModSize)(tgt - typeName));
				Morph::Type::Value code = getTypeCode(tag).strict;

				// To letter converting part of speech number, it adds to pattern
				dst_.append(ModUnicodeChar(code + 0x0100));
			} else {

				// Because it is other than part of speech name, that way addition
				dst_.append(*tgt);
			}

		}
	}

	virtual bool getCost(unsigned char *word,int &cost)
	{
		cost = 0;
		return false;
	}

	ModLanguageSet& getLanguageSet(){return m_lang;}

	virtual const char *getRule(){return m_rule;}

	unsigned int getRuleID()
	{
		if ( m_lang.isContained( ModLanguage::ja ) == ModTrue )
		{
			return ( ModLanguage::ja );
		}
		else if ( m_lang.isContained( ModLanguage::en ) )
		{
			return ( ModLanguage::en );
		}
		else
		{
			return ( ModLanguage::undefined );
		}
	}
	virtual void registSameType(const ModUnicodeString& type_){}
};

} // namespace UNA

#endif  // __UNA_DicSet__H

//
// Copyright (c) 2004-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
