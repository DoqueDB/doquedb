// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.java -- プリペアステートメントクラス
//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

/**
 * プリペアステートメントクラス
 *
 */
public final class PrepareStatement extends Object
{
	/** データソース */
	private DataSource _dataSource;
	/** データベース名 */
	private String _databaseName;
	/** セッション */
	private Session _session;
	/** プリペアID */
	private int _prepareID;

	/**
	 * 新しくプリペアステートメントオブジェクトを作成する。
	 *
	 * @param dataSource_	データソース
	 * @param databaseName_	データベース名
	 * @param prepareID_	プリペアID
	 */
	public PrepareStatement(DataSource dataSource_,
							String databaseName_,
							int prepareID_)
	{
		super(Object.PREPARE_STATEMENT);
		_dataSource = dataSource_;
		_databaseName = databaseName_;
		_session = null;
		_prepareID = prepareID_;
	}

	/**
	 * 新しくプリペアステートメントオブジェクトを作成する。
	 *
	 * @param session_		セッション
	 * @param prepareID_	プリペアID
	 */
	public PrepareStatement(Session session_,
							int prepareID_)
	{
		super(Object.PREPARE_STATEMENT);
		_dataSource = null;
		_databaseName = null;
		_session = session_;
		_prepareID = prepareID_;
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void close()
		throws java.io.IOException
	{
		if (_prepareID != 0)
		{
			try
			{
				if (_session != null)
				{
					_session.erasePrepareStatement(_prepareID);
				}
				else
				{
					Connection clientConnection
						= _dataSource.getClientConnection();
					if (clientConnection != null)
					{
						//プリペアステートメントを削除する
						clientConnection.erasePrepareStatement(
							_databaseName, _prepareID);
					}
				}
			}
			catch (Exception e) {}	//例外は無視する

			_prepareID = 0;
		}
	}

	/**
	 * データベース名を得る。
	 *
	 * @return	データベース名
	 */
	public String getDatabaseName()
	{
		return _databaseName;
	}

	/**
	 * プリペアステートメントIDを得る。
	 *
	 * @return	プリペアステートメントID
	 */
	public int getPrepareID()
	{
		return _prepareID;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
