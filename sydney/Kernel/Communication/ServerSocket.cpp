// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServerSocket.cpp -- サーバ側のソケットクラス
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Communication/ServerSocket.h"

#include "Buffer/File.h"

_TRMEISTER_USING

using namespace Communication;

//
//	FUNCTION public
//	Communication::Socket::Socket -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModServerSocket* pServerSocket_
//		サーバソケット
//		バッファサイズ4096のコーデックを持つものとする
//
//	CryptCodec* pCodec_(暗号化対応)
//		サーバソケットに渡されたコーデック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ServerSocket::ServerSocket(ModServerSocket* pServerSocket_, CryptCodec* pCodec_)
	: Socket(pServerSocket_, pCodec_)
{
}

//
//	FUNCTION public
//	Communication::Socket::~ServerSocket -- デストラクタ
//
//	NOTES
//	デストラクタ
//
// 	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ServerSocket::~ServerSocket()
{
}

//
//	FUNCTION public
//	Communication::ServerSocket::close -- クローズする
//
//	NOTES
//	入出力アーカイブを破棄し、ソケットをクローズする
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
ServerSocket::close()
{
	// 基底から
	Socket::close();
	// バッファモジュールにファイルディスクリプターを破棄したことを知らせる
	Buffer::File::returnDescriptor();
}

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
