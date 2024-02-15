// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.java -- trmeister.clientパッケージ共通の基底クラス
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

package jp.co.ricoh.doquedb.client;

import jp.co.ricoh.doquedb.common.*;
import jp.co.ricoh.doquedb.port.*;

/**
 * trmeister.clientパッケージ共通の基底クラス
 *
 */
public abstract class Object
{
	// 各クラスをあらわす値
	public final static int DATA_SOURCE			= 0;
	public final static int PORT				= 1;
	public final static int CONNECTION			= 2;
	public final static int SESSION				= 3;
	public final static int RESULT_SET			= 4;
	public final static int PREPARE_STATEMENT	= 5;

	/** 派生クラスのタイプ */
	private int _type;

	/**
	 * コンストラクタ
	 *
	 * @param type_		派生クラスのタイプ
	 */
	public Object(int type_)
	{
		_type = type_;
	}

	/**
	 * クラスタイプを得る。
	 *
	 * @return	クラスタイプ
	 */
	public int getType()
	{
		return _type;
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	abstract public void close()
		throws java.io.IOException;
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
