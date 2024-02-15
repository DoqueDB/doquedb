// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XADataSource.java -- JDBCX の XADataSourceクラス
//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

import java.sql.Connection;
import java.sql.SQLException;
import java.util.logging.Logger;

import javax.sql.XAConnection;

import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * XAConnection オブジェクトのファクトリです。
 *
 */
public class XADataSource extends ConnectionPoolDataSource implements javax.sql.XADataSource {

	/**
	 * 分散トランザクションで使用可能な、物理データベース接続の確立を試みます。
	 *
	 * @return この XADataSource オブジェクトが表すデータベースへの物理接続である <code>javax.sql.XADataSource</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public XAConnection getXAConnection() throws SQLException {

		Connection conn = getConnection();

		return new jp.co.ricoh.doquedb.jdbcx.XAConnection(conn, conn.getAutoCommit());
	}
	/**
	 * 指定されたユーザ名とパスワードを使用して、物理データベース接続の確立を試みます。
	 * 返される接続は、分散トランザクションで使用できます。
	 *
	 * @param	user_
	 * 			ユーザ名。
	 * @param	password_
	 * 			ユーザパスワード。
	 * @return この XAConnection オブジェクトが表すデータベースへの物理接続である <code>javax.sql.XAConnection</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public XAConnection getXAConnection(String user_, String password_) throws SQLException {

		Connection conn = getConnection(user_, password_);
		return new jp.co.ricoh.doquedb.jdbcx.XAConnection(conn, conn.getAutoCommit());
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
//Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//All rights reserved.
//
