// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionPoolDataSource.java -- JDBCX の ConnectionPoolDataSourceクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbcx;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.sql.SQLException;
import java.util.logging.Logger;

//import javax.sql.ConnectionPoolDataSource;
import javax.sql.PooledConnection;

import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * PooledConnection オブジェクトのファクトリです。
 *
 */
public class ConnectionPoolDataSource extends SydDataSource implements Serializable, javax.sql.ConnectionPoolDataSource
{
	private boolean defaultAutoCommit = true;

	/**
	 * プールされた接続として使用可能な、データベースへの物理接続の確立を試みます。
	 *
	 * @return この ConnectionPoolDataSource オブジェクトが表すデータベースへの物理接続である <code>javax.sql.PooledConnection</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public PooledConnection getPooledConnection() throws SQLException
	{
		return new jp.co.ricoh.doquedb.jdbcx.XAConnection(getConnection(), defaultAutoCommit);
	}

	/**
	 * プールされた接続として使用可能な、データベースへの物理接続の確立を試みます。
	 *
	 * @param	user_
	 * 			ユーザ名。
	 * @param	password_
	 * 			ユーザパスワード。
	 * @return この ConnectionPoolDataSource オブジェクトが表すデータベースへの物理接続である <code>javax.sql.PooledConnection</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public PooledConnection getPooledConnection(String user_, String password_) throws SQLException
	{
		return new jp.co.ricoh.doquedb.jdbcx.XAConnection(getConnection(user_, password_), defaultAutoCommit);
	}

	/**
	 * このプールで供給された接続がデフォルトで autoCommit をつけさせているか否か得ます。
	 *
	 */
	public boolean isDefaultAutoCommit()
	{
		return defaultAutoCommit;
	}

	/**
	 * このプールで供給された接続が autoCommit を持つか否かをセットします。
	 *
	 * @param	defaultAutoCommit_
	 * 			true/falese
	 */
	public void setDefaultAutoCommit(boolean defaultAutoCommit_)
	{
		this.defaultAutoCommit = defaultAutoCommit_;
	}

	private void writeObject(ObjectOutputStream out_) throws IOException
	{
		writeBaseObject(out_);
		out_.writeBoolean(defaultAutoCommit);
	}

	private void readObject(ObjectInputStream in_) throws IOException, ClassNotFoundException
	{
		readBaseObject(in_);
		defaultAutoCommit = in_.readBoolean();
	}

	/**
	 * ドライバで使用されるすべてのロガーの親ロガーを返す。
	 * <B>[サポート外！]</B>
	 *
	 * @return	Loggerオブジェクト。
	 * @throws NotSupported
	 *
	 */
	@Override
	public Logger getParentLogger() throws NotSupported {
		throw new NotSupported();
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
