// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeFile.cpp --
// 
// Copyright (c) 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
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
#include "KdTree/KdTreeFile.h"

#include "KdTree/Allocator.h"
#include "KdTree/DoSearch.h"
#include "KdTree/KdTreeIndex.h"
#include "KdTree/KdTreeIndexSet.h"
#include "KdTree/MergeReserve.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Os/Math.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"

#include "Schema/File.h"

#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// パス
	Os::Path _cMaster("Master");
	Os::Path _cInfo("Info");

	// マージ起動の閾値
	Common::Configuration::ParameterInteger
	_cMergeCountThreshold("KdTree_MergeCountThreshold", 100000);

	// マージ時の削除単位(一回の削除動作で何件削除するか)
	Common::Configuration::ParameterInteger
	_cMergeExpungeUnit("KdTree_UnitMergeExpungeCount", 1000);

	// マージ時の挿入単位(一回の挿入動作で何ページ分挿入するか)
	Common::Configuration::ParameterInteger
	_cMergeInsertUnit("KdTree_UnitMergeInsertPageCount", 5);

	//
	//	CLASS
	//	_$$::_AutoPage
	//
	class _AutoPage
	{
	public:
		_AutoPage(KdTreeFile* pFile_)
			: m_pFile(pFile_)
			{}
		~_AutoPage()
			{
				if (m_pFile) m_pFile->recoverAllPages();
			}
		void flush()
			{
				if (m_pFile) m_pFile->flushAllPages();
			}

	private:
		KdTreeFile* m_pFile;
	};

	// バッチインサート用
	class _DirectionForBatch : public KdTreeIndex::Direction
	{
	public:
		_DirectionForBatch(const Trans::Transaction& trans_)
			: m_trans(trans_) {}

		bool isAbort() const { return m_trans.isCanceledStatement(); }

	private:
		const Trans::Transaction& m_trans;
	};

	// マージ用
	class _DirectionForMerge : public KdTreeIndex::Direction
	{
	public:
		_DirectionForMerge(const Common::Thread& thread_)
			: m_thread(thread_) {}

		bool isAbort() const { return m_thread.isAborted(); }

	private:
		const Common::Thread& m_thread;
	};
	
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::KdTreeFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
KdTreeFile::KdTreeFile(FileID& cFileID_)
	: MultiFile(cFileID_, cFileID_.getPath()),
	  m_pInfoFile(0), m_pDataFile(0), m_pIndexFile(0),
	  m_pSmallDataFile1(0), m_pSmallDataFile2(0),
	  m_bBatch(false)
{
	attach();
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::~KdTreeFile -- デストラクタ
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
KdTreeFile::~KdTreeFile()
{
	if (isOpened()) close();
	detach();
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::setBatchMode -- バッチモードに設定する
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
KdTreeFile::setBatchMode()
{
	m_bBatch = true;

	SydMessage << "Start KDTree Batch Insert. ("
			   << m_cFileID.getPath() << ")" << ModEndl;
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		対応指示
//	Admin::Verification::Progress& cProgress_
//		経過報告
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::verify(const Trans::Transaction& cTransaction_,
				   const Admin::Verification::Treatment::Value eTreatment_,
				   Admin::Verification::Progress& cProgress_)
{
	// KD-Treeの整合性検査は、とりあえず、
	// 各ファイルの全データにアクセスできるかだけを検査する

	m_pInfoFile->verify(cTransaction_, eTreatment_, cProgress_);
	m_pDataFile->verify(cTransaction_, eTreatment_, cProgress_);
	m_pIndexFile->verify(cTransaction_, eTreatment_, cProgress_);
	m_pSmallDataFile1->verify(cTransaction_, eTreatment_, cProgress_);
	m_pSmallDataFile2->verify(cTransaction_, eTreatment_, cProgress_);
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::close -- クローズする
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
KdTreeFile::close()
{
	if (m_bBatch)
	{
		// オープンし直すので、すべてのページを確定する

		flushAllPages();

		SydMessage << "KDTree: Data Insert Done." << ModEndl;

		try
		{
			//【注意】	データファイルからエントリをロードするとき、
			//			マルチスレッドで実行される
			//			データファイル自体はスレッド分コピーされるが、
			//			Version::Page は共有される
			//			Version::Page は ReadWrite アクセスの場合、
			//			複数のスレッドから同時にアクセスされることを
			//			想定していないので、ReadWrite のままでは正しく動作しない
			//			そのため、データファイルを一旦 close して、
			//			ReadOnly で open し直す
			
			m_pDataFile->close();
			m_pDataFile->open(*m_pTransaction, Buffer::Page::FixMode::ReadOnly);

			// 索引を作成する

			_DirectionForBatch direction(*m_pTransaction);
			KdTreeIndexSet::create(*m_pTransaction,
								   m_cFileID, *m_pDataFile, *m_pIndexFile,
								   direction);
		}
		catch (...)
		{
			recoverAllPages();
			_SYDNEY_RETHROW;
		}
		
		// ファイルを確定する

		flushAllPages();

		SydMessage << "End KDTree Batch Insert." << ModEndl;

		m_bBatch = false;
	}

	MultiFile::close();
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const ModVector<float>& vecValue_
//		多次元ベクトル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::insert(ModUInt32 uiRowID_,
				   const ModVector<float>& vecValue_)
{
	// KD-Tree索引セットを得る
	KdTreeIndexSet* pIndexSet = KdTreeIndexSet::attach(*m_pTransaction,
													   m_cFileID);
	
	// 登録するために、エントリに変換する
	Entry* pEntry = allocateEntry(uiRowID_, vecValue_);
	
	try
	{
		if (m_bBatch)
		{
			// バッチインサート
			// メインのデータファイルに登録し、close 時に索引を作成する
			
			// データファイルに登録する
			m_pDataFile->insert(*pEntry);
		}
		else
		{
			// 通常のインサート
			// 差分索引と差分データファイルに登録する

			// 差分データファイルを得る
			BtreeDataFile* pSmallFile = attachSmall();
			pSmallFile->insert(*pEntry);

			// 差分索引を得る
			KdTreeIndex* pSmallIndex = allocateLog(pIndexSet);
			if (pSmallIndex->isEmpty())
			{
				// 空なら作成
				pSmallIndex->create(*pSmallFile);
			}
			else
			{
				// 空じゃないのでインサート
				pSmallIndex->insert(pEntry);
			}

			// マージの閾値に達していたらマージを起動
			if (pSmallFile->getCount() >=
				static_cast<ModUInt32>(_cMergeCountThreshold.get()))
			{
				// マージを要求する
				MergeReserve::pushBack(m_cFileID.getLockName(),
									   MergeReserve::Type::Merge);
			}
		}
	}
	catch (...)
	{
		// 解放する
		freeEntry(pEntry);
		_SYDNEY_RETHROW;
	}

	// 解放する
	freeEntry(pEntry);
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::expunge(ModUInt32 uiRowID_)
{
	// KD-Tree索引セットを得る
	KdTreeIndexSet* pIndexSet = KdTreeIndexSet::attach(*m_pTransaction,
													   m_cFileID);
	
	// 通常の削除
	// 差分索引と差分データファイルから削除する
	//
	// 差分データファイルからの削除は、
	// 差分データファイル自体に挿入されているものは削除されるが、
	// 存在していないものは、削除フラグファイルに挿入される

	// 差分データファイルを得る
	BtreeDataFile* pSmallFile = attachSmall();
	pSmallFile->expunge(uiRowID_);

	// 差分索引を得る
	KdTreeIndex* pSmallIndex = allocateLog(pIndexSet);
	if (pSmallIndex->isEmpty())
	{
		// 空なら作成
		pSmallIndex->create(*pSmallFile);
	}
	else
	{
		// 空じゃないので削除
		pSmallIndex->expunge(uiRowID_);
	}
	
	// マージの閾値に達していたらマージを起動
	if (pSmallFile->getExpungedEntryCount() >=
		static_cast<ModUInt32>(_cMergeCountThreshold.get()))
	{
		// マージを要求する
		MergeReserve::pushBack(m_cFileID.getLockName(),
							   MergeReserve::Type::Merge);
	}
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::nnsearch -- 最近傍検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModVector<float> >& vecCondition_
//		クエリ
//	ModVector<ModVector<ModPair<ModUInt32, double> > >& vecResult_
//		検索結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::nnsearch(
	const ModVector<ModVector<float> >& vecCondition_,
	Node::TraceType::Value eTraceType_,
	int iMaxCalculateCount_,
	ModSize uiLimit_,
	ModVector<ModVector<ModPair<ModUInt32, double> > >& vecResult_)
{
	// KD-Tree索引セットを得る
	KdTreeIndexSet* pIndexSet = KdTreeIndexSet::attach(*m_pTransaction,
													   m_cFileID);

	// 個々の索引を得る
	const KdTreeIndex* pMain = pIndexSet->attachMain(*m_pTransaction);
	// 小索引は、入れ替わるので専用のメソッドが必要
	const KdTreeIndex* pSmall = attachLog(pIndexSet);
	const KdTreeIndex* pSmallForMerge = attachLogForMerge(pIndexSet);

	// 削除リストを得る
	Common::BitSet cMainBitSet;
	Common::BitSet cSmall1BitSet;
	
	if (pMain)
	{
		BtreeDataFile* pSmallFile = 0;
		
		// ファイルを得る
		if (pSmallForMerge)
			// マージ中
			pSmallFile = attachSmallForMerge();
		else
			// マージ中ではない
			pSmallFile = attachSmall();

		// 削除リストを得る
		pSmallFile->getExpungedEntry(cMainBitSet);
	}

	const KdTreeIndex* pSmall1 = 0; // 古い方
	const KdTreeIndex* pSmall2 = 0; // 新しい方

	if (pSmallForMerge)
	{
		// マージ中なので、小索引１の削除リストも作成する

		// ファイルを得る
		BtreeDataFile* pSmallFile = attachSmall();

		// 削除リストを得る
		pSmallFile->getExpungedEntry(cSmall1BitSet);

		// 小索引２の削除リストの内容をメイン索引の削除リストにも加える
		cMainBitSet |= cSmall1BitSet;

		pSmall1 = pSmallForMerge;
		pSmall2 = pSmall;
	}
	else
	{
		// マージ中ではない

		pSmall1 = pSmall;
		pSmall2 = 0;
	}

	// 検索状態クラス
	//
	// KD-Treeの場合、近いものは近いほど少ないコストで
	// 見つけることができ、近くないものを検索するとコストが大きくなる
	// KD-Treeの検索において、最悪なのは、どのデータにも近くない
	// データで検索することである。言ってみれば、多くのコストをかけて、
	// どのデータにも近くなかったということを調べていることになる
	// それを回避するため、距離計算回数制限に達した場合、
	// 途中で検索をあきらめるようになっている
	// よって、距離計算回数制限の数値は、検索速度に大きく影響する
	// このような性質があるので、今回のような索引を分割していると、
	// 見つけるべきデータが挿入されている索引は、少ないコストで
	// 検索することができるが、挿入されているない索引の検索コストは
	// 大きくなっていまう。ほとんどのデータはメイン索引に
	// 入っているので、メイン索引で見つかる可能性が高く、
	// 小索引では見つかる可能性は低い
	// そのため、小索引の検索コストの方が大きくなってしまい、
	// 全体の検索速度に大きく影響していまう
	// そこで、小索引の距離計算回数制限をメイン索引の半分にすることで、
	// その影響を小さくする

	Node::Status cMainStatus(eTraceType_, iMaxCalculateCount_,
							 &cMainBitSet);
	Node::Status cSmall1Status(eTraceType_, iMaxCalculateCount_ / 2,
							   &cSmall1BitSet);
	Node::Status cSmall2Status(eTraceType_, iMaxCalculateCount_ / 2,
							   0);
	if (uiLimit_ != 0)
	{
		cMainStatus.setLimit(uiLimit_);
		cSmall1Status.setLimit(uiLimit_);
		cSmall2Status.setLimit(uiLimit_);
	}
	
	// OpenMPで並列処理する

	DoSearch search(pMain, pSmall1, pSmall2,
					vecCondition_,
					&cMainStatus, &cSmall1Status, &cSmall2Status);
	search.run();

	vecResult_ = search.getResult();
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::openForMerge -- マージ用のオープン
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
KdTreeFile::openForMerge(const Trans::Transaction& cTransaction_)
{
	//【注意】	実際のファイルのオープンは、必要に応じて行われる

	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(cTransaction_),
						   m_cFileID.getLockName());

	// トランザクションを保存するため、自クラスをオープンする
	File::open(cTransaction_, Buffer::Page::FixMode::Write |
			   Buffer::Page::FixMode::Discardable);

	// 更新するのは、InfoFile だけなので、それ以外のファイルはオープンしない
	m_pInfoFile->open(cTransaction_, Buffer::Page::FixMode::Write |
					  Buffer::Page::FixMode::Discardable);
	
	try
	{
		// attachしたページを解放する
		_AutoPage cAuto(this);

		// 小索引を入れ替え、マージ中にする
		m_pInfoFile->flip();

		cAuto.flush();
	}
	catch (...)
	{
		// ファイルをクローズする
		m_pInfoFile->close();
		_SYDNEY_RETHROW;
	}
		
	// ファイルをクローズする
	m_pInfoFile->close();
}

//
//	FUNCTION public
//	KdTree::KdTreeFile::closeForMerge -- マージ用のクローズ
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
KdTreeFile::closeForMerge()
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());
	
	// 更新するのは、InfoFile だけなので、それ以外のファイルはオープンしない
	m_pInfoFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
					  Buffer::Page::FixMode::Discardable);

	try
	{
		// attachしたページを解放する
		_AutoPage cAuto(this);

		// マージ終了に設定する
		m_pInfoFile->mergeDone();

		cAuto.flush();
	}
	catch (...)
	{
		// すべてのファイルをクローズする
		close();
		_SYDNEY_RETHROW;
	}

	// すべてのファイルをクローズする
	close();
}

//
//	FUNCTION public
//	KdTree::KdTree::merge -- マージ
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Thread& cThread_
//		中断要求をチェックするためのスレッドオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::merge(const Common::Thread& cThread_)
{
	// 削除を反映
	reflectDeletedData(cThread_);

	// 挿入を反映
	reflectInsertedData(cThread_);

	// 索引を作成
	makeIndex(cThread_);

	// 差分ファイルをクリアする
	clearSmallFile();
}
		
//
//	FUNCTION private
//	KdTree::KdTreeFile::attach -- ファイルをattachする
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
KdTreeFile::attach()
{
	ModAutoPointer<InfoFile> pInfoFile
		= new InfoFile(m_cFileID, getPath());
	ModAutoPointer<VectorDataFile> pDataFile
		= new VectorDataFile(m_cFileID,
							 Os::Path(getPath()).addPart(_cMaster));
	ModAutoPointer<IndexFile> pIndexFile
		= new IndexFile(m_cFileID);
	ModAutoPointer<BtreeDataFile> pSmall1
		= new BtreeDataFile(
			m_cFileID,
			Os::Path(getPath()).addPart(FileID::getSmallPath1()));
	ModAutoPointer<BtreeDataFile> pSmall2
		= new BtreeDataFile(
			m_cFileID,
			Os::Path(getPath()).addPart(FileID::getSmallPath2()));

	// MultiFileに登録する
	reserveSubFile(5);
	pushBackSubFile(pInfoFile.get());
	pushBackSubFile(pDataFile.get());
	pushBackSubFile(pIndexFile.get());
	pushBackSubFile(pSmall1.get());
	pushBackSubFile(pSmall2.get());

	// メンバー変数に設定する
	m_pInfoFile = pInfoFile.release();
	m_pDataFile = pDataFile.release();
	m_pIndexFile = pIndexFile.release();
	m_pSmallDataFile1 = pSmall1.release();
	m_pSmallDataFile2 = pSmall2.release();
}

//
//	FUNCTION private
//	KdTree::KdTreeFile::detach -- ファイルをdetachする
//
// 	NOTES
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
KdTreeFile::detach()
{
	clearSubFile();

	delete m_pInfoFile, m_pInfoFile = 0;
	delete m_pDataFile, m_pDataFile = 0;
	delete m_pIndexFile, m_pIndexFile = 0;
	delete m_pSmallDataFile1, m_pSmallDataFile1 = 0;
	delete m_pSmallDataFile2, m_pSmallDataFile2 = 0;
}

//
//	FUNCTION private
//	KdTree::KdTreeFile::allocateEntry -- エントリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const ModVector<float>& vecValue_
//		多次元ベクトル
//
//	RETURN
//	KdTree::Entry*
//		エントリ
//
//	EXCEPTIONS
//
Entry*
KdTreeFile::allocateEntry(ModUInt32 uiRowID_,
						  const ModVector<float>& vecValue_)
{
	Entry* pEntry = syd_reinterpret_cast<Entry*>(
		Os::Memory::allocate(Entry::calcSize(m_cFileID.getDimension())));
	
	pEntry->m_uiID = uiRowID_;
	pEntry->m_iDimensionSize = m_cFileID.getDimension();

	// 多次元ベクトル部分をコピーする
	// FileIDの要素数と異なっていてもエラーにはしない

	ModSize dim = m_cFileID.getDimension();
	ModSize size = (vecValue_.getSize() < dim) ? vecValue_.getSize() : dim;
	if (size)
		Os::Memory::copy(pEntry->m_pValue, &(*vecValue_.begin()),
						 sizeof(float) * size);

	return pEntry;
}

//
//	FUNCTION private
//	KdTreeFile::freeEntry -- エントリを解放する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Entry* pEntry_
//		解放するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::freeEntry(Entry* pEntry_)
{
	void* p = pEntry_;
	Os::Memory::free(p);
}

//
//	FUNCTION private
//	KdTreeFile::allocateLog -- 更新のためにエグゼキュータ側の小索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::KdTreeIndexSet* pIndexSet_
//		索引セット
//
//	RETURN
//	KdTree::KdTreeIndex*
//	   	小索引
//
//	EXCEPTIONS
//
KdTreeIndex*
KdTreeFile::allocateLog(KdTreeIndexSet* pIndexSet_)
{
	return (m_pInfoFile->getIndex() == 0) ?
		pIndexSet_->allocateLog1(*m_pTransaction)
		: pIndexSet_->allocateLog2(*m_pTransaction);
}

//
//	FUNCTION private
//	KdTreeFile::attachLog -- エグゼキュータ側の小索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::KdTreeIndexSet* pIndexSet_
//		索引セット
//
//	RETURN
//	KdTree::KdTreeIndex*
//		小索引
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeFile::attachLog(KdTreeIndexSet* pIndexSet_)
{
	return (m_pInfoFile->getIndex() == 0) ?
		pIndexSet_->attachLog1(*m_pTransaction)
		: pIndexSet_->attachLog2(*m_pTransaction);
}

//
//	FUNCTION private
//	KdTreeFile::attachLogForMerge -- マージ側の小索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::KdTreeIndexSet* pIndexSet_
//		索引セット
//
//	RETURN
//	KdTree::KdTreeIndex*
//		小索引
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeFile::attachLogForMerge(KdTreeIndexSet* pIndexSet_)
{
	if (m_pInfoFile->isProceeding() == false)
	{
		// マージ中じゃないので、マージ側はない

		return 0;
	}
	
	return (m_pInfoFile->getIndex() == 0) ?
		pIndexSet_->attachLog2(*m_pTransaction)
		: pIndexSet_->attachLog1(*m_pTransaction);
}

//
//	FUNCTION private
//	KdTreeFile::attachSmall -- エグゼキュータ側の差分データファイルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::BtreeDataFile*
//		差分データファイル
//
//	EXCEPTIONS
//
BtreeDataFile*
KdTreeFile::attachSmall()
{
	return (m_pInfoFile->getIndex() == 0) ?
		m_pSmallDataFile1 : m_pSmallDataFile2;
}

//
//	FUNCTION private
//	KdTreeFile::attachSmallForMerge -- マージ側の差分データファイルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::BtreeDataFile*
//		差分データファイル
//
//	EXCEPTIONS
//
BtreeDataFile*
KdTreeFile::attachSmallForMerge()
{
	return (m_pInfoFile->getIndex() == 0) ?
		m_pSmallDataFile2 : m_pSmallDataFile1;
}

//
//	FUNCTION private
//	KdTreeFile::reflectDeletedData
//		-- 差分ファイルの削除データをメインファイルに反映する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Thread& cThread_
//		中断監視のためのスレッドクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::reflectDeletedData(const Common::Thread& cThread_)
{
	// マージ側の差分ファイル
	BtreeDataFile* pSmall = 0;

	{
		// ラッチする
		Trans::AutoLatch latch(const_cast<Trans::Transaction&>(
								   *m_pTransaction),
							   m_cFileID.getLockName());

		// attachしたページを解放する
		_AutoPage cAuto(this);

		// オープンする
		m_pInfoFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		m_pDataFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		
		// マージ側の差分ファイルを得る
		pSmall = attachSmallForMerge();

		// ファイルをオープンする
		pSmall->open(*m_pTransaction, Buffer::Page::FixMode::ReadOnly);

		cAuto.flush();
	}
	
	// 削除した行のセット
	Common::BitSet cExpungeSet;

	try
	{
		{
			// 削除されたものを得る
		
			// ラッチする
			Trans::AutoLatch latch(const_cast<Trans::Transaction&>(
									   *m_pTransaction),
								   m_cFileID.getLockName());
			// attachしたページを解放する
			_AutoPage cAuto(this);

			// 削除済みのエントリの一覧を取得する
			pSmall->getExpungedEntry(cExpungeSet);

			// 中断を確認する
			if (cThread_.isAborted())
				_SYDNEY_THROW0(Exception::Cancel);

			cAuto.flush();
		}

		{
			// 削除する

			Common::BitSet::ConstIterator i = cExpungeSet.begin();

			while (i != cExpungeSet.end())
			{
				// ラッチする
				Trans::AutoLatch latch(
					const_cast<Trans::Transaction&>(*m_pTransaction),
					m_cFileID.getLockName());
				
				// attachしたページを解放する
				_AutoPage cAuto(this);

				// 1000件づつ削除する
				int u = _cMergeExpungeUnit.get();
				for (int n = 0; n < u; ++n)
				{
					if (i == cExpungeSet.end())
						break;

					// マージが中断した場合など、
					// すでに削除済みの場合があるので、
					// 検索して確認する

					ModUInt32 rowid = *i;

					if (m_pDataFile->test(rowid) == true)

						// 登録されているので、削除する

						m_pDataFile->expunge(*i);

					++i;
				}
			
				// 中断を確認する
				if (cThread_.isAborted())
					_SYDNEY_THROW0(Exception::Cancel);

				cAuto.flush();
			}
		}
	}
	catch (...)
	{
		// ファイルをクローズする
		pSmall->close();
		m_pInfoFile->close();
		m_pDataFile->close();

		_SYDNEY_RETHROW;
	}

	// ファイルをクローズする
	pSmall->close();
	m_pInfoFile->close();
	m_pDataFile->close();
}

//
//	FUNCTION private
//	KdTreeFile::reflectInsertedData
//		-- 差分ファイルの挿入データをメインファイルに反映する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Thread& cThread_
//		中断監視のためのスレッドクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::reflectInsertedData(const Common::Thread& cThread_)
{
	// マージ側の差分ファイル
	BtreeDataFile* pSmall = 0;
	
	{
		// ラッチする
		Trans::AutoLatch latch(const_cast<Trans::Transaction&>(
								   *m_pTransaction),
							   m_cFileID.getLockName());

		// attachしたページを解放する
		_AutoPage cAuto(this);

		// オープンする
		m_pInfoFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		m_pDataFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		
		// マージ側の差分ファイルを得る
		pSmall = attachSmallForMerge();

		// ファイルをオープンする
		pSmall->open(*m_pTransaction, Buffer::Page::FixMode::ReadOnly);

		cAuto.flush();
	}

	try
	{
		// 挿入する

		Allocator cAllocator(m_cFileID.getDimension());
		PhysicalFile::PageID uiCurrentPageID = 0;

		while (uiCurrentPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// ラッチする
			Trans::AutoLatch latch(
				const_cast<Trans::Transaction&>(*m_pTransaction),
				m_cFileID.getLockName());
				
			// attachしたページを解放する
			_AutoPage cAuto(this);

			if (uiCurrentPageID == 0)
			{
				// 初めて
				uiCurrentPageID = pSmall->getNextPageID(uiCurrentPageID);
			}

			// 5ページごと処理する

			int u = _cMergeInsertUnit.get();
			for (int i = 0; i < u; ++i)
			{
				if (uiCurrentPageID
					== PhysicalFile::ConstValue::UndefinedPageID)
					// 終了
					break;

				// 1ページ分のデータを得る
				Common::LargeVector<Entry*> vecpEntry;
				pSmall->getPageData(uiCurrentPageID, 
									cAllocator,
									vecpEntry);

				Common::LargeVector<Entry*>::Iterator j = vecpEntry.begin();
				for (; j != vecpEntry.end(); ++j)
				{
					// マージが中断した場合など、
					// すでに削除済みの場合があるので、
					// 検索して確認する

					if (m_pDataFile->test((*j)->getID()) == false)
					
						// 登録されていないので登録する
						
						m_pDataFile->insert(*(*j));
				}

				// 次のページを得る
				uiCurrentPageID = pSmall->getNextPageID(uiCurrentPageID);
			}
			
			// 中断を確認する
			if (cThread_.isAborted())
				_SYDNEY_THROW0(Exception::Cancel);

			cAuto.flush();
		}
	}
	catch (...)
	{
		// ファイルをクローズする
		pSmall->close();
		m_pInfoFile->close();
		m_pDataFile->close();

		_SYDNEY_RETHROW;
	}
	
	// ファイルをクローズする
	pSmall->close();
	m_pInfoFile->close();
	m_pDataFile->close();
}

//
//	FUNCTION private
//	KdTreeFile::makeIndex
//		-- KD-Tree索引を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Thread& cThread_
//		中断監視のためのスレッドクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeFile::makeIndex(const Common::Thread& cThread_)
{
	// 必要なファイルを適切なモードでオープンする
	//
	//【注意】	DataFile のオープンモードは ReadOnly でなければならない
	///			索引作成はマルチスレッドで実行される
	//			DataFile自体はスレッドこごにコピーされるが、
	//			下位のVersion層はコピーされず、複数のスレッドで実行される
	//			しかし、Version層はオープンモードが Write の場合、
	//			複数のスレッドで同時にアクセスされることを想定していない
	//			そのため、ReadOnly でなければならない

	m_pDataFile->open(*m_pTransaction, Buffer::Page::FixMode::ReadOnly);
	m_pIndexFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
					   Buffer::Page::FixMode::Discardable);

	try
	{
		//【注意】	マージ中は同期処理も停止しており、
		//			通常のクエリではメインデータファイルを参照・更新
		//			することはないので、ラッチしない
		//
		//			本当にラッチしなくて大丈夫なの？->たぶん大丈夫...
		
		// attachしたページを解放する
		_AutoPage cAuto(this);

		// 索引を作成する
		_DirectionForMerge direction(cThread_);
		KdTreeIndexSet::create(*m_pTransaction,
							   m_cFileID, *m_pDataFile, *m_pIndexFile,
							   direction);

		cAuto.flush();
	}
	catch (...)
	{
		// ファイルをクローズする
		m_pDataFile->close();
		m_pIndexFile->close();
		_SYDNEY_RETHROW;
	}
	
	// ファイルをクローズする
	m_pDataFile->close();
	m_pIndexFile->close();
}

//
//	FUNCTION private
//	KdTreeFile::clearSmallFile -- 差分ファイルをクリアする
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
KdTreeFile::clearSmallFile()
{
	// マージ側の差分ファイル
	BtreeDataFile* pSmall = 0;
	
	{
		// ラッチする
		Trans::AutoLatch latch(const_cast<Trans::Transaction&>(
								   *m_pTransaction),
							   m_cFileID.getLockName());

		// attachしたページを解放する
		_AutoPage cAuto(this);

		// オープンする
		m_pInfoFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		m_pDataFile->open(*m_pTransaction, Buffer::Page::FixMode::Write |
						  Buffer::Page::FixMode::Discardable);
		
		// マージ側の差分ファイルを得る
		pSmall = attachSmallForMerge();

		// ファイルをオープンする
		pSmall->open(*m_pTransaction, Buffer::Page::FixMode::Write |
					 Buffer::Page::FixMode::Discardable);

		cAuto.flush();
	}

	try
	{
		// ラッチする
		Trans::AutoLatch latch(
			const_cast<Trans::Transaction&>(*m_pTransaction),
			m_cFileID.getLockName());
				
		// attachしたページを解放する
		_AutoPage cAuto(this);

		// ファイルをクリアする
		pSmall->clear();

		cAuto.flush();
	}
	catch (...)
	{
		// ファイルをクローズする
		m_pInfoFile->close();
		m_pDataFile->close();
		pSmall->close();
		_SYDNEY_RETHROW;
	}

	// ファイルをクロースする
	m_pInfoFile->close();
	m_pDataFile->close();
	pSmall->close();
}
	
//
//	Copyright (c) 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
