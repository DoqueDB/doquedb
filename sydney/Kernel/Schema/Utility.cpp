// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.cpp -- スキーマモジュールで共通に使う便利関数の定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Utility.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"

#include "Checkpoint/FileDestroyer.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/CompressedBinaryData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "Exception/Object.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/MetaDatabaseCorrupted.h"

#include "LogicalFile/File.h"
#include "LogicalFile/FileDriver.h"

#include "Os/File.h"
#include "Os/Memory.h"
#include "Os/Path.h"

#include "Trans/Transaction.h"

#include "ModDefaultManager.h"
#include "ModMemory.h"
#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
#ifdef SYD_COVERAGE // カバレージ測定のときはバッファの初期サイズを小さくしておく
const ModSize		_iBufferSizeInitial = 20;
#else
const ModSize		_iBufferSizeInitial = 2048;
#endif
const ModSize		_iBufferSizeMax = ModSizeMax;
											// アーカイブに使うバッファの
											// 最大サイズ
}

namespace _File
{
	//	FUNCTION public
	//	$$::_File::_isDirectory -- パスがディレクトリーか調べる
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		Os::Path&
	//			調べようとしているパス名
	//
	//	RETURN
	//		bool	true  : 指定されたパスはディレクトリーである
	//				false : 指定されたパスはディレクトリーでない、または存在しない
	//
	//	EXCEPTIONS

	inline
	bool _isDirectory(const Os::Path& path_)
	{
		// isFoundしてからGetAttributesするのと、GetAttributesしてから場合によりcatchするのと
		// どちらが得なのか分からないが、try-catchを増やすことを避ける方向を選択した

		return Utility::File::isFound(path_) &&
			ModOsDriver::File::isDirectory(path_) == ModTrue;
	}
}

namespace _Binary
{
	// 足りなくなったら拡大するModMemoryのサブクラス
	class _Memory
		: public ModMemory
	{
	public:
		_Memory()
			: ModMemory(0, 0, houseFill),
			  m_pBuffer(0), m_iBufferSize(0)
		{ }
		virtual ~_Memory()
		{
			Os::Memory::free(m_pBuffer);
		}

		void reserve(ModSize size_, const void* address_ = 0);

	private:
		virtual void	writeOverFlow(void* address_, ModSize size_);
		virtual void	readOverFlow(void* address_, ModSize size_);

		void* m_pBuffer;
		ModSize m_iBufferSize;
	};

	void
	_Memory::reserve(ModSize size_, const void* address_ /* = 0 */)
	{
		void* prevBuffer = m_pBuffer;
		ModSize prevBufferSize = m_iBufferSize;
		ModFileOffset prevPosition = getCurrentPosition();

		if (m_iBufferSize < size_) {

			// 現在のバッファサイズの倍数で要求サイズ以上のものを新サイズにする
			if (!m_iBufferSize) {
				m_iBufferSize = (((size_ - 1) / _iBufferSizeInitial) + 1) * _iBufferSizeInitial;

			} else if (m_iBufferSize > _iBufferSizeMax / 2
				|| m_iBufferSize > _iBufferSizeMax - size_)
				m_iBufferSize = _iBufferSizeMax;
			else
				m_iBufferSize *= (size_ / m_iBufferSize) + 1;

			m_pBuffer = Os::Memory::allocate(m_iBufferSize);
			if (prevBuffer) {
				Os::Memory::copy(m_pBuffer, prevBuffer, prevBufferSize);
				Os::Memory::free(prevBuffer);
			}
		}

		renewalMemory(m_pBuffer, m_iBufferSize);
		if (address_) {
			Os::Memory::copy(m_pBuffer, address_, size_);

		} else {
			currentPosition = static_cast<ModOffset>(prevPosition);
			currentAddress = syd_reinterpret_cast<char*>(m_pBuffer) + currentPosition;
		}
	}

	void
	_Memory::writeOverFlow(void* address_, ModSize size_)
	{
		if (_iBufferSizeMax - size_ < m_iBufferSize) {
			SydErrorMessage << "Too large serialized data." << ModEndl;
			_SYDNEY_THROW0(Exception::MemoryExhaust);
		}
		ModFileOffset prevPosition = getCurrentPosition();
		reserve(m_iBufferSize + size_);
		copy(address_, size_);
	}

	void
	_Memory::readOverFlow(void* address_, ModSize size_)
	{
		// この関数が呼び出されることはファイルが壊れていることを意味する
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}
}

/////////////////////
// Schema::Utility //
/////////////////////

//	FUNCTION public
//	Schema::Utility::initialize -- Utilityの初期化
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Utility::
initialize()
{ }

//	FUNCTION public
//	Schema::Utility::terminate -- Utilityの後処理
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Utility::
terminate()
{ }

/////////////////////////////////
// Schema::Utility::BinaryData //
/////////////////////////////////

//	FUNCTION public
//	Schema::Utility::BinaryData::BinaryData -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Utility::BinaryData::
BinaryData()
	: m_pArchive(0), m_pIn(0), m_pOut(0)
{
}

//	FUNCTION public
//	Schema::Utility::BinaryData::~BinaryData -- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Utility::BinaryData::
~BinaryData()
{
	terminate();
}

//	FUNCTION public
//	Schema::Utility::BinaryData::put --
//		シリアル化したデータを保持するBinaryDataを作る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Externalizable* pData_
//			シリアル化するデータ
//			nullポインター(=0)でもよい
//		bool bCompressed_ = false
//			trueのとき圧縮したデータを返す
//
//	RETURN
//		Common::Data::Pointer
//			シリアル化したデータを保持するData
//
//	EXCEPTIONS
//		Exception::MemoryExhaust
//			シリアル化するデータが異常に大きいためシリアル化できなかった

Common::Data::Pointer
Utility::BinaryData::
put(const Common::Externalizable* pData_, bool bCompress_ /* = false */)
{
	Common::Data::Pointer pResult;

	if (!pData_) {
		// nullポインターならNULLデータにする
		return Common::NullData::getInstance();
	}

	// nullでないならバイナリデータにシリアライズして格納する

	initializeOutput();
	reset();

	// アーカイブに書き込む
	m_pOut->writeObject(pData_);

	// アーカイブの内容からBinaryDataを作る
	return freeze(bCompress_);
}

//	FUNCTION public
//	Schema::Utility::BinaryData::get --
//		シリアル化したデータを保持するBinaryDataからデータを復元する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& cData_
//			シリアル化したデータを保持するData
//
//	RETURN
//		Common::Externalizable*
//			復元されたデータ
//
//	EXCEPTIONS

Common::Externalizable*
Utility::BinaryData::
get(const Common::Data::Pointer& pData_)
{
	if (!pData_.get() || pData_->isNull()) {
		// NullDataならnullを返す
		return 0;
	}

	// BinaryDataをアーカイバーに展開する
	melt(pData_);

	// アーカイブからオブジェクトを得て返り値とする
	return m_pIn->readObject();
}

//	FUNCTION public
//	Schema::Utility::BinaryData::freeze --
//		シリアル化したデータを保持するBinaryDataを作る
//
//	NOTES
//
//	ARGUMENTS
//		bool bCompressed_ = false
//			trueのとき圧縮したデータを返す
//
//	RETURN
//		Common::Data::Pointer
//			シリアル化したデータを保持するData
//
//	EXCEPTIONS

Common::Data::Pointer
Utility::BinaryData::
freeze(bool bCompress_ /* = false */)
{
	if (m_pOut) {
		const void* pAddress =
			static_cast<const void*>(m_pOut->getHeadAddress());
		unsigned int uiSize =
			static_cast<unsigned int>(m_pOut->getSize());

		if (bCompress_) {
			return new Common::CompressedBinaryData(pAddress, uiSize);
		} else {
			return new Common::BinaryData(pAddress, uiSize);
		}
	}
	return Common::NullData::getInstance();
}

//	FUNCTION public
//	Schema::Utility::BinaryData::melt --
//		BinaryDataからシリアル化したデータに展開する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data::Pointer& cData_
//			展開するデータの入ったCommon::Data
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
melt(const Common::Data::Pointer& pData_)
{
	; _SYDNEY_ASSERT(pData_->getType() == Common::DataType::Binary);

	const Common::BinaryData& cData =
		_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, *pData_);

	const void* pAddress = cData.getValue();
	; _SYDNEY_ASSERT(pAddress);

	ModSize iSize = static_cast<ModSize>(cData.getSize());

	// 入力のアーカイブを初期化する
	initializeInput(0);
	reset();
	; _SYDNEY_ASSERT(m_pArchive);
	; _SYDNEY_ASSERT(m_pIn);

	// 必要なサイズを確保し、データをコピーする
	m_pArchive->reserve(iSize, pAddress);
}

//	FUNCTION public
//	Schema::Utility::BinaryData::in --
//		入力アーカイブを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			読み込むものがオブジェクトの場合に使用するデータベース
//
//	RETURN
//		Schema::Utility::InputArchive&
//			入力アーカイブへの参照
//
//	EXCEPTIONS
//		Exception::MemoryExhaust
//			シリアル化するデータが異常に大きいためシリアル化できなかった

Utility::InputArchive&
Utility::BinaryData::
in(Database* pDatabase_)
{
	initializeInput(pDatabase_);
	return *m_pIn;
}

//	FUNCTION public
//	Schema::Utility::BinaryData::out --
//		出力アーカイブを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Utility::OutputArchive&
//			出力アーカイブへの参照
//
//	EXCEPTIONS
//		Exception::MemoryExhaust
//			シリアル化するデータが異常に大きいためシリアル化できなかった

Utility::OutputArchive&
Utility::BinaryData::
out()
{
	initializeOutput();
	return *m_pOut;
}

//	FUNCTION public
//	Schema::Utility::BinaryData::reset --
//		アーカイブの内容を初期化する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
reset()
{
	if (m_pOut) {
		m_pOut->seek(0, ModSerialIO::seekSet);
		m_pOut->resetSize();
	}
	if (m_pIn) {
		m_pIn->seek(0, ModSerialIO::seekSet);
		m_pIn->resetSize();
	}
}

//	FUNCTION protected
//	Schema::Utility::BinaryData::initialize -- BinaryDataに関係する初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
initialize()
{
	if (!m_pArchive) {
		m_pArchive = new _Binary::_Memory;
		m_pArchive->reserve(_iBufferSizeInitial);
	}
	; _SYDNEY_ASSERT(m_pArchive);
}

//	FUNCTION protected
//	Schema::Utility::BinaryData::initializeInput --
//		入力アーカイブの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			入力したオブジェクトが属するデータベース
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
initializeInput(Database* pDatabase_)
{
	initialize();
	; _SYDNEY_ASSERT(m_pArchive);

	if (!m_pIn)
		m_pIn = new InputArchive(*m_pArchive, pDatabase_);
	else
		m_pIn->setDatabase(pDatabase_);
}

//	FUNCTION protected
//	Schema::Utility::BinaryData::initializeOutput --
//		出力アーカイブの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
initializeOutput()
{
	initialize();
	; _SYDNEY_ASSERT(m_pArchive);

	if (!m_pOut)
		m_pOut = new OutputArchive(*m_pArchive);
}

//	FUNCTION protected
//	Schema::Utility::BinaryData::terminate -- BinaryDataに関係する後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::BinaryData::
terminate()
{
	if (m_pArchive)
		delete m_pArchive, m_pArchive = 0;
	if (m_pIn)
		delete m_pIn, m_pIn = 0;
	if (m_pOut)
		delete m_pOut, m_pOut = 0;
}

///////////////////////////
// Schema::Utility::File //
///////////////////////////

//	FUNCTION public
//	Schema::Utility::File::mkdir --
//		ディレクトリーがないときに親ディレクトリーまでさかのぼって作る
//
//	NOTES
//		ModOsDriverの関数ではディレクトリーが存在すると例外になるので
//		この関数を用意した
//
//	ARGUMENTS
//		const Os::Path& path
//			作成するディレクトリーのフルパス名
//
//		const bool doCheck = false
//			作成するディレクトリの存在チェック
//			true 時に存在をチェックし、既にあるなら false を返す
//
//	RETURN
//		bool	true  : bCheckDir_ が true 時、成功
//						それ以外は常に true を返す
//				false : bCheckDir_ が true 時、ディレクトリは既に有る
//
//	EXCEPTIONS

bool
Utility::File::mkdir(const Os::Path& path, const bool doCheck)
{
	initialize();

	if (isFound(path))

		// 指定されたディレクトリが存在する

		return !doCheck;

	// 指定されたディレクトリが存在しないので、生成する
	//
	//【注意】	もし、親ディレクトリがなければ、親ディレクトリから生成する

	Os::Directory::create(path, Os::Directory::Permission::MaskOwner, true);
	return true;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Schema::Utility::File::destroy --
//		ファイルを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作するトランザクション記述子
//		LogicalFile::FileDriver* pDriver_
//			削除するファイルのファイルドライバー
//		LogicalFile::File* pFile_
//			削除するファイル
//		bool bForce_ = true
//			trueの場合システムパラメーターの値に関係なく即座に削除する
//			falseの場合システムパラメーターで指定されない限り猶予を設ける
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
destroy(Trans::Transaction& cTrans_,
		LogicalFile::FileDriver* pDriver_, LogicalFile::File* pFile_,
		bool bForce_)
{
	; _SYDNEY_ASSERT(pDriver_);
	; _SYDNEY_ASSERT(pFile_);

	// ロック機構により使用中のファイルを消すことはないので
	// 常にすぐ消してよい

	try {
		initialize();

		// 論理ファイルに削除を要求する
		pFile_->destroy(cTrans_);

	} catch (ModException& me) {
		switch (me.getErrorNumber()) {
		case ModOsErrorFileNotFound:
			// ファイルが見つからないというエラーは無視する
			ModErrorHandle::reset();
			break;
		default:
			_SYDNEY_RETHROW;
		}
	}

	// ドライバーからのデタッチもここで行う
	pDriver_->detachFile(pFile_);
}
#endif

//	FUNCTION public
//	Schema::Utility::File::rmAll --
//		ディレクトリーの中身まで含めてすべて削除する
//
//	NOTES
//		ディレクトリーが存在しないときは何もしない
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			削除するディレクトリーのフルパス名
//		bool bForce_ = true
//			true  : すぐ削除する
//			false : Checkpoint モジュールに削除依頼する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
rmAll(const Os::Path& cPath_, bool bForce_)
{
#ifdef DEBUG
	SydSchemaDebugMessage << "rmall : " << cPath_ << ModEndl;
#endif

	initialize();

	if (isFound(cPath_)) {

		// DatabaseIDが渡されない場合は常に即座に削除する
		// ディレクトリーの中身も
		// 再帰的に削除するモード
		ModOsDriver::File::rmAll(cPath_, ModTrue);
	}
}

//	FUNCTION public
//	Schema::Utility::File::rmAll --
//		ディレクトリーの中身まで含めてすべて削除する
//
//	NOTES
//		ディレクトリーが存在しないときは何もしない
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			削除するディレクトリーのフルパス名
//		bool bForce_ = true
//			true  : すぐ削除する
//			false : Checkpoint モジュールに削除依頼する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
rmAll(ObjectID::Value iDatabaseID_, const Os::Path& cPath_, bool bForce_)
{
#ifdef DEBUG
	SydSchemaDebugMessage << "rmall : " << cPath_ << ModEndl;
#endif

	initialize();

	if (isFound(cPath_)) {

		if (bForce_)
			// ディレクトリーの中身も
			// 再帰的に削除するモード
			ModOsDriver::File::rmAll(cPath_, ModTrue);
		else
			// Checkpointモジュールに破棄を依頼する
			Checkpoint::FileDestroyer::enter(iDatabaseID_, cPath_, false /* not only dir */);
	}
}

//	FUNCTION public
//	Schema::Utility::File::rmAllExcept --
//		指定したパスを除いてディレクトリーの中身まで含めてすべて削除する
//
//	NOTES
//		ディレクトリーが存在しないときは何もしない
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			削除するディレクトリーのフルパス名
//		const ModVector<ModUnicodeString*>& vecpExceptPath_
//			削除の対象から除外するファイルまたはディレクトリーのフルパス名リスト
//		bool bForce_ = true
//			true  : すぐ削除する
//			false : Checkpoint モジュールに削除依頼する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
rmAllExcept(const Os::Path& cPath_,
			const ModVector<ModUnicodeString*>& vecpExceptPath_,
			bool bForce_)
{
	ModSize n = vecpExceptPath_.getSize();
	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(vecpExceptPath_[i]);
		switch (cPath_.compare(*vecpExceptPath_[i])) {
		case Os::Path::CompareResult::Identical:
		case Os::Path::CompareResult::Child:
			// 同一なら何もしない
			// 除外するパスが親なら何もしない
			// → 関数を抜けてよい
			return;
		case Os::Path::CompareResult::Unrelated:
			// 無関係ならほかの除外パスを調べる
			break;
		case Os::Path::CompareResult::Parent:
			{
				// 除外するパスが子にあるので調べる必要がある
				initialize();

				ModVector<ModUnicodeString> vecFiles;
				if (getFileList(cPath_, vecFiles)) {
					for (ModSize i = 0; i < vecFiles.getSize(); ++i) {
						// サブディレクトリーに除外パスが複数含まれる可能性があるので
						// サブディレクトリーでもすべての除外パスを調べる必要がある
						rmAllExcept(Os::Path(cPath_).addPart(vecFiles[i]),
									vecpExceptPath_, bForce_);
					}
				}
				return;
			}
		}
	}
	// すべて無関係だったので後は調べずにすべて消す
	rmAll(cPath_, bForce_);
}

//	FUNCTION public
//	Schema::Utility::File::rmEmpty --
//		指定したパスが空のディレクトリーなら削除する
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			削除するディレクトリーのフルパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
rmEmpty(const Os::Path& cPath_)
{
	ModVector<ModUnicodeString> cList;
	if (getFileList(cPath_, cList) && cList.isEmpty()) {
		// 空のディレクトリーだったので削除する
		ModOsDriver::File::rmAll(cPath_);
	}
}

// ディレクトリーの破棄を取り消す
void
Utility::File::
undoRmAll(const Os::Path& cPath_)
{
	Checkpoint::FileDestroyer::erase(cPath_);
}

//	FUNCTION public
//	Schema::Utility::File::getFileList --
//		指定したディレクトリのファイル名、ディレクトリ名のリストを取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cSrc_
//			取得元パス名
//		ModVector<ModUnicodeString>& cList_
//			ファイルリスト格納配列
//			cSrc_ 以下の文字列しか入らない
//	RETURN
//		true .. 引数のパスが存在し、ファイルリストが取得できた
//		false.. 引数のパスが存在しない
//
//	EXCEPTIONS
bool
Utility::File::
getFileList(const Os::Path& cSrc_, ModVector<ModUnicodeString>& cList_)
{
	if (!_File::_isDirectory(cSrc_)) {
		// 指定したパスがディレクトリーでないか存在しない場合はやらなくて良い
		return false;
	}

	int i = 0;
	int iMax = 0;

	ModUnicodeString** pEntries = 0;
	ModOsDriver::File::getDirectoryEntry(cSrc_, &pEntries, &iMax);

	try{
		cList_.reserve(iMax);

		// 一覧からファイルリストの作成
		for ( i = 0; i < iMax; ++i ) {

			; _SYDNEY_ASSERT(pEntries[i]);
			cList_.pushBack(*pEntries[i]);

			// 使用したリストの開放
			delete pEntries[i], pEntries[i] = 0;
		}
	}
	catch ( ... ) {
		SydErrorMessage << "get DirectoryEntry failed : " << cSrc_ << ModEndl;

		for ( ; i < iMax; ++i ) delete pEntries[i];

		// 一覧を開放
		ModOsManager::free(pEntries, sizeof(ModUnicodeString*) * iMax);

		throw;
	}

	// 最後に一覧を開放
	ModOsManager::free(pEntries, sizeof(ModUnicodeString*) * iMax);

	return true;
}

//	FUNCTION public
//	Schema::Utility::File::move --
//		ファイルを移動する
//
//	NOTES
//		ディレクトリーは作成されていることを期待する
//		現在は bForce_ の処理は使わないので実装していない。
//		常にすぐ move する。
//		移動後の親ディレクトリーはこの関数呼び出し前に存在していなければならない
//
//	ARGUMENTS
//		const Trans::Trasaction& cTrans_
//			トランザクション記述子
//		const Os::Path& cSrc_
//			移動元ファイル名
//		const Os::Path& cDst_
//			移動先ファイル名
//		bool bForce_ = true
//			true  : すぐに移動する
//			false : チェックポイント処理時に移動する
//		bool  bRecovery_ = false
//			true : 移動前後のパスのどちらにあるか調べる
//			false: 移動後のパスにファイルがあったら例外
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Utility::File::
move(Trans::Transaction& cTrans_,
	 const Os::Path& cSrc_, const Os::Path& cDst_,
	 bool bForce_, bool bRecovery_)
{
#ifdef DEBUG
	SydSchemaDebugMessage << "File move " << cSrc_ << " -> " << cDst_ << ModEndl;
#endif
	
	// 同じパスなら処理しない
	if ( cSrc_.compare(cDst_) == Os::Path::CompareResult::Identical )
		return;

	// 移動元のファイルが存在しなければ何もしない
	if (!isFound(cSrc_))
		return;

	initialize();
	if (_File::_isDirectory(cSrc_)) {
		// 移動元がディレクトリーの場合、再帰的に処理する

		// 移動先のディレクトリーを作成する
		// (存在していてもエラーにならない)
		AutoRmDir distAutoRmDir; // 失敗したら自動的に破棄する
		distAutoRmDir.setDir(cDst_);

		// ディレクトリーの中身を得る
		ModVector<ModUnicodeString> vecFiles;
		if (getFileList(cSrc_, vecFiles)) {
			ModSize n = vecFiles.getSize();
			ModSize iMovedIndex = 0;

			// ディレクトリーの中身を移動中に失敗したら
			// 移動してしまったものについてもとに戻す必要があるので
			// try-catchで囲む
			try {
				for (ModSize i = 0; i < n; ++i) {
					move(cTrans_,
						 Os::Path(cSrc_).addPart(vecFiles[i]),
						 Os::Path(cDst_).addPart(vecFiles[i]),
						 bForce_, bRecovery_);
					// ひとつでも移動したら移動後のパスを破棄できない
					distAutoRmDir.disable();
					iMovedIndex = i + 1;
				}
				// 移動元のディレクトリーを破棄する
				Os::Directory::remove(cSrc_);

			} catch (...) {
				// 移動してしまったものについて元に戻す
				// ただし、bRecovery_==trueのときは
				// エラー処理かUndo処理が行われているので
				// 戻すことはしない

				if (!bRecovery_) {
					for (ModSize i = 0; i < iMovedIndex; ++i) {
						move(cTrans_,
							 Os::Path(cDst_).addPart(vecFiles[i]),
							 Os::Path(cSrc_).addPart(vecFiles[i]),
							 bForce_, true /* エラー処理 */);
					}
					// すべて元に戻したら移動後のパスを破棄できる
					distAutoRmDir.enable();
				}

				_SYDNEY_RETHROW;
			}
		}
	} else {
		// 移動元がファイルの場合、普通に移動する
		if (isFound(cDst_)) {
			// 移動先のファイルが存在する
			if (!bRecovery_) {
				// リカバリー時でなければ例外
				_SYDNEY_THROW1(Exception::FileAlreadyExisted, cDst_);
			}
		}
		// 親ディレクトリを作成する
		// ★注意★
		// ディレクトリーを走査しているときにディレクトリーも作成してしまえばいいが、
		// ディレクトリーだけあってファイルが空のときがあり、その場合はディレクトリーも作らないようにする
		Os::Path cParent;
		if (cDst_.getParent(cParent) && !isFound(cParent)) {
			Os::Directory::create(cParent, Os::Directory::Permission::MaskOwner, true /* recursive */);
		}
		// ファイルを移動する
		Os::File::rename(cSrc_, cDst_, bRecovery_ /* overwrite */);
	}
}

//	FUNCTION public
//	Schema::Utility::File::isFound -- パスが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			調査対象パス名
//
//	RETURN
//		true .. 存在する
//
//	EXCEPTIONS

bool
Utility::File::
isFound(const Os::Path& cPath_)
{
	// MODのisNotFoundよりもOsのaccessのほうが速いのでこちらを使う
	return Os::Directory::access(cPath_, Os::Directory::AccessMode::File);
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
