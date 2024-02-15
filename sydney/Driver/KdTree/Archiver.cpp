// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Archiver.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/Archiver.h"

#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::Archiver::Archiver -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::IndexFile& cIndexFile_
//		索引ファイル
//	bool bUpdate_
//		更新モードかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Archiver::Archiver(IndexFile& cIndexFile_, bool bUpdate_)
	: m_cIndexFile(cIndexFile_), m_bUpdate(bUpdate_),
	  m_uiPageID(0), m_pBuffer(0), m_pEnd(0)
{
	nextPage();
}

//
//	FUNCTION public
//	KdTree::Archiver::~Archiver -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Archiver::~Archiver()
{
	m_cPage.unfix(m_bUpdate);
}

//
//	FUNCTION public
//	KdTree::Archiver::write -- 書き込む
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		書き込むintegerデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Archiver::write(int n_)
{
	write(syd_reinterpret_cast<const char*>(&n_), sizeof(int));
}

//
//	FUNCTION public
//	KdTree::Archiver:::write -- 書き込む
//
//	NOTES
//
//	ARGUMENTS
//	const char* p_
//		書き込む領域の先頭
//	int size_
//		書き込む領域のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Archiver::write(const char* p_, int size_)
{
	while (size_)
	{
		int rest = static_cast<int>(m_pEnd - m_pBuffer);
		int s = (size_ > rest) ? rest : size_;

		Os::Memory::copy(m_pBuffer, p_, s);
		
		m_pBuffer += s;
		p_ += s;
		size_ -= s;

		if (size_)
		{
			// 次のページを得る
			nextPage();
		}
	}
}

//
//	FUNCTION public
//	KdTree::Archiver::read -- 読み込む
//
//	NOTES
//
//	ARGUMENTS
//	int& n_
//		読み込んだintegerデータを格納する領域
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Archiver::read(int& n_)
{
	read(syd_reinterpret_cast<char*>(&n_), sizeof(int));
}

//
//	FUNCTION public
//	KdTree::Archiver::read -- 読み込む
//
//	NOTES
//
//	ARGUMENTS
//	char* p_
//		読み込んだデータを格納する領域の先頭
//	int size_
//		読み込む領域のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Archiver::read(char* p_, int size_)
{
	while (size_)
	{
		int rest = static_cast<int>(m_pConstEnd - m_pConstBuffer);
		int s = (size_ > rest) ? rest : size_;

		Os::Memory::copy(p_, m_pConstBuffer, s);

		m_pConstBuffer += s;
		p_ += s;
		size_ -= s;

		if (size_)
		{
			// 次のページを得る
			nextPage();
		}
	}
}

//
//	FUNCTION private
//	KdTree::Archiver::nextPage -- 次のページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Archiver::nextPage()
{
	if (m_uiPageID)
	{
		// 直前のものをunfixする
		m_cPage.unfix(m_bUpdate);
	}

	if (m_bUpdate)
	{
		// 次のページを得る
		m_cPage = m_cIndexFile.allocatePage(m_uiPageID++);

		// 領域を得る
		m_pBuffer = m_cPage.operator char*();
		m_pEnd = m_pBuffer + m_cPage.getSize();
	}
	else
	{
		// 次のページを得る
		m_cPage = m_cIndexFile.fixPage(m_uiPageID++);

		// 領域を得る
		m_pConstBuffer = m_cPage.operator const char*();
		m_pConstEnd = m_pConstBuffer + m_cPage.getSize();
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
