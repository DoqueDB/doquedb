// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CriticalSectionManager.cpp -- クリティカルセクションマネージャ
// 
// Copyright (c) 2018, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyKernelVersion.h"

#include "ModException.h"
#include "ModTime.h"
#include "ModCharString.h"
#include "ModFile.h"
#include "ModOsDriver.h"
#include "Exception/Object.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/Message.h"
#include "Os/CriticalSectionManager.h"
#include "Os/Path.h"
#include "Exception/InvalidPath.h"

#include <fstream>
#include <boost/thread/mutex.hpp>

namespace {

	// 出力先のファイルパスを指定するパラメータ名

	const char* const _SystemParameter_PrintCriticalSection = "Os_PrintCriticalSection";
	
	// ハッシュ表のサイズ
	
	const ModUInt32 _iTableSize = 65000;
}

_TRMEISTER_USING
_TRMEISTER_OS_USING

//	FUNCTION public
//	Os::CriticalSectionManager::add -- クリティカルセクションオブジェクトをマネージャに登録する
//
//	NOTES
//		この関数はスレッドセーフ
//
//	ARGUMENTS
//		const CriticalSection* pCriticalSection
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
//  static
void
CriticalSectionManager::add(const CriticalSection* pCriticalSection)
{
	; _SYDNEY_ASSERT(pCriticalSection);

	boost::mutex::scoped_lock lock(getMutex());

	getInstance()->m_cCriticalSectionSet.insert(pCriticalSection);
}

//	FUNCTION public
//	Os::CriticalSectionManager::remove -- クリティカルセクションオブジェクトをマネージャから削除する
//
//	NOTES
//		この関数はスレッドセーフ。
//		基本的にはありえないが、もしマップに存在しないときは何もしない。
//
//	ARGUMENTS
//		const CriticalSection* pCriticalSection
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
//  static
void
CriticalSectionManager::remove(const CriticalSection* pCriticalSection)
{
	; _SYDNEY_ASSERT(pCriticalSection);

	boost::mutex::scoped_lock lock(getMutex());

	CriticalSectionManager* pManager = getInstance();
	pManager->m_cCriticalSectionSet.erase(pCriticalSection);
}


//	FUNCTION public
//	Os::CriticalSectionManager::printOut -- クリティカルセクション情報をログに書き出す
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
//
// static 
void
CriticalSectionManager::printOut()
{
	ModUnicodeString cPath;
	
	if(!loadOutputPath(cPath)) {

		// 出力先が未指定なのでその旨をシステムログに出力して終わる

		SydErrorMessage << "Cannot print out critical section log because the system parameter '"
						<< _SystemParameter_PrintCriticalSection << "' is not set." << ModEndl;
		
		return;
	}

	// 書き込む前に出力先のパスにアクセスできるか調べる
	
	checkLogPath(cPath);

	// 出力時刻を取得する
	
	ModCharString cTime = ModTime::getCurrentTime().getString(ModTrue);

	// この先はクリティカルセクションマップにアクセスするので排他制御する
	
	boost::mutex::scoped_lock lock(getMutex());

	getInstance()->printOut(cPath, cTime);
}


//	FUNCTION private
//	Os::CriticalSectionManager::getMutex -- クリティカルセクションマネージャを保護するミューテックスを得る
//
//	NOTES
//		C++11 以降ならこの関数はスレッドセーフ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		boost::mutex&
//		ミューテックスオブジェクト
//
//	EXCEPTIONS
//
//  static
boost::mutex&
CriticalSectionManager::getMutex()
{
	// 静的ローカル変数でインスタンスを初期化する

	static boost::mutex* pMutex = new boost::mutex();

	// この初期化は C++11 以降ならスレッドセーフ (Standard# N2660)
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm
	//
	// gcc4.3, icc11 以降で N2660 をサポート 	
	// https://gcc.gnu.org/projects/cxx-status.html#cxx11
	// https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler

	// 1. 初期化の注意
	// 実際には CriticalSection を使う静的変数の初期化時に本関数が呼び出されるため、
	// 初回呼び出しでレースコンディションになることはなく、スレッドセーフでなくても問題ないはず
	//
	// 2. 終了処理の注意
	// プログラム終了直前に、クリティカルセクションを使う静的変数のデストラクタから本関数は呼び出される。
	// 静的変数の初期化やデストラクトの順序は C++ の仕様上制御できないため、
	// pMutex は最後まで開放しない(意図的なメモリリーク)
	
	return *pMutex;
}

//	FUNCTION private
//	Os::CriticalSectionManager::getInstance -- クリティカルセクションマネージャの唯一のインスタンスを返す
//
//	NOTES
//		C++11以降ならこの関数はスレッドセーフ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		CriticalSectionManager*
//
//	EXCEPTIONS
//
//  static
CriticalSectionManager*
CriticalSectionManager::getInstance()
{
	// 静的ローカル変数でインスタンスを初期化する
	
	static CriticalSectionManager* pManager = new CriticalSectionManager(_iTableSize);

	// この初期化は C++11 以降ならスレッドセーフ (Standard# N2660)
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm
	//
	// gcc4.3, icc11 以降で N2660 をサポート 	
	// https://gcc.gnu.org/projects/cxx-status.html#cxx11
	// https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler

	// 1. 初期化の注意
	// 実際には CriticalSection を使う静的変数の初期化時に本関数が呼び出されるため、
	// 初回呼び出しでレースコンディションになることはなく、スレッドセーフでなくても問題ないはず
	//
	// 2. 終了処理の注意
	// プログラム終了直前に、クリティカルセクションを使う静的変数のデストラクタから本関数は呼び出される。
	// 静的変数の初期化やデストラクトの順序は C++ の仕様上制御できないため、
	// pManager は最後まで開放しない(意図的なメモリリーク)
	
	return pManager;
}


//	FUNCTION private
//	Os::CriticalSectionManager::loadOutputPath -- 出力先のパラメータを読み込む
//
//	NOTES
//		このメソッドは Common モジュールを利用するのでミューテックス内で呼び出さないこと
//		ファイルパスにマルチバイト文字が含まれる場合は UTF-8 のみ対応
//
//	ARGUMENTS
//		const ModUnicodeString& cstrPath
//		出力先ファイルパスの格納先
//
//	RETURN
//		bool
//		パラメータを読み込んだら true, パラメータが未設定なら false
//
//	EXCEPTIONS
//		ModException
//		Exception::Object
//
//  static
bool
CriticalSectionManager::loadOutputPath(ModUnicodeString& cstrPath /* output */)
{
	try {
		// 注意
		// Common::Configuration::ParameterMessage は内部で Os::CriticalSection を利用する

		Common::Configuration::ParameterMessage
			cParam(_SystemParameter_PrintCriticalSection);

		if(!cParam.isOutput()) {
			return false;
		}

		cstrPath = cParam.get();
	}
	catch (ModException& e)
	{
		SydErrorMessage << "Cannot load a system parameter: "
						<< _SystemParameter_PrintCriticalSection << ModEndl;
		ModErrorMessage << e << ModEndl;
		_SYDNEY_RETHROW;
	}
	catch (Exception::Object& e)
	{
		SydErrorMessage << "Cannot load a system parameter: "
						<< _SystemParameter_PrintCriticalSection << ModEndl;
		SydErrorMessage << e << ModEndl;
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		SydErrorMessage << "Unexpected error occured. Cannot load a system parameter: "
						<< _SystemParameter_PrintCriticalSection << ModEndl;
		_SYDNEY_RETHROW;
	}
	
	return true;
}

//	FUNCTION private
//	Os::CriticalSectionManager::checkLogPath -- 出力先のパスがアクセス可能か調べる
//
//	NOTES
//		アクセス不可の場合、例外が送出する。
//
//	ARGUMENTS
//		const ModUnicodeString& cstrPath
//		出力先ファイルパスの格納先
//
//	RETURN
//
//	EXCEPTIONS
//		Exception::InvalidPath パスが不正
//		ModOsErrorPermissionDenied  権限がない
//
// static
void
CriticalSectionManager::checkLogPath(const ModUnicodeString& cstrPath)
{
	// CriticalSection ログを実際に書き込む段階(CriticalSectionManager::printOut())では
	// 標準ライブラリしか利用できないので、TRMeister のシステムログにメッセージを書き込めない。
	// そこで、ここでログを書き込めるか事前にできるだけチェックして、
	// ダメならエラーメッセージをシステムログに出力する
	
	try {
		
		if (ModFile::doesExist(cstrPath) == ModTrue) {

			// 出力先のパスはすでに存在する

			if (ModOsDriver::File::isDirectory(cstrPath) == ModTrue) {

				// ファイルではなくディレクトリが指定されているのでエラーとする

				_TRMEISTER_THROW1(Exception::InvalidPath, cstrPath);
			}
			
			// 書き込み権限あるか検査する
			// 権限がなければ ModOsErrorPermissionDenied 例外が送出される
			
			ModOsDriver::File::access(cstrPath, ModOs::accessWrite);

			return;
		}

		//
		// 出力先が存在しない場合は、親ディレクトリの権限を検査する
		// 親ディレクトリが存在しない場合は、エラーとする
		//
		
		Os::Path cPath(cstrPath);
		Os::Path cParent;
		cPath.getParent(cParent);
		
		// 親ディレクトリにファイルを作成する権限があるか検査する
		// 親ディレクトリが存在しない、または権限がなければ ModOsErrorPermissionDenied 例外が飛ぶ
		
		ModOsDriver::File::access(cParent, ModOs::accessExecute | ModOs::accessWrite);
	}
	catch (Exception::InvalidPath& e)
	{
		SydErrorMessage << "Directory is specified for log file path: " << cstrPath << ModEndl;
		// 'invalid schema operation' など、こことは関係ないエラーメッセージがでるので例外のメッセージを出力しない
		// SydErrorMessage << e << ModEndl;
		_SYDNEY_RETHROW;
	}	   
	catch (Exception::Object& e)
	{
		SydErrorMessage << "Cannot access log file: " << cstrPath << ModEndl;
		SydErrorMessage << "Possible cause is invalid path, permission denied"
						<< " or the parent directory does not exist." << ModEndl;
		SydErrorMessage << e << ModEndl;
		_SYDNEY_RETHROW;
	}
	catch (ModException& e)
	{
		SydErrorMessage << "Cannot access log file: " << cstrPath << ModEndl;
		SydErrorMessage << "Possible cause is invalid path, permission denied"
						<< " or the parent directory does not exist." << ModEndl;
		ModErrorMessage << e << ModEndl;
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		SydErrorMessage << "Cannot access log file: " << cstrPath << ModEndl;
		SydErrorMessage << "Possible cause is invalid path, permission denied"
						<< " or the parent directory does not exist." << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Os::CriticalSectionManager::CriticalSectionManager -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModUInt32 iTableSize
//		ハッシュテーブルのサイズ
//
//	RETURN
//
//	EXCEPTIONS
//
// static 
// private
CriticalSectionManager::CriticalSectionManager(ModUInt32 iTableSize)
	:m_cCriticalSectionSet(iTableSize)
{
}


//	FUNCTION private
//	Os::CriticalSectionManager::printOut -- ログを書き出す
//
//	NOTES
//		呼び出し側で排他制御すること。
//		ログの出力はすべて標準ライブラリの関数で処理する。
//		IO関係でエラーが発生しても SydErrorMessage 等を使えないので、それをログに残すことはできない。
//
//	ARGUMENTS
//		const ModUnicodeString& cPath
//		出力先のファイルパス (UTF-8)
//
//		const ModCharString& cTime
//		出力時刻
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
CriticalSectionManager::printOut(const ModUnicodeString& cPath, const ModCharString& cTime)
{
	// 日本語のファイルパスは UTF-8 前提
	
	std::ofstream logfile(cPath.getString(), std::ios_base::app);

	if(!logfile) {

		// ファイルのオープンに失敗した
		// ログには残せない
		
		return;
	}
	
	// ヘッダー行を書き出す
	logfile << cTime.getString() << std::endl;
	logfile << "TRMeister " << SYD_KERNEL_VERSION << std::endl;
	logfile << "CriticalSection" << "\t" << "ThreadID(LWP)" << std::endl;
	logfile << "---" << std::endl;
	
	int locked = 0;
	
	CriticalSectionSet::iterator ite(m_cCriticalSectionSet.begin());
	for(; ite != m_cCriticalSectionSet.end(); ++ite) {

		const CriticalSection* pCriticalSection = *ite;

		; _SYDNEY_ASSERT(pCriticalSection);

		int ownerID = pCriticalSection->getOwnerID();
		
		if(!ownerID) {

			// ロックされていないのでスキップする
			
			continue;
		}

		++locked;

		// クリティカルセクションの情報を書き出す
		// 上のロック中かの検査後にロックが開放された場合はそれも出力される (スレッドIDは 0)
		
		logfile << std::hex << pCriticalSection << std::dec << "\t" << ownerID << std::endl;
	}

	if(!locked) {
		logfile << "(empty)" << std::endl;
	}

	logfile << "---" << std::endl;
	logfile << "locked: " << locked << std::endl;
	logfile << "total: " << m_cCriticalSectionSet.size() << std::endl;
	logfile << std::endl;

	logfile.flush();

	// ofstream はデストラクタで close() されるが念のため
	
	logfile.close();
}

//
// Copyright (c) 2018, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
