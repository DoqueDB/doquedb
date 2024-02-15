// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoAttach.h -- File/Pageのアタッチ・デタッチユーティリティークラス
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_AUTOATTACH_H
#define __SYDNEY_FILECOMMON_AUTOATTACH_H


// Common
#include "Common/Object.h"
#include "Common/Assert.h"
// PhysicalFile
#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "PhysicalFile/Page.h"
// FileCommon
#include "FileCommon/AutoObject.h"
#include "FileCommon/IDNumber.h"

_SYDNEY_BEGIN

namespace FileCommon
{
	//
	//	CLASS
	//	AutoPhysicalFile -- 
	//
	//	NOTES
    //	スコープの前後で物理ファイルをアタッチ・デタッチする。
	//
	class AutoPhysicalFile : AutoObject
	{
	public:
		AutoPhysicalFile( const PhysicalFile::File::StorageStrategy& StorageStrategy_
		                , const PhysicalFile::File::BufferingStrategy& BufferingStrategy_
		                , const Lock::FileName& LockName_)
			: m_pFile( PhysicalFile::Manager::attachFile( StorageStrategy_, BufferingStrategy_ ,LockName_) )
		{
		}

		~AutoPhysicalFile()
		{
			detach();
		}

		PhysicalFile::File* get()        { return m_pFile; }
		operator PhysicalFile::File*()   { return get(); }
		PhysicalFile::File* operator->() { return get(); }
		PhysicalFile::File& operator*() { return *get(); }

		PhysicalFile::File* release()
			{ PhysicalFile::File* ret = get(); m_pFile = 0; return ret; }

		void detach()
			{ if (m_pFile) { PhysicalFile::Manager::detachFile(m_pFile); m_pFile = 0; } }

	private:
		PhysicalFile::File* m_pFile;
	};

	//
	//	CLASS
	//	AutoPhysicalPage -- 
	//
	//	NOTES
    //	スコープの前後で物理ページをアタッチ・デタッチする。
	//
	class AutoPhysicalPage : AutoObject
	{
	public:
		AutoPhysicalPage( PhysicalFile::File& cFile_
		                 ,const Trans::Transaction& cTrans_
		                 ,PhysicalFile::PageID iPageID_
		                 ,Buffer::Page::FixMode::Value eMode_
		                 ,Buffer::ReplacementPriority::Value ePriority_ = Buffer::ReplacementPriority::Low
		                )
			: m_cFile( cFile_ )
			, m_pPage( cFile_.attachPage( cTrans_ ,iPageID_ ,eMode_ ,ePriority_) )
			, m_eUnfixMode( PhysicalFile::Page::UnfixMode::NotDirty )
		{
			_SYDNEY_ASSERT(m_pPage);
		}
		AutoPhysicalPage( PhysicalFile::File* pFile_
		                 ,const Trans::Transaction& cTrans_
		                 ,PhysicalFile::PageID iPageID_
		                 ,Buffer::Page::FixMode::Value eMode_
		                 ,Buffer::ReplacementPriority::Value ePriority_ = Buffer::ReplacementPriority::Low
		                )
			: m_cFile( *pFile_ )
			, m_pPage(0)
			, m_eUnfixMode( PhysicalFile::Page::UnfixMode::NotDirty )
		{
			_SYDNEY_ASSERT(pFile_);
			m_pPage = pFile_->attachPage( cTrans_ ,iPageID_ ,eMode_ ,ePriority_);
			_SYDNEY_ASSERT(m_pPage);
		}

		AutoPhysicalPage( PhysicalFile::File& cFile_
		                 ,const Trans::Transaction& cTrans_
		                 ,Buffer::Page::FixMode::Value eMode_
		                 ,Buffer::ReplacementPriority::Value ePriority_ = Buffer::ReplacementPriority::Low
		                )
			: m_cFile( cFile_ )
			, m_pPage( cFile_.attachPage( cTrans_ ,eMode_ ,ePriority_) )
			, m_eUnfixMode( PhysicalFile::Page::UnfixMode::NotDirty )
		{
			_SYDNEY_ASSERT(m_pPage);
		}
		AutoPhysicalPage( PhysicalFile::File* pFile_
		                 ,const Trans::Transaction& cTrans_
		                 ,Buffer::Page::FixMode::Value eMode_
		                 ,Buffer::ReplacementPriority::Value ePriority_ = Buffer::ReplacementPriority::Low
		                )
			: m_cFile( *pFile_ )
			, m_pPage(0)
			, m_eUnfixMode( PhysicalFile::Page::UnfixMode::NotDirty )
		{
			_SYDNEY_ASSERT(pFile_);
			m_pPage = pFile_->attachPage( cTrans_ ,eMode_ ,ePriority_);
			_SYDNEY_ASSERT(m_pPage);
		}

		~AutoPhysicalPage()
		{
			detach();
		}

		PhysicalFile::Page* get()        { return m_pPage; }
		operator PhysicalFile::Page*()   { return get(); }
		PhysicalFile::Page* operator->() { return get(); }
		PhysicalFile::Page& operator*() { return *get(); }

		void dirty() { m_eUnfixMode = PhysicalFile::Page::UnfixMode::Dirty; }

		PhysicalFile::Page* release()
			{ PhysicalFile::Page* ret = get(); m_pPage = 0; return ret; }

		void detach()
			{ if (m_pPage) { m_cFile.detachPage( m_pPage, m_eUnfixMode ); m_pPage = 0; } }

	private:
		PhysicalFile::File& m_cFile;
		PhysicalFile::Page* m_pPage;
		PhysicalFile::Page::UnfixMode::Value m_eUnfixMode;
	};

	//
	//	CLASS
	//	AutoPhysicalPageVerify -- 
	//
	//	NOTES
    //	スコープの前後で物理ページをVerifyモードでアタッチ・デタッチする。
	//
	class AutoPhysicalPageVerify : AutoObject
	{
	public:
		AutoPhysicalPageVerify( PhysicalFile::File* pFile_
		                       ,const Trans::Transaction& cTrans_
		                       ,PhysicalFile::PageID iPageID_
		                       ,Buffer::Page::FixMode::Value eMode_
		                       ,Admin::Verification::Progress& cProgress_
		                      )
			: m_cFile( *pFile_ )
			, m_pPage(0)
		{
			_SYDNEY_ASSERT(pFile_);
			m_pPage = pFile_->verifyPage( cTrans_ ,iPageID_ ,eMode_ ,cProgress_);
			_SYDNEY_ASSERT(m_pPage);
		}

		AutoPhysicalPageVerify( PhysicalFile::File& cFile_
		                       ,const Trans::Transaction& cTrans_
		                       ,PhysicalFile::PageID iPageID_
		                       ,Buffer::Page::FixMode::Value eMode_
		                       ,Admin::Verification::Progress& cProgress_
		                      )
			: m_cFile( cFile_ )
			, m_pPage( cFile_.verifyPage( cTrans_ ,iPageID_ ,eMode_ ,cProgress_) )
		{
			_SYDNEY_ASSERT(m_pPage);
		}

		AutoPhysicalPageVerify( PhysicalFile::File* pFile_
		                       ,const Trans::Transaction& cTrans_
		                       ,Buffer::Page::FixMode::Value eMode_
		                       ,Admin::Verification::Progress& cProgress_
		                      )
			: m_cFile( *pFile_ )
			, m_pPage(0)
		{
			_SYDNEY_ASSERT(pFile_);
			m_pPage = pFile_->verifyPage( cTrans_ ,eMode_ ,cProgress_);
			_SYDNEY_ASSERT(m_pPage);
		}

		AutoPhysicalPageVerify( PhysicalFile::File& cFile_
		                       ,const Trans::Transaction& cTrans_
		                       ,Buffer::Page::FixMode::Value eMode_
		                       ,Admin::Verification::Progress& cProgress_
		                      )
			: m_cFile( cFile_ )
			, m_pPage( cFile_.verifyPage( cTrans_ ,eMode_ ,cProgress_) )
		{
			_SYDNEY_ASSERT(m_pPage);
		}

		~AutoPhysicalPageVerify()
		{
			detach();
		}

		PhysicalFile::Page* get()        { return m_pPage; }
		operator PhysicalFile::Page*()   { return get(); }
		PhysicalFile::Page* operator->() { return get(); }
		PhysicalFile::Page& operator*() { return *get(); }

		PhysicalFile::Page* release()
			{ PhysicalFile::Page* ret = get(); m_pPage = 0; return ret; }

		void detach()
			//Verifyモードでオープンしたので必ずNotDirty
			{ if (m_pPage) { m_cFile.detachPage(m_pPage, PhysicalFile::Page::UnfixMode::NotDirty ); m_pPage = 0; } }

	private:
		PhysicalFile::File& m_cFile;
		PhysicalFile::Page* m_pPage;
	};

} // namespace FileCommon


_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_AUTOATTACH_H

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
 
