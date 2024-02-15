// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Search.cpp -- Ｂ＋木ファイルクラスの実現ファイル（検索関連）
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
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
#include "Exception/IllegalFileAccess.h"
#include "Exception/MemoryExhaust.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "Btree/FileInformation.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::File::search -- オブジェクトを検索する
//
//	NOTES
//	B+木ファイルオープン後、または、リセット後の
//	初回のFile::get()呼び出しの際のオブジェクト検索を行なう。
//	関数名が"search"だが、Scanモードの場合も、
//	Fetchモードの場合も、Searchモードの場合も、
//	更にSearch + Fetchモードの場合にも
//	本関数が適切な（利用者に返すべき）オブジェクトを
//	検索する。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Common::DataArrayData* pTuple_
//		検索結果であるオブジェクトへのポインタ。
//
//	RETURN
//		ファイル内に該当オブジェクトが存在した場合には、true を返す。
//		ファイル内に該当オブジェクトが存在しない場合には、false を返す。
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::search(FileInformation&	FileInfo_,
			 ValueFile*			ValueFile_,
			 Common::DataArrayData* pTuple_)
{
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

		attachNodePages.reserve(FileInfo_.readTreeDepth() * 2);
		attachValuePages.reserve(4);

		try
		{
			// オブジェクトを検索する
			// ※ Search + Fetch の場合には、
			// 　 検索条件と一致するオブジェクトの
			// 　 オブジェクトIDが得られる。
			// 　 そのオブジェクトIDが示すオブジェクトが
			// 　 Fetch検索条件とも一致するかどうかの
			// 　 判断は、後の処理で行う。
			this->m_ullObjectID = this->search(FileInfo_,
											   attachNodePages,
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
												   pTuple_,//[out]
												   true); // 初回のget
				}
				else
				{
					// Search + Fetchモード以外…

					// では、オブジェクトを
					// ファイルから読み込みましょう。
					bResult = this->getObject(
								  this->m_ullObjectID,
								  ValueFile_,
								  pTuple_,//[out]
								  true, // プロジェクションフィールドのみ読み込む
								  attachNodePages,
								  attachValuePages);
				}
			}

			// 下位の関数がキャッシュした
			// ノードページ／バリューページをデタッチする

			File::detachPageAll(this->m_pPhysicalFile,attachNodePages,ValueFile_->m_PhysicalFile,attachValuePages,PhysicalFile::Page::UnfixMode::NotDirty,this->m_SavePage);

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

				bResult = false;

				this->m_CatchMemoryExhaust = true;

				File::detachPageAll(this->m_pPhysicalFile,attachNodePages,ValueFile_->m_PhysicalFile,attachValuePages,PhysicalFile::Page::UnfixMode::NotDirty,false);
			}
		}

	} // end while (true)

	return bResult;
}

//
//	FUNCTION private
//	Btree::File::searchAndFetch --
//		Search + Fetchでオブジェクトを検索する
//
//	NOTES
//	Search + Fetchでのオブジェクト検索時、
//	検索条件と一致するオブジェクトが見つかったのちに、
//	Fetch検索条件とも一致するオブジェクトを検索する。
//	つまり、検索条件と一致するオブジェクトの検索は、
//	呼び出し側で行い、その後のFetch検索条件と一致するオブジェクトの
//	検索を本関数が行う。
//
//	ARGUMENTS
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Common::DataArrayData*&	ResultObject_
//		検索条件ともFetch検索条件とも一致する
//		オブジェクトを指すポインタ
//	const bool				FirstGet_
//		初回のFile::get()での呼び出しかどうか
//			true  : 初回のFile::get()
//			false : 2回目以降のFile::get()
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::searchAndFetch(PageVector&				AttachNodePages_,
					 ValueFile*					ValueFile_,
					 PageVector&				AttachValuePages_,
					 Common::DataArrayData*		ResultObject_,
					 const bool					FirstGet_)
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchAndFetchSimpleKey(AttachNodePages_,
											 ValueFile_,
											 AttachValuePages_,
											 ResultObject_,
											 FirstGet_);
	}

	// 検索条件と一致するオブジェクトは
	// 既に見つかっているはずなので、
	// 適切なオブジェクトIDが設定されているはず。
	; _SYDNEY_ASSERT(this->m_ullObjectID != FileCommon::ObjectID::Undefined);

	//
	// バリューオブジェクトが記録されているバリューページをアタッチする
	//

	PhysicalFile::AreaID valueObjectAreaID =
		Common::ObjectIDData::getLatterValue(m_ullObjectID);

	PhysicalFile::Page*	valuePage = File::attachPage(
		m_pTransaction, ValueFile_->m_PhysicalFile,
		Common::ObjectIDData::getFormerValue(m_ullObjectID),
		m_FixMode, m_CatchMemoryExhaust, AttachValuePages_);

	//
	// Fetch検索条件を設定する
	//

	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(
		this->m_FetchOptionData->getElementType() == Common::DataType::Data);

	const Common::DataArrayData*	fetchCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 this->m_FetchOptionData.get());

	; _SYDNEY_ASSERT(fetchCondition != 0);

	if (FirstGet_)
	{
		// 初回のgetで呼び出された…

		// では、Fetchのためのヒントを設定する。
		this->m_FetchHint.setCondition(fetchCondition);
	}

	//
	// まずは、キーフィールドがFetch検索条件と一致しているかを
	// 確認する。
	//

	// バリューオブジェクトから、
	// リーフページの物理ページ識別子と
	// キー情報のインデックスを読み込む。

	const void*	objectAreaTop = File::getConstAreaTop(valuePage,
													  valueObjectAreaID);

	ValueFile_->readLeafInfo(objectAreaTop,
							 this->m_LeafPageID,
							 this->m_KeyInfoIndex);

	// キー情報が記録されているリーフページをアタッチする
	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 this->m_LeafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	// キー情報から、キーオブジェクトのオブジェクトIDを読み込む
	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							this->m_KeyInfoIndex,
							true); // リーフページ

	ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

	bool	exist = true;

	if (this->compareToFetchCondition(leafPage,
									  AttachNodePages_,
									  keyObjectID,
									  fetchCondition)
		!= 0)
	{
		// キーフィールドがFetch検索条件と一致しなかった…

		//
		// しょうがないので、検索条件ともFetch検索条件とも
		// 一致するキーオブジェクトを
		// “次、次…”（または“前、前…”）と
		// 検索していく…
		// （下はそのためのループ）
		//

		bool	lp = true;
		do
		{
			if (this->m_pOpenParameter->m_bSortReverse)
			{
				// オブジェクトの挿入ソート順とは逆順に
				// オブジェクトをget…

				// “前、前…”
				this->m_ullObjectID =
					this->getPrevObjectIDSearch(AttachNodePages_);
			}
			else
			{
				// オブジェクトの挿入ソート順にオブジェクトをget…

				// “次、次…”
				this->m_ullObjectID =
					this->getNextObjectIDSearch(AttachNodePages_);
			}

			if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
			{
				// 検索条件にすら一致するオブジェクトがもうない…

				exist = false;

				break;
			}

			const PhysicalFile::PageID valuePageID =
				Common::ObjectIDData::getFormerValue(m_ullObjectID);
			valueObjectAreaID =
				Common::ObjectIDData::getLatterValue(m_ullObjectID);

			if (valuePage->getID() != valuePageID)
			{
				// 今回のオブジェクトは、
				// 以前アタッチしていたバリューページとは、
				// 別のバリューページに記録されている…

				// バリューページのアタッチし直し

				if (this->m_CatchMemoryExhaust)
				{
					ValueFile_->m_PhysicalFile->detachPage(
						valuePage,
						PhysicalFile::Page::UnfixMode::NotDirty,
						false); // 本当にデタッチ（アンフィックス）
						        // してしまい、
				                // 物理ファイルマネージャが
				                // キャッシュしないようにする。
				}

				valuePage = File::attachPage(this->m_pTransaction,
											 ValueFile_->m_PhysicalFile,
											 valuePageID,
											 this->m_FixMode,
											 this->m_CatchMemoryExhaust,
											 AttachValuePages_);

			}

			// バリューオブジェクトから、
			// リーフページの物理ページ識別子と
			// キー情報のインデックスを読み込む。

			const void*	objectAreaTop =
				File::getConstAreaTop(valuePage,
									  valueObjectAreaID);

			ValueFile_->readLeafInfo(objectAreaTop,
									 this->m_LeafPageID,
									 this->m_KeyInfoIndex);

			if (leafPage->getID() != this->m_LeafPageID)
			{
				// キー情報が記録されているリーフページのアタッチし直し

				checkMemoryExhaust(leafPage);

				leafPage = File::attachPage(this->m_pTransaction,
											this->m_pPhysicalFile,
											this->m_LeafPageID,
											this->m_FixMode,
											this->m_CatchMemoryExhaust,
											AttachNodePages_);

				keyInfo.resetPhysicalPage(leafPage);
			}

			keyInfo.setStartOffsetByIndex(this->m_KeyInfoIndex);

			keyObjectID = keyInfo.readKeyObjectID();

			if (this->compareToFetchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID,
											  fetchCondition)
				== 0)
			{
				// 検索条件ともFetch検索条件とも
				// 一致するキーオブジェクトが見つかった…

				break;
			}
		}
		while (lp);
	}

	if (exist == false)
	{
		// 検索条件ともFetch検索条件とも一致するオブジェクトは
		// 存在しなかった…

		this->m_LeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
		this->m_KeyInfoIndex = ModUInt32Max;

		if (this->m_CatchMemoryExhaust)
		{
			ValueFile_->m_PhysicalFile->detachPage(valuePage,PhysicalFile::Page::UnfixMode::NotDirty,false); // 本当にデタッチ（アンフィックス）してしまい、
				        // 物理ファイルマネージャが
				        // キャッシュしないようにする。
		}
		checkMemoryExhaust(leafPage);

		return false;
	}

	checkMemoryExhaust(leafPage);

	//
	// キーフィールドがFetch検索条件と一致していたので、
	// 次にバリューフィールドがFetch検索条件と一致しているかを
	// 確認する。
	//

	if (this->m_FetchHint.m_OnlyKey == false)
	{
		// バリューフィールドもFetch対象フィールドである…

		if (this->compareToFetchCondition(valuePage,
										  AttachValuePages_,
										  valueObjectAreaID,
										  fetchCondition,
										  FetchHint::CompareType::OnlyValue,
										  false) // バリューオブジェクト
			!= 0)
		{
			// バリューフィールドがFetch検索条件と一致しなかった…

			//
			// しょうがないので、検索条件ともFetch検索条件とも
			// 一致するキーオブジェクトを
			// “次、次…”（または“前、前…”）と
			// 検索していく…
			// （下はそのためのループ）
			//

			bool	lp = true;
			do
			{
				if (this->m_pOpenParameter->m_bSortReverse)
				{
					// オブジェクトの挿入ソート順とは逆順に
					// オブジェクトをget…

					// “前、前…”
					this->m_ullObjectID =
						this->getPrevObjectIDSearch(AttachNodePages_);
				}
				else
				{
					// オブジェクトの挿入ソート順にオブジェクトをget…

					// “次、次…”
					this->m_ullObjectID =
						this->getNextObjectIDSearch(AttachNodePages_);
				}

				if (this->m_ullObjectID == FileCommon::ObjectID::Undefined)
				{
					// 検索条件ともFetch検索条件とも一致するオブジェクトは
					// 存在しなかった…

					this->m_LeafPageID =
						PhysicalFile::ConstValue::UndefinedPageID;
					this->m_KeyInfoIndex = ModUInt32Max;

					if (this->m_CatchMemoryExhaust)
					{
						ValueFile_->m_PhysicalFile->detachPage(
							valuePage,
							PhysicalFile::Page::UnfixMode::NotDirty,
							false); // 本当にデタッチ（アンフィックス）
							        // してしまい、
							        // 物理ファイルマネージャが
							        // キャッシュしないようにする。
					}

					return false;
				}

				const PhysicalFile::PageID valuePageID =
					Common::ObjectIDData::getFormerValue(m_ullObjectID);
				valueObjectAreaID =
					Common::ObjectIDData::getLatterValue(m_ullObjectID);

				if (valuePage->getID() != valuePageID)
				{
					// 今回のオブジェクトは、
					// 以前アタッチしていたバリューページとは、
					// 別のバリューページに記録されている…

					// バリューページのアタッチし直し

					if (this->m_CatchMemoryExhaust)
					{
						ValueFile_->m_PhysicalFile->detachPage(
							valuePage,
							PhysicalFile::Page::UnfixMode::NotDirty,
							false); // 本当にデタッチ（アンフィックス）
									// してしまい、
							        // 物理ファイルマネージャが
							        // キャッシュしないようにする。
					}

					valuePage = File::attachPage(this->m_pTransaction,
												 ValueFile_->m_PhysicalFile,
												 valuePageID,
												 this->m_FixMode,
												 this->m_CatchMemoryExhaust,
												 AttachValuePages_);
				}

				// バリューオブジェクトから、
				// リーフページの物理ページ識別子と
				// キー情報のインデックスを読み込む。

				const void*	objectAreaTop =
					File::getConstAreaTop(valuePage,
										  valueObjectAreaID);

				ValueFile_->readLeafInfo(objectAreaTop,
										 this->m_LeafPageID,
										 this->m_KeyInfoIndex);

				if (this->compareToFetchCondition(
						valuePage,
						AttachValuePages_,
						valueObjectAreaID,
						fetchCondition,
						FetchHint::CompareType::OnlyValue,
						false) // バリューオブジェクト
					== 0)
				{
					// 検索条件ともFetch検索条件とも
					// 一致するバリューオブジェクトが見つかった…

					break;
				}
			}
			while (lp);
		}
	}

	if (this->m_CatchMemoryExhaust)
	{
		ValueFile_->m_PhysicalFile->detachPage(
			valuePage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	// 検索条件ともFetch検索条件とも完全に一致したオブジェクトが
	// 見つかったので、オブジェクト全体をファイルから読み込む。
	return this->getObject(this->m_ullObjectID,
						   ValueFile_,
						   ResultObject_,
						   true, // プロジェクションフィールドのみ読み込む
						   AttachNodePages_,
						   AttachValuePages_);
}

//
//	FUNCTION private
//	Btree::File::search -- オブジェクトを検索する
//
//	NOTES
//	オブジェクトを検索し、そのオブジェクトのIDを返す。
//	関数名が"search"だが、Scanモードの場合も、
//	Fetchモードの場合も、Searchモードの場合も、
//	本関数が適切な（利用者に返すべき）オブジェクトを
//	検索する。
//	ただし、Search + Fetchモードの場合は、
//	検索条件と一致するオブジェクトを検索する。
//	（そのオブジェクトがFetch検索条件とも一致するかどうかは、
//	　呼び出し側に任せてある。）
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&	AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		利用者に返すべきオブジェクトのオブジェクトID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::search(FileInformation&	FileInfo_,
			 PageVector&		AttachNodePages_,
			 ValueFile*			ValueFile_,
			 PageVector&		AttachValuePages_)
{
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (this->m_pOpenParameter->m_iOpenMode ==
		FileCommon::OpenMode::Read)
	{
		// Scanモードまたは、Fetch（単独）モード…

		if (this->m_pOpenParameter->m_iReadSubMode ==
			OpenParameter::ScanRead)
		{
			// Scanモード…

			//
			// ということは、オープン時に指定されている
			// オブジェクト取得順により、
			// “先頭オブジェクト”または“最終オブジェクト”を
			// 返す。
			//

			if (this->m_pOpenParameter->m_bSortReverse)
			{
				// オブジェクトが挿入されているソート順の
				// 逆順でオブジェクトを取得するように
				// オープン時に指定されている…

				// オブジェクトID順またはキー値順に
				// 最後のオブジェクトを取得する
				resultObjectID = this->getLastObjectID(FileInfo_,
													   AttachNodePages_);
			}
			else
			{
				// オブジェクトが挿入されているソート順で
				// オブジェクトを取得するように
				// オープン時に指定されている…

				// オブジェクトID順またはキー値順に
				// 先頭のオブジェクトを取得する
				resultObjectID = this->getTopObjectID(FileInfo_,
													  AttachNodePages_);
			}
		}
		else
		{
			// Fetchモード…

			; _SYDNEY_ASSERT(
				this->m_pOpenParameter->m_iReadSubMode ==
				OpenParameter::FetchRead);

			if (this->m_FetchOptionData.get() == 0)
			{
				// File::fetch→File::getの順で
				// 関数を呼び出していない…
				// または、
				// File::fetchが正常に完了していない…

				throw Exception::IllegalFileAccess(moduleName,
												   srcFile, 
												   __LINE__);
			}

			if (this->m_FetchHint.m_ByKey)
			{
				// キー値でFetch

				ModUInt32	treeDepth = FileInfo_.readTreeDepth();

				PhysicalFile::PageID	rootNodePageID =
					FileInfo_.readRootNodePageID();

				if (this->m_pOpenParameter->m_bSortReverse)
				{
					// オブジェクトが挿入されているソート順の
					// 逆順でオブジェクトを取得するように
					// オープン時に指定されている…

					resultObjectID = this->fetchByKeyRev(treeDepth,
														 rootNodePageID,
														 AttachNodePages_,
														 ValueFile_,
														 AttachValuePages_);
				}
				else
				{
					// オブジェクトが挿入されているソート順で
					// オブジェクトを取得するように
					// オープン時に指定されている…

					resultObjectID = this->fetchByKey(treeDepth,
													  rootNodePageID,
													  AttachNodePages_,
													  ValueFile_,
													  AttachValuePages_);
				}
			}
			else
			{
				// オブジェクトIDでFetch…
#ifdef OBSOLETE // 将来に対する予約
				resultObjectID = this->fetchByOID(ValueFile_,
												  AttachValuePages_);
#else //OBSOLETE
				throw Exception::NotSupported(moduleName, srcFile, __LINE__);
#endif
			}
		}
	}
	else
	{
		// Searchモード…

		; _SYDNEY_ASSERT(
			this->m_pOpenParameter->m_iOpenMode ==
			FileCommon::OpenMode::Search);

		if (this->m_pOpenParameter->m_bSortReverse)
		{
			// オブジェクトが挿入されているソート順の
			// 逆順でオブジェクトを取得するように
			// オープン時に指定されている…

			resultObjectID = this->searchByKeyRev(FileInfo_,
												  AttachNodePages_,
												  ValueFile_,
												  AttachValuePages_);
		}
		else
		{
			// オブジェクトが挿入されているソート順で
			// オブジェクトを取得するように
			// オープン時に指定されている…

			resultObjectID = this->searchByKey(FileInfo_,
											   AttachNodePages_);
		}
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::getTopObjectID --
//		先頭オブジェクトのオブジェクトIDを返す
//
//	NOTES
//	（キー値順での）先頭オブジェクトのオブジェクトIDを返す。
//	ただし、引数GetKeyObjectID_がtrueの場合には、
//	バリューオブジェクトのオブジェクトIDではなく、
//	キーオブジェクトのオブジェクトIDを返す。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					GetKeyObjectID_ = false
//		キーオブジェクトのオブジェクトIDを返すかどうか
//			true  : キーオブジェクトのオブジェクトIDを返す
//			false : バリューオブジェクトのオブジェクトIDを返す
//
//	RETURN
//	ModUInt64
//		先頭オブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getTopObjectID(FileInformation&	FileInfo_,
					 PageVector&		AttachNodePages_,
					 const bool			GetKeyObjectID_ // = false
					 ) const
{
	// キーテーブルが、1つの物理ページに収まろうが
	// 収まらなかろうが、いずれの場合も
	// キー値順で先頭のオブジェクトには、
	// 同じ方法で辿れる。
	// （先頭リーフページの先頭物理ページの
	// 　先頭キー情報を参照すればよい。）

	PhysicalFile::PageID	leafPageID = FileInfo_.readTopLeafPageID();

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							0,    // 先頭のキー情報
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	topObjectID =
		GetKeyObjectID_ ?
			keyInfo.readKeyObjectID() : keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(topObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return topObjectID;
}

//
//	FUNCTION private
//	Btree::File::getLastObjectID --
//		最終オブジェクトのオブジェクトIDを返す
//
//	NOTES
//	（キー値順での）最終オブジェクトのオブジェクトIDを返す。
//	ただし、引数GetKeyObjectID_がtrueの場合には、
//	バリューオブジェクトのオブジェクトIDではなく、
//	キーオブジェクトのオブジェクトIDを返す。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					GetKeyObjectID_ = false
//		キーオブジェクトのオブジェクトIDを返すかどうか
//			true  : キーオブジェクトのオブジェクトIDを返す
//			false : バリューオブジェクトのオブジェクトIDを返す
//
//	RETURN
//	ModUInt64
//		最終オブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::getLastObjectID(FileInformation&	FileInfo_,
					  PageVector&		AttachNodePages_,
					  const bool		GetKeyObjectID_ // = false
					  ) const
{
	// ファイル管理情報に記録されている
	//     「最終リーフページの物理ページ識別子」
	// へのポインタを設定する。
	// “最終リーフページ”とは、
	// キー値順で最後のキーオブジェクトが記録されている
	// リーフページであり、
	// また、ここに記録されているのは、
	// リーフページを構成する物理ページのうちの
	// 先頭物理ページの識別子である。
	PhysicalFile::PageID	leafPageID =
		FileInfo_.readLastLeafPageID();

	// 最終リーフページを構成する物理ページのうちの
	// 先頭の物理ページをアタッチする。
	// ※ キーテーブルはこの物理ページに存在する。

	PhysicalFile::Page*	leafPage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 leafPageID,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(leafPage != 0);

	// リーフページヘッダに記録されている
	//     「使用中のキー情報数」
	// へのポインタを設定する。
	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	const ModUInt32 useKeyInfoNum =
		leafPageHeader.readUseKeyInformationNumber();
	; _SYDNEY_ASSERT(useKeyInfoNum > 0);

	// キーテーブル中の最後のキー情報に記録されている
	//     「バリューオブジェクトのオブジェクトID」
	// を読み込む。
	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							useKeyInfoNum - 1,
							true, // リーフページ
							this->m_cFileParameter.m_KeyNum,
							this->m_cFileParameter.m_KeySize);

	ModUInt64	lastObjectID =
		GetKeyObjectID_ ?
			keyInfo.readKeyObjectID() : keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(lastObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return lastObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchByKey --
//		キー値によりオブジェクトを検索する
//
//	NOTES
//	キー値によりオブジェクトを検索し、
//	該当するオブジェクトが存在すればそのオブジェクトのIDを返し、
//	該当するオブジェクトが存在しなければ、
//	FileCommon::ObjectID::Undefinedを返す。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//	※ Search + Fetchモードの場合、
//	　 Fetch検索条件と一致するかどうかは
//	　 本関数以下では意識していない。
//	　 もしかしたらFetch検索条件とは一致しないオブジェクトの
//	　 オブジェクトIDを返すかもしれないので、
//	　 呼び出し側でその確認が必要である。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByKey(FileInformation&	FileInfo_,
				  PageVector&		AttachNodePages_) const
{
	if (this->m_SearchHint.m_ConditionType ==
		SearchHint::ConditionType::Single)
	{
		// 先頭キーフィールドのみでの検索…

		if (this->m_SearchHint.m_LastIsSpan)
		{
			// 範囲指定…

			return this->searchBySpanCondition(FileInfo_,
											   AttachNodePages_);
		}
		else
		{
			// 単一条件…

			return this->searchBySingleCondition(FileInfo_,
												 AttachNodePages_);
		}
	}
	else
	{
		// 複数キーフィールドでの検索（複合条件）…

		; _SYDNEY_ASSERT(
			this->m_SearchHint.m_ConditionType ==
			SearchHint::ConditionType::Multi);

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->searchByMultiCondition(treeDepth,
											rootNodePageID,
											AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::searchBySingleCondition --
//		先頭キーフィールドへの単一条件でオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドへの単一条件でオブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		検索条件と一致するオブジェクトのオブジェクトID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleCondition(FileInformation&	FileInfo_,
							  PageVector&		AttachNodePages_) const
{
	// 先頭キーフィールドへの単一条件なので、
	// m_SearchHint.m_StartOperatorArray[0]に
	// 比較演算子が設定されている。

	if (*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::Equals)
	{
		// 比較演算子がEquals…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->searchBySingleEquals(treeDepth,
										  rootNodePageID,
										  AttachNodePages_);
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThan)
	{
		// 比較演算子がLessThan…

		return this->searchBySingleLessThan(FileInfo_,
											AttachNodePages_,
											false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		// 比較演算子がLessThanEquals…

		return this->searchBySingleLessThan(FileInfo_,
											AttachNodePages_,
											true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThan)
	{
		// 比較演算子がGreaterThan…

		return this->searchBySingleGreaterThan(FileInfo_,
											   AttachNodePages_,
											   false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		// 比較演算子がGreaterThanEquals…

		return this->searchBySingleGreaterThan(FileInfo_,
											   AttachNodePages_,
											   true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::Like)
	{
		// like演算子による文字列検索…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->likeSearch(treeDepth,
								rootNodePageID,
								AttachNodePages_);
	}
	else
	{
		// 比較演算子がEqualsToNull…
		// （のはず）

		; _SYDNEY_ASSERT(
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::EqualsToNull);

		return this->searchBySingleEqualsToNull(FileInfo_,
												AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEquals --
//		先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	const ModUInt64				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEquals(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchBySingleEqualsSimpleKey(TreeDepth_,
												   RootNodePageID_,
												   AttachNodePages_);
	}

	// 先頭キーフィールドが検索条件と一致するオブジェクトが
	// 記録されている可能性があるリーフページを検索する。
	// ※ ここで得られる物理ページ記述子は、
	// 　 “キーオブジェクト”が記録されている
	// 　 物理ページの記述子だとは限らない。
	// 　 “キー情報”が記録されている物理ページの記述子である。
	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForSingleCondition(TreeDepth_,
											   RootNodePageID_,
											   AttachNodePages_);

	if (leafPage == 0)
	{
		// 先頭キーフィールドが検索条件と一致するオブジェクトが
		// 記録されている可能性があるリーフページすら
		// 存在しなかった…

		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	// 先頭キーフィールドが検索条件に最も近い値の
	// キーオブジェクトへ辿るキー情報のインデックスを取得する。
	// そのキーオブジェクトが検索条件と一致しているかどうかは、
	// File::getKeyInformationIndexForSingleConditionが設定する
	// ローカル変数matchを参照すればわかる。
	int	keyInfoIndex =
		this->getKeyInformationIndexForSingleCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		// 先頭キーフィールドが検索条件と一致する
		// キーオブジェクトは存在しなかった…

		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	//
	// ここまでたどり着いたということは、
	// 先頭キーフィールドが検索条件と一致する
	// キーオブジェクトが見つかったということ！
	//

	// 先頭キーフィールドが検索条件と一致する
	// キーオブジェクトへ辿るキー情報から
	// バリューオブジェクトのオブジェクトIDを読み込む。

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	checkMemoryExhaust(leafPage);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleLessThan --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&							FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool										CntainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//		※ File::searchBySpanCondition()のための引数である。
//	PhysicalFile::PageID*							KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーオブジェクトへ辿れる
//		キー情報が記録されているリーフページの
//		物理ページ識別子へのポインタ
//		※ File::searchBySpanCondition()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*										KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//	ModUInt64*										KeyObjectID_ = 0
//		同、キーオブジェクトのオブジェクトIDへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleLessThan(
	FileInformation&						FileInfo_,
	PageVector&								AttachNodePages_,
	const bool								ContainEquals_,
	const SearchHint::CompareTarget::Value	Target_,            // = Start
	PhysicalFile::PageID*					KeyInfoLeafPageID_, // = 0
	ModUInt32*								KeyInfoIndex_,      // = 0
	ModUInt64*								KeyObjectID_        // = 0
	) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		; _SYDNEY_ASSERT(KeyObjectID_ == 0);

		return this->searchBySingleLessThanSimpleKey(FileInfo_,
													 AttachNodePages_,
													 ContainEquals_,
													 Target_,
													 KeyInfoLeafPageID_,
													 KeyInfoIndex_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// 「 key1 < 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に先頭オブジェクト”となる。

		// でも、“キー値順に先頭オブジェクト”の
		// 先頭キーフィールドの値に、例えば、100という値が
		// 記録されていたとして、
		// 検索条件が「 key1 < 50 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に先頭オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に先頭オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleLessThanは、
		// File::searchBySpanConditionからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が降順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0 &&
						 KeyObjectID_ == 0);

		// しかも、“検索開始条件”と比較するはず。
		; _SYDNEY_ASSERT(Target_ == SearchHint::CompareTarget::Start);

		keyObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 true); // キーオブジェクトの
								        // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		int	compareResult = compareToTopSearchCondition(
			leafPage,
			AttachNodePages_,
			static_cast<PhysicalFile::AreaID>(
				Common::ObjectIDData::getLatterValue(keyObjectID)));

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 false); // バリューオブジェクトの
								         // オブジェクトIDを取得する
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage = this->searchLeafPageForSingleCondition(treeDepth,
														  rootNodePageID,
														  AttachNodePages_,
														  Target_);

		if (leafPage == 0)
		{
			return resultObjectID;
		}

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				AttachNodePages_,
				true, // リーフページ
				match,
				Target_);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true); // リーフページ

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		resultObjectID = keyInfo.readValueObjectID();

		if (this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID,
											  Target_)
			< 0)
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// キー値順で前だった…

			// つまり、検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				int	compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID,
													  Target_);

				if (compareResult > 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
		else
		{
			if (ContainEquals_ == false)
			{
				bool	lp = true;
				do
				{
					if (this->assignNextKeyInformation(leafPage,
													   AttachNodePages_,
													   leafPageHeader,
													   keyInfo)
						== false)
					{
						// なかった…

						checkMemoryExhaust(leafPage);

						return FileCommon::ObjectID::Undefined;
					}

					keyObjectID = keyInfo.readKeyObjectID();

					; _SYDNEY_ASSERT(
						keyObjectID != FileCommon::ObjectID::Undefined &&
						keyObjectID != 0);

					if (this->compareToTopSearchCondition(leafPage,
														  AttachNodePages_,
														  keyObjectID,
														  Target_)
						< 0)
					{
						resultObjectID = keyInfo.readValueObjectID();

						lp = false;

						if (KeyInfoLeafPageID_ != 0)
						{
							// File::searchBySpanConditionから呼ばれた…

							// ならば、
							// リーフページのキーオブジェクトに関する情報を
							// 設定する必要がある。

							; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
											 KeyObjectID_ != 0);

							*KeyInfoLeafPageID_ = leafPage->getID();

							*KeyInfoIndex_ = keyInfo.getIndex();

							*KeyObjectID_ = keyObjectID;
						}
					}
				}
				while (lp);
			}
			else
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}
			}
		}
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleGreaterThan --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool				ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//	PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーオブジェクトへ辿れる
//		キー情報が記録されているリーフページの
//		物理ページ識別子へのポインタ
//		※ File::searchBySpanCondition()のための引数であり、
//		　 本関数がポインタが指している領域に物理ページ識別子を設定する。
//	ModUInt32*				KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//	ModUInt64*				KeyObjectID_ = 0
//		同、キーオブジェクトのオブジェクトIDへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleGreaterThan(
	FileInformation&		FileInfo_,
	PageVector&				AttachNodePages_,
	const bool				ContainEquals_,
	PhysicalFile::PageID*	KeyInfoLeafPageID_, // = 0
	ModUInt32*				KeyInfoIndex_,      // = 0
	ModUInt64*				KeyObjectID_        // = 0
	) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		; _SYDNEY_ASSERT(KeyObjectID_ == 0);

		return this->searchBySingleGreaterThanSimpleKey(FileInfo_,
														AttachNodePages_,
														ContainEquals_,
														KeyInfoLeafPageID_,
														KeyInfoIndex_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage = this->searchLeafPageForSingleCondition(treeDepth,
														  rootNodePageID,
														  AttachNodePages_);

		if (leafPage == 0)
		{
			return resultObjectID;
		}

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				AttachNodePages_,
				true, // リーフページ
				match);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true); // リーフページ

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		resultObjectID = keyInfo.readValueObjectID();

		if (this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID)
			< 0)
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// キー値順で前だった…

			// つまり、検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 小さかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				int	compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID);

				if (compareResult > 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
		else
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…
			// または、
			// 検索条件と
			// 記録されているキーオブジェクトが
			// 等しかった…

			if (ContainEquals_ == false)
			{
				bool	lp = true;
				do
				{
					if (this->assignNextKeyInformation(leafPage,
													   AttachNodePages_,
													   leafPageHeader,
													   keyInfo)
						== false)
					{
						// なかった…

						checkMemoryExhaust(leafPage);

						return FileCommon::ObjectID::Undefined;
					}

					keyObjectID = keyInfo.readKeyObjectID();

					; _SYDNEY_ASSERT(
						keyObjectID != FileCommon::ObjectID::Undefined &&
						keyObjectID != 0);

					if (this->compareToTopSearchCondition(leafPage,
														  AttachNodePages_,
														  keyObjectID)
						< 0)
					{
						resultObjectID = keyInfo.readValueObjectID();

						lp = false;

						if (KeyInfoLeafPageID_ != 0)
						{
							// File::searchBySpanConditionから呼ばれた…

							// ならば、
							// リーフページのキーオブジェクトに関する情報を
							// 設定する必要がある

							; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
											 KeyObjectID_ != 0);

							*KeyInfoLeafPageID_ = leafPage->getID();

							*KeyInfoIndex_ = keyInfo.getIndex();

							*KeyObjectID_ = keyObjectID;
						}
					}
				}
				while (lp);
			}
			else
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}
			}
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// 「 key1 > 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に先頭オブジェクト”となる。

		// でも、“キー値順に先頭オブジェクト”の
		// 先頭キーフィールドの値に、例えば、50という値が
		// 記録されていたとして、
		// 検索条件が「 key1 > 100 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に先頭オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に先頭オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleGreaterThanは、
		// File::searchBySpanConditionからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が昇順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0 &&
						 KeyObjectID_ == 0);

		keyObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 true); // キーオブジェクトの
								        // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		int	compareResult = compareToTopSearchCondition(
			leafPage,
			AttachNodePages_,
			static_cast<PhysicalFile::AreaID>(
				Common::ObjectIDData::getLatterValue(keyObjectID)));

		if (compareResult < 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 false); // バリューオブジェクトの
								         // オブジェクトIDを取得する
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsToNull --
//		先頭キーフィールドの値がヌル値のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値がヌル値のオブジェクトを検索する
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsToNull(FileInformation&	FileInfo_,
								 PageVector&		AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchBySingleEqualsToNullSimpleKey(FileInfo_,
														 AttachNodePages_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// “キー値順に先頭オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならばそのオブジェクトを返せばよいし、
		// そうでなければ検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		keyObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 true); // キーオブジェクトの
								        // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		// ※ getTopFieldPointerは、
		// 　 先頭キーフィールドの値として
		// 　 ヌル値が設定されている場合には、
		// 　 ヌルポインタ(0)を返す。
		// 　 ちなみに、ヌル値が設定されているかどうかは
		// 　 記録されているフィールドタイプの違いにより
		// 　 判断可能である。

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 先頭オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 false); // バリューオブジェクトの
								         // オブジェクトIDを取得する
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// “キー値順に最終オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならば最終オブジェクトから順に
		// キー値順に先頭オブジェクト方向にオブジェクトを
		// スキャンしていき、最も先頭オブジェクトに近い
		// 先頭キーフィールドにヌル値が設定されているオブジェクトを
		// 見つけ出し、そのオブジェクトを返せばよい。

		keyObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  true); // キーオブジェクトの
								         // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドにヌル値が
			// 設定されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  false); // バリューオブジェクトの
								          // オブジェクトIDを取得する

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		keyInfoLeafPageID = FileInfo_.readLastLeafPageID();

		if (leafPage->getID() != keyInfoLeafPageID)
		{
			checkMemoryExhaust(leafPage);

			leafPage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										keyInfoLeafPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);
		}

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		const ModUInt32 useKeyInfoNum =
			leafPageHeader.readUseKeyInformationNumber();
		; _SYDNEY_ASSERT(useKeyInfoNum > 0);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								useKeyInfoNum - 1,
								true); // リーフページ

		bool	lp = true;
		do
		{
			ModUInt64	saveObjectID = resultObjectID;

			if (this->assignPrevKeyInformation(leafPage,
											   AttachNodePages_,
											   leafPageHeader,
											   keyInfo)
				== false)
			{
				resultObjectID = saveObjectID;

				break;
			}

			keyObjectID = keyInfo.readKeyObjectID();

			; _SYDNEY_ASSERT(
				keyObjectID != FileCommon::ObjectID::Undefined &&
				keyObjectID != 0);

			PhysicalFile::Page*	keyObjectPage = 0;

			bool	attached =
				File::attachObjectPage(this->m_pTransaction,
									   keyObjectID,
									   leafPage,
									   keyObjectPage,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

			if (getTopKeyPointer(
					keyObjectPage,
					Common::ObjectIDData::getLatterValue(keyObjectID))) {
				lp = false;
			}
			else
			{
				resultObjectID = keyInfo.readValueObjectID();
			}

			if (attached)
			{
				checkMemoryExhaust(keyObjectPage);
			}
		}
		while (lp);
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySpanCondition --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件で指定された範囲内のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySpanCondition(FileInformation&	FileInfo_,
							PageVector&			AttachNodePages_) const
{
	//
	// 範囲指定ならば、
	// 検索開始条件の比較演算子がGreaterThanかGreaterThanEqualsで
	// 検索終了条件の比較演算子がLessThanかLessThanEqualsのはず。
	// 

	; _SYDNEY_ASSERT(
		*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::GreaterThan ||
		*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::GreaterThanEquals);

	; _SYDNEY_ASSERT(
		this->m_SearchHint.m_StopOperator ==
		LogicalFile::TreeNodeInterface::LessThan ||
		this->m_SearchHint.m_StopOperator ==
		LogicalFile::TreeNodeInterface::LessThanEquals);

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchBySpanConditionSimpleKey(FileInfo_,
													AttachNodePages_);
	}

	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		//
		// まずは、検索開始条件と一致するオブジェクトを検索する。
		//

		bool	containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

		resultObjectID = this->searchBySingleGreaterThan(FileInfo_,
														 AttachNodePages_,
														 containEquals,
														 &keyInfoLeafPageID,
														 &keyInfoIndex,
														 &keyObjectID);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索開始条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		//
		// 検索終了条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		int	compareResult =
			this->compareToTopSearchCondition(
				keyInfoLeafPage,
				AttachNodePages_,
				keyObjectID,
				SearchHint::CompareTarget::Stop);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult < 0)
		{
			// 検索終了条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			this->m_SearchHint.m_StopOperator ==
			LogicalFile::TreeNodeInterface::LessThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		//
		// まずは、検索終了条件と一致するオブジェクトを検索する
		//

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

		resultObjectID =
			this->searchBySingleLessThan(FileInfo_,
										 AttachNodePages_,
										 containEquals,
										 SearchHint::CompareTarget::Stop,
										 &keyInfoLeafPageID,
										 &keyInfoIndex,
										 &keyObjectID);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索終了条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		//
		// 検索開始条件とも一致するかチェックする
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		int	compareResult =
			this->compareToTopSearchCondition(
				keyInfoLeafPage,
				AttachNodePages_,
				keyObjectID,
				SearchHint::CompareTarget::Start);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult < 0)
		{
			// 検索開始条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::GreaterThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchByMultiCondition --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		オブジェクトを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByMultiCondition(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchByMultiConditionSimpleKey(TreeDepth_,
													 RootNodePageID_,
													 AttachNodePages_);
	}

	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForMultiCondition(TreeDepth_,
											  RootNodePageID_,
											  AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForMultiCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		if (this->compareToLastCondition(keyObjectID,
										 leafPage,
										 AttachNodePages_)
			== false)
		{
			resultObjectID = FileCommon::ObjectID::Undefined;

			NodePageHeader	leafPageHeader(this->m_pTransaction,
										   leafPage,
										   true); // リーフページ

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
//	Btree::File::compareToLastCondition --
//		最終検索対象キーフィールドの値と検索条件を比較する
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件を比較する。
//
//	ARGUMENTS
//	const ModUInt64		KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		比較結果
//			true  : 最終検索対象キーフィールドの値が検索条件と一致する
//			false : 最終検索対象キーフィールドの値が検索条件と一致しない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastCondition(const ModUInt64		KeyObjectID_,
							 PhysicalFile::Page*	KeyInfoPage_,
							 PageVector&			AttachNodePages_) const
{
	bool	match = false;

	LogicalFile::TreeNodeInterface::Type	lastCompOpe =
		*(this->m_SearchHint.m_StartOperatorArray +
		  (this->m_SearchHint.m_FieldNumber - 1));

	if (lastCompOpe == LogicalFile::TreeNodeInterface::Equals)
	{
		// 比較演算子がEquals…

		match = this->compareToLastEquals(KeyObjectID_,
										  KeyInfoPage_,
										  AttachNodePages_);
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::LessThan)
	{
		// 比較演算子がLessThan…

		match = this->compareToLastLessThan(KeyObjectID_,
											KeyInfoPage_,
											AttachNodePages_,
											false); // Equals含まず
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		// 比較演算子がLessThanEquals…

		match = this->compareToLastLessThan(KeyObjectID_,
											KeyInfoPage_,
											AttachNodePages_,
											true); // Equals含む
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::GreaterThan)
	{
		// 比較演算子がGreaterThan…

		match = this->compareToLastGreaterThan(KeyObjectID_,
											   KeyInfoPage_,
											   AttachNodePages_,
											   false); // Equals含まず
	}
	else if (lastCompOpe == LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		// 比較演算子がGreaterThanEquals…

		match = this->compareToLastGreaterThan(KeyObjectID_,
											   KeyInfoPage_,
											   AttachNodePages_,
											   true); // Equals含む
	}
	else
	{
		// 比較演算子がEqualsToNull…
		// （のはず。）

		; _SYDNEY_ASSERT(
			lastCompOpe == LogicalFile::TreeNodeInterface::EqualsToNull);

		match = this->compareToLastEqualsToNull(KeyObjectID_,
												KeyInfoPage_,
												AttachNodePages_);
	}

	return match;
}

//
//	FUNCTION private
//	Btree::File::compareToLastEquals --
//		最終検索対象キーフィールドの値と検索条件が一致するかどうかを調べる
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件が一致するかどうかを調べる。
//
//	ARGUMENTS
//	const ModUInt64		KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値と検索条件が一致するかどうか
//			true  : 一致する
//			false : 一致しない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastEquals(const ModUInt64		KeyObjectID_,
						  PhysicalFile::Page*	KeyInfoPage_,
						  PageVector&			AttachNodePages_) const
{
	return (this->compareToLastSearchCondition(KeyObjectID_,
											   KeyInfoPage_,
											   AttachNodePages_)
			== 0);
}

//
//	FUNCTION private
//	Btree::File::compareToLastLessThan --
//		最終検索対象キーフィールドの値が検索条件以下（または未満）かどうかを
//		調べる
//
//	NOTES
//	最終検索対象キーフィールドの値が、そのキーフィールドに対する
//	検索条件以下（または未満）かどうかを調べる。
//
//	ARGUMENTS
//	const ModUInt64		KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値が検索条件以下（または未満）かどうか
//			true  : 検索条件以下（または未満）
//			false : 検索条件超（または以上）
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastLessThan(const ModUInt64		KeyObjectID_,
							PhysicalFile::Page*	KeyInfoPage_,
							PageVector&			AttachNodePages_,
							const bool			ContainEquals_) const
{
	int	compareResult =
		this->compareToLastSearchCondition(KeyObjectID_,
										   KeyInfoPage_,
										   AttachNodePages_);

	int	arrayIndex = this->m_SearchHint.m_FieldNumber - 1;

	int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + arrayIndex);

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + fieldIndex) ==
		FileParameter::SortOrder::Ascending)
	{
		return ContainEquals_ ? (compareResult >= 0) : (compareResult > 0);
	}
	else
	{
		return ContainEquals_ ? (compareResult <= 0) : (compareResult < 0);
	}
}

//
//	FUNCTION private
//	Btree::File::compareToLastGreaterThan --
//		最終検索対象キーフィールドの値が検索条件以上（または超）かどうかを
//		調べる
//
//	NOTES
//	最終検索対象キーフィールドの値が、そのキーフィールドに対する
//	検索条件以上（または超）かどうかを調べる。
//
//	ARGUMENTS
//	const ModUInt64		KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool			ContainEquals_
//		比較演算子がGreaterThanとGreatenEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値が検索条件以上（または超）かどうか
//			true  : 検索条件以上（または超）
//			false : 検索条件未満（または以下）
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastGreaterThan(const ModUInt64		KeyObjectID_,
							   PhysicalFile::Page*	KeyInfoPage_,
							   PageVector&			AttachNodePages_,
							   const bool			ContainEquals_) const
{
	int	compareResult =
		this->compareToLastSearchCondition(KeyObjectID_,
										   KeyInfoPage_,
										   AttachNodePages_);

	bool	match = false;

	int	arrayIndex = this->m_SearchHint.m_FieldNumber - 1;

	int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + arrayIndex);

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + fieldIndex) ==
		FileParameter::SortOrder::Ascending)
	{
		match = ContainEquals_ ? (compareResult <= 0) : (compareResult < 0);
	}
	else
	{
		match = ContainEquals_ ? (compareResult >= 0) : (compareResult > 0);
	}

	if (match == false)
	{
		return false;
	}

	if (this->m_SearchHint.m_StopOperator !=
		LogicalFile::TreeNodeInterface::Undefined)
	{
		// 範囲指定…

		; _SYDNEY_ASSERT(this->m_SearchHint.m_StopOperator ==
						 LogicalFile::TreeNodeInterface::LessThan ||
						 this->m_SearchHint.m_StopOperator ==
						 LogicalFile::TreeNodeInterface::LessThanEquals);

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		compareResult =
			this->compareToLastSearchCondition(
				KeyObjectID_,
				KeyInfoPage_,
				AttachNodePages_,
				SearchHint::CompareTarget::Stop);

		if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray +
			  fieldIndex) ==
			FileParameter::SortOrder::Ascending)
		{
			match =
				containEquals ? (compareResult >= 0) : (compareResult > 0);
		}
		else
		{
			match =
				containEquals ? (compareResult <= 0) : (compareResult < 0);
		}
	}

	return match;
}

//
//	FUNCTION private
//	Btree::File::compareToLastEqualsToNull --
//		最終検索対象キーフィールドの値がヌル値かどうかを調べる
//
//	NOTES
//	最終検索対象キーフィールドの値がヌル値かどうかを調べる。
//
//	ARGUMENTS
//	const ModUInt64		KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*	KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		最終検索対象キーフィールドの値がヌル値かどうか
//			true  : ヌル値
//			false : ヌル値ではない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::compareToLastEqualsToNull(
	const ModUInt64				KeyObjectID_,
	PhysicalFile::Page*			KeyInfoPage_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  KeyObjectID_,
											  KeyInfoPage_,
											  keyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	int	fieldIndex =
		*(this->m_SearchHint.m_FieldIndexArray +
		  (this->m_SearchHint.m_FieldNumber - 1));

	bool compareResult = (getFieldPointer(
		keyObjectPage,
		Common::ObjectIDData::getLatterValue(KeyObjectID_),
		fieldIndex,
		true) // キーオブジェクト
		 == 0);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToLastSearchCondition --
//		最終検索対象キーフィールドの値と検索条件を比較する
//
//	NOTES
//	最終検索対象キーフィールドの値と、そのキーフィールドに対する
//	検索条件を比較する。
//
//	ARGUMENTS
//	const ModUInt64									KeyObjectID_
//		キーオブジェクトのID
//	PhysicalFile::Page*								KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToLastSearchCondition(
	const ModUInt64							KeyObjectID_,
	PhysicalFile::Page*						KeyInfoPage_,
	PageVector&								AttachNodePages_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  KeyObjectID_,
											  KeyInfoPage_,
											  keyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	int	compareResult = compareToMultiSearchCondition(
		keyObjectPage, AttachNodePages_,
		Common::ObjectIDData::getLatterValue(KeyObjectID_),
		SearchHint::CompareType::OnlyLastKey, Target_);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForSingleCondition --
//		先頭キーフィールドへの検索条件と一致するキーオブジェクトが
//		記録されている可能性のあるリーフページを検索する
//
//	NOTES
//	先頭キーフィールドへの検索条件と一致するキーオブジェクトを
//	記録している可能性があるリーフページを検索する。
//	ただし、返すのは実際にキーオブジェクトが記録されている
//	物理ページの記述子ではなく、そのキーオブジェクトへ辿ることができる
//	キー情報が記録されている物理ページの記述子である。
//
//	ARGUMENTS
//	const ModUInt32									TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID						RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::Page*
//		先頭キーフィールドへの検索条件と一致するキーオブジェクトが
//		記録されている可能性のあるリーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchLeafPageForSingleCondition(
	const ModUInt32							TreeDepth_,
	const PhysicalFile::PageID				RootNodePageID_,
	PageVector&								AttachNodePages_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	if (TreeDepth_ == 1)
	{
		return
			this->containTargetKeyObjectForSingleCondition(
				RootNodePageID_,
				AttachNodePages_,
				true, // リーフページ
				Target_);
	}

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32	depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID =
			this->searchChildNodePageForSingleCondition(nodePageID,
														AttachNodePages_,
														Target_);

		if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			return 0;
		}
	}

	return File::attachPage(this->m_pTransaction,
							this->m_pPhysicalFile,
							nodePageID,
							this->m_FixMode,
							this->m_CatchMemoryExhaust,
							AttachNodePages_);
}

//
//	FUNCTION private
//	Btree::File::containTargetKeyObjectForSingleCondition --
//		検索条件と一致するキーオブジェクトがノードページ内に
//		存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	検索条件と一致する先頭キーフィールドの値が書かれているキーオブジェクトが
//	存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID						NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool										IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::Page*
//		検索条件と一致する先頭キーフィールドの値が書かれている
//		キーオブジェクトがノードページ内に存在する可能性があれば、
//		そのノードページの物理ページ記述子、存在する可能性がなければ、
//		0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetKeyObjectForSingleCondition(
	const PhysicalFile::PageID				NodePageID_,
	PageVector&								AttachNodePages_,
	const bool								IsLeafPage_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*	nodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 NodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(nodePage != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   nodePage,
								   IsLeafPage_);

	const ModUInt32 useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	bool	exist = true;

	if (useKeyInfoNum > 0)
	{
		KeyInformation	keyInfo(this->m_pTransaction,
								nodePage,
								useKeyInfoNum - 1,
								IsLeafPage_);

		ModUInt64	lastKeyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			lastKeyObjectID != FileCommon::ObjectID::Undefined &&
			lastKeyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(nodePage,
											  AttachNodePages_,
											  lastKeyObjectID,
											  Target_);

		if (compareResult > 0)
		{
			exist = false;
		}
	}
	else
	{
		exist = false;
	}

	if (exist == false)
	{
		checkMemoryExhaust(nodePage);

		nodePage = 0;
	}

	return nodePage;
}

//
//	FUNCTION private
//	Btree::File::searchChildNodePageForSingleCondition --
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		子ノードページを検索する
//
//	NOTES
//	検索条件と一致する先頭キーフィールドの値が書かれている
//	キーオブジェクトへ辿ることのできるキー情報が記録されている可能性のある
//	子ノードページを検索する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID						ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const Btree::SearchHint::CompareTarget::Value	Target_
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	PhysicalFile::PageID
//		検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
//		子ノードページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildNodePageForSingleCondition(
	const PhysicalFile::PageID				ParentNodePageID_,
	PageVector&								AttachNodePages_,
	const SearchHint::CompareTarget::Value	Target_) const
{
	PhysicalFile::Page*	parentNodePage =
		this->containTargetKeyObjectForSingleCondition(
			ParentNodePageID_,
			AttachNodePages_,
			false, // リーフページではない
			Target_);

	if (parentNodePage == 0)
	{
		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	bool	match = false; // dummy

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getKeyInformationIndexForSingleCondition(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			false, // リーフページではない
			match,
			Target_);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	PhysicalFile::PageID	childNodePageID = keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForSingleCondition --
//		検索条件に最も近い先頭キーフィールドの値が書かれている
//		キーオブジェクトへ辿ることができるキー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	検索条件に最も近い先頭キーフィールドの値が書かれている
//	キーオブジェクトへ辿ることができるキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32									UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool										IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&											Match_
//		そのキー情報に書かれている先頭キーフィールドの値が、
//		検索条件と完全に一致しているかどうか
//			true  : 完全に一致している
//			false : 完全に一致していない
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getKeyInformationIndexForSingleCondition(
	PhysicalFile::Page*						KeyInfoPage_,
	const ModUInt32							UseKeyInfoNum_,
	PageVector&								AttachNodePages_,
	const bool								IsLeafPage_,
	bool&									Match_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	; _SYDNEY_ASSERT(UseKeyInfoNum_ > 0);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	int	keyInfoIndex = -1;

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							0,
							IsLeafPage_);

	Match_ = false;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToTopSearchCondition(KeyInfoPage_,
											  AttachNodePages_,
											  keyObjectID,
											  Target_);

		if (compareResult < 0)
		{
			lastKeyInfoIndex = midKeyInfoIndex - 1;
		}
		else if (compareResult > 0)
		{
			firstKeyInfoIndex = midKeyInfoIndex + 1;
		}
		else
		{
			Match_ = true;

			keyInfoIndex = midKeyInfoIndex;

			int	addNum;

			if (this->m_pOpenParameter->m_bSortReverse)
			{
				if (keyInfoIndex == static_cast<int>(UseKeyInfoNum_) - 1)
				{
					break;
				}

				addNum = 1;
			}
			else
			{
				if (keyInfoIndex == 0)
				{
					break;
				}

				addNum = -1;
			}

			while (true)
			{
				keyInfoIndex += addNum;

				keyInfo.setStartOffsetByIndex(keyInfoIndex);

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				if (this->compareToTopSearchCondition(KeyInfoPage_,
													  AttachNodePages_,
													  keyObjectID,
													  Target_)
					!= 0)
				{
					keyInfoIndex -= addNum;

					break;
				}

				if (this->m_pOpenParameter->m_bSortReverse)
				{
					if (keyInfoIndex ==
						static_cast<int>(UseKeyInfoNum_ - 1))
					{
						break;
					}
				}
				else
				{
					if (keyInfoIndex == 0)
					{
						break;
					}
				}

			} // end while

			break;
		}
	}

	if (keyInfoIndex == -1)
	{
		keyInfoIndex = firstKeyInfoIndex;
	}

	return keyInfoIndex;
}

//
//	FUNCTION private
//	Btree::File::compareToTopSearchCondition --
//		検索条件と先頭キーフィールドの値を比較する
//
//	NOTES
//	検索条件と先頭キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyInfoPage_
//		比較対象であるキーオブジェクトへ辿ることのできる
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	ModUInt64										KeyObjectID_
//		比較対象であるキーオブジェクトのID
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToTopSearchCondition(
	PhysicalFile::Page*						KeyInfoPage_,
	PageVector&								AttachNodePages_,
	ModUInt64								KeyObjectID_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*		keyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  KeyObjectID_,
											  KeyInfoPage_,
											  keyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	int	compareResult = compareToTopSearchCondition(
		keyObjectPage, AttachNodePages_,
		static_cast<PhysicalFile::AreaID>(
			Common::ObjectIDData::getLatterValue(KeyObjectID_)),
		Target_);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToTopSearchCondition --
//		検索条件と先頭キーフィールドの値を比較する
//
//	NOTES
//	検索条件と先頭キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyObjectPage_
//		キーオブジェクトが記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const PhysicalFile::AreaID						KeyObjectAreaID_
//		キーオブジェクトが記録されている物理エリアの識別子
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToTopSearchCondition(
	PhysicalFile::Page*						KeyObjectPage_,
	PageVector&								AttachNodePages_,
	const PhysicalFile::AreaID				KeyObjectAreaID_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	; _SYDNEY_ASSERT(KeyObjectPage_ != 0);

	// m_SearchHint.m_FieldTypeArray[0]を取る
	Common::DataType::Type	fieldType =
		*this->m_SearchHint.m_FieldTypeArray;

	// m_SearchHint.m_IsFixedArray[0]を見る
	if (*this->m_SearchHint.m_IsFixedArray)
	{
		// 先頭キーフィールドが固定長フィールド…

		// ※ getTopFieldPointerは、
		// 　 先頭キーフィールドの値として
		// 　 ヌル値が設定されている場合には、
		// 　 ヌルポインタ(0)を返す。
		// 　 ちなみに、ヌル値が設定されているかどうかは
		// 　 記録されているフィールドタイプの違いにより
		// 　 判断可能である。

		void*	fieldValue = this->getTopKeyPointer(KeyObjectPage_,
													KeyObjectAreaID_);

		if (fieldValue == 0)
		{
			// 先頭キーフィールドの値として
			// ヌル値が設定されていた…

			// ヌル値はいかなる検索条件よりも
			// 小さいと見なす。

			return 1 * *this->m_SearchHint.m_MultiNumberArray;
		}

		if (Target_ == SearchHint::CompareTarget::Start)
		{
			return
				this->m_SearchHint.compareToFixedStartCondition(
					fieldValue,
					0);
		}
		else
		{
			return
				this->m_SearchHint.compareToFixedStopCondition(
					fieldValue);
		}
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::EqualsToNull)
	{
		if (this->getTopKeyPointer(KeyObjectPage_,
								   KeyObjectAreaID_)
			!= 0)
		{
			return -1 * *this->m_SearchHint.m_MultiNumberArray;
		}
	}
	else
	{
		// variable field

		// キーフィールドにできる可変長データは、
		// Stringだけ！
		; _SYDNEY_ASSERT(fieldType == Common::DataType::String);

		return this->compareToTopStringSearchCondition(KeyObjectPage_,
													   AttachNodePages_,
													   KeyObjectAreaID_,
													   Target_);
	}

	return 0;
}

//
//	FUNCTION private
//	Btree::File::compareToTopStringSearchCondition --
//		検索条件と先頭文字列キーフィールドの値を比較する
//
//	NOTES
//	検索条件と先頭文字列キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyObjectPage_,
//		キーオブジェクトが記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_,
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const PhysicalFile::AreaID						KeyObjectAreaID_,
//		キーオブジェクトが記録されている物理エリアの識別子
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToTopStringSearchCondition(
	PhysicalFile::Page*						KeyObjectPage_,
	PageVector&								AttachNodePages_,
	const PhysicalFile::AreaID				KeyObjectAreaID_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*	outsideObjectPage = 0;
	ModSize	numChar = 0;
	bool	isDivide;
	bool	isCompressed;

	ModUnicodeChar*	fieldValue =
		this->getTopStringFieldPointer(KeyObjectPage_,
									   outsideObjectPage,
									   AttachNodePages_,
									   KeyObjectAreaID_,
									   numChar,
									   isDivide,
									   isCompressed);

	checkMemoryExhaust(outsideObjectPage);

	if (fieldValue == 0)
	{
		// 先頭文字列キーフィールドの値として
		// ヌル値が設定されていた…

		// ヌル値はいかなる検索条件よりも
		// 小さいと見なす。

		return 1 * *this->m_SearchHint.m_MultiNumberArray;
	}

	ModUnicodeChar	forZeroByte = 0;
	if (numChar == 0)
	{
		fieldValue = &forZeroByte;
	}

	const Common::Data*	condition = 0;

	if (Target_ == SearchHint::CompareTarget::Start)
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStartArray.
				getElement(0).get();
	}
	else
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStopArray.
				getElement(0).get();
	}

	; _SYDNEY_ASSERT(condition != 0);

	; _SYDNEY_ASSERT(
		condition->getType() == Common::DataType::String);

	const Common::StringData*	stringCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*, condition);

	; _SYDNEY_ASSERT(stringCondition != 0);

	const ModUnicodeString&	unicodeCondition =
		stringCondition->getValue();

	if (isDivide)
	{
		// divide...

		char*	objectIDReadPos =
			static_cast<char*>(
				this->getFieldPointer(KeyObjectPage_,
									  KeyObjectAreaID_,
									  1,	  // field index
									  true)); // key object

		ModUInt64	objectID;
		File::readObjectID(objectIDReadPos, objectID);

		ModUnicodeString	unicodeFieldValue;

		File::readOutsideStringField(this->m_pTransaction,
									 KeyObjectPage_,
									 objectID,
									 unicodeFieldValue,
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 AttachNodePages_);

		return
			unicodeCondition.compare(unicodeFieldValue) *
			*this->m_SearchHint.m_MultiNumberArray;
	}
	else if (isCompressed)
	{
		char*	objectIDReadPos =
			static_cast<char*>(
				this->getFieldPointer(KeyObjectPage_,
									  KeyObjectAreaID_,
									  1,      // field index
									  true)); // key object

		ModUInt64	objectID;
		File::readObjectID(objectIDReadPos, objectID);

		Common::Data::Pointer	data;

		File::readOutsideVariableField(this->m_pTransaction,
									   KeyObjectPage_,
									   objectID,
									   Common::DataType::String,
									   data,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

		return
			stringCondition->compareTo(data.get()) *
			*this->m_SearchHint.m_MultiNumberArray;
	}
	else
	{
		// normal...

		if (numChar == unicodeCondition.getLength())
		{
			return
				unicodeCondition.compare(fieldValue, numChar) *
				*this->m_SearchHint.m_MultiNumberArray;
		}
		else
		{
			if (*(this->m_cFileParameter.m_FieldOutsideArray + 1))
			{
				// 外置き…

				ModUnicodeString	unicodeFieldValue(fieldValue, numChar);

				return
					unicodeCondition.compare(unicodeFieldValue) *
					*this->m_SearchHint.m_MultiNumberArray;
			}
			else
			{
				// 外置きではない…

				//
				// 外置きでないのなら、短い文字列なので、
				// ModUnicodeStringをいちいちコンストラクトするよりも
				// 違い領域にコピーして、比較した方が早いはず。
				// 領域は64文字分もあれば十分。
				//

				; _SYDNEY_ASSERT(numChar < 64);

				ModUnicodeChar	fieldBuffer[64 + 1];

				ModUnicodeCharTrait::copy(fieldBuffer, fieldValue, numChar);

				fieldBuffer[numChar] = 0;

				return
					unicodeCondition.compare(fieldBuffer) *
					*this->m_SearchHint.m_MultiNumberArray;
			}
		}
	}
}

//
//	FUNCTION private
//	Btree::File::getTopStringFieldPointer --
//		先頭文字列キーフィールドの値が記録されている位置へのポインタを返す
//
//	NOTES
//	先頭文字列キーフィールドの値が記録されている位置へのポインタを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			KeyObjectPage_
//		キーオブジェクトが記録されているノードページの物理ページ記述子
//	PhysicalFile::Page*&		OutsideObjectPage_
//		外置きオブジェクトが記録されているノードページの物理ページ記述子
//		（この関数内でアタッチする）
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const PhysicalFile::AreaID	KeyObjectAreaID_
//		キーオブジェクトが記録されている物理エリアの識別子
//	ModSize&					NumChar_
//		文字数
//		（この関数が設定）
//	bool&						isDivide_
//		文字列が分割されて記録されているかどうか
//			true  : 文字列が分割されている
//			false : 文字列が分割されていない
//	bool&						IsCompressed_
//		圧縮された文字列が記録されているかどうか
//			true  : 圧縮されている記録されている
//			false : 圧縮されていない状態で記録されている
//
//	RETURN
//	ModUnicodeChar*
//		文字列先頭へのポインタ
//		※ ただし、外置きの場合には、
//		　 外置きオブジェクトのIDへのポインタとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUnicodeChar*
File::getTopStringFieldPointer(
	PhysicalFile::Page*			KeyObjectPage_,
	PhysicalFile::Page*&		OutsideObjectPage_,
	PageVector&					AttachNodePages_,
	const PhysicalFile::AreaID	KeyObjectAreaID_,
	ModSize&					NumChar_,
	bool&						IsDivide_,
	bool&						IsCompressed_) const
{
	const void*	keyObjectAreaTop =
		File::getConstAreaTop(KeyObjectPage_,
							  KeyObjectAreaID_);

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(keyObjectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	IsDivide_ = false;
	IsCompressed_ = false;

	const NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(objectType + sizeof(File::ObjectType));

	if (NullBitmap::isNull(nullBitmapTop,
						   this->m_cFileParameter.m_KeyNum,
						   0))
	{
		return 0;
	}

	const char*	keyTop =
		static_cast<const char*>(
			NullBitmap::getConstTail(nullBitmapTop,
									 this->m_cFileParameter.m_KeyNum));		

	if (*(this->m_cFileParameter.m_FieldOutsideArray + 1))
	{
		// outside

		ModUInt64	objectID;
		File::readObjectID(keyTop, objectID);

		PhysicalFile::Page*	page = 0;

		const PhysicalFile::PageID objectPageID =
			Common::ObjectIDData::getFormerValue(objectID);

		if (objectPageID == KeyObjectPage_->getID())

			// same page...

			page = KeyObjectPage_;
		else {

			// other page...

			page = File::attachPage(
				m_pTransaction, m_pPhysicalFile, objectPageID,
				m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

			OutsideObjectPage_ = page;
		}

		; _SYDNEY_ASSERT(page != 0);

		const PhysicalFile::AreaID objectAreaID =
			Common::ObjectIDData::getLatterValue(objectID);

		keyObjectAreaTop = File::getConstAreaTop(page, objectAreaID);

		objectType =
			static_cast<const File::ObjectType*>(keyObjectAreaTop);

		// check object type
		if (*objectType == File::NormalObjectType)
		{
			Os::Memory::Size	fieldLen =
				page->getAreaSize(objectAreaID) -
				File::ObjectTypeArchiveSize;

			NumChar_ = fieldLen / sizeof(ModUnicodeChar);
			//                  ~~~~~~~~~~~~~~~~~~~~~~~~~
			//                  length to number of chars

			return
				syd_reinterpret_cast<ModUnicodeChar*>(
					const_cast<File::ObjectType*>(
						objectType + sizeof(File::ObjectType)));
		}
		else if (*objectType == File::CompressedObjectType)
		{
			IsCompressed_ = true;

			const Os::Memory::Size*	uncompressedSizeReadPos =
				syd_reinterpret_cast<const Os::Memory::Size*>(objectType + sizeof(File::ObjectType));

			NumChar_ = *uncompressedSizeReadPos / sizeof(ModUnicodeChar);

			return
				syd_reinterpret_cast<ModUnicodeChar*>(
					const_cast<Os::Memory::Size*>(
						uncompressedSizeReadPos + 2));
		}
		else if (*objectType == File::DivideCompressedObjectType)
		{
			IsCompressed_ = true;

			const char*	nextLinkObjectIDReadPos =
				syd_reinterpret_cast<const char*>(objectType + sizeof(File::ObjectType));

			const Os::Memory::Size*	uncompressedSizeReadPos =
				syd_reinterpret_cast<const Os::Memory::Size*>(
					nextLinkObjectIDReadPos + File::ObjectIDArchiveSize);

			NumChar_ = *uncompressedSizeReadPos / sizeof(ModUnicodeChar);

			return
				syd_reinterpret_cast<ModUnicodeChar*>(
					const_cast<Os::Memory::Size*>(
						uncompressedSizeReadPos + 2));
		}
		else
		{
			IsDivide_ = true;

			const char*	nextLinkObjectIDReadPos =
				syd_reinterpret_cast<const char*>(objectType + sizeof(File::ObjectType));

			ModUInt64	nextLinkObjectID;
			File::readObjectID(nextLinkObjectIDReadPos,
							   nextLinkObjectID);

			Os::Memory::Size	fieldLen =
				page->getAreaSize(objectAreaID) -
				File::ObjectTypeArchiveSize -
				File::ObjectIDArchiveSize;

			NumChar_ = fieldLen / sizeof(ModUnicodeChar);

			return
				syd_reinterpret_cast<ModUnicodeChar*>(
					const_cast<char*>(
						nextLinkObjectIDReadPos +
						File::ObjectIDArchiveSize));
		}
	}
	else
	{
		// inside

		const File::InsideVarFieldLen*	fieldLen =
			syd_reinterpret_cast<const File::InsideVarFieldLen*>(keyTop);

		NumChar_ = *fieldLen / sizeof(ModUnicodeChar);

		return
			syd_reinterpret_cast<ModUnicodeChar*>(
				const_cast<File::InsideVarFieldLen*>(
					fieldLen + 1));
	}
}

//
//	FUNCTION private
//	Btree::File::searchLeafPageForMultiCondition --
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		リーフページを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキーオブジェクトへ辿ることのできる
//	キー情報が記録されている可能性のあるリーフページを検索する。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		リーフページの物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::searchLeafPageForMultiCondition(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (TreeDepth_ == 1)
	{
		// 木の深さ（高さ）が1段の木構造…

		// つまり、ルートノードページがリーフページ

		// では、ルートノードページに検索条件と一致する
		// オブジェクトが記録されている可能性があるかを調べ、
		// その可能性があるならば、ルートノードページの
		// 物理ページ記述子を返せばよい。

		return
			this->containTargetKeyObjectForMultiCondition(
				RootNodePageID_,
				AttachNodePages_,
				true); // リーフページ
	}

	//
	// 以下は、木の深さ（高さ）が2段以上の木構造の場合の処理
	//

	// リーフページまで該当するノードページを辿っていく…

	PhysicalFile::PageID	nodePageID = RootNodePageID_;

	for (ModUInt32 depth = 1; depth < TreeDepth_; depth++)
	{
		nodePageID =
			this->searchChildNodePageForMultiCondition(nodePageID,
													   AttachNodePages_);

		if (nodePageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 検索条件と一致するオブジェクトが
			// 記録されている可能性がなくなってしまった…

			return 0;
		}
	}

	// ※ 物理ページのデタッチは、呼び出し側任せ。

	return File::attachPage(this->m_pTransaction,
							this->m_pPhysicalFile,
							nodePageID,
							this->m_FixMode,
							this->m_CatchMemoryExhaust,
							AttachNodePages_);
}

//
//	FUNCTION private
//	Btree::File::searchChildNodePageForMultiCondition --
//		複合条件と一致するキーフィールドの値が記録されている可能性のある
//		子ノードページを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキーオブジェクトが記録されている
//	可能性のある子ノードページを検索する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		親ノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	PhysicalFile::PageID
//		複合条件と一致するキーフィールドの値が書かれている
//		キーオブジェクトへ辿ることができるキー情報が記録されている
//		可能性のある子ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::PageID
File::searchChildNodePageForMultiCondition(
	const PhysicalFile::PageID	ParentNodePageID_,
	PageVector&					AttachNodePages_) const
{
	PhysicalFile::Page*	parentNodePage =
		this->containTargetKeyObjectForMultiCondition(
			ParentNodePageID_,
			AttachNodePages_,
			false); // リーフページではない

	if (parentNodePage == 0)
	{
		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	bool	match = false; // dummy

	NodePageHeader	parentNodePageHeader(this->m_pTransaction,
										 parentNodePage,
										 false); // リーフページではない

	int	keyInfoIndex =
		this->getKeyInformationIndexForMultiCondition(
			parentNodePage,
			parentNodePageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			false, // リーフページではない
			match);

	KeyInformation	keyInfo(this->m_pTransaction,
							parentNodePage,
							keyInfoIndex,
							false); // リーフページではない

	PhysicalFile::PageID	childNodePageID = keyInfo.readChildNodePageID();

	; _SYDNEY_ASSERT(
		childNodePageID != PhysicalFile::ConstValue::UndefinedPageID);

	checkMemoryExhaust(parentNodePage);

	return childNodePageID;
}

//
//	FUNCTION private
//	Btree::File::getKeyInformationIndexForMultiCondition --
//		複合条件に最も近いキーフィールドの値が書かれている
//		キーオブジェクトへ辿ることができるキー情報のインデックスを返す
//
//	NOTES
//	引数KeyInfoPage_が示すノードページ内で、
//	検索条件で指定された複合条件に最も近いキーフィールドの値が書かれている
//	キーオブジェクトへ辿ることができるキー情報のインデックスを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	const ModUInt32				UseKeyInfoNum_
//		引数KeyInfoPage_が示すノードページ内の使用中のキー情報数
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	bool&						Match_
//		そのキー情報に書かれている先頭キーフィールドの値が、
//		検索条件と完全に一致しているかどうか
//			true  : 完全に一致している
//			false : 完全に一致していない
//
//	RETURN
//	int
//		キー情報のインデックス
//
//	EXCEPTIONS
//	[YET!]
//
int
File::getKeyInformationIndexForMultiCondition(
	PhysicalFile::Page*			KeyInfoPage_,
	const ModUInt32				UseKeyInfoNum_,
	PageVector&					AttachNodePages_,
	const bool					IsLeafPage_,
	bool&						Match_) const
{
	; _SYDNEY_ASSERT(UseKeyInfoNum_ > 0);

	int	midKeyInfoIndex = 0;
	int	firstKeyInfoIndex = 0;
	int	lastKeyInfoIndex = UseKeyInfoNum_ - 1;

	int	keyInfoIndex = -1;

	KeyInformation	keyInfo(this->m_pTransaction,
							KeyInfoPage_,
							0,
							IsLeafPage_);

	Match_ = false;

	while (firstKeyInfoIndex <= lastKeyInfoIndex)
	{
		midKeyInfoIndex = (firstKeyInfoIndex + lastKeyInfoIndex) >> 1;

		keyInfo.setStartOffsetByIndex(midKeyInfoIndex);

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		int	compareResult =
			this->compareToMultiSearchCondition(KeyInfoPage_,
												AttachNodePages_,
												keyObjectID);

		if (compareResult < 0)
		{
			lastKeyInfoIndex = midKeyInfoIndex - 1;
		}
		else if (compareResult > 0)
		{
			firstKeyInfoIndex = midKeyInfoIndex + 1;
		}
		else
		{
			Match_ = true;

			keyInfoIndex = midKeyInfoIndex;

			int	addNum;

			if (this->m_pOpenParameter->m_bSortReverse)
			{
				if (keyInfoIndex == static_cast<int>(UseKeyInfoNum_) - 1)
				{
					break;
				}

				addNum = 1;
			}
			else
			{
				if (keyInfoIndex == 0)
				{
					break;
				}

				addNum = -1;
			}

			while (true)
			{
				keyInfoIndex += addNum;

				keyInfo.setStartOffsetByIndex(keyInfoIndex);

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				if (this->compareToMultiSearchCondition(KeyInfoPage_,
														AttachNodePages_,
														keyObjectID)
					!= 0)
				{
					keyInfoIndex -= addNum;

					break;
				}

				if (this->m_pOpenParameter->m_bSortReverse)
				{
					if (keyInfoIndex ==
						static_cast<int>(UseKeyInfoNum_ - 1))
					{
						break;
					}
				}
				else
				{
					if (keyInfoIndex == 0)
					{
						break;
					}
				}

			} // end while

			break;
		}
	}

	if (keyInfoIndex == -1)
	{
		keyInfoIndex = firstKeyInfoIndex;
	}

	return keyInfoIndex;
}

//
//	FUNCTION private
//	Btree::File::containTargetKeyObjectForMultiCondition --
//		複合条件と一致するキーオブジェクトがノードページ内に
//		存在する可能性があるかどうかを知らせる
//
//	NOTES
//	引数NodePageID_で示されるノードページ内に、
//	検索条件で指定された複合条件と一致する
//	キーフィールドの値が書かれているキーオブジェクトが
//	存在する可能性があるかどうかを知らせる。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	NodePageID_
//		調べるノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	PhysicalFile::Page*
//		複合条件と一致するキーフィールドの値が書かれている
//		キーオブジェクトがノードページ内に存在する可能性があれば、
//		そのノードページの物理ページ記述子、存在する可能性がなければ、
//		0（ヌルポインタ）。
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
File::containTargetKeyObjectForMultiCondition(
	const PhysicalFile::PageID	NodePageID_,
	PageVector&					AttachNodePages_,
	const bool					IsLeafPage_) const
{
	PhysicalFile::Page*	nodePage =
		File::attachPage(this->m_pTransaction,
						 this->m_pPhysicalFile,
						 NodePageID_,
						 this->m_FixMode,
						 this->m_CatchMemoryExhaust,
						 AttachNodePages_);

	; _SYDNEY_ASSERT(nodePage != 0);

	NodePageHeader	nodePageHeader(this->m_pTransaction,
								   nodePage,
								   IsLeafPage_);

	const ModUInt32 useKeyInfoNum =
		nodePageHeader.readUseKeyInformationNumber();

	bool	exist = true;

	if (useKeyInfoNum > 0)
	{
		KeyInformation	keyInfo(this->m_pTransaction,
								nodePage,
								useKeyInfoNum - 1,
								IsLeafPage_);

		ModUInt64	lastKeyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			lastKeyObjectID != FileCommon::ObjectID::Undefined &&
			lastKeyObjectID != 0);

		int	compareResult =
			this->compareToMultiSearchCondition(nodePage,
												AttachNodePages_,
												lastKeyObjectID);

		if (compareResult > 0)
		{
			exist = false;
		}
	}
	else
	{
		exist = false;
	}

	if (exist == false)
	{
		checkMemoryExhaust(nodePage);

		nodePage = 0;
	}

	return nodePage;
}

//
//	FUNCTION private
//	Btree::File::compareToMultiSearchCondition --
//		複合条件とキーフィールドの値を比較する
//
//	NOTES
//	検索条件で指定される複合条件とキーオブジェクトに記録されている
//	キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyInfoPage_
//		キー情報が記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const ModUInt64									KeyObjectID_
//		キーオブジェクトのID
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToMultiSearchCondition(
	PhysicalFile::Page*						KeyInfoPage_,
	PageVector&								AttachNodePages_,
	const ModUInt64							KeyObjectID_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	PhysicalFile::Page*	keyObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_pTransaction,
											  KeyObjectID_,
											  KeyInfoPage_,
											  keyObjectPage,
											  this->m_FixMode,
											  this->m_CatchMemoryExhaust,
											  AttachNodePages_);

	SearchHint::CompareType::Value	compareType =
		SearchHint::CompareType::Undefined;

	if (this->m_SearchHint.m_ExistSeparateKey)
	{
		compareType = SearchHint::CompareType::OnlyLinkKey;
	}
	else
	{
		if (this->m_SearchHint.m_LastStartOperatorIsEquals)
		{
			compareType = SearchHint::CompareType::All;
		}
		else
		{
			compareType = SearchHint::CompareType::OnlyNotLastKey;
		}
	}

	int	compareResult = compareToMultiSearchCondition(
		keyObjectPage, AttachNodePages_,
		Common::ObjectIDData::getLatterValue(KeyObjectID_),
		compareType, Target_);

	if (attached)
	{
		checkMemoryExhaust(keyObjectPage);
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToMultiSearchCondition --
//		複合条件とキーフィールドの値を比較する
//
//	NOTES
//	検索条件で指定される複合条件とキーオブジェクトに記録されている
//	キーフィールドの値を比較し、比較結果を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*								KeyObjectPage_
//		キーオブジェクトが記録されているノードページの物理ページ記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const PhysicalFile::AreaID						KeyObjectAreaID_
//		キーオブジェクトが記録されている物理エリアの識別子
//	const Btree::SearchHint::CompareType::Value		CompareType_
//		比較対象
//			All             : 全ての検索対象キーフィールドを比較
//			OnlyLinkKey     : 連続した検索対象キーフィールドのみ比較
//			OnlySeparateKey : 離れた検索対象キーフィールドのみ比較
//			OnlyLastKey     : 最終検索対象キーフィールドのみ比較
//			OnlyNotLastKey  : 最終検索対象キーフィールド以外を比較
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToMultiSearchCondition(
	PhysicalFile::Page*						KeyObjectPage_,
	PageVector&								AttachNodePages_,
	const PhysicalFile::AreaID				KeyObjectAreaID_,
	const SearchHint::CompareType::Value	CompareType_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	; _SYDNEY_ASSERT(KeyObjectPage_ != 0);

	int	compareResult = 0;

	for (int i = 0; i < this->m_SearchHint.m_FieldNumber; i++)
	{
		int	fieldIndex = *(this->m_SearchHint.m_FieldIndexArray + i);

		if (CompareType_ == SearchHint::CompareType::OnlyLinkKey &&
			fieldIndex > this->m_SearchHint.m_LastLinkKeyFieldIndex)
		{
			break;
		}

		if (CompareType_ == SearchHint::CompareType::OnlySeparateKey &&
			fieldIndex <= this->m_SearchHint.m_LastLinkKeyFieldIndex)
		{
			continue;
		}

		if (CompareType_ == SearchHint::CompareType::OnlyLastKey &&
			fieldIndex < this->m_SearchHint.m_LastKeyFieldIndex)
		{
			continue;
		}

		if (CompareType_ == SearchHint::CompareType::OnlyNotLastKey &&
			fieldIndex == this->m_SearchHint.m_LastKeyFieldIndex)
		{
			break;
		}

		if (*(this->m_SearchHint.m_IsFixedArray + i))
		{
			void*	fieldValue =
				this->getFieldPointer(KeyObjectPage_,
									  KeyObjectAreaID_,
									  fieldIndex,
									  true); // キーオブジェクト

			if (fieldValue == 0)
			{
				return 1 * *(this->m_SearchHint.m_MultiNumberArray + i);
			}

			if (Target_ == SearchHint::CompareTarget::Start)
			{
				compareResult =
					this->m_SearchHint.compareToFixedStartCondition(
						fieldValue,
						i);
			}
			else
			{
				compareResult =
					this->m_SearchHint.compareToFixedStopCondition(
						fieldValue);
			}

			if (compareResult != 0)
			{
				break;
			}
		}
		else if (*(this->m_SearchHint.m_StartOperatorArray + i) ==
				 LogicalFile::TreeNodeInterface::EqualsToNull)
		{
			if (this->getFieldPointer(KeyObjectPage_,
									  KeyObjectAreaID_,
									  fieldIndex,
									  true) // キーオブジェクト
				!= 0)
			{
				return -1 * *(this->m_SearchHint.m_MultiNumberArray + i);
			}
		}
		else
		{
			; _SYDNEY_ASSERT(
				*(this->m_SearchHint.m_FieldTypeArray + i) ==
				Common::DataType::String);

			if (*(this->m_cFileParameter.m_FieldOutsideArray + fieldIndex))
			{
				// outside

				char*	fieldObjectIDReadPos =
					static_cast<char*>(
						this->getFieldPointer(KeyObjectPage_,
											  KeyObjectAreaID_,
											  fieldIndex,
											  true)); // キーオブジェクト

				if (fieldObjectIDReadPos == 0)
				{
					// is null

					return 1 * *(this->m_SearchHint.m_MultiNumberArray + i);
				}

				ModUInt64	fieldObjectID;
				File::readObjectID(fieldObjectIDReadPos, fieldObjectID);

				compareResult =
					this->compareToStringSearchCondition(KeyObjectPage_,
														 AttachNodePages_,
														 fieldObjectID,
														 i,
														 Target_);
			}
			else
			{
				// inside

				const File::InsideVarFieldLen*	fieldLength =
					static_cast<const File::InsideVarFieldLen*>(
						this->getFieldPointer(KeyObjectPage_,
											  KeyObjectAreaID_,
											  fieldIndex,
											  true)); // キーオブジェクト

				if (fieldLength == 0)
				{
					// is null

					return 1 * *(this->m_SearchHint.m_MultiNumberArray + i);
				}

				compareResult =
					this->compareToStringSearchCondition(fieldLength,
														 i,
														 Target_);
			}

			if (compareResult != 0)
			{
				break;
			}
		}
	}

	return compareResult;
}

//
//	FUNCTION private
//	Btree::File::compareToStringSearchCondition --
//		検索条件と外置きではない文字列フィールドの値を比較する
//
//	NOTES
//	検索条件と外置きではない文字列フィールドの値を比較する。
//
//	ARGUMENTS
//	Btree::File::InsideVarFieldLen*					FieldLenghtReadPos_
//		文字列長が記録されている領域へのポインタ
//	const int										ArrayIndex_
//		検索ヒントの配列のインデックス
//	const Btree::SearchHint::CompareTarget::Value	Target_
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToStringSearchCondition(
	const File::InsideVarFieldLen*			FieldLengthReadPos_,
	const int								ArrayIndex_,
	const SearchHint::CompareTarget::Value	Target_) const
{
	ModSize	numChar = *FieldLengthReadPos_ / sizeof(ModUnicodeChar);

	const ModUnicodeChar*	fieldValue =
		syd_reinterpret_cast<const ModUnicodeChar*>(FieldLengthReadPos_ + 1);

	ModUnicodeChar	forZeroByte = 0;

	if (numChar == 0)
	{
		fieldValue = &forZeroByte;
	}

	ModUnicodeString	unicodeStringFieldValue(fieldValue, numChar);

	const Common::Data*	condition = 0;

	if (Target_ == SearchHint::CompareTarget::Start)
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStartArray.
				getElement(ArrayIndex_).get();
	}
	else
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStopArray.
				getElement(ArrayIndex_).get();
	}

	; _SYDNEY_ASSERT(condition != 0);

	; _SYDNEY_ASSERT(condition->getType() == Common::DataType::String);

	const Common::StringData*	stringCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*, condition);

	; _SYDNEY_ASSERT(stringCondition != 0);

	const ModUnicodeString&	unicodeCondition =
		stringCondition->getValue();

	return
		unicodeCondition.compare(unicodeStringFieldValue) *
		*(this->m_SearchHint.m_MultiNumberArray + ArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::compareToStringSearchCondition --
//		検索条件と外置き文字列フィールドの値を比較する
//
//	NOTES
//	検索条件と外置き文字列フィールドの値を比較する。
//
//	ARGUMENTS
//	PhysicalFile::Page*								FieldObjectIDPage_
//		外置きフィールドオブジェクトのIDが記録されているノードページの記述子
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	const ModUInt64									FieldObjectID_
//		外置きフィールドオブジェクトのID
//	const int										ArrayIndex_
//		検索ヒントの配列のインデックス
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//
//	RETURN
//	int
//		比較結果
//			< 0 : 検索条件の方がキー値順で前方
//			= 0 : 検索条件とキーフィールドの値が等しい
//			> 0 : 検索条件の方がキー値順で後方
//
//	EXCEPTIONS
//	[YET!]
//
int
File::compareToStringSearchCondition(
	PhysicalFile::Page*						FieldObjectIDPage_,
	PageVector&								AttachNodePages_,
	const ModUInt64							FieldObjectID_,
	const int								ArrayIndex_,
	const SearchHint::CompareTarget::Value	Target_ // = Start
	) const
{
	const Common::Data*	condition = 0;

	if (Target_ == SearchHint::CompareTarget::Start)
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStartArray.
				getElement(ArrayIndex_).get();
	}
	else
	{
		condition =
			this->m_pOpenParameter->m_cSearchCondition.m_cStopArray.
				getElement(ArrayIndex_).get();
	}

	; _SYDNEY_ASSERT(condition != 0);

	; _SYDNEY_ASSERT(
		condition->getType() == Common::DataType::String);

	const Common::StringData*	stringCondition =
		_SYDNEY_DYNAMIC_CAST(const Common::StringData*, condition);

	; _SYDNEY_ASSERT(stringCondition != 0);

	const ModUnicodeString&	unicodeCondition =
		stringCondition->getValue();

	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(FieldObjectID_);

	PhysicalFile::Page*	objectPage = 0;

	bool	attached = false;

	if (FieldObjectIDPage_->getID() == objectPageID)
	{
		objectPage = FieldObjectIDPage_;
	}
	else
	{
		attached = true;

		objectPage = File::attachPage(this->m_pTransaction,
									  this->m_pPhysicalFile,
									  objectPageID,
									  this->m_FixMode,
									  this->m_CatchMemoryExhaust,
									  AttachNodePages_);
	}

	int	compareResult = 0;

	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const void*	objectAreaTop = File::getConstAreaTop(objectPage,
													  objectAreaID);

	const File::ObjectType*	objectType =
		static_cast<const File::ObjectType*>(objectAreaTop);

	// object type
	if (*objectType == File::NormalObjectType)
	{
		Os::Memory::Size	fieldLen =
			objectPage->getAreaSize(objectAreaID) -
			File::ObjectTypeArchiveSize;

		ModSize	numChar = fieldLen / sizeof(ModUnicodeChar);

		const ModUnicodeChar*	fieldValue =
			syd_reinterpret_cast<const ModUnicodeChar*>(objectType + sizeof(File::ObjectType));

		ModUnicodeChar	forZeroByte = 0;

		if (numChar == 0)
		{
			fieldValue = &forZeroByte;
		}

		ModUnicodeString	unicodeStringFieldValue(fieldValue, numChar);

		compareResult = unicodeCondition.compare(unicodeStringFieldValue);
	}
	else if (*objectType == File::CompressedObjectType ||
			 *objectType == File::DivideCompressedObjectType)
	{
		Common::Data::Pointer	stringFieldValue;

		File::readOutsideVariableField(this->m_pTransaction,
									   FieldObjectIDPage_,
									   FieldObjectID_,
									   Common::DataType::String,
									   stringFieldValue,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

		compareResult = stringCondition->compareTo(stringFieldValue.get());
	}
	else
	{
		; _SYDNEY_ASSERT(*objectType == File::DivideObjectType);

		ModUnicodeString	unicodeStringFieldValue;

		File::readOutsideStringField(this->m_pTransaction,
									 FieldObjectIDPage_,
									 FieldObjectID_,
									 unicodeStringFieldValue,
									 this->m_FixMode,
									 this->m_CatchMemoryExhaust,
									 AttachNodePages_);

		compareResult = unicodeCondition.compare(unicodeStringFieldValue);
	}

	if (attached)
	{
		checkMemoryExhaust(objectPage);
	}

	return
		compareResult *
		*(this->m_SearchHint.m_MultiNumberArray + ArrayIndex_);
}

//
//	FUNCTION private
//	Btree::File::searchByKeyRev --
//		キー値によりオブジェクトを検索する
//
//	NOTES
//	キー値によりオブジェクトを検索し、
//	該当するオブジェクトが存在すればそのオブジェクトのIDを返し、
//	該当するオブジェクトが存在しなければ、
//	FileCommon::ObjectID::Undefinedを返す。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//	※ Search + Fetchモードの場合、
//	　 Fetch検索条件と一致するかどうかは
//	　 本関数以下では意識していない。
//	　 もしかしたらFetch検索条件とは一致しないオブジェクトの
//	　 オブジェクトIDを返すかもしれないので、
//	　 呼び出し側でその確認が必要である。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByKeyRev(FileInformation&	FileInfo_,
					 PageVector&		AttachNodePages_,
					 ValueFile*			ValueFile_,
					 PageVector&		AttachValuePages_) const
{
	if (this->m_SearchHint.m_ConditionType ==
		SearchHint::ConditionType::Single)
	{
		if (this->m_SearchHint.m_LastIsSpan)
		{
			return this->searchBySpanConditionRev(FileInfo_,
												  AttachNodePages_,
												  ValueFile_,
												  AttachValuePages_);
		}
		else
		{
			return this->searchBySingleConditionRev(FileInfo_,
													AttachNodePages_,
													ValueFile_,
													AttachValuePages_);
		}
	}
	else
	{
		; _SYDNEY_ASSERT(
			this->m_SearchHint.m_ConditionType ==
			SearchHint::ConditionType::Multi);

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->searchByMultiConditionRev(treeDepth,
											   rootNodePageID,
											   AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::searchBySingleConditionRev --
//		先頭キーフィールドへの単一条件でオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドへの単一条件でオブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		検索条件と一致するオブジェクトのオブジェクトID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleConditionRev(FileInformation&	FileInfo_,
								 PageVector&		AttachNodePages_,
								 ValueFile*			ValueFile_,
								 PageVector&		AttachValuePages_) const
{
	// m_SearchHint.m_StartOperatorArray[0]を見る

	if (*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::Equals)
	{
		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->searchBySingleEqualsRev(treeDepth,
											 rootNodePageID,
											 AttachNodePages_);
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThan)
	{
		return this->searchBySingleLessThanRev(FileInfo_,
											   AttachNodePages_,
											   ValueFile_,
											   AttachValuePages_,
											   false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::LessThanEquals)
	{
		return this->searchBySingleLessThanRev(FileInfo_,
											   AttachNodePages_,
											   ValueFile_,
											   AttachValuePages_,
											   true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThan)
	{
		return this->searchBySingleGreaterThanRev(FileInfo_,
												  AttachNodePages_,
												  ValueFile_,
												  AttachValuePages_,
												  false); // Equals含まず
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals)
	{
		return this->searchBySingleGreaterThanRev(FileInfo_,
												  AttachNodePages_,
												  ValueFile_,
												  AttachValuePages_,
												  true); // Equals含む
	}
	else if (*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::Like)
	{
		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		return this->likeSearchRev(treeDepth,
								   rootNodePageID,
								   AttachNodePages_);
	}
	else
	{
		; _SYDNEY_ASSERT(
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::EqualsToNull);

		return this->searchBySingleEqualsToNullRev(FileInfo_,
												   AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsRev --
//		先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsRev(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchBySingleEqualsRevSimpleKey(TreeDepth_,
													  RootNodePageID_,
													  AttachNodePages_);
	}

	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForSingleCondition(TreeDepth_,
											   RootNodePageID_,
											   AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForSingleCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	bool	lp = true;
	do
	{
		ModUInt64	saveObjectID = resultObjectID;

		if (this->assignNextKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo)
			== false)
		{
			resultObjectID = saveObjectID;

			break;
		}

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		if (this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID)
			!= 0)
		{
			lp = false;
		}
		else
		{
			resultObjectID = keyInfo.readValueObjectID();
		}
	}
	while (lp);

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleLessThanRev --
//		先頭キーフィールドの値が検索条件以下（または未満）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以下（または未満）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&							FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&								AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*								ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&								AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool										ContainEquals_
//		比較演算子がLessThanとLessThanEqualsのどちらか
//			true  : LessThanEquals
//			false : LessThan
//	const Btree::SearchHint::CompareTarget::Value	Target_ = Start
//		比較対象
//			Start : 検索開始条件と比較
//			Stop  : 検索終了条件と比較
//		※ File::searchBySpanConditionRevのための引数である。
//	PhysicalFile::PageID*							KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーオブジェクトへ辿れる
//		キー情報が記録されているリーフページの
//		物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionRevのための引数であり、
//		　 本関数がポインタが指している領域に設定する。
//	ModUInt32										KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//	ModUInt64										KeyObjectID_ = 0
//		同、キーオブジェクトのオブジェクトIDへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以下（または未満）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleLessThanRev(
	FileInformation&						FileInfo_,
	PageVector&								AttachNodePages_,
	ValueFile*								ValueFile_,
	PageVector&								AttachValuePages_,
	const bool								ContainEquals_,
	const SearchHint::CompareTarget::Value	Target_,            // = Start
	PhysicalFile::PageID*					KeyInfoLeafPageID_, // = 0
	ModUInt32*								KeyInfoIndex_,      // = 0
	ModUInt64*								KeyObjectID_        // = 0
	) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		; _SYDNEY_ASSERT(KeyObjectID_ == 0);

		return
			this->searchBySingleLessThanRevSimpleKey(FileInfo_,
													 AttachNodePages_,
													 ValueFile_,
													 AttachValuePages_,
													 ContainEquals_,
													 Target_,
													 KeyInfoLeafPageID_,
													 KeyInfoIndex_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage = this->searchLeafPageForSingleCondition(treeDepth,
														  rootNodePageID,
														  AttachNodePages_,
														  Target_);

		if (leafPage == 0)
		{
			// もしかしたら、挿入されているオブジェクトすべてが
			// 検索条件に一致するのかもしれない。
			// 例えば、先頭キーフィールドの最大値が100で、
			// 検索条件が「 key < 200 」のような場合、
			// File::searchLeafPageForSingleConditionは、
			// 0を返すので、ここで、“キー値順での最終オブジェクト”と
			// 検索条件を比較してみる。

			keyObjectID =
				this->getLastObjectID(FileInfo_,
									  AttachNodePages_,
									  true); // キーオブジェクトの
									         // オブジェクトIDを取得する

			leafPage = File::attachPage(
				m_pTransaction, m_pPhysicalFile,
				Common::ObjectIDData::getFormerValue(keyObjectID),
				m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);

			if (compareToTopSearchCondition(
					leafPage,
					AttachNodePages_,
					static_cast<PhysicalFile::AreaID>(
						Common::ObjectIDData::getLatterValue(keyObjectID)),
					Target_)
				> 0)
			{
				// すべてのオブジェクトが検索条件と一致する…

				resultObjectID =
					this->getLastObjectID(FileInfo_,
										  AttachNodePages_,
										  false); // バリューオブジェクト
										          // のオブジェクトIDを
										          // 取得する
			}

			if (KeyInfoLeafPageID_ != 0)
			{
				// File::searchBySpanConditionRevから呼ばれた…

				// ならば、
				// リーフページのキーオブジェクトに関する情報を
				// 設定する必要がある。

				; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
								 KeyObjectID_ != 0);

				ValueFile_->readLeafInfo(resultObjectID,
										 AttachValuePages_,
										 this->m_CatchMemoryExhaust,
										 *KeyInfoLeafPageID_,
										 *KeyInfoIndex_);

				if (*KeyInfoLeafPageID_ != leafPage->getID())
				{
					checkMemoryExhaust(leafPage);

					leafPage = File::attachPage(this->m_pTransaction,
												this->m_pPhysicalFile,
												*KeyInfoLeafPageID_,
												this->m_FixMode,
												this->m_CatchMemoryExhaust,
												AttachNodePages_);
				}

				KeyInformation	keyInfo(this->m_pTransaction,
										leafPage,
										*KeyInfoIndex_,
										true); // リーフページ

				*KeyObjectID_ = keyInfo.readKeyObjectID();
			}

			checkMemoryExhaust(leafPage);

			return resultObjectID;
		}

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				AttachNodePages_,
				true, // リーフページ
				match,
				Target_);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true); // リーフページ

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(keyObjectID != FileCommon::ObjectID::Undefined &&
						 keyObjectID != 0);

		resultObjectID = keyInfo.readValueObjectID();

		// LessThanEqualsでmatchでも、
		// もっと先に検索条件に一致するオブジェクトが
		// 存在するかもしれない。
		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID,
											  Target_);

		if ((ContainEquals_ == false && compareResult <= 0) ||
			(ContainEquals_ && compareResult < 0))
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 小さかった…
			// または、
			// 検索条件と
			// 記録されているキーオブジェクトが
			// 等しかった…

			bool	lp = true;
			do
			{
				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					// なかった…

					checkMemoryExhaust(leafPage);

					return FileCommon::ObjectID::Undefined;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID,
													  Target_);

				if (compareResult > 0 ||
					(ContainEquals_ && compareResult == 0))
				{
					resultObjectID = keyInfo.readValueObjectID();

					lp = false;

					if (KeyInfoLeafPageID_ != 0)
					{
						// File::searchBySpanConditionRevから呼ばれた…

						// ならば、
						// リーフページのキーオブジェクトに関する情報を
						// 設定する必要がある。

						; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
										 KeyObjectID_ != 0);

						*KeyInfoLeafPageID_ = leafPage->getID();

						*KeyInfoIndex_ = keyInfo.getIndex();

						*KeyObjectID_ = keyObjectID;
					}
				}
			}
			while (lp);
		}
		else
		{
			// 検索条件の方が
			// 記録されているキーオブジェクトよりも
			// 大きかった…

			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// File::searchBySpanConditionRevから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignNextKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID,
													  Target_);

				if (compareResult < 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// 「 key1 < 100 」のような検索条件で、
		// 一番最初に返すオブジェクトは、
		// “キー値順に最終オブジェクト”となる。

		// でも、“キー値順に最終オブジェクト”の
		// 先頭キーフィールドの値に、例えば、100という値が
		// 記録されていたとして、
		// 検索条件が「 key1 < 50 」のような場合もある。
		// このような場合には、
		// 無条件で“キー値順に最終オブジェクト”を
		// 返すわけにもいかない。
		// しかもSearch+Fetchの可能性もあるわけだし…。

		// なので、“キー値順に最終オブジェクト”と
		// 検索条件との比較が必要となる。

		// File::searchBySingleLessThanは、
		// File::searchBySpanConditionからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が昇順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0 &&
						 KeyObjectID_ == 0);

		// しかも、“検索開始条件”と比較するはず。
		; _SYDNEY_ASSERT(Target_ == SearchHint::CompareTarget::Start);

		keyObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  true); // キーオブジェクトの
								         // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		int	compareResult = compareToTopSearchCondition(
			leafPage, AttachNodePages_,
			static_cast<PhysicalFile::AreaID>(
				Common::ObjectIDData::getLatterValue(keyObjectID)));

		if (compareResult > 0 ||
			ContainEquals_ == false && compareResult == 0)
		{
			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  false); // バリューオブジェクトの
								          // オブジェクトIDを取得する
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleGreaterThanRev --
//		先頭キーフィールドの値が検索条件以上（または超）の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件以上（または超）の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*			ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool					ContainEquals_
//		比較演算子がGreaterThanとGreaterThanEqualsのどちらか
//			true  : GreaterThanEquals
//			false : GreaterThan
//	PhysicalFile::PageID*		KeyInfoLeafPageID_ = 0
//		検索条件と一致するキーオブジェクトへ辿れる
//		キー情報が記録されているリーフページの
//		物理ページ識別子へのポインタ
//		※ File::searchBySpanConditionRevのための引数であり、
//		　 本関数がポインタが指している領域に設定する。
//	ModUInt32					KeyInfoIndex_ = 0
//		同、キー情報のインデックスへのポインタ
//		※ 同上。
//	ModUInt64					KeyObjectID_ = 0
//		同、キーオブジェクトのオブジェクトIDへのポインタ
//		※ 同上。
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値が検索条件以上（または超）のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleGreaterThanRev(
	FileInformation&		FileInfo_,
	PageVector&				AttachNodePages_,
	ValueFile*				ValueFile_,
	PageVector&				AttachValuePages_,
	const bool				ContainEquals_,
	PhysicalFile::PageID*	KeyInfoLeafPageID_, // = 0
	ModUInt32*				KeyInfoIndex_,      // = 0
	ModUInt64*				KeyObjectID_        // = 0
	) const
{
	ModUInt32	treeDepth = FileInfo_.readTreeDepth();

	PhysicalFile::PageID	rootNodePageID = FileInfo_.readRootNodePageID();

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		; _SYDNEY_ASSERT(KeyObjectID_ == 0);

		return
			this->searchBySingleGreaterThanRevSimpleKey(FileInfo_,
														AttachNodePages_,
														ValueFile_,
														AttachValuePages_,
														ContainEquals_,
														KeyInfoLeafPageID_,
														KeyInfoIndex_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// File::searchBySingleGreaterThanRevは、
		// File::searchBySpanConditionRevからも呼ばれる。
		// しかし、それは、
		// “先頭キーフィールドのソート順が降順”の場合のみである。
		// したがって、引数KeyInfoLeafPageID_などは
		// この場合、0であるはず。
		; _SYDNEY_ASSERT(KeyInfoLeafPageID_ == 0 &&
						 KeyInfoIndex_ == 0 &&
						 KeyObjectID_ == 0);

		keyObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  true); // キーオブジェクトの
								         // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		int	compareResult = compareToTopSearchCondition(
			leafPage, AttachNodePages_,
			static_cast<PhysicalFile::AreaID>(
				Common::ObjectIDData::getLatterValue(keyObjectID)));

		if (compareResult > 0 ||
			(ContainEquals_ == false && compareResult == 0))
		{
			// なかった…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		resultObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  false); // バリューオブジェクトの
								          // オブジェクトIDを取得する
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		ModUInt32	treeDepth = FileInfo_.readTreeDepth();

		PhysicalFile::PageID	rootNodePageID =
			FileInfo_.readRootNodePageID();

		leafPage = this->searchLeafPageForSingleCondition(treeDepth,
														  rootNodePageID,
														  AttachNodePages_);

		if (leafPage == 0)
		{
			// もしかしたら、挿入されているオブジェクトすべてが
			// 検索条件に一致するのかもしれない。
			// 例えば、先頭キーフィールドの最小値が300で、
			// 検索条件が「 key > 200 」のような場合、
			// File::searchLeafPageForSingleConditionは、
			// 0を返すので、ここで、“キー値順での最終オブジェクト”と
			// 検索条件を比較してみる。

			keyObjectID =
				this->getLastObjectID(FileInfo_,
									  AttachNodePages_,
									  true); // キーオブジェクトの
									         // オブジェクトIDを取得する

			leafPage = File::attachPage(
				m_pTransaction, m_pPhysicalFile,
				Common::ObjectIDData::getFormerValue(keyObjectID),
				m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);

			if (compareToTopSearchCondition(
					leafPage, AttachNodePages_,
					static_cast<PhysicalFile::AreaID>(
						Common::ObjectIDData::getLatterValue(keyObjectID)))
				> 0)
			{
				// すべてのオブジェクトが検索条件と一致する…

				resultObjectID =
					this->getLastObjectID(FileInfo_,
										  AttachNodePages_,
										  false); // バリューオブジェクト
										          // のオブジェクトIDを
										          // 取得する
			}

			if (KeyInfoLeafPageID_ != 0)
			{
				// File::searchBySpanConditionRevから呼ばれた…

				// ならば、
				// リーフページのキーオブジェクトに関する情報を
				// 設定する必要がある。

				; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
								 KeyObjectID_ != 0);

				ValueFile_->readLeafInfo(resultObjectID,
										 AttachValuePages_,
										 this->m_CatchMemoryExhaust,
										 *KeyInfoLeafPageID_,
										 *KeyInfoIndex_);

				if (*KeyInfoLeafPageID_ != leafPage->getID())
				{
					checkMemoryExhaust(leafPage);

					leafPage = File::attachPage(this->m_pTransaction,
												this->m_pPhysicalFile,
												*KeyInfoLeafPageID_,
												this->m_FixMode,
												this->m_CatchMemoryExhaust,
												AttachNodePages_);
				}

				KeyInformation	keyInfo(this->m_pTransaction,
										leafPage,
										*KeyInfoIndex_,
										true); // リーフページ

				*KeyObjectID_ = keyInfo.readKeyObjectID();
			}

			checkMemoryExhaust(leafPage);

			return resultObjectID;
		}

		bool	match = false; // dummy

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		int	keyInfoIndex =
			this->getKeyInformationIndexForSingleCondition(
				leafPage,
				leafPageHeader.readUseKeyInformationNumber(),
				AttachNodePages_,
				true, // リーフページ
				match);

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								keyInfoIndex,
								true); // リーフページ

		keyObjectID = keyInfo.readKeyObjectID();

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		resultObjectID = keyInfo.readValueObjectID();

		int	compareResult =
			this->compareToTopSearchCondition(leafPage,
											  AttachNodePages_,
											  keyObjectID);

		if ((ContainEquals_ == false && compareResult <= 0) ||
			(ContainEquals_ && compareResult < 0))
		{
			bool	lp = true;
			do
			{
				if (this->assignPrevKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					// なかった…

					checkMemoryExhaust(leafPage);

					return FileCommon::ObjectID::Undefined;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID);

				if (compareResult > 0 ||
					(ContainEquals_ && compareResult == 0))
				{
					resultObjectID = keyInfo.readValueObjectID();

					lp = false;

					if (KeyInfoLeafPageID_ != 0)
					{
						// searchBySpanConditionから呼ばれた…

						// ならば、
						// リーフページのキーオブジェクトに関する情報を
						// 設定する必要がある。

						; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
										 KeyObjectID_ != 0);

						*KeyInfoLeafPageID_ = leafPage->getID();

						*KeyInfoIndex_ = keyInfo.getIndex();

						*KeyObjectID_ = keyObjectID;
					}
				}
			}
			while (lp);
		}
		else
		{
			bool	lp = true;
			do
			{
				if (KeyInfoLeafPageID_ != 0)
				{
					// searchBySpanConditionから呼ばれた…

					// ならば、
					// リーフページのキーオブジェクトに関する情報を
					// 設定する必要がある。

					; _SYDNEY_ASSERT(KeyInfoIndex_ != 0 &&
									 KeyObjectID_ != 0);

					*KeyInfoLeafPageID_ = leafPage->getID();

					*KeyInfoIndex_ = keyInfo.getIndex();

					*KeyObjectID_ = keyObjectID;
				}

				ModUInt64	saveObjectID = resultObjectID;

				if (this->assignNextKeyInformation(leafPage,
												   AttachNodePages_,
												   leafPageHeader,
												   keyInfo)
					== false)
				{
					resultObjectID = saveObjectID;

					break;
				}

				keyObjectID = keyInfo.readKeyObjectID();

				; _SYDNEY_ASSERT(
					keyObjectID != FileCommon::ObjectID::Undefined &&
					keyObjectID != 0);

				compareResult =
					this->compareToTopSearchCondition(leafPage,
													  AttachNodePages_,
													  keyObjectID);

				if (compareResult < 0 ||
					(ContainEquals_ == false && compareResult == 0))
				{
					lp = false;
				}
				else
				{
					resultObjectID = keyInfo.readValueObjectID();
				}
			}
			while (lp);
		}
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySingleEqualsToNullRev --
//		先頭キーフィールドの値がヌル値のオブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値がヌル値のオブジェクトを検索する
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		先頭キーフィールドの値がヌル値のオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySingleEqualsToNullRev(
	FileInformation&	FileInfo_,
	PageVector&			AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return
			this->searchBySingleEqualsToNullRevSimpleKey(FileInfo_,
														 AttachNodePages_);
	}

	ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;
	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	PhysicalFile::Page*	leafPage = 0;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		// ということは、
		// ”キー値順に先頭オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならば、先頭から順にヌル値かどうかを
		// チェックしていって、ヌル値ではないオブジェクトの
		// 手前のオブジェクトを返せばよい。
		// ヌル値でないのなら、検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		keyObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 true); // キーオブジェクトの
								        // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 先頭オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID =
			this->getTopObjectID(FileInfo_,
								 AttachNodePages_,
								 false); // バリューオブジェクトの
								         // オブジェクトIDを取得する

		PhysicalFile::PageID	keyInfoLeafPageID =
			FileInfo_.readTopLeafPageID();

		if (leafPage->getID() != keyInfoLeafPageID)
		{
			checkMemoryExhaust(leafPage);

			leafPage = File::attachPage(this->m_pTransaction,
										this->m_pPhysicalFile,
										keyInfoLeafPageID,
										this->m_FixMode,
										this->m_CatchMemoryExhaust,
										AttachNodePages_);

			; _SYDNEY_ASSERT(leafPage != 0);
		}

		NodePageHeader	leafPageHeader(this->m_pTransaction,
									   leafPage,
									   true); // リーフページ

		KeyInformation	keyInfo(this->m_pTransaction,
								leafPage,
								0,
								true); // リーフページ

		bool	lp = true;
		do
		{
			ModUInt64	saveObjectID = resultObjectID;

			if (this->assignNextKeyInformation(leafPage,
											   AttachNodePages_,
											   leafPageHeader,
											   keyInfo)
				== false)
			{
				resultObjectID = saveObjectID;

				break;
			}

			keyObjectID = keyInfo.readKeyObjectID();

			; _SYDNEY_ASSERT(
				keyObjectID != FileCommon::ObjectID::Undefined &&
				keyObjectID != 0);

			PhysicalFile::Page*	keyObjectPage = 0;

			bool	attached =
				File::attachObjectPage(this->m_pTransaction,
									   keyObjectID,
									   leafPage,
									   keyObjectPage,
									   this->m_FixMode,
									   this->m_CatchMemoryExhaust,
									   AttachNodePages_);

			if (getTopKeyPointer(
					keyObjectPage,
					Common::ObjectIDData::getLatterValue(keyObjectID))) {
				lp = false;
			}
			else
			{
				resultObjectID = keyInfo.readValueObjectID();
			}

			if (attached)
			{
				checkMemoryExhaust(keyObjectPage);
			}
		}
		while (lp);
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		// ということは、
		// “キー値順に最終オブジェクト”が
		// ヌル値かな？とチェックして、
		// ヌル値ならばそのオブジェクトを返せばよいし、
		// そうでなければ検索条件と一致するオブジェクトは
		// 存在しないこととなる。

		keyObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  true); // キーオブジェクトの
									  // オブジェクトIDを取得する

		leafPage = File::attachPage(
			m_pTransaction, m_pPhysicalFile,
			Common::ObjectIDData::getFormerValue(keyObjectID),
			m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

		; _SYDNEY_ASSERT(leafPage != 0);

		// ※ getTopFieldPointerは、
		// 　 先頭キーフィールドの値として
		// 　 ヌル値が設定されている場合には、
		// 　 ヌルポインタ(0)を返す。
		// 　 ちなみに、ヌル値が設定されているかどうかは
		// 　 記録されているフィールドタイプの違いにより
		// 　 判断可能である。

		if (getTopKeyPointer(
				leafPage,
				Common::ObjectIDData::getLatterValue(keyObjectID))) {

			// 先頭キーフィールドにヌル値が
			// 記録されているオブジェクトは
			// 存在しない…

			checkMemoryExhaust(leafPage);

			return FileCommon::ObjectID::Undefined;
		}

		// 最終オブジェクトの先頭キーフィールドに
		// ヌル値が記録されているので、ここに来た。

		resultObjectID =
			this->getLastObjectID(FileInfo_,
								  AttachNodePages_,
								  false); // バリューオブジェクトの
								          // オブジェクトIDを取得する
	}

	checkMemoryExhaust(leafPage);

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchBySpanConditionRev --
//		先頭キーフィールドの値が検索条件で指定された範囲内の
//		オブジェクトを検索する
//
//	NOTES
//	先頭キーフィールドの値が検索条件で指定された範囲内の
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	Btree::FileInformation&	FileInfo_
//		ファイル管理情報への参照
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::ValueFile*		ValueFile_
//		バリューファイル記述子
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		検索条件に一致するオブジェクトのオブジェクトID。
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedを返す。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchBySpanConditionRev(FileInformation&	FileInfo_,
							   PageVector&		AttachNodePages_,
							   ValueFile*		ValueFile_,
							   PageVector&		AttachValuePages_) const
{
	//
	// 範囲指定ならば、
	// 検索開始条件の比較演算子がGreaterThanかGreaterThanEqualsで
	// 検索終了条件の比較演算子がLessThanかLessThanEqualsのはず。
	// 

	; _SYDNEY_ASSERT(
		*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::GreaterThan ||
		*this->m_SearchHint.m_StartOperatorArray ==
		LogicalFile::TreeNodeInterface::GreaterThanEquals);

	; _SYDNEY_ASSERT(
		this->m_SearchHint.m_StopOperator ==
		LogicalFile::TreeNodeInterface::LessThan ||
		this->m_SearchHint.m_StopOperator ==
		LogicalFile::TreeNodeInterface::LessThanEquals);

	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchBySpanConditionRevSimpleKey(FileInfo_,
													   AttachNodePages_,
													   ValueFile_,
													   AttachValuePages_);
	}

	ModUInt64	resultObjectID = FileCommon::ObjectID::Undefined;

	if (*(this->m_cFileParameter.m_KeyFieldSortOrderArray + 1) ==
		FileParameter::SortOrder::Ascending)
	{
		// 先頭キーフィールドのソート順は昇順…

		//
		// まずは、検索終了条件と一致するオブジェクトを検索する。
		//

		bool	containEquals =
			(this->m_SearchHint.m_StopOperator ==
			 LogicalFile::TreeNodeInterface::LessThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

		resultObjectID =
			this->searchBySingleLessThanRev(FileInfo_,
											AttachNodePages_,
											ValueFile_,
											AttachValuePages_,
											containEquals,
											SearchHint::CompareTarget::Stop,
											&keyInfoLeafPageID,
											&keyInfoIndex,
											&keyObjectID);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索終了条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		//
		// 検索開始条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		int	compareResult =
			this->compareToTopSearchCondition(
				keyInfoLeafPage,
				AttachNodePages_,
				keyObjectID,
				SearchHint::CompareTarget::Start);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult > 0)
		{
			// 検索開始条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			*this->m_SearchHint.m_StartOperatorArray ==
			LogicalFile::TreeNodeInterface::GreaterThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}
	else
	{
		// 先頭キーフィールドのソート順は降順…

		//
		// まずは、検索開始条件と一致するオブジェクトを検索する
		//

		bool	containEquals =
			(*this->m_SearchHint.m_StartOperatorArray ==
			 LogicalFile::TreeNodeInterface::GreaterThanEquals);

		PhysicalFile::PageID	keyInfoLeafPageID =
			PhysicalFile::ConstValue::UndefinedPageID;

		ModUInt32	keyInfoIndex = ModUInt32Max;

		ModUInt64	keyObjectID = FileCommon::ObjectID::Undefined;

		resultObjectID =
			this->searchBySingleGreaterThanRev(FileInfo_,
											   AttachNodePages_,
											   ValueFile_,
											   AttachValuePages_,
											   containEquals,
											   &keyInfoLeafPageID,
											   &keyInfoIndex,
											   &keyObjectID);

		if (resultObjectID == FileCommon::ObjectID::Undefined)
		{
			// 検索開始条件と一致するオブジェクトが
			// ファイル内に存在しなかった…

			return resultObjectID;
		}

		; _SYDNEY_ASSERT(
			keyInfoLeafPageID != PhysicalFile::ConstValue::UndefinedPageID);

		; _SYDNEY_ASSERT(keyInfoIndex != ModUInt32Max);

		; _SYDNEY_ASSERT(
			keyObjectID != FileCommon::ObjectID::Undefined &&
			keyObjectID != 0);

		//
		// 検索終了条件とも一致するかチェックする。
		//

		PhysicalFile::Page*	keyInfoLeafPage =
			File::attachPage(this->m_pTransaction,
							 this->m_pPhysicalFile,
							 keyInfoLeafPageID,
							 this->m_FixMode,
							 this->m_CatchMemoryExhaust,
							 AttachNodePages_);

		int	compareResult =
			this->compareToTopSearchCondition(
				keyInfoLeafPage,
				AttachNodePages_,
				keyObjectID,
				SearchHint::CompareTarget::Stop);

		checkMemoryExhaust(keyInfoLeafPage);

		if (compareResult > 0)
		{
			// 検索終了条件とは一致しなかった…

			// ということは、ファイル内に
			// 指定範囲内のオブジェクトは存在しない。

			return FileCommon::ObjectID::Undefined;
		}

		if (compareResult == 0 &&
			this->m_SearchHint.m_StopOperator ==
			LogicalFile::TreeNodeInterface::LessThan)
		{
			// 同上

			return FileCommon::ObjectID::Undefined;
		}
	}

	return resultObjectID;
}

//
//	FUNCTION private
//	Btree::File::searchByMultiConditionRev --
//		複数キーフィールドの値が検索条件で指定された複合条件と一致する
//		オブジェクトを検索する
//
//	NOTES
//	複数キーフィールドの値が検索条件で指定された複合条件と一致する
//	オブジェクトを検索する。
//	オブジェクト挿入ソート順の逆順でオブジェクトを返す場合に、呼び出される。
//
//	ARGUMENTS
//	const ModUInt32				TreeDepth_
//		現在の木の深さ
//	const PhysicalFile::PageID	RootNodePageID_
//		ルートノードページの物理ページ識別子
//	Btree::PageVector&			AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	ModUInt64
//		いずれの検索対象キーフィールドの値も、
//		そのキーフィールドに対する検索条件と一致するオブジェクトのID
//		ファイル内に該当オブジェクトが存在しない場合には、
//		FileCommon::ObjectID::Undefinedとなる。
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::searchByMultiConditionRev(
	const ModUInt32				TreeDepth_,
	const PhysicalFile::PageID	RootNodePageID_,
	PageVector&					AttachNodePages_) const
{
	if (this->m_cFileParameter.m_KeyPosType == KeyPosType::KeyInfo)
	{
		return this->searchByMultiConditionRevSimpleKey(TreeDepth_,
														RootNodePageID_,
														AttachNodePages_);
	}

	PhysicalFile::Page*	leafPage =
		this->searchLeafPageForMultiCondition(TreeDepth_,
											  RootNodePageID_,
											  AttachNodePages_);

	if (leafPage == 0)
	{
		return FileCommon::ObjectID::Undefined;
	}

	PhysicalFile::PageID	leafPageID = leafPage->getID();

	bool	match = false;

	NodePageHeader	leafPageHeader(this->m_pTransaction,
								   leafPage,
								   true); // リーフページ

	int	keyInfoIndex =
		this->getKeyInformationIndexForMultiCondition(
			leafPage,
			leafPageHeader.readUseKeyInformationNumber(),
			AttachNodePages_,
			true, // リーフページ
			match);

	if (match == false)
	{
		checkMemoryExhaust(leafPage);

		return FileCommon::ObjectID::Undefined;
	}

	KeyInformation	keyInfo(this->m_pTransaction,
							leafPage,
							keyInfoIndex,
							true); // リーフページ

	bool	isEOF = true;

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

		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

		if (this->compareToMultiSearchCondition(leafPage,
												AttachNodePages_,
												keyObjectID)
			!= 0)
		{
			this->assignPrevKeyInformation(leafPage,
										   AttachNodePages_,
										   leafPageHeader,
										   keyInfo);

			if (leafPage->getID() != leafPageID)
			{
				leafPageID = leafPage->getID();

				leafPageHeader.resetPhysicalPage(leafPage);
			}

			isEOF = false;

			break;
		}
	}

	if (isEOF)
	{
		leafPage = File::attachPage(this->m_pTransaction,
									this->m_pPhysicalFile,
									leafPageID,
									this->m_FixMode,
									this->m_CatchMemoryExhaust,
									AttachNodePages_);

		leafPageHeader.resetPhysicalPage(leafPage);
	}

	ModUInt64	resultObjectID = keyInfo.readValueObjectID();

	; _SYDNEY_ASSERT(resultObjectID != FileCommon::ObjectID::Undefined);

	if (this->m_SearchHint.m_ExistSeparateKey ||
		this->m_SearchHint.m_LastStartOperatorIsEquals == false)
	{
		ModUInt64	keyObjectID = keyInfo.readKeyObjectID();

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
//	Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
