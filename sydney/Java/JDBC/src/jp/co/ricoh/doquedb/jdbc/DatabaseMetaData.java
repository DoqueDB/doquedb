// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseMetaData.java -- JDBC のデータベースメタデータクラス
//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2016, 2023, 2024 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbc;

import java.sql.ResultSet;
import java.sql.RowIdLifetime;
import java.sql.SQLException;

import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.NullData;
import jp.co.ricoh.doquedb.common.StringArrayData;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.exception.ConnectionRanOut;
import jp.co.ricoh.doquedb.exception.DatabaseNotFound;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * JDBC のデータベースメタデータクラス。
 * <P>
 * このドキュメントでは、「データベース」という用語は通常、
 * ドライバと DBMS の両方を指しています。
 *
 */
public class DatabaseMetaData implements java.sql.DatabaseMetaData
{
	/**
	 * <code>getTables</code> メソッドによって返される結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetTables = null;

	/**
	 * <code>getColumns</code> メソッドによって返される結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetColumns = null;

	/**
	 * <code>getPrimaryKeys</code> メソッドによって返される
	 * 結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetPrimaryKeys = null;

	/**
	 * <code>getTypeInfo</code> メソッドによって返される結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetTypeInfo = null;

	/**
	 * <code>getIndexInfo</code> メソッドによって返される結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetIndexInfo = null;

	/**
	 * <code>getBestRowIdentifier</code> メソッドによって返される
	 * 結果セットの列の名前。
	 */
	private static StringArrayData	_cnGetBestRowIdentifier = null;

	/**
	 * このメタデータオブジェクトを生成した接続。
	 *
	 * @see	jp.co.ricoh.doquedb.jdbc.Connection
	 */
	private Connection	_connection;

	/**
	 * 新しくメタデータオブジェクトを作成する。
	 *
	 * @param	connection_
	 *			このメタデータオブジェクトを生成する接続。
	 */
	DatabaseMetaData(Connection	connection_)
	{
		this._connection = connection_;
	}

	/**
	 * <code>getProcedures</code> メソッドによって返される
	 * すべてのプロシージャが、現在のユーザから呼び出せるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean allProceduresAreCallable() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return false;
	}

	/**
	 * <code>getTables</code> メソッドによって返されるすべてのテーブルが、
	 * 現在のユーザによって使用できるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean allTablesAreSelectable() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * この DBMS の URL を取得します。
	 *
	 * @return	この DBMS の URL 。生成できない場合は <code>null</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getURL() throws java.sql.SQLException
	{
		// [YET!] DBMS の URL とは？コネクションの URL でよい？
		return this._connection.getURL();
	}

	/**
	 * このデータベースに記録されているユーザ名を取得します。
	 *
	 * @return	データベースユーザ名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getUserName() throws java.sql.SQLException
	{
		// [YET!] データベースに記録されているユーザ名とは？
		//        コネクションのユーザ名でよい？
		return this._connection.getUserName();
	}

	/**
	 * このデータベースが読み込み専用モードかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isReadOnly() throws java.sql.SQLException
	{
		java.sql.Connection	connection = null;
		java.sql.Statement	statement = null;
		java.sql.ResultSet	resultSet = null;

		boolean	databaseIsReadOnly = false;

		try {

			String	databaseName =
				this._connection.getSession().getDatabaseName();
			String	query =
				"select Flag " +
				"from System_Database " +
				"where Name like '" + databaseName + "';";
			connection = this.connectSystemDatabase();
			statement = connection.createStatement();
			resultSet = statement.executeQuery(query);

			if (resultSet.next() == false) {
				// データベースが存在しない
				throw new DatabaseNotFound(databaseName);
			}

			int	databaseFlag = resultSet.getInt(1);
			// System_Database.Flag の最下位ビットが読み取り専用フラグ。
			databaseIsReadOnly = ((databaseFlag & 0x1) != 0);

		} catch (java.sql.SQLException	sqle) {

			throw sqle;

		} catch (java.lang.Exception	e) {

			// [YET!] なんらかのエラーおよび SQLSTATE を割り当てるべき。
			throw new Unexpected();

		} finally {

			if (resultSet != null) resultSet.close();
			if (statement != null) statement.close();
			if (connection != null) connection.close();
		}

		return databaseIsReadOnly;
	}

	/**
	 * <code>NULL</code> 値が高位にソートされるかどうかを取得します。
	 * 高位にソートされるとは、 <code>NULL</code> 値が
	 * ドメイン内の他のどの値よりもソート順が上であるということです。
	 * 昇順では、このメソッドが <code>true</code> を返す場合、
	 * <code>NULL</code> 値は最後に現れます。
	 * 対照的に、 <code>nullsAreSortedAtEnd</code> メソッドは
	 * <code>NULL</code> 値がソート順にかかわらず最後にソートされることを
	 * 示します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、 <code>NULL</code> 値は下位にソートされるので、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean nullsAreSortedHigh() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <code>NULL</code> 値が下位にソートされるかどうかを取得します。
	 * 下位にソートされるとは、 <code>NULL</code> 値が
	 * ドメイン内の他のどの値よりもソート順が下であるということです。
	 * 昇順では、このメソッドが <code>true</code> を返す場合、
	 * <code>NULL</code> 値は最初に現れます。
	 * 対照的に、 <code>nullsAreSortedAtStart</code> メソッドは
	 * <code>NULL</code> 値がソート順にかかわらず最初にソートされることを
	 * 示します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、 <code>NULL</code> 値は下位にソートされるので、
	 * 常に <code>true</code> を返します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean nullsAreSortedLow() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * <code>NULL</code> 値が、ソート順にかかわらず
	 * 最初にソートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、ソート順により <code>NULL</code> 値が高位または下位の
	 * いずれかにソートされるので、常に <code>false</code> を返します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean nullsAreSortedAtStart() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <code>NULL</code> 値が、ソート順にかかわらず
	 * 最後にソートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、ソート順により <code>NULL</code> 値が高位または下位の
	 * いずれかにソートされるので、常に <code>false</code> を返します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean nullsAreSortedAtEnd() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベース製品の名前を取得します。
	 *
	 * @return	データベース製品の名前。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getDatabaseProductName() throws java.sql.SQLException
	{
		return "DoqueDB";
	}

	/**
	 * このデータベース製品のバージョンを取得します。
	 *
	 * @return	データベースのバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getDatabaseProductVersion() throws java.sql.SQLException
	{
		try {
			return _connection.getSession().queryProductVersion();
		} catch (java.io.IOException ioe) {
			ConnectionRanOut	croe = new ConnectionRanOut();
			croe.initCause(ioe);
			throw croe;

		} catch (ClassNotFoundException cnfe) {
			Unexpected	ue = new Unexpected();
			ue.initCause(cnfe);
			throw ue;
		}
	}

	/**
	 * この JDBC ドライバの名前を取得します。
	 *
	 * @return	JDBC ドライバの名前。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getDriverName() throws java.sql.SQLException
	{
		return "DoqueDB JDBC Driver";
	}

	/**
	 * この JDBC ドライバのバージョンを <code>java.lang.String</code> として
	 * 取得します。
	 *
	 * @return	JDBC ドライバのバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getDriverVersion() throws java.sql.SQLException
	{
		String	driverVersion =
			String.valueOf(Driver.MAJOR_VERSION) +
			"." +
			String.valueOf(Driver.MINOR_VERSION);
		return driverVersion;
	}

	/**
	 * この JDBC ドライバのメジャーバージョンを取得します。
	 *
	 * @return	JDBC ドライバのメジャーバージョン。
	 */
	public int getDriverMajorVersion()
	{
		return Driver.MAJOR_VERSION;
	}

	/**
	 * この JDBC ドライバのマイナーバージョンを取得します。
	 *
	 * @return	JDBC ドライバのマイナーバージョン。
	 */
	public int getDriverMinorVersion()
	{
		return Driver.MINOR_VERSION;
	}

	/**
	 * このデータベースが、ひとつのローカルファイルに複数のテーブルを
	 * 格納するかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、ひとつのローカルファイルに複数のテーブルは格納しないので、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean usesLocalFiles() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、テーブルごとにひとつのファイルを使用するかどうかを
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * DoqueDB では、テーブルごとに複数のファイルを使用するので、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean usesLocalFilePerTable() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符なしの SQL 識別子を、
	 * 大文字小文字を区別して処理し、大文字小文字混在で格納するかどうかを
	 * 取得します。
	 *
	 * @return	常に  <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMixedCaseIdentifiers() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符なしの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、大文字で格納するかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesUpperCaseIdentifiers() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符なしの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、小文字で格納するかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesLowerCaseIdentifiers() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符なしの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、大文字小文字混在で格納するかどうかを
	 * 取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesMixedCaseIdentifiers() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符付きの SQL 識別子を、
	 * 大文字小文字を区別して処理し、結果として大文字小文字混在で
	 * 格納するかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMixedCaseQuotedIdentifiers()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符付きの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、大文字で格納するかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesUpperCaseQuotedIdentifiers()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符付きの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、小文字で格納するかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesLowerCaseQuotedIdentifiers()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースが、大文字小文字が混在する引用符付きの SQL 識別子を、
	 * 大文字小文字を区別しないで処理し、大文字小文字混在で
	 * 格納するかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean storesMixedCaseQuotedIdentifiers()
		throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * SQL 識別子を引用するのに使用する文字列を取得します。
	 *
	 * @return	常に二重引用符。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getIdentifierQuoteString() throws java.sql.SQLException
	{
		return "\"";
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースの SQL キーワードであって、SQL92 のキーワードではない、
	 * すべてのキーワードをコンマで区切ったリストを取得します。
	 *
	 * @return	SQL92 のキーワードではない、
	 *			このデータベースの SQL キーワードのリスト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getSQLKeywords() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースで使用可能なコンマで区切った数学関数のリストを
	 * 取得します。これらは JDBC 関数のエスケープ節で使用される
	 * Open /Open CLI 数学関数名です。
	 *
	 * @return	このデータベースによってサポートされる数学関数のリスト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getNumericFunctions() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースで使用可能なコンマで区切った文字列関数のリストを
	 * 取得します。これらは JDBC 関数のエスケープ節で使用される
	 * Open Group CLI 文字列関数名です。
	 *
	 * @return	このデータベースによってサポートされる文字列関数のリスト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getStringFunctions() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースで使用可能なコンマで区切ったシステム関数のリストを
	 * 取得します。これらは JDBC 関数のエスケープ節で使用される
	 * Open Group CLI システム列関数名です。
	 *
	 * @return	このデータベースによってサポートされるシステム関数のリスト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getSystemFunctions() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースで使用可能な時間関数と日付関数をコンマで区切ったリストを
	 * 取得します。
	 *
	 * @return	このデータベースによってサポートされる
	 *			時間関数と日付関数のリスト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getTimeDateFunctions() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * ワイルドカード文字をエスケープするのに使用できる文字列を取得します。
	 * これは、パターンのカタログ検索パラメータで <code>'_'</code> や
	 * <code>'%'</code> をエスケープするのに使用できる文字列です
	 * (パターンであるためワイルドカード文字を使用) 。
	 *
	 * @return	ワイルドカード文字をエスケープするのに使用する文字列。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getSearchStringEscape() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 引用符で囲まれていない識別名に使用できるすべての「特殊」文字
	 * ( a-z 、 A-Z 、 0-9 、および _ 以外) を取得します。
	 *
	 * @return	特殊文字を含む文字列。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getExtraNameCharacters() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * このデータベースによって、追加列のある <code>ALTER TABLE</code> が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsAlterTableWithAddColumn()
		throws java.sql.SQLException
	{
		return _connection.getMasterID() > jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION1;
	}

	/**
	 * このデータベースによって、ドロップ列のある <code>ALTER TABLE</code> が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ドロップ列のある <code>ALTER TABLE</code> を
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsAlterTableWithDropColumn()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ドロップ列のある ALTER TABLE は未サポート。
		return false;
	}

	/**
	 * このデータベースによって列の別名がサポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsColumnAliasing() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースが、 <code>NULL</code> 値と非 <code>NULL</code> 値の
	 * 連結を <code>NULL</code> とするかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean nullPlusNonNullIsNull() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、SQL の型間の <code>CONVERT</code> 関数が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>CONVERT</code> 関数を
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsConvert() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] CONVERT 関数は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、指定された SQL の型間の
	 * <code>CONVERT</code> 関数がサポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>CONVERT</code> 関数を
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @param	fromType_
	 *			変換元の型。 <code>java.sql.Types</code> クラスの
	 *			型コードのうちの 1 つ。
	 * @param	toType_
	 *			変換先の型。 <code>java.sql.Types</code> クラスの
	 *			型コードのうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsConvert(int	fromType_,
								   int	toType_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] CONVERT 関数は未サポート。
		return false;
	}

	/**
	 * このデータベースによってテーブル相互関係名がサポートされるかどうかを
	 * 取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsTableCorrelationNames()
		throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * テーブル相互関係名がサポートされる場合、
	 * テーブルの名前と異なる名前であるという制限を付けるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsDifferentTableCorrelationNames()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、 <code>ORDER BY</code> リスト中で
	 * 式がサポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>ORDER BY</code> リスト中での
	 * 式をサポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsExpressionsInOrderBy() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ORDER BY リスト中での式は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、 <code>ORDER BY</code> 節で
	 * <code>SELECT</code> 文中にない列の使用がサポートされるかどうかを
	 * 取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOrderByUnrelated() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、 <code>GROUP BY</code> 節のフォームが
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsGroupBy() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、 <code>GROUP BY</code> 節で
	 * <code>SELECT</code> 文中にない列の使用がサポートされるかどうかを
	 * 取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsGroupByUnrelated() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * <code>SELECT</code> 文中のすべての列が <code>GROUP BY</code> 節に
	 * 含まれるという条件で、このデータベースによって、
	 * <code>GROUP BY</code> 節で <code>SELECT</code> 文中にない列の使用が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsGroupByBeyondSelect() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、<code>LIKE</code> エスケープ節の指定が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsLikeEscapeClause() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、 <code>execute</code> メソッドの
	 * 単一の呼び出しからの複数の
	 * <code>java.sql.ResultSet</code> オブジェクトの取得が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * <code>java.sql.Statement.execute</code> メソッドおよび
	 * <code>java.sql.PreparedStatement.execute</code> メソッドを
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMultipleResultSets() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] execute メソッドは未サポート。
		return false;
	}

	/**
	 * このデータベースが一度に複数のトランザクションを (異なった接続で)
	 * オープンできるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMultipleTransactions() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースの列を非 <code>null</code> として定義できるかどうかを
	 * 取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsNonNullableColumns() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、ODBC Minimum SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMinimumSQLGrammar() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、ODBC Core SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCoreSQLGrammar() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、ODBC Extended SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsExtendedSQLGrammar() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、ANSI92 エントリレベルの SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsANSI92EntryLevelSQL() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、ANSI92 中間レベルの SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsANSI92IntermediateSQL() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、ANSI92 完全レベルの SQL 文法が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsANSI92FullSQL() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって、SQL Integrity Enhancement Facility が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsIntegrityEnhancementFacility()
		throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * このデータベースによって、外部結合のなんらかの形式が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOuterJoins() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、完全入れ子の外部結合が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、完全入れ子の外部結合をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsFullOuterJoins() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 完全入れ子の外部結合は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、外部結合に関し、制限されたサポートが
	 * 提供されるかどうかを取得します
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsLimitedOuterJoins() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * <B>[サポート外]</B>
	 * 「schema」に対するデータベースベンダーの推奨用語を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていません。
	 *
	 * @return	「schema」に対するベンダーの用語。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getSchemaTerm() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 「procedure」に対するデータベースベンダーの推奨用語を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていません。
	 *
	 * @return	「procedure」に対するベンダーの用語。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getProcedureTerm() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 「catalog」に対するデータベースベンダーの推奨用語を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていません。
	 *
	 * @return	「catalog」に対するベンダーの用語。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getCatalogTerm() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 完全修飾されたテーブル名の開始部分 (または終了部分) に
	 * カタログが現れるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていません。
	 *
	 * @return	完全修飾されたテーブル名の開始部分がカタログ名に現れる場合は
	 *			<code>true</code> 、そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isCatalogAtStart() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		throw new NotSupported();
	}

	/**
	 * このデータベースがカタログ名とテーブル名のセパレータとして使用する
	 * <code>java.lang.String</code> を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に空文字列を返します。
	 *
	 * @return	現在のバージョンでは、常に空文字列。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getCatalogSeparator() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return "";
	}

	/**
	 * データ操作文でスキーマ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSchemasInDataManipulation()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return false;
	}

	/**
	 * プロシージャ呼び出し文でスキーマ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSchemasInProcedureCalls()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return false;
	}

	/**
	 * テーブル定義文でスキーマ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSchemasInTableDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return false;
	}

	/**
	 * インデックス定義文でスキーマ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSchemasInIndexDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return false;
	}

	/**
	 * 特権定義文でスキーマ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSchemasInPrivilegeDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return false;
	}

	/**
	 * データ操作文でカタログ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCatalogsInDataManipulation()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return false;
	}

	/**
	 * プロシージャ呼び出し文でカタログ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCatalogsInProcedureCalls()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return false;
	}

	/**
	 * テーブル定義文でカタログ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCatalogsInTableDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return false;
	}

	/**
	 * インデックス定義文でカタログ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCatalogsInIndexDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return false;
	}

	/**
	 * 特権定義文でカタログ名を使用できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCatalogsInPrivilegeDefinitions()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return false;
	}

	/**
	 * このデータベースによって、位置指定された <code>DELETE</code> 文が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * 位置指定された <code>DELETE</code> 文をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsPositionedDelete() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 位置指定つき DELETE 文は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、位置指定された <code>UPDATE</code> 文が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * 位置指定された <code>UPDATE</code> 文をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsPositionedUpdate() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 位置指定つき UPDATE 文は未サポート。
		return false;
	}

	/**
	 * このデータベースによって <code>SELECT FOR UPDATE</code> 文が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、<code>SELECT FOR UPDATE</code> 文を
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSelectForUpdate() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SELECT FOR UPDATE 文は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、ストアドプロシージャエスケープ構文を使用する
	 * ストアドプロシージャコールがサポートされるかどうかを判定します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャコールを
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsStoredProcedures() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって、比較式中でサブクエリーが
	 * サポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSubqueriesInComparisons()
		throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * このデータベースによって、<code>EXISTS</code> 式中でサブクエリーが
	 * サポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSubqueriesInExists() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって、<code>IN</code> 文中でサブクエリーが
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、<code>IN</code> 文をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSubqueriesInIns() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって、定量化された式中でサブクエリーが
	 * サポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSubqueriesInQuantifieds()
		throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * このデータベースによって照合関係サブクエリーが
	 * サポートされるかどうかを取得します。
	 * <P>
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsCorrelatedSubqueries() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースによって SQL <code>UNION</code> が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>UNION</code> をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsUnion() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL UNION は未サポート。
		return false;
	}

	/**
	 * このデータベースによって SQL <code>UNION ALL</code> が
	 * サポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 SQL <code>UNION ALL</code> を
	 * サポートしていないため、常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsUnionAll() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] SQL UNION ALL は未サポート。
		return false;
	}

	/**
	 * このデータベースによって、コミット間でカーソルが
	 * オープンされたままの状態がサポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOpenCursorsAcrossCommit()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		return false;
	}

	/**
	 * このデータベースによって、ロールバック間でカーソルが
	 * オープンされたままの状態がサポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOpenCursorsAcrossRollback()
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		return false;
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって、コミット間で文がオープンされたままの状態が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	文が常にオープンされた状態の場合は <code>true</code> 、
	 *			オープンされた状態ではない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOpenStatementsAcrossCommit()
		throws java.sql.SQLException
	{
		// [YET!] まだ未実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって、ロールバック間で文がオープンされたままの状態が
	 * サポートされるかどうかを取得します。
	 *
	 * @return	文が常にオープンされた状態の場合は <code>true</code> 、
	 *			オープンされた状態ではない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsOpenStatementsAcrossRollback()
		throws java.sql.SQLException
	{
		// [YET!] まだ未実装。
		throw new NotSupported();
	}

	/**
	 * このデータベースで、インラインバイナリリテラル中に入れられる
	 * 16 進数の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxBinaryLiteralLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、キャラクタリテラルの最大文字数を取得します。
	 * 16 進数の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxCharLiteralLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、列名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnNameLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、 <code>GROUP BY</code> 節中の列数の最大値を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnsInGroupBy() throws java.sql.SQLException
	{
		// [YET!] 現在のバージョンでは暫定的に“不明”を示す 0 を返す。
		return 0;
	}

	/**
	 * このデータベースでの、インデックス中の列数の最大値を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnsInIndex() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、 <code>ORDER BY</code> 節中の列数の最大値を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnsInOrderBy() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、<code>SELECT</code> リスト中の列数の最大値を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnsInSelect() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、テーブル中の列数の最大値を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxColumnsInTable() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースへの可能な現在の接続の最大数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxConnections() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、カーソル名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カーソルをサポートしていないため、
	 * 常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxCursorNameLength() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カーソルは未サポート。
		return 0;
	}

	/**
	 * このデータベースでの、インデックスの全部分を含む、
	 * インデックスの最大バイト数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxIndexLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、スキーマ名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxSchemaNameLength() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return 0;
	}

	/**
	 * このデータベースでの、プロシージャ名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * 常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxProcedureNameLength() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return 0;
	}

	/**
	 * このデータベースでの、カタログ名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxCatalogNameLength() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return 0;
	}

	/**
	 * このデータベースでの、1 行の最大バイト数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxRowSize() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * <code>getMaxRowSize</code> メソッドの戻り値が SQL データの型の
	 * <code>java.sql.Types.LONGVARCHAR</code> および
	 * <code>java.sql.Types.LONGVARBINARY</code> を含むかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、 <code>getMaxRowSize</code> の戻り値が不明なので、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean doesMaxRowSizeIncludeBlobs() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースでの、SQL 文の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxStatementLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースの同時にオープンできるアクティブな文の最大数を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxStatements() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、テーブル名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxTableNameLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、 <code>SELECT</code> 文の最大テーブル数を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxTablesInSelect() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースでの、ユーザ名の最大文字数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、不明なので常に <code>0</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>0</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getMaxUserNameLength() throws java.sql.SQLException
	{
		return 0;
	}

	/**
	 * このデータベースのデフォルトのトランザクション遮断レベルを取得します。
	 * 取り得る値は、 <code>java.sql.Connection</code> で定義されています。
	 *
	 * @return	デフォルトの遮断レベル。
	 *			常に
	 *			<code>java.sql.Connection.TRANSACTION_READ_COMMITTED</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getDefaultTransactionIsolation() throws java.sql.SQLException
	{
		return java.sql.Connection.TRANSACTION_READ_COMMITTED;
	}

	/**
	 * このデータベースによってトランザクションがサポートされるかどうかを
	 * 取得します。サポートされない場合、<code>Connection.commit</code>
	 * メソッドを呼び出しても操作なし (noop) で、遮断レベルは
	 * <code>java.sql.Connection.TRANSACTION_NONE</code> です。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsTransactions() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * このデータベースが、指定されたトランザクション遮断レベルを
	 * サポートするかどうかを取得します。
	 *
	 * @param	level_
	 *			<code>java.sql.Connection</code> で定義される
	 *			トランザクション遮断レベルのうちの 1 つ。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsTransactionIsolationLevel(int	level_)
		throws java.sql.SQLException
	{
		switch (level_) {
		case java.sql.Connection.TRANSACTION_READ_UNCOMMITTED:
		case java.sql.Connection.TRANSACTION_READ_COMMITTED:
		case java.sql.Connection.TRANSACTION_REPEATABLE_READ:
		case java.sql.Connection.TRANSACTION_SERIALIZABLE:
		case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT:
			return true;
		}

		return false;
	}

	/**
	 * このデータベースによって、トランザクションで、データ定義文と
	 * データ操作文の両方がサポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsDataDefinitionAndDataManipulationTransactions()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、トランザクションでデータ操作文だけが
	 * サポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsDataManipulationTransactionsOnly()
		throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * トランザクションのデータ定義文が、トランザクションを強制的に
	 * コミットさせるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean dataDefinitionCausesTransactionCommit()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * このデータベースによって、トランザクションでデータ定義文が
	 * 無視されるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean dataDefinitionIgnoredInTransactions()
		throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * 指定されたカタログで使用可能なストアドプロシージャに関する記述を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * ストアドプロシージャをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。データベースに格納されたカタログ名と
	 *			一致しなければならない。 "" はカタログなしでカタログ名を
	 *			検索する。 <code>null</code> は、カタログ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。データベースに格納されたスキーマ名と
	 *			一致しなければならない。 "" はスキーマなしでスキーマ名を
	 *			検索する。 <code>null</code> は、スキーマ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	procedureNamePattern_
	 *			プロシージャ名パターン。データベースに格納された
	 *			プロシージャ名と一致しなければならない。
	 * @return	現在のバージョンでは、常に空の <code>java.sql.ResultSet</code>
	 *			オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getProcedures(String	catalog_,
											String	schemaPattern_,
											String	procedureNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 指定されたカタログのストアドプロシージャパラメータと結果列に関する記述を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * ストアドプロシージャをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。データベースに格納されたカタログ名と
	 *			一致しなければならない。 "" はカタログなしでカタログ名を
	 *			検索する。 <code>null</code> は、カタログ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。データベースに格納されたスキーマ名と
	 *			一致しなければならない。 "" はスキーマなしでスキーマ名を
	 *			検索する。 <code>null</code> は、スキーマ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	procedureNamePattern_
	 *			プロシージャ名パターン。データベースに格納された
	 *			プロシージャ名と一致しなければならない。
	 * @param	columnNamePattern_
	 *			列名パターン。データベースに格納された列名と
	 *			一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getSearchStringEscape()
	 */
	public java.sql.ResultSet
	getProcedureColumns(String	catalog_,
						String	schemaPattern_,
						String	procedureNamePattern_,
						String	columnNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 使用可能なテーブルに関する記述を取得します。
	 * <P>
	 * テーブル名の条件に一致するテーブルの記述だけが返されます。
	 * それらは、 TABLE_NAME によって順序付けられます。
	 * <P>
	 * 各テーブルの記述には次の列があります。
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String =&gt;
	 *		テーブルカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_SCHEM</B> String =&gt;
	 *		テーブルスキーマ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_NAME</B> String =&gt;
	 *		テーブル名
	 *	<LI><B>TABLE_TYPE</B> String =&gt;
	 *		テーブルの型
	 *		(現在のバージョンでは常に <code>"TABLE"</code> である)
	 *	<LI><B>REMARKS</B> String =&gt;
	 *		テーブルに関する説明
	 *		(現在のバージョンでは常に空文字列である)
	 *	<LI><B>TYPE_CAT</B> String =&gt;
	 *		型のカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TYPE_SCHEM</B> String =&gt;
	 *		型のスキーマ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TYPE_NAME</B> String =&gt;
	 *		型名
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>SELF_REFERENCING_COL_NAME</B> String =&gt;
	 *		型付きテーブルの指定された「識別子」列の名前
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>REF_GENERATION</B> String =&gt;
	 *		SELF_REFERENCING_COL_NAME の値の作成方法を指定する。
	 *		値は、 "SYSTEM" 、 "USER" 、 "DERIVED"
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	tableNamePattern_
	 *			テーブル名パターン。データベースに格納されたテーブル名と
	 *			一致しなければならない。
	 * @param	types_
	 *			組み込むテーブルの型のリスト。
	 *			現在のバージョンでは、このパラメータは無視され、
	 *			常にすべての型を返します。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。
	 *			各行はテーブルの記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getTables(String		catalog_,
										String		schemaPattern_,
										String		tableNamePattern_,
										String[]	types_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] catalog_, schemaPattern_, types_ は無視。

		//java.sql.Connection	connection = null;
		java.sql.Statement	statement = null;
		java.sql.ResultSet	resultSet = null;

		DataArrayData	tables = new DataArrayData();

		try {

			String	query = "select Name from System_Table";
			// 表名パターン未指定時、および空文字列の場合は
			// すべてのテーブルについて返す
			if (tableNamePattern_ != null && tableNamePattern_.length() > 0) query = query + " where Name like '" + tableNamePattern_ + "'";
			query = query + " order by Name";

			//connection =
			//	java.sql.DriverManager.getConnection(this._connection.getURL(),
			//										 "",
			//										 "");
			//statement = connection.createStatement();
			statement = this._connection.createStatement();
			resultSet = statement.executeQuery(query);

			boolean	isExist = false;

			while (resultSet.next()) {

				isExist = true;

				String	tableName = resultSet.getString(1);
				DataArrayData	tableInfo =
					this.getTableInfo(tableName);
				tables.addElement(tableInfo);
			}

			// 表名パターンに該当する表が存在しない場合、
			// 例外を投げるのは止め、空の ResultSet を返す

			if (isExist == false) return new MetaDataResultSet();
			//if (isExist == false) {
			//	String	databaseName =
			//		this._connection.getSession().getDatabaseName();
			//	// [YET!] SQLSTATE は、
			//	//        syntax error or access rule violation -
			//	//        table not found in database
			//	//        (42503)
			//	throw new TableNotFound(tableNamePattern_, databaseName);
			//}

		} catch (java.sql.SQLException	sqle) {

			throw sqle;

		} catch (java.lang.Exception	e) {

			// [YET!] なんらかのエラーおよび SQLSTATE を割り当てるべき。
			throw new Unexpected();

		} finally {

			if (resultSet != null) resultSet.close();
			if (statement != null) statement.close();
			//if (connection != null) connection.close();
		}

		return new MetaDataResultSet(tables,
									 this.getTableInfoColumnNames());
	}

	/**
	 * このデータベースで使用可能なスキーマ名を取得します。
	 * 結果はスキーマ名で順序付けられます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、スキーマをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getSchemas() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] スキーマは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースで使用可能なカタログ名を取得します。
	 * 結果はカタログ名によって順序付けられます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、カタログをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getCatalogs() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] カタログは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースで使用可能なテーブルの型を取得します。
	 * 結果はテーブルの型によって順序付けられます。
	 * <P>
	 * テーブルの型は次のようになります。
	 *	<OL>
	 *	<LI><B>TABLE_TYPE</B> String =&gt;
	 *		テーブルの型。典型的な型は、 "TABLE" 、 "VIEW" 、
	 *		"SYSTEM TABLE" 、 "GLOBAL TEMPORARY" 、 "LOCAL TEMPORARY" 、
	 *		"ALIAS" 、 "SYNONYM" である
	 *	</OL>
	 *
	 * @return	ResultSet オブジェクト。各行は、
	 *			テーブルの型である単一の <code>java.lang.String</code> の列。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getTableTypes() throws java.sql.SQLException
	{
		return new MetaDataResultSet();
	}

	/**
	 * テーブル列の記述を取得します。
	 * <P>
	 * テーブル名、および列名の条件に一致する列の記述だけが返されます。
	 * それらは、 TABLE_NAME 、 ORDINAL_POSITION によって順序付けられます。
	 * <P>
	 * 各列の説明を次にします。
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String =&gt;
	 *		テーブルカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_SCHEM</B> String =&gt;
	 *		テーブルスキーマ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_NAME</B> String =&gt;
	 *		テーブル名
	 *	<LI><B>COLUMN_NAME</B> String =&gt;
	 *		列名
	 *	<LI><B>DATA_TYPE</B> short =&gt;
	 *		<code>java.sql.Types</code> からの SQL の型。
	 *		配列型の列の場合には要素の型ではなく <code>java.sql.Types.ARRAY</code> となる
	 *	<LI><B>TYPE_NAME</B> String =&gt;
	 *		データソース依存の型名。 UDT の場合、型名は完全指定。
	 *		配列型の列の場合には要素の型名 ＋ <code>" array"</code> となる
	 *	<LI><B>COLUMN_SIZE</B> int =&gt;
	 *		列サイズ。 <code>char</code> や <code>date</code> の型については
	 *		最大文字数、<code>numeric</code> や <code>decimal</code> の
	 *		型については精度。
	 *		最大文字数が無制限の場合は <code>-1</code>
	 *	<LI><B>BUFFER_LENGTH</B> int =&gt;
	 *		未使用
	 *	<LI><B>DECIMAL_DIGITS</B> int =&gt;
	 *		小数点以下の桁数
	 *	<LI><B>NUM_PREC_RADIX</B> int =&gt;
	 *		基数
	 *	<LI><B>NULLABLE</B> int =&gt;
	 *		<code>NULL</code> は許されるか
	 *		<UL>
	 *		<LI>columnNoNulls -
	 *			<code>NULL</code> 値を許さない可能性がある
	 *		<LI>columnNullable -
	 *			必ず <code>NULL</code> 値を許す
	 *		<LI>columnNullableUnknown -
	 *			<code>NULL</code> 値を許すかどうかは不明
	 *		</UL>
	 *	<LI><B>REMARKS</B> String =>
	 *		コメント記述列
	 *		列へのヒント。ヒントがなければ <code>null</code>
	 *	<LI><B>COLUMN_DEF</B> String =&gt;
	 *		デフォルト値
	 *	<LI><B>SQL_DATA_TYPE</B> int =&gt;
	 *		未使用
	 *	<LI><B>SQL_DATETIME_SUB</B> int =&gt;
	 *		未使用
	 *	<LI><B>CHAR_OCTET_LENGTH</B> int =&gt;
	 *		<code>char</code> の型については列の最大バイト数。
	 *		無制限の場合は <code>-1</code>
	 *	<LI><B>ORDINAL_POSITION</B> int =&gt;
	 *		テーブル中の列のインデックス ( 1 から始まる)
	 *	<LI><B>IS_NULLABLE</B> String =&gt;
	 *		"NO" は、列は決して <code>NULL</code> 値を許さないことを意味する。
	 *		"YES" は <code>NULL</code> 値を許す可能性があることを意味する。
	 *		空の文字列は不明であることを意味する
	 *	<LI><B>SCOPE_CATLOG</B> String =&gt;
	 *		参照属性のスコープであるテーブルのカタログ
	 *		(DATA_TYPE が <code>java.sql.Types.REF</code> でない場合は
	 *		<code>null</code> )
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>SCOPE_SCHEMA</B> String =&gt;
	 *		参照属性のスコープであるテーブルのスキーマ
	 *		(DATA_TYPE が <code>java.sql.Types.REF</code> でない場合は
	 *		<code>null</code> )
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>SCOPE_TABLE</B> String =&gt;
	 *		参照属性のスコープであるテーブル名
	 *		(DATA_TYPE が <code>java.sql.Types.REF</code> でない場合は
	 *		<code>null</code> )
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>SOURCE_DATA_TYPE</B> short =&gt;
	 *		個別の型またはユーザ生成 <code>Ref</code> 型、
	 *		<code>java.sql.Types</code> の SQL 型のソースの型
	 *		(DATA_TYPE が <code>java.sql.Types.DISTINCT</code>
	 *		またはユーザ生成 <code>java.sql.Types.REF</code> でない場合は
	 *		<code>null</code> )
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	tableNamePattern_
	 *			テーブル名パターン。データベースに格納されたテーブル名と
	 *			一致しなければならない。
	 * @param	columnNamePattern_
	 *			列名パターン。データベースに格納された列名と
	 *			一致しなければならない。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。各行は列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getColumns(String	catalog_,
										 String	schemaPattern_,
										 String	tableNamePattern_,
										 String	columnNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] catalog_, schemaPattern_ は無視。
		//java.sql.Connection	connection = null;
		java.sql.Statement	statement = null;
		java.sql.ResultSet	resultSet = null;

		DataArrayData	columns = new DataArrayData();

		try {

			String	query =
				"select " +
				"System_Table.Name, " +
				"System_Column.Name, " +
				"System_Column.Position";

			boolean	isNotV14 = (_connection.getMasterID() > jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION1);
			if (isNotV14) {

				query =
					query +
					", " +
					"System_Column.Flag, " +
					"System_Column.MetaData ";

			} else {

				// System_Column 表には Flag, MetaData 列はなし
				query = query + " ";
			}

			query = query + "from System_Column, System_Table where ";
			if (tableNamePattern_ != null &&
				tableNamePattern_.length() > 0) {
				query =
					query +
					"System_Table.Name like '" + tableNamePattern_ + "' and ";
			}
			query =
				query +
				"System_Table.RowID = System_Column.ParentID ";
			if (columnNamePattern_ != null &&
				columnNamePattern_.length() > 0) {
				query =
					query + "and System_Column.Name like '" +
					 columnNamePattern_ + "' ";
			}
			query =
				query + "order by System_Table.Name, System_Column.Position;";

			//connection =
			//	java.sql.DriverManager.getConnection(this._connection.getURL(),
			//										 "",
			//										 "");
			//statement = connection.createStatement();
			statement = this._connection.createStatement();
			resultSet = statement.executeQuery(query);

			boolean	isExist = false;

			while (resultSet.next()) {

				isExist = true;

				String			tableName = resultSet.getString(1);
				String			columnName = resultSet.getString(2);
				int				columnPosition = resultSet.getInt(3);
				int				columnFlag = 0;
				java.sql.Array	columnMetaData = null;
				if (isNotV14) {
					columnFlag = resultSet.getInt(4);
					columnMetaData = resultSet.getArray(5);
				}

				if (columnName.compareToIgnoreCase("RowID") != 0) {
					DataArrayData	columnInfo;
					if (isNotV14) {

						columnInfo =
							this.getColumnInfo(tableName,
											   columnName,
											   columnPosition,
											   columnFlag,
											   (String[])columnMetaData.getArray());
					} else {

						columnInfo =
							this.getColumnInfo(tableName,
											   columnName,
											   columnPosition);
					}

					columns.addElement(columnInfo);
				}
			}

			if (isExist == false) return new MetaDataResultSet();

		} catch (java.sql.SQLException	sqle) {

			throw sqle;

		} catch (java.lang.Exception	e) {

			// [YET!] なんらかのエラーおよび SQLSTATE を割り当てるべき。
			throw new Unexpected();

		} finally {

			if (resultSet != null) resultSet.close();
			if (statement != null) statement.close();
			//if (connection != null) connection.close();
		}

		return new MetaDataResultSet(columns,
									 this.getColumnInfoColumnNames());
	}

	/**
	 * テーブルの列へのアクセス権に関する記述を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、アクセス権をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> を返します。
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @param	columnNamePattern_
	 *			列名パターン。
	 *			データベースに格納された列名と一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getSearchStringEscape()
	 */
	public java.sql.ResultSet getColumnPrivileges(String	catalog_,
												  String	schema_,
												  String	table_,
												  String	columnNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] アクセス権は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * カタログで使用可能な各テーブルに対するアクセス権に関する記述を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、アクセス権をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> を返します。
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する
	 * @param	schemaPattern_
	 *			スキーマ名パターン。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する
	 * @param	tableNamePattern_
	 *			テーブル名パターン。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getSearchStringEscape()
	 */
	public java.sql.ResultSet getTablePrivileges(String	catalog_,
												 String	schemaPattern_,
												 String	tableNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] アクセス権は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 行を一意に識別するテーブルの最適な列セットに関する記述を取得します。
	 * それらは、SCOPE によって順序付けられます。
	 * <P>
	 * 各列の説明を次にします。
	 *	<OL>
	 *	<LI><B>SCOPE</B> short =&gt;
	 *		結果の実際のスケール
	 *		<UL>
	 *		<LI>bestRowTemporary - 行は一時的に使用中
	 *		<LI>bestRowTransaction - 現在のトランザクションの残りの部分に有効
	 *		<LI>bestRowSession - 現在のセッションの残りの部分に有効
	 *		</UL>
	 *	<LI><B>COLUMN_NAME</B> String =&gt;
	 *		列名
	 *	<LI><B>DATA_TYPE</B> short =&gt;
	 *		<code>java.sql.Types</code> からの SQL データの型
	 *	<LI><B>TYPE_NAME</B> String =&gt;
	 *		データソース依存の型名。 UDT の場合、型名は完全指定
	 *	<LI><B>COLUMN_SIZE</B> int =&gt;
	 *		精度
	 *	<LI><B>BUFFER_LENGTH</B> int =&gt;
	 *		未使用
	 *	<LI><B>DECIMAL_DIGITS</B> short =&gt;
	 *		スケール
	 *	<LI><B>PSEUDO_COLUMN</B> short =&gt;
	 *		Oracle ROWID のような疑似列
	 *		<UL>
	 *		<LI>bestRowUnknown - 疑似列であるか、またはそうでない可能性がある
	 *		<LI>bestRowNotPseudo - 疑似列ではない
	 *		<LI>bestRowPseudo - 疑似列である
	 *		</UL>
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @param	scope_
	 *			対象のスケール。SCOPE と同じ値を使用する。
	 * @param	nullable_
	 *			null 値を許す列を含む。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。各行は列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getBestRowIdentifier(String	catalog_,
												   String	schema_,
												   String	table_,
												   int		scope_,
												   boolean	nullable_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] catalog_, schema_ は無視。

		// テーブル名のチェック
		// （テーブルが存在しなければ空の結果セットを返す。）
		java.sql.ResultSet	rs = this.getTables(null, null, table_, null);
		boolean	existTable = rs.next();
		rs.close();
		if (existTable == false) return new MetaDataResultSet();

		java.sql.Statement	statement = null;
		java.sql.ResultSet	resultSet = null;

		String[]	rowIDMetaData = null;
		DataArrayData	bestRowIdentifiers = new DataArrayData();

		//  System_Column 表には MetaData 列はなし
		if (_connection.getMasterID() > jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION1) {
			try {

				// テーブルが存在すればそのテーブルには列 ROWID はある

				statement = this._connection.createStatement();
				resultSet = statement.executeQuery("select metadata from system_column where name = 'ROWID'");
				if (resultSet.next()) rowIDMetaData = (String[])resultSet.getArray(1).getArray();

			} catch (java.sql.SQLException	sqle) {

				throw sqle;

			} catch (java.lang.Exception	e) {

				// [YET!] なんらかのエラーおよび SQLSTATE を割り当てるべき。
				throw new Unexpected();

			} finally {

				if (resultSet != null) resultSet.close();
				if (statement != null) statement.close();
			}
		}

		bestRowIdentifiers.addElement(this.getBestRowIdentifier(rowIDMetaData));
		return new MetaDataResultSet(bestRowIdentifiers,
									 this.getBestRowIdentifierColumnNames());
	}

	/**
	 * 行の任意の値が変更された場合に、自動的に更新されるテーブルの列に関する
	 * 記述を取得します。順序付けは行われません。
	 * <P>
	 * 各列の説明を次にします。
	 *	<OL>
	 *	<LI><B>SCOPE</B> short =&gt;
	 *		未使用
	 *	<LI><B>COLUMN_NAME</B> String =&gt;
	 *		列名
	 *	<LI><B>DATA_TYPE</B> short =&gt;
	 *		<code>java.sql.Types</code> からの SQL データの型
	 *	<LI><B>TYPE_NAME</B> String =&gt;
	 *		データソース依存の型名
	 *	<LI><B>COLUMN_SIZE</B> int =&gt;
	 *		精度
	 *	<LI><B>BUFFER_LENGTH</B> int =&gt;
	 *		列値のバイト長
	 *	<LI><B>DECIMAL_DIGITS</B> short =&gt;
	 *		スケール
	 *	<LI><B>PSEUDO_COLUMN</B> short =&gt;
	 *		Oracle ROWID のような疑似列
	 *		<UL>
	 *		<LI>versionColumnUnknown -
	 *			疑似列であるか、またはそうでない可能性がある
	 *		<LI>versionColumnNotPseudo - 疑似列ではない
	 *		<LI>versionColumnPseudo - 疑似列である
	 *		</UL>
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。各行は列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getVersionColumns(String	catalog_,
												String	schema_,
												String	table_)
		throws java.sql.SQLException
	{
		return new MetaDataResultSet();
	}

	/**
	 * 指定されたテーブルの主キー列の記述を取得します。
	 * それらは、COLUMN_NAME によって順序付けられます。
	 * <P>
	 * 各主キー列の記述には、次の列があります。
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String =&gt;
	 *		テーブルカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_SCHEM</B> String =&gt;
	 *		テーブルスキーマ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_NAME</B> String =&gt;
	 *		テーブル名
	 *	<LI><B>COLUMN_NAME</B> String =&gt;
	 *		列名
	 *	<LI><B>KEY_SEQ</B> short =&gt;
	 *		主キー中の連番
	 *	<LI><B>PK_NAME</B> String =&gt;
	 *		主キー名
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	schema_
	 *			スキーマ名。
	 *			現在のバージョンでは、このパラメータは無視されます。
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。
	 *			各行は主キー列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getPrimaryKeys(String	catalog_,
											 String	schema_,
											 String	table_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] catalog_, schema_ は無視。

		// テーブル名のチェック
		// （テーブルが存在しなければ空の結果セットを返す。）
		java.sql.ResultSet	rs = this.getTables(null, null, table_, null);
		boolean	existTable = rs.next();
		rs.close();
		if (existTable == false) return new MetaDataResultSet();

		//java.sql.Connection	connection1 = null;
		//java.sql.Statement	statement1 = null;
		java.sql.ResultSet	resultSet1 = null;
		//java.sql.Connection	connection2 = null;
		//java.sql.Statement	statement2 = null;
		java.sql.ResultSet	resultSet2 = null;
		java.sql.Statement	statement = null;

		DataArrayData	keys = new DataArrayData();

		try {

			// 主キー制約の情報を示す列名
			String	primaryKeyColumnName;
			if (_connection.getMasterID() > jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION1) {
				primaryKeyColumnName = "Type";
			} else {
				primaryKeyColumnName = "Category";
			}
			// The value of category which represents Primary Key is different.
			String	primaryKeyCondition;
			if (_connection.getMasterID() >= jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
				primaryKeyCondition = " in (1,3)";
			} else {
				primaryKeyCondition = " = 1";
			}

			String	query =
				"select System_Constraint.Name, " +
				"System_Constraint.ColumnID, System_Table.Name " +
				"from System_Table, System_Constraint " +
				"where System_Table.Name like '" + table_ + "' " +
				"and System_Constraint.ParentID = System_Table.RowID " +
				"and System_Constraint." + primaryKeyColumnName + primaryKeyCondition + ";";
				//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 主キー制約

			//connection1 =
			//	java.sql.DriverManager.getConnection(this._connection.getURL(),
			//										 "",
			//										 "");
			//statement1 = connection1.createStatement();
			//resultSet1 = statement1.executeQuery(query);
			statement = this._connection.createStatement();
			resultSet1 = statement.executeQuery(query);

			boolean	isExist1 = false;

			// 主キーはテーブルにひとつしかないのでループにしない。
			if (resultSet1.next()) {

				isExist1 = true;

				String		keyName = resultSet1.getString(1);
				if (resultSet1.wasNull()) keyName = null;
				Integer[]	columnIDs =
					(Integer[])resultSet1.getArray(2).getArray();
				String		tableName = resultSet1.getString(3);

				if (columnIDs.length == 0) {
					// [YET!] なんらかのエラーおよび SQLSTATE を
					//        割り当てるべき。
					throw new Unexpected();
				}

				StringBuilder	queryBuffer =
					new StringBuilder("select Name from System_Column where");
				for (int i = 0; i < columnIDs.length; i++) {

					queryBuffer.append(" RowID = ");
					queryBuffer.append(columnIDs[i].intValue());
					if (i < columnIDs.length - 1) queryBuffer.append(" or");
				}
				queryBuffer.append(" order by Name;");

				//connection2 =
				//	java.sql.DriverManager.getConnection(
				//		this._connection.getURL(),
				//		"",
				//		"");
				//statement2 = connection2.createStatement();
				//resultSet2 = statement2.executeQuery(queryBuffer.toString());
				resultSet2 = statement.executeQuery(queryBuffer.toString());

				short	sequentialNo = 0;

				boolean	isExist2 = false;

				while (resultSet2.next()) {

					isExist2 = true;

					String	columnName = resultSet2.getString(1);

					DataArrayData	keyInfo =
						this.getPrimaryKeyInfo(tableName,
											   columnName,
											   sequentialNo++,
											   keyName);
					keys.addElement(keyInfo);
				}

				resultSet2.close();
				resultSet2 = null;
				//statement2.close();
				//statement2 = null;

				if (isExist2 == false) {
					// [YET!] なんらかのエラーおよび SQLSTATE を
					//        割り当てるべき。
					throw new Unexpected();
				}

			} // end of if

			if (isExist1 == false) {
				// 主キーがなかったので、空の ResultSet を返す。
				return new MetaDataResultSet();
			}

		} catch (java.sql.SQLException	sqle) {

			throw sqle;

		} catch (java.lang.Exception	e) {

			// [YET!] 何らかのエラーおよび SQLSTATE を割り当てるべき。
			throw new Unexpected();

		} finally {

			if (resultSet2 != null) resultSet2.close();
			//if (statement2 != null) statement2.close();
			//if (connection2 != null) connection2.close();
			if (resultSet1 != null) resultSet1.close();
			//if (statement1 != null) statement1.close();
			//if (connection1 != null) connection1.close();
		}

		return new MetaDataResultSet(keys,
									 this.getPrimaryKeyInfoColumnNames());
	}

	/**
	 * テーブルの外部キー列 (テーブルによってインポートされる主キー) を参照する
	 * 主キー列に関する記述を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、外部キーをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> を返します。
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getExportedKeys(String, String, String)
	 */
	public java.sql.ResultSet getImportedKeys(String	catalog_,
											  String	schema_,
											  String	table_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 外部キーは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 指定されたテーブルの主キー列 (テーブルによってエクスポートされた
	 * 外部キー) を参照する外部キー列に関する記述を取得します。
	 * それらは、 FKTABLE_CAT 、 FKTABLE_SCHEM 、 FKTABLE_NAME 、 KEY_SEQ
	 * によって順序付けられます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、外部キーをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> を返します。
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	table_
	 *			テーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		#getImportedKeys(String, String, String)
	 */
	public java.sql.ResultSet getExportedKeys(String	catalog_,
											  String	schema_,
											  String	table_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 外部キーは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 指定された主キーテーブルの主キー列を参照する指定された
	 * 外部のキーテーブル中の、外部のキー列に関する記述
	 * (テーブルが別のキーをインポートする方法を記述) を取得します。
	 * ほとんどのテーブルは、テーブルから外部キーを一度だけインポートするため、
	 * これは通常、単一の外部キー/主キーのペアを返さなければなりません。
	 * それらは、 FKTABLE_CAT 、 FKTABLE_SCHEM 、 FKTABLE_NAME 、 KEY_SEQ
	 * によって順序付けられます。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、外部キーをサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> を返します。
	 *
	 * @param	primaryCatalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、選択条件からカタログ名を除外することを
	 *			意味する。
	 * @param	primarySchema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、選択条件からスキーマ名を除外することを
	 *			意味する。
	 * @param	primaryTable_
	 *			キーをエクスポートするテーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @param	foreignCatalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、選択条件からカタログ名を除外することを
	 *			意味する。
	 * @param	foreignSchema_
	 *			スキーマ名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、選択条件からスキーマ名を除外することを
	 *			意味する。
	 * @param	foreignTable_
	 *			キーをインポートするテーブル名。
	 *			データベースに格納されたテーブル名と一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getCrossReference(String	primaryCatalog_,
												String	primarySchema_,
												String	primaryTable_,
												String	foreignCatalog_,
												String	foreignSchema_,
												String	foreignTable_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 外部キーは未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースでサポートされているすべての標準 SQL の型に関する記述を
	 * 取得します。それらは、DATA_TYPE によって順序付けされ、次いで、
	 * 対応する JDBC SQL の型に割り当てるデータの型にどの程度近いかによって
	 * 順序付けされます。
	 * <P>
	 * 各型の記述には次の列があります。
	 *	<OL>
	 *	<LI><B>TYPE_NAME</B> String =&gt;
	 *		型名
	 *	<LI><B>DATA_TYPE</B> short =&gt;
	 *		<code>java.sql.Types</code> からの SQL データの型
	 *	<LI><B>PRECISION</B> int =&gt;
	 *		最大の精度
	 *		(現在のバージョンでは常に <code>0</code> である)
	 *	<LI><B>LITERAL_PREFIX</B> String =&gt;
	 *		リテラルを引用するのに使用する接頭辞
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>LITERAL_SUFFIX</B> String =&gt;
	 *		リテラルを引用するのに使用する接尾辞
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>CREATE_PARAMS</B> String =&gt;
	 *		型の作成に使用するパラメータ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>NULLABLE</B> short =&gt;
	 *		この型に <code>NULL</code> を使用できるか
	 *		<UL>
	 *		<LI>typeNoNulls - <code>NULL</code> 値を許さない
	 *		<LI>typeNullable - <code>NULL</code> 値を許す
	 *		<LI>typeNullableUnknown - <code>NULL</code> 値を許すかどうかは不明
	 *		</UL>
	 *		(現在のバージョンでは常に <code>typeNullableUnknown</code> である)
	 *	<LI><B>CASE_SENSITIVE</B> boolean=&gt;
	 *		大文字小文字を区別するか
	 *		(常に <code>false</code> である)
	 *	<LI><B>SEARCHABLE</B> short =&gt;
	 *		この型に基づき、<code>"WHERE"</code> を使用できるかどうかを示す値。
	 *		以下のいずれか。
	 *		<UL>
	 *		<LI>typePredNone - サポートしない
	 *		<LI>typePredChar - <code>WHERE .. LIKE</code> でだけサポートされる
	 *		<LI>typePredBasic -
	 *			<code>WHERE .. LIKE</code> 以外に対しサポートされる
	 *		<LI>typeSearchable -
	 *			すべての <code>WHERE ..</code> でサポートされる
	 *		</UL>
	 *	<LI><B>UNSIGNED_ATTRIBUTE</B> boolean =&gt;
	 *		符号なしか
	 *	<LI><B>FIXED_PREC_SCALE</B> boolean =&gt;
	 *		通貨の値になれるか
	 *	<LI><B>AUTO_INCREMENT</B> boolean =&gt;
	 *		自動インクリメントの値に使用できるか
	 *		(現在のバージョンでは常に <code>false</code> である)
	 *	<LI><B>LOCAL_TYPE_NAME</B> String =&gt;
	 *		型名の地域対応されたバージョン
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>MINIMUM_SCALE</B> short =&gt;
	 *		サポートされる最小スケール
	 *		(現在のバージョンでは常に <code>0</code> である)
	 *	<LI><B>MAXIMUM_SCALE</B> short =&gt;
	 *		サポートされる最大スケール
	 *		(現在のバージョンでは常に <code>0</code> である)
	 *	<LI><B>SQL_DATA_TYPE</B> int =&gt;
	 *		未使用
	 *	<LI><B>SQL_DATETIME_SUB</B> int =&gt;
	 *		未使用
	 *	<LI><B>NUM_PREC_RADIX</B> int =&gt;
	 *		通常は、2 または 10
	 *	</OL>
	 *
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。
	 *			各行は SQL の型の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getTypeInfo() throws java.sql.SQLException
	{
		DataArrayData	types = new DataArrayData();

		DataArrayData	typeInfo;

		// BINARY (java.sql.Types.BINARY = -2)
		typeInfo = this.getTypeInfo("BINARY",
									java.sql.Types.BINARY,
									java.sql.DatabaseMetaData.typePredNone,
									false);
		types.addElement(typeInfo);

		// CHAR (java.sql.Types.CHAR = 1)
		typeInfo = this.getTypeInfo("CHAR",
									java.sql.Types.CHAR,
									java.sql.DatabaseMetaData.typeSearchable,
									false);
		types.addElement(typeInfo);

		// NCHAR (java.sql.Types.CHAR = 1)
		typeInfo = this.getTypeInfo("NCHAR",
									java.sql.Types.CHAR,
									java.sql.DatabaseMetaData.typeSearchable,
									false);
		types.addElement(typeInfo);

		// INT (java.sql.Types.INTEGER = 4)
		typeInfo = this.getTypeInfo("INT",
									java.sql.Types.INTEGER,
									java.sql.DatabaseMetaData.typePredBasic,
									true); // 通貨の値になれる
		types.addElement(typeInfo);

		// Decimal type is supported from v15.0
		if (_connection.getMasterID() > jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION1) {
			// BIGINT (java.sql.Types.BIGINT = -5)
			typeInfo = this.getTypeInfo("BIGINT",
										java.sql.Types.BIGINT,
										java.sql.DatabaseMetaData.typePredBasic,
										true); // 通貨の値になれる
			types.addElement(typeInfo);
		}

		// Decimal type is supported from v16.1
		if (_connection.getMasterID() >= jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION4) {
			// DECIMAL (java.sql.Types.DECIMAL = 3)
			typeInfo = this.getTypeInfo("DECIMAL",
										java.sql.Types.DECIMAL,
										java.sql.DatabaseMetaData.typePredBasic,
										false); // 通貨の値になれない
			types.addElement(typeInfo);
		}

		// FLOAT (java.sql.Types.DOUBLE = 8)
		typeInfo = this.getTypeInfo("FLOAT",
									java.sql.Types.DOUBLE,
									java.sql.DatabaseMetaData.typePredBasic,
									false); // 通貨の値になれない
		types.addElement(typeInfo);

		// VARCHAR (java.sql.Types.VARCHAR = 12)
		typeInfo = this.getTypeInfo("VARCHAR",
									java.sql.Types.VARCHAR,
									java.sql.DatabaseMetaData.typeSearchable,
									false);
		types.addElement(typeInfo);

		// DATETIME (java.sql.Types.TIME = 92)
		typeInfo = this.getTypeInfo("DATETIME",
									java.sql.Types.TIME,
									java.sql.DatabaseMetaData.typePredBasic,
									false);
		types.addElement(typeInfo);

		return new MetaDataResultSet(types,
									 this.getTypeInfoColumnNames());
	}

	/**
	 * 指定されたテーブルのインデックスと統計情報に関する記述を取得します。
	 * それらは、 NON_UNIQUE 、 TYPE 、 INDEX_NAME 、 ORDINAL_POSITION
	 * によって順序付けされます。
	 * <P>
	 * 各インデックス列の記述には次の列があります。
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String =&gt;
	 *		テーブルカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_SCHEM</B> String =&gt;
	 *		テーブルスキーマ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>TABLE_NAME</B> String =&gt;
	 *		テーブル名
	 *	<LI><B>NON_UNIQUE</B> boolean =&gt;
	 *		インデックス値は一意でない値にできるか
	 *	<LI><B>INDEX_QUALIFIER</B> String =&gt;
	 *		インデックスカタログ
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>INDEX_NAME</B> String =&gt;
	 *		インデックス名
	 *	<LI><B>TYPE</B> short =&gt;
	 *		インデックスの型
	 *		(現在のバージョンでは常に
	 *		 <code>java.sql.DatabaseMetaData.tableIndexOther</code> である)
	 *	<LI><B>ORDINAL_POSITION</B> short =&gt;
	 *		インデックス中の列シーケンス ( 1 から始まる)
	 *	<LI><B>COLUMN_NAME</B> String =&gt;
	 *		列名
	 *	<LI><B>ASC_OR_DESC</B> String =&gt;
	 *		列ソートシーケンス、"A" =&gt; 昇順、"D" =&gt; 降順、
	 *		ソートシーケンスがサポートされていない場合は、
	 *		<code>null</code> の可能性がある
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>CARDINALITY</B> int =&gt;
	 *		TYPE が tableIndexStatistic の場合、テーブル中の列数。
	 *		そうでない場合は、インデックス中の一意の値の数
	 *	<LI><B>PAGES</B> int =&gt;
	 *		TYPE が tableIndexStatistic の場合、テーブルで使用されるページ数。
	 *		そうでない場合は、現在のインデックスで使用されるページ数
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	<LI><B>FILTER_CONDITION</B> String =&gt;
	 *		フィルタ条件
	 *		(現在のバージョンでは常に <code>null</code> である)
	 *	</OL>
	 *
	 * @param	catalog_
	 *			カタログ名。このデータベースに格納されたカタログ名と
	 *			一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を
	 *			検索の限定に使用してはならないことを意味する。
	 * @param	schema_
	 *			スキーマ名。このデータベースに格納されたスキーマ名と
	 *			一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を
	 *			検索の限定に使用してはならないことを意味する。
	 * @param	table_
	 *			テーブル名。このデータベースに格納されたテーブル名と
	 *			一致しなければならない。
	 * @param	unique_
	 *			<code>true</code> の場合は、一意の値のインデックスだけを返す。
	 *			<code>false</code> の場合は、一意であるかどうかにかかわらず
	 *			インデックスを返す。
	 * @param	approximate_
	 *			<code>true</code> の場合は、結果は概数またはデータ値から
	 *			外れることもある。
	 *			<code>false</code> の場合は、正確であることが要求される。
	 * @return	<code>java.sql.ResultSet</code> オブジェクト。
	 *			各行はインデックス列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getIndexInfo(String	catalog_,
										   String	schema_,
										   String	table_,
										   boolean	unique_,
										   boolean	approximate_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] catalog_, schema_, approximate_ は無視。

		// テーブル名のチェック
		// （テーブルが存在しなければ空の結果セットを返す。）
		java.sql.ResultSet	rs = this.getTables(null, null, table_, null);
		boolean	existTable = rs.next();
		rs.close();
		if (existTable == false) return new MetaDataResultSet();

		//java.sql.Connection	connection = null;
		java.sql.Statement	statement = null;
		java.sql.ResultSet	resultSet = null;

		boolean	isFirst = true;
		boolean	baseNonUnique = false; // 先頭記述（タプル）は nonUnique か
		boolean	existBoth = false;	// unique も nonUnique もあったかどうか
		boolean	sortIsNeccessary = false; // NON_UNIQUE でソートが必要か

		DataArrayData	indexes = new DataArrayData();

		try {

			String	query1 =
				"select System_Index.Flag, System_Index.RowID, " +
				"System_Index.Name, System_Table.Name " +
				"from System_Index, System_Table " +
				"where System_Table.Name like '" + table_ + "' " +
				"and System_Index.ParentID = System_Table.RowID " +
				"order by System_Index.Name;";

			//connection =
			//	java.sql.DriverManager.getConnection(this._connection.getURL(),
			//										 "",
			//										 "");
			//statement = connection.createStatement();
			statement = this._connection.createStatement();
			resultSet = statement.executeQuery(query1);

			java.util.Vector	indexIDs = new java.util.Vector();
			java.util.Vector	indexNames = new java.util.Vector();
			java.util.Vector	tableNames = new java.util.Vector();
			java.util.Vector	nonUniques = new java.util.Vector();

			while (resultSet.next()) {

				int		indexFlag = resultSet.getInt(1);
				// System_Index.Flag の最下位ビットが unique のビット。
				boolean	nonUnique = ((indexFlag & 0x01) == 0);
				if (isFirst) {
					isFirst = false;
					baseNonUnique = nonUnique;
				} else if (sortIsNeccessary == false) {
					if ((nonUnique != baseNonUnique && baseNonUnique == false)
						|| existBoth) {
							sortIsNeccessary = true;
					}
				}
				if (unique_ == false || nonUnique == false) {

					int	indexID = resultSet.getInt(2);
					indexIDs.add(new Integer(indexID));
					String	indexName = resultSet.getString(3);
					indexNames.add(indexName);
					String	tableName = resultSet.getString(4);
					tableNames.add(tableName);
					nonUniques.add(new Boolean(nonUnique));
				}
			}

			resultSet.close();

			// インデックスがなかったら空の ResultSet を返す。
			int	numberOfIndexIDs = indexIDs.size();
			if (numberOfIndexIDs == 0) {
				statement.close();
				return new MetaDataResultSet();
			}

			for (int i = 0; i < numberOfIndexIDs; i++) {

				int	indexID = ((Integer)indexIDs.elementAt(i)).intValue();
				String	query2 =
					"select System_Column.Name " +
					"from System_Column, System_Key " +
					"where System_Key.ParentID = " +
					Integer.toString(indexID) + " " +
					"and System_Key.ColumnID = System_Column.RowID " +
					"order by System_Key.Position;";

				resultSet = statement.executeQuery(query2);

				short	ordinalPosition = 1;

				while (resultSet.next()) {

					String	columnName = resultSet.getString(1);

					boolean	nonUnique =
						((Boolean)nonUniques.elementAt(i)).booleanValue();
					DataArrayData	indexInfo =
						this.getIndexInfo((String)tableNames.elementAt(i),
										  nonUnique,
										  (String)indexNames.elementAt(i),
										  ordinalPosition++,
										  columnName,
										  i);	// <- cardinality
					indexes.addElement(indexInfo);
				}

				resultSet.close();
			}

			// unique_ == false の場合には NON_UNIQUE でソート。
			if (unique_ == false && sortIsNeccessary) {
				indexes = sortIndexInfoByNonUnique(indexes);
			}

		} catch (java.sql.SQLException	sqle) {

			throw sqle;

		} catch (java.lang.Exception	e) {

			// [YET!] 何らかのエラーおよび SQLSTATE を割り当てるべき。
			throw new Unexpected();

		} finally {

			if (resultSet != null) resultSet.close();
			if (statement != null) statement.close();
			//if (connection != null) connection.close();
		}

		return new MetaDataResultSet(indexes,
									 this.getIndexInfoColumnNames());
	}

	/**
	 * このデータベースが、指定された結果セットの型をサポートするかどうかを
	 * 取得します。
	 * <P>
	 * 現在のバージョンでは、
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> のみを
	 * サポートします。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> で定義されている型。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsResultSetType(int	type_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 結果セットの型は
		//                  java.sql.ResultSet.TYPE_FORWARD_ONLY
		//                  のみをサポート。
		return (type_ == java.sql.ResultSet.TYPE_FORWARD_ONLY);
	}

	/**
	 * このデータベースが、指定された結果セットの型と与えられた並行処理の
	 * 種類の組み合わせをサポートするかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * <code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> と、
	 * <code>java.sql.ResultSet.CONCUR_READ_ONLY</code> の組み合わせのみを
	 * サポートします。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> で定義されている型
	 * @param	concurrency_
	 *			<code>java.sql.ResultSet</code> で定義されている型
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsResultSetConcurrency(int	type_,
												int	concurrency_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 結果セットの型は
		//                  java.sql.ResultSet.TYPE_FORWARD_ONLY
		//                  のみを、並行処理の種類は
		//                  java.sql.ResultSet.CONCUR_READ_ONLY
		//                  のみをサポート。
		return
			(type_ == java.sql.ResultSet.TYPE_FORWARD_ONLY &&
			 concurrency_ == java.sql.ResultSet.CONCUR_READ_ONLY);
	}

	/**
	 * 結果セット自身の更新が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、結果セットの更新をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean ownUpdatesAreVisible(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 結果セットの更新は未サポート。
		return false;
	}

	/**
	 * 結果セット自身の削除が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、結果セットの削除をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean ownDeletesAreVisible(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 結果セットの削除は未サポート。
		return false;
	}

	/**
	 * 結果セット自身の挿入が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、結果セットの挿入をサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean ownInsertsAreVisible(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 結果セットの挿入は未サポート。
		return false;
	}

	/**
	 * 他で行われた更新が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視ではないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean othersUpdatesAreVisible(int	type_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 可視ではない。
		return false;
	}

	/**
	 * 他で行われた削除が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視ではないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean othersDeletesAreVisible(int	type_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 可視ではない。
		return false;
	}

	/**
	 * 他で行われた挿入が可視かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視ではないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean othersInsertsAreVisible(int	type_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 可視ではない。
		return false;
	}

	/**
	 * <code>java.sql.ResultSet.rowUpdated</code> メソッドを
	 * 呼び出すことによって可視の行が更新されたことを検出できるかどうかを
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が更新されたことを
	 * 検出できないため、常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean updatesAreDetected(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 検出できない。
		return false;
	}

	/**
	 * <code>java.sql.ResultSet.rowDeleted</code> メソッドを
	 * 呼び出すことによって可視の行が削除されたことを検出できるかどうかを
	 * 取得します。 <code>deletesAreDetected</code> メソッドが
	 * <code>false</code> を返す場合は、
	 * 削除された行が結果セットから除去されることを意味します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が削除されたことを
	 * 検出できないため、常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean deletesAreDetected(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 検出できない。
		return false;
	}

	/**
	 * <code>java.sql.ResultSet.rowInserted</code> メソッドを
	 * 呼び出すことによって可視の行が挿入されたことを検出できるかどうかを
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、可視の行が挿入されたことを
	 * 検出できないため、常に <code>false</code> を返します。
	 *
	 * @param	type_
	 *			<code>java.sql.ResultSet</code> の型。
	 *			<code>java.sql.ResultSet.TYPE_FORWARD_ONLY</code> 、
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_INSENSITIVE</code> 、
	 *			または
	 *			<code>java.sql.ResultSet.TYPE_SCROLL_SENSITIVE</code>
	 *			のうちの 1 つ。
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean insertsAreDetected(int	type_) throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 検出できない。
		return false;
	}

	/**
	 * このデータベースによってバッチ更新がサポートされるかどうかを取得します。
	 *
	 * @return	常に <code>true</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsBatchUpdates() throws java.sql.SQLException
	{
		return true;
	}

	/**
	 * 特定のスキーマで定義されているユーザ定義型 (UDT) の説明を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ユーザ定義型をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。
	 *			データベースに格納されたカタログ名と一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	schemaPattern_
	 *			スキーマパターン名。
	 *			データベースに格納されたスキーマ名と一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に使用しては
	 *			ならないことを意味する。
	 * @param	typeNamePattern_
	 *			型名パターン。
	 *			データベースに格納された型名と一致しなければならない。
	 *			完全指定名の可能性がある。
	 * @param	types_
	 *			ユーザ定義型のリスト
	 *			( <code>java.sql.Types.JAVA_OBJECT</code> 、
	 *			<code>java.sql.Types.STRUCT</code> 、
	 *			または <code>java.sql.Types.DISTINCT</code> を含む)。
	 *			<code>null</code> の場合はすべての型を返す。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getUDTs(String	catalog_,
									  String	schemaPattern_,
									  String	typeNamePattern_,
									  int[]		types_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ユーザ定義型は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このメタデータオブジェクトを生成した接続 (コネクションオブジェクト) を
	 * 取り出します。
	 *
	 * @return	このメタデータオブジェクトを生成した
	 *			接続 (コネクションオブジェクト) 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.Connection getConnection() throws java.sql.SQLException
	{
		return this._connection;
	}

	/**
	 * このデータベースによってセーブポイントがサポートされるかどうかを
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、セーブポイントをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsSavepoints() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] セーブポイントは未サポート。
		return false;
	}

	/**
	 * このデータベースによって、 <code>callable</code> 文への
	 * 名前付きパラメータがサポートされるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsNamedParameters() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return false;
	}

	/**
	 * <code>java.sql.CallableStatement</code> オブジェクトから同時に返された
	 * 複数の <code>java.sql.ResultSet</code> オブジェクトを持つことが
	 * 可能かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ストアドプロシージャをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsMultipleOpenResults() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ストアドプロシージャは未サポート。
		return false;
	}

	/**
	 * 文が実行されたあとに自動生成キーを取得できるかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、自動生成キーをサポートしていないため、
	 * 常に <code>false</code> を返します。
	 *
	 * @return	現在のバージョンでは、常に <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsGetGeneratedKeys() throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 自動生成キーは未サポート。
		return false;
	}

	/**
	 * このデータベースの特定のスキーマで定義されている
	 * ユーザ定義型 (UDT) 階層の説明を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ユーザ定義型をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。 "" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、選択条件からカタログ名を
	 *			除外することを意味する。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。 "" はスキーマなしでスキーマ名を検索する。
	 * @param	typeNamePattern_
	 *			UDT 名パターン。完全指定名の可能性がある。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getSuperTypes(String	catalog_,
											String	schemaPattern_,
											String	typeNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ユーザ定義型および型の階層は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースの特定のスキーマで定義されている
	 * テーブル階層の説明を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、型の階層をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。 "" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、選択条件からカタログ名を除外することを
	 *			意味する。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。 "" はスキーマなしでスキーマ名を検索する。
	 * @param	tableNamePattern_
	 *			テーブル名パターン。完全指定名の可能性がある。
	 * @return	現在のバージョンでは、常に空の <code>java.sql.ResultSet</code>
	 *			オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getSuperTables(String	catalog_,
											 String	schemaPattern_,
											 String	tableNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 型の階層は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * 指定されたスキーマおよびカタログで使用可能な
	 * ユーザ定義の型 (UDT) のための指定された型の指定された属性に関する記述を
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、ユーザ定義型をサポートしていないため、
	 * 常に空の <code>java.sql.ResultSet</code> オブジェクトを返します。
	 *
	 * @param	catalog_
	 *			カタログ名。データベースに格納されたカタログ名と
	 *			一致しなければならない。
	 *			"" はカタログなしでカタログ名を検索する。
	 *			<code>null</code> は、カタログ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	schemaPattern_
	 *			スキーマ名パターン。データベースに格納されたスキーマ名と
	 *			一致しなければならない。
	 *			"" はスキーマなしでスキーマ名を検索する。
	 *			<code>null</code> は、スキーマ名を検索の限定に
	 *			使用してはならないことを意味する。
	 * @param	typeNamePattern_
	 *			型名パターン。データベースに格納された型名と
	 *			一致しなければならない。
	 * @param	attributeNamePattern_
	 *			属性名パターン。データベースで宣言された属性名と
	 *			一致しなければならない。
	 * @return	現在のバージョンでは、
	 *			常に空の <code>java.sql.ResultSet</code> オブジェクト。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSet getAttributes(String	catalog_,
											String	schemaPattern_,
											String	typeNamePattern_,
											String	attributeNamePattern_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] ユーザ定義型は未サポート。
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースが、指定された結果セットの保持機能を
	 * サポートするかどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、
	 * <code>java.sql.ResultSet.CLOSE_CURSURS_AT_COMMIT</code> のみを
	 * サポートします。
	 *
	 * @param	holdability_
	 *			保持機能。
	 *			<code>java.sql.ResultSet.HOLD_CURSORS_OVER_COMMIT</code>
	 *			または
	 *			<code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> 。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsResultSetHoldability(int	holdability_)
		throws java.sql.SQLException
	{
		// [NOT SUPPORTED!] 保持機能は、
		//                  java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT
		//                  のみをサポート。
		return holdability_ == java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT;
	}

	/**
	 * <code>java.sql.ResultSet</code> オブジェクトの
	 * デフォルトの保持機能を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、常に
	 * <code>java.sql.ResultSet.CLOSE_CUSORS_AT_COMMIT</code> を返します。
	 *
	 * @return	常に <code>java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getResultSetHoldability() throws java.sql.SQLException
	{
		return java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT;
	}

	/**
	 * <B>[サポート外]</B>
	 * 基本となるデータベースのメジャーバージョンを取得します。
	 *
	 * @return	基本となるデータベースのメジャーバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getDatabaseMajorVersion() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * 基本となるデータベースのマイナーバージョンを取得します。
	 *
	 * @return	基本となるデータベースのマイナーバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getDatabaseMinorVersion() throws java.sql.SQLException
	{
		throw new NotSupported();
	}

	/**
	 * このドライバの JDBC メジャーバージョンを取得します。
	 *
	 * @return	JDBC メジャーバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getJDBCMajorVersion() throws java.sql.SQLException
	{
		// 一応 JDBC 4.0 対応
		return 4;
	}

	/**
	 * このドライバの JDBC マイナーバージョンを取得します。
	 *
	 * @return	JDBC マイナーバージョン。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getJDBCMinorVersion() throws java.sql.SQLException
	{
		// 一応 JDBC 4.0 対応
		return 0;
	}

	/**
	 * <code>SQLException.getSQLState</code> によって返される SQLSTATE が
	 *	X/Open (現在は Open Group) の SQL CLI であるか SQL99 であるかを
	 * 示します。
	 *
	 * @return	SQLSTATE の型。 常に <code>sqlStateSQL99</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getSQLStateType() throws java.sql.SQLException
	{
		return java.sql.DatabaseMetaData.sqlStateSQL99;
	}

	/**
	 * <B>[サポート外]</B>
	 * LOB への変更が、コピーに対して行われたのか、
	 * LOB に直接行われたのかを示します。
	 *
	 * @return	変更が LOB のコピーに対して行われた場合は <code>true</code> 、
	 *			LOB に直接行われた場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean locatorsUpdateCopy() throws java.sql.SQLException
	{
		// [YET!] まだ未実装。
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * このデータベースによって文のプールがサポートされるかどうかを取得します。
	 *
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean supportsStatementPooling() throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * <B>[JDBCに含まれない関数]</B>
	 * 接続中のプロトコルバージョンを取得します。
	 *
	 * @return	プロトコルバージョン
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getProtocolVersion() throws java.sql.SQLException
	{
		return _connection.getMasterID();
	}

	/**
	 * システムデータベースへの接続を取得します。
	 *
	 * @return	システムデータベースへの接続。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	private java.sql.Connection connectSystemDatabase()
		throws java.sql.SQLException
	{
		String	url = this._connection.getURL();
		// URL のうち、データベース名のみをシステムデータベースに変える。
		String	systemDatabaseUrl =
			url.substring(0, url.lastIndexOf("/") + 1) + "$$SystemDB";
		return java.sql.DriverManager.getConnection(systemDatabaseUrl, getUserName(), this._connection.getPassword());
	}

	/**
	 * <code>getTables</code> メソッドによって返される結果セットの各列の名前を
	 * 返します。
	 *
	 * @return	<code>getTables</code> メソッドによって返される結果セットの
	 *			各列の名前。
	 */
	private StringArrayData getTableInfoColumnNames()
	{
		if (_cnGetTables == null) {

			_cnGetTables = new StringArrayData(10);
			_cnGetTables.setElement(0, "TABLE_CAT");
			_cnGetTables.setElement(1, "TABLE_SCHEM");
			_cnGetTables.setElement(2, "TABLE_NAME");
			_cnGetTables.setElement(3, "TABLE_TYPE");
			_cnGetTables.setElement(4, "REMARKS");
			_cnGetTables.setElement(5, "TYPE_CAT");
			_cnGetTables.setElement(6, "TYPE_SCHEM");
			_cnGetTables.setElement(7, "TYPE_NAME");
			_cnGetTables.setElement(8, "SELF_REFERENCING_COL_NAME");
			_cnGetTables.setElement(9, "REF_GENERATION");
		}

		return _cnGetTables;
	}

	/**
	 * <code>getTables</code> メソッドによって返される結果セットのうち、
	 * ひとつのテーブルに関する情報を返します。
	 *
	 * @param	tableName_
	 *			テーブル名。
	 * @return	<code>getTables</code> メソッドによって返される
	 *			結果セットのうちのひとつのテーブルに関する情報。
	 */
	private DataArrayData getTableInfo(String	tableName_)
	{
		DataArrayData	tableInfo = new DataArrayData(10);

		// [YET!] （現状ではしかたないが、）テーブル名以外、ほとんど null ！！
		// [YET!] TABLE_TYPE は "TABLE" 固定で構わないか？

		// TABLE_CAT
		tableInfo.setElement(0, new NullData());
		// TABLE_SCHEM
		tableInfo.setElement(1, new NullData());
		// TABLE_NAME
		tableInfo.setElement(2, new StringData(tableName_));
		// TABLE_TYPE
		tableInfo.setElement(3, new StringData("TABLE"));
		// REMARKS
		tableInfo.setElement(4, new StringData(""));
		// TYPE_CAT
		tableInfo.setElement(5, new NullData());
		// TYPE_SCHEM
		tableInfo.setElement(6, new NullData());
		// TYPE_NAME
		tableInfo.setElement(7, new NullData());
		// SELF_REFERENCING_COL_NAME
		tableInfo.setElement(8, new NullData());
		// REF_GENERATION
		tableInfo.setElement(9, new NullData());

		return tableInfo;
	}

	/**
	 * <code>getColumns</code> メソッドによって返される結果セットの各列の名前を
	 * 返します。
	 *
	 * @return	<code>getColumns</code> メソッドによって返される結果セットの
	 *			各列の名前。
	 */
	private StringArrayData getColumnInfoColumnNames()
	{
		if (_cnGetColumns == null) {

			_cnGetColumns = new StringArrayData(22);
			_cnGetColumns.setElement( 0, "TABLE_CAT");
			_cnGetColumns.setElement( 1, "TABLE_SCHEM");
			_cnGetColumns.setElement( 2, "TABLE_NAME");
			_cnGetColumns.setElement( 3, "COLUMN_NAME");
			_cnGetColumns.setElement( 4, "DATA_TYPE");
			_cnGetColumns.setElement( 5, "TYPE_NAME");
			_cnGetColumns.setElement( 6, "COLUMN_SIZE");
			_cnGetColumns.setElement( 7, "BUFFER_LENGTH");
			_cnGetColumns.setElement( 8, "DECIMAL_DIGITS");
			_cnGetColumns.setElement( 9, "NUM_PREC_RADIX");
			_cnGetColumns.setElement(10, "NULLABLE");
			_cnGetColumns.setElement(11, "REMARKS");
			_cnGetColumns.setElement(12, "COLUMN_DEF");
			_cnGetColumns.setElement(13, "SQL_DATA_TYPE");
			_cnGetColumns.setElement(14, "SQL_DATETIME_SUB");
			_cnGetColumns.setElement(15, "CHAR_OCTET_LENGTH");
			_cnGetColumns.setElement(16, "ORDINAL_POSITION");
			_cnGetColumns.setElement(17, "IS_NULLABLE");
			_cnGetColumns.setElement(18, "SCOPE_CATLOG");
			_cnGetColumns.setElement(19, "SCOPE_SCHEMA");
			_cnGetColumns.setElement(20, "SCOPE_TABLE");
			_cnGetColumns.setElement(21, "SOURCE_DATA_TYPE");
		}

		return _cnGetColumns;
	}

	/**
	 * <code>getColumns</code> メソッドによって返される結果セットのうち、
	 * ひとつの列に関する情報を返します。
	 *
	 * @param	tableName_
	 *			テーブル名。
	 * @param	columnName_
	 *			列名。
	 * @param	columnPosition_
	 *			テーブル中の列のインデックス ( 1 から始まる) 。
	 * @param	columnFlag_
	 *			列の属性。最下位ビットが NULL 可／不可。
	 * @param	columnMetaData_
	 *			列のメタデータ。
	 * @return	<code>getColumns</code> メソッドによって返される
	 *			結果セットのうちのひとつの列に関する情報。
	 */
	private DataArrayData getColumnInfo(String		tableName_,
										String		columnName_,
										int			columnPosition_,
										int			columnFlag_,
										String[]	columnMetaData_)
	{
		// NULL の可／不可の設定
		int		iNullable = 0;
		String	sNullable = "";
		if ((columnFlag_ & 0x1) == 0) {
			// 偶数
			iNullable = java.sql.DatabaseMetaData.columnNullable;
			sNullable = "YES";
		} else {
			// 奇数
			iNullable = java.sql.DatabaseMetaData.columnNoNulls;
			sNullable = "NO";
		}

		DataArrayData	columnInfo = new DataArrayData(22);

		// TABLE_CAT
		columnInfo.setElement(0, new NullData());
		// TABLE_SCHEM
		columnInfo.setElement(1, new NullData());
		// TABLE_NAME
		columnInfo.setElement(2, new StringData(tableName_));
		// COLUMN_NAME
		columnInfo.setElement(3, new StringData(columnName_));
		// DATA_TYPE
		String	arraySymbol = "";
		if (Integer.parseInt(columnMetaData_[4]) == 0) {
			// 配列型ではない
			columnInfo.setElement(4, new IntegerData(ResultSetMetaData.getJDBCType(Integer.parseInt(columnMetaData_[0]))));
		} else {
			// 配列型
			columnInfo.setElement(4, new IntegerData(java.sql.Types.ARRAY));
			arraySymbol = " array";
		}
		// TYPE_NAME
		columnInfo.setElement(5, new StringData(columnMetaData_[1] + arraySymbol));
		// COLUMN_SIZE
		columnInfo.setElement(6, new IntegerData(Integer.parseInt(columnMetaData_[2])));
		// BUFFER_LENGTH - 未使用
		columnInfo.setElement(7, new NullData());
		// DECIMAL_DIGITS
		columnInfo.setElement(8, new IntegerData(Integer.parseInt(columnMetaData_[3])));
		// NUM_PREC_RADIX
		columnInfo.setElement(9, new IntegerData(10));
		// NULLABLE
		columnInfo.setElement(10, new IntegerData(iNullable));
		// REMARKS
		if (columnMetaData_[6] == null) {
			columnInfo.setElement(11, new NullData());
		} else {
			columnInfo.setElement(11, new StringData(columnMetaData_[6]));
		}
		// COLUMN_DEF
		if (columnMetaData_[5] == null) {
			columnInfo.setElement(12, new NullData());
		} else {
			columnInfo.setElement(12, new StringData(columnMetaData_[5]));
		}
		// SQL_DATA_TYPE - 未使用
		columnInfo.setElement(13, new NullData());
		// SQL_DATETIME_SUB - 未使用
		columnInfo.setElement(14, new NullData());
		// CHAR_OCTET_LENGTH
		switch (ResultSetMetaData.getJDBCType(Integer.parseInt(columnMetaData_[0]))) {
		case java.sql.Types.CHAR:
		case java.sql.Types.VARCHAR:
		case java.sql.Types.CLOB:
			int	len = Integer.parseInt(columnMetaData_[2]);
			if (len == -1) {
				// 無制限長
				columnInfo.setElement(15, new IntegerData(-1));
			} else {
				// 文字数指定あり
				// char, varchar では文字列長（文字数）
				// nchar, nvarchar では文字列長（文字数）の 2 倍
				switch (Integer.parseInt(columnMetaData_[0])) {
				case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER:
				case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER_VARYING:
					len <<= 1;
					break;
				default:
				}
				columnInfo.setElement(15, new IntegerData(len));
			}
			break;
		default:
			columnInfo.setElement(15, new NullData());
		}
		// ORDINAL_POSITION
		columnInfo.setElement(16, new IntegerData(columnPosition_));
		// IS_NULLABLE
		columnInfo.setElement(17, new StringData(sNullable));
		// SCOPE_CATLOG
		columnInfo.setElement(18, new NullData());
		// SCOPE_SCHEMA
		columnInfo.setElement(19, new NullData());
		// SCOPE_TABLE
		columnInfo.setElement(20, new NullData());
		// SOURCE_DATA_TYPE
		columnInfo.setElement(21, new NullData());

		return columnInfo;
	}

	/**
	 * <code>getColumns</code> メソッドによって返される結果セットのうち、
	 * ひとつの列に関する情報を返します。
	 *
	 * @param	tableName_
	 *			テーブル名。
	 * @param	columnName_
	 *			列名。
	 * @param	columnPosition_
	 *			テーブル中の列のインデックス ( 1 から始まる) 。
	 * @return	<code>getColumns</code> メソッドによって返される
	 *			結果セットのうちのひとつの列に関する情報。
	 */
	private DataArrayData getColumnInfo(String	tableName_,
										String	columnName_,
										int		columnPosition_)
	{
		DataArrayData	columnInfo = new DataArrayData(22);

		// TABLE_CAT
		columnInfo.setElement(0, new NullData());
		// TABLE_SCHEM
		columnInfo.setElement(1, new NullData());
		// TABLE_NAME
		columnInfo.setElement(2, new StringData(tableName_));
		// COLUMN_NAME
		columnInfo.setElement(3, new StringData(columnName_));
		// DATA_TYPE
		columnInfo.setElement(4, new NullData());
		// TYPE_NAME
		columnInfo.setElement(5, new NullData());
		// COLUMN_SIZE
		columnInfo.setElement(6, new NullData());
		// BUFFER_LENGTH - 未使用
		columnInfo.setElement(7, new NullData());
		// DECIMAL_DIGITS
		columnInfo.setElement(8, new NullData());
		// NUM_PREC_RADIX
		columnInfo.setElement(9, new NullData());
		// NULLABLE
		columnInfo.setElement(
			10,
			new IntegerData(java.sql.DatabaseMetaData.columnNullableUnknown));
		// REMARKS
		columnInfo.setElement(11, new NullData());
		// COLUMN_DEF
		columnInfo.setElement(12, new NullData());
		// SQL_DATA_TYPE - 未使用
		columnInfo.setElement(13, new NullData());
		// SQL_DATETIME_SUB - 未使用
		columnInfo.setElement(14, new NullData());
		// CHAR_OCTET_LENGTH
		columnInfo.setElement(15, new NullData());
		// ORDINAL_POSITION
		columnInfo.setElement(16, new IntegerData(columnPosition_));
		// IS_NULLABLE
		columnInfo.setElement(17, new StringData(""));
		// SCOPE_CATLOG
		columnInfo.setElement(18, new NullData());
		// SCOPE_SCHEMA
		columnInfo.setElement(19, new NullData());
		// SCOPE_TABLE
		columnInfo.setElement(20, new NullData());
		// SOURCE_DATA_TYPE
		columnInfo.setElement(21, new NullData());

		return columnInfo;
	}

	/**
	 * <code>getPrimaryKeys</code> メソッドによって返される結果セットの
	 * 各列の名前を返します。
	 *
	 * @return	<code>getPrimaryKeys</code> メソッドによって返される
	 *			結果セットの各列の名前。
	 */
	private StringArrayData getPrimaryKeyInfoColumnNames()
	{
		if (_cnGetPrimaryKeys == null) {

			_cnGetPrimaryKeys = new StringArrayData(6);
			_cnGetPrimaryKeys.setElement(0, "TABLE_CAT");
			_cnGetPrimaryKeys.setElement(1, "TABLE_SCHEM");
			_cnGetPrimaryKeys.setElement(2, "TABLE_NAME");
			_cnGetPrimaryKeys.setElement(3, "COLUMN_NAME");
			_cnGetPrimaryKeys.setElement(4, "KEY_SEQ");
			_cnGetPrimaryKeys.setElement(5, "PK_NAME");
		}

		return _cnGetPrimaryKeys;
	}

	/**
	 * <code>getPrimaryKeys</code> メソッドによって返される結果セットのうち、
	 * ひとつの列に関する情報を返します。
	 *
	 * @param	tableName_
	 *			テーブル名。
	 * @param	columnName_
	 *			列名。
	 * @param	sequentialNo_
	 *			主キー中の連番。
	 * @param	keyName_
	 *			主キー名。
	 * @return	<code>getPrimaryKeys</code> メソッドによって返される
	 *			結果セットのうちのひとつの列に関する情報。
	 */
	private DataArrayData getPrimaryKeyInfo(String	tableName_,
											String	columnName_,
											short	sequentialNo_,
											String	keyName_)
	{
		DataArrayData	primaryKeyInfo = new DataArrayData(6);

		// TABLE_CAT
		primaryKeyInfo.setElement(0, new NullData());
		// TABLE_SCHEM
		primaryKeyInfo.setElement(1, new NullData());
		// TABLE_NAME
		primaryKeyInfo.setElement(2, new StringData(tableName_));
		// COLUMN_NAME
		primaryKeyInfo.setElement(3, new StringData(columnName_));
		// KEY_SEQ
		primaryKeyInfo.setElement(4, new IntegerData((int)sequentialNo_));
		// PK_NAME
		if (keyName_ == null) {
			primaryKeyInfo.setElement(5, new NullData());
		} else {
			primaryKeyInfo.setElement(5, new StringData(keyName_));
		}

		return primaryKeyInfo;
	}

	/**
	 * <code>getTypeInfo</code> メソッドによって返される結果セットの
	 * 各列の名前を返します。
	 *
	 * @return	<code>getTypeInfo</code> メソッドによって返される
	 *			結果セットの各列の名前。
	 */
	private StringArrayData getTypeInfoColumnNames()
	{
		if (_cnGetTypeInfo == null) {

			_cnGetTypeInfo = new StringArrayData(18);
			_cnGetTypeInfo.setElement( 0, "TYPE_NAME");
			_cnGetTypeInfo.setElement( 1, "DATA_TYPE");
			_cnGetTypeInfo.setElement( 2, "PRECISION");
			_cnGetTypeInfo.setElement( 3, "LITERAL_PREFIX");
			_cnGetTypeInfo.setElement( 4, "LITERAL_SUFFIX");
			_cnGetTypeInfo.setElement( 5, "CREATE_PARAMS");
			_cnGetTypeInfo.setElement( 6, "NULLABLE");
			_cnGetTypeInfo.setElement( 7, "CASE_SENSITIVE");
			_cnGetTypeInfo.setElement( 8, "SEARCHABLE");
			_cnGetTypeInfo.setElement( 9, "UNSIGNED_ATTRIBUTE");
			_cnGetTypeInfo.setElement(10, "FIXED_PREC_SCALE");
			_cnGetTypeInfo.setElement(11, "AUTO_INCREMENT");
			_cnGetTypeInfo.setElement(12, "LOCAL_TYPE_NAME");
			_cnGetTypeInfo.setElement(13, "MINIMUM_SCALE");
			_cnGetTypeInfo.setElement(14, "MAXIMUM_SCALE");
			_cnGetTypeInfo.setElement(15, "SQL_DATA_TYPE");
			_cnGetTypeInfo.setElement(16, "SQL_DATETIME_SUB");
			_cnGetTypeInfo.setElement(17, "NUM_PREC_RADIX");
		}

		return _cnGetTypeInfo;
	}

	/**
	 * <code>getTypeInfo</code> メソッドによって返される結果セットのうち、
	 * ひとつの列に関する情報を返します。
	 *
	 * @param	typeName_
	 *			型名。
	 * @param	dataType_
	 *			<code>java.sql.Types</code> からの SQL データの型。
	 * @param	searchable_
	 *			<code>"WHERE"</code> を使用できるかどうかを示す値。
	 *			以下のいずれか。
	 *			<UL>
	 *			<LI>typePredNone - サポートしない
	 *			<LI>typePredChar -
	 *				<code>WHERE .. LIKE</code> でだけサポートされる
	 *			<LI>typePredBasic -
	 *				<code>WHERE .. LIKE</code> 以外に対しサポートされる
	 *			<LI>typeSearchable -
	 *				すべての <code>WHERE ..</code> でサポートされる
	 *			</UL>
	 * @param	fixedPrecScale_
	 *			通貨の値になれるか。
	 * @return	<code>getTypeInfo</code> メソッドによって返される
	 *			結果セットのうちのひとつの列に関する情報。
	 */
	private DataArrayData getTypeInfo(String	typeName_,
									  int		dataType_,
									  int		searchable_,
									  boolean	fixedPrecScale_)
	{
		DataArrayData	typeInfo = new DataArrayData(18);

		// TYPE_NAME
		typeInfo.setElement(0, new StringData(typeName_));
		// DATA_TYPE
		typeInfo.setElement(1, new IntegerData(dataType_));
		// PRECISION
		typeInfo.setElement(2, new IntegerData(0));
		// LITERAL_PREFIX
		typeInfo.setElement(3, new NullData());
		// LITERAL_SUFFIX
		typeInfo.setElement(4, new NullData());
		// CREATE_PARAMS
		typeInfo.setElement(5, new NullData());
		// NULLABLE
		typeInfo.setElement(
			6,
			new IntegerData(java.sql.DatabaseMetaData.typeNullableUnknown));
		// CASE_SENSITIVE
		typeInfo.setElement(7, new IntegerData(0)); // false
		// SEARCHABLE
		typeInfo.setElement(8, new IntegerData(searchable_));
		// UNSIGNED_ATTRIBUTE
		typeInfo.setElement(9, new IntegerData(0)); // false
		// FIXED_PREC_SCALE
		typeInfo.setElement(10, new IntegerData(fixedPrecScale_ ? 1 : 0));
		// AUTO_INCREMENT
		typeInfo.setElement(11, new IntegerData(0)); // false
		// LOCAL_TYPE_NAME
		typeInfo.setElement(12, new NullData());
		// MINIMUM_SCALE
		typeInfo.setElement(13, new IntegerData(0));
		// MAXIMUM_SCALE
		typeInfo.setElement(14, new IntegerData(0));
		// SQL_DATA_TYPE
		typeInfo.setElement(15, new NullData());
		// SQL_DATETIME_SUB
		typeInfo.setElement(16, new NullData());
		// NUM_PREC_RADIX
		typeInfo.setElement(17, new IntegerData(10));

		return typeInfo;
	}

	/**
	 * <code>getIndexInfo</code> メソッドによって返される結果セットの
	 * 各列の名前を返します。
	 *
	 * @return	<code>getIndexInfo</code> メソッドによって返される
	 *			結果セットの各列の名前。
	 */
	private StringArrayData getIndexInfoColumnNames()
	{
		if (_cnGetIndexInfo == null) {

			_cnGetIndexInfo = new StringArrayData(13);
			_cnGetIndexInfo.setElement( 0, "TABLE_CAT");
			_cnGetIndexInfo.setElement( 1, "TABLE_SCHEM");
			_cnGetIndexInfo.setElement( 2, "TABLE_NAME");
			_cnGetIndexInfo.setElement( 3, "NON_UNIQUE");
			_cnGetIndexInfo.setElement( 4, "INDEX_QUALIFIER");
			_cnGetIndexInfo.setElement( 5, "INDEX_NAME");
			_cnGetIndexInfo.setElement( 6, "TYPE");
			_cnGetIndexInfo.setElement( 7, "ORDINAL_POSITION");
			_cnGetIndexInfo.setElement( 8, "COLUMN_NAME");
			_cnGetIndexInfo.setElement( 9, "ASC_OR_DESC");
			_cnGetIndexInfo.setElement(10, "CARDINALITY");
			_cnGetIndexInfo.setElement(11, "PAGES");
			_cnGetIndexInfo.setElement(12, "FILTER_CONDITION");
		}

		return _cnGetIndexInfo;
	}

	/**
	 * <code>getIndexInfo</code> メソッドによって返される結果セットのうち、
	 * ひとつの列に関する情報を返します。
	 *
	 * @param	tableName_
	 *			テーブル名。
	 * @param	nonUnique_
	 *			インデックス値は一意でない値にできるか。
	 * @param	indexName_
	 *			インデックス名。
	 * @param	ordinalPosition_
	 *			インデックス中の列シーケンス。
	 * @param	columnName_
	 *			列名。
	 * @param	cardinality_
	 *			インデックス中の一意の数値。
	 * @return	<code>getIndexinfo</code> メソッドによって返される
	 *			結果セットのうちのひとつの列に関する情報。
	 */
	private DataArrayData getIndexInfo(String	tableName_,
									   boolean	nonUnique_,
									   String	indexName_,
									   short	ordinalPosition_,
									   String	columnName_,
									   int		cardinality_)
	{
		DataArrayData	indexInfo = new DataArrayData(13);

		// TABLE_CAT
		indexInfo.setElement(0, new NullData());
		// TABLE_SCHEM
		indexInfo.setElement(1, new NullData());
		// TABLE_NAME
		indexInfo.setElement(2, new StringData(tableName_));
		// NON_UNIQUE
		indexInfo.setElement(3, new IntegerData(nonUnique_ ? 1 : 0));
		// INDEX_QUALIFIER
		indexInfo.setElement(4, new NullData());
		// INDEX_NAME
		indexInfo.setElement(5, new StringData(indexName_));
		// TYPE
		indexInfo.setElement(
			6,
			new IntegerData(java.sql.DatabaseMetaData.tableIndexOther));
		// ORDINAL_POSITION
		indexInfo.setElement(7, new IntegerData((int)ordinalPosition_));
		// COLUMN_NAME
		indexInfo.setElement(8, new StringData(columnName_));
		// ASC_OR_DESC
		indexInfo.setElement(9, new NullData());
		// CARDINALITY
		indexInfo.setElement(10, new IntegerData(cardinality_));
		// PAGES
		indexInfo.setElement(11, new NullData());
		// FILTER_CONDITION
		indexInfo.setElement(12, new NullData());

		return indexInfo;
	}

	/**
	 * <code>getIndexInfo</code> メソッドによって返される結果セットを、
	 * NON_UNIQUE の値によって true → false の順にソートします。
	 *
 	 * @param	indexes_
	 *			<code>getIndexInfo</code> メソッドによって返される結果セット。
	 * @return	ソート後の
	 *			<code>getIndexInfo</code> メソッドによって返される結果セット。
	 */
	private DataArrayData sortIndexInfoByNonUnique(DataArrayData	indexes_)
	{
		DataArrayData	nonUniqueIndexInfo = new DataArrayData();
		DataArrayData	uniqueIndexInfo = new DataArrayData();

		int	numberOfIndexInfos = indexes_.getCount();
		for (int i = 0; i < numberOfIndexInfos; i++) {
			DataArrayData	indexInfo =
				(DataArrayData)(indexes_.getElement(i));
			if (((IntegerData)indexInfo.getElement(3)).getValue() != 0) {
				nonUniqueIndexInfo.addElement(indexInfo);
			} else {
				uniqueIndexInfo.addElement(indexInfo);
			}
		}

		int	numberOfUniqueIndexInfos = uniqueIndexInfo.getCount();
		for (int i = 0; i < numberOfUniqueIndexInfos; i++) {
			nonUniqueIndexInfo.addElement(uniqueIndexInfo.getElement(i));
		}

		return nonUniqueIndexInfo;
	}

	/**
	 * <code>getBestRowIdentifier</code> メソッドによって返される結果セットの
	 * 各列の名前を返します。
	 *
	 * @return	<code>getBestRowIdentifier</code> メソッドによって返される
	 *			結果セットの各列の名前。
	 */
	private StringArrayData getBestRowIdentifierColumnNames()
	{
		if (_cnGetBestRowIdentifier == null) {

			_cnGetBestRowIdentifier = new StringArrayData(8);
			_cnGetBestRowIdentifier.setElement(0, "SCOPE");
			_cnGetBestRowIdentifier.setElement(1, "COLUMN_NAME");
			_cnGetBestRowIdentifier.setElement(2, "DATA_TYPE");
			_cnGetBestRowIdentifier.setElement(3, "TYPE_NAME");
			_cnGetBestRowIdentifier.setElement(4, "COLUMN_SIZE");
			_cnGetBestRowIdentifier.setElement(5, "BUFFER_LENGTH");
			_cnGetBestRowIdentifier.setElement(6, "DECIMAL_DIGITS");
			_cnGetBestRowIdentifier.setElement(7, "PSEUDO_COLUMN");
		}

		return _cnGetBestRowIdentifier;
	}

	/**
	 * <code>getBestRowIdentifier</code> メソッドによって返される
	 * 結果セットの情報を返します。
	 * @return	<code>getBestRowIdentifier</code> メソッドによって返される
	 *			結果セットの情報。
	 */
	private DataArrayData getBestRowIdentifier(String[]	rowIDMetaData_)
	{
		DataArrayData	bestRowIdentifier = new DataArrayData(8);

		// SCOPE
		bestRowIdentifier.setElement(
			0,
			new IntegerData(java.sql.DatabaseMetaData.bestRowSession));
		// COLUMN_NAME
		bestRowIdentifier.setElement(1, new StringData("ROWID"));
		if (rowIDMetaData_ != null) {
			// DATA_TYPE
			bestRowIdentifier.setElement(2,
										 new IntegerData(ResultSetMetaData.getJDBCType(Integer.parseInt(rowIDMetaData_[0]))));
			// TYPE_NAME
			bestRowIdentifier.setElement(3, new StringData(rowIDMetaData_[1]));
			// COLUMN_SIZE
			bestRowIdentifier.setElement(4, new IntegerData(Integer.parseInt(rowIDMetaData_[2])));
		} else {
			// DATA_TYPE
			bestRowIdentifier.setElement(2,
										 new IntegerData(java.sql.Types.INTEGER));
			// TYPE_NAME
			bestRowIdentifier.setElement(3, new StringData("int"));
			// COLUMN_SIZE
			bestRowIdentifier.setElement(4, new NullData());
		}
		// BUFFER_LENGTH
		bestRowIdentifier.setElement(5, new NullData());
		if (rowIDMetaData_ != null) {
			// DECIMAL_DIGITS
			bestRowIdentifier.setElement(6, new IntegerData(Integer.parseInt(rowIDMetaData_[3])));
		} else {
			// DECIMAL_DIGITS
			bestRowIdentifier.setElement(6, new NullData());
		}
		// PSEUDO_COLUMN
		bestRowIdentifier.setElement(
			7,
			new IntegerData(java.sql.DatabaseMetaData.bestRowPseudo));

		return bestRowIdentifier;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 常に<code>false</code>を返します
	 */
	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		// unwrap を実装している場合は true を返すが、
		// それ以外の場合は false を返す

		return false;
	}

	/**
	 * <code>ROWID</code>型はサポートしていない
	 */
	@Override
	public RowIdLifetime getRowIdLifetime() throws SQLException {
		// サポートしていない
		return RowIdLifetime.ROWID_UNSUPPORTED;
	}

	/**
	 * スキーマはサポートしていない
	 *
	 * @return	常に空の <code>java.sql.ResultSet</code> オブジェクト
	 */
	@Override
	public ResultSet getSchemas(String catalog, String schemaPattern)
			throws SQLException {
		// [NOT SUPPORTED!] スキーマは未サポート
		return new MetaDataResultSet();
	}

	/**
	 * このデータベースが、ストアドプロシージャーエスケープ構文を使用した、
	 * ユーザ定義関数またはベンダー関数の呼び出しをサポートするかどうかを
	 * 取得します。
	 *
	 * @return	常に<code>false</code>
	 */
	@Override
	public boolean supportsStoredFunctionsUsingCallSyntax()
			throws SQLException {
		// ストアドプリシージャーはサポートしていない
		return false;
	}

	/**
	 * <code>autoCommit</code>が<code>true</code>の場合に、
	 * 自動コミットが失敗した場合、すべての<code>ResultSet</code>がクローズ
	 * されるかどうかを返します。
	 */
	@Override
	public boolean autoCommitFailureClosesAllResultSets() throws SQLException {
		// 自動コミットに失敗してもすべてのResultSetがcloseされるわけではない
		return false;
	}

	/**
	 * クライアント情報プロパティーはサポートしていない
	 *
	 * @return 	常に空の <code>java.sql.ResultSet</code> オブジェクト
	 */
	@Override
	public ResultSet getClientInfoProperties() throws SQLException {
		// [NOT SUPPORTED!] クライアント情報プロパティは未サポート
		return new MetaDataResultSet();
	}

	/**
	 * <B>[サポート外！]</B>
	 */
	@Override
	public ResultSet getFunctions(String catalog, String schemaPattern,
			String functionNamePattern) throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * <B>[サポート外！]</B>
	 */
	@Override
	public ResultSet getFunctionColumns(String catalog, String schemaPattern,
			String functionNamePattern, String columnNamePattern)
			throws SQLException {
		// サポート外
		throw new NotSupported();
	}

	/**
	 * 自動採番列を常に返すかどうか。
	 */
	@Override
	public boolean generatedKeyAlwaysReturned() throws SQLException {
		// 自動採番列を常に返すわけではないので false
		return false;
	}
	@Override
	public ResultSet getPseudoColumns(String catalog, String schemaPattern,
	String tableNamePattern, String columnNamePattern) throws SQLException {
		// サポート外
		throw new NotSupported();
	}
}

//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2016, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
