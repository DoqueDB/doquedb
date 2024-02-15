// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionType.java -- コネクションのタイプ
//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.port;

/**
 * コネクションのタイプ
 *
 */
public final class ConnectionType
{
	/** ローカルコネクション(メモリー) */
	public final static int LOCAL	= 1;
	/** リモートコネクション(ソケット) */
	public final static int REMOTE	= 2;
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
