// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	Keyword.cpp -- キーワード実装ファイル
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

#include "LibUna/UnaNameSpace.h"
#include "LibUna/Algorithm.h"
#include "LibUna/Algorithm.h"
#include "LibUna/Morph.h"
#include "LibUna/Keyword.h"
#include "ModUnicodeOstrStream.h"

_UNA_USING

//
// FUNCTION public
//	Keyword::Keyword
//		-- Keyword クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Keyword::Keyword()
: _score(0.0)
{
}

//
// FUNCTION public
//	Keyword::Keyword
//		-- Keyword クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Keyword::Keyword(ModVector< Type::Range<const Morph*> >& words_)
: _position(words_), _score(0.0)
{
}

//
// FUNCTION public
//	Keyword::Keyword
//		-- Keyword クラスのコピーコンストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Keyword::Keyword(const Keyword& keyword_)
: _position(keyword_._position),
  _score(keyword_._score),
  _calcoperation(keyword_._calcoperation)
{
}

//
//	FUNCTION pubic
//	Keyword::getSize  -- 構成数を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	unsigned int
//		構成数
//
//	EXCEPTIONS
//
unsigned int
Keyword::getSize() const
{
	return _position[0]._len;
}

//
//	FUNCTION pubic
//	Keyword::getOrg  -- キーワードの文字列(異表記正規化前)を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Keyword::getOrg() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;

		for ( ; it != fin; ++it ) {
			str << it->getOrg();
		}
	}
	return str.getString();
}

//
// FUNCTION pubic
//	Keyword::getOrgEuroStyle
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS
//
ModUnicodeString
Keyword::getOrgEuroStyle() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head  = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;
		for ( ; it != fin; ++it ) {
			str << (*it).getOrg() << " ";
		}
	}
	return str.getString();
}

//
//	FUNCTION pubic
//	Keyword::getNorm  -- キーワードの文字列(異表記正規化後)を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Keyword::getNorm() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;

		for ( ; it != fin; ++it ) {
			str << it->getNorm();
		}
	}
	return str.getString();
}

//
// FUNCTION pubic
//	Keyword::getNormEuroStyle
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS
//
ModUnicodeString
Keyword::getNormEuroStyle() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head  = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;
		for ( ; it != fin; ++it ) {
			str << (*it).getNorm() << " ";
		}
	}
	return str.getString();
}

//
//	FUNCTION pubic
//	Keyword::getOrgMessage  -- キーワードを構成する形態素(異表記正規化前)を文字列として返す。デバッグ用
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Keyword::getOrgMessage() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head  = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;
		for ( ; it != fin; ++it ) {
			str << (*it).getOrg() << "/";
		}
	}
	return str.getString();
}

//
//	FUNCTION pubic
//	Keyword::getNormMessage  -- キーワードを構成する形態素(異表記正規化後)を文字列として返す。デバッグ用
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Keyword::getNormMessage() const
{
	ModUnicodeOstrStream str;
	ModVector<KeywordRange>::ConstIterator head  = _position.begin();
	if ( head != _position.end() ) {

		const Morph* it  = (*head)._start;
		const Morph* fin = it + (*head)._len;
		for ( ; it != fin; ++it ) {
			str << (*it).getNorm() << "/";
		}
	}
	return str.getString();
}

//
//	FUNCTION pubic
//	Keyword::pushBackPosition -- 位置情報を追加する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Morph*	pos_
//		キーワード出現位置
//	ModSize len_
//		キーワード長さ（形態素数）
//
//	RETURN
//	void
//
//	EXCEPTIONS
//
void
Keyword::pushBackPosition(const Morph* pos_, ModSize len_)
{
	_position.pushBack(KeywordRange(pos_, len_));
}

//
//	FUNCTION pubic
//	Keyword::operator[]  -- Morph へのアクセサ
//
//	NOTES
//
//	ARGUMENTS
//	int i_
//		要素番号
//
//	RETURN
//	const Morph&
//		形態素クラス
//
//	EXCEPTIONS
//
const Morph&
Keyword::operator[](int i_) const
{
	return *(_position[0]._start + i_);
}

//
//	FUNCTION pubic
//	Keyword::operator=
//		   ::operator+=
//
//	NOTES
//
//	ARGUMENTS
//	const Keyword& keyword_
//		キーワードクラス
//
//	RETURN
//	Keyword&
//		キーワードクラス
//
//	EXCEPTIONS
//
Keyword&
Keyword::operator=(const Keyword& keyword_)
{
	if ( &keyword_ != this ) {
		_position = keyword_._position;
		_score = keyword_._score;
		_calcoperation = keyword_._calcoperation;
	}
	return *this;
}

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
