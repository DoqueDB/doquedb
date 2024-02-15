// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RxTools.cpp -- Implement file of RxTools class
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/Algorithm.h"
#include "LibUna/RxTools.h"
#include "LibUna/UnicodeChar.h"
#include "ModCharString.h"

_UNA_USING

namespace
{
	namespace UniChar = UnicodeChar;

	// 品詞番号を文字に変換した時のオフセット値
	const ModUnicodeChar			TypeStartValue		=	0x0100;

	// 正規表現で使用するエスケープ文字
	const ModUnicodeChar			EscapeChar		=	UniChar::usBackSlash;	// '\'

	// 品詞文字の区切り
	const ModUnicodeChar			TypeNameDelimita	=	UniChar::usWquate;	// '"'

	// 正規表現の範囲指定演算子
	const ModUnicodeChar			StartRange		=	UniChar::usLbracket;	// '['
	const ModUnicodeChar			EndRange		=	UniChar::usRbracket;	// ']'
} // end of namespace

//
// FUNCTION pubic
//	RxTools::getUnicodeChar(int type_);
//
// NOTES
//
// ARGUMENTS
//	int type_	-- part of speech id
//
// RETURN
//	ModUnicodeChar
//
// EXCEPTIONS
//
ModUnicodeChar
RxTools::getUnicodeChar(Morph::Type::Value type_)
{
	return (type_ + TypeStartValue);
}

//
// FUNCTION pubic
// RxTools::getTypeString -- 形態素列から品詞化された文字列を取得する
//
// NOTES
//
// ARGUMENTS
//   ModVector<Common::Morph>::ConstIterator srcIt_
//     START position of morph list
//   ModVector<Common::Morph>::ConstIterator srcFin_
//     end position of morph list
//   ModVector<ModUnicodeChar> & type_
//     品詞化文字列格納位置
//
// RETURN
//
// EXCEPTIONS
//
void
RxTools::getTypeString(ModVector<Morph>::ConstIterator srcIt_,
		   ModVector<Morph>::ConstIterator srcFin_,
		   ModVector<ModUnicodeChar>& type_)
{
	// 領域の予約(null文字分を追加しておく)
	type_.reserve(srcFin_ - srcIt_ + 1);

	// 品詞を文字化しながら追加
	for ( ; srcIt_ != srcFin_; ++srcIt_ )
		type_.pushBack(getUnicodeChar((*srcIt_).getType()));
	type_.pushBack(UnicodeChar::usNull);
}

//
// FUNCTION pubic
// RxTools::findReplaceString -- 文字列を置き換える
//
// NOTES
//	delim_ で囲まれた部分を groupList_ 内の文字列に置き換える
//	注：delim_ も含んで置き換える
//
// ARGUMENTS
//	const ModUnicodeString& src_
//		置き換え文字列を含んだ参照文字列
//	const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& list_
//		置き換え文字リスト
//   	ModUnicodeChar delim_
//		置き換え対象文字の区切り
//	ModUnicodeString& dst_
//		置き換え済み文字列格納文字列
//
// RETURN
//
// EXCEPTIONS
//
void
RxTools::findReplaceString(const ModUnicodeString& src_,
						   const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& list_,
						   ModUnicodeChar delim_,
						   ModUnicodeString& dst_)
{
	typedef ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
				GroupString;

	const ModUnicodeChar* tgt = src_;
	const ModUnicodeChar* fin = tgt + src_.getLength();
	const ModUnicodeChar* last = tgt;

	for ( ; tgt != fin; ++tgt ) {

		if ( *tgt == delim_ ) {

			// 区切り文字までの文字列を保存しておく
			if ( last != tgt )
				dst_.append(ModUnicodeString(last, (ModSize)(tgt - last)));

			// 開始位置を保存
			const ModUnicodeChar* start = tgt;

			// 次の区切り文字を探す
			while ( delim_ != *(++tgt) ) {
				if ( tgt == fin ) {
					// 区切り文字は偶数個存在していないといけない
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument,
							 ModErrorLevelError);
				}
			}

			// 区切り文字までの文字を Map 内で検索

			GroupString::ConstIterator it = list_.find(ModUnicodeString(start, (ModSize)(tgt - start + 1)));
			if ( it == list_.end() ) {
				// 見つからないのはエラー
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument,
						 ModErrorLevelError);
			}

			// 区切り文字で囲まれた部分の変わりに登録されている文字を追加
			dst_.append((*it).second);

			// 最終処理位置を保存
			last = tgt+1;
		}
	}

	// 残った文字を追加
	dst_.append(last);
}

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
