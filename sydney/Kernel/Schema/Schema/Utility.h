// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.h -- スキーマモジュールで共通に使う便利関数の宣言
// 
// Copyright (c) 2000, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SCHEMA_UTILITY_H
#define __SYDNEY_SCHEMA_UTILITY_H

#include "Schema/Module.h"
#include "Schema/ObjectID.h"

#include "Common/Object.h"
#include "Common/Data.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"

#include "Os/Path.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

// 内部で定義しているクラス
namespace _Binary
{
	class _Memory;
}

_SYDNEY_BEGIN

namespace Common
{
	class BinaryData;
	class Externalizable;
	class InputArchive;
	class OutputArchive;
}

namespace LogicalFile
{
	class File;
	class FileDriver;
}

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;

namespace Utility
{
	// 以下のクラスに対する初期化、後処理を行う
	void					initialize();
	void					terminate();

	//	CLASS
	//	Schema::Utility::InputArchive -- データベースオブジェクトを保持できるアーカイバー
	//
	//	NOTES
	class InputArchive
		: public Common::InputArchive
	{
	public:
		InputArchive(ModSerialIO& cIO_, Database* pDatabase_)
			: Common::InputArchive(cIO_), m_pDatabase(pDatabase_)
		{}
		virtual ~InputArchive()
		{}

		void setDatabase(Database* pDatabase_) {m_pDatabase = pDatabase_;}
		Database* getDatabase() const {return m_pDatabase;}

	private:
		Database* m_pDatabase;
	};

	//	CLASS
	//	Schema::Utility::OutputArchive --
	//
	//	NOTES
	class OutputArchive
		: public Common::OutputArchive
	{
	public:
		OutputArchive(ModSerialIO& cIO_)
			: Common::OutputArchive(cIO_)
		{}
		virtual ~OutputArchive()
		{}
	};

	//	CLASS
	//	Schema::Utility::BinaryData -- BinaryDataを使ったシリアル化を行う
	//
	//	NOTES

	class BinaryData
		: public Common::Object
	{
	public:
		BinaryData();							// コンストラクター
		~BinaryData();							// デストラクター

		// シリアル化してDataとして得る
		Common::Data::Pointer
							put(const Common::Externalizable* pData_,
								bool bCompress_ = false);
		// シリアル化してBinaryDataに格納されているものを戻す
		Common::Externalizable*
							get(const Common::Data::Pointer& cData_);

		// シリアライズされたデータをCommon::Dataとして得る
		Common::Data::Pointer freeze(bool bCompress_ = false);
		// Common::Dataからシリアライズされたデータを展開する
		void				melt(const Common::Data::Pointer& cData_);

		// アーカイブの内容を初期化する
		void				reset();

		// 入出力のアーカイブを得る
		InputArchive&		in(Database* pDatabase_);
		OutputArchive&		out();

	protected:
		// 初期化を行う
		void				initialize();
		void				initializeInput(Database* pDatabase_);
		void				initializeOutput();
		// 終了処理を行う
		void				terminate();

	private:
		_Binary::_Memory*	m_pArchive;			// アーカイブに使うメモリ
		InputArchive*		m_pIn;				// 入力のアーカイブ
		OutputArchive*		m_pOut;				// 出力のアーカイブ

	};

	namespace File
	{
		// ディレクトリーがなかったら親ディレクトリーまでさかのぼって作る
		bool				mkdir(const Os::Path& cPath_, const bool bCheckDir_ = false);
		// ディレクトリーの中にあるものまで含めてすべて削除する
		void				rmAll(const Os::Path& cPath_, bool bForce_ = true);
		void				rmAll(Schema::ObjectID::Value iDatabaseID_, const Os::Path& cPath_, bool bForce_ = true);
		// 指定したパス以外をすべて削除する
		void				rmAllExcept(const Os::Path& cPath_,
										const ModVector<ModUnicodeString*>& vecpExceptPath_,
										bool bForce_ = true);
		// 指定したパスが空のディレクトリーなら削除する
		void				rmEmpty(const Os::Path& cPath_);
		// ディレクトリーの破棄を取り消す
		void				undoRmAll(const Os::Path& cPath_);

#ifdef OBSOLETE
		// 論理ファイルを削除する
		void				destroy(Trans::Transaction& cTrans_,
									LogicalFile::FileDriver* pDriver_,
									LogicalFile::File* pFile_,
									bool bForce_ = true);
#endif
		// 指定したディレクトリのファイル名、ディレクトリ名のリストを取得する
		bool				getFileList(const Os::Path& cSrc_,
										ModVector< ModUnicodeString >& cList_);
		// ファイルまたはディレクトリーを移動する
		void				move(Trans::Transaction& cTrans_,
								 const Os::Path& cSrc_,
								 const Os::Path& cDst_,
								 bool bForce_ = true,
								 bool bRecovery_ = false);
		// パスが存在するか調べる
		bool				isFound(const Os::Path& cPath_);

		//	CLASS
		//	Schema::Utility::File::AutoRmDir --
		//		mkdir後のエラー処理を自動的に行うクラス
		//
		//	NOTES
		class AutoRmDir : public Common::Object
		{
		public:
			AutoRmDir()
				: m_cPath(), m_bRmAll(false), m_bEnable(true)
			{}
			~AutoRmDir()
			{
				if (m_bRmAll && m_bEnable) {
					rmAll(m_cPath);
				}
			}
			// エラー時にrmdirするパスを設定する
			void setDir(const ModUnicodeString& cPath_)
			{
				m_bRmAll = !isFound((m_cPath = Os::Path(cPath_)));
			}
			// エラー時にrmdirするパスを設定する
			void setDir(const Os::Path& cPath_)
			{
				m_bRmAll = !isFound((m_cPath = cPath_));
			}
			// 機能のON/OFF
			void disable() {m_bEnable = false;}
			void enable() {m_bEnable = true;}
		private:
			Os::Path m_cPath;
			bool m_bRmAll; // エラー時にrmAllするかを示す
			bool m_bEnable;
		};

	} // namespace File
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_UTILITY_H

//
//	Copyright (c) 2000, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
