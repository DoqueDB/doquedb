// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- Ｂ＋木ファイルクラスの実現ファイル
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Btree/File.h"

#include "Exception/BadArgument.h"
#include "Exception/FileNotOpen.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/UniquenessViolation.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Checkpoint/Database.h"
#include "Common/Assert.h"
#ifdef SYD_C_MS6_0
#include "Common/AutoCaller.h"
#endif

#include "PhysicalFile/Manager.h"

#include "LogicalFile/ObjectID.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/AutoAttach.h"

#include "Btree/TreeFile.h"
#include "Btree/ValueFile.h"
#include "Btree/FileInformation.h"
#include "Btree/OpenOptionAnalyzer.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"
#include "Btree/Estimator.h"
#include "Btree/Config.h"
#include "Btree/UseInfo.h"
#include "Btree/Version.h"
#include "Btree/Message_VerifyFailed.h"

#include "Os/File.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	INLINE METHOD
//
//////////////////////////////////////////////////
//
//	FUNCTION private
//	Btree::File::recoverPageAll --
//		ページベクターにキャッシュされている
//		すべての物理ページ記述子を破棄し、
//		ページの内容をアタッチ前の状態に戻す
//
//	NOTES
//		File::recoverPageAll を Node/Value 同時に行う
//	ページベクターにキャッシュされている
//	すべての物理ページ記述子を破棄し、
//	ページの内容をアタッチ前の状態に戻す。
//
//	ARGUMENTS
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
inline void 
File::recoverPageAll( PageVector& NodeAttachPages_, PageVector& ValueAttachPages_)
{
	File::recoverPageAll(*this->m_pTransaction,            this->m_pPhysicalFile,            NodeAttachPages_);
	File::recoverPageAll(*this->m_ValueFile->m_Transaction,this->m_ValueFile->m_PhysicalFile,ValueAttachPages_);
}


//
//	FUNCTION private
//	Btree::File::freePageAll --
//		ページ識別子ベクターにキャッシュされている
//		すべての物理ページ識別子が示す物理ページを解放する
//
//	NOTES
//		File::freePageAll を Node/Value 同時に行う
//	ページ識別子ベクターにキャッシュされている
//	すべての物理ページ識別子が示す物理ページを解放する。
//
//	ARGUMENTS
//	Btree::PageIDVector&		PageIDs_
//		ページ識別子ベクターへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
inline void 
File::freePageAll( PageIDVector& NodePageIDs_, PageIDVector& ValuePageIDs_)
{
	File::freePageAll(*this->m_pTransaction,            this->m_pPhysicalFile,            NodePageIDs_);
	File::freePageAll(*this->m_ValueFile->m_Transaction,this->m_ValueFile->m_PhysicalFile,ValuePageIDs_);
}


//
//	FUNCTION private
//	Btree::File::reusePageAll --
//		ページ識別子ベクターにキャッシュされている
//		すべての物理ページ識別子が示す物理ページを再利用する
//
//	NOTES
//		File::reusePageAll を Node/Value 同時に行う
//	ページ識別子ベクターにキャッシュされている
//	すべての物理ページ識別子が示す物理ページを再利用する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			PhysicalFile_
//		物理ファイル記述子
//	Btree::PageIDVector&		PageIDs_
//		ページ識別子ベクターへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
inline void 
File::reusePageAll( PageIDVector& NodePageIDs_, PageIDVector& ValuePageIDs_)
{
	File::reusePageAll(*this->m_pTransaction,            this->m_pPhysicalFile,            NodePageIDs_);
	File::reusePageAll(*this->m_ValueFile->m_Transaction,this->m_ValueFile->m_PhysicalFile,ValuePageIDs_);
}



//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	可変長フィールド用情報
//

//
//	CONST private
//	Btree::File::UnlimitedFieldLen -- 無制限可変長フィールドを示す値
//
//	NOTES
//	無制限可変長フィールドを示す値。
//
// static
const Os::Memory::Size
File::UnlimitedFieldLen = 0;

//
//	オブジェクトタイプ
//

//
//	CONST private
//	Btree::File::XXXXXObjectType -- オブジェクトタイプ
//
//	NOTES
//	オブジェクトタイプ。
//	v4.0から、各オブジェクトに付加される
//	オブジェクトタイプは1バイトとなった。
//	各オブジェクトタイプに割り当てられるビットは下図の通り。
//
//	　┌─ 代表オブジェクトタイプ
//	　｜
//	┌┼┬─┬─┬─┬─┬─┬─┬─┐
//	│７│６│５│４│３│２│１│０│
//	└─┴┼┴┼┴┼┴┼┴─┴┼┴┼┘
//	　　　│　│　│　│　　　│　│
//	　　　│　│　│　│　　　│　└─ ノーマルオブジェクトタイプ
//	　　　│　│　│　│　　　│
//	　　　│　│　│　│　　　└─── 分割オブジェクトタイプ
//	　　　│　│　│　│
//	　　　│　│　│　│
//	　　　│　│　│　│
//	　　　│　│　│　└─────── 配列オブジェクトタイプ
//	　　　│　│　│
//	　　　│　│　└───────── 分割配列オブジェクトタイプ
//	　　　│　│
//	　　　│　└─────────── 圧縮オブジェクトタイプ
//	　　　│
//	　　　└───────────── 分割圧縮オブジェクトタイプ

//
//	CONST private
//	Btree::File::NormalObjectType -- ノーマルオブジェクトタイプ
//
//	NOTES
//	ノーマルオブジェクトタイプ。
//
// static
const File::ObjectType
File::NormalObjectType = 0x01;

//
//	CONST private
//	Btree::File::DivideObjectType -- 分割オブジェクトタイプ
//
//	NOTES
//	分割オブジェクトタイプ。
//
// static
const File::ObjectType
File::DivideObjectType = 0x02;

//
//	CONST private
//	Btree::File::ArrayObjectType -- 配列オブジェクトタイプ
//
//	NOTES
//	配列オブジェクトタイプ。
//
// static
const File::ObjectType
File::ArrayObjectType = 0x08;

//
//	CONST private
//	Btree::File::DivideArrayObjectType -- 分割配列オブジェクトタイプ
//
//	NOTES
//	分割配列オブジェクトタイプ。
//
// static
const File::ObjectType
File::DivideArrayObjectType = 0x10;

//
//	CONST private
//	Btree::File::CompressedObjectType -- 圧縮オブジェクトタイプ
//
//	NOTES
//	圧縮オブジェクトタイプ。
//	圧縮された可変長フィールドの値を記録するためのオブジェクトのタイプ。
//
// static
const File::ObjectType
File::CompressedObjectType = 0x20;

//
//	CONST private
//	Btree::File::DivideCompressedObjectType -- 分割圧縮オブジェクトタイプ
//
//	NOTES
//	分割圧縮オブジェクトタイプ。
//	圧縮された可変長フィールドの値を記録するためのオブジェクトのタイプ。
//
// static
const File::ObjectType
File::DivideCompressedObjectType = 0x40;

//
//	CONST private
//	Btree::File::DirectObjectType -- 代表オブジェクトタイプ
//
//	NOTES
//	代表オブジェクトタイプ。
//	単独では使用せず、ノーマルオブジェクトタイプと一緒に使用する。
//	また、キーオブジェクトには使用しない。
//	（バリューオブジェクトのみに使用する。）
//	
// static
const File::ObjectType
File::DirectObjectType = 0x80;

//
//	記録サイズ
//

//
//	CONST private
//	Btree::File::ModUInt32ArchiveSize --
//		ModUInt32型変数の記録サイズ
//
//	NOTES
//	ModUInt32型の変数をファイルに記録するために必要なサイズ。[byte]
//
// static
const Os::Memory::Size
File::ModUInt32ArchiveSize =
	FileCommon::DataManager::getModUInt32ArchiveSize();

//
//	CONST private
//	Btree::File::ObjectTypeArchiveSize --
//		オブジェクトタイプの記録サイズ
//
//	NOTES
//	オブジェクトタイプをファイルに記録するために必要なサイズ。[byte]
//
// static
const Os::Memory::Size
File::ObjectTypeArchiveSize = sizeof(File::ObjectType);

//
//	CONST private
//	Btree::File::PageIDArchiveSize --
//		物理ページ識別子の記録サイズ
//
//	NOTES
//	物理ページ識別子をファイルに記録するために必要なサイズ。 [byte]
//
// static
const Os::Memory::Size
File::PageIDArchiveSize = sizeof(PhysicalFile::PageID);

//
//	CONST private
//	Btree::File::ObjectIDArchiveSize --
//		オブジェクトIDの記録サイズ
//
//	NOTES
//	オブジェクトIDをファイルに記録するために必要なサイズ。 [byte]
//
// static
const Os::Memory::Size
File::ObjectIDArchiveSize =
	sizeof(PhysicalFile::PageID) + sizeof(unsigned short);

//
//	CONST private
//	Btree::File::InsideVarFieldLenArchiveSize --
//		外置きではない可変長フィールドのフィールド長の記録サイズ
//
//	NOTES
//	外置きではない（代表オブジェクトに直接フィールド値を記録する）
//	可変長フィールドのフィールド長の記録サイズ。
//
// static
const Os::Memory::Size
File::InsideVarFieldLenArchiveSize = sizeof(File::InsideVarFieldLen);

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::File::File -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイルの論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		※ FileID_に設定されているパラメータに不正があった場合に発生
//		( Btree::FileParameter )
//
File::File(const LogicalFile::FileID& FileID_)
	: LogicalFile::File(),
	  m_pTransaction(0),
	  m_pPhysicalFile(0),
	  m_ValueFile(0),
	  m_pOpenParameter(0),
	  m_Update(false),
	  m_Searched(false),
	  m_ullObjectID(FileCommon::ObjectID::Undefined),
	  m_MarkedObjectID(FileCommon::ObjectID::Undefined),
	  m_FetchOptionData(),
	  m_SavePage(false),
	  m_FixMode(Buffer::Page::FixMode::ReadOnly),
	  m_LeafPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_KeyInfoIndex(ModUInt32Max),
	  m_CatchMemoryExhaust(false),
	  m_SearchHint(),
	  m_FetchHint(),
	  m_cFileParameter(FileID_)
{

	this->m_NodePageFreeSizeMax =
		PhysicalFile::File::getPageDataSize(
			PhysicalFile::AreaManageType,
			this->m_cFileParameter.m_PhysicalPageSize,
			2);
}

//
//	FUNCTION public
//	Btree::File::~File -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
File::~File()
{
	if (m_pPhysicalFile != 0)
	{
		// close する
		close();
	}
	
	if (m_pOpenParameter) delete m_pOpenParameter;
}

//
//	FUNCTION public
//	Btree::File::initialize -- システムを初期化する
//
//	NOTES
//	システムを初期化する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
File::initialize()
{
	;	// do nothing
}

//
//	FUNCTION public
//	Btree::File::terminate -- システムの後処理をする
//
//	NOTES
//	システムの後処理をする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
File::terminate()
{
	;	// do nothing
}

//
//	FUNCTION public
//	Btree::File::getFileID -- ファイルIDを返す
//
//	NOTES
//	ファイルIDを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const LogicalFile::FileID&
//		論理ファイルIDオブジェクトへの参照
//
//	EXCEPTIONS
//	なし
//
//
const LogicalFile::FileID&
File::getFileID() const
{
	return this->m_cFileParameter;
}

//
//	FUNCTION public
//	Btree::File::getSize -- ファイルサイズを返す
//
//	NOTES
//	ファイルサイズを返す。
//	Ｂ＋木ファイルはただ 1 つの物理ファイルからなるので
//	その物理ファイルのサイズを返す。
//	この関数を呼び出す場合、
//	(見積りのために)オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ファイルサイズ [byte]
//
//	EXCEPTIONS
//	FileNotOpen
//		見積りのためにオープンされていない
//	Unexpected
//		予想外のエラー
//		( Btree::Estimator::getSize )
//
ModUInt64
File::getSize() const
{
	if (this->m_pPhysicalFile == 0 /*|| 見積りでなくて良い
		this->m_pOpenParameter->m_bEstimate == false*/)
	{
		// (見積りのために)オープンされていない…

		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	return (isMounted(*m_pTransaction)) ?
		Estimator::getSize(
			m_pTransaction, m_pPhysicalFile, m_ValueFile->m_PhysicalFile) : 0;
}

//
//	FUNCTION public
//	Btree::File::getCount -- 挿入されているオブジェクト数を返す
//
//	NOTES
//	Ｂ＋木ファイルは、自身に挿入されているオブジェクトの総数を返す。
//	この関数を呼び出す場合、目的のＢ＋木ファイルが
//	見積りのためにオープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルが見積りのためにオープンされていない
//	Unexpected
//		予想外のエラー
//		( Btree::Estimator::getCount )
//
ModInt64
File::getCount() const
{
	//
	// Ｂ＋木ファイルが見積りのためにオープンされているかチェック
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate == false)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	return (isMounted(*m_pTransaction)) ?
		Estimator::getCount(m_pTransaction, m_pPhysicalFile) : 0;
}

#ifdef SYD_C_MS6_0
// VC6だと継承関係の判定にバグがあり、
// using宣言ができないため、LogicalFileと同じ定義をする
ModUInt64
File::getSize(const Trans::Transaction& trans_)
{
	// default実装では内部でopenして実行
	LogicalFile::OpenOption cOpenOption;

	// ReadモードでEstimate=trueにしてオープンする
	cOpenOption.setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode, LogicalFile::OpenOption::OpenMode::Read);
	cOpenOption.setBoolean(LogicalFile::OpenOption::KeyNumber::Estimate, true);

	ModInt64 iResult = 0;

	open(trans_, cOpenOption);
	{
		// スコープを抜けたら自動的にcloseする
		Common::AutoCaller0<File> autoCloser(this, &File::close);
		iResult = getSize();
	}

	return iResult;
}
#endif

//
//	FUNCTION public
//	Btree::File::getOverhead -- 検索時のオーバヘッドコストを返す
//
//	NOTES
//	検索時のオーバヘッドコストの概算を秒数で返す。
//	ここでいう検索とは、Ｂ＋木ファイルオープン後の初回オブジェクト取得時
//	（関数 get をopen または reset 後に最初に呼び出した時）に行なう
//	検索である。
//	この関数を呼び出す場合、目的の Ｂ＋木ファイルが
//	見積りのためにオープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		検索時のオーバヘッドコスト [秒]
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルが見積りのためにオープンされていない
//	Unexpected
//		予想外のエラー
//		( Btree::Estimator::getOverhead )
//
double
File::getOverhead() const
{
	//
	// Ｂ＋木ファイルが見積りのためにオープンされているかチェック
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate == false)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	return (isMounted(*m_pTransaction)) ?
		Estimator::getOverhead(m_pTransaction, m_pPhysicalFile,
							   m_cFileParameter, *m_pOpenParameter) : 0.0;
}

//
//	FUNCTION public
//	Btree::File::getProcessCost --
//		オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	オブジェクトへアクセスする際のプロセスコストを返す。
//	プロセスコストはオープンモードにより、異なる。
//	この関数を呼び出す場合、目的のＢ＋木ファイルが
//	見積りのためにオープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		オブジェクトへアクセスする際のプロセスコスト [秒]
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルが見積りのためにオープンされていない
//	Unexpected
//		予想外のエラー
//		( Btree::Estimator::getProcessCost )
//
double
File::getProcessCost() const
{
	//
	// Ｂ＋木ファイルが見積りのためにオープンされているかチェック
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate == false)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	return (isMounted(*m_pTransaction)) ?
		Estimator::getProcessCost(
			m_pTransaction, m_pPhysicalFile, m_ValueFile,
			m_cFileParameter, *m_pOpenParameter) : 0.0;
}

//
//	FUNCTION public
//	Btree::File::getSearchParameter -- 検索オープンパラメータを設定する
//
//	NOTES
//	引数 pCondition_ で示される検索条件での
//	オブジェクト高速検索が可能な場合には、
//	引数 cOpenOption_ に検索オープンパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pCondition_
//		検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&				cOpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		指定した検索条件でオブジェクトの高速検索が可能かどうか
//			true  : 指定した検索条件でオブジェクトの高速検索が可能
//			        ※検索結果が 'φ’になってしまう場合には相当しない（true を返す）
//			false : 指定した検索条件ではオブジェクトの高速検索が不可能（※）
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenOption::Analyzer::getSearchParameter )
//	[YET!]
//
bool
File::getSearchParameter(
	const LogicalFile::TreeNodeInterface*	pCondition_,
	LogicalFile::OpenOption&				cOpenOption_) const
{
	return OpenOptionAnalyzer::getSearchParameter(pCondition_,
												  cOpenOption_,
												  m_cFileParameter);
}

//
//	FUNCTION public
//	Btree::File::getProjectionParameter --
//		プロジェクションオープンパラメータを設定する
//
//	NOTES
//	Ｂ＋木ファイルは、引数 cProjection_ で指定されている
//	1 つ以上のフィールドインデックスを読みとり、
//	オブジェクト取得時には、該当するフィールドのみで
//	オブジェクトを構成するようにオープンオプションを設定する。
//	例えば、
//		・オブジェクトIDフィールド（これは、必ず存在する）
//		・キーフィールド × 2 (key#1, key#2)
//		・バリューフィールド × 3 (value#1, value#2, value#3)
//	で構成されるオブジェクトを挿入するためのＢ＋木ファイルから
//	オブジェクトを取得する際に、key#2, value#3 のみを
//	取得するのであれば、引数 cProjection_ は、下図のように設定する。
//
//	      cProjection_
//	   ┌───────┐
//	   │     ┌──┐ │
//	   │ [0] │  2 │ │
//	   │     ├──┤ │
//	   │ [1] │  5 │ │
//	   │     └──┘ │
//	   └───────┘
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールドインデックス配列オブジェクトへの参照
//	LogicalFile::OpenOption&		cOpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		Ｂ＋木ファイルの場合、すべてのフィールドを返すことが可能なので、
//		常に true を返す。（ true を返せないようなときはエラーである。）
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenOption::Analyzer::getTargetParameter )
//	[YET!]
//
bool
File::getProjectionParameter(
	const Common::IntegerArrayData&	cProjection_,
	LogicalFile::OpenOption&		cOpenOption_) const
{
	return
		OpenOptionAnalyzer::getTargetParameter(cProjection_,
											   cOpenOption_,
											   FileCommon::OpenMode::Read,
											   m_cFileParameter);
}

//
//	FUNCTION public
//	Btree::File::getUpdateParameter -- 更新オープンパラメータを設定する
//
//	NOTES
//	Ｂ＋木ファイルは、引数 cUpdateFields_ で指定されている
//	1 つ以上のフィールドインデックスを読みとり、
//	オブジェクト更新時には、該当するフィールドのみを
//	更新するようにオープンオプションに設定する。
//	ただし、オブジェクトIDの更新はできない。
//	例えば、
//		・オブジェクトIDフィールド（これは、必ずある）
//		・キーフィールド × 2 (key#1, key#2)
//		・バリューフィールド × 3 (value#1, value#2, value#3)
//	で構成されるオブジェクトを挿入するためのＢ＋木ファイルに
//	既に挿入されているオブジェクトを更新する際に、value#1, value#3 のみを
//	更新するのであれば、引数 cUpdateFields_ は、下図のように設定する。
//
//	      cUpdateFields_
//	   ┌───────┐
//	   │     ┌──┐ │
//	   │ [0] │  3 │ │
//	   │     ├──┤ │
//	   │ [1] │  5 │ │
//	   │     └──┘ │
//	   └───────┘
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cUpdateFields_
//		フィールドインデックス配列オブジェクトへの参照
//	LogicalFile::OpenOption&		cOpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		Ｂ＋木ファイルの場合、オブジェクトID以外はすべて更新可能なので、
//		オブジェクトIDフィールドが指定されている場合には false を、
//		そうでないなら true を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenOption::Analyzer::getTargetParameter )
//	[YET!]
//
bool
File::getUpdateParameter(
	const Common::IntegerArrayData&	cUpdateFields_,
	LogicalFile::OpenOption&		cOpenOption_) const
{
	return
		OpenOptionAnalyzer::getTargetParameter(cUpdateFields_,
											   cOpenOption_,
											   FileCommon::OpenMode::Update,
											   m_cFileParameter);
}

//
//	FUNCTION public
//	Btree::File::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	オブジェクト取得時のソート順パラメータを設定する。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	cOrders_
//		引数 cKeys_ で指定されたフィールドのソート順の列への参照
//	LogicalFile::OpenOption&		cOpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		指定されたソート順でオブジェクトを返せる場合には true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenOption::Analyzer::getSortParameter )
//	[YET!]
//
bool
File::getSortParameter(
	const Common::IntegerArrayData&	cKeys_,
	const Common::IntegerArrayData&	cOrders_,
	LogicalFile::OpenOption&		cOpenOption_) const
{
	return OpenOptionAnalyzer::getSortParameter(cKeys_,
											    cOrders_,
											    cOpenOption_,
											    m_cFileParameter);
}

//
//	FUNCTION public
//	Btree::File::create -- Ｂ＋木ファイルを生成する
//
//	NOTES
//	Ｂ＋木ファイルは、物理ファイルを生成し、
//	生成した物理ファイルの初期化を行なう。
//	生成に成功すると、自身が持つ論理ファイルIDオブジェクトへの
//	参照を返す。
//
//	ARGUMENTS
//	const Trans::Transaction& Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	const LogicalFile::FileID&
//		論理ファイルIDオブジェクトへの参照
//
//	EXCEPTIONS
//	Unexpected
//		予想外のエラー
//	[YET!]
//
const LogicalFile::FileID&
File::create(const Trans::Transaction&	Transaction_)
{
	// FileIDを変えるのみ

	//生成直後は mounted
	m_cFileParameter.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), true);
	//バージョンは 0
	m_cFileParameter.setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), FileVersion::CurrentVersion);
	return this->m_cFileParameter;
}

//	FUNCTION private
//	Btree::File::substantiate --  実体を作成する
//
//	NOTES
//
//	ARGUMENTS
// 		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::substantiate()
{
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
		return;

	// 以下、open でセットされているはずである
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(m_pPhysicalFile);
	; _SYDNEY_ASSERT(m_ValueFile);

	PhysicalFile::Page*	headerPage = 0;

	const Os::Path& treeFileDir =
		m_cFileParameter.m_TreeFileStorageStrategy.
		m_VersionFileInfo._path._masterData;

	enum {
		None,
		Create
	} eStatus = None;

	try	{
		// ツリーファイルの実体である物理ファイルを生成する

		m_pPhysicalFile->create(*m_pTransaction);
		eStatus = Create;

		//
		// 以下の物理ページの生成と初期化をおこなう
		//   ・ヘッダページ
		//   ・リーフページ
		//

		// ヘッダページを生成する
		PhysicalFile::PageID	realHeaderPageID =
			m_pPhysicalFile->allocatePage(*m_pTransaction,
									   TreeFile::HeaderPageID);

		; _SYDNEY_ASSERT(realHeaderPageID == TreeFile::HeaderPageID);

		headerPage =
			m_pPhysicalFile->attachPage(*m_pTransaction,
									 TreeFile::HeaderPageID,
									 Buffer::Page::FixMode::Write,
									 Buffer::ReplacementPriority::Middle);

		// ファイル管理情報を初期化する
		FileInformation::initialize(m_pTransaction, headerPage);

		//
		// ※ Ｂ＋木ファイル管理情報に
		//      ・ルートノードの物理ページ識別子
		//      ・先頭リーフページの物理ページ識別子
		//      ・先頭オブジェクトのオブジェクトID
		//    を書き込むため、ここではヘッダページのデタッチはしない。
		//    後でデタッチする。
		//

		// 先頭リーフページを生成する
		PhysicalFile::PageID	topLeafPageID =
			this->createNodePage(true); // リーフページ

		//
		// ヘッダページ内のＢ＋木ファイル管理情報に
		//   ・ルートノードの物理ページ識別子
		//   ・先頭リーフページの物理ページ識別子
		//   ・最終リーフページの物理ページ識別子
		// を書き込む。
		// ※ いずれも、先頭リーフページの物理ページ識別子である。
		//

		FileInformation	fileInfo(m_pTransaction, headerPage);

		fileInfo.writeRootNodePageID(topLeafPageID);
		fileInfo.writeTopLeafPageID(topLeafPageID);
		fileInfo.writeLastLeafPageID(topLeafPageID);
		fileInfo.writeModificationTime();

		m_pPhysicalFile->detachPage(headerPage,
								 PhysicalFile::Page::UnfixMode::Dirty,
								 false);

		headerPage = 0;
	}
	catch (Exception::Object&)
	{
		if (headerPage != 0)
		{
			m_pPhysicalFile->detachPage(
				headerPage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false);
		}
		// 生成済みのツリーファイルを削除
		if (eStatus >= Create)
			m_pPhysicalFile->destroy(*m_pTransaction);
		// サブディレクトリーを破棄する
		//
		//【注意】	サブディレクトリは
		//			実体である物理ファイルの生成時に
		//			必要に応じて生成されるが、
		//			エラー時には削除されないので、
		//			この関数で削除する必要がある
		// ツリーファイルのディレクトリを削除
		rmdirOnError(treeFileDir, &m_cFileParameter);

		// 以下、open() 後の状況と同じ（close()で処理される）
		; _SYDNEY_ASSERT(m_pTransaction);
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		; _SYDNEY_ASSERT(m_ValueFile);
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		// 予想外のエラー…
		if (headerPage != 0)
		{
			m_pPhysicalFile->detachPage(
				headerPage,
				PhysicalFile::Page::UnfixMode::NotDirty,
				false);
		}
		// 生成済みのツリーファイルを削除
		if (eStatus >= Create)
			m_pPhysicalFile->destroy(*m_pTransaction);
		// ツリーファイルのディレクトリを削除
		rmdirOnError(treeFileDir, &m_cFileParameter);

		// 以下、open() 後の状況と同じ（close()で処理される）
		; _SYDNEY_ASSERT(m_pTransaction);
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		; _SYDNEY_ASSERT(m_ValueFile);
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	//
	// バリューファイルを生成する
	//
	try
	{
		m_ValueFile->create();
	}
	catch (Exception::Object&)
	{
		// 生成済みのツリーファイルを削除
		m_pPhysicalFile->destroy(*m_pTransaction);
		// ツリーファイルのディレクトリを削除
		rmdirOnError(treeFileDir, &m_cFileParameter);

		// 以下、open() 後の状況と同じ（close()で処理される）
		; _SYDNEY_ASSERT(m_pTransaction);
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		; _SYDNEY_ASSERT(m_ValueFile);
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		// 生成済みのツリーファイルを削除
		m_pPhysicalFile->destroy(*m_pTransaction);
		// ツリーファイルのディレクトリを削除
		rmdirOnError(treeFileDir, &m_cFileParameter);

		// 以下、open() 後の状況と同じ（close()で処理される）
		; _SYDNEY_ASSERT(m_pTransaction);
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		; _SYDNEY_ASSERT(m_ValueFile);
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

// 以下、close()で処理される
; _SYDNEY_ASSERT(m_pTransaction);
; _SYDNEY_ASSERT(m_pPhysicalFile);
; _SYDNEY_ASSERT(m_ValueFile);
}

//	FUNCTION public
//	Btree::File::destroy -- Ｂ＋木ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	// 物理ファイル格納戦略、バッファリング戦略を得て
	// 物理ファイルをアタッチする
	{
	FileCommon::AutoPhysicalFile file(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	// 物理ファイルを破棄する
	file->destroy(cTransaction_);
	}

	rmdir(m_cFileParameter.m_TreeFileStorageStrategy.
		  m_VersionFileInfo._path._masterData, &m_cFileParameter);

	//
	// バリューファイルを破棄する
	//

	try {
		ValueFile	valueFile(&cTransaction_,
							  &this->m_cFileParameter,
							  this->m_FixMode);
		valueFile.destroy();

	} catch (...) {

		// ツリーファイルは破棄できたのに、
		// バリューファイルを破棄することができなかった…

		Checkpoint::Database::setAvailability(
			m_cFileParameter.m_IDNumber->getLockName(), false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Btree::File::isAccessible -- 実体である OS ファイルが存在するか調べる
//
//	NOTES
//	Ｂ＋木ファイルはオープンされていても、
//	オープンされていなくても構わない。
//
//	ARGUMENTS
//		bool		force
//			true
//				実体である OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				実体である OS ファイルの存在を必要があれば調べる
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

bool
File::isAccessible(bool force) const
{
	return (isOpen()) ?
		m_pPhysicalFile->isAccessible(force) :
		FileCommon::AutoPhysicalFile(
			m_cFileParameter.m_TreeFileStorageStrategy,
			m_cFileParameter.m_BufferingStrategy,
			m_cFileParameter.m_IDNumber->getLockName())->isAccessible(force);
}

//	FUNCTION public
//	Btree::File::isMounted -- マウントされているか調べる
//
//	NOTES
//		B木ファイルはオープンされていなくてもよい
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			マウントされているか調べる
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

bool
File::isMounted(const Trans::Transaction& trans) const
{
	return (isOpen()) ?
		m_pPhysicalFile->isMounted(trans) :
		FileCommon::AutoPhysicalFile(
			m_cFileParameter.m_TreeFileStorageStrategy,
			m_cFileParameter.m_BufferingStrategy,
			m_cFileParameter.m_IDNumber->getLockName())->isMounted(trans);
}

//
//	FUNCTION public
//	Btree::File::clear -- ファイルを空の状態にする
//
//	NOTES
//	ファイルを空の状態にする。
//	ファイル内部の情報を空の状態にするだけで、
//	ファイルサイズが縮小されるわけではない。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::clear(const Trans::Transaction&	Transaction_)
{
	if (!isMounted(Transaction_))
		return;

	// ファイルが利用可能かどうか
	bool	isAvailable = true;

	try
	{
		this->m_pPhysicalFile =
			PhysicalFile::Manager::attachFile(
				this->m_cFileParameter.m_TreeFileStorageStrategy,
				this->m_cFileParameter.m_BufferingStrategy,
				this->m_cFileParameter.m_IDNumber->getLockName());

		this->m_pTransaction = &Transaction_;

		// 物理ファイルを空の状態にする
		this->m_pPhysicalFile->clear(*this->m_pTransaction);

		// 物理ファイルを空にした後で、
		// 何らかの例外をキャッチしてしまったら、
		// 元に戻せないので、利用不可。
		isAvailable = false;

		// ヘッダページを生成する
		PhysicalFile::PageID	realHeaderPageID =
			this->m_pPhysicalFile->allocatePage(*this->m_pTransaction,
												TreeFile::HeaderPageID);

		; _SYDNEY_ASSERT(realHeaderPageID == TreeFile::HeaderPageID);

		PhysicalFile::Page*	headerPage =
			this->m_pPhysicalFile->attachPage(
				*this->m_pTransaction,
				TreeFile::HeaderPageID,
				Buffer::Page::FixMode::Write,
				Buffer::ReplacementPriority::Middle);

		// ファイル管理情報を初期化する
		FileInformation::initialize(this->m_pTransaction, headerPage);

		// 先頭リーフページを生成する
		PhysicalFile::PageID	topLeafPageID =
			this->createNodePage(true); // リーフページ

		FileInformation	fileInfo(this->m_pTransaction, headerPage);

		fileInfo.writeRootNodePageID(topLeafPageID);
		fileInfo.writeTopLeafPageID(topLeafPageID);
		fileInfo.writeLastLeafPageID(topLeafPageID);
		fileInfo.writeModificationTime();

		// ヘッダページをデタッチする
		this->m_pPhysicalFile->detachPage(headerPage,
										  PhysicalFile::Page::UnfixMode::Dirty,
										  false); 

		// 物理ファイルをデタッチする
		PhysicalFile::Manager::detachFile(this->m_pPhysicalFile);

		this->m_pPhysicalFile = 0;
	//移動:28 may '01	this->m_pTransaction = 0;

		// バリューファイルをアタッチする
		ValueFile	valueFile(this->m_pTransaction,
							  &this->m_cFileParameter,
							  Buffer::Page::FixMode::Write);

		// バリューファイルを空の状態にする
		valueFile.clear();
		this->m_pTransaction = 0;//移動:28 may '01
	}
	catch (Exception::Object&)
	{
		if (isAvailable == false)
			Checkpoint::Database::setAvailability(
				m_cFileParameter.m_IDNumber->getLockName(), false);
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		if (isAvailable == false)
			Checkpoint::Database::setAvailability(
				m_cFileParameter.m_IDNumber->getLockName(), false);

		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION public
//	Btree::File::open -- Ｂ＋木ファイルをオープンする
//
//	NOTES
//	Ｂ＋木ファイルをオープンする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const LogicalFile::OpenOption&	OpenOption_
//		Ｂ＋木ファイルオープンオプションへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::File::open )
//	NotSupported
//		Ｂ＋木ファイルでサポートされていない機能が指定された
//		( Btree::File::open )
//	[YET!]
//
void
File::open(const Trans::Transaction&		Transaction_,
		   const LogicalFile::OpenOption&	OpenOption_)
{
	this->open(Transaction_, &OpenOption_, 0);
}

//
//	FUNCTION public
//	Btree::File::open -- Ｂ＋木ファイルをオープンする
//
//	NOTES
//	Ｂ＋木ファイルをオープンする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Btree::OpenParameter&	OpenParameter_
//		Ｂ＋木ファイルオープンパラメータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::File::open )
//	NotSupported
//		Ｂ＋木ファイルでサポートされていない機能が指定された
//		( Btree::File::open )
//	[YET!]
//
void
File::open(const Trans::Transaction&	Transaction_,
		   const OpenParameter&			OpenParameter_)
{
	this->open(Transaction_, 0, &OpenParameter_);
}

//
//	FUNCTION public
//	Btree::File::isOpen -- ファイルがオープンされているかどうかを知らせる
//
//	NOTES
//	ファイルがオープンされているかどうかを呼び出し側へ知らせる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ファイルがオープンされているかどうか
//			true  : オープンされている
//			false : オープンされていない
//
//	EXCEPTIONS
//	なし
//
bool
File::isOpen() const
{
	return this->m_pPhysicalFile != 0;
}

//
//	FUNCTION public
//	Btree::File::resetOpenParameter -- オープンパラメータを設定し直す
//
//	NOTES
//	オープン中にオープンパラメータを設定し直す。
//
//	ARGUMENTS
//	const OpenParameter&	OpenParameter_
//		Ｂ＋木ファイルオープンパラメータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenParameter::OpenParameter )
//	[YET!]
//
void
File::resetOpenParameter(const OpenParameter&	OpenParameter_)
{
	if (m_pPhysicalFile == 0)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	this->m_Searched = false;

	this->m_ullObjectID = FileCommon::ObjectID::Undefined;

	this->m_MarkedObjectID = FileCommon::ObjectID::Undefined;

	this->m_LeafPageID = PhysicalFile::ConstValue::UndefinedPageID;

	this->m_KeyInfoIndex = ModUInt32Max;

	delete this->m_pOpenParameter;
	this->m_pOpenParameter = new OpenParameter(this->m_cFileParameter,
											   OpenParameter_);
}

//
//	FUNCTION public
//	Btree::File::close -- ファイルをクローズする
//
//	NOTES
//	ファイルをクローズする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::close()
{
	if (m_pPhysicalFile == 0)
	{
		// オープンされていない
		return;
	}

	if (this->m_Update)
	{
		// ファイルの更新があった…

		//
		// ファイル管理情報の「最終更新時刻」に
		// 現在の時刻を書き込む。
		//

		FileInformation	fileInfo(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 this->m_FixMode,
								 false);

		fileInfo.writeModificationTime();
	}

	delete m_ValueFile, m_ValueFile = 0;

	// 物理ファイルをデタッチする
	PhysicalFile::Manager::detachFile(m_pPhysicalFile);
	m_pPhysicalFile = 0;

	m_pTransaction = 0;

	delete m_pOpenParameter;
	m_pOpenParameter = 0;

	// Common::Data::copyの仕様が変わったので修正
	//delete this->m_FetchOptionData;
	//this->m_FetchOptionData = 0;
	m_FetchOptionData.setPointer(0);
}

//
//	FUNCTION public
//	Btree::File::startPageCache --
//		物理ページキャッシュの開始を指示する
//
//	NOTES
//	Btree::File::endPageCacheが呼び出されるまでの間、
//	アタッチした物理ページは物理ファイルマネージャで
//	キャッシュしておく。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::startPageCache()
{
	this->m_SavePage = true;
}

//
//	FUNCTION public
//	Btree::File::endPageCache --
//		物理ページキャッシュの終了を指示する
//
//	NOTES
//	物理ページをキャッシュしないようにする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::endPageCache()
{
	; _SYDNEY_ASSERT(this->m_pPhysicalFile != 0);

	// アタッチしている物理ページをすべてデタッチする。

	this->m_pPhysicalFile->detachPageAll();

	; _SYDNEY_ASSERT(this->m_ValueFile != 0);
	; _SYDNEY_ASSERT(this->m_ValueFile->m_PhysicalFile != 0);

	this->m_ValueFile->m_PhysicalFile->detachPageAll();

	this->m_SavePage = false;
}

//
//	FUNCTION public
//	Btree::File::fetch -- 検索条件を設定する
//
//	NOTES
//	検索条件を設定する。
//	実際の検索は次の get で行う。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pOption_
//		オブジェクト取得オプションオブジェクトへのポインタ。
//		Ｂ＋木ファイルの関数File::fetch()のオプションとは、
//		FetchモードでのFetch検索条件である。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルがオープンされていない
//		※ 見積りモードでオープンされている場合でも発生
//	IllegalFileAccess
//		不正なファイルアクセス
//	BadArgument
//		不正な引数
//
void
File::fetch(const Common::DataArrayData*	pOption_)
{
	//
	// Ｂ＋木ファイルがオープンされているかチェック
	// また、見積りのためにオープンされているならば
	// fetch は呼び出してはいけない
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	//
	// オープンモードのチェック
	//
	if (m_pOpenParameter->m_iOpenMode != FileCommon::OpenMode::Read &&
		m_pOpenParameter->m_iOpenMode != FileCommon::OpenMode::Search)
	{
		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	// Read ならば Fetch モードでオープンされているはず
	if (m_pOpenParameter->m_iOpenMode == FileCommon::OpenMode::Read &&
		m_pOpenParameter->m_iReadSubMode != OpenParameter::FetchRead)
	{
		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction)) {

		// 指定された条件を保持しておく

		this->m_FetchOptionData = pOption_->copy();

		m_Searched = false;
	}
}

//
//	FUNCTION public
//	Btree::File::get -- Ｂ＋木ファイルに挿入されているオブジェクトを返す
//
//	NOTES
//	Ｂ＋木ファイルに挿入されているオブジェクトを返す。
//	File::getSerchParameter()やFile::fetch()で指定された条件による検索は
//	初回の呼び出し時に行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data*
//		Ｂ＋木ファイルから読み込んだオブジェクトへのポインタ
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルがオープンされていない
//		※ 見積りモードでオープンされている場合でも発生
//	IllegalFileAccess
//		不正なファイルアクセス
//	BadArgument
//		不正な引数
//	[YET!]
//
//Common::Data*
bool
File::get(Common::DataArrayData* pTuple_)
{
	// getSearchParameter() の時点で既に検索結果が'φ'だと判断できているか。

	; _SYDNEY_ASSERT(m_pTransaction);
	if (!isMounted(*m_pTransaction) || m_SearchHint.m_VoidSearch) {
		return false;
	}
	bool bResult = false;

	if (this->m_Searched == false)
	{
		// 初回呼び出し…

		//
		// オブジェクトを返すための準備ができているかをチェックする。
		// ※ これらのチェックは、初回呼び出し時にするだけで良い。
		//

		//
		// Ｂ＋木ファイルがオープンされているかチェック
		// また、見積りのためにオープンされているならば
		// File::get()は呼び出してはいけない。
		//
		if (this->m_pPhysicalFile == 0 ||
			this->m_pOpenParameter->m_bEstimate)
		{
			throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
		}

		//
		// オープンモードのチェック
		//
		if (m_pOpenParameter->m_iOpenMode != FileCommon::OpenMode::Read &&
			m_pOpenParameter->m_iOpenMode != FileCommon::OpenMode::Search)
		{
			throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
		}

		//
		// オブジェクトを返すための準備ができているかのチェックはここまで。
		//

		//
		// 初回呼び出し時にはファイル管理情報を参照する必要がある。
		// （2回目以降のFile::get()以下の処理では、
		// 　ファイル管理情報に記録されている情報に依存する処理は、
		// 　1つもない。）
		//

		FileInformation	fileInfo(this->m_pTransaction,
								 this->m_pPhysicalFile,
								 this->m_FixMode,
								 this->m_SavePage);

		//
		// オブジェクトが1つも挿入されていないのなら
		// ヌルポインタを返すだけで良い。
		//

		if (fileInfo.readObjectNum() != 0)
		{
			// オブジェクトが挿入されている…

			if (this->m_pOpenParameter->m_bGetByBitSet)
			{
				// ビットセットでオブジェクトを返す…

				; _SYDNEY_ASSERT(pTuple_->getCount() == 1);
				; _SYDNEY_ASSERT(pTuple_->getElement(0)->getType() == Common::DataType::BitSet);

				//
				// Fetchモードは未サポート。
				//
				; _SYDNEY_ASSERT(this->m_FetchOptionData.get() == 0);

				this->m_Searched = true;

				bResult = this->getBitSet(fileInfo, this->m_ValueFile,
										  _SYDNEY_DYNAMIC_CAST(Common::BitSet&, *pTuple_->getElement(0)));
			}
			else
			{
				// 通常の形式でオブジェクトを返す…

				bResult = this->search(fileInfo, this->m_ValueFile, pTuple_);

				//
				// File::getBitSet()がfalseを返すことはないが、
				// File::search()はfalseを返すことがある。
				// （検索条件と一致するオブジェクトが存在しない場合など。）
				// その場合、2回目以降のFile::get()は呼ばれるはずはないが、
				// もし呼ばれたならば、
				// その時点でファイル内のオブジェクト数から
				// 参照しなおす必要がある。
				// したがって、今回オブジェクトを返す場合にのみ、
				// this->m_Searchedをtrueにする。
				// （今回オブジェクトを返すのならば、
				// 　次回File::get()の時点まで
				// 　今回のオブジェクトはそのままの状態で存在することが
				// 　保証されているので、オブジェクト数が0になることは
				// 　ありえない。）
				//

				if (bResult)
				{
					this->m_Searched = true;
				}
			}
		}
	}
	else
	{
		// 2回目以降の呼び出し…

		if (this->m_pOpenParameter->m_bGetByBitSet == false)
		{
			// 通常の形式でオブジェクトを返す…

			if (this->m_pOpenParameter->m_bSortReverse)
			{
				// オブジェクトの挿入ソート順とは逆順でオブジェクトを返す…

				bResult = this->getPrevObject(this->m_ValueFile, pTuple_);
			}
			else
			{
				// オブジェクトの挿入ソート順でオブジェクトを返す…

				bResult = this->getNextObject(this->m_ValueFile, pTuple_);
			}
		}
	}

	return bResult;
}

//
//	FUNCTION public
//	Btree::File::insert -- オブジェクトを挿入する
//
//	NOTES
//	オブジェクトを挿入する。
//
//	ARGUMENTS
//	Common::DataArrayData*	Object_
//		挿入するオブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::insert(Common::DataArrayData*	Object_)
{
	substantiate();

	//
	// 挿入の前処理
	//

	this->insertCheck(Object_);

	this->m_FetchHint.setCondition(Object_,
								   1); // Object_には、
								       // 先頭にオブジェクトIDが
								       // 付いているので、
								       // それを考慮している。

	FileInformation	fileInfo(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 this->m_FixMode,
							 this->m_SavePage);

	bool	savePage = this->m_SavePage;
	this->m_SavePage = true;

	// アタッチしたノードページ／バリューページの
	// 物理ページ記述子をキャッシュするためのベクター
	PageVector	attachNodePages;
	PageVector	attachValuePages;

	attachNodePages.reserve(fileInfo.readTreeDepth() * 2);
	attachValuePages.reserve(4);

	// アロケートしたノードページ／バリューページの
	// 物理ページ識別子をキャッシュするためのベクター
	PageIDVector	allocateNodePageIDs;
	PageIDVector	allocateValuePageIDs;

	this->m_CatchMemoryExhaust = false;

	while (true)
	{
		try
		{
			bool	doUniqueCheck = false;
			if (this->m_cFileParameter.m_UniqueType !=
				FileParameter::UniqueType::NotUnique)
			{
				// オブジェクトがユニーク指定されている…

				//
				// File::insertKeyの中で、
				// ユニークチェックが必要かどうか、
				// 挿入オブジェクトのキーフィールドまたは
				// バリューフィールドのいずれかに
				// ヌル値が記録されていないかどうかをチェックする。
				// （ヌル値のフィールドがあれば、
				// 　即ちそのオブジェクトはユニークなので、
				// 　わざわざユニークチェックをする必要はない。）
				//

				doUniqueCheck =
					(this->hasNullField(Object_) == false);
			}

			//
			// ツリーファイルにキーを挿入
			//

			this->insertKey(fileInfo,
							this->m_ValueFile,
							Object_,
							attachNodePages,
							allocateNodePageIDs,
							attachValuePages,
							doUniqueCheck);

			//
			// バリューファイルにバリューを挿入
			//

			this->m_ullObjectID =
				this->m_ValueFile->insert(Object_,
										  fileInfo.readObjectNum(),
										  this->m_LeafPageID,
										  this->m_KeyInfoIndex,
										  this->m_CatchMemoryExhaust,
										  attachValuePages,
										  allocateValuePageIDs);

			//
			// 挿入の後処理
			//

			// リーフページのキー情報にオブジェクトIDを書き込む
			this->setObjectID(attachNodePages);

			// ファイル管理情報を更新する
			fileInfo.incObjectNum();

			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// MemoryExhaustをキャッチ…

			if (this->m_CatchMemoryExhaust == false)
			{
				// 今回の挿入で初めてMemoryExhaustをキャッチ…

				this->m_CatchMemoryExhaust = true;

				this->m_SavePage = false;

				File::recoverPageAll( attachNodePages, attachValuePages);

				File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);
			}
			else
			{
				// 今回の挿入でMemoryExhaustを2回キャッチ…

				// どうしようもない、もう利用不可…

				Checkpoint::Database::setAvailability(
					m_cFileParameter.m_IDNumber->getLockName(), false);
				_SYDNEY_RETHROW;
			}
		}
		catch (Exception::UniquenessViolation&)
		{
			// ユニークエラーをキャッチ…

#ifdef DEBUG
			SydErrorMessage
				<< "Area[0]          : \""
				<< m_cFileParameter.m_TreeFileStorageStrategy.
						m_VersionFileInfo._path._masterData
				<< "\""
				<< ModEndl;
			SydErrorMessage
				<< "PageSize         : "
				<< this->m_cFileParameter.m_PhysicalPageSize / 1024
				<< " [Kbyte]"
				<< ModEndl;
			SydErrorMessage
				<< "KeyObjectPerNode : "
				<< this->m_cFileParameter.m_KeyPerNode
				<< ModEndl;
			char	uniqueType[32];
			if (this->m_cFileParameter.m_UniqueType ==
				FileParameter::UniqueType::Object)
			{
				ModCharTrait::copy(uniqueType, "Object");
			}
			else if (this->m_cFileParameter.m_UniqueType ==
					 FileParameter::UniqueType::Key)
			{
				ModCharTrait::copy(uniqueType, "KeyField");
			}
			else
			{
				ModCharTrait::copy(uniqueType, "Undefined");
			}
			SydErrorMessage
				<< "Unique           : "
				<< uniqueType
				<< ModEndl;
			SydErrorMessage
				<< "FieldNumber      : "
				<< this->m_cFileParameter.m_FieldNum
				<< ModEndl;
			SydErrorMessage
				<< "KeyFieldNumber   : "
				<< this->m_cFileParameter.m_KeyNum
				<< ModEndl;
			for (int x = 0; x < this->m_cFileParameter.m_FieldNum; x++)
			{
				Common::DataType::Type	fieldType =
					*(this->m_cFileParameter.m_FieldTypeArray + x);

				SydErrorMessage
					<< "FieldType[" << x << "]     : "
					<< static_cast<int>(fieldType)
					<< ModEndl;
			}

#endif

			if (savePage == false)
			{
				this->m_SavePage = false;
			}

			//
			// ユニークエラーをキャッチしたということは、
			// まだ、どの物理ページの内容も更新していない。
			// なので、File::recoverPageAll()を呼び出す必要はない。
			//

			File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
			File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

			_SYDNEY_RETHROW;
		}
		catch (...)
		{
			if (savePage == false)
			{
				this->m_SavePage = false;
			}

			File::recoverPageAll( attachNodePages, attachValuePages);

			File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);

			fileInfo.setRecoverPage();

			_SYDNEY_RETHROW;
		}
	}

	// オブジェクトの先頭フィールドにオブジェクトIDを設定する
	this->setObjectID(Object_);

	if (savePage == false)
	{
		this->m_SavePage = false;
	}

	File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);
	File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);

	this->m_Update = true;
}

//
//	FUNCTION public
//	Btree::File::update -- オブジェクトを更新する
//
//	NOTES
//	引数SearchCondition_と一致するオブジェクトを検索し、
//	引数Object_に設定されている内容（各フィールド値）に更新する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	SearchCondition_
//		更新対象オブジェクトへのポインタ（検索条件）
//	Common::DataArrayData*			Object_
//		更新後のオブジェクトへのポインタ
//		
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルがオープンされていない
//	IllegalFileAccess
//		不正なファイルアクセス
//	BadArgument
//		不正な引数
//
void
File::update(const Common::DataArrayData*	SearchCondition_,
			 Common::DataArrayData*			Object_)
{
	this->update(SearchCondition_, Object_, UpdateSearchTarget::Object);
}

//
//	FUNCTION public
//	Btree::File::update -- オブジェクトを更新する
//
//	NOTES
//	引数SearchCondition_と一致するオブジェクトを検索し、
//	引数Object_に設定されている内容（各フィールド値）に更新する。
//
//	ARGUMENTS
//	const Common::DataArrayData*			SearchCondition_
//		更新対象オブジェクトへのポインタ（検索条件）
//	Common::DataArrayData*					Object_
//		更新後のオブジェクトへのポインタ
//	const Btree::UpdateSearchTarget::Value	SearchTarget_
//		検索対象
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::update(const Common::DataArrayData*		SearchCondition_,
			 Common::DataArrayData*				Object_,
			 const UpdateSearchTarget::Value	SearchTarget_)
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	this->updateCheck(SearchCondition_, Object_);

	FileInformation	fileInfo(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 this->m_FixMode,
							 this->m_SavePage);

	bool	savePage = this->m_SavePage;
	this->m_SavePage = true;

	if (fileInfo.readObjectNum() == 0)
	{
		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	ModUInt32	treeDepth = fileInfo.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = fileInfo.readRootNodePageID();

	this->m_FetchHint.m_SetUpdateSearchTarget = false;

	this->m_FetchHint.setUpdateSearchTarget(
		SearchTarget_ == UpdateSearchTarget::Key);

	// Common::Data::copyの仕様が変わったので修正
	//if (this->m_FetchOptionData != 0)
	//{
	//	delete this->m_FetchOptionData;
	//	this->m_FetchOptionData = 0;
	//}

	this->m_FetchOptionData = SearchCondition_->copy();

	this->m_ullObjectID = FileCommon::ObjectID::Undefined;

	// アタッチしたノードページ／バリューページの
	// 物理ページ記述子をキャッシュするためのベクター
	PageVector	attachNodePages;
	PageVector	attachValuePages;

	// アロケートしたノードページ／バリューページの
	// 物理ページ識別子をキャッシュするためのベクター
	PageIDVector	allocateNodePageIDs;
	PageIDVector	allocateValuePageIDs;
	PageIDVector	freeNodePageIDs;
	PageIDVector	freeValuePageIDs;

	this->m_CatchMemoryExhaust = false;

	try
	{
		this->m_ullObjectID = this->fetchByKey(treeDepth,
											   rootNodePageID,
											   attachNodePages,
											   this->m_ValueFile,
											   attachValuePages);
	}
	catch (Exception::MemoryExhaust&)
	{
		if (this->m_CatchMemoryExhaust == false)
		{
			this->m_CatchMemoryExhaust = true;

			this->m_SavePage = false;

			File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
			File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

			this->m_ullObjectID = this->fetchByKey(treeDepth,
												   rootNodePageID,
												   attachNodePages,
												   this->m_ValueFile,
												   attachValuePages);
		}
		else
		{
			_SYDNEY_RETHROW;
		}
	}
	catch (...)
	{
		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
		File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

		_SYDNEY_RETHROW;
	}

	if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
	{
		// なかった…

		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
		File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	while (true)
	{
		try
		{
			ModAutoPointer<Common::DataArrayData> afterObject;

			this->m_ValueFile->readLeafInfo(this->m_ullObjectID,
											attachValuePages,
											this->m_CatchMemoryExhaust,
											this->m_LeafPageID,
											this->m_KeyInfoIndex);

			afterObject = this->makeUpdateObject(Object_,
												 attachNodePages,
												 this->m_ValueFile,
												 attachValuePages);

			this->m_FetchHint.m_FieldNumber =
				this->m_cFileParameter.m_KeyNum +
				this->m_cFileParameter.m_ValueNum;

			this->m_FetchHint.m_OnlyKey = false;

			this->m_FetchHint.setCondition(afterObject.get(), 1);

			// Common::Data::copyの仕様が変わったので修正
			//delete this->m_FetchOptionData;
			this->m_FetchOptionData = afterObject->copy();

			_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
								 this->m_FetchOptionData.get())->popFront();

			//
			// 更新前後で、オブジェクトを構成するすべてのフィールドが
			// 変わらないのであれば、更新をする必要はない。
			//
			const int iCompareResult = this->compareToFetchCondition(afterObject.get(),
																	 1, // for object ID field
																	 this->m_ullObjectID,
																	 attachNodePages,
																	 this->m_ValueFile,
																	 attachValuePages,
																	 this->m_LeafPageID,
																	 this->m_KeyInfoIndex);
			if ( iCompareResult != 0) // 一致しなければ、!= 0 が返る。（bit0: Key が一致しない，bit1: Value が一致しない）
			{
				// 更新前後で、オブジェクトが異なる…

				if (this->m_cFileParameter.m_UniqueType ==
					FileParameter::UniqueType::Key)
				{
					this->m_FetchHint.m_OnlyKey = true; // Key のみを対象にする。
				}

				if ( this->m_cFileParameter.m_UniqueType != FileParameter::UniqueType::NotUnique //ユニーク指定あり
				  && this->hasNullField(afterObject.get()) == false ) // Null フィールドがない（∵Null フィールドは全てにマッチ）
				{
					// 決定表
					// m_OnlyKey	× × × × ○ ○ ○ ○	×:false,     ○:true
					// isKeyUpdate	× × ○ ○ × × ○ ○	×:false,     ○:true ※iCompareResult & 0x01/*bit0*/も比較する
					// fetchByKey	× ○ × ○ × ○ × ○	×:Undefined, ○:succeeded
					// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
					// unique error	× ○ × ○ × × × ○

					bool bUniqError = false;
					if ( (!this->m_FetchHint.m_OnlyKey) || ((iCompareResult & 0x01/*bit0*/) && this->isKeyUpdate()) ) {
						// 更新後と同じ形式のオブジェクトが既に存在するか
						bUniqError = ( this->fetchByKey(treeDepth,
														rootNodePageID,
														attachNodePages,
														this->m_ValueFile,
														attachValuePages) != FileCommon::ObjectID::Undefined ); // 同一オブジェクトが存在する
					}
					if (bUniqError)
					{
						// オブジェクトを更新すると、
						// ユニークではなくなる…

#ifdef DEBUG
						SydErrorMessage << "Unique error." << ModEndl;
						SydErrorMessage
							<< "Area[0]          : \""
							<< m_cFileParameter.m_TreeFileStorageStrategy.
								m_VersionFileInfo._path._masterData
							<< "\""
							<< ModEndl;
						SydErrorMessage
							<< "PageSize         : "
							<< this->m_cFileParameter.m_PhysicalPageSize / 1024
							<< " [Kbyte]"
							<< ModEndl;
						SydErrorMessage
							<< "KeyObjectPerNode : "
							<< this->m_cFileParameter.m_KeyPerNode
							<< ModEndl;
						char	uniqueType[32];
						if (this->m_cFileParameter.m_UniqueType ==
							FileParameter::UniqueType::Object)
						{
							ModCharTrait::copy(uniqueType, "Object");
						}
						else if (this->m_cFileParameter.m_UniqueType ==
								 FileParameter::UniqueType::Key)
						{
							ModCharTrait::copy(uniqueType, "KeyField");
						}
						else
						{
							ModCharTrait::copy(uniqueType, "Undefined");
						}
						SydErrorMessage
							<< "Unique           : "
							<< uniqueType
							<< ModEndl;
						SydErrorMessage
							<< "FieldNumber      : "
							<< this->m_cFileParameter.m_FieldNum
							<< ModEndl;
						SydErrorMessage
							<< "KeyFieldNumber   : "
							<< this->m_cFileParameter.m_KeyNum
							<< ModEndl;
						for (int x = 0; x < this->m_cFileParameter.m_FieldNum; x++)
						{
							Common::DataType::Type	fieldType =
								*(this->m_cFileParameter.m_FieldTypeArray + x);

							SydErrorMessage
								<< "FieldType[" << x << "]     : "
								<< static_cast<int>(fieldType)
								<< ModEndl;
						}

#endif

						throw Exception::UniquenessViolation(moduleName,
															srcFile,
															__LINE__);
					}
				}

				bool	keyMove = false;

				if (this->isKeyUpdate())
				{
					if (this->isNecessaryMoveKey(afterObject.get(),
												 attachNodePages))
					{
						this->deleteLeafPageKey(fileInfo,
												attachNodePages,
												freeNodePageIDs,
												allocateNodePageIDs,
												this->m_ValueFile,
												attachValuePages);

						this->insertKey(fileInfo,
										this->m_ValueFile,
										afterObject.get(),
										attachNodePages,
										allocateNodePageIDs,
										attachValuePages);

						// リーフページのキー情報に
						// オブジェクトIDを書き込む
						this->setObjectID(attachNodePages);

						keyMove = true;
					}
					else
					{
						this->updateKey(afterObject.get(),
										attachNodePages,
										allocateNodePageIDs,
										freeNodePageIDs);
					}
				}

				if (this->isValueUpdate())
				{
					this->m_ValueFile->update(this->m_ullObjectID,
											  afterObject.get(),
											  this->m_pOpenParameter,
											  this->m_LeafPageID,
											  this->m_KeyInfoIndex,
											  this->m_CatchMemoryExhaust,
											  attachValuePages,
											  allocateValuePageIDs,
											  freeValuePageIDs);
				}
				else if (keyMove)
				{
					PhysicalFile::Page*	dummyPage = 0;

					this->m_ValueFile->update(this->m_ullObjectID,
											  this->m_LeafPageID,
											  this->m_KeyInfoIndex,
											  this->m_CatchMemoryExhaust,
											  dummyPage,
											  attachValuePages);
				}
			}

			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			if (this->m_CatchMemoryExhaust == false)
			{
				this->m_CatchMemoryExhaust = true;

				this->m_SavePage = false;

				File::recoverPageAll( attachNodePages, attachValuePages);

				File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);

				File::reusePageAll( freeNodePageIDs, freeValuePageIDs);
			}
			else
			{
				// 今回の更新でMemoryExhaustを2回キャッチ…

				// どうしようもない、もう利用不可…

				Checkpoint::Database::setAvailability(
					m_cFileParameter.m_IDNumber->getLockName(), false);
				_SYDNEY_RETHROW;
			}
		}
		catch (...)
		{
			if (savePage == false)
			{
				this->m_SavePage = false;
			}

			File::recoverPageAll( attachNodePages, attachValuePages);

			File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);

			File::reusePageAll( freeNodePageIDs, freeValuePageIDs);

			fileInfo.setRecoverPage();

			_SYDNEY_RETHROW;
		}

	} // end while

	if (savePage == false)
	{
		this->m_SavePage = false;
	}

	File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);
	File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);

	this->m_Update = true;
}

//
//	FUNCTION public
//	Btree::File::expunge -- オブジェクトを削除する
//
//	NOTES
//	引数SearchCondition_と一致するオブジェクトを検索し、削除する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	SearchCondition_
//		削除対象オブジェクトへのポインタ（検索条件）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::expunge(const Common::DataArrayData*	SearchCondition_)
{
	this->expunge(SearchCondition_, UpdateSearchTarget::Object);
}

//
//	FUNCTION public
//	Btree::File::expunge -- オブジェクトを削除する
//
//	NOTES
//	引数SearchCondition_と一致するオブジェクトを検索し、削除する。
//
//	ARGUMENTS
//	const Common::DataArrayData*					SearchCondition_
//		削除対象オブジェクトへのポインタ（検索条件）
//	const Btree::File::UpdateSearchTarget::Value	SearchTarget_
//		検索対象
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//

void
File::expunge(const Common::DataArrayData*		SearchCondition_,
			  const UpdateSearchTarget::Value	SearchTarget_)
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	this->expungeCheck(SearchCondition_);

	FileInformation	fileInfo(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 this->m_FixMode,
							 this->m_SavePage);

	bool	savePage = this->m_SavePage;
	this->m_SavePage = true;

	if (fileInfo.readObjectNum() == 0)
	{
		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		throw Exception::IllegalFileAccess(moduleName, srcFile, __LINE__);
	}

	ModUInt32	treeDepth = fileInfo.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = fileInfo.readRootNodePageID();

	if (this->m_FetchHint.m_SetUpdateSearchTarget == false)
	{
		this->m_FetchHint.setUpdateSearchTarget(
			SearchTarget_ == UpdateSearchTarget::Key);
	}

	// Common::Data::copyの仕様が変わったので修正
	//if (this->m_FetchOptionData != 0)
	//{
	//	delete this->m_FetchOptionData;
	//	this->m_FetchOptionData = 0;
	//}

	this->m_FetchOptionData = SearchCondition_->copy();

	this->m_ullObjectID = FileCommon::ObjectID::Undefined;

	// アタッチしたノードページ／バリューページの
	// 物理ページ記述子をキャッシュするためのベクター
	PageVector	attachNodePages;
	PageVector	attachValuePages;

	// アロケートしたノードページ／バリューページの
	// 物理ページ識別子をキャッシュするためのベクター
	PageIDVector	allocateNodePageIDs;
	PageIDVector	allocateValuePageIDs;
	PageIDVector	freeNodePageIDs;
	PageIDVector	freeValuePageIDs;

	this->m_CatchMemoryExhaust = false;

	try
	{
		this->m_ullObjectID = this->fetchByKey(treeDepth,
											   rootNodePageID,
											   attachNodePages,
											   this->m_ValueFile,
											   attachValuePages);
	}
	catch (Exception::MemoryExhaust&)
	{
		if (this->m_CatchMemoryExhaust == false)
		{
			this->m_CatchMemoryExhaust = true;

			this->m_SavePage = false;

			File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
			File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, false);

			this->m_ullObjectID = this->fetchByKey(treeDepth,
												   rootNodePageID,
												   attachNodePages,
												   this->m_ValueFile,
												   attachValuePages);
		}
		else
		{
			_SYDNEY_RETHROW;
		}
	}
	catch (...)
	{
		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
		File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, false);

		_SYDNEY_RETHROW;
	}

	if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
	{
		// なかった…

		if (savePage == false)
		{
			this->m_SavePage = false;
		}

		File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
		File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	while (true)
	{
		try
		{
			this->m_ValueFile->readLeafInfo(this->m_ullObjectID,
											attachValuePages,
											this->m_CatchMemoryExhaust,
											this->m_LeafPageID,
											this->m_KeyInfoIndex);

			; _SYDNEY_ASSERT(this->m_LeafPageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);
			; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

			this->m_ValueFile->expunge(this->m_ullObjectID,
									   fileInfo,
									   this->m_CatchMemoryExhaust,
									   attachValuePages,
									   freeValuePageIDs);

			this->deleteLeafPageKey(fileInfo,
									attachNodePages,
									freeNodePageIDs,
									allocateNodePageIDs,
									this->m_ValueFile,
									attachValuePages);

			// ファイル管理情報を更新する
			fileInfo.decObjectNum();

			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			if (this->m_CatchMemoryExhaust == false)
			{
				this->m_CatchMemoryExhaust = true;

				this->m_SavePage = false;

				File::recoverPageAll( attachNodePages, attachValuePages);

				File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);

				File::reusePageAll( freeNodePageIDs, freeValuePageIDs);
			}
			else
			{
				// 今回の削除でMemoryExhaustを2回キャッチ…

				// どうしようもない、もう利用不可…
				Checkpoint::Database::setAvailability(
					m_cFileParameter.m_IDNumber->getLockName(), false);
				_SYDNEY_RETHROW;
			}
		}
		catch (...)
		{
			if (savePage == false)
			{
				this->m_SavePage = false;
			}

			File::recoverPageAll( attachNodePages, attachValuePages);

			File::freePageAll( allocateNodePageIDs, allocateValuePageIDs);

			File::reusePageAll( freeNodePageIDs, freeValuePageIDs);

			fileInfo.setRecoverPage();

			_SYDNEY_RETHROW;
		}
	}

	if (savePage == false)
	{
		this->m_SavePage = false;
	}

	File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);
	File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);

	this->m_Update = true;
}

//
//	FUNCTION public
//	Btree::File::mark -- 巻き戻しの位置を記録する
//
//	NOTES
//	巻き戻しの位置を記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::mark()
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));
	; _SYDNEY_ASSERT(m_pPhysicalFile);

	// get されていないのに mark が呼ばれることはない ??
	; _SYDNEY_ASSERT(this->m_Searched);
	; _SYDNEY_ASSERT(this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	this->m_MarkedObjectID = m_ullObjectID;
}

//
//	FUNCTION public
//	Btree::File::rewind -- 記録した位置に戻る
//
//	NOTES
//	巻き戻しで記録した位置に戻る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルがオープンされていない
//		( Btree::File::reset )
//
void
File::rewind()
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));
	; _SYDNEY_ASSERT(m_pPhysicalFile);

	// get されていないのに rewind が呼ばれることはない ??
	; _SYDNEY_ASSERT(this->m_Searched);

	// getがnullを返すようになったあとでrewindすることはある。
	// _SYDNEY_ASSERT(this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	// mark されていないのに rewind が呼ばれることはない
	//_SYDNEY_ASSERT(m_MarkedObjectID != FileCommon::ObjectID::Undefined);

	//
	// markされていないのにrewindが呼ばれることはある。
	//
	if (this->m_MarkedObjectID == FileCommon::ObjectID::Undefined)
	{
		this->reset();
	}
	else
	{
		this->m_ullObjectID = this->m_MarkedObjectID;
	}
}

//
//	FUNCTION public
//	Btree::File::reset -- Ｂ＋木ファイルへのカーソルをリセットする
//
//	NOTES
//	Ｂ＋木ファイルへのカーソルをリセットする。
//	Ｂ＋木ファイルにはカーソルという概念はないが、
//	本関数が呼ばれた後、初めて呼ばれる get では、
//	Ｂ＋木ファイルオープン後初めて呼ばれた時と
//	同じ処理を行なうこととなる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		Ｂ＋木ファイルがオープンされていない
//
void
File::reset()
{
	//
	// Ｂ＋木ファイルがオープンされているかチェック
	// また、見積りのためにオープンされているならば
	// reset は呼び出してはいけない
	//

	if (this->m_pPhysicalFile == 0 ||
		this->m_pOpenParameter->m_bEstimate)
	{
		throw Exception::FileNotOpen(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	//
	// 次回呼び出される get 内では
	// open 後の初回 get と同じ処理をすることとなる。
	// （ Scan モードならば、先頭／最終オブジェクトから
	//   Scan し直しとなるし、 Search モードならば、再検索となる、などなど。）
	//

	m_Searched = false;
	m_ullObjectID = FileCommon::ObjectID::Undefined;
	this->m_MarkedObjectID = FileCommon::ObjectID::Undefined;

	this->m_LeafPageID = PhysicalFile::ConstValue::UndefinedPageID;

	this->m_KeyInfoIndex = ModUInt32Max;
}

//
//	FUNCTION public
//	Btree::File::equals -- 比較
//
//	NOTES
//	自身が持つ数値としての論理ファイルIDと、
//	引数 pOther_ が持つそれを比較する。
//
//	ARGUMENTS
//	const Common::Object*	pOther_
//		比較対象オブジェクトへのポインタ
//
//	RETURN
//	bool
//		同じ物理ファイル（ディレクトリ）を指していたら true を、
//		そうでなければ false を返す。
//
//	EXCEPTIONS
//	なし
//
bool
File::equals(const Common::Object*	pOther_) const
{
	if (pOther_ == 0)
	{
		return false;
	}

	const Btree::File*	pOther =
		dynamic_cast<const Btree::File*>(pOther_);

	if (pOther == 0)
	{
		return false;
	}

	// 同じ物理ファイルを指しているかどうか

	return
		this->m_cFileParameter.
			m_TreeFileStorageStrategy.m_VersionFileInfo._path._masterData ==
		pOther->m_cFileParameter.
			m_TreeFileStorageStrategy.m_VersionFileInfo._path._masterData;
}

//
//	Utility
//

//
//	FUNCTION public
//	Btree::File::move -- ファイルを移動する
//
//	NOTES
//  ファイルを移動する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const Common::StringArrayData&	Area_
//		Ｂ＋木ファイル移動先ディレクトリパスの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::move(
	const Trans::Transaction&		Transaction_,
	const Common::StringArrayData&	Area_)
{
	const bool accessible = isAccessible();

	//
	// ツリーファイルの移動先ディレクトリを作成する。
	//

	// 移動先のパス
	// Area[0] しか使用しない
	Os::Path	moveTo(Area_.getElement(0));

	Os::Path	treeFileMoveTo(moveTo);
	treeFileMoveTo.addPart(TreeFile::DirectoryName);

	if (treeFileMoveTo.compare(m_cFileParameter.m_TreeFileStorageStrategy.
							   m_VersionFileInfo._path._masterData) ==
		Os::Path::CompareResult::Identical)
		return;

	//
	// ツリーファイルを移動する。
	//

	PhysicalFile::File*	physicalFile =
		PhysicalFile::Manager::attachFile(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	Version::File::StorageStrategy::Path	treeFileMoveToPath;
	treeFileMoveToPath._masterData = treeFileMoveTo;

	if (this->m_cFileParameter.m_BufferingStrategy.m_VersionFileInfo._category !=
		Buffer::Pool::Category::Temporary)
	{
		treeFileMoveToPath._versionLog = treeFileMoveTo;
		treeFileMoveToPath._syncLog = treeFileMoveTo;
	}

	try
	{
		physicalFile->move(Transaction_, treeFileMoveToPath);
	}
	catch (Exception::Object&)
	{
		PhysicalFile::Manager::detachFile(physicalFile);

		if (accessible)

			// サブディレクトリーを破棄する
			//
			//【注意】	サブディレクトリは
			//			実体である物理ファイルの移動時に
			//			必要に応じて生成されるが、
			//			エラー時には削除されないので、
			//			この関数で削除する必要がある

			rmdirOnError(treeFileMoveTo);

		// 移動先のディレクトリだけが残るのならば、
		// まだファイルは利用可能。
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		PhysicalFile::Manager::detachFile(physicalFile);

		if (accessible)
			rmdirOnError(treeFileMoveTo);

		// 移動先のディレクトリだけが残るのならば、
		// まだファイルは利用可能。
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	//
	// バリューファイルを移動する
	//

	try
	{
		ValueFile	valueFile(&Transaction_,
							  &this->m_cFileParameter,
							  this->m_FixMode);

		valueFile.move(moveTo, accessible);
	}
	catch (Exception::Object&)
	{
		// ツリーファイルを移動前のディレクトリへ戻す

		bool	treeRemoved = false;

		try
		{
			physicalFile->move(
				Transaction_,
				this->m_cFileParameter.m_TreeFileStorageStrategy.m_VersionFileInfo._path);

			treeRemoved = true;
		}
		catch (...)
		{
			// ツリーファイルを移動前のディレクトリに
			// 戻すことができなかった…

			Checkpoint::Database::setAvailability(
				m_cFileParameter.m_IDNumber->getLockName(), false);
		}

		if (treeRemoved && accessible) {
			rmdirOnError(treeFileMoveTo);

			// 移動先のディレクトリだけが残るのならば、
			// まだファイルは利用可能。
		}

		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		bool	treeRemoved = false;

		try
		{
			physicalFile->move(
				Transaction_,
				this->m_cFileParameter.m_TreeFileStorageStrategy.m_VersionFileInfo._path);

			treeRemoved = true;
		}
		catch (...)
		{
			// ツリーファイルを移動前のディレクトリに
			// 戻すことができなかった…

			Checkpoint::Database::setAvailability(
				m_cFileParameter.m_IDNumber->getLockName(), false);
		}

		if (treeRemoved && accessible) {
			rmdirOnError(treeFileMoveTo);

			// 移動先のディレクトリだけが残るのならば、
			// まだファイルは利用可能。
		}

		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	PhysicalFile::Manager::detachFile(physicalFile);

	//
	// 移動前のツリーファイルが格納されていたディレクトリを消去する。
	//

	if (accessible)
		rmdir(m_cFileParameter.m_TreeFileStorageStrategy.
			  m_VersionFileInfo._path._masterData);

	//
	// ファイルパラメータを更新する
	//

	this->m_cFileParameter.changeBtreeFilePath(moveTo);

	//
	// B+木ファイルとしては、Area[0]しか使用しない。
	// しかし、Kernel/SchemaモジュールがArea[1]以降にも
	// 指定することがある。
	// なので、指定されているのならば自身にも設定する。
	// ファイルIDに設定されていれば良く、
	// B+木ファイル側でArea[1]以降を認識する必要はないということ。
	// Btree::File::move()でも引数Area_に複数のエリアが指定されていることが
	// あるはずなので、以下でthis->m_cFileParameterに設定する。
	//

	int	areaNum = Area_.getCount();
	for (int i = 1; i < areaNum; i++)
	{
		ModUnicodeString	paramValue(Area_.getElement(i));
		this->m_cFileParameter.setString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, i), paramValue);
	}

	return;
}

// ラッチが不要なオペレーションを返す
//virtual
LogicalFile::File::Operation::Value
File::
getNoLatchOperation()
{
	// Open, Close, Resetだけ不要
	return Operation::Open
		| Operation::Close
		| Operation::Reset;
}

//	FUNCTION public
//	Btree::File::toString -- ファイルの文字列表現を返す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルの文字列表現
//
//	EXCEPTIONS

ModUnicodeString
File::toString() const
{
	return m_cFileParameter.m_TreeFileStorageStrategy.
		m_VersionFileInfo._path._masterData;
}

//	FUNCTION public
//	Btree::File::mount -- B+木ファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	const LogicalFile::FileID&
//		論理ファイルIDオブジェクトへの参照
//
//	EXCEPTIONS

const LogicalFile::FileID&
File::mount(const Trans::Transaction&	Transaction_)
{
	if (!isMounted(Transaction_)) {

		// マウントされていなければ、マウントしてみる
		{
			FileCommon::AutoPhysicalFile file(
				m_cFileParameter.m_TreeFileStorageStrategy,
				m_cFileParameter.m_BufferingStrategy,
				m_cFileParameter.m_IDNumber->getLockName());

			ValueFile valueFile(&Transaction_, &m_cFileParameter);

			int st = 0;
			try {
				file->mount(Transaction_);
				++st;//1
				valueFile.mount();
				++st;//2
			} catch (...) {
				switch (st) {
				case 2:
					valueFile.unmount();
					//fall thru
				case 1:
					file->unmount(Transaction_);
					//fall thru
				case 0:
				default:
					;//nop
				}
				_SYDNEY_RETHROW;
			}
		}

		// マウントされたことをファイル識別子に記録する

		m_cFileParameter.setBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(
				FileCommon::FileOption::Mounted::Key), true);
		m_cFileParameter.
			m_TreeFileStorageStrategy.m_VersionFileInfo._mounted = true;
		m_cFileParameter.
			m_ValueFileStorageStrategy.m_VersionFileInfo._mounted = true;
	}
	return m_cFileParameter;
}

//	FUNCTION public
//	Btree::File::unmount -- B+木ファイルをアンマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	const LogicalFile::FileID&
//		論理ファイルIDオブジェクトへの参照
//
//	EXCEPTIONS

const LogicalFile::FileID&
File::unmount(const Trans::Transaction&	Transaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない
	{
		FileCommon::AutoPhysicalFile file(
			m_cFileParameter.m_TreeFileStorageStrategy,
			m_cFileParameter.m_BufferingStrategy,
			m_cFileParameter.m_IDNumber->getLockName());

		ValueFile valueFile(&Transaction_, &m_cFileParameter);

		int st = 0;
		try {
			file->unmount(Transaction_);
			++st;//1
			valueFile.unmount();
			++st;//2
		} catch (...) {
			switch (st) {
			case 2:
				valueFile.mount();
				//fall thru
			case 1:
				file->mount(Transaction_);
				//fall thru
			case 0:
			default:
				;//nop
			}
			_SYDNEY_RETHROW;
		}
	}

	// アンマウントされたことをファイル識別子に記録する

	m_cFileParameter.setBoolean(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
		false);
	m_cFileParameter.
		m_TreeFileStorageStrategy.m_VersionFileInfo._mounted = false;
	m_cFileParameter.
		m_ValueFileStorageStrategy.m_VersionFileInfo._mounted = false;

	return m_cFileParameter;
}

//
//	FUNCTION public
//	Btree::File::flush --
//		B+木ファイルをフラッシュする
//
//	NOTES
//	B+木ファイルをフラッシュする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::flush(const Trans::Transaction&	Transaction_)
{
	if (!isMounted(Transaction_))
		return;
	{
	FileCommon::AutoPhysicalFile file(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	file->flush(Transaction_);
	}

	ValueFile	valueFile(&Transaction_, &this->m_cFileParameter);

	valueFile.flush();
}

//
//	FUNCTION public
//	Btree::File::startBackup --
//		B+木ファイルに対してバックアップ開始を通知する
//
//	NOTES
//	B+木ファイルに対してバックアップ開始を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const bool					Restorable_
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        読取専用トランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::startBackup(const Trans::Transaction&	Transaction_,
				  const bool				Restorable_)
{
	if (!isMounted(Transaction_))
		return;

	FileCommon::AutoPhysicalFile file(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	ValueFile	valueFile(&Transaction_, &this->m_cFileParameter);

	int st = 0;
	try {
		file->startBackup(Transaction_, Restorable_);
		++st;//1
		valueFile.startBackup(Restorable_);
		++st;//2
	} catch (...) {
		switch (st) {
		case 2:
			valueFile.endBackup();
			//fall thru
		case 1:
			file->endBackup(Transaction_);
			//fall thru
		case 0:
		default:
			;//nop
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree::File::endBackup --
//		B+木ファイルに対してバックアップ終了を通知する
//
//	NOTES
//	B+木ファイルに対してバックアップ終了を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::endBackup(const Trans::Transaction&	Transaction_)
{
	if (!isMounted(Transaction_))
		return;
	{
	FileCommon::AutoPhysicalFile file(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	file->endBackup(Transaction_);
	}

	ValueFile	valueFile(&Transaction_, &this->m_cFileParameter);

	valueFile.endBackup();
}

//	FUNCTION public
//	Btree::File::recover -- B+木ファイルを障害回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::recover(const Trans::Transaction&	Transaction_,
			  const Trans::TimeStamp&	Point_)
{
	if (isMounted(Transaction_)) {
		{
		FileCommon::AutoPhysicalFile file(
			m_cFileParameter.m_TreeFileStorageStrategy,
			m_cFileParameter.m_BufferingStrategy,
			m_cFileParameter.m_IDNumber->getLockName());

		file->recover(Transaction_, Point_);
		}

		ValueFile valueFile(&Transaction_, &m_cFileParameter);
		valueFile.recover(Point_);

		if (!isAccessible()) {

			// リカバリの結果、
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトリを削除する

			rmdir(m_cFileParameter.m_TreeFileStorageStrategy.
				  m_VersionFileInfo._path._masterData, &m_cFileParameter);
			rmdir(m_cFileParameter.m_ValueFileStorageStrategy.
				  m_VersionFileInfo._path._masterData, &m_cFileParameter);
		}
	}
}

//
//	FUNCTION public
//	Btree::File::restore --
//		あるタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	NOTES
//	あるタイムスタンプの表す時点に開始された
//	読取専用トランザクションの参照する版が
//	最新版になるようにバージョンファイルを変更する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		このタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::restore(const Trans::Transaction&	Transaction_,
			  const Trans::TimeStamp&	Point_)
{
	if (!isMounted(Transaction_))
		return;
	{
	FileCommon::AutoPhysicalFile file(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	file->restore(Transaction_, Point_);
	}

	ValueFile	valueFile(&Transaction_, &this->m_cFileParameter);

	valueFile.restore(Point_);
}

//	FUNCTION public
//	Btree::File::sync -- B木ファイルの同期を取る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			B木ファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でB木ファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でB木ファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、B木ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でB木ファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でB木ファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、B木ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::sync(const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	if (isMounted(trans)) {
		FileCommon::AutoPhysicalFile(
			m_cFileParameter.m_TreeFileStorageStrategy,
			m_cFileParameter.m_BufferingStrategy,
			m_cFileParameter.m_IDNumber->getLockName())->
			sync(trans, incomplete, modified);

		ValueFile(&trans, &m_cFileParameter).sync(incomplete, modified);
	}
}

//
//	FUNCTION public
//	Btree::File::verify -- 整合性検査を行う
//
//	NOTES
//	整合性検査を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const unsigned int				Treatment_
//		整合性検査の検査方法
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::verify(const Trans::Transaction&		Transaction_,
			 const unsigned int				Treatment_,
			 Admin::Verification::Progress&	Progress_)
{
	if (!isMounted(Transaction_))
		return;

	if (Progress_.isGood() == false)
	{
		return;
	}

	// ツリーファイルをアタッチする
	PhysicalFile::File*	treePhysicalFile =
		PhysicalFile::Manager::attachFile(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	// バリューファイルをアタッチする
	ValueFile	valueFile(&Transaction_,
						  &this->m_cFileParameter,
						  this->m_FixMode);

	bool	treePhysicalFileStarted = false;
	bool	valuePhysicalFileStarted = false;

	PhysicalFile::Page*	headerPage = 0;

	try
	{
		UseInfo	treeFileUseInfo;
		UseInfo	valueFileUseInfo;

		//
		// まずは、物理ファイルマネージャに、整合性検査開始を指示する。
		// ★ できる限り下位モジュールから順に検査を実施するために、
		// 　 これが最初に行うべきことである。
		//

		treePhysicalFile->startVerification(Transaction_,
											Treatment_,
											Progress_);
		treePhysicalFileStarted = true;

		if (Progress_.isGood())
		{
			valueFile.m_PhysicalFile->startVerification(Transaction_,
														Treatment_,
														Progress_);
			valuePhysicalFileStarted = true;

			if (Progress_.isGood())
			{
				//
				// Ｂ＋木ファイル内で使用しているすべての物理ページと
				// 物理エリアを登録する。
				//

				headerPage =
					treePhysicalFile->verifyPage(
						Transaction_,
						TreeFile::HeaderPageID,
						Buffer::Page::FixMode::ReadOnly,
												 Progress_);
			}
		}

		if (Progress_.isGood() == false)
		{
			treePhysicalFile->endVerification(Transaction_,
											  Progress_);
			treePhysicalFileStarted = false;

			if (valuePhysicalFileStarted)
			{
				valueFile.m_PhysicalFile->endVerification(Transaction_,
														  Progress_);
				valuePhysicalFileStarted = false;
			}

			// アタッチした物理ファイルをデタッチする
			// ※ 明示的にバリューファイルの物理ファイルをデタッチする
			// 　 必要はない。
			// 　 バリューファイル記述子のデストラクタ内でやってくれるから。

			PhysicalFile::Manager::detachFile(treePhysicalFile);

			return;
		}

		// ここにくるのはisGoodのときだけ。

		; _SYDNEY_ASSERT(headerPage != 0);

		FileInformation	fileInfo(&Transaction_, headerPage);

		this->setUseInfo(Transaction_,
						 treePhysicalFile,
						 fileInfo,
						 treeFileUseInfo,
						 &valueFile,
						 valueFileUseInfo,
						 Progress_);

		treePhysicalFile->detachPage(headerPage,
									 PhysicalFile::Page::UnfixMode::NotDirty);
		headerPage = 0;

		if (Progress_.isGood() == false)
		{
			treePhysicalFile->endVerification(Transaction_,
											  Progress_);
			treePhysicalFileStarted = false;

			valueFile.m_PhysicalFile->endVerification(Transaction_,
													  Progress_);
			valuePhysicalFileStarted = false;

			// アタッチした物理ファイルをデタッチする

			PhysicalFile::Manager::detachFile(treePhysicalFile);

			return;
		}

		//
		// 物理ファイルマネージャに、
		// Ｂ＋木ファイル内で使用している
		// すべての物理ページと物理エリアを
		// 通知する。
		//

		if (Progress_.isGood())
		{
			this->notifyUsePageAndArea(Transaction_,
									   treePhysicalFile,
									   treeFileUseInfo,
									   Progress_);
		}

		if (Progress_.isGood())
		{
			this->notifyUsePageAndArea(Transaction_,
									   valueFile.m_PhysicalFile,
									   valueFileUseInfo,
									   Progress_);
		}

		//
		// 物理ファイルマネージャに、整合性検査終了を指示する。
		//

		//
		// statusがどうなっていようと、
		// PhysicalFile::File::endVerificationは、呼ばなければ。
		//

		treePhysicalFile->endVerification(Transaction_,
										  Progress_);
		treePhysicalFileStarted = false;

		if (valuePhysicalFileStarted)
		{
			valueFile.m_PhysicalFile->endVerification(Transaction_,
													  Progress_);
			valuePhysicalFileStarted = false;
		}

		if (Progress_.isGood())
		{
			FileInformation	fileInfo(&Transaction_,
									 treePhysicalFile,
									 Buffer::Page::FixMode::ReadOnly,
									 false); // ヘッダページを
									         // 物理ファイルマネージャで
									         // キャッシュしない

			//
			// Ｂ＋木ファイル内の整合性検査を行う。
			//

			//
			// 下位メソッドで、自身のメンバの
			// トランザクションと物理ファイルの記述子を
			// 使用するため、ここでコピーする。
			//

			this->m_pTransaction = &Transaction_;
			this->m_pPhysicalFile = treePhysicalFile;
			this->m_ValueFile = &valueFile;

			this->checkBtreeFile(fileInfo,
								 &valueFile,
								 Progress_);

			this->m_pTransaction = 0;
			this->m_pPhysicalFile = 0;
			this->m_ValueFile = 0;
		}
	}
	catch (...)
	{
		if (headerPage != 0)
		{
			treePhysicalFile->detachPage(
				headerPage,
				PhysicalFile::Page::UnfixMode::NotDirty);

			headerPage = 0;
		}

		if (treePhysicalFileStarted)
		{
			treePhysicalFile->endVerification(Transaction_, Progress_);
		}

		if (valuePhysicalFileStarted)
		{
			valueFile.m_PhysicalFile->endVerification(Transaction_,
													  Progress_);
		}

		// アタッチした物理ファイルをデタッチする

		PhysicalFile::Manager::detachFile(treePhysicalFile);

		ModUnicodeString	areaPath;
		this->m_cFileParameter.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), areaPath);

		_SYDNEY_VERIFY_ABORTED(Progress_, areaPath, Message::VerifyFailed());

		_SYDNEY_RETHROW;
	}

	// アタッチした物理ファイルをデタッチする

	PhysicalFile::Manager::detachFile(treePhysicalFile);
}

// for debug
#ifdef DEBUG
void
File::setRoute(ModUInt64					ValueObjectID_,
			   Common::IntegerArrayData&	Route_) const
{
	PhysicalFile::PageID	leafPageID =
		PhysicalFile::ConstValue::UndefinedPageID;

	ModUInt32	keyInfoIndex = ModUInt32Max;

	this->m_ValueFile->readLeafInfo(ValueObjectID_,
									leafPageID,
									keyInfoIndex);

	FileInformation	fileInfo(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 Buffer::Page::FixMode::ReadOnly,
							 false);

	ModUInt32	treeDepth = fileInfo.readTreeDepth();

	int	arrayIndex = treeDepth - 1;

	Route_.setElement(arrayIndex, static_cast<int>(leafPageID));

	arrayIndex--;

	bool	isLeafPage = true;

	PhysicalFile::PageID	nodePageID = leafPageID;

	for (; treeDepth > 1; treeDepth--, arrayIndex--)
	{
		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   this->m_pPhysicalFile,
									   nodePageID,
									   Buffer::Page::FixMode::ReadOnly,
									   isLeafPage,
									   this->m_SavePage);

		nodePageID = nodePageHeader.readParentNodePageID();

		; _SYDNEY_ASSERT(
			nodePageID != PhysicalFile::ConstValue::UndefinedPageID);

		Route_.setElement(arrayIndex, nodePageID);

		isLeafPage = false;
	}
}
#endif

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::open -- Ｂ＋木ファイルをオープンする
//
//	NOTES
//	Ｂ＋木ファイルをオープンする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const LogicalFile::OpenOption*	OpenOption_
//		Ｂ＋木ファイルオープンオプションへのポインタ
//	const Btree::OpenParameter*	OpenParameter_
//		Ｂ＋木ファイルオープンパラメータへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenParameter::OpenParameter )
//	NotSupported
//		Ｂ＋木ファイルでサポートされていない機能が指定された
//		( Btree::OpenParameter::OpenParameter )
//	[YET!]
//
void
File::open(const Trans::Transaction&		Transaction_,
		   const LogicalFile::OpenOption*	OpenOption_,
		   const OpenParameter*				OpenParameter_)
{
	if (this->m_pPhysicalFile != 0)
	{
		// 既にオープンされている…

		// オープンの回数は1回まで。
		// それ以上はサポートしていない。

		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	// トランザクション記述子を保持する
	this->m_pTransaction = &Transaction_;

	// オープンパラメータを設定する
	if (OpenParameter_ == 0)
	{
		; _SYDNEY_ASSERT(OpenOption_ != 0);

		this->m_pOpenParameter = new OpenParameter(this->m_cFileParameter,
												   *OpenOption_);
	}
	else
	{
		this->m_pOpenParameter = new OpenParameter(this->m_cFileParameter,
												   *OpenParameter_);
	}

	if (this->m_pOpenParameter->m_iOpenMode ==
		FileCommon::OpenMode::Initialize)
	{
		// Initializeモードでオープンされた場合、
		// 物理ファイルをアタッチしない。
		// （オープン時の処理として何もしない。）

		return;
	}

	// 物理ページのフィックスモードを設定する
	this->m_FixMode =
		(this->m_pOpenParameter->m_iOpenMode ==
		 FileCommon::OpenMode::Update) ?
			Buffer::Page::FixMode::Write |
				Buffer::Page::FixMode::Discardable :
			Buffer::Page::FixMode::ReadOnly;

	// 物理ファイルをアタッチする
	this->m_pPhysicalFile =
		PhysicalFile::Manager::attachFile(
			this->m_cFileParameter.m_TreeFileStorageStrategy,
			this->m_cFileParameter.m_BufferingStrategy,
			this->m_cFileParameter.m_IDNumber->getLockName());

	; _SYDNEY_ASSERT(this->m_pPhysicalFile != 0);

	// バリューファイルをアタッチする
	this->m_ValueFile = new ValueFile(&Transaction_,
									  &this->m_cFileParameter,
									  this->m_FixMode);

	// まだ、ファイルの中身（オブジェクト）は、
	// 更新されていないので…
	this->m_Update = false;

	// まだ、オブジェクトを検索していないので…
	this->m_Searched = false;

	// まだ、1回もオブジェクトにマークをつけていないので…
	this->m_MarkedObjectID = FileCommon::ObjectID::Undefined;

	// まだ、1回もオブジェクトを返していないので…
	this->m_ullObjectID = FileCommon::ObjectID::Undefined;

	this->m_LeafPageID = PhysicalFile::ConstValue::UndefinedPageID;

	this->m_KeyInfoIndex = ModUInt32Max;

	//
	// 検索などのヒントを設定する
	//

	if (this->m_pOpenParameter->m_iOpenMode == FileCommon::OpenMode::Search)
	{
		// Searchモード（ Search + Fetch も含む）でオープン…

		this->m_SearchHint.set(
			this->m_cFileParameter,
			this->m_pOpenParameter->m_cSearchCondition);
	}

	if (this->m_pOpenParameter->m_iReadSubMode == OpenParameter::FetchRead)
	{
		// Fetchモード（ Search + Fetch も含む）でオープン…

		this->m_FetchHint.set(
			this->m_cFileParameter,
			this->m_pOpenParameter->m_cFetchFieldIndexArray);
	}

	if (this->m_pOpenParameter->m_iOpenMode == FileCommon::OpenMode::Update)
	{
		// Updateモードでオープン…

		// v4.0からは、
		// オブジェクト値（キー＋バリュー）やキー値を指定し、
		// それを検索条件として、
		// 検索条件と一致するオブジェクトを削除／更新する仕様となった。
		// そのため、Updateモード（削除／更新）でオープンする際には
		// Fetchのためのヒントの設定が必要となった。

		this->m_FetchHint.setForUpdate(this->m_cFileParameter);
	}
}

//
//	FUNCTION private
//	Btree::File::createNodePage -- ノード／リーフページを生成する
//
//	NOTES
//	ノード／リーフページを生成する
//
//	ARGUMENTS
//	const bool	bIsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::PageID
//		生成したノード／リーフページの物理ページ識別子
//		ノード／リーフページが複数の物理ページで構成されている場合には、
//		先頭物理ページの物理ページ識別子を返す。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::createNodePage(const bool	IsLeafPage_) const
{
	; _SYDNEY_ASSERT(this->m_pPhysicalFile != 0);

	// 1ノードあたりのキーオブジェクト数を取得する
	// ＝ 1ノード／リーフあたりのキーテーブルを構成するキー情報数
	ModUInt32	keyPerNode = this->m_cFileParameter.m_KeyPerNode;

	// ノードページをキャッシュするためのベクター
	PageVector	attachNodePages;

	//
	// キーテーブルエリアを生成＋アタッチする
	//

	// ノード／リーフページヘッダサイズを取得する
	Os::Memory::Size	nodePageHeaderSize =
		NodePageHeader::getArchiveSize(IsLeafPage_);

	// キー情報のアーカイブサイズを取得する
	Os::Memory::Size	keyInfoSize =
		KeyInformation::getSize(IsLeafPage_,
								this->m_cFileParameter.m_KeyPosType,
								this->m_cFileParameter.m_KeyNum) +
		this->m_cFileParameter.m_KeySize;

	// 1つの物理ページにいくつキー情報を記録できるか
	ModUInt32	keyPerPage = 0;

	do
	{
		// ノード／リーフページを構成する物理ページを生成する

		PhysicalFile::PageID	nodePageID =
			this->m_pPhysicalFile->allocatePage(*this->m_pTransaction);

		PhysicalFile::Page*		nodePage =
			this->m_pPhysicalFile->attachPage(
				*this->m_pTransaction,
				nodePageID,
				Buffer::Page::FixMode::Write);

		// 物理ページ記述子をベクターに記録
		attachNodePages.pushBack(nodePage);

		if (keyPerPage == 0)
		{
			// 以下の2つの物理エリアを生成した後の
			// 物理ページ内で利用できる領域サイズを取得する
			//     ・ノードページヘッダ
			//     ・キーテーブル
			Os::Memory::Size	pageFreeSize =
				nodePage->getFreeAreaSize(*this->m_pTransaction,
										  2);
			//                           ~~~ 上記2つの物理エリアの分

			// 求めた領域サイズからノードページヘッダの分を引く
			pageFreeSize -= nodePageHeaderSize;

			// 1つの物理ページにいくつキー情報を記録できるかを求める
			keyPerPage = pageFreeSize / keyInfoSize;
		}

		; _SYDNEY_ASSERT(keyPerNode <= keyPerPage);

		ModUInt32	keyPerCurrentPage =
			(keyPerNode > keyPerPage) ? keyPerPage :
										keyPerNode;

		; _SYDNEY_ASSERT(keyPerCurrentPage > 0);

		// ノード／リーフページヘッダ用の物理エリアを生成する

		PhysicalFile::AreaID	nodePageHeaderAreaID =
			nodePage->allocateArea(*this->m_pTransaction,
								   nodePageHeaderSize);

		; _SYDNEY_ASSERT(nodePageHeaderAreaID == NodePageHeader::AreaID);

		//
		// 初期状態のノード／リーフページヘッダを書き込む
		//

		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   nodePage,
									   IsLeafPage_);

		nodePageHeader.write(PhysicalFile::ConstValue::UndefinedPageID,
							 PhysicalFile::ConstValue::UndefinedPageID,
							 PhysicalFile::ConstValue::UndefinedPageID,
							 keyPerCurrentPage,
							 0,
							 PhysicalFile::ConstValue::UndefinedPageID,
							 PhysicalFile::ConstValue::UndefinedPageID);

		//
		// キーテーブルエリアを生成する
		//

		ModSize		keyTableSize = keyInfoSize * keyPerCurrentPage;

		PhysicalFile::AreaID	keyTableAreaID =
			nodePage->allocateArea(*this->m_pTransaction,
								   keyTableSize);

		; _SYDNEY_ASSERT(keyTableAreaID == KeyInformation::KeyTableAreaID);

		if (keyPerNode > keyPerPage)
		{
			keyPerNode -= keyPerPage;
		}
		else
		{
			keyPerNode = 0;

			//
			// 最後の物理ページなので、そのページに
			// 必ず 1 つキーオブジェクトが書き込めるかをチェックし、
			// 書き込めないようならば、さらに1物理ページを
			// 生成＋アタッチ（ノード／リーフページヘッダも）する
			// ※ その物理ページにはキーテーブルが存在しないので
			//    キーテーブルエリアを生成する必要はない
			//

			// キーオブジェクトを記録するための物理エリアを
			// 生成した後の空き領域サイズを求める
			ModSize	currentPageFreeAreaSize =
				nodePage->getFreeAreaSize(*this->m_pTransaction,
										  3);

			Os::Memory::Size	directKeyObjectSize =
				this->m_cFileParameter.m_DirectKeyObjectSize;

			if (currentPageFreeAreaSize < directKeyObjectSize)
			{
				// もう1つ物理ページを生成する

				nodePageID =
					this->m_pPhysicalFile->allocatePage(
						*this->m_pTransaction);

				nodePage =
					this->m_pPhysicalFile->attachPage(
						*this->m_pTransaction,
						nodePageID,
						Buffer::Page::FixMode::Write);

				// 物理ページ記述子をベクターに記録
				attachNodePages.pushBack(nodePage);

				// ノード／リーフページヘッダ用の物理エリアを生成する
				nodePageHeaderAreaID =
					nodePage->allocateArea(*this->m_pTransaction,
										   nodePageHeaderSize);

				; _SYDNEY_ASSERT(
					nodePageHeaderAreaID == NodePageHeader::AreaID);

				//
				// 初期状態のノード／リーフページヘッダを書き込む
				//

				nodePageHeader.resetPhysicalPage(nodePage);

				nodePageHeader.write(
					PhysicalFile::ConstValue::UndefinedPageID,
					PhysicalFile::ConstValue::UndefinedPageID,
					PhysicalFile::ConstValue::UndefinedPageID,
					0,
					0,
					PhysicalFile::ConstValue::UndefinedPageID,
					PhysicalFile::ConstValue::UndefinedPageID);
			}

		} // end else
	}
	while (keyPerNode > 0);

	; _SYDNEY_ASSERT(attachNodePages.getSize() > 0);

	PhysicalFile::PageID	topNodePageID = (attachNodePages[0])->getID();

	//
	// 物理ページ の前後関係をノード／リーフページヘッダに記録する
	//

	int	last = attachNodePages.getSize() - 1;
	for (int i = 0; i < last; i++)
	{
		PhysicalFile::PageID	nodePageID1 =
			(attachNodePages[i])->getID();

		PhysicalFile::PageID	nodePageID2 =
			(attachNodePages[i + 1])->getID();

		//
		// 現在の物理ページのノード／リーフページヘッダに
		// 「次の物理ページの物理ページ識別子」を設定
		//

		NodePageHeader	nodePageHeader(this->m_pTransaction,
									   attachNodePages[i],
									   IsLeafPage_);

		nodePageHeader.writeNextPhysicalPageID(nodePageID2);

		//
		// 次の物理ページのリーフページヘッダに
		// 「前の物理ページの物理ページ識別子」を設定
		//

		nodePageHeader.resetPhysicalPage(attachNodePages[i + 1]);

		nodePageHeader.writePrevPhysicalPageID(nodePageID1);
	}

	// アタッチした物理ページと物理エリアをデタッチする
	File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::Dirty, this->m_SavePage);

	return topNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getBitSet -- オブジェクトをビットセットにして返す
//
//	NOTES
//	検索条件などと一致する全てのオブジェクトの
//	特定の 1 つのフィールドの値からビットセットを作成し、返す。
//	ビットセットにして返すことができるフィールドは、
//	Common::DataType::UnsignedInteger型のフィールドのみである。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Common::BitSet&
//		返り値を入れるビットセットオブジェクト
//
//	RETURN
//		true .. オブジェクトがあった
//		false.. 空だった、または2回め以降のgetである
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::File::getNextObject )
//		( Btree::File::searchObject )
//	[YET!]
//
bool
File::getBitSet(FileInformation&	FileInfo_,
				ValueFile*			ValueFile_,
				Common::BitSet&		cResult_)
{
	// まず、最初に該当するオブジェクトを検索する
	; _SYDNEY_ASSERT(this->m_pOpenParameter->m_TargetFieldNum == 1);

	Common::DataArrayData cObject;
	cObject.pushBack(new Common::UnsignedIntegerData);

	bool bResult = this->search(FileInfo_, ValueFile_, &cObject);

	//
	// 【 オプティマイザのためのGetByBitSetの仕様変更 】
	//
	// これまで、該当するオブジェクトが存在しない場合、
	// または、2回目のget時には、0（ヌルポインタ）を
	// 返していた。
	// オプティマイザからの依頼により、
	// これらのときには、「空のCommon::BitSet」を
	// 返すように変更となった。
	//
	// ->
	//    LogicalFile::File::get の仕様変更により false を返せばよい

	if (!bResult) return false;

	//
	// 該当するオブジェクトを全て取得し、ビットセットを設定する
	//

	//
	// ビットセット対象のフィールドのみを選択して
	// オブジェクト(pObject)を読み込んでいるので、
	// すでにプロジェクションされるようになった。
	// （これまでは、プロジェクションされていなかった。）
	// したがって、ビットセット対象のフィールドインデックスを
	// 参照していたが、必ずフィールドインデックスは"0"となった。
	//

	//int	iBitSetFieldIndex =
	//	*this->m_pOpenParameter->m_TargetFieldIndexArray;

	const Common::Data*	field = (cObject.getElement(0)).get();
		//(pObject->getElement(iBitSetFieldIndex)).get();

	Common::DataType::Type	fieldType = field->getType();

	const Common::UnsignedIntegerData*	bitSetField = 0;

	if (!field->isNull() && fieldType == Common::DataType::UnsignedInteger)
	{
		bitSetField =
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*,
								 field);

		; _SYDNEY_ASSERT(bitSetField != 0);

		cResult_.set(bitSetField->getValue());
	}
	else
	{
		; _SYDNEY_ASSERT(field->isNull());
	}

	while (getNextObject(ValueFile_, &cObject))
	{
		field = (cObject.getElement(0)).get();
		//field = (pObject->getElement(iBitSetFieldIndex)).get();

		fieldType = field->getType();

		if (!field->isNull() && fieldType == Common::DataType::UnsignedInteger)
		{
			const Common::UnsignedIntegerData*	bitSetField =
				_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*,
									 field);

			; _SYDNEY_ASSERT(bitSetField != 0);

			cResult_.set(bitSetField->getValue());
		}
		else
		{
			; _SYDNEY_ASSERT(field->isNull());
		}
	}

	return true;
}

#ifdef OBSOLETE // 将来に対する予約
//
//	FUNCTION private
//	Btree::File::isDirectObject --
//		代表バリューオブジェクトかどうかを知らせる
//
//	NOTES
//	代表バリューオブジェクトかどうかを知らせる
//
//	ARGUMENTS
//	PhysicalFile::Page*			ValuePage_
//		バリューページの物理ページ記述子
//	const PhysicalFile::AreaID	ValueObjectAreaID_
//		代表オブジェクトかどうかをチェックするバリューオブジェクトが
//		記録されている物理エリアの識別子
//
//	RETURN
//	bool
//		指定されたバリューオブジェクトが代表バリューオブジェクトかどうか
//			true  : 代表オブジェクト
//			false : 非代表オブジェクト
//
//	EXCEPTIONS
//	なし
//
bool
File::isDirectObject(PhysicalFile::Page*		ValuePage_,
					 const PhysicalFile::AreaID	ValueObjectAreaID_) const
{
	try
	{
		if (ValueObjectAreaID_ <
			ValuePage_->getTopAreaID(*this->m_pTransaction) ||
			ValueObjectAreaID_ >
			ValuePage_->getLastAreaID(*this->m_pTransaction))
		{
			// 存在しないエリアの物理エリア識別子が指定された。
			// ※ isDirectObject は、例外を投げない。

			return false;
		}

		const void*	valueObjectAreaTop =
			File::getConstAreaTop(ValuePage_,
								  ValueObjectAreaID_);

		const File::ObjectType*	objectType =
			static_cast<const File::ObjectType*>(valueObjectAreaTop);

		return this->isDirectObject(*objectType);
	}
	catch (...)
	{
		// 何もしないで false を返す
	}

	return false;
}

//
//	FUNCTION private
//	Btree::File::isDirectObject --
//		代表バリューオブジェクトかどうかを知らせる
//
//	NOTES
//	代表バリューオブジェクトかどうかを知らせる。
//
//	ARGUMENTS
//	const Btree::File::ObjectType	ObjectType_
//		オブジェクトタイプ
//
//	RETURN
//	bool
//		オブジェクトタイプが代表オブジェクトタイプかどうか
//			true  : 代表オブジェクト
//			false : 非代表オブジェクト
//
//	EXCEPTIONS
//	なし
//
bool
File::isDirectObject(const File::ObjectType	ObjectType_) const
{
	return (ObjectType_ & File::DirectObjectType) != 0;
}
#endif

//	FUNCTION private
//	Btree::File::makeObjectID -- オブジェクトIDを生成して返す
//
//	NOTES
//	オブジェクトIDを生成して返す。
//
//	ARGUMENTS
//	PhysicalFile::PageID	PageID_
//		オブジェクトが書き込まれている物理ページの物理ページ識別子
//	PhysicalFile::AreaID	AreaID_
//		オブジェクトが書き込まれている物理エリアの物理エリア識別子
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//
//	EXCEPTIONS
//	なし

// static
ModUInt64
File::makeObjectID(PhysicalFile::PageID	PageID_, PhysicalFile::AreaID AreaID_)
{
 	return Common::ObjectIDData(PageID_, AreaID_).getValue();
}

//
//	FUNCTION private
//	Btree::File::getNextObject -- 次のオブジェクトを返す
//
//	NOTES
//	次のオブジェクトを返す。
//	該当するオブジェクトが存在しない場合には、false を返す。
//
//	ARGUMENTS
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData* pTuple_
//		次のオブジェクトへのポインタ
//
//	RETURN
//		該当するオブジェクトが存在する場合には、true
//		該当するオブジェクトが存在しない場合には、false
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::getNextObject(ValueFile*	ValueFile_, Common::DataArrayData* pTuple_)
{
	; _SYDNEY_ASSERT(this->m_Searched);

	this->m_CatchMemoryExhaust = false;

	bool bResult = false;

	//
	// 物理ページをキャッシュし過ぎて、
	// 下位モジュールからMemoryExhaustが送出された場合に
	// 物理ページをキャッシュしないようにして、
	// オブジェクト検索をリトライするために、
	// ループとしてある。
	// リトライは1回だけ行い、
	// それでもMemoryExhaustが送出された場合には、
	// オブジェクト検索を諦める。
	//

	while (true)
	{
		// アタッチしたノードページ／バリューページの
		// 記述子をキャッシュするためのベクター
		PageVector	attachNodePages;
		PageVector	attachValuePages;

		attachNodePages.reserve(4);
		attachValuePages.reserve(4);

		try
		{
			if (this->m_ullObjectID != FileCommon::ObjectID::Undefined)
			{
				ValueFile_->readLeafInfo(this->m_ullObjectID,
										 attachValuePages,
										 this->m_CatchMemoryExhaust,
										 this->m_LeafPageID,
										 this->m_KeyInfoIndex);
			}

			// 次のオブジェクトのIDを得る
			// ※ Search + Fetch の場合には、
			// 　 検索条件と一致するオブジェクトの
			// 　 オブジェクトIDが得られる。
			// 　 そのオブジェクトIDが示すオブジェクトが
			// 　 Fetch検索条件とも一致するかどうかの
			// 　 判断は、後の処理で行う。
			this->m_ullObjectID = this->getNextObjectID(attachNodePages,
														ValueFile_,
														attachValuePages);

			if (this->m_ullObjectID != FileCommon::ObjectID::Undefined)
			{
				// 利用者に返すべきオブジェクトが
				// ファイル内に存在した…

				if (this->m_pOpenParameter->m_iOpenMode ==
					FileCommon::OpenMode::Search &&
					this->m_pOpenParameter->m_iReadSubMode ==
					OpenParameter::FetchRead)
				{
					// Search + Fetchモード…

					// もう既に検索条件と一致するオブジェクトは
					// 見つかっている。
					// ここで、Fetch検索条件とも一致するオブジェクトを
					// 検索する。
					bResult = this->searchAndFetch(attachNodePages,
												   ValueFile_,
												   attachValuePages,
												   pTuple_,
												   false); // 初回のgetではない
				}
				else
				{
					// Search + Fetchモード以外…

					// では、オブジェクトを
					// ファイルから読み込みましょう。
					bResult = this->getObject(
								  this->m_ullObjectID,
								  ValueFile_,
								  pTuple_,
								  true, // プロジェクションフィールドのみ読み込む
								  attachNodePages,
								  attachValuePages);
				}
			}

			// 下位の関数がキャッシュした
			// ノードページ／バリューページをデタッチする

			File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
			File::detachPageAll(ValueFile_->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

			// MemoryExhaustが送出されることなく、
			// 目的のオブジェクトをファイルから読み込むことができたので
			// ループから抜ける。
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// MemoryExhaustをキャッチ…

			if (this->m_CatchMemoryExhaust)
			{
				// 各物理ページをアタッチ後に不要になったら
				// 即デタッチしていたにもかかわらず
				// またまたMemoryExhaustが起こったのであれば
				// どうしようもない…

				_SYDNEY_RETHROW;
			}
			else
			{
				// MemoryExhaustをキャッチしたのは、
				// 今回が最初…

				// リトライの準備をする。

				this->m_CatchMemoryExhaust = true;

				File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
				File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
			}
		}

	} // end while (true)

	return bResult;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectID -- 次のオブジェクトのIDを返す
//
//	NOTES
//	次のオブジェクトのIDを返す。
//	該当するオブジェクトが存在しない場合には、
//	FileCommon::ObjectID::Undefinedを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectID(PageVector&	AttachNodePages_,
					  ValueFile*	ValueFile_,
					  PageVector&	AttachValuePages_) const
{
	; _SYDNEY_ASSERT(this->m_Searched);

	if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
	{
		// 既に該当するオブジェクトは存在していない…

		return this->m_ullObjectID;
	}

	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (this->m_pOpenParameter->m_iOpenMode ==
		FileCommon::OpenMode::Read)
	{
		// Scanモードまたは、Fetch（単独）モード…

		if (this->m_pOpenParameter->m_iReadSubMode ==
			OpenParameter::ScanRead)
		{
			// Scanモード…

			resultObjectID = this->getNextObjectIDScan(AttachNodePages_);
		}
		else
		{
			// Fetchモード…
			// （のはず。）

			; _SYDNEY_ASSERT(
				this->m_pOpenParameter->m_iReadSubMode ==
				OpenParameter::FetchRead);

			if (this->m_FetchHint.m_ByKey)
			{
				// キー値でFetch…

				resultObjectID =
					this->getNextObjectIDFetchByKey(AttachNodePages_,
													ValueFile_,
													AttachValuePages_);
			}
			else
			{
				// オブジェクトIDでFetch…

				//
				// オブジェクトIDには重複がないので
				// 初回だけオブジェクトを返せばよい。
				// 従って、nextと言われてもUndefinedを返せばよい。
				//

				resultObjectID = FileCommon::ObjectID::Undefined;
			}
		}
	}
	else
	{
		// Searchモード…
		// （のはず。）

		; _SYDNEY_ASSERT(
			this->m_pOpenParameter->m_iOpenMode ==
			FileCommon::OpenMode::Search);

		resultObjectID = this->getNextObjectIDSearch(AttachNodePages_);
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDScan -- 次のオブジェクトのIDを返す(Scan)
//
//	NOTES
//	キー値でScanする際の次のオブジェクトのIDを返す。
//	既にファイル終端までオブジェクトを返し終わっている場合には、
//	FileCommon::ObjectID::Undefinedを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		既にファイル終端までオブジェクトを返し終わっている場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDScan(PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true,  // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDSearch -- 次のオブジェクトのIDを返す(Search)
//
//	NOTES
//	キー値でSearchする際の次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDSearch(PageVector&	AttachNodePages_) const
{
	if (this->m_SearchHint.m_ConditionType ==
		SearchHint::ConditionType::Single)
	{
		// 検索対象は先頭キーフィールドのみ…

		if (this->m_SearchHint.m_LastIsSpan)
		{
			// 先頭キーフィールドに対する範囲指定…

			return this->getNextObjectIDBySpanCondition(AttachNodePages_);
		}
		else
		{
			// 先頭キーフィールドに対する単一条件…

			return this->getNextObjectIDBySingleCondition(AttachNodePages_);
		}
	}
	else
	{
		// 検索対象は複数キーフィールド…

		; _SYDNEY_ASSERT(
			this->m_SearchHint.m_ConditionType ==
			SearchHint::ConditionType::Multi);

		return this->getNextObjectIDByMultiCondition(AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleCondition --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対する単一条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する単一条件でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleCondition(PageVector&	AttachNodePages_) const
{
	// m_StartOperators[0]を見る。

	if (*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::Equals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がEquals…

		return this->getNextObjectIDBySingleEquals(AttachNodePages_);
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThan)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLessThan…

		return
			this->getNextObjectIDBySingleLessThan(AttachNodePages_,
												  false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLessThanEquals…

		return
			this->getNextObjectIDBySingleLessThan(AttachNodePages_,
												  true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThan)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がGreaterThan…

		return
			this->getNextObjectIDBySingleGreaterThan(AttachNodePages_,
													 false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がGreaterThanEquals…

		return
			this->getNextObjectIDBySingleGreaterThan(AttachNodePages_,
													 true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::Like)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLike…

		return this->getNextObjectIDByLike(AttachNodePages_);
	}
	else
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がEqualsToNull…
		// （のはず。）

		; _SYDNEY_ASSERT(
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::EqualsToNull);

		return this->getNextObjectIDBySingleEqualsToNull(AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleEquals --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対するEquals条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がEqualsの
//	単一条件でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleEquals(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDBySingleEqualsSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	if (this->compareToTopSearchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID)
		!= 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleLessThan --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対するLessThan(Equals)条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がLessThanまたは
//	LessThanEqualsの単一条件でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子にEqualsを含むかどうか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleLessThan(PageVector&	AttachNodePages_,
									  const bool	ContainEquals_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDBySingleLessThanSimpleKey(AttachNodePages_,
														   ContainEquals_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

	// 先頭キーフィールドのソート順が降順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 検索条件を満たしているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleGreaterThan --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対するGreaterThan(Equals)条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がGreaterThanまたは
//	GreaterThanEqualsの単一条件でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子にEqualsを含むかどうか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleGreaterThan(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDBySingleGreaterThanSimpleKey(
				AttachNodePages_,
				ContainEquals_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		this->m_pPhysicalFile->attachPage(*this->m_pTransaction,
										  this->m_LeafPageID,
										  this->m_FixMode);

	AttachNodePages_.pushBack(leafPage);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	// 先頭キーフィールドのソート順が昇順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 検索条件を満たしているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID);

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySingleEqualsToNull --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対するEqualsToNull条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がEqualsToNullの
//	単一条件でSearchする際の次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySingleEqualsToNull(
	PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDBySingleEqualsToNullSimpleKey(
				AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	// 先頭キーフィールドのソート順が降順で、
	// そのソート順にオブジェクトを返していくのであれば、
	// 検索条件との比較は不要である。
	// ずーっと、キー値順での最終オブジェクトまで、
	// 先頭キーフィールドにヌル値が記録されているはずなので。

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		const ModUInt64 keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			keyObjectID && keyObjectID != FileCommon::ObjectID::Undefined);

		if (leafPage->getID() !=
			Common::ObjectIDData::getFormerValue(keyObjectID)) {

			checkMemoryExhaust(leafPage);

			leafPage = File::attachPage(
				m_pTransaction, m_pPhysicalFile,
				Common::ObjectIDData::getFormerValue(keyObjectID),
				m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);
			; _SYDNEY_ASSERT(leafPage != 0);
		}

		// ※ File::getTopFieldPointerは、
		// 　 先頭キーフィールドの値として
		// 　 ヌル値が設定されている場合には、
		// 　 ヌルポインタ(0)を返す。
		// 　 ちなみに、ヌル値が設定されているかどうかは
		// 　 記録されているフィールドタイプの違いにより
		// 　 判断可能である。

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドがヌル値ではなかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDBySpanCondition --
//		次のオブジェクトのIDを返す
//		（先頭キーフィールドに対する範囲指定でSearch）
//
//	NOTES
//	先頭キーフィールドに対する範囲指定でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDBySpanCondition(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDBySpanConditionSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	SearchHint::CompareTarget::Value	compareTarget =
		SearchHint::CompareTarget::Undefined;

	bool	containEquals = false;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		compareTarget = SearchHint::CompareTarget::Stop;

		containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);
	}
	else
	{
		compareTarget = SearchHint::CompareTarget::Start;

		containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);
	}

	int	compareResult =
		this->compareToTopSearchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID,
										  compareTarget);

	checkMemoryExhaust(leafPage);

	if (compareResult < 0)
	{
		// 検索条件と一致しなかった…

		// ということは、ファイル内に
		// 指定範囲内のオブジェクトはもう存在しない。

		return FileCommon::ObjectID::Undefined;
	}

	if (compareResult == 0 && containEquals == false)
	{
		// 同上

		return FileCommon::ObjectID::Undefined;
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getNextObjectIDByMultiCondition --
//		次のオブジェクトのIDを返す
//		（複数キーフィールドに対する複合条件でSearch）
//
//	NOTES
//	複数キーフィールドに対する複合条件でSearchする際の
//	次のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getNextObjectIDByMultiCondition(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getNextObjectIDByMultiConditionSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignNextKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// キー値順での次のオブジェクトが
		// ファイル内に存在しない…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	if (leafPage->getID() != leafPageID)
	{
		leafPageID = leafPage->getID();

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	if (this->compareToMultiSearchCondition(leafPage,
											AttachNodePages_,
											keyObjectID)
		!= 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		if (this->compareToLastCondition(keyObjectID,
										 leafPage,
										 AttachNodePages_)
			== false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			while (this->assignNextKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				keyObjectID = keyInfo.readKeyObjectID();

				if (this->compareToMultiSearchCondition(leafPage,
														AttachNodePages_,
														keyObjectID)
					!= 0)
				{
					break;
				}

				if (this->compareToLastCondition(keyObjectID,
												 leafPage,
												 AttachNodePages_))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObject -- 前のオブジェクトを返す
//
//	NOTES
//	前のオブジェクトを返す。
//	該当するオブジェクトが存在しない場合には、false を返す。
//
//	ARGUMENTS
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData* pTuple_
//		前のオブジェクトへのポインタ
//
//	RETURN
//		該当するオブジェクトが存在した場合には、true
//		該当するオブジェクトが存在しない場合には、false
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::getPrevObject(ValueFile*	ValueFile_, Common::DataArrayData* pTuple_)
{
	; _SYDNEY_ASSERT(this->m_Searched);

	this->m_CatchMemoryExhaust = false;

	bool bResult = false;

	//
	// 物理ページをキャッシュし過ぎて、
	// 下位モジュールからMemoryExhaustが送出された場合に
	// 物理ページをキャッシュしないようにして、
	// オブジェクト検索をリトライするために、
	// ループとしてある。
	// リトライは1回だけ行い、
	// それでもMemoryExhaustが送出された場合には、
	// オブジェクト検索を諦める。
	//

	while (true)
	{
		// アタッチしたノードページ／バリューページの
		// 記述子をキャッシュするためのベクター
		PageVector	attachNodePages;
		PageVector	attachValuePages;

		attachNodePages.reserve(4);
		attachValuePages.reserve(4);

		try
		{
			if (this->m_ullObjectID != FileCommon::ObjectID::Undefined)
			{
				ValueFile_->readLeafInfo(this->m_ullObjectID,
										 attachValuePages,
										 this->m_CatchMemoryExhaust,
										 this->m_LeafPageID,
										 this->m_KeyInfoIndex);
			}

			// 前のオブジェクトのIDを得る
			// ※ Search + Fetch の場合には、
			// 　 検索条件と一致するオブジェクトの
			// 　 オブジェクトIDが得られる。
			// 　 そのオブジェクトIDが示すオブジェクトが
			// 　 Fetch検索条件とも一致するかどうかの
			// 　 判断は、後の処理で行う。
			this->m_ullObjectID = this->getPrevObjectID(attachNodePages,
														ValueFile_,
														attachValuePages);

			if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
			{
				// 利用者に返すべきオブジェクトが
				// ファイル内に存在しなかった…

				this->m_LeafPageID =
					PhysicalFile::ConstValue::UndefinedPageID;
				this->m_KeyInfoIndex = ModUInt32Max;
			}
			else
			{
				if (this->m_pOpenParameter->m_iOpenMode ==
					FileCommon::OpenMode::Search &&
					this->m_pOpenParameter->m_iReadSubMode ==
					OpenParameter::FetchRead)
				{
					// Search + Fetchモード…

					// もう既に検索条件と一致するオブジェクトは
					// 見つかっている。
					// ここで、Fetch検索条件とも一致するオブジェクトを
					// 検索する。
					bResult = this->searchAndFetch(attachNodePages,
												   ValueFile_,
												   attachValuePages,
												   pTuple_,
												   false); // 初回のgetではない
				}
				else
				{
					// Search + Fetchモード以外…

					// では、オブジェクトを
					// ファイルから読み込みましょう。
					bResult = this->getObject(
								  this->m_ullObjectID,
								  ValueFile_,
								  pTuple_,
								  true, // プロジェクションフィールドのみ読み込む
								  attachNodePages,
								  attachValuePages);
				}
			}

			// 下位の関数がキャッシュした
			// ノードページ／バリューページをデタッチする

			File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);
			File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, this->m_SavePage);

			// MemoryExhaustが送出されることなく、
			// 目的のオブジェクトをファイルから読み込むことができたので
			// ループから抜ける。
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			// MemoryExhaustをキャッチ…

			if (this->m_CatchMemoryExhaust)
			{
				// 各物理ページをアタッチ後に不要になったら
				// 即デタッチしていたにもかかわらず
				// またまたMemoryExhaustが起こったのであれば
				// どうしようもない…

				_SYDNEY_RETHROW;
			}
			else
			{
				// MemoryExhaustをキャッチしたのは、
				// 今回が最初…

				// リトライの準備をする。

				this->m_CatchMemoryExhaust = true;

				File::detachPageAll(this->m_pPhysicalFile, attachNodePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
				File::detachPageAll(this->m_ValueFile->m_PhysicalFile, attachValuePages, PhysicalFile::Page::UnfixMode::NotDirty, false);
			}
		}

	} // end while (true)

	return bResult;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectID --
//		前のオブジェクトのIDを返す
//
//	NOTES
//	前のオブジェクトのIDを返す。
//	該当するオブジェクトが存在しない場合には、
//	FileCommon::ObjectID::Undefinedを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*	ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectID(PageVector&	AttachNodePages_,
					  ValueFile*	ValueFile_,
					  PageVector&	AttachValuePages_) const
{
	; _SYDNEY_ASSERT(this->m_Searched);

	if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
	{
		// 既に該当するオブジェクトは存在していない…

		return this->m_ullObjectID;
	}

	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (this->m_pOpenParameter->m_iOpenMode ==
		FileCommon::OpenMode::Read)
	{
		// Scanモードまたは、Fetch（単独）モード…

		if (this->m_pOpenParameter->m_iReadSubMode ==
			OpenParameter::ScanRead)
		{
			// Scanモード…

			resultObjectID = this->getPrevObjectIDScan(AttachNodePages_);
		}
		else
		{
			// Fetchモード…
			// （のはず。）

			; _SYDNEY_ASSERT(
				this->m_pOpenParameter->m_iReadSubMode ==
				OpenParameter::FetchRead);

			if (this->m_FetchHint.m_ByKey)
			{
				// キー値でFetch…

				resultObjectID =
					this->getPrevObjectIDFetchByKey(AttachNodePages_,
													ValueFile_,
													AttachValuePages_);
			}
			else
			{
				// オブジェクトIDでFetch…

				//
				// オブジェクトIDには重複がないので
				// 初回だけオブジェクトを返せばよい。
				// 従って、prevと言われてもUndefinedを返せばよい。
				//

				resultObjectID = FileCommon::ObjectID::Undefined;
			}
		}
	}
	else
	{
		// Searchモード…
		// （のはず。）

		; _SYDNEY_ASSERT(
			this->m_pOpenParameter->m_iOpenMode ==
			FileCommon::OpenMode::Search);

		resultObjectID = this->getPrevObjectIDSearch(AttachNodePages_);
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDScan -- 前のオブジェクトのIDを返す(Scan)
//
//	NOTES
//	キー値でScanする際の前のオブジェクトのIDを返す。
//	既にファイル始端までオブジェクトを返し終わっている場合には、
//	FileCommon::ObjectID::Undefinedを返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		既にファイル始端までオブジェクトを返し終わっている場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDScan(PageVector&	AttachNodePages_) const
{
	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDSearch -- 前のオブジェクトのIDを返す(Search)
//
//	NOTES
//	キー値でSearchする際の前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（岩頭するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDSearch(PageVector&	AttachNodePages_) const
{
	if (this->m_SearchHint.m_ConditionType ==
		SearchHint::ConditionType::Single)
	{
		if (this->m_SearchHint.m_LastIsSpan)
		{
			return this->getPrevObjectIDBySpanCondition(AttachNodePages_);
		}
		else
		{
			return this->getPrevObjectIDBySingleCondition(AttachNodePages_);
		}
	}
	else
	{
		; _SYDNEY_ASSERT(
			this->m_SearchHint.m_ConditionType ==
			SearchHint::ConditionType::Multi);

		return this->getPrevObjectIDByMultiCondition(AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleCondition --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対する単一条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する単一条件でSearchする際の
//	前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleCondition(PageVector&	AttachNodePages_) const
{
	//
	// 以下の if ... else if... では、
	// this->m_SearchHint.m_StartOperatorArray[0]を
	// 参照している。
	//

	if (*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::Equals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がEquals…

		return this->getPrevObjectIDBySingleEquals(AttachNodePages_);
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThan)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLessThan…

		return
			this->getPrevObjectIDBySingleLessThan(AttachNodePages_,
												  false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLessThanEquals…

		return
			this->getPrevObjectIDBySingleLessThan(AttachNodePages_,
												  true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThan)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がGreaterThan…

		return
			this->getPrevObjectIDBySingleGreaterThan(AttachNodePages_,
													 false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がGreaterThanEquals…

		return
			this->getPrevObjectIDBySingleGreaterThan(AttachNodePages_,
													 true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::Like)
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がLike…

		return this->getPrevObjectIDByLike(AttachNodePages_);
	}
	else
	{
		// 先頭キーフィールドに対する検索条件の
		// 比較演算子がEqualsToNull…
		// （のはず。）

		; _SYDNEY_ASSERT(
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::EqualsToNull);

		return this->getPrevObjectIDBySingleEqualsToNull(AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleEquals --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対するEquals条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がEqualsの
//	単一条件でSearchする際の前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleEquals(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDBySingleEqualsSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	if (this->compareToTopSearchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID)
		!= 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleLessThan --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対するLessThan(Equals)条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がLessThanまたは
//	LessThanEqualsの単一条件でSearchする際の
//	前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子にEqualsを含むかどうか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleLessThan(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDBySingleLessThanSimpleKey(AttachNodePages_,
														   ContainEquals_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		this->m_pPhysicalFile->attachPage(*this->m_pTransaction,
										  this->m_LeafPageID,
										  this->m_FixMode);

	AttachNodePages_.pushBack(leafPage);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID);

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleGreaterThan --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対するGreaterThan(Equals)条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がGreaterThanまたは
//	GreaterThanEqualsの単一条件でSearchする際の
//	前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子にEqualsを含むかどうか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleGreaterThan(
	PageVector&	AttachNodePages_,
	const bool	ContainEquals_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDBySingleGreaterThanSimpleKey(
				AttachNodePages_,
				ContainEquals_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		// 前回返したオブジェクトが
		// “キー値順での先頭オブジェクト”であった…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID);

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySingleEqualsToNull --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対するEqualsToNull条件でSearch）
//
//	NOTES
//	先頭キーフィールドに対する検索条件の比較演算子がEqualsToNullの
//	単一条件でSearchする際の前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySingleEqualsToNull(
	PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDBySingleEqualsToNullSimpleKey(
				AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Descending)
	{
		// 先頭キーフィールドのソート順は降順…

		const ModUInt64 keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID &&
						 keyObjectID !=	FileCommon::ObjectID::Undefined);

		if (leafPage->getID() !=
			Common::ObjectIDData::getFormerValue(keyObjectID)) {

			checkMemoryExhaust(leafPage);

			leafPage = File::attachPage(
				m_pTransaction, m_pPhysicalFile,
				Common::ObjectIDData::getFormerValue(keyObjectID),
				m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);
			; _SYDNEY_ASSERT(leafPage != 0);
		}

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドがヌル値ではなかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDBySpanCondition --
//		前のオブジェクトのIDを返す
//		（先頭キーフィールドに対する範囲指定でSearch）
//
//	NOTES
//	先頭キーフィールドに対する範囲指定でSearchする際の
//	前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDBySpanCondition(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDBySpanConditionSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	; _SYDNEY_ASSERT(this->m_KeyInfoIndex != ModUInt32Max);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	SearchHint::CompareTarget::Value	compareTarget =
		SearchHint::CompareTarget::Undefined;

	bool	containEquals = false;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		compareTarget = SearchHint::CompareTarget::Start;

		containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);
	}
	else
	{
		compareTarget = SearchHint::CompareTarget::Stop;

		containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);
	}

	int	compareResult =
		this->compareToTopSearchCondition(leafPage,
										  AttachNodePages_,
										  keyObjectID,
										  compareTarget);

	checkMemoryExhaust(leafPage);

	if (compareResult > 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	if (compareResult == 0 && containEquals == false)
	{
		return FileCommon::ObjectID::Undefined;
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getPrevObjectIDByMultiCondition --
//		前のオブジェクトのIDを返す
//		（複数キーフィールドに対する複合条件でSearch）
//
//	NOTES
//	複数キーフィールドに対する複合条件でSearchする際の
//	前のオブジェクトのIDを返す。
//	もう該当するオブジェクトが存在しない（該当するオブジェクトをすべて
//	利用者に返し終わった）場合には、FileCommon::ObjectID::Undefinedを
//	返す。
//
//	ARGUMENTS
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//		該当するオブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefined。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getPrevObjectIDByMultiCondition(PageVector&	AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->getPrevObjectIDByMultiConditionSimpleKey(AttachNodePages_);
	}

	; _SYDNEY_ASSERT(
		this->m_LeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	if (this->assignPrevKeyInformation(leafPage,
									   AttachNodePages_,
									   leafPageHeader,
									   keyInfo)
		== false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
					 keyObjectID != 0);

	if (leafPage->getID() != leafPageID)
	{
		leafPageID = leafPage->getID();

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	if (this->compareToMultiSearchCondition(leafPage,
											AttachNodePages_,
											keyObjectID)
		!= 0)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		if (this->compareToLastCondition(keyObjectID,
										 leafPage,
										 AttachNodePages_)
			== false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			while (this->assignPrevKeyInformation(leafPage,
												  AttachNodePages_,
												  leafPageHeader,
												  keyInfo))
			{
				if (leafPage->getID() != leafPageID)
				{
					leafPageID = leafPage->getID();

					leafPageHeader.resetPhysicalPage(leafPage);
				}

				keyObjectID = keyInfo.readKeyObjectID();

				if (this->compareToMultiSearchCondition(leafPage,
														AttachNodePages_,
														keyObjectID)
					!= 0)
				{
					break;
				}

				if (this->compareToLastCondition(keyObjectID,
												 leafPage,
												 AttachNodePages_))
				{
					resultObjectID = keyInfo.readValueObjectID();

					break;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getFieldPointer --
//		フィールドの値が記録されている位置へのポインタを返す
//
//	NOTES
//	フィールドの値が記録されている位置へのポインタを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Page*	ObjectPage_
//		オブジェクトが記録されている物理ページの記述子
//	const PhysicalFile::AreaID	ObjectAreaID_
//		オブジェクトが記録されている物理エリアの識別子
//	int							FieldIndex_
//		フィールドインデックス
//	const bool					IsKeyObject_
//		キーオブジェクトかバリューオブジェクトか
//			true  : キーオブジェクト
//			false : バリューオブジェクト
//
//	RETURN
//	void*
//		フィールドの値が記録されている位置へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
void*
File::getFieldPointer(const PhysicalFile::Page*		ObjectPage_,
					  const PhysicalFile::AreaID	ObjectAreaID_,
					  int							FieldIndex_,
					  const bool					IsKeyObject_) const
{
	; _SYDNEY_ASSERT(ObjectPage_ != 0);

	return getFieldPointer(*ObjectPage_, ObjectAreaID_, FieldIndex_, IsKeyObject_);
}

void*
File::getFieldPointer(const PhysicalFile::Page&		ObjectPage_,
					  const PhysicalFile::AreaID	ObjectAreaID_,
					  int							FieldIndex_,
					  const bool					IsKeyObject_) const
{
	const char*	objectAreaTop =
		static_cast<const char*>(File::getConstAreaTop(ObjectPage_,
													   ObjectAreaID_));

	const File::ObjectType*	objectType =
		syd_reinterpret_cast<const File::ObjectType*>(objectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	const NullBitmap::Value*	nullBitmapTop = 0;

	if (IsKeyObject_)
	{
		nullBitmapTop =
			syd_reinterpret_cast<const NullBitmap::Value*>(objectType + 1);
	}
	else
	{
		nullBitmapTop =
			syd_reinterpret_cast<const NullBitmap::Value*>(
				objectAreaTop +
				File::ObjectTypeArchiveSize +
				File::PageIDArchiveSize +
				File::ModUInt32ArchiveSize);
	}

	//
	// キーオブジェクト／バリューオブジェクト内での
	// フィールド数やフィールドインデックスを設定する。
	//

	int	fieldNum = 0;
	int	bitIndex = 0;
	if (IsKeyObject_)
	{
		bitIndex = FieldIndex_ - 1;

		fieldNum = this->m_cFileParameter.m_KeyNum;
	}
	else
	{
		bitIndex =
			FieldIndex_ - this->m_cFileParameter.m_TopValueFieldIndex;

		fieldNum = this->m_cFileParameter.m_ValueNum;
	}

	if (NullBitmap::isNull(nullBitmapTop, fieldNum, bitIndex))
	{
		return 0;
	}

	const char*	field =
		static_cast<const char*>(NullBitmap::getConstTail(nullBitmapTop,
														  fieldNum));

	int	i = 0;

	if (IsKeyObject_)
	{
		i = 1;
	}
	else
	{
		i = this->m_cFileParameter.m_TopValueFieldIndex;
	}

	for (; i < FieldIndex_; i++)
	{
		field += this->m_cFileParameter.getFieldArchiveSize(i);
	}

	return const_cast<char*>(field);
}

//
//	FUNCTION private
//	Btree::File::getTopKeyPointer --
//		先頭キーフィールドの値が記録されている位置へのポインタを返す
//
//	NOTES
//	先頭キーフィールドの値が記録されている位置へのポインタを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			KeyObjectPage_
//		キーオブジェクトが記録されているノードページの物理ページ記述子
//	const PhysicalFile::AreaID	KeyObjectAreaID_
//		キーオブジェクトが記録されている物理エリアの識別子
//
//	RETURN
//	void*
//		先頭キーフィールドにヌル値が記録されている場合には、
//		ヌルポインタを返す。
//
//	EXCEPTIONS
//	[YET!]
//
void*
File::getTopKeyPointer(PhysicalFile::Page*			KeyObjectPage_,
					   const PhysicalFile::AreaID	KeyObjectAreaID_) const
{
	const void*	keyObjectAreaTop = File::getConstAreaTop(KeyObjectPage_,
														 KeyObjectAreaID_);

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(keyObjectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	const NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(objectType + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	return
		nullBitmap.isNull(0) ?
			0 : const_cast<void*>(nullBitmap.getConstTail());
}

//
//	FUNCTION private
//	Btree::File::getObject --
//		ファイルに記録されている各フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている各フィールドの値を読み込む。
//
//	ARGUMENTS
//	const ModUInt64				ValueObjectID_
//		バリューオブジェクトのオブジェクトID
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData*&		ResultObject_
//		オブジェクトへの参照
//		この引数が示すオブジェクトへ
//		ファイルから各フィールドの値を
//		読み込む。
//	const bool					DoProjection_
//		プロジェクション指定がされていれば、指定されたフィールドの値だけを
//		読み込むかどうか
//			true  : プロジェクション指定がされていれば、
//			        指定されたフィールドの値だけを読み込む。
//			false : プロジェクション指定がされていても、
//			        オブジェクト全体を読み込む。
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::getObject(const ModUInt64			ValueObjectID_,
				ValueFile*				ValueFile_,
				Common::DataArrayData*	ResultObject_,
				const bool				DoProjection_,
				PageVector&				AttachNodePages_,
				PageVector&				AttachValuePages_) const
{
	; _SYDNEY_ASSERT(ResultObject_ != 0);

	int	reserveNum = 0;
	if (DoProjection_ && this->m_pOpenParameter->m_bFieldSelect)
	{
		reserveNum = this->m_pOpenParameter->m_TargetFieldNum;
	}
	else
	{
		reserveNum = this->m_cFileParameter.m_FieldNum;
	}

	; _SYDNEY_ASSERT(ResultObject_->getCount() == reserveNum);
	int iElement = 0;

	// オブジェクトにオブジェクトIDを設定する
	if (DoProjection_ == false ||
		this->m_pOpenParameter->m_SelectObjectID)
	{
		File::setObjectID(ValueObjectID_, ResultObject_);
		++iElement;
	}

	// リーフページに記録されているキーオブジェクトから
	// 各キーフィールドの値を読み込む
	if (DoProjection_ == false ||
		this->m_pOpenParameter->m_ExistTargetFieldInKey)
	{
		this->readKey(ValueObjectID_,
					  ValueFile_,
					  ResultObject_,
					  iElement,
					  DoProjection_,
					  AttachNodePages_,
					  AttachValuePages_);
	}

	// バリューページに記録されているバリューオブジェクトから
	// 各バリューフィールドの値を読み込む
	if (DoProjection_ == false ||
		this->m_pOpenParameter->m_ExistTargetFieldInValue)
	{
		ValueFile_->read(ValueObjectID_,
						 ResultObject_,
						 iElement,
						 this->m_pOpenParameter,
						 DoProjection_,
						 this->m_CatchMemoryExhaust,
						 AttachValuePages_);
	}

	// 成功したのでtrueを返す
	return true;
}

//	FUNCTION private
//	Btree::File::setObjectID --
//		オブジェクトの先頭フィールドにオブジェクトIDを設定する
//
//	NOTES
//	オブジェクトの先頭フィールドに
//	バリューオブジェクトのオブジェクトIDを設定する。
//	※ 先頭フィールドにLogicalFile::ObjectIDをnewする。
//
//	ARGUMENTS
//	const ModUInt64			ObjectID_
//		バリューオブジェクトのオブジェクトID
//	Common::DataArrayData*	Object_
//		オブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

// static
void
File::setObjectID(const ModUInt64 ObjectID_, Common::DataArrayData* Object_)
{
	// 呼び出し側がデータをセットしているので
	// ここでLogicalFile::ObjectIDを new することはやめた

	// ObjectIDを格納するフィールドを得る
	LogicalFile::ObjectID* pID =
		_SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, Object_->getElement(0).get());
	; _SYDNEY_ASSERT(pID);

	// オブジェクトIDを設定する
	pID->setValue(ObjectID_);
}

//
//	FUNCTION private
//	Btree::File::readKey -- キーフィールドを読み込む
//
//	NOTES
//	リーフページに記録されているキーオブジェクトから
//	各キーフィールドの値を読み込む。
//
//	ARGUMENTS
//	const ModUInt64			ValueObjectID_
//		バリューオブジェクトのオブジェクトID
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Common::Data::Pointer&	ResultObject_
//		オブジェクトへのポインタ
//		この引数が示すオブジェクトに
//		各キーフィールドの値を読み込む。
//	const bool				DoProjection_
//		プロジェクション指定がされていれば、指定されたフィールドの値だけを
//		読み込むかどうか
//			true  : プロジェクション指定がされていれば、
//			        指定されたフィールドの値だけを読み込む。
//			false : プロジェクション指定がされていても、
//			        オブジェクト全体を読み込む。
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::readKey(const ModUInt64			ValueObjectID_,
			  ValueFile*				ValueFile_,
			  Common::DataArrayData*	ResultObject_,
			  int&						iElement_,
			  const bool				DoProjection_,
			  PageVector&				AttachNodePages_,
			  PageVector&				AttachValuePages_) const
{
	//
	// まずはバリューオブジェクトに記録されている
	// リーフページの物理ページ識別子と
	// キー情報のインデックスを読み込む。
	//

	PhysicalFile::PageID	keyInfoPageID =
		PhysicalFile::ConstValue::UndefinedPageID;
	ModUInt32				keyInfoIndex = ModUInt32Max;

	ValueFile_->readLeafInfo(ValueObjectID_,
							 AttachValuePages_,
							 this->m_CatchMemoryExhaust,
							 keyInfoPageID,
							 keyInfoIndex);

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		this->readSimpleKey(keyInfoPageID,
							keyInfoIndex,
							ResultObject_,
							iElement_,
							DoProjection_,
							AttachNodePages_);

		return;
	}

	// バリューオブジェクトに辿ることができる
	// キー情報が記録されているリーフページを
	// アタッチする。
	PhysicalFile::Page*	keyInfoPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 keyInfoPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	// キー情報からキーオブジェクトのオブジェクトIDを読み込む

	KeyInformation	keyInfo(this->m_pTransaction,
							keyInfoPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	directKeyObjectID = keyInfo.readKeyObjectID();

	// 代表キーオブジェクトが記録されている
	// リーフ（物理）ページをアタッチする

	PhysicalFile::Page*		directKeyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  directKeyObjectID,
											  keyInfoPage,
											  directKeyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	const void*	directKeyObjectAreaTop = getConstAreaTop(
		directKeyObjectPage,
		Common::ObjectIDData::getLatterValue(directKeyObjectID));

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(directKeyObjectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	// キーオブジェクトから
	// 各キーフィールドの値を読み込む

	this->readKey(directKeyObjectPage,
				  directKeyObjectAreaTop,
				  ResultObject_,
				  iElement_,
				  DoProjection_,
				  AttachNodePages_);

	checkMemoryExhaust(keyInfoPage);
	if (attached)
	{
		checkMemoryExhaust(directKeyObjectPage);
	}
}

//
//	FUNCTION private
//	Btree::File::readKey --
//		ファイルに記録されている各キーフィールドの値を読み込む
//		（代表キーオブジェクトがノーマルキーオブジェクト）
//
//	NOTES
//	ファイルに記録されている各キーフィールドの値を読み込む。
//	（代表キーオブジェクトがノーマルキーオブジェクト）
//
//	ARGUMENTS
//	PhysicalFile::Page*		DirectKeyObjectPage_
//		代表キーオブジェクトが記録されているノードページの物理ページ記述子
//	void*					DirectKeyObjectAreaTop_
//		代表キーオブジェクトが記録されている物理エリア先頭へのポインタ
//	Common::DataArrayData*	ResultObject_
//		オブジェクトへのポインタ
//		この引数が示すオブジェクトに
//		各キーフィールドの値を読み込む。
//	const bool				DoProjection_
//		プロジェクション指定がされていれば、指定されたフィールドの値だけを
//		読み込むかどうか
//			true  : プロジェクション指定がされていれば、
//			        指定されたフィールドの値だけを読み込む。
//			false : プロジェクション指定がされていても、
//			        オブジェクト全体を読み込む。
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::readKey(PhysicalFile::Page*		DirectKeyObjectPage_,
			  const void*				DirectKeyObjectAreaTop_,
			  Common::DataArrayData*	ResultObject_,
			  int&						iElement_,
			  const bool				DoProjection_,
			  PageVector&				AttachNodePages_) const
{
	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(DirectKeyObjectAreaTop_);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	const NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(objectType + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const char*	keyTop =
		static_cast<const char*>(nullBitmap.getConstTail());

	// 各キーフィールドを読み込む…

	bool	doProjection =
		DoProjection_ && this->m_pOpenParameter->m_bFieldSelect;

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		if (doProjection &&
			File::isSelected(this->m_pOpenParameter, i) == false)
		{
			keyTop += this->m_cFileParameter.getFieldArchiveSize(i);

			continue;
		}

		// NullDataのインスタンスはgetInstanceでとるようにしたので修正
		Common::Data::Pointer	field;

		Common::DataType::Type	fieldType =
			*(this->m_cFileParameter.m_FieldTypeArray + i);

		if (existNull && nullBitmap.isNull(i - 1))
		{
			field = Common::NullData::getInstance();

			keyTop += this->m_cFileParameter.getFieldArchiveSize(i);
		}
		else if (*(this->m_cFileParameter.m_IsFixedFieldArray + i))
		{
			// 固定長キーフィールド…

			keyTop = File::readFixedField(fieldType, keyTop, field);
		}
		else
		{
			// 可変長キーフィールド…

			bool	isOutside =
				*(this->m_cFileParameter.m_FieldOutsideArray + i);

			Os::Memory::Size	fieldMaxLen =
				isOutside ?
					0 :
					*(this->m_cFileParameter.m_FieldMaxLengthArray + i);

			keyTop = File::readVariableField(this->m_pTransaction,
											 fieldType,
											 fieldMaxLen,
											 isOutside,
											 DirectKeyObjectPage_,
											 keyTop,
											 field,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachNodePages_);
		}

		; _SYDNEY_ASSERT(field.get() != 0);

		//ResultObject_->setElement(i, field);
		ResultObject_->getElement(iElement_++)->assign(field.get());
	}
}

//
//	FUNCTION private
//	Btree::File::readSimpleKey --
//		キー情報に記録されているキーフィールドの値を読み込む
//
//	NOTES
//	キー情報に記録されているキーフィールドの値を読み込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	KeyInfoPageID_,
//		キー情報が記録されているノードページの物理ページ識別子
//	const ModUInt32				KeyInfoIndex_
//		キー情報のインデックス
//	Common::DataArrayData*		ResultObject_
//		オブジェクトへのポインタ
//		この引数が示すオブジェクトに
//		各キーフィールドの値を読み込む。
//	const bool					DoProjection_
//		プロジェクション指定がされていれば、指定されたフィールドの値だけを
//		読み込むかどうか
//			true  : プロジェクション指定がされていれば、
//			        指定されたフィールドの値だけを読み込む。
//			false : プロジェクション指定がされていても、
//			        オブジェクト全体を読み込む。
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::readSimpleKey(const PhysicalFile::PageID	KeyInfoPageID_,
					const ModUInt32				KeyInfoIndex_,
					Common::DataArrayData*		ResultObject_,
					int&						iElement_,
					const bool					DoProjection_,
					PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	keyInfoPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 KeyInfoPageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	KeyInformation	keyInfo(this->m_pTransaction,
							keyInfoPage,
							KeyInfoIndex_,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	const NullBitmap::Value*	nullBitmapTop =
		keyInfo.assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const char*	key = static_cast<const char*>(keyInfo.assignConstKeyTop());

	; _SYDNEY_ASSERT(key != 0);

	bool	doProjection =
		DoProjection_ && this->m_pOpenParameter->m_bFieldSelect;

	for (int i = 1; i < this->m_cFileParameter.m_TopValueFieldIndex; i++)
	{
		Common::DataType::Type	keyDataType =
			*(this->m_cFileParameter.m_FieldTypeArray + i);

		; _SYDNEY_ASSERT(
			*(this->m_cFileParameter.m_IsFixedFieldArray + i));

		if (doProjection == false ||
			File::isSelected(this->m_pOpenParameter, i))
		{
			Common::Data::Pointer	field;

			if (existNull && nullBitmap.isNull(i - 1))
			{
				field = Common::NullData::getInstance();
			}
			else
			{
				FileCommon::DataManager::DataType type;
				type._name = keyDataType;
				field =
					FileCommon::DataManager::createCommonData(type);

				FileCommon::DataManager::readFixedCommonData(*field, key);
			}

			ResultObject_->getElement(iElement_++)->assign(field.get());
			//ResultObject_->setElement(i, field);
		}

		key +=
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				keyDataType);
	}

	checkMemoryExhaust(keyInfoPage);
}

// Common::Data::copyの仕様が変わったのでreadFixedFieldの引数を修正

#ifdef OBSOLETE
//
//	FUNCTION private
//	Btree::File::readFixedField -- 固定長フィールドの値を読み込む
//
//	NOTES
//	固定長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const int				FieldIndex_
//		フィールドインデックス
//	const char*				FieldTop_
//		読み込む固定長フィールドの値が記録されている領域へのポインタ
//	Common::Data::Pointer&	Field_
//		フィールド（オブジェクトの要素）へのポインタ
//		この引数が示すフィールドに値を読み込む。
//
//	RETURN
//	const char*
//		次のフィールドの値が記録されている領域へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
const char*
File::readFixedField(const int				FieldIndex_,
					 const char*			FieldTop_,
					 Common::Data::Pointer&	Field_) const
{
	; _SYDNEY_ASSERT(FieldTop_ != 0);
	; _SYDNEY_ASSERT(Field_.get() == 0);

	Common::DataType::Type	fieldDataType =
		*(this->m_cFileParameter.m_FieldTypeArray + FieldIndex_);

	return File::readFixedField(fieldDataType, FieldTop_, Field_);
}
#endif //OBSOLETE

//	FUNCTION private
//	Btree::File::readFixedField -- 固定長フィールドの値を読み込む
//
//	NOTES
//	固定長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Common::DataType::Type	FieldDataType_
//		フィールドのデータ型
//	const char*						FieldTop_
//		読み込む固定長フィールドの値が記録されている領域へのポインタ
//	Common::Data::Pointer&			Field_
//		フィールド(オブジェクトの要素）へのポインタ
//		この引数が示すフィールドに値を読み込む。
//
//	RETURN
//	const char*
//		次のフィールドの値が記録されている領域へのポインタ
//
//	EXCEPTIONS
//	[YET!]

// static
const char*
File::readFixedField(const Common::DataType::Type FieldDataType_,
					 const char* FieldTop_, Common::Data::Pointer& Field_)
{
	FileCommon::DataManager::DataType type;
	type._name = FieldDataType_;
	Field_ = FileCommon::DataManager::createCommonData(type);
	return FieldTop_ + Field_->setDumpedValue(FieldTop_);
}

//
//	FUNCTION private
//	Btree::File::attachObjectPage --
//		オブジェクトが記録されている物理ページをアタッチする
//
//	NOTES
//	オブジェクトが記録されている物理ページをアタッチする。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	const ModUInt64						ObjectID_
//		オブジェクトID
//	PhysicalFile::Page*					SrcPage_
//		基準となる物理ページの記述子
//		オブジェクトがこの引数が示す物理ページに
//		記録されているのであれば、
//		新たに物理ページをアタッチすることはない。
//	PhysicalFile::Page*&				ObjectPage_
//		オブジェクトが記録されている物理ページの記述子
//		この関数内で設定する
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		オブジェクトが記録されている物理ページをアタッチしたかどうか
//			true  : アタッチした
//			false : アタッチしていない
//			        （引数SrcPage_を引数ObjectPage_に設定しただけ）
//
//	EXCEPTIONS
//	[YET!]

// static
bool
File::attachObjectPage(
	const Trans::Transaction*			Transaction_,
	const ModUInt64						ObjectID_,
	PhysicalFile::Page*					SrcPage_,
	PhysicalFile::Page*&				ObjectPage_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);

	; _SYDNEY_ASSERT(SrcPage_ != 0);

	PhysicalFile::PageID	srcPageID = SrcPage_->getID();

	bool	attached = false;

	if (srcPageID == objectPageID)
	{
		// オブジェクトは、基準となる物理ページに記録されている…

		ObjectPage_ = SrcPage_;
	}
	else
	{
		// オブジェクトは、基準となる物理ページ以外に記録されている…

		attached = true;

		ObjectPage_ = File::attachPage(Transaction_,
									   SrcPage_->getFile(),
									   objectPageID,
									   FixMode_,
									   CatchMemoryExhaust_,
									   AttachPages_);
	}

	; _SYDNEY_ASSERT(ObjectPage_ != 0);

	return attached;
}

//
//	FUNCTION private
//	Btree::File::getAttachedPage -- 生成済みの物理ページ記述子を返す
//
//	NOTES
//	引数AttachPages_で指定されるページベクターに
//	引数PageID_が示す物理ページの記述子がキャッシュされていれば
//	その記述子を返し、
//	キャッシュされていなければ、ヌルポインタを返す。
//
//	ARGUMENTS
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	なし
//
// static
PhysicalFile::Page*
File::getAttachedPage(PageVector&					AttachPages_,
					  const PhysicalFile::PageID	PageID_)
{
	if (AttachPages_.isEmpty() == ModTrue)
	{
		return 0;
	}

	PageVector::Iterator	page = AttachPages_.begin();
	PageVector::Iterator	endOfPages = AttachPages_.end();

	while (page != endOfPages)
	{
		if ((*page)->getID() == PageID_)
		{
			return *page;
		}

		page++;
	}

	return 0;
}

//
//	FUNCTION private
//	Btree::File::attachPage -- 物理ページ記述子を生成する
//
//	NOTES
//	引数PageID_で示される物理ページの記述子を生成する。
//	既に生成されており、ページベクターにキャッシュされているのであれば、
//	単にその物理ページ記述子を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::File*					PhysicalFile_
//		物理ファイル記述子
//	PhysicalFile::PageID				PageID_
//		記述子を生成する物理ページの識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		生成した物理ページの記述子
//
//	EXCEPTIONS
//	[YET!]
//
// static
PhysicalFile::Page*
File::attachPage(const Trans::Transaction*			Transaction_,
				 PhysicalFile::File*				PhysicalFile_,
				 PhysicalFile::PageID				PageID_,
				 const Buffer::Page::FixMode::Value	FixMode_,
				 const bool							CatchMemoryExhaust_,
				 PageVector&						AttachPages_)
{
	PhysicalFile::Page*	page = 0;

	if (CatchMemoryExhaust_ == false)
	{
		page = File::getAttachedPage(AttachPages_, PageID_);
	}

	if (page == 0)
	{
		page = PhysicalFile_->attachPage(*Transaction_,
										 PageID_,
										 FixMode_);

		if (CatchMemoryExhaust_ == false)
		{
			AttachPages_.pushBack(page);
		}
	}

	return page;
}

//
//	FUNCTION private
//	Btree::File::detachPage -- 物理ページ記述子を破棄する
//
//	NOTES
//	引数Page_で示される物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	Btree::PageVector&							AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	PhysicalFile::Page*&						Page_
//		破棄する物理ページ記述子への参照
//	const PhysicalFile::Page::UnfixMode::Value	UnfixMode_
//		アンフィックスモード
//	const bool									SavePage_
//		デタッチする物理ページを、
//		物理ファイルマネージャでキャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュしない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::detachPage(PageVector&								AttachPages_,
				 PhysicalFile::Page*&						Page_,
				 const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
				 const bool									SavePage_) const
{
	File::detachPage(this->m_pPhysicalFile,
					 AttachPages_,
					 Page_,
					 UnfixMode_,
					 SavePage_);
}

//
//	FUNCTION private
//	Btree::File::detachPage -- 物理ページ記述子を破棄する
//
//	NOTES
//	引数Page_で示される物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	PhysicalFile::File*							PhysicalFile_
//		物理ファイル記述子
//	Btree::PageVector&							AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	PhysicalFile::Page*&						Page_
//		破棄する物理ページ記述子への参照
//	const PhysicalFile::Page::UnfixMode::Value	UnfixMode_
//		アンフィックスモード
//	const bool									SavePage_
//		デタッチする物理ページを、
//		物理ファイルマネージャでキャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュしない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::detachPage(PhysicalFile::File*						PhysicalFile_,
				 PageVector&								AttachPages_,
				 PhysicalFile::Page*&						Page_,
				 const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
				 const bool									SavePage_)
{
	PageVector::Iterator	page = AttachPages_.begin();
	PageVector::Iterator	endOfPages = AttachPages_.end();

	while (page != endOfPages)
	{
		if (*page == Page_)
		{
			PhysicalFile_->detachPage(*page, UnfixMode_, SavePage_);

			AttachPages_.erase(page);

			Page_ = 0;

			return;
		}

		page++;
	}

	PhysicalFile_->detachPage(Page_, UnfixMode_, SavePage_);

	Page_ = 0;
}

//
//	FUNCTION private
//	Btree::File::detachPageAll --
//		ページベクターにキャッシュされている
//		すべての物理ページ記述子を破棄する
//
//	NOTES
//	ページベクターにキャッシュされている
//	すべての物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	PhysicalFile::File*							PhysicalFile_
//		物理ファイル記述子
//	Btree::PageVector&							AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	const PhysicalFile::Page::UnfixMode::Value	UnfixMode_
//		アンフィックスモード
//	const bool									SavePage_
//		デタッチする物理ページを、
//		物理ファイルマネージャでキャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュしない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::detachPageAll(
	PhysicalFile::File*							PhysicalFile_,
	PageVector&									AttachPages_,
	const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
	const bool									SavePage_)
{
	if (AttachPages_.isEmpty() == ModFalse)
	{
		PageVector::Iterator	page = AttachPages_.begin();
		PageVector::Iterator	endOfPages = AttachPages_.end();

		while (page != endOfPages)
		{
			PhysicalFile_->detachPage(*page,
									  UnfixMode_,
									  SavePage_);

			page++;
		}

		AttachPages_.clear();
	}
	PhysicalFile_->unfixVersionPage(true);
}

//
//	FUNCTION private
//	Btree::File::recoverPageAll --
//		ページベクターにキャッシュされている
//		すべての物理ページ記述子を破棄し、
//		ページの内容をアタッチ前の状態に戻す
//
//	NOTES
//	ページベクターにキャッシュされている
//	すべての物理ページ記述子を破棄し、
//	ページの内容をアタッチ前の状態に戻す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_,
//		トランザクション記述子
//	PhysicalFile::File*			PhysicalFile_,
//		物理ファイル記述子
//	Btree::PageVector&			AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::recoverPageAll(const Trans::Transaction&	Transaction_,
					 PhysicalFile::File*		PhysicalFile_,
					 PageVector&				AttachPages_)
{
	if (AttachPages_.isEmpty() == ModFalse)
	{
		PageVector::Iterator	page = AttachPages_.begin();
		PageVector::Iterator	endOfPages = AttachPages_.end();

		for (; page != endOfPages; page++)
		{
			PhysicalFile_->recoverPage(Transaction_, *page);
		}

		AttachPages_.clear();
	}
	PhysicalFile_->unfixVersionPage(true);
}

//
//	FUNCTION private
//	Btree::File::freePageAll --
//		ページ識別子ベクターにキャッシュされている
//		すべての物理ページ識別子が示す物理ページを解放する
//
//	NOTES
//	ページ識別子ベクターにキャッシュされている
//	すべての物理ページ識別子が示す物理ページを解放する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			PhysicalFile_
//		物理ファイル記述子
//	Btree::PageIDVector&		PageIDs_
//		ページ識別子ベクターへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::freePageAll(const Trans::Transaction&	Transaction_,
				  PhysicalFile::File*		PhysicalFile_,
				  PageIDVector&				PageIDs_)
{
	if (PageIDs_.isEmpty() == ModFalse)
	{
		PageIDVector::Iterator	pageID = PageIDs_.begin();
		PageIDVector::Iterator	endOfPageIDs = PageIDs_.end();

		for (; pageID != endOfPageIDs; pageID++)
		{
			PhysicalFile_->freePage(Transaction_, *pageID);
		}

		PageIDs_.clear();
	}
	PhysicalFile_->unfixVersionPage(true);
}

//
//	FUNCTION private
//	Btree::File::reusePageAll --
//		ページ識別子ベクターにキャッシュされている
//		すべての物理ページ識別子が示す物理ページを再利用する
//
//	NOTES
//	ページ識別子ベクターにキャッシュされている
//	すべての物理ページ識別子が示す物理ページを再利用する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	PhysicalFile::File*			PhysicalFile_
//		物理ファイル記述子
//	Btree::PageIDVector&		PageIDs_
//		ページ識別子ベクターへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::reusePageAll(const Trans::Transaction&	Transaction_,
				   PhysicalFile::File*			PhysicalFile_,
				   PageIDVector&				PageIDs_)
{
	if (PageIDs_.isEmpty() == ModFalse)
	{
		PageIDVector::Iterator	pageID = PageIDs_.begin();
		PageIDVector::Iterator	endOfPageIDs = PageIDs_.end();

		for (; pageID != endOfPageIDs; pageID++)
		{
			PhysicalFile_->reusePage(Transaction_, *pageID);
		}

		PageIDs_.clear();
	}
	PhysicalFile_->unfixVersionPage(true);
}

//
//	FUNCTION private
//	Btree::File::getNextKeyInformationPage --
//		次のキー情報が記録されているリーフページを返す
//
//	NOTES
//	次のキー情報が記録されているリーフページを返す。
//	“次のキー情報”とは、
//	キー値順に次のキーオブジェクトへ辿ることができるキー情報のこと。
//
//	ARGUMENTS
//	Btree::NodePageHeader&	LeafPageHeader_
//		リーフページヘッダへの参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		次のキー情報が記録されているリーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::getNextKeyInformationPage(NodePageHeader&	LeafPageHeader_,
								PageVector&		AttachNodePages_) const
{
	const PhysicalFile::PageID nextLeafPageID =
		LeafPageHeader_.readNextLeafPageID();

	if (nextLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)

		// ※ 先頭物理ページには“使用中のキー情報”が必ず存在するはず

		return File::attachPage(this->m_pTransaction,
								this->m_pPhysicalFile,
								nextLeafPageID,
								this->m_FixMode,
								this->m_CatchMemoryExhaust,
								AttachNodePages_);

	// ファイル内で、キー値順に最後のリーフページであった。

	return 0;
}

//
//	FUNCTION private
//	Btree::File::assignNextKeyInformation --
//		リーフページ内の次のキー情報へ移動する
//
//	NOTES
//	引数KeyInfo_を、リーフページ内の次のキー情報へ移動する。
//	異なる物理ページへ移動した場合には、
//	引数LeafPage_およびLeafPageHeader_も更新される。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	LeafPage_
//		リーフページ記述子への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::NodePageHeader&	LeafPageHeader_
//		リーフページヘッダへの参照
//	Btree::KeyInformation&	KeyInfo_
//		キー情報への参照
//
//	RETURN
//	bool
//		次のキー情報へ移動できたかどうか
//			true  : 移動できた
//			false : 移動できなかった
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::assignNextKeyInformation(PhysicalFile::Page*&	LeafPage_,
							   PageVector&			AttachNodePages_,
							   NodePageHeader&		LeafPageHeader_,
							   KeyInformation&		KeyInfo_) const
{
	ModUInt32	useKeyInfoNum =
		LeafPageHeader_.readUseKeyInformationNumber();

	if (KeyInfo_.next(useKeyInfoNum - 1) == false)
	{
		// キーテーブル内で、次のキー情報へ移動できなかった…

		// つまり、キーテーブル内の最終キー情報であった。

		// 次のキーテーブルが記録されているリーフページを
		// アタッチする
		PhysicalFile::Page*	nextPage =
			this->getNextKeyInformationPage(LeafPageHeader_,
											AttachNodePages_);

		checkMemoryExhaust(LeafPage_);

		LeafPage_ = nextPage;

		if (LeafPage_ == 0)
		{
			// 次のリーフページが存在しなかった…

			// つまり、ファイル内で最終キー情報であった。

			return false;
		}

		//
		// リーフページヘッダと、キー情報の物理ページを更新する。
		// キー情報は、オフセットも更新する。
		//

		LeafPageHeader_.resetPhysicalPage(LeafPage_);

		KeyInfo_.resetPhysicalPage(LeafPage_);

		KeyInfo_.setStartOffsetByIndex(0);
	}

	return true;
}

//
//	FUNCTION private
//	Btree::File::getPrevKeyInformationPage --
//		前のキー情報が記録されているリーフページを返す
//
//	NOTES
//	前のキー情報が記録されているリーフページを返す。
//	“前のキー情報”とは、
//	キー値順に前のキーオブジェクトへ辿ることができるキー情報のこと。
//
//	ARGUMENTS
//	Btree::NodePageHeader&	LeafPageHeader_
//		リーフページヘッダへの参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		前のキー情報が記録されているリーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::getPrevKeyInformationPage(NodePageHeader&	LeafPageHeader_,
								PageVector&		AttachNodePages_) const
{
	const PhysicalFile::PageID prevLeafPageID =
		LeafPageHeader_.readPrevLeafPageID();

	if (prevLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)

		// ※ 先頭物理ページに使用中のキー情報が必ず存在するはず

		return File::attachPage(this->m_pTransaction,
								this->m_pPhysicalFile,
								prevLeafPageID,
								this->m_FixMode,
								this->m_CatchMemoryExhaust,
								AttachNodePages_);

	// ファイル内で、キー値順に先頭のリーフページであった。

	return 0;
}

//
//	FUNCTION private
//	Btree::File::assignPrevKeyInformation --
//		リーフページ内の前のキー情報へ移動する
//
//	NOTES
//	引数KeyInfo_を、リーフページ内の前のキー情報へ移動する。
//	異なる物理ページへ移動した場合には、
//	引数LeafPage_およびLeafPageHeader_も更新される。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	LeafPage_
//		リーフページ記述子への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::NodePageHeader&	LeafPageHeader_
//		リーフページヘッダへの参照
//	Btree::KeyInformation&	KeyInfo_
//		キー情報への参照
//
//	RETURN
//	bool
//		前のキー情報へ移動できたかどうか
//			true  : 移動できた
//			false : 移動できなかった
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::assignPrevKeyInformation(PhysicalFile::Page*&	LeafPage_,
							   PageVector&			AttachNodePages_,
							   NodePageHeader&		LeafPageHeader_,
							   KeyInformation&		KeyInfo_) const
{
	if (KeyInfo_.prev() == false)
	{
		// キーテーブル内で、前のキー情報へ移動できなかった…

		// つまり、キーテーブル内の先頭キー情報であった。

		// 前のキーテーブルが記録されているリーフページを
		// アタッチする
		PhysicalFile::Page*	prevPage =
			this->getPrevKeyInformationPage(LeafPageHeader_,
											AttachNodePages_);

		checkMemoryExhaust(LeafPage_);

		LeafPage_ = prevPage;

		if (LeafPage_ == 0)
		{
			// 前のリーフページが存在しなかった…

			// つまり、ファイル内で先頭キー情報であった。

			return false;
		}

		//
		// リーフページヘッダと、キー情報の物理ページを更新する。
		// キー情報は、オフセットも更新する。
		//

		LeafPageHeader_.resetPhysicalPage(LeafPage_);

		KeyInfo_.resetPhysicalPage(LeafPage_);

		const ModUInt32 useKeyInfoNum =
			LeafPageHeader_.readUseKeyInformationNumber();
		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInfo_.setStartOffsetByIndex(useKeyInfoNum - 1);
	}

	return true;
}

//
//	FUNCTION private
//	Btree::File::hasNullField -- ヌル値のフィールドが含まれているか調べる
//
//	NOTES
//	引数Object_が指すオブジェクトに
//	ヌル値を持つフィールドが含まれているかどうかを調べる。
//	配列フィールドの場合には、ヌル値の要素が含まれているかどうかまで
//	調べる。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Object_
//		オブジェクトへのポインタ
//
//	RETURN
//	bool
//		ヌル値のフィールドが含まれているかどうか
//			true  : ヌル値のフィールドが含まれている
//			false : ヌル値のフィールドが含まれていない
//
//	EXCEPTIONS
//	なし
//
bool
File::hasNullField(const Common::DataArrayData*	Object_) const
{
	//
	// 各フィールドについて、ヌル値かどうかを調べる。
	//

	for (int i = 1; i < this->m_cFileParameter.m_FieldNum; i++)
	{
		Common::Data*	field = Object_->getElement(i).get();

		if (field->isNull())
		{
			// フィールドデータ型がCommon::DataType::Null…

			// つまり、ヌル値。

			return true;
		}
		else if (field->getType() == Common::DataType::Array)
		{
			// フィールドデータ型がCommon::DataType::Array…

			//
			// フィールドが配列なので、その配列の
			// 各要素について、ヌル値かどうかを調べる。
			//

			Common::DataArrayData*	arrayField =
				_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, field);

			; _SYDNEY_ASSERT(arrayField != 0);

			int	elementNum = arrayField->getCount();

			for (int j = 0; j < elementNum; j++)
			{
				Common::Data*	element = arrayField->getElement(j).get();

				if (element->isNull())
				{
					// 要素データ型がCommon::DataType::Null…

					// つまり、ヌル値。

					return true;
				}
			}
		}
	}

	return false;
}

//	FUNCTION private
//	Btree::File::getAreaTop -- 物理エリア先頭へのポインタを返す
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page*			Page_
//		物理ページ記述子
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	void*
//		物理エリア先頭へのポインタ
//
//	EXCEPTIONS
//	なし

// static
void*
File::getAreaTop(PhysicalFile::Page* Page_, const PhysicalFile::AreaID AreaID_)
{
	; _SYDNEY_ASSERT(Page_);
	return File::getAreaTop(*Page_, AreaID_);
}

//	FUNCTION private
//	Btree::File::getConstAreaTop -- 物理エリア先頭へのポインタを返す
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::Page*	Page_
//		物理ページ記述子
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	const void*
//		物理エリア先頭へのポインタ
//
//	EXCEPTIONS

// static
const void*
File::getConstAreaTop(const PhysicalFile::Page* Page_,
					  const PhysicalFile::AreaID AreaID_)
{
	; _SYDNEY_ASSERT(Page_);
	return File::getConstAreaTop(*Page_, AreaID_);
}

//	FUNCTION private
//	Btree::File::rmdir -- ディレクトリを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//
//		FileParameter*		param
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
File::rmdir(const Os::Path& path, const FileParameter* param/*=0*/)
{
	try	{
		if (Os::Directory::access(path, Os::Directory::AccessMode::File))
			Os::Directory::remove(path);
	} catch (...) {

		if (param)
			Checkpoint::Database::setAvailability(
				param->m_IDNumber->getLockName(), false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Btree::File::rmdirOnError -- ディレクトリを削除する
//
//	NOTES
//		エラー処理の中で実行されることを前提にしている
//
//	ARGUMENTS
//		Os::Path&			path
//
//		FileParameter*		param
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
File::rmdirOnError(const Os::Path& path, const FileParameter* param/*=0*/)
{
	//注意！！エラー処理の中で実行されることを前提にしている。

	try {
		if (Os::Directory::access(path, Os::Directory::AccessMode::File))
			Os::Directory::remove(path);
	} catch (...) {

		if (param)
			Checkpoint::Database::setAvailability(
				param->m_IDNumber->getLockName(), false);
	}
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
