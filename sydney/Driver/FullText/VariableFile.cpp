// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableFile.cpp -- 全文ファイルのその他情報を格納する可変長ファイル
// 
// Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/VariableFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/OpenOption.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"
#include "PhysicalFile/Manager.h"

#include "Os/File.h"
#include "Os/Memory.h"

#include "Schema/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_Directory -- ディレクトリ
	//
	ModUnicodeString _Directory("Variable");

	//
	//	FUNCTION local
	//	_$$::_getSize -- サイズを得る
	//
	ModSize _getSize(const char*& p)
	{
		ModUInt32 s;
		Os::Memory::copy(&s, p, sizeof(ModUInt32));
		p += sizeof(ModUInt32);
		return static_cast<ModSize>(s);
	}

	//
	//	FUNCTION local
	//	_$$::_getSize -- サイズを得る
	//
	ModSize _getSize(const PhysicalFile::DirectArea& area_)
	{
		// エリアの先頭ポインターを得る
		const char* p = syd_reinterpret_cast<const char*>(
			area_.operator const void*());
		
		return _getSize(p);
	}

	//
	//	FUNCTION local
	//	_$$::_setSize -- サイズを設定する
	//
	void _setSize(char*& p, ModSize size)
	{
		ModUInt32 s = static_cast<ModUInt32>(size);
		Os::Memory::copy(p, &s, sizeof(ModUInt32));
		p += sizeof(ModUInt32);
	}

	//
	//	FUNCTION local
	//	_$$::_getAreaID -- エリアIDを得る
	//
	PhysicalFile::DirectArea::ID _getAreaID(const char*& p)
	{
		PhysicalFile::DirectArea::ID id;
		Os::Memory::copy(&id, p, sizeof(PhysicalFile::DirectArea::ID));
		p += sizeof(PhysicalFile::DirectArea::ID);
		return id;
	}

	//
	//	FUNCTION local
	//	_$$::_setAreaID -- エリアIDを設定する
	//
	void _setAreaID(char*& p, const PhysicalFile::DirectArea::ID& id)
	{
		Os::Memory::copy(p, &id, sizeof(PhysicalFile::DirectArea::ID));
		p += sizeof(PhysicalFile::DirectArea::ID);
	}
}

//
//	FUNCTION public
//	FullText::VariableFile::Archiver::Archiver -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText::VariableFile& cFile_
//	  	可変長ファイル
//	const PhysicalFile::DirectArea& cArea_
//		エリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VariableFile::Archiver::Archiver(VariableFile& cFile_,
								 const PhysicalFile::DirectArea& cArea_)
	: m_cFile(cFile_), m_cArea(cArea_)
{
	if (m_cArea.isValid())
		// 先頭のIDとして保存しておく
		m_cAreaID = m_cArea.getID();
}

//
//	FUNCTION public
//	FullText::VariableFile::Archiver::~Archiver -- デストラクタ
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
VariableFile::Archiver::~Archiver()
{
	m_cArea.detach();
}

//
//	FUNCTION public
//	FullText::VariableFile::Archiver::readSerial -- 読み出す
//
//	NOTES
//
//	ARGUMENTS
//	void* buffer_
//		読み込んだものを書き込むバッファ
//	ModSize byte_
//		読み込むサイズ(バイト)
//	ModSerialIO::DataType type_
//		読み込むデータ型(参照していない)
//
//	RETURN
//	int
//		実際に読み込んだサイズ
//
//	EXCEPTIONS
//
int
VariableFile::Archiver::readSerial(void* buffer_, ModSize byte_, DataType type_)
{
	// 読み込んだものを書き込むバッファ
	char* buf = syd_reinterpret_cast<char*>(buffer_);
	
	// エリアの先頭ポインターを得る
	const char* p = syd_reinterpret_cast<const char*>(
		const_cast<const PhysicalFile::DirectArea&>(m_cArea).
		operator const void*());
	// エリアの終端ポインターを得る
	const char* q = p + m_cArea.getSize();

	// 先頭のエリアの先頭には、データ全体のサイズが格納されている
	//
	// <--4byte---><-------------- n byte ------------>
	// [  サイズ  ][          データ                  ]

	// サイズを得る
	ModSize size = _getSize(p);
	if (size != byte_)
	{
		// 予期せぬエラー
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	while (byte_)
	{
		if (static_cast<int>(byte_) > (q - p))
		{
			// 複数のエリアに跨っているエリアは、
			// 以下のようなフォーマットになっている
			//
			// <---------8byte--------><-------- n byte ------->
			// [      次のエリア      ][        データ         ]
			//

			// 次のエリアIDを得る
			PhysicalFile::DirectArea::ID next = _getAreaID(p);

			// 引数のバッファにデータをコピーする
			ModSize s = static_cast<ModSize>(q - p);
			Os::Memory::copy(buf, p, s);
			buf += s;
			byte_ -= s;

			// 次のエリアを割り当てる
			m_cArea.detach();
			m_cArea = m_cFile.attachArea(next);

			// エリアの先頭ポインターを得る
			p = syd_reinterpret_cast<const char*>(
				const_cast<const PhysicalFile::DirectArea&>(m_cArea).
				operator const void*());
			// エリアの終端ポインターを得る
			q = p + m_cArea.getSize();
		}
		else
		{
			// このエリア内で収まっている

			// 引数のバッファにデータをコピーする
			Os::Memory::copy(buf, p, byte_);
			byte_ = 0;
		}
	}

	return static_cast<int>(size);
}

//
//	FUNCTION public
//	FullText::VariableFile::Archiver::writeSerial -- 書き出す
//
//	NOTES
//
//	ARGUMENTS
//	const void* buffer_
//		書き出すものが格納されているバッファ
//	ModSize byte_
//		書き出すサイズ(バイト)
//	ModSerialIO::DataType type_
//		書き出すデータ型(参照していない)
//
//	RETURN
//	int
//		実際に書き出したサイズ
//
//	EXCEPTIONS
//
int
VariableFile::Archiver::writeSerial(const void* buffer_, ModSize byte_,
									DataType type_)
{
	// 書き出すものが格納されているバッファ
	const char* buf = syd_reinterpret_cast<const char*>(buffer_);
	// 書き出すデータサイズ
	ModSize size = byte_;
	// エリアの最大サイズ
	ModSize maxSize = m_cFile.getMaxStorableAreaSize();

	// 書き出すサイズ(引数のサイズ + サイズ格納分)
	ModSize totalSize = byte_ + sizeof(ModUInt32);
	// 最初のエリアに格納するサイズを求める
	ModSize newSize = (totalSize < maxSize) ? totalSize : maxSize;
	
	// エリアを確保する
	m_cArea.detach();
	m_cArea = m_cFile.allocateArea(newSize);
	m_cAreaID = m_cArea.getID();	// エリアIDを保存する
	m_cArea.dirty();
	
	// エリアの先頭ポインターを得る
	char* p = syd_reinterpret_cast<char*>(m_cArea.operator void*());
	// エリアの終端ポインターを得る
	char* q = p + m_cArea.getSize();

	// データのサイズを格納する
	_setSize(p, byte_);

	while (byte_)
	{
		if (static_cast<int>(byte_) > (q - p))
		{
			// 複数のエリアに跨っているエリアは、
			// 以下のようなフォーマットになっている
			//
			// <---------8byte--------><-------- n byte ------->
			// [      次のエリア      ][        データ         ]
			//

			// 新しいエリアのサイズを得る
			newSize = byte_
				- (static_cast<ModSize>(q - p)
				   - sizeof(PhysicalFile::DirectArea::ID));
			if (newSize > maxSize)
				newSize = maxSize;

			// 新しいエリアを得る
			PhysicalFile::DirectArea newArea
				= m_cFile.allocateArea(newSize);
			newArea.dirty();

			// エリアIDを書く
			_setAreaID(p, newArea.getID());
			// データを書く
			ModSize s = static_cast<ModSize>(q - p);
			Os::Memory::copy(p, buf, s);
			buf += s;
			byte_ -= s;

			// エリアの先頭ポインターを得る
			p = syd_reinterpret_cast<char*>(newArea.operator void*());
			// エリアの終端ポインターを得る
			q = p + newArea.getSize();

			// 前のエリアをdetachする
			m_cArea.detach();
			m_cArea = newArea;
		}
		else
		{
			// このエリア内で収まる

			// 引数のバッファの内容をエリアにコピーする
			Os::Memory::copy(p, buf, byte_);
			byte_ = 0;
		}
	}
	// エリアをdetachする
	m_cArea.detach();

	return size;
}

//
//	FUNCTION public
//	FullText::VariableFile::VariableFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VariableFile::VariableFile(FullText::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pPhysicalFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown), m_bVerify(false),
	  m_pProgress(0)
{
	attach();
}

//
//	FUNCTION public
//	FullText::VariableFile::~VariableFile -- デストラクタ
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
VariableFile::~VariableFile()
{
	detach();
}

//
//	FUNCTION public
//	FullText::VariableFile::create -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::create(const Trans::Transaction& cTransaction_)
{
	try
	{
		// 物理ファイルに作成を依頼する
		m_pPhysicalFile->create(*m_pTransaction);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		Os::Directory::remove(m_cPath);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText::VariableFile::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_pPhysicalFile->destroy(cTransaction_);
	ModOsDriver::File::rmAll(m_cPath, ModTrue);
}

//
//	FUNCTION public
//	FullText::VariableFile::recover -- リカバリする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::recover(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pPhysicalFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する

			Os::Directory::remove(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText::VariableFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		修正方法
//	Admin::Verification::Progress& cProgress_
//		状態
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_))
	{
		m_pPhysicalFile->startVerification(cTransaction_,
										   uiTreatment_,
										   cProgress_);
	}

	m_pTransaction = &cTransaction_;
	m_pProgress = &cProgress_;
	m_bVerify = true;
}

//
//	FUNCTION public
//	FullText::VariableFile::endVerification -- 整合性検査を終了する
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
VariableFile::endVerification()
{
	if (m_pTransaction && isMounted(*m_pTransaction))
	{
		try
		{
			m_pPhysicalFile->endVerification(*m_pTransaction,
											 *m_pProgress);
		}
		catch (...)
		{
			m_pTransaction = 0;
			m_pProgress = 0;
			m_bVerify = false;
			_SYDNEY_RETHROW;
		}
	}
	
	m_pTransaction = 0;
	m_pProgress = 0;
	m_bVerify = false;
}

//
//	FUNCTION public
//	FullText::VariableFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::open(const Trans::Transaction& cTransaction_,
				   const LogicalFile::OpenOption& cOption_)
{
	// OpenModeを求める
	int value = cOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	// OpenModeからFixModeを得る
	if (value == LogicalFile::OpenOption::OpenMode::Update ||
		value == LogicalFile::OpenOption::OpenMode::Batch)
	{
		m_eFixMode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Discardable;
	}
	else
	{
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
	}
	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
	m_bVerify = false;
}

//
//	FUNCTION public
//	FullText::VariableFile::close -- ファイルをクローズする
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
VariableFile::close()
{
	flushAllPages();
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	FullText::VariableFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::move(const Trans::Transaction& cTransaction_,
				   const Common::StringArrayData& cArea_)
{
	// 新しいパス
	Os::Path cPath(cArea_.getElement(0));
	cPath.addPart(_Directory);

	// ファイルがあるか
	bool accessible = isAccessible();

	// 一時ファイルか
	bool temporary
		= (m_pPhysicalFile->getBufferingStrategy().
		   m_VersionFileInfo._category == Buffer::Pool::Category::Temporary);
	
	int step = 0;
	try
	{
		Version::File::StorageStrategy::Path cVersionPath;
		cVersionPath._masterData = cPath;
		if (!temporary)
		{
			cVersionPath._versionLog = cPath;
			cVersionPath._syncLog = cPath;
		}
		
		m_pPhysicalFile->move(cTransaction_, cVersionPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			Os::Directory::remove(m_cPath);
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1:
			{
				Version::File::StorageStrategy::Path cVersionPath;
				cVersionPath._masterData = m_cPath;
				if (!temporary)
				{
					cVersionPath._versionLog = m_cPath;
					cVersionPath._syncLog = m_cPath;
				}
				m_pPhysicalFile->move(cTransaction_, cVersionPath);
			}
		case 0:
			if (accessible)
				Os::Directory::remove(cPath);
		}
		_SYDNEY_RETHROW;
	}

	m_cPath = cPath;
}

//
//	FUNCTION public
//	FullText::VariableFile::recoverAllPages -- ページの更新を破棄する
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
VariableFile::recoverAllPages()
{
	m_pPhysicalFile->recoverAllAreas();
}

//
//	FUNCTION public
//	FullText::VariableFile::flushAllPages -- ページの更新を反映する
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
VariableFile::flushAllPages()
{
	m_pPhysicalFile->detachAllAreas();
}

//
//	FUNCTION public
//	FullText::VariableFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::DataArrayData& cValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea::ID
VariableFile::insert(const Common::Data& cValue_)
{
	Archiver archiver(*this);
	cValue_.dumpValue(archiver);
	
	return archiver.getAreaID();
}

//
//	FUNCTION public
//	FullText::VariableFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID id_
//		OID
//	Common::Data& cValue_
//		取得した値を格納するデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::get(const PhysicalFile::DirectArea::ID& id_,
				  Common::Data& cValue_)
{
	// エリアをattachし、サイズを得る
	PhysicalFile::DirectArea area = attachArea(id_);
	ModSize size = _getSize(area);
	// アーカイバーを確保する
	Archiver archiver(*this, area);
	cValue_.setDumpedValue(archiver, size);
	// attachしたエリアをdetachは、アーカイバーのデストラクタで実行される
}

//
//	FUNCTION public
//	FullText::VariableFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID& id_
//		削除するエリアのエリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableFile::expunge(const PhysicalFile::DirectArea::ID& id_)
{
	// 先頭のエリアをattachする
	PhysicalFile::DirectArea cArea = attachArea(id_);
	PhysicalFile::DirectArea::ID cID = id_;
	
	// エリアの先頭ポインターを得る
	const char* p = syd_reinterpret_cast<const char*>(
		const_cast<const PhysicalFile::DirectArea&>(cArea).
		operator const void*());
	// エリアの終端ポインターを得る
	const char* q = p + cArea.getSize();

	// 先頭のエリアの先頭には、データ全体のサイズが格納されている
	//
	// <--4byte---><-------------- n byte ------------>
	// [  サイズ  ][          データ                  ]

	// サイズを得る
	ModSize size = _getSize(p);

	while (size)
	{
		if (static_cast<int>(size) > (q - p))
		{
			// 次のエリアがあるので、エリアIDを得る
			PhysicalFile::DirectArea::ID next = _getAreaID(p);
			// 現在のエリアに格納されるサイズを引く
			size -= static_cast<ModSize>(q - p);
			// 次のエリアをattachする
			cArea.detach();
			cArea = attachArea(next);

			// エリアの先頭ポインターを得る
			p = syd_reinterpret_cast<const char*>(
				const_cast<const PhysicalFile::DirectArea&>(cArea).
				operator const void*());
			// エリアの終端ポインターを得る
			q = p + cArea.getSize();

			// 現在のエリアを削除する
			m_pPhysicalFile->freeArea(*m_pTransaction, cID);

			cID = next;
		}
		else
		{
			// このエリアが最後

			// 現在のエリアを削除する
			m_pPhysicalFile->freeArea(*m_pTransaction, cID);
			size = 0;
		}
	}
}

//
//	FUNCTION public
//	FullText::VariableFile::attachArea -- エリアをattachする
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea::ID& id_
//		attachするエリアのエリアID
//
//	RETURN
//	PhysicalFile:DirectArea
//		attachしたエリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
VariableFile::attachArea(const PhysicalFile::DirectArea::ID& id_)
{
	if (m_bVerify)
	{
		return m_pPhysicalFile->verifyArea(*m_pTransaction,
										   id_,
										   m_eFixMode,
										   *m_pProgress);
	}

	return m_pPhysicalFile->attachArea(*m_pTransaction,
									   id_,
									   m_eFixMode);
}

//
//	FUNCTION public
//	FullText::VariableFile::allocateArea -- エリアをallocateする
//
//	NOTES
//
//	ARGUMENTS
//	ModSize size_
//		allocateするエリアのサイズ
//
//	RETURN
//	PhysicalFile::DirectArea
//		allocateしたエリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
VariableFile::allocateArea(ModSize size_)
{
	return m_pPhysicalFile->allocateArea(*m_pTransaction,
										 size_);
}

//
//	FUNCTION private
//	FullText::VariableFile::attach -- 物理ファイルをアタッチする
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
VariableFile::attach()
{
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//	物理ファイル格納戦略を設定する
	//

	// 物理ファイルタイプ
	cStorageStrategy.m_PhysicalFileType = PhysicalFile::DirectAreaType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = m_cFileID.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = m_cFileID.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize
		= m_cFileID.getVariablePageSize();

	// パスを保存
	m_cPath = m_cFileID.getPath();
	m_cPath.addPart(_Directory);

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy.m_VersionFileInfo._path._masterData = m_cPath;
	if (m_cFileID.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._versionLog = m_cPath;
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._syncLog = m_cPath;
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._masterData
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._masterData
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;

	
	//
	//	物理ファイルバッファリング戦略を設定する
	//
	if (m_cFileID.isTemporary())
	{
		// 一時なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Temporary;
	}
	else if (m_cFileID.isReadOnly())
	{
		// 読み取り専用なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		// その他
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Normal;
	}


	// 物理ファイルをアタッチする
	m_pPhysicalFile
		= PhysicalFile::Manager::attachFile(cStorageStrategy,
											cBufferingStrategy,
											m_cFileID.getLockName());
}

//
//	FUNCTION private
//	FullText::VariableFile::detach -- 物理ファイルをデタッチする
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
VariableFile::detach()
{
	if (m_pPhysicalFile)
	{
		PhysicalFile::Manager::detachFile(m_pPhysicalFile);
		m_pPhysicalFile = 0;
	}
}

//
//	Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
