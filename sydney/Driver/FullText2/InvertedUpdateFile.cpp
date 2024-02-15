// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUpdateFile.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/InvertedUpdateFile.h"
#include "FullText2/InvertedSection.h"

#include "ModInvertedCoder.h"

#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//  FUNCTION public
//  FullText2::InvertedUpdateFile::InvertedUpdateFile -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//	FullText2::InvertedSectin& cSection_
//		転置ファイルセクション
//  const Os::Path& cPath_
//		パス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedUpdateFile::InvertedUpdateFile(InvertedSection& cSection_,
									   const Os::Path& cPath_)
	: InvertedFile(cSection_.getFileID(), cPath_), m_cSection(cSection_)
{
}

//
//  FUNCTION public
//  FullText2::InvertedUpdateFile::~InvertedUpdateFile -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedUpdateFile::~InvertedUpdateFile()
{
}

//
//	FUNCTION public
//	FullText2::InvertedUpdateFile::getIdCoder
//	FullText2::InvertedUpdateFile::getFrequencyCoder
//	FullText2::InvertedUpdateFile::getLengthCoder
//	FullText2::InvertedUpdateFile::getLocationCoder
//		-- 圧縮器を得る
//
//	NOTES
//	与えられたキーに対応した圧縮器を取得する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//	RETURN
//	ModInvertedCoder*
//		圧縮器
//
//	EXCEPTIONS
//
ModInvertedCoder*
InvertedUpdateFile::getIdCoder(const ModUnicodeString& cstrKey_)
{
	return m_cSection.getIdCoder(cstrKey_);
}
ModInvertedCoder*
InvertedUpdateFile::getFrequencyCoder(const ModUnicodeString& cstrKey_)
{
	return m_cSection.getFrequencyCoder(cstrKey_);
}
ModInvertedCoder*
InvertedUpdateFile::getLengthCoder(const ModUnicodeString& cstrKey_)
{
	return m_cSection.getLengthCoder(cstrKey_);
}
ModInvertedCoder*
InvertedUpdateFile::getLocationCoder(const ModUnicodeString& cstrKey_)
{
	return m_cSection.getLocationCoder(cstrKey_);
}

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
