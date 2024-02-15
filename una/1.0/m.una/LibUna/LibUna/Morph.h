// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Morph.h -- Definition file of Morph class
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_LIBUNA_MORPH_H
#define __UNA_LIBUNA_MORPH_H

#include "Type.h"
#include "ModAutoPointer.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "UnicodeChar.h"
#include "ModUnicodeOstrStream.h"


namespace UNA {


//
//	CLASS
//		Morph
//

class Morph	: public	ModDefaultObject
{
public:
	//	TYPEDEF
	//		MorphRange -- morph range class
	typedef		Type::Range<const ModUnicodeChar*>	MorphRange;

	class Type {
	public:
		// part of speech number
		typedef	int	Value;
		enum {	Num = 0 };
	};
	// Constructor, Destructor
	Morph(){}
	Morph(Type::Value type_,
	  const ModUnicodeChar* org_, ModSize orglen_,
	  const ModUnicodeChar* norm_, ModSize normlen_)
	  {
	  	  	_type = type_;
	  		_org  = MorphRange(org_, orglen_);
	  		_norm = MorphRange(norm_, normlen_);
	  }

	~Morph(){}
	// get part of speech

	virtual
	Type::Value			getType() const{return _type;}
	// Access function to character string
	ModUnicodeString		getNorm() const
	{
		if ( _norm._start )
			return ModUnicodeString(_norm._start, _norm._len);
		return ModUnicodeString(_org._start, _org._len);
	}
	const ModUnicodeChar*
	getNormPos() const
	{
		if ( _norm._start )
			return _norm._start;
		return _org._start;
	}
	const ModUnicodeChar*
	getForceNormPos() const
	{
		return _norm._start;
	}

	ModSize getNormLen() const
	{
		if ( _norm._start )
			return _norm._len;
		return _org._len;
	}
	void setNorm(const ModUnicodeChar* norm_)
	{
		_norm._start = norm_;
	}
	void setOrg(const ModUnicodeChar* org_)
	{
		_org._start = org_;
	}

	void getNorm(const ModUnicodeChar*& norm_, ModSize& len_) const
	{
		if ( _norm._start ) {
			norm_ = _norm._start;
			len_  = _norm._len;
		} else {
			norm_ = _org._start;
			len_  = _org._len;
		}
	}

	ModUnicodeString		getOrg() const
	{
		return ModUnicodeString(_org._start, _org._len);
	}
	const ModUnicodeChar*
	getOrgPos() const
	{
		return _org._start;
	}
	ModSize getOrgLen() const
	{
		return _org._len;
	}
	void				getOrg(const ModUnicodeChar*& pos_, ModSize& len_) const;
		

	// Properly you inquire whether it is expressed
	ModBoolean			isNorm() const	
	{
		return (_norm._start != 0 ? ModTrue : ModFalse);
	}
	// input operator
	Morph&				operator=(const Morph& morph_)
	{
		if ( this != &morph_ ) {
			_type = morph_._type;
			_org = morph_._org;
			_norm = morph_._norm;
		}
		return *this;
	}


private:

	Type::Value			_type;		// part of speech
	MorphRange			_org;		// original word
	MorphRange			_norm;		// normalized word

};

//
// CLASS
//	IncludeCharFunction -- The function class which you inspect whether character position is included in morpheme,
//
class IncludeCharFunction
{
public:
	ModBoolean operator()(const Morph& morph_, const ModUnicodeChar* pos_) const
	{
		if ( morph_.isNorm() )
			return (morph_.getNormPos() < pos_) ? ModTrue : ModFalse;
		else
			return (morph_.getOrgPos() < pos_) ? ModTrue : ModFalse;
	}
};

}

#endif // __UNA_LIBUNA_MORPH_H

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
