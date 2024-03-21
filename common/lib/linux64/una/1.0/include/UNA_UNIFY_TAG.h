// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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
#ifndef UNA_UNIFY_TAG_H
#define UNA_UNIFY_TAG_H
enum UNA_UNIFY_TAG
{
	UNA_NOUN 		= 0x00000001,	// Noun
	UNA_PROPER 		= 0x00000002,	// Proper Name
	UNA_PRO 		= 0x00000004,	// Pronoun
	UNA_VERB 		= 0x00000008,	// Verb
	UNA_QUALIFIER 	= 0x00000010,	// Qualifier
	UNA_A 			= 0x00000020,	// Adjective
	UNA_ADV 		= 0x00000040,	// Adverb
	UNA_CONJ 		= 0x00000080,	// Conjection
	UNA_PREP 		= 0x00000100,	// Preposition
	UNA_PFX 		= 0x00000200,	// Prefix
	UNA_SFX 		= 0x00000400,	// Suffix
	UNA_DET 		= 0x00000800,	// Article
	UNA_FILLER 		= 0x00001000,	// Filler
	UNA_SYMBOL 		= 0x00002000,	// Symbol
	UNA_NUMBER 		= 0x00004000,	// Number
	UNA_UNKNOWN 	= 0x00008000,	// Unknown
	UNA_OTHER 		= 0x00010000	// Other
};


struct TransTable{
	unsigned short	code;
	const char*	name;
};

struct TagTable
{
	const char *name;
	int id;
};
	
struct TagCode 
{
	int strict;
	int common;
	TagCode(){ strict = 0; common = UNA_OTHER;}
	TagCode(int strict_,int common_)
	{
		strict = strict_;
		common = common_;
	}
};
#endif

