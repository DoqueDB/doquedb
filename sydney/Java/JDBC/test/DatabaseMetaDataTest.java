// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseMetaDataTest.java -- jp.co.ricoh.doquedb.jdbc.DatabaseMetaData クラスのテスト
//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Types;

import jp.co.ricoh.doquedb.exception.InvalidPath;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.ReorganizeFailed;

class ColumnInfo
{
	static final int	CHECK_BYTES = 1;
	static final int	NOT_CHECK_BYTES = 0;
	String	tableName;
	String	columnName;
	int		index;
	int		dataType;
	String	typeName;
	int		columnSize;
	int		checkBytes;
	int		columnBytes;
	int		decimalDigits;
	int		numPrecRadix;
	int		nullable;
	String	isNullable;
	String	columnDefault;
	String	remarks;

	ColumnInfo(	String	tableName_,
				String	columnName_,
				int		index_,
				int		dataType_,
				String	typeName_,
				int		columnSize_,
				int		checkBytes_,
				int		columnBytes_,
				int		decimalDigits_,
				int		nullable_,
				String	columnDefault_,
				String	remarks_)
	{
		this(tableName_, columnName_, index_, dataType_, typeName_, columnSize_, checkBytes_, columnBytes_, decimalDigits_, nullable_, columnDefault_);
		remarks = remarks_;
	}

	ColumnInfo(	String	tableName_,
				String	columnName_,
				int		index_,
				int		dataType_,
				String	typeName_,
				int		columnSize_,
				int		checkBytes_,
				int		columnBytes_,
				int		decimalDigits_,
				int		nullable_,
				String	columnDefault_)
	{
		this(tableName_, columnName_, index_, dataType_, typeName_, columnSize_, checkBytes_, columnBytes_, decimalDigits_, nullable_);
		columnDefault = columnDefault_;
	}

	ColumnInfo(	String	tableName_,
				String	columnName_,
				int		index_,
				int		dataType_,
				String	typeName_,
				int		columnSize_,
				int		checkBytes_,
				int		columnBytes_,
				int		decimalDigits_,
				int		nullable_)
	{
		tableName = tableName_;
		columnName = columnName_;
		index = index_;
		dataType = dataType_;
		typeName = typeName_;
		columnSize = columnSize_;
		checkBytes = checkBytes_;
		columnBytes = columnBytes_;
		decimalDigits = decimalDigits_;
		numPrecRadix = 10;
		nullable = nullable_;
		switch (nullable) {
		case java.sql.DatabaseMetaData.columnNoNulls:
			isNullable = "NO";
			break;
		case java.sql.DatabaseMetaData.columnNullable:
			isNullable = "YES";
			break;
		default:
			isNullable = "";
			break;
		}
		columnDefault = null;
		remarks = null;
	}
}

public class DatabaseMetaDataTest extends TestBase
{
	// Sydney 製品名
	private final static String	SYDNEY_PRODUCT_NAME = "DoqueDB";
	// Sydney JDBC ドライバ名
	private final static String	SYDNEY_DRIVER_NAME = "DoqueDB JDBC Driver";

	// 現在の JDBC ドライバのメジャーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.Driver.MAJOR_VERSION が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_MAJOR_VERSION = 2;

	// 現在の JDBC ドライバのマイナーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.Driver.MINOR_VERSION が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_MINOR_VERSION = 0;

	// 現在の JDBC メジャーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.DatabaseMetaData.getJDBCMajorVersion() の戻り値が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_JDBC_MAJOR_VERSION = 4;

	// 現在の JDBC マイナーバージョン
	// ※ jp.co.ricoh.doquedb.jdbc.DatabaseMetaData.getJDBCMinorVersion() の戻り値が更新された場合、
	// 　 この値も等値に更新する必要がある。
	private final static int	CURRENT_JDBC_MINOR_VERSION = 0;

	// 不明な列数などで使う数値
	private final static int	UNKNOWN_VALUE = 0;

	public DatabaseMetaDataTest(String	nickname)
	{
		super(nickname);
	}

	// DatabaseMetaData.allProceduresAreCallable() のテスト
	public void test_allProceduresAreCallable() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().allProceduresAreCallable());

		c.close();
	}

	// DatabaseMetaData.allTablesAreSelectable() のテスト
	public void test_allTablesAreSelectable() throws Exception
	{
		Connection	c = getConnection();

		// すべてのテーブルがユーザによって使用できるはず
		assertTrue(c.getMetaData().allTablesAreSelectable());

		c.close();
	}

	// DatabaseMetaData.getURL() のテスト
	public void test_getURL() throws Exception
	{
		Connection	c = getConnection();

		// 現状、Connection の URL を返すだけとなっているはず
		assertEquals(super.URL, c.getMetaData().getURL());

		c.close();
	}

	// DatabaseMetaData.getUserName() のテスト
	public void test_getUserName() throws Exception
	{
		Connection	c = getConnection();

		// 現状、Connection のユーザ名を返すだけとなっているはず
		assertEquals(c.getMetaData().getUserName(), "root");

		c.close();
	}

	// DatabaseMetaData.isReadOnly() のテスト
	public void test_isReadOnly() throws Exception
	{
		Connection	c = getConnection();

		// データベース TEST は、書き込み可であるはず
		assertFalse(c.getMetaData().isReadOnly());

		c.close();

		String	path = "C:\\Sydney\\ro";

		// データベースパスに指定されたディレクトリーが存在していたらデータベースが作れない
		String	readOnlyDatabaseName = "RO_TEST";
		String	system_url = "jdbc:ricoh:doquedb://localhost:54321/$$SystemDB";
		c = DriverManager.getConnection(system_url, "root", "doqadmin");
		Statement	s = c.createStatement();
		//s.executeUpdate("create database " + readOnlyDatabaseName + " read only path '" + path + "'");
		s.executeUpdate("create database " + readOnlyDatabaseName + " path '" + path + "'");
		s.executeUpdate("alter database " + readOnlyDatabaseName + " read only");
		s.close();
		c.close();

		String	url = "jdbc:ricoh:doquedb://localhost:54321/" + readOnlyDatabaseName;
		c = DriverManager.getConnection(url, "root", "doqadmin");

		// データベース RO_TEST は、書き込み不可であるはず
		assertTrue(c.getMetaData().isReadOnly());

		c.close();

		c = DriverManager.getConnection(system_url, "root", "doqadmin");
		s = c.createStatement();

		s.executeUpdate("alter database " + readOnlyDatabaseName + " read write");
		s.executeUpdate("drop database " + readOnlyDatabaseName);
		s.close();
		c.close();
	}


	/* TODO: 要修正
	// データベースパスに存在するディレクトリパスを指定しての create database のテスト
	// DatabaseMetaData のテストではないが、上の test_isReadOnly() の【 仕様変更 】のコメントの通りなので、ついでにここにテスト追加
	public void test_0332() throws Exception
	{
		String	path = "/var/lib/DoqueDB/db/0332";
		java.io.File	f = new java.io.File(path);
		f.delete();
		f.mkdir();

		String	system_url = "jdbc:ricoh:doquedb://localhost:54321/$$SystemDB";
		Connection	c = DriverManager.getConnection(system_url, "root", "doqadmin");
		Statement	s = c.createStatement();

		String	ExpectedSQLState = (new InvalidPath("dummy message")).getSQLState();
		String	SQLState = "";
		boolean	noErr = true;
		try {
			s.executeUpdate("create database DB0332 path '" + path + "'");
		} catch (SQLException	sqle) {
			noErr = false;
			SQLState = sqle.getSQLState();
		}
		f.delete();
		assertEquals(ExpectedSQLState, SQLState);

		s.close();
		c.close();
	}
	 */

	// DatabaseMetaData.nullsAreSortedHigh() のテスト
	public void test_nullsAreSortedHigh() throws Exception
	{
		Connection	c = getConnection();

		// NULL は高位にソートされないはず
		assertFalse(c.getMetaData().nullsAreSortedHigh());

		c.close();
	}

	// DatabaseMetaData.nullsAreSortedLow() のテスト
	public void test_nullsAreSortedLow() throws Exception
	{
		Connection	c = getConnection();

		// NULL は下位にソートされるはず
		assertTrue(c.getMetaData().nullsAreSortedLow());

		c.close();
	}

	// DatabaseMetaData.nullsAreSortedAtStart() のテスト
	public void test_nullsAreSortedAtStart() throws Exception
	{
		Connection	c = getConnection();

		// NULL はソート順により高位または下位にソートされるので、必ず最初にソートされるわけではないはず
		assertFalse(c.getMetaData().nullsAreSortedAtStart());

		c.close();
	}

	// DatabaseMetaData.nullsAreSortedAtEnd() のテスト
	public void test_nullsAreSortedAtEnd() throws Exception
	{
		Connection	c = getConnection();

		// NULL はソート順により高位または下位にソートされるので、必ず最初にソートされるわけではないはず
		assertFalse(c.getMetaData().nullsAreSortedAtEnd());

		c.close();
	}

	// DatabaseMetaData.getDatabaseProductName() のテスト
	public void test_getDatabaseProductName() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(SYDNEY_PRODUCT_NAME, c.getMetaData().getDatabaseProductName());

		c.close();
	}

	// DatabaseMetaData.getDatabaseProductVersion() のテスト
	public void test_getDatabaseProductVersion() throws Exception
	{
		Connection	c = getConnection();

		String productVersion = c.getMetaData().getDatabaseProductVersion();
		String	expected = "";
		assertNotNull(productVersion);
		c.close();
	}

	// DatabaseMetaData.getDriverName() のテスト
	public void test_getDriverName() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(SYDNEY_DRIVER_NAME, c.getMetaData().getDriverName());

		c.close();
	}

	// DatabaseMetaData.getDriverVersion() のテスト
	public void test_getDriverVersion() throws Exception
	{
		Connection	c = getConnection();

		String	currentVersion = CURRENT_MAJOR_VERSION + "." + CURRENT_MINOR_VERSION;
		assertEquals(currentVersion, c.getMetaData().getDriverVersion());

		c.close();
	}

	// DatabaseMetaData.getDriverMajorVersion() のテスト
	public void test_getDriverMajorVersion() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(CURRENT_MAJOR_VERSION, c.getMetaData().getDriverMajorVersion());

		c.close();
	}

	// DatabaseMetaData.getDriverMinorVersion() のテスト
	public void test_getDriverMinorVersion() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(CURRENT_MINOR_VERSION, c.getMetaData().getDriverMinorVersion());

		c.close();
	}

	// DatabaseMetaData.usesLocalFiles() のテスト
	public void test_usesLocalFiles() throws Exception
	{
		Connection	c = getConnection();

		// ひとつのローカルファイルに複数のテーブルは格納しないはず
		assertFalse(c.getMetaData().usesLocalFiles());

		c.close();
	}

	// DatabaseMetaData.usesLocalFilePerTable() のテスト
	public void test_usesLocalFilePerTable() throws Exception
	{
		Connection	c = getConnection();

		// テーブルごとにひとつのファイルを使用するわけではないはず
		assertFalse(c.getMetaData().usesLocalFilePerTable());

		c.close();
	}

	// DatabaseMetaData.supportsMixedCaseIdentifiers() のテスト
	public void test_supportsMixedCaseIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別して処理しないはず
		assertFalse(c.getMetaData().supportsMixedCaseIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesUpperCaseIdentifiers() のテスト
	public void test_storesUpperCaseIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理するものの、大文字で格納するわけではないはず
		assertFalse(c.getMetaData().storesUpperCaseIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesLowerCaseIdentifiers() のテスト
	public void test_storesLowerCaseIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理するものの、小文字で格納するわけではないはず
		assertFalse(c.getMetaData().storesLowerCaseIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesMixedCaseIdentifiers() のテスト
	public void test_storesMixedCaseIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理し、大文字小文字混在で格納するはず
		assertTrue(c.getMetaData().storesMixedCaseIdentifiers());

		c.close();
	}

	// DatabaseMetaData.supportsMixedCaseQuotedIdentifiers() のテスト
	public void test_supportsMixedCaseQuotedIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないはず
		assertFalse(c.getMetaData().supportsMixedCaseQuotedIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesUpperCaseQuotedIdentifiers() のテスト
	public void test_storesUpperCaseQuotedIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理するものの、大文字で格納するするわけではないはず
		assertFalse(c.getMetaData().storesUpperCaseQuotedIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesLowerCaseQuotedIdentifiers() のテスト
	public void test_storesLowerCaseQuotedIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理するものの、小文字で格納するするわけではないはず
		assertFalse(c.getMetaData().storesLowerCaseQuotedIdentifiers());

		c.close();
	}

	// DatabaseMetaData.storesMixedCaseQuotedIdentifiers() のテスト
	public void test_storesMixedCaseQuotedIdentifiers() throws Exception
	{
		Connection	c = getConnection();

		// 大文字小文字を区別しないで処理し、大文字小文字混在で格納するはず
		assertTrue(c.getMetaData().storesMixedCaseQuotedIdentifiers());

		c.close();
	}

	// DatabaseMetaData.getIdentifierQuoteString() のテスト
	public void test_getIdentifierQuoteString() throws Exception
	{
		Connection	c = getConnection();

		// SQL 識別子を引用するのに使用する文字は、二重引用符のはず
		assertEquals("\"", c.getMetaData().getIdentifierQuoteString());

		c.close();
	}

	// DatabaseMetaData.getSQLKeywords() のテスト
	public void test_getSQLKeywords() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getSQLKeywords() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getSQLKeywords();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getNumericFunctions() のテスト
	public void test_getNumericFunctions() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getNumericFunctions() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getNumericFunctions();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getStringFunctions() のテスト
	public void test_getStringFunctions() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getStringFunctions() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getStringFunctions();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getSystemFunctions() のテスト
	public void test_getSystemFunctions() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getSystemFunctions() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getSystemFunctions();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getTimeDateFunctions() のテスト
	public void test_getTimeDateFunctions() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getTimeDateFunctions() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getTimeDateFunctions();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getSearchStringEscape() のテスト
	public void test_getSearchStringEscape() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getSearchStringEscape() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getSearchStringEscape();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getExtraNameCharacters() のテスト
	public void test_getExtraNameCharacters() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getExtraNameCharacters() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getExtraNameCharacters();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsAlterTableWithAddColumn() のテスト
	public void test_supportsAlterTableWithAddColumn() throws Exception
	{
		Connection	c = getConnection();

		// v15.0 以降では追加列のある ALTER TABLE をサポートしているはず
		assertTrue(c.getMetaData().supportsAlterTableWithAddColumn());

		c.close();
	}

	// DatabaseMetaData.supportsAlterTableWithDropColumn() のテスト
	public void test_supportsAlterTableWithDropColumn() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではドロップ列のある ALTER TABLE をサポートしていないはず
		assertFalse(c.getMetaData().supportsAlterTableWithDropColumn());

		c.close();
	}

	// DatabaseMetaData.supportsColumnAliasing() のテスト
	public void test_supportsColumnAliasing() throws Exception
	{
		Connection	c = getConnection();

		// 列の別名はサポートしているはず
		assertTrue(c.getMetaData().supportsColumnAliasing());

		c.close();
	}

	// DatabaseMetaData.nullPlusNonNullIsNull() のテスト
	public void test_nullPlusNonNullIsNull() throws Exception
	{
		Connection	c = getConnection();

		// NULL と非 NULL の連結は NULL になるはず
		assertTrue(c.getMetaData().nullPlusNonNullIsNull());

		c.close();
	}

	// DatabaseMetaData.supportsConvert() のテスト
	public void test_supportsConvert1() throws Exception
	{
		Connection	c = getConnection();

		// 現状では CONVERT 関数をサポートしていないはず
		assertFalse(c.getMetaData().supportsConvert());

		c.close();
	}

	// DatabaseMetaData.supportsConvert(int, int) のテスト
	public void test_supportsConvert2() throws Exception
	{
		Connection	c = getConnection();

		// 現状では CONVERT 関数をサポートしていないはず
		// なので、引数に何を送ろうとも常に false のはず
		assertFalse(c.getMetaData().supportsConvert(Types.SMALLINT, Types.INTEGER));
		assertFalse(c.getMetaData().supportsConvert(Types.FLOAT, Types.REAL));
		assertFalse(c.getMetaData().supportsConvert(Types.TIME, Types.TIMESTAMP));
		assertFalse(c.getMetaData().supportsConvert(Types.BIGINT, Types.SMALLINT));
		assertFalse(c.getMetaData().supportsConvert(Types.DECIMAL, Types.BIGINT));
		assertFalse(c.getMetaData().supportsConvert(Types.CHAR, Types.LONGVARCHAR));
		assertFalse(c.getMetaData().supportsConvert(Types.NUMERIC, Types.BIGINT));
		assertFalse(c.getMetaData().supportsConvert(Types.NULL, Types.NULL));
		assertFalse(c.getMetaData().supportsConvert(Types.DATE, Types.TIME));
		assertFalse(c.getMetaData().supportsConvert(Types.BLOB, Types.CLOB));
		assertFalse(c.getMetaData().supportsConvert(Types.BINARY, Types.BLOB));
		assertFalse(c.getMetaData().supportsConvert(Types.VARCHAR, Types.CHAR));

		c.close();
	}

	// DatabaseMetaData.supportsTableCorrelationNames() のテスト
	public void test_supportsTableCorrelationNames() throws Exception
	{
		Connection	c = getConnection();

		// テーブル相互関係名はサポートしているはず
		assertTrue(c.getMetaData().supportsTableCorrelationNames());

		c.close();
	}

	// DatabaseMetaData.supportsDifferentTableCorrelationNames() のテスト
	public void test_supportsDifferentTableCorrelationNames() throws Exception
	{
		Connection	c = getConnection();

		// テーブル相互関係名はサポートしているが、テーブルの名前と異なる名前であるという制限は付いていないはず
		assertFalse(c.getMetaData().supportsDifferentTableCorrelationNames());

		c.close();
	}

	// DatabaseMetaData.supportsExpressionsInOrderBy() のテスト
	public void test_supportsExpressionsInOrderBy() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ORDER BY リスト中での式をサポートしていないはず
		assertFalse(c.getMetaData().supportsExpressionsInOrderBy());

		c.close();
	}

	// DatabaseMetaData.supportsOrderByUnrelated() のテスト
	public void test_supportsOrderByUnrelated() throws Exception
	{
		Connection	c = getConnection();

		// ORDER BY で SELECT 中にない列の使用ができるはず
		assertTrue(c.getMetaData().supportsOrderByUnrelated());

		c.close();
	}

	// DatabaseMetaData.supportsGroupBy() のテスト
	public void test_supportsGroupBy() throws Exception
	{
		Connection	c = getConnection();

		// GROUP BY はサポートしているはず
		assertTrue(c.getMetaData().supportsGroupBy());

		c.close();
	}

	// DatabaseMetaData.supportsGroupByUnrelated() のテスト
	public void test_supportsGroupByUnrelated() throws Exception
	{
		Connection	c = getConnection();

		// GROUP BY で SELECT 中にない列の使用ができるはず
		assertTrue(c.getMetaData().supportsGroupByUnrelated());

		c.close();
	}

	// DatabaseMetaData.supportsGroupByBeyondSelect() のテスト
	public void test_supportsGroupByBeyondSelect() throws Exception
	{
		Connection	c = getConnection();

		// GROUP BY で SELECT 中にない列の使用ができるはず
		assertTrue(c.getMetaData().supportsGroupByBeyondSelect());

		c.close();
	}

	// DatabaseMetaData.supportsLikeEscapeClause() のテスト
	public void test_supportsLikeEscapeClause() throws Exception
	{
		Connection	c = getConnection();

		// LIKE エスケープ節の指定はサポートしているはず
		assertTrue(c.getMetaData().supportsLikeEscapeClause());

		c.close();
	}

	// DatabaseMetaData.supportsMultipleResultSets() のテスト
	public void test_supportsMultipleResultSets() throws Exception
	{
		Connection	c = getConnection();

		// 現状では Statement.execute() をサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsMultipleResultSets());

		c.close();
	}

	// DatabaseMetaData.supportsMultipleTransactions() のテスト
	public void test_supportsMultipleTransactions() throws Exception
	{
		Connection	c = getConnection();

		// 異なる接続で複数のトランザクションを開始できるはず
		assertTrue(c.getMetaData().supportsMultipleTransactions());

		c.close();
	}

	// DatabaseMetaData.supportsNonNullableColumns() のテスト
	public void test_supportsNonNullableColumns() throws Exception
	{
		Connection	c = getConnection();

		// 列を非 NULL として定義できるはず
		assertTrue(c.getMetaData().supportsNonNullableColumns());

		c.close();
	}

	// DatabaseMetaData.supportsMinimumSQLGrammar() のテスト
	public void test_supportsMinimumSQLGrammar() throws Exception
	{
		Connection	c = getConnection();

		// ODBC Minimum SQL 文法はサポートしているはず
		assertTrue(c.getMetaData().supportsMinimumSQLGrammar());

		c.close();
	}

	// DatabaseMetaData.supportsCoreSQLGrammar() のテスト
	public void test_supportsCoreSQLGrammar() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ODBC Core SQL 文法をサポートしていないはず
		assertFalse(c.getMetaData().supportsCoreSQLGrammar());

		c.close();
	}

	// DatabaseMetaData.supportsExtendedSQLGrammar() のテスト
	public void test_supportsExtendedSQLGrammar() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ODBC Extended SQL 文法をサポートしていないはず
		assertFalse(c.getMetaData().supportsExtendedSQLGrammar());

		c.close();
	}

	// DatabaseMetaData.supportsANSI92EntryLevelSQL() のテスト
	public void test_supportsANSI92EntryLevelSQL() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ANSI92 エントリレベルの SQL 文法をサポートしていないはず
		assertFalse(c.getMetaData().supportsANSI92EntryLevelSQL());

		c.close();
	}

	// DatabaseMetaData.supportsANSI92IntermediateSQL() のテスト
	public void test_supportsANSI92IntermediateSQL() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ANSI92 中間レベルの SQL 文法をサポートしていないはず
		assertFalse(c.getMetaData().supportsANSI92IntermediateSQL());

		c.close();
	}

	// DatabaseMetaData.supportsANSI92FullSQL() のテスト
	public void test_supportsANSI92FullSQL() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ANSI92 完全レベルの SQL 文法をサポートしていないはず
		assertFalse(c.getMetaData().supportsANSI92FullSQL());

		c.close();
	}

	// DatabaseMetaData.supportsIntegrityEnhancementFacility() のテスト
	public void test_supportsIntegrityEnhancementFacility() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsIntegrityEnhancementFacility() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().supportsIntegrityEnhancementFacility();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsOuterJoins() のテスト
	public void test_supportsOuterJoins() throws Exception
	{
		Connection	c = getConnection();

		// 外部結合のなんらかの形式がサポートされているはず
		assertTrue(c.getMetaData().supportsOuterJoins());

		c.close();
	}

	// DatabaseMetaData.supportsFullOuterJoins() のテスト
	public void test_supportsFullOuterJoins() throws Exception
	{
		Connection	c = getConnection();

		// 完全入れ子の外部結合はサポートしていないはず
		assertFalse(c.getMetaData().supportsFullOuterJoins());

		c.close();
	}

	// DatabaseMetaData.supportsLimitedOuterJoins() のテスト
	public void test_supportsLimitedOuterJoins() throws Exception
	{
		Connection	c = getConnection();

		// 外部結合に関し、制限されたサポートが提供されるはず
		assertTrue(c.getMetaData().supportsLimitedOuterJoins());

		c.close();
	}

	// DatabaseMetaData.getSchemaTerm() のテスト
	public void test_getSchemaTerm() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getSchemaTerm() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getSchemaTerm();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getProcedureTerm() のテスト
	public void test_getProcedureTerm() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getProcedureTerm() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getProcedureTerm();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getCatalogTerm() のテスト
	public void test_getCatalogTerm() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getCatalogTerm() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getCatalogTerm();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.isCatalogAtStart() のテスト
	public void test_isCatalogAtStart() throws Exception
	{
		Connection	c = getConnection();

		// 現状では isCatalogAtStart() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().isCatalogAtStart();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getCatalogSeparator() のテスト
	public void test_getCatalogSeparator() throws Exception
	{
		Connection	c = getConnection();

		// カタログをサポートしていないので常に空文字列のはず
		assertEquals("", c.getMetaData().getCatalogSeparator());

		c.close();
	}

	// DatabaseMetaData.supportsSchemasInDataManipulation() のテスト
	public void test_supportsSchemasInDataManipulation() throws Exception
	{
		Connection	c = getConnection();

		// データ操作文でスキーマ名は使用できないはず
		assertFalse(c.getMetaData().supportsSchemasInDataManipulation());

		c.close();
	}

	// DatabaseMetaData.supportsSchemasInProcedureCalls() のテスト
	public void test_supportsSchemasInProcedureCalls() throws Exception
	{
		Connection	c = getConnection();

		// プロシージャ呼び出し文でスキーマ名を使用できないはず
		assertFalse(c.getMetaData().supportsSchemasInProcedureCalls());

		c.close();
	}

	// DatabaseMetaData.supportsSchemasInTableDefinitions() のテスト
	public void test_supportsSchemasInTableDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// テーブル定義文でスキーマ名を使用できないはず
		assertFalse(c.getMetaData().supportsSchemasInTableDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsSchemasInIndexDefinitions() のテスト
	public void test_supportsSchemasInIndexDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// インデックス定義文でスキーマ名を使用できないはず
		assertFalse(c.getMetaData().supportsSchemasInIndexDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsSchemasInPrivilegeDefinitions() のテスト
	public void test_supportsSchemasInPrivilegeDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// 特権定義文でスキーマ名を使用できないはず
		assertFalse(c.getMetaData().supportsSchemasInPrivilegeDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsCatalogsInDataManipulation() のテスト
	public void test_supportsCatalogsInDataManipulation() throws Exception
	{
		Connection	c = getConnection();

		// データ操作文でカタログ名を使用できないはず
		assertFalse(c.getMetaData().supportsCatalogsInDataManipulation());

		c.close();
	}

	// DatabaseMetaData.supportsCatalogsInProcedureCalls() のテスト
	public void test_supportsCatalogsInProcedureCalls() throws Exception
	{
		Connection	c = getConnection();

		// プロシージャ呼び出し文でカタログ名を使用できないはず
		assertFalse(c.getMetaData().supportsCatalogsInProcedureCalls());

		c.close();
	}

	// DatabaseMetaData.supportsCatalogsInTableDefinitions() のテスト
	public void test_supportsCatalogsInTableDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// テーブル定義文でカタログ名を使用できないはず
		assertFalse(c.getMetaData().supportsCatalogsInTableDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsCatalogsInIndexDefinitions() のテスト
	public void test_supportsCatalogsInIndexDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// インデックス定義文でカタログ名を使用できないはず
		assertFalse(c.getMetaData().supportsCatalogsInIndexDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsCatalogsInPrivilegeDefinitions() のテスト
	public void test_supportsCatalogsInPrivilegeDefinitions() throws Exception
	{
		Connection	c = getConnection();

		// 特権定義文でカタログ名を使用できないはず
		assertFalse(c.getMetaData().supportsCatalogsInPrivilegeDefinitions());

		c.close();
	}

	// DatabaseMetaData.supportsPositionedDelete() のテスト
	public void test_supportsPositionedDelete() throws Exception
	{
		Connection	c = getConnection();

		// 位置指定の DELETE はできないはず
		assertFalse(c.getMetaData().supportsPositionedDelete());

		c.close();
	}

	// DatabaseMetaData.supportsPositionedUpdate() のテスト
	public void test_supportsPositionedUpdate() throws Exception
	{
		Connection	c = getConnection();

		// 位置指定の UPDATE はできないはず
		assertFalse(c.getMetaData().supportsPositionedUpdate());

		c.close();
	}

	// DatabaseMetaData.supportsSelectForUpdate() のテスト
	public void test_supportsSelectForUpdate() throws Exception
	{
		Connection	c = getConnection();

		// 現状では SELECT FOR UPDATE 文をサポートしていないはず
		assertFalse(c.getMetaData().supportsSelectForUpdate());

		c.close();
	}

	// DatabaseMetaData.supportsStoredProcedures() のテスト
	public void test_supportsStoredProcedures() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ストアドプロシージャをサポートしていないはず
		assertFalse(c.getMetaData().supportsStoredProcedures());

		c.close();
	}

	// DatabaseMetaData.supportsSubqueriesInComparisons() のテスト
	public void test_supportsSubqueriesInComparisons() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsSubqueriesInComparisons() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().supportsSubqueriesInComparisons();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsSubqueriesInExists() のテスト
	public void test_supportsSubqueriesInExists() throws Exception
	{
		Connection	c = getConnection();

		// EXISTS 中にサブクエリーを書けるはず
		assertTrue(c.getMetaData().supportsSubqueriesInExists());

		c.close();
	}

	// DatabaseMetaData.supportsSubqueriesInIns() のテスト
	public void test_supportsSubqueriesInIns() throws Exception
	{
		Connection	c = getConnection();

		// 現状では IN をサポートしていないはず
		assertFalse(c.getMetaData().supportsSubqueriesInIns());

		c.close();
	}

	// DatabaseMetaData.supportsSubqueriesInQuantifieds() のテスト
	public void test_supportsSubqueriesInQuantifieds() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsSubqueriesInQuantifieds() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().supportsSubqueriesInQuantifieds();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsCorrelatedSubqueries() のテスト
	public void test_supportsCorrelatedSubqueries() throws Exception
	{
		Connection	c = getConnection();

		// 照合関係サブクエリーはサポートしているはず
		assertTrue(c.getMetaData().supportsCorrelatedSubqueries());

		c.close();
	}

	// DatabaseMetaData.supportsUnion() のテスト
	public void test_supportsUnion() throws Exception
	{
		Connection	c = getConnection();

		// 現状では UNION をサポートしていないはず
		assertFalse(c.getMetaData().supportsUnion());

		c.close();
	}

	// DatabaseMetaData.supportsUnionAll() のテスト
	public void test_supportsUnionAll() throws Exception
	{
		Connection	c = getConnection();

		// 現状では UNION ALL をサポートしていないはず
		assertFalse(c.getMetaData().supportsUnionAll());

		c.close();
	}

	// DatabaseMetaData.supportsOpenCursorsAcrossCommit() のテスト
	public void test_supportsOpenCursorsAcrossCommit() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではカーソルをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsOpenCursorsAcrossCommit());

		c.close();
	}

	// DatabaseMetaData.supportsOpenCursorsAcrossRollback() のテスト
	public void test_supportsOpenCursorsAcrossRollback() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではカーソルをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsOpenCursorsAcrossRollback());

		c.close();
	}

	// DatabaseMetaData.supportsOpenStatementsAcrossCommit() のテスト
	public void test_supportsOpenStatementsAcrossCommit() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsOpenStatementsAcrossCommit() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().supportsOpenStatementsAcrossCommit();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsOpenStatementsAcrossRollback() のテスト
	public void test_supportsOpenStatementsAcrossRollback() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsOpenStatementsAcrossRollback() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().supportsOpenStatementsAcrossRollback();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getMaxBinaryLiteralLength() のテスト
	public void test_getMaxBinaryLiteralLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではインラインバイナリリテラル中に入れられる 16 進数の最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxBinaryLiteralLength());

		c.close();
	}

	// DatabaseMetaData.getMaxCharLiteralLength() のテスト
	public void test_getMaxCharLiteralLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではキャラクタリテラルの最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxCharLiteralLength());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnNameLength() のテスト
	public void test_getMaxColumnNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状では列名の最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnsInGroupBy() のテスト
	public void test_getMaxColumnsInGroupBy() throws Exception
	{
		Connection	c = getConnection();

		// 現状では GROUP BY 中の最大列数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnsInGroupBy());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnsInIndex() のテスト
	public void test_getMaxColumnsInIndex() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではインデックス中の最大列数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnsInIndex());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnsInOrderBy() のテスト
	public void test_getMaxColumnsInOrderBy() throws Exception
	{
		Connection	c = getConnection();

		// 現状では ORDER BY 中の最大列数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnsInOrderBy());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnsInSelect() のテスト
	public void test_getMaxColumnsInSelect() throws Exception
	{
		Connection	c = getConnection();

		// 現状では SELECT 中の最大列数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnsInSelect());

		c.close();
	}

	// DatabaseMetaData.getMaxColumnsInTable() のテスト
	public void test_getMaxColumnsInTable() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではテーブル中の最大列数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxColumnsInTable());

		c.close();
	}

	// DatabaseMetaData.getMaxConnections() のテスト
	public void test_getMaxConnections() throws Exception
	{
		Connection	c = getConnection();

		// 現状では可能な最大接続数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxConnections());

		c.close();
	}

	// DatabaseMetaData.getMaxCursorNameLength() のテスト
	public void test_getMaxCursorNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではカーソルをサポートしていないのでカーソル名の最大文字数は常に 0 のはず
		assertZero(c.getMetaData().getMaxCursorNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxIndexLength() のテスト
	public void test_getMaxIndexLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではインデックスの最大バイト数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxIndexLength());

		c.close();
	}

	// DatabaseMetaData.getMaxSchemaNameLength() のテスト
	public void test_getMaxSchemaNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではスキーマをサポートしていないのでスキーマ名の最大文字数は常に 0 のはず
		assertZero(c.getMetaData().getMaxSchemaNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxProcedureNameLength() のテスト
	public void test_getMaxProcedureNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないのでプロシージャ名の最大文字数は常に 0 のはず
		assertZero(c.getMetaData().getMaxProcedureNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxCatalogNameLength() のテスト
	public void test_getMaxCatalogNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではカタログをサポートしていないのでカタログ名の最大文字数は常に 0 のはず
		assertZero(c.getMetaData().getMaxCatalogNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxRowSize() のテスト
	public void test_getMaxRowSize() throws Exception
	{
		Connection	c = getConnection();

		// 現状では 1 行の最大バイト数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxRowSize());

		c.close();
	}

	// DatabaseMetaData.doesMaxRowSizeIncludeBlobs() のテスト
	public void test_doesMaxRowSizeIncludeBlobs() throws Exception
	{
		Connection	c = getConnection();

		// 現状では 1 行の最大バイト数が“不明”なので常に false のはず
		assertFalse(c.getMetaData().doesMaxRowSizeIncludeBlobs());

		c.close();
	}

	// DatabaseMetaData.getMaxStatementLength() のテスト
	public void test_getMaxStatementLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状では SQL 文の最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxStatementLength());

		c.close();
	}

	// DatabaseMetaData.getMaxStatements() のテスト
	public void test_getMaxStatements() throws Exception
	{
		Connection	c = getConnection();

		// 現状では同時にオープン可能なアクティブな文の最大数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxStatements());

		c.close();
	}

	// DatabaseMetaData.getMaxTableNameLength() のテスト
	public void test_getMaxTableNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではテーブル名の最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxTableNameLength());

		c.close();
	}

	// DatabaseMetaData.getMaxTablesInSelect() のテスト
	public void test_getMaxTablesInSelect() throws Exception
	{
		Connection	c = getConnection();

		// 現状では SELECT 中の最大テーブル数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxTablesInSelect());

		c.close();
	}

	// DatabaseMetaData.getMaxUserNameLength() のテスト
	public void test_getMaxUserNameLength() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではユーザ名の最大文字数は“不明”のはず
		assertEquals(UNKNOWN_VALUE, c.getMetaData().getMaxUserNameLength());

		c.close();
	}

	// DatabaseMetaData.getDefaultTransactionIsolation() のテスト
	public void test_getDefaultTransactionIsolation() throws Exception
	{
		Connection	c = getConnection();

		// デフォルトのトランザクション遮断レベルは Connection.TRANSACTION_READ_COMMITTED のはず
		assertEquals(Connection.TRANSACTION_READ_COMMITTED, c.getMetaData().getDefaultTransactionIsolation());

		c.close();
	}

	// DatabaseMetaData.supportsTransactions() のテスト
	public void test_supportsTransactions() throws Exception
	{
		Connection	c = getConnection();

		// トランザクションはサポートしているはず
		assertTrue(c.getMetaData().supportsTransactions());

		c.close();
	}

	// DatabaseMetaData.supportsTransactionIsolationLevel() のテスト
	public void test_supportsTransactionIsolationLevel() throws Exception
	{
		Connection	c = getConnection();

		// 以下、サポートしているはずのトランザクション遮断レベル
		assertTrue(c.getMetaData().supportsTransactionIsolationLevel(Connection.TRANSACTION_READ_UNCOMMITTED));
		assertTrue(c.getMetaData().supportsTransactionIsolationLevel(Connection.TRANSACTION_READ_COMMITTED));
		assertTrue(c.getMetaData().supportsTransactionIsolationLevel(Connection.TRANSACTION_REPEATABLE_READ));
		assertTrue(c.getMetaData().supportsTransactionIsolationLevel(Connection.TRANSACTION_SERIALIZABLE));
		assertTrue(c.getMetaData().supportsTransactionIsolationLevel(jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT));
		// 以下、サポートしていないはずのトランザクション遮断レベル
		assertFalse(c.getMetaData().supportsTransactionIsolationLevel(Connection.TRANSACTION_NONE));
		assertFalse(c.getMetaData().supportsTransactionIsolationLevel(-1));

		c.close();
	}

	// DatabaseMetaData.supportsDataDefinitionAndDataManipulationTransactions() のテスト
	public void test_supportsDataDefinitionAndDataManipulationTransactions() throws Exception
	{
		Connection	c = getConnection();

		assertFalse(c.getMetaData().supportsDataDefinitionAndDataManipulationTransactions());

		c.close();
	}

	// DatabaseMetaData.supportsDataManipulationTransactionsOnly() のテスト
	public void test_supportsDataManipulationTransactionsOnly() throws Exception
	{
		Connection	c = getConnection();

		assertTrue(c.getMetaData().supportsDataManipulationTransactionsOnly());

		c.close();
	}

	// DatabaseMetaData.dataDefinitionCausesTransactionCommit() のテスト
	public void test_dataDefinitionCausesTransactionCommit() throws Exception
	{
		Connection	c = getConnection();

		assertFalse(c.getMetaData().dataDefinitionCausesTransactionCommit());

		c.close();
	}

	// DatabaseMetaData.dataDefinitionIgnoredInTransactions() のテスト
	public void test_dataDefinitionIgnoredInTransactions() throws Exception
	{
		Connection	c = getConnection();

		assertFalse(c.getMetaData().dataDefinitionIgnoredInTransactions());

		c.close();
	}

	// DatabaseMetaData.getProcedures() のテスト
	public void test_getProcedures() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getProcedures(null, null, null).next());
		//                                                         ~~~~~~~ 空の ResultSet なので、
		//                                                                 初回 next() の戻り値が
		//                                                                 false であるはず

		c.close();
	}

	// DatabaseMetaData.getProcedureColumns() のテスト
	public void test_getProcedureColumns() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getProcedureColumns(null, null, null, null).next());
		//                                                                     ~~~~~~~ 同上

		c.close();
	}

	// DatabaseMetaData.getTables() のテスト
	public void test_getTables() throws Exception
	{
		Connection	c = getConnection();

		// 下準備

		//	tbl1
		//		id

		//	tbl2
		//		id

		//	tbl3
		//		id

		//	tbl4
		//		id

		//	tbl5
		//		id

		// _1007ttt
		//		id

		// _1008ttt
		//		id

		// _1009ttt
		//		id

		// _1010ttt
		//		id

		// _1011ttt
		//		id

		// _1012ttt
		//		id

		// _1013ttt
		//		id

		// _1014ttt
		//		id

		// _1015ttt
		//		id

		// _1016ttt
		//		id

		// _1017ttt
		//		id

		// _1018ttt
		//		id

		// _1019ttt
		//		id

		Statement	s = c.createStatement();
		for (int i = 1; i < 6; i++) {
			String	query = "create table tbl" + i + " (id int)";
			s.executeUpdate(query);
		}
		for (int i = 1007; i < 1020; i++) {
			String	query = "create table _" + i + "ttt (id int)";
			s.executeUpdate(query);
		}
		s.close();

		ResultSet	rs = null;

		// 表名パターンに一致する表が存在する場合には一致するすべての表の情報が得られるはず − その１
		//	条件
		//		表名パターン	_100%tt%
		//		↓
		//	結果
		//		表名
		//		_1007ttt
		//		_1008ttt
		//		_1009ttt
		{
			assertNotNull(rs = c.getMetaData().getTables(null, null, "_100%tt%", null));
			String[]	tables = { "_1007ttt", "_1008ttt", "_1009ttt" };
			assertTableResults(rs, tables);
			rs.close();
		}

		// 表名パターンに一致する表が存在する場合には一致するすべての表の情報が得られるはず − その２
		//	条件
		//		表名パターン	tbl%
		//		↓
		//	結果
		//		表名
		//		tbl1
		//		tbl2
		//		tbl3
		//		tbl4
		//		tbl5
		{
			assertNotNull(rs = c.getMetaData().getTables(null, null, "tbl%", null));
			String[]	tables = { "tbl1", "tbl2", "tbl3", "tbl4", "tbl5" };
			assertTableResults(rs, tables);
			rs.close();
		}

		// 表名パターンに一致する表が存在する場合には一致するすべての表の情報が得られるはず − その３
		//	条件
		//		表名パターン	_1015ttt
		//		↓
		//	結果
		//		表名
		//		_1015ttt
		{
			assertNotNull(rs = c.getMetaData().getTables(null, null, "_1015ttt", null));
			String[]	tables = { "_1015ttt" };
			assertTableResults(rs, tables);
			rs.close();
		}

		// 表名パターンを指定しない場合、データベース内に存在するすべての表の情報が得られるはず（ null 指定／空文字列指定）
		// 表名パターンに一致する表が存在する場合には一致するすべての表の情報が得られるはず − その３
		//	条件
		//		表名パターン	未指定（ null 指定／空文字列指定）
		//		↓
		//	結果
		//		表名
		//		_1007ttt
		//		_1008ttt
		//		_1009ttt
		//		_1010ttt
		//		_1011ttt
		//		_1012ttt
		//		_1013ttt
		//		_1014ttt
		//		_1015ttt
		//		_1016ttt
		//		_1017ttt
		//		_1018ttt
		//		_1019ttt
		//		tbl1
		//		tbl2
		//		tbl3
		//		tbl4
		//		tbl5
		for (int i = 0; i < 2; i++) {

			String	tableNamePattern = null;
			if (i > 0) tableNamePattern = "";
			assertNotNull(rs = c.getMetaData().getTables(null, null, tableNamePattern, null));
			String[]	tables = {
				"_1007ttt", "_1008ttt", "_1009ttt", "_1010ttt", "_1011ttt", "_1012ttt", "_1013ttt",
				"_1014ttt", "_1015ttt", "_1016ttt", "_1017ttt", "_1018ttt", "_1019ttt",
				"tbl1", "tbl2", "tbl3", "tbl4", "tbl5"
			};
			assertTableResults(rs, tables);
			rs.close();
		}

		// 表名パターンに一致する表が存在しない場合には空の ResultSet が得られるはず − その１
		assertFalse(c.getMetaData().getTables(null, null, "x", null).next());

		// 表名パターンに一致する表が存在しない場合には空の ResultSet が得られるはず − その２
		assertFalse(c.getMetaData().getTables(null, null, "%x%", null).next());

		// 後始末
		s = c.createStatement();
		for (int i = 1; i < 6; i++) {
			String	query = "drop table tbl" + i;
			s.executeUpdate(query);
		}
		for (int i = 1007; i < 1020; i++) {
			String	query = "drop table _" + i + "ttt";
			s.executeUpdate(query);
		}
		s.close();

		c.close();
	}

	// DatabaseMetaData.getSchemas() のテスト
	public void test_getSchemas() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではスキーマをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getSchemas().next());
		//                                      ~~~~~~~ 空の ResultSet なので、
		//                                              初回 next() の戻り値が
		//                                              false であるはず

		c.close();
	}

	// DatabaseMetaData.getCatalogs() のテスト
	public void test_getCatalogs() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではカタログをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getCatalogs().next());
		//                                       ~~~~~~~ 空の ResultSet なので、
		//                                               初回 next() の戻り値が
		//                                               false であるはず

		c.close();
	}

	// DatabaseMetaData.getTableTypes() のテスト
	public void test_getTableTypes() throws Exception
	{
		Connection	c = getConnection();

		// 現状では常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getTableTypes().next());
		//                                         ~~~~~~~ 空の ResultSet なので、
		//                                                 初回 next() の戻り値が
		//                                                 false であるはず

		c.close();
	}

	// DatabaseMetaData.getColumns() のテスト
	public void test_getColumns() throws Exception
	{
		Connection	c = getConnection();
		Statement	s = c.createStatement();

		// 下準備

		//	tbl1001
		//		serialNumber	int
		//		name			char(100)
		//		areaNumber		int

		//	tbl1003
		//		fldInt						int
		//		fldBigInt					bigint
		//		fldDecimal					decimal(10,5)
		//		fldFloat					float
		//		fldDateTime					datetime
		//		fldUniqueIdentifier			uniqueidentifier
		//		fldImage					image
		//		fldLanguage					language
		//		fldNChar64					nchar(64)
		//		fldChar8					char(8)
		//		fldNVarChar256				nvarchar(256)
		//		fldVarChar128				varchar(128)
		//		fldNText					ntext
		//		fldFullText					fulltext
		//		fldBinary50					binary(50)
		//		fldNClob					nclob
		//		fldBlob						blob
		//		fldIntArray					int					array[no limit]
		//		fldBigIntArray				bigint				array[no limit]
		//		fldDecimalArray				decimal				array[no limit]
		//		fldFloatArray				float				array[no limit]
		//		fldDateTimeArray			datetime			array[no limit]
		//		fldUniqueIdentifierArray	uniqueidentifier	array[no limit]
		//		fldImageArray				image				array[no limit]
		//		fldLanguageArray			language			array[no limit]
		//		fldNChar64Array				nchar(64)			array[no limit]
		//		fldChar8Array				char(8)				array[no limit]
		//		fldNVarChar256Array			nvarchar(256)		array[no limit]
		//		fldVarChar128Array			varchar(128)		array[no limit]
		//		fldNTextArray				ntext				array[no limit]
		//		fldFullTextArray			fulltext			array[no limit]
		//		fldBinary50Array			binary(50)			array[no limit]

		//	tbl1013
		//		id			int
		//		name		nvarchar(500)
		//		address		ntext
		//		typeNumber	int
		//		fldNo		int

		//	t1
		//		f_int_pk		int					← primary key
		//		f_int			int
		//		f_int_df		int					default 32
		//		f_int_nn		int																	not null
		//		f_int_df_nn		int					default 98										not null
		//		f_big			bigint
		//		f_big_df		bigint				default 43298741683643
		//		f_big_nn		bigint																not null
		//		f_big_df_nn		bigint				default 7846827364876234						not null
		//		f_dec			decimal(10,5)
		//		f_dec_df		decimal(10,5)		default 99999.99999
		//		f_dec_nn		decimal(10,5)														not null
		//		f_dec_df_nn		decimal(10,5)		default 99999.99999								not null
		//		f_flt			float
		//		f_flt_df		float				default 7.9075
		//		f_flt_nn		float																not null
		//		f_flt_df_nn		float				default 0.006023								not null
		//		f_dat			datetime
		//		f_dat_df		datetime			default '2005-03-03 11:41:39.683'
		//		f_dat_nn		datetime															not null
		//		f_dat_df_nn		datetime			default '2005-02-28 09:12:34.393'				not null
		//		f_uni			uniqueidentifier
		//		f_uni_df		uniqueidentifier	default 'D599A207-25BC-4e20-BE23-9C6F777C193C'
		//		f_uni_nn		uniqueidentifier													not null
		//		f_uni_df_nn		uniqueidentifier	default '5E42CFEA-9BF5-480c-9DB6-72D4118F09B4'	not null
		//		f_img			image
		//		f_img_df		image				default 'abcdefg'
		//		f_img_nn		image																not null
		//		f_img_df_nn		image				default 'ABCD'									not null
		//		f_lng			language
		//		f_lng_df		language			default 'en'
		//		f_lng_nn		language															not null
		//		f_lng_df_nn		language			default 'ja'									not null
		//		f_nch			nchar(64)
		//		f_nch_df		nchar(64)			default 'hogehoge'
		//		f_nch_nn		nchar(64)															not null
		//		f_nch_df_nn		nchar(64)			default 'foo'									not null
		//		f_chr			char(8)
		//		f_chr_df		char(8)				default 'def'
		//		f_chr_nn		char(8)																not null
		//		f_chr_df_nn		char(8)				default 'hoge'									not null
		//		f_nvc			nvarchar(256)
		//		f_nvc_df		nvarchar(256)		default 'AABBCCDDEEFFGG'
		//		f_nvc_nn		nvarchar(256)														not null
		//		f_nvc_df_nn		nvarchar(256)		default 'abcdefg'								not null
		//		f_vch			varchar(128)
		//		f_vch_df		varchar(128)		default 'false'
		//		f_vch_nn		varchar(128)														not null
		//		f_vch_df_nn		varchar(128)		default 'xxxxxxx'								not null
		//		f_ntx			ntext
		//		f_ntx_df		ntext				default 'text data'
		//		f_ntx_nn		ntext																not null
		//		f_ntx_df_nn		ntext				default 'text data 2'							not null
		//		f_ntc			ntext																			hint heap 'compressed'
		//		f_ntc_df		ntext				default 'compressed text data'								hint heap 'compressed'
		//		f_ntc_nn		ntext																not null	hint heap 'compressed'
		//		f_ntc_df_nn		ntext				default 'compressed text value'					not null	hint heap 'compressed'
		//		f_ftx			fulltext
		//		f_ftx_df		fulltext			default 'fulltext default data'
		//		f_ftx_nn		fulltext															not null
		//		f_ftx_df_nn		fulltext			default 'fulltext default data'					not null
		//		f_bin			binary(50)
		//		f_bin_df		binary(50)			default '8798234'
		//		f_bin_nn		binary(50)															not null
		//		f_bin_df_nn		binary(50)			default 'xyz'									not null
		//		f_clb			nclob
		//		f_clb_df		nclob				default 'abc'
		//		f_clb_nn		nclob																not null
		//		f_clb_df_nn		nclob				default 'HOGE'									not null
		//		f_blb			blob
		//		f_blb_df		blob				default '321'
		//		f_blb_nn		blob																not null
		//		f_blb_df_nn		blob				default 'hijklmn'								not null
		//		f_int_a			int					array[10]
		//		f_int_a_nn		int					array[10]										not null
		//		f_int_nla		int					array[no limit]
		//		f_int_nla_nn	int					array[no limit]									not null
		//		f_big_a			bigint				array[6]
		//		f_big_a_nn		bigint				array[6]										not null
		//		f_big_nla		bigint				array[no limit]
		//		f_big_nla_nn	bigint				array[no limit]									not null
		//		f_dec_a			decimal(10,5)		array[6]
		//		f_dec_a_nn		decimal(10,5)		array[6]										not null
		//		f_dec_nla		decimal(10,5)		array[no limit]
		//		f_dec_nla_nn	decimal(10,5)		array[no limit]									not null
		//		f_flt_a			float				array[20]
		//		f_flt_a_nn		float				array[20]										not null
		//		f_flt_nla		float				array[no limit]
		//		f_flt_nla_nn	float				array[no limit]									not null
		//		f_dat_a			datetime			array[3]
		//		f_dat_a_nn		datetime			array[3]										not null
		//		f_dat_nla		datetime			array[no limit]
		//		f_dat_nla_nn	datetime			array[no limit]									not null
		//		f_uni_a			uniqueidentifier	array[5]
		//		f_uni_a_nn		uniqueidentifier	array[5]										not null
		//		f_uni_nla		uniqueidentifier	array[no limit]
		//		f_uni_nla_nn	uniqueidentifier	array[no limit]									not null
		//		f_img_a			image				array[10]
		//		f_img_a_nn		image				array[10]										not null
		//		f_img_nla		image				array[no limit]
		//		f_img_nla_nn	image				array[no limit]									not null
		//		f_lng_a			language			array[30]
		//		f_lng_a_nn		language			array[30]										not null
		//		f_lng_nla		language			array[no limit]
		//		f_lng_nla_nn	language			array[no limit]									not null
		//		f_nch_a			nchar(64)			array[5]
		//		f_nch_a_nn		nchar(64)			array[5]										not null
		//		f_nch_nla		nchar(64)			array[no limit]
		//		f_nch_nla_nn	nchar(64)			array[no limit]									not null
		//		f_chr_a			char(8)				array[20]
		//		f_chr_a_nn		char(8)				array[20]										not null
		//		f_chr_nla		char(8)				array[no limit]
		//		f_chr_nla_nn	char(8)				array[no limit]									not null
		//		f_nvc_a			nvarchar(256)		array[3]
		//		f_nvc_a_nn		nvarchar(256)		array[3]										not null
		//		f_nvc_nla		nvarchar(256)		array[no limit]
		//		f_nvc_nla_nn	nvarchar(256)		array[no limit]									not null
		//		f_vch_a			varchar(128)		array[5]
		//		f_vch_a_nn		varchar(128)		array[5]										not null
		//		f_vch_nla		varchar(128)		array[no limit]
		//		f_vch_nla_nn	varchar(128)		array[no limit]									not null
		//		f_ntx_a			ntext				array[10]
		//		f_ntx_a_nn		ntext				array[10]										not null
		//		f_ntx_nla		ntext				array[no limit]
		//		f_ntx_nla_nn	ntext				array[no limit]									not null
		//		f_ntc_a			ntext				array[8]													hint heap 'compressed'
		//		f_ntc_a_nn		ntext				array[8]										not null	hint heap 'compressed'
		//		f_ntc_nla		ntext				array[no limit]												hint heap 'compressed'
		//		f_ntc_nla_nn	ntext				array[no limit]									not null	hint heap 'compressed'
		//		f_ftx_a			fulltext			array[3]
		//		f_ftx_a_nn		fulltext			array[3]										not null
		//		f_ftx_nla		fulltext			array[no limit]
		//		f_ftx_nla_nn	fulltext			array[no limit]									not null
		//		f_bin_a			binary(50)			array[8]
		//		f_bin_a_nn		binary(50)			array[8]										not null
		//		f_bin_nla		binary(50)			array[no limit]
		//		f_bin_nla_nn	binary(50)			array[no limit]									not null

		String	query =
			"create table tbl1001 ("		+
				"serialNumber	int,"		+
				"name			char(100),"	+
				"areaNumber		int"		+
			")";
		assertEquals(0, s.executeUpdate(query));

		query =
			"create table tbl1003 ("												+
				"fldInt						int,";

		// bigint 列は v15.0 からサポート
		query = query +
				"fldBigInt					bigint,";

		// decimal 列は v16.1 からサポート
		query = query +
				"fldDecimal					decimal(10,5),";

		query = query +
				"fldFloat					float,"									+
				"fldDateTime				datetime,"								+
				"fldUniqueIdentifier		uniqueidentifier,"						+
				"fldImage					image,"									+
				"fldLanguage				language,"								+
				"fldNChar64					nchar(64),"								+
				"fldChar8					char(8),"								+
				"fldNVarChar256				nvarchar(256),"							+
				"fldVarChar128				varchar(128),"							+
				"fldNText					ntext,"									+
				"fldFullText				fulltext,"								+
				"fldBinary50				binary(50),"							+
				"fldNClob					nclob,"									+
				"fldBlob					blob,"									+
				"fldIntArray				int					array[no limit],";

		// bigint 列は v15.0 からサポート
		query = query +
				"fldBigIntArray				bigint				array[no limit],";

		// decimal 列は v16.1 からサポート
		query = query +
				"fldDecimalArray			decimal(10,5)		array[no limit],";

		query = query +
				"fldFloatArray				float				array[no limit],"	+
				"fldDateTimeArray			datetime			array[no limit],"	+
				"fldUniqueIdentifierArray	uniqueidentifier	array[no limit],"	+
				"fldImageArray				image				array[no limit],"	+
				"fldLanguageArray			language			array[no limit],"	+
				"fldNChar64Array			nchar(64)			array[no limit],"	+
				"fldChar8Array				char(8)				array[no limit],"	+
				"fldNVarChar256Array		nvarchar(256)		array[no limit],"	+
				"fldVarChar128Array			varchar(128)		array[no limit],"	+
				"fldNTextArray				ntext				array[no limit],"	+
				"fldFullTextArray			fulltext			array[no limit],"	+
				"fldBinary50Array			binary(50)			array[no limit]"	+
			")";
		assertEquals(0, s.executeUpdate(query));

		query =
			"create table tbl1013 ("		+
				"id			int,"			+
				"name		nvarchar(500),"	+
				"address	ntext,"			+
				"typeNumber	int,"			+
				"fldNo		int"			+
			")";
		assertEquals(0, s.executeUpdate(query));

		query =
			"create table t1 ("																												+
				"f_int_pk		int,"																										+
				"f_int			int,"																										+
				"f_int_df		int					default 32,"																			+
				"f_int_nn		int																	not null,"								+
				"f_int_df_nn	int					default 98										not null,";

		// bigint 列は v15.0 からサポート
		query = query +
				"f_big			bigint,"																									+
				"f_big_df		bigint				default 43298741683643,"																+
				"f_big_nn		bigint																not null,"								+
				"f_big_df_nn	bigint				default 7846827364876234						not null,";

		// decimal 列は v16.1 からサポート
		query = query +
				"f_dec			decimal(10,5),"																									+
				"f_dec_df		decimal(10,5)		default 99999.99999,"																+
				"f_dec_nn		decimal(10,5)														not null,"								+
				"f_dec_df_nn	decimal(10,5)		default 99999.99999								not null,";

		query = query +
				"f_flt			float,"																										+
				"f_flt_df		float				default 7.9075,"																		+
				"f_flt_nn		float																not null,"								+
				"f_flt_df_nn	float				default 0.006023								not null,"								+
				"f_dat			datetime,"																									+
				"f_dat_df		datetime			default '2005-03-03 11:41:39.683',"														+
				"f_dat_nn		datetime															not null,"								+
				"f_dat_df_nn	datetime			default '2005-02-28 09:12:34.393'				not null,"								+
				"f_uni			uniqueidentifier,"																							+
				"f_uni_df		uniqueidentifier	default 'D599A207-25BC-4e20-BE23-9C6F777C193C',"										+
				"f_uni_nn		uniqueidentifier													not null,"								+
				"f_uni_df_nn	uniqueidentifier	default '5E42CFEA-9BF5-480c-9DB6-72D4118F09B4'	not null,"								+
				"f_img			image,"																										+
				"f_img_df		image				default 'abcdefg',"																		+
				"f_img_nn		image																not null,"								+
				"f_img_df_nn	image				default 'ABCD'									not null,"								+
				"f_lng			language,"																									+
				"f_lng_df		language			default 'en',"																			+
				"f_lng_nn		language															not null,"								+
				"f_lng_df_nn	language			default 'ja'									not null,"								+
				"f_nch			nchar(64),"																									+
				"f_nch_df		nchar(64)			default 'hogehoge',"																	+
				"f_nch_nn		nchar(64)															not null,"								+
				"f_nch_df_nn	nchar(64)			default 'foo'									not null,"								+
				"f_chr			char(8),"																									+
				"f_chr_df		char(8)				default 'def',"																			+
				"f_chr_nn		char(8)																not null,"								+
				"f_chr_df_nn	char(8)				default 'hoge'									not null,"								+
				"f_nvc			nvarchar(256),"																								+
				"f_nvc_df		nvarchar(256)		default 'AABBCCDDEEFFGG',"																+
				"f_nvc_nn		nvarchar(256)														not null,"								+
				"f_nvc_df_nn	nvarchar(256)		default 'abcdefg'								not null,"								+
				"f_vch			varchar(128),"																								+
				"f_vch_df		varchar(128)		default 'false',"																		+
				"f_vch_nn		varchar(128)														not null,"								+
				"f_vch_df_nn	varchar(128)		default 'xxxxxxx'								not null,"								+
				"f_ntx			ntext,"																										+
				"f_ntx_df		ntext				default 'text data',"																	+
				"f_ntx_nn		ntext																not null,"								+
				"f_ntx_df_nn	ntext				default 'text data 2'							not null,"								+
				"f_ntc			ntext																			hint heap 'compressed',"	+
				"f_ntc_df		ntext				default 'compressed text data'								hint heap 'compressed' ,"	+
				"f_ntc_nn		ntext																not null	hint heap 'compressed',"	+
				"f_ntc_df_nn	ntext				default 'compressed text value'					not null	hint heap 'compressed',"	+
				"f_ftx			fulltext,"																									+
				"f_ftx_df		fulltext			default 'fulltext default data',"														+
				"f_ftx_nn		fulltext															not null,"								+
				"f_ftx_df_nn	fulltext			default 'fulltext default data'					not null,"								+
				"f_bin			binary(50),"																								+
				"f_bin_df		binary(50)			default '8798234',"																		+
				"f_bin_nn		binary(50)															not null,"								+
				"f_bin_df_nn	binary(50)			default 'xyz'									not null,"								+
				"f_clb			nclob,"																										+
				"f_clb_df		nclob				default 'abc',"																			+
				"f_clb_nn		nclob																not null,"								+
				"f_clb_df_nn	nclob				default 'HOGE'									not null,"								+
				"f_blb			blob,"																										+
				"f_blb_df		blob				default '321',"																			+
				"f_blb_nn		blob																not null,"								+
				"f_blb_df_nn	blob				default 'hijklmn'								not null,"								+
				"f_int_a		int					array[10],"																				+
				"f_int_a_nn		int					array[10]										not null,"								+
				"f_int_nla		int					array[no limit],"																		+
				"f_int_nla_nn	int					array[no limit]									not null,";

		// bigint 列は v15.0 からサポート
		query = query +
				"f_big_a		bigint				array[6],"																				+
				"f_big_a_nn		bigint				array[6]										not null,"								+
				"f_big_nla		bigint				array[no limit],"																		+
				"f_big_nla_nn	bigint				array[no limit]									not null,";

		// decimal 列は v16.1 からサポート
		query = query +
				"f_dec_a		decimal(10,5)		array[6],"																				+
				"f_dec_a_nn		decimal(10,5)		array[6]										not null,"								+
				"f_dec_nla		decimal(10,5)		array[no limit],"																		+
				"f_dec_nla_nn	decimal(10,5)		array[no limit]									not null,";

		query = query +
				"f_flt_a		float				array[20],"																				+
				"f_flt_a_nn		float				array[20]										not null,"								+
				"f_flt_nla		float				array[no limit],"																		+
				"f_flt_nla_nn	float				array[no limit]									not null,"								+
				"f_dat_a		datetime			array[3],"																				+
				"f_dat_a_nn		datetime			array[3]										not null,"								+
				"f_dat_nla		datetime			array[no limit],"																		+
				"f_dat_nla_nn	datetime			array[no limit]									not null,"								+
				"f_uni_a		uniqueidentifier	array[5],"																				+
				"f_uni_a_nn		uniqueidentifier	array[5]										not null,"								+
				"f_uni_nla		uniqueidentifier	array[no limit],"																		+
				"f_uni_nla_nn	uniqueidentifier	array[no limit]									not null,"								+
				"f_img_a		image				array[10],"																				+
				"f_img_a_nn		image				array[10]										not null,"								+
				"f_img_nla		image				array[no limit],"																		+
				"f_img_nla_nn	image				array[no limit]									not null,"								+
				"f_lng_a		language			array[30],"																				+
				"f_lng_a_nn		language			array[30]										not null,"								+
				"f_lng_nla		language			array[no limit],"																		+
				"f_lng_nla_nn	language			array[no limit]									not null,"								+
				"f_nch_a		nchar(64)			array[5],"																				+
				"f_nch_a_nn		nchar(64)			array[5]										not null,"								+
				"f_nch_nla		nchar(64)			array[no limit],"																		+
				"f_nch_nla_nn	nchar(64)			array[no limit]									not null,"								+
				"f_chr_a		char(8)				array[20],"																				+
				"f_chr_a_nn		char(8)				array[20]										not null,"								+
				"f_chr_nla		char(8)				array[no limit],"																		+
				"f_chr_nla_nn	char(8)				array[no limit]									not null,"								+
				"f_nvc_a		nvarchar(256)		array[3],"																				+
				"f_nvc_a_nn		nvarchar(256)		array[3]										not null,"								+
				"f_nvc_nla		nvarchar(256)		array[no limit],"																		+
				"f_nvc_nla_nn	nvarchar(256)		array[no limit]									not null,"								+
				"f_vch_a		varchar(128)		array[5],"																				+
				"f_vch_a_nn		varchar(128)		array[5]										not null,"								+
				"f_vch_nla		varchar(128)		array[no limit],"																		+
				"f_vch_nla_nn	varchar(128)		array[no limit]									not null,"								+
				"f_ntx_a		ntext				array[10],"																				+
				"f_ntx_a_nn		ntext				array[10]										not null,"								+
				"f_ntx_nla		ntext				array[no limit],"																		+
				"f_ntx_nla_nn	ntext				array[no limit]									not null,"								+
				"f_ntc_a		ntext				array[8]													hint heap 'compressed',"	+
				"f_ntc_a_nn		ntext				array[8]										not null	hint heap 'compressed',"	+
				"f_ntc_nla		ntext				array[no limit]												hint heap 'compressed',"	+
				"f_ntc_nla_nn	ntext				array[no limit]									not null	hint heap 'compressed',"	+
				"f_ftx_a		fulltext			array[3],"																				+
				"f_ftx_a_nn		fulltext			array[3]										not null,"								+
				"f_ftx_nla		fulltext			array[no limit],"																		+
				"f_ftx_nla_nn	fulltext			array[no limit]									not null,"								+
				"f_bin_a		binary(50)			array[8],"																				+
				"f_bin_a_nn		binary(50)			array[8]										not null,"								+
				"f_bin_nla		binary(50)			array[no limit],"																		+
				"f_bin_nla_nn	binary(50)			array[no limit]									not null,"								+
				"primary key(f_int_pk)"																										+
			")";
		assertEquals(0, s.executeUpdate(query));

		s.close();

		ResultSet	rs = null;

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その１
		//	条件
		//		表名パターン	未指定（ null 指定／空文字列指定）
		//		列名パターン	%Number%
		//		↓
		//	結果
		//		表名	列名			表中のインデックス	SQL データ型			Sydney データ型名	文字数/精度	nullable
		//		tbl1001	serialNumber	1					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		//		tbl1001	areaNumber		3					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		//		tbl1013	typeNumber		4					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		for (int i = 0; i < 2; i++) {

			String	tableNamePattern = null;
			if (i > 0) tableNamePattern = "";
			assertNotNull(rs = c.getMetaData().getColumns(null, null, tableNamePattern, "%Number%"));
			ColumnInfo[]	columnInfos = new ColumnInfo[3];
			columnInfos[0] = new ColumnInfo("tbl1001",	"serialNumber",	1,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES, 0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[1] = new ColumnInfo("tbl1001",	"areaNumber",	3,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES,	0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[2] = new ColumnInfo("tbl1013",	"typeNumber",	4,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES,	0,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その２
		//	条件
		//		表名パターン	tbl100%
		//		列名パターン	%Number%
		//		↓
		//	結果
		//		表名	列名			表中のインデックス	SQL データ型			Sydney データ型名	文字数/精度	nullable
		//		tbl1001	serialNumber	1					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		//		tbl1001	areaNumber		3					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		{
			assertNotNull(rs = c.getMetaData().getColumns(null, null, "tbl100%", "%Number%"));
			ColumnInfo[]	columnInfos = new ColumnInfo[2];
			columnInfos[0] = new ColumnInfo("tbl1001",	"serialNumber",	1,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES,	0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[1] = new ColumnInfo("tbl1001",	"areaNumber",	3,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES,	0,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その３
		//	条件
		//		表名パターン	未指定（ null 指定／空文字列指定）
		//		列名パターン	fldN%
		//		↓
		//	結果
		//		表名	列名					表中のインデックス	SQL データ型			Sydney データ型名	文字数/精度	nullable
		//		tbl1003	fldNChar64				 8					java.sql.Types.CHAR		nchar				 64			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNVarChar256			10					java.sql.Types.VARCHAR	nvarchar			256			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNText				12					java.sql.Types.VARCHAR	nvarchar			 -1			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNClob				15					java.sql.Types.CLOB		nclob				 -1			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNChar64Array			24					java.sql.Types.ARRAY	nchar array			 64			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNVarChar256Array		26					java.sql.Types.ARRAY	nvarchar array		256			java.sql.DatabaseMetaData.columnNullable
		//		tbl1003	fldNTextArray			28					java.sql.Types.ARRAY	nvarchar array		 -1			java.sql.DatabaseMetaData.columnNullable
		//		tbl1013	fldNo					 5					java.sql.Types.INTEGER	int					 10			java.sql.DatabaseMetaData.columnNullable
		for (int i = 0; i < 2; i++) {

			String	tableNamePattern = null;
			if (i > 0) tableNamePattern = "";
			assertNotNull(rs = c.getMetaData().getColumns(null, null, tableNamePattern, "fldN%"));
			ColumnInfo[]	columnInfos = new ColumnInfo[8];
			int[] positions = {
				9,
				11,
				13,
				16,
				26,
				28,
				30,
				5
			};
			int j = 0;
			columnInfos[j]   = new ColumnInfo("tbl1003",	"fldNChar64",	positions[j],	java.sql.Types.CHAR,	"nchar",	 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNVarChar256", positions[j],	java.sql.Types.VARCHAR,	"nvarchar",	256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNText",		positions[j],	java.sql.Types.VARCHAR,	"nvarchar",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNClob",		positions[j],	java.sql.Types.CLOB,	"nclob",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNChar64Array", positions[j],java.sql.Types.ARRAY,	"nchar array", 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNVarChar256Array", positions[j], java.sql.Types.ARRAY, "nvarchar array",256,ColumnInfo.CHECK_BYTES,	512,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1003",	"fldNTextArray", positions[j],	java.sql.Types.ARRAY,	"nvarchar array",-1,ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[++j] = new ColumnInfo("tbl1013",	"fldNo",		positions[j],	java.sql.Types.INTEGER,	"int",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その４
		//	条件
		//		表名パターン	tbl101%
		//		列名パターン	fldN%
		//		↓
		//	結果
		//		表名	列名	表中のインデックス	SQL データ型			Sydney データ型名	文字数/精度	nullable
		//		tbl1013	fldNo	5					java.sql.Types.INTEGER	int					10			java.sql.DatabaseMetaData.columnNullable
		{
			assertNotNull(rs = c.getMetaData().getColumns(null, null, "tbl101%", "fldN%"));
			ColumnInfo[]	columnInfos = new ColumnInfo[1];
			columnInfos[0] = new ColumnInfo("tbl1013",	"fldNo",	5,	java.sql.Types.INTEGER,	"int",	10,	ColumnInfo.NOT_CHECK_BYTES,	0,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その５
		//	条件
		//		表名パターン	未指定（ null 指定／空文字列指定）
		//		列名パターン	name
		//		↓
		//	結果
		//		表名	列名	表中のインデックス	SQL データ型			Sydney データ型名	文字数/精度	nullable
		//		tbl1001	name	2					java.sql.Types.CHAR		char				100			java.sql.DatabaseMetaData.columnNullable
		//		tbl1013	name	2					java.sql.Types.VARCHAR	nvarchar			500			java.sql.DatabaseMetaData.columnNullable
		for (int i = 0; i < 2; i++) {

			String	tableNamePattern = null;
			if (i > 0) tableNamePattern = "";
			assertNotNull(rs = c.getMetaData().getColumns(null, null, tableNamePattern, "name"));
			ColumnInfo[]	columnInfos = new ColumnInfo[2];
			columnInfos[0] = new ColumnInfo("tbl1001",	"name",	2,	java.sql.Types.CHAR,		"char",		100,	ColumnInfo.CHECK_BYTES,	 100,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[1] = new ColumnInfo("tbl1013",	"name",	2,	java.sql.Types.VARCHAR,		"nvarchar",	500,	ColumnInfo.CHECK_BYTES,	1000,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 列名パターンに一致する列が存在する場合には一致するすべての列の情報が得られるはず − その６
		//	条件
		//		表名パターン	tbl100%
		//		列名パターン	name
		//		↓
		//	結果
		//		表名	列名	表中のインデックス	SQL データ型		Sydney データ型名	文字数/精度	nullable
		//		tbl1001	name	2					java.sql.Types.CHAR	char				100			java.sql.DatabaseMetaData.columnNullable
		{
			assertNotNull(rs = c.getMetaData().getColumns(null, null, "tbl100%", "name"));
			ColumnInfo[]	columnInfos = new ColumnInfo[1];
			columnInfos[0] = new ColumnInfo("tbl1001",	"name",	2,	java.sql.Types.CHAR, "char",	100,	ColumnInfo.CHECK_BYTES,	100,	0,	java.sql.DatabaseMetaData.columnNullable);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		{
			String hintCompressed = "heap 'compressed'";
			assertNotNull(rs = c.getMetaData().getColumns(null, null, "t1", "f_%"));
			int	i = 0;
			int size = 137;
			ColumnInfo[]	columnInfos = new ColumnInfo[size];
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_pk",		i,	java.sql.Types.INTEGER,		"int",				 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int",		i,	java.sql.Types.INTEGER,		"int",				 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_df",		i,	java.sql.Types.INTEGER,		"int",				 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"32");
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_nn",		i,	java.sql.Types.INTEGER,		"int",				 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_df_nn",	i,	java.sql.Types.INTEGER,		"int",				 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"98");

			// bigint 列は v15.0 からサポート
			columnInfos[i++] = new ColumnInfo("t1",	"f_big",		i,	java.sql.Types.BIGINT,	"bigint",			 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_df",		i,	java.sql.Types.BIGINT,	"bigint",			 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"43298741683643");
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_nn",		i,	java.sql.Types.BIGINT,	"bigint",			 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_df_nn",	i,	java.sql.Types.BIGINT,	"bigint",			 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"7846827364876234");

			// decimal 列は v16.1 からサポート
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec",		i,	java.sql.Types.DECIMAL,	"decimal",			 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_df",		i,	java.sql.Types.DECIMAL,	"decimal",			 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNullable,	"99999.99999");
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_nn",		i,	java.sql.Types.DECIMAL,	"decimal",			 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_df_nn",	i,	java.sql.Types.DECIMAL,	"decimal",			 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNoNulls,	"99999.99999");

			columnInfos[i++] = new ColumnInfo("t1",	"f_flt",		i,	java.sql.Types.DOUBLE,		"float",			 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_df",		i,	java.sql.Types.DOUBLE,		"float",			 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"7.9075E0");
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_nn",		i,	java.sql.Types.DOUBLE,		"float",			 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_df_nn",	i,	java.sql.Types.DOUBLE,		"float",			 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"6.023E-3");
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat",		i,	java.sql.Types.TIMESTAMP,	"datetime",			 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_df",		i,	java.sql.Types.TIMESTAMP,	"datetime",			 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"2005-03-03 11:41:39.683");
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_nn",		i,	java.sql.Types.TIMESTAMP,	"datetime",			 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_df_nn",	i,	java.sql.Types.TIMESTAMP,	"datetime",			 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"2005-02-28 09:12:34.393");
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni",		i,	java.sql.Types.CHAR,		"char",				 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_df",		i,	java.sql.Types.CHAR,		"char",				 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNullable,	"D599A207-25BC-4e20-BE23-9C6F777C193C");
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_nn",		i,	java.sql.Types.CHAR,		"char",				 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_df_nn",	i,	java.sql.Types.CHAR,		"char",				 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"5E42CFEA-9BF5-480c-9DB6-72D4118F09B4");
			columnInfos[i++] = new ColumnInfo("t1",	"f_img",		i,	java.sql.Types.VARBINARY,	"varbinary",		 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_df",		i,	java.sql.Types.VARBINARY,	"varbinary",		 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"abcdefg");
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_nn",		i,	java.sql.Types.VARBINARY,	"varbinary",		 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_df_nn",	i,	java.sql.Types.VARBINARY,	"varbinary",		 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"ABCD");
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng",		i,	java.sql.Types.OTHER,		"language",			  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_df",		i,	java.sql.Types.OTHER,		"language",			  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"en");
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_nn",		i,	java.sql.Types.OTHER,		"language",			  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_df_nn",	i,	java.sql.Types.OTHER,		"language",			  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"ja");
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch",		i,	java.sql.Types.CHAR,		"nchar",			 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_df",		i,	java.sql.Types.CHAR,		"nchar",			 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable,	"hogehoge");
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_nn",		i,	java.sql.Types.CHAR,		"nchar",			 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_df_nn",	i,	java.sql.Types.CHAR,		"nchar",			 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"foo");
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr",		i,	java.sql.Types.CHAR,		"char",				  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_df",		i,	java.sql.Types.CHAR,		"char",				  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNullable,	"def");
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_nn",		i,	java.sql.Types.CHAR,		"char",				  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_df_nn",	i,	java.sql.Types.CHAR,		"char",				  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"hoge");
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc",		i,	java.sql.Types.VARCHAR,		"nvarchar",			256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_df",		i,	java.sql.Types.VARCHAR,		"nvarchar",			256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNullable,	"AABBCCDDEEFFGG");
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_nn",		i,	java.sql.Types.VARCHAR,		"nvarchar",			256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_df_nn",	i,	java.sql.Types.VARCHAR,		"nvarchar",			256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"abcdefg");
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch",		i,	java.sql.Types.VARCHAR,		"varchar",			128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_df",		i,	java.sql.Types.VARCHAR,		"varchar",			128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable,	"false");
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_nn",		i,	java.sql.Types.VARCHAR,		"varchar",			128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_df_nn",	i,	java.sql.Types.VARCHAR,		"varchar",			128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"xxxxxxx");
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_df",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	"text data");
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_nn",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_df_nn",	i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"text data 2");
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	null,						hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_df",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	"compressed text data",		hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_nn",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	null,						hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_df_nn",	i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"compressed text value",	hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_df",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	"fulltext default data");
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_nn",		i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_df_nn",	i,	java.sql.Types.VARCHAR,		"nvarchar",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"fulltext default data");
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin",		i,	java.sql.Types.BINARY,		"binary",			 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_df",		i,	java.sql.Types.BINARY,		"binary",			 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"8798234");
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_nn",		i,	java.sql.Types.BINARY,		"binary",			 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_df_nn",	i,	java.sql.Types.BINARY,		"binary",			 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"xyz");
			columnInfos[i++] = new ColumnInfo("t1",	"f_clb",		i,	java.sql.Types.CLOB,		"nclob",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_clb_df",		i,	java.sql.Types.CLOB,		"nclob",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	"abc");
			columnInfos[i++] = new ColumnInfo("t1",	"f_clb_nn",		i,	java.sql.Types.CLOB,		"nclob",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_clb_df_nn",	i,	java.sql.Types.CLOB,		"nclob",			 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"HOGE");
			columnInfos[i++] = new ColumnInfo("t1",	"f_blb",		i,	java.sql.Types.BLOB,		"blob",				 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_blb_df",		i,	java.sql.Types.BLOB,		"blob",				 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable,	"321");
			columnInfos[i++] = new ColumnInfo("t1",	"f_blb_nn",		i,	java.sql.Types.BLOB,		"blob",				 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_blb_df_nn",	i,	java.sql.Types.BLOB,		"blob",				 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls,	"hijklmn");
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_a",		i,	java.sql.Types.ARRAY,		"int array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_a_nn",	i,	java.sql.Types.ARRAY,		"int array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_nla",	i,	java.sql.Types.ARRAY,		"int array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_int_nla_nn",	i,	java.sql.Types.ARRAY,		"int array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);

			// bigint 列は v15.0 からサポート
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_a",		i,	java.sql.Types.ARRAY,		"bigint array",		 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_a_nn",	i,	java.sql.Types.ARRAY,		"bigint array",		 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_nla",	i,	java.sql.Types.ARRAY,		"bigint array",		 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_big_nla_nn",	i,	java.sql.Types.ARRAY,		"bigint array",		 19,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);

			// decimal 列は v16.1 からサポート
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_a",		i,	java.sql.Types.ARRAY,		"decimal array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_a_nn",	i,	java.sql.Types.ARRAY,		"decimal array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_nla",	i,	java.sql.Types.ARRAY,		"decimal array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dec_nla_nn",	i,	java.sql.Types.ARRAY,		"decimal array",		 10,	ColumnInfo.NOT_CHECK_BYTES,	  0,	5,	java.sql.DatabaseMetaData.columnNoNulls);

			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_a",		i,	java.sql.Types.ARRAY,		"float array",		 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_a_nn",	i,	java.sql.Types.ARRAY,		"float array",		 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_nla",	i,	java.sql.Types.ARRAY,		"float array",		 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_flt_nla_nn",	i,	java.sql.Types.ARRAY,		"float array",		 15,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_a",		i,	java.sql.Types.ARRAY,		"datetime array",	 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_a_nn",	i,	java.sql.Types.ARRAY,		"datetime array",	 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_nla",	i,	java.sql.Types.ARRAY,		"datetime array",	 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_dat_nla_nn",	i,	java.sql.Types.ARRAY,		"datetime array",	 23,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_a",		i,	java.sql.Types.ARRAY,		"char array",		 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_a_nn",	i,	java.sql.Types.ARRAY,		"char array",		 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_nla",	i,	java.sql.Types.ARRAY,		"char array",		 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_uni_nla_nn",	i,	java.sql.Types.ARRAY,		"char array",		 36,	ColumnInfo.CHECK_BYTES,		 36,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_a",		i,	java.sql.Types.ARRAY,		"varbinary array",	 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_a_nn",	i,	java.sql.Types.ARRAY,		"varbinary array",	 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_nla",	i,	java.sql.Types.ARRAY,		"varbinary array",	 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_img_nla_nn",	i,	java.sql.Types.ARRAY,		"varbinary array",	 -1,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_a",		i,	java.sql.Types.ARRAY,		"language array",	  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_a_nn",	i,	java.sql.Types.ARRAY,		"language array",	  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_nla",	i,	java.sql.Types.ARRAY,		"language array",	  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_lng_nla_nn",	i,	java.sql.Types.ARRAY,		"language array",	  0,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_a",		i,	java.sql.Types.ARRAY,		"nchar array",		 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_a_nn",	i,	java.sql.Types.ARRAY,		"nchar array",		 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_nla",	i,	java.sql.Types.ARRAY,		"nchar array",		 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nch_nla_nn",	i,	java.sql.Types.ARRAY,		"nchar array",		 64,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_a",		i,	java.sql.Types.ARRAY,		"char array",		  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_a_nn",	i,	java.sql.Types.ARRAY,		"char array",		  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_nla",	i,	java.sql.Types.ARRAY,		"char array",		  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_chr_nla_nn",	i,	java.sql.Types.ARRAY,		"char array",		  8,	ColumnInfo.CHECK_BYTES,		  8,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_a",		i,	java.sql.Types.ARRAY,		"nvarchar array",	256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_a_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_nla",	i,	java.sql.Types.ARRAY,		"nvarchar array",	256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_nvc_nla_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	256,	ColumnInfo.CHECK_BYTES,		512,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_a",		i,	java.sql.Types.ARRAY,		"varchar array",	128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_a_nn",	i,	java.sql.Types.ARRAY,		"varchar array",	128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_nla",	i,	java.sql.Types.ARRAY,		"varchar array",	128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_vch_nla_nn",	i,	java.sql.Types.ARRAY,		"varchar array",	128,	ColumnInfo.CHECK_BYTES,		128,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_a",		i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_a_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_nla",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntx_nla_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_a",		i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	null,	hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_a_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	null,	hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_nla",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable,	null,	hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ntc_nla_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls,	null,	hintCompressed);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_a",		i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_a_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_nla",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_ftx_nla_nn",	i,	java.sql.Types.ARRAY,		"nvarchar array",	 -1,	ColumnInfo.CHECK_BYTES,		 -1,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_a",		i,	java.sql.Types.ARRAY,		"binary array",		 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_a_nn",	i,	java.sql.Types.ARRAY,		"binary array",		 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_nla",	i,	java.sql.Types.ARRAY,		"binary array",		 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNullable);
			columnInfos[i++] = new ColumnInfo("t1",	"f_bin_nla_nn",	i,	java.sql.Types.ARRAY,		"binary array",		 50,	ColumnInfo.NOT_CHECK_BYTES,	  0,	0,	java.sql.DatabaseMetaData.columnNoNulls);
			assertColumnResults(rs, columnInfos);
			rs.close();
		}

		// 後始末
		s = c.createStatement();
		s.executeUpdate("drop table tbl1001");
		s.executeUpdate("drop table tbl1003");
		s.executeUpdate("drop table tbl1013");
		s.executeUpdate("drop table t1");
		s.close();

		c.close();
	}

	// DatabaseMetaData.getColumnPrivileges() のテスト
	public void test_getColumnPrivileges() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではアクセス権をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getColumnPrivileges(null, null, null, null).next());
		//                                                                     ~~~~~~~ 空の ResultSet なので、
		//                                                                             初回 next() の戻り値が
		//                                                                             false であるはず

		c.close();
	}

	// DatabaseMetaData.getTablePrivileges() のテスト
	public void test_getTablePrivileges() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではアクセス権をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getTablePrivileges(null, null, null).next());
		//                                                              ~~~~~~~ 空の ResultSet なので、
		//                                                                      初回 next() の戻り値が
		//                                                                      false であるはず

		c.close();
	}

	// DatabaseMetaData.getBestRowIdentifier() のテスト
	public void test_getBestRowIdentifier() throws Exception
	{
		Connection	c = getConnection();

		// 下準備

		//	table01
		//		id

		//	table11
		//		id

		//	table12
		//		id

		Statement	s = c.createStatement();
		s.executeUpdate("create table table01 (id int)");
		s.executeUpdate("create table table11 (id int)");
		s.executeUpdate("create table table12 (id int)");
		s.close();

		ResultSet	rs = null;

		// 表名に一致する表が存在する場合には得られるはず − その１
		assertNotNull(rs = c.getMetaData().getBestRowIdentifier(null, null, "table01", 0, false));
		assertBestRowID(rs);
		rs.close();

		// 表名に一致する表が存在する場合には得られるはず − その２
		assertNotNull(rs = c.getMetaData().getBestRowIdentifier(null, null, "table11", 0, false));
		assertBestRowID(rs);
		rs.close();

		// 表名に一致する表が存在する場合には得られるはず − その３
		assertNotNull(rs = c.getMetaData().getBestRowIdentifier(null, null, "table12", 0, false));
		assertBestRowID(rs);
		rs.close();

		// 表名に一致する表が存在する場合には得られるはず − その４
		// ※ 一致する表が複数存在する場合でも得られるのはひとつだけかを確認
		assertNotNull(rs = c.getMetaData().getBestRowIdentifier(null, null, "table1%", 0, false));
		assertBestRowID(rs);
		rs.close();

		// 表名に一致する表が存在しない場合には空の ResultSet が得られるはず
		assertNotNull(rs = c.getMetaData().getBestRowIdentifier(null, null, "table05", 0, false));
		assertFalse(rs.next());
		rs.close();

		// 後始末
		s = c.createStatement();
		s.executeUpdate("drop table table01");
		s.executeUpdate("drop table table11");
		s.executeUpdate("drop table table12");
		s.close();

		c.close();
	}

	// DatabaseMetaData.getVersionColumns() のテスト
	public void test_getVersionColumns() throws Exception
	{
		Connection	c = getConnection();

		// 現状では空の ResultSet であるはず
		assertFalse(c.getMetaData().getVersionColumns(null, null, null).next());
		//                                                             ~~~~~~~ 空の ResultSet なので、
		//                                                                     初回 next() の戻り値が
		//                                                                     false であるはず

		c.close();
	}

	// DatabaseMetaData.getPrimaryKeys() のテスト
	public void test_getPrimaryKeys() throws Exception
	{
		Connection	c = getConnection();

		// 下準備

		// table01
		//		id		<- primary key
		//		name
		//		address

		// table02
		//		id
		//		name
		//		address

		Statement	s = c.createStatement();
		s.executeUpdate("create table table01 (id int, name char(100), address ntext, primary key (id))");
		s.executeUpdate("create table table02 (id int, name char(100), address ntext)");
		s.close();

		ResultSet	rs = null;

		// 表名に一致する表が存在し主キーも存在する場合には得られるはず
		String	tableName = "table01";
		assertNotNull(rs = c.getMetaData().getPrimaryKeys(null, null, tableName));
		assertPrimaryKeys(rs, tableName, "id", (short)0, "");
		rs.close();

		// 表名に一致する表が存在してもその表に主キーが存在しない場合には空の ResultSet が得られるはず
		tableName = "table02";
		assertNotNull(rs = c.getMetaData().getPrimaryKeys(null, null, tableName));
		assertFalse(rs.next());
		rs.close();

		// 表名に一致する表が存在しない場合には空の ResultSet が得られるはず
		tableName = "table99";
		assertNotNull(rs = c.getMetaData().getPrimaryKeys(null, null, tableName));
		assertFalse(rs.next());
		rs.close();

		// 後始末
		s = c.createStatement();
		s.executeUpdate("drop table table01");
		s.executeUpdate("drop table table02");
		s.close();

		c.close();
	}

	// DatabaseMetaData.getImportedKeys() のテスト
	public void test_getImportedKeys() throws Exception
	{
		Connection	c = getConnection();

		// 現状では外部キーをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getImportedKeys(null, null, null).next());
		//                                                           ~~~~~~~ 空の ResultSet なので、
		//                                                                   初回 next() の戻り値が
		//                                                                   false であるはず

		c.close();
	}

	// DatabaseMetaData.getExportedKeys() のテスト
	public void test_getExportedKeys() throws Exception
	{
		Connection	c = getConnection();

		// 現状では外部キーをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getExportedKeys(null, null, null).next());
		//                                                           ~~~~~~~ 空の ResultSet なので、
		//                                                                   初回 next() の戻り値が
		//                                                                   false であるはず

		c.close();
	}

	// DatabaseMetaData.getCrossReference() のテスト
	public void test_getCrossReference() throws Exception
	{
		Connection	c = getConnection();

		// 現状では外部キーをサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getCrossReference(null, null, null, null, null, null).next());
		//                                                                               ~~~~~~~ 空の ResultSet なので、
		//                                                                                       初回 next() の戻り値が
		//                                                                                       false であるはず

		c.close();
	}

	// DatabaseMetaData.getTypeInfo() のテスト
	public void test_getTypeInfo() throws Exception
	{
		Connection	c = getConnection();

		ResultSet	rs = null;
		assertTypeInfo(rs = c.getMetaData().getTypeInfo());
		rs.close();

		c.close();
	}

	// DatabaseMetaData.getIndexInfo() のテスト
	public void test_getIndexInfo() throws Exception
	{
		Connection	c = getConnection();

		// 下準備

		// table01
		//		id		<- primary key
		//		name	<- index
		//		address
		//		phone	<- unique
		//		type	<- index
		//		subtype	<- index

		// table02
		//		title	<- index
		//		subtitle
		//		memo	<- fulltext index
		//		price	<- index
		//		serial	<- unique + index

		// table03
		//		id
		//		name

		Statement	s = c.createStatement();
		s.executeUpdate("create table table01 (id int, name char(100), address ntext, phone char(50), type int, subtype int, primary key (id), unique (phone))");
		s.executeUpdate("create index index01_name on table01(name)");
		s.executeUpdate("create index index01_types on table01(type, subtype)");

		s.executeUpdate("create table table02 (title nchar(100), subtitle nchar(200), memo ntext, price int, serial int, unique (serial))");
		s.executeUpdate("create index index02_title on table02(title)");
		s.executeUpdate("create fulltext index index02_memo on table02(memo)");
		s.executeUpdate("create index index02_price on table02(price)");
		s.executeUpdate("create index index02_serial on table02(serial)");

		s.executeUpdate("create table table03 (id int, name char(100))");

		s.close();

		ResultSet	rs = null;

		// 表名に一致する表が存在し索引も付けられていれば得られるはず−その１（ユニークのみ）
		String	tableName = "table01";
		boolean	unique = true;
		{
			assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
			boolean[]	nonUniques			= { false,					false					};
			String[]	indexNames			= {	"UNIQUE_phone",			"table01_$$PrimaryKey"	};
			short[]		ordinalPositions	= { 1,						1						};
			String[]	columnNames			= {	"phone",				"id"					};
			assertIndexInfo(	rs,
								tableName,
								nonUniques,
								indexNames,
								ordinalPositions,
								columnNames);
			rs.close();
		}

		// 表名に一致する表が存在し索引も付けられていれば得られるはず−その２（ユニーク以外も）
		unique = false;
		{
			assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
			boolean[]	nonUniques			= { true,					true,					true,					false,					false					};
			String[]	indexNames			= {	"index01_name",			"index01_types",		"index01_types",		"UNIQUE_phone",			"table01_$$PrimaryKey"	};
			short[]		ordinalPositions	= { 1,						1,						2,						1,						1						};
			String[]	columnNames			= {	"name",					"type",					"subtype",				"phone",				"id"					};
			assertIndexInfo(	rs,
								tableName,
								nonUniques,
								indexNames,
								ordinalPositions,
								columnNames);
			rs.close();
		}

		// 表名に一致する表が存在し索引も付けられていれば得られるはず−その３（ユニークのみ）
		unique = true;
		tableName = "table02";
		{
			assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
			boolean[]	nonUniques			= { false			};
			String[]	indexNames			= { "UNIQUE_serial"	};
			short[]		ordinalPositions	= { 1				};
			String[]	columnNames			= { "serial"		};
			assertIndexInfo(	rs,
								tableName,
								nonUniques,
								indexNames,
								ordinalPositions,
								columnNames);
			rs.close();
		}

		// 表名に一致する表が存在し索引も付けられていれば得られるはず−その４（ユニーク以外も）
		unique = false;
		{
			assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
			boolean[]	nonUniques			= { true,					true,					true,					true,					false			};
			String[]	indexNames			= { "index02_memo",			"index02_price",		"index02_serial",		"index02_title",		"UNIQUE_serial"	};
			short[]		ordinalPositions	= { 1,						1,						1,						1,						1				};
			String[]	columnNames			= { "memo",					"price",				"serial",				"title",				"serial"		};
			assertIndexInfo(	rs,
								tableName,
								nonUniques,
								indexNames,
								ordinalPositions,
								columnNames);
			rs.close();
		}

		// 表名に一致する表が存在してもその表中の列に索引が付けられていない場合には空の ResultSet が得られるはず−その１（ユニークのみ）
		unique = true;
		tableName = "table03";
		assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
		assertFalse(rs.next());
		rs.close();

		// 表名に一致する表が存在してもその表中の列に索引が付けられていない場合には空の ResultSet が得られるはず−その２（ユニーク以外も）
		unique = false;
		assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
		assertFalse(rs.next());
		rs.close();

		// 表名に一致する表が存在しない場合には空の ResultSet が得られるはず−その１（ユニークのみ）
		unique = true;
		tableName = "table99";
		assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
		assertFalse(rs.next());
		rs.close();

		// 表名に一致する表が存在しない場合には空の ResultSet が得られるはず−その２（ユニーク以外も）
		unique = false;
		assertNotNull(rs = c.getMetaData().getIndexInfo(null, null, tableName, unique, false));
		assertFalse(rs.next());
		rs.close();

		// 後始末
		s = c.createStatement();
		s.executeUpdate("drop table table01");
		s.executeUpdate("drop table table02");
		s.executeUpdate("drop table table03");
		s.close();

		c.close();
	}

	// DatabaseMetaData.supportsResultSetType() のテスト
	public void test_supportsResultSetType() throws Exception
	{
		Connection	c = getConnection();

		// 以下、サポートしているはずの結果セットの型
		assertTrue(c.getMetaData().supportsResultSetType(ResultSet.TYPE_FORWARD_ONLY));
		// 以下、サポートしていないはずの結果セットの型
		assertFalse(c.getMetaData().supportsResultSetType(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().supportsResultSetType(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().supportsResultSetType(-1));

		c.close();
	}

	// DatabaseMetaData.supportsResultSetConcurrency() のテスト
	public void test_supportsResultSetConcurrency() throws Exception
	{
		Connection	c = getConnection();

		// 以下、サポートしているはずの結果セットの型と並行処理の種類の組み合わせ
		assertTrue(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_READ_ONLY));
		// 以下、サポートしていないはずの結果セットの型と並行処理の種類の組み合わせ
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_FORWARD_ONLY, -1));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_READ_ONLY));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_UPDATABLE));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_INSENSITIVE, -1));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_READ_ONLY));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(ResultSet.TYPE_SCROLL_SENSITIVE, -1));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(-1, ResultSet.CONCUR_READ_ONLY));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(-1, ResultSet.CONCUR_UPDATABLE));
		assertFalse(c.getMetaData().supportsResultSetConcurrency(-1, -1));

		c.close();
	}

	// DatabaseMetaData.ownUpdatesAreVisible() のテスト
	public void test_ownUpdatesAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では結果セットの更新をサポートしていないので常に false のはず
		assertFalse(c.getMetaData().ownUpdatesAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().ownUpdatesAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().ownUpdatesAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().ownUpdatesAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.ownDeletesAreVisible() のテスト
	public void test_ownDeletesAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では結果セットの削除をサポートしていないので常に false のはず
		assertFalse(c.getMetaData().ownDeletesAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().ownDeletesAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().ownDeletesAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().ownDeletesAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.ownInsertsAreVisible() のテスト
	public void test_ownInsertsAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では結果セットの挿入をサポートしていないので常に false のはず
		assertFalse(c.getMetaData().ownInsertsAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().ownInsertsAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().ownInsertsAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().ownInsertsAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.othersUpdatesAreVisible() のテスト
	public void test_othersUpdatesAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では他で行われた更新は可視ではないはず
		assertFalse(c.getMetaData().othersUpdatesAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().othersUpdatesAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().othersUpdatesAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().othersUpdatesAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.othersDeletesAreVisible() のテスト
	public void test_othersDeletesAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では他で行われた削除は可視ではないはず
		assertFalse(c.getMetaData().othersDeletesAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().othersDeletesAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().othersDeletesAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().othersDeletesAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.othersInsertsAreVisible() のテスト
	public void test_othersInsertsAreVisible() throws Exception
	{
		Connection	c = getConnection();

		// 現状では他で行われた挿入は可視ではないはず
		assertFalse(c.getMetaData().othersInsertsAreVisible(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().othersInsertsAreVisible(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().othersInsertsAreVisible(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().othersInsertsAreVisible(-1));

		c.close();
	}

	// DatabaseMetaData.updatesAreDetected() のテスト
	public void test_updatesAreDetected() throws Exception
	{
		Connection	c = getConnection();

		// 現状では可視の行が更新されたことを検出できないはず
		assertFalse(c.getMetaData().updatesAreDetected(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().updatesAreDetected(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().updatesAreDetected(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().updatesAreDetected(-1));

		c.close();
	}

	// DatabaseMetaData.deletesAreDetected() のテスト
	public void test_deletesAreDetected() throws Exception
	{
		Connection	c = getConnection();

		// 現状では可視の行が削除されたことを検出できないはず
		assertFalse(c.getMetaData().deletesAreDetected(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().deletesAreDetected(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().deletesAreDetected(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().deletesAreDetected(-1));

		c.close();
	}

	// DatabaseMetaData.insertsAreDetected() のテスト
	public void test_insertsAreDetected() throws Exception
	{
		Connection	c = getConnection();

		// 現状では可視の行が挿入されたことを検出できないはず
		assertFalse(c.getMetaData().insertsAreDetected(ResultSet.TYPE_FORWARD_ONLY));
		assertFalse(c.getMetaData().insertsAreDetected(ResultSet.TYPE_SCROLL_INSENSITIVE));
		assertFalse(c.getMetaData().insertsAreDetected(ResultSet.TYPE_SCROLL_SENSITIVE));
		assertFalse(c.getMetaData().insertsAreDetected(-1));

		c.close();
	}

	// DatabaseMetaData.supportsBatchUpdates() のテスト
	public void test_supportsBatchUpdates() throws Exception
	{
		Connection	c = getConnection();

		// バッチ更新可能なはず
		assertTrue(c.getMetaData().supportsBatchUpdates());

		c.close();
	}

	// DatabaseMetaData.getUDTs() のテスト
	public void test_getUDTs() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではユーザ定義型をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getUDTs(null, null, null, null).next());
		//                                                         ~~~~~~~ 空の ResultSet なので、
		//                                                                 初回 next() の戻り値が
		//                                                                 false であるはず

		c.close();
	}

	// DatabaseMetaData.getConnection() のテスト
	public void test_getConnection() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(c, c.getMetaData().getConnection());

		c.close();
	}

	// DatabaseMetaData.supportsSavepoints() のテスト
	public void test_supportsSavepoints() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではセーブポイントをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsSavepoints());

		c.close();
	}

	// DatabaseMetaData.supportsNamedParameters() のテスト
	public void test_supportsNamedParameters() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsNamedParameters());

		c.close();
	}

	// DatabaseMetaData.supportsMultipleOpenResults() のテスト
	public void test_supportsMultipleOpenResults() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではストアドプロシージャをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsMultipleOpenResults());

		c.close();
	}

	// DatabaseMetaData.supportsGetGeneratedKeys() のテスト
	public void test_supportsGetGeneratedKeys() throws Exception
	{
		Connection	c = getConnection();

		// 現状では自動生成キーをサポートしていないので常に false のはず
		assertFalse(c.getMetaData().supportsGetGeneratedKeys());

		c.close();
	}

	// DatabaseMetaData.getSuperTypes() のテスト
	public void test_getSuperTypes() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではユーザ定義型および型の階層をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getSuperTypes(null, null, null).next());
		//                                                         ~~~~~~~ 空の ResultSet なので、
		//                                                                 初回 next() の戻り値が
		//                                                                 false であるはず

		c.close();
	}

	// DatabaseMetaData.getSuperTables() のテスト
	public void test_getSuperTables() throws Exception
	{
		Connection	c = getConnection();

		// 現状では型の階層をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getSuperTables(null, null, null).next());
		//                                                          ~~~~~~~ 空の ResultSet なので、
		//                                                                  初回 next() の戻り値が
		//                                                                  false であるはず

		c.close();
	}

	// DatabaseMetaData.getAttributes() のテスト
	public void test_getAttributes() throws Exception
	{
		Connection	c = getConnection();

		// 現状ではユーザ定義をサポートしていないので常に空の ResultSet であるはず
		assertFalse(c.getMetaData().getAttributes(null, null, null, null).next());
		//                                                               ~~~~~~~ 空の ResultSet なので、
		//                                                                       初回 next() の戻り値が
		//                                                                       false であるはず

		c.close();
	}

	// DatabaseMetaData.supportsResultSetHoldability() のテスト
	public void test_supportsResultSetHoldability() throws Exception
	{
		Connection	c = getConnection();

		// 以下、サポートしているはずの保持機能
		assertTrue(c.getMetaData().supportsResultSetHoldability(ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 以下、サポートしていないはずの保持機能
		assertFalse(c.getMetaData().supportsResultSetHoldability(ResultSet.HOLD_CURSORS_OVER_COMMIT));
		assertFalse(c.getMetaData().supportsResultSetHoldability(-1));

		c.close();
	}

	// DatabaseMetaData.getResultSetHoldability() のテスト
	public void test_getResultSetHoldability() throws Exception
	{
		Connection	c = getConnection();

		// デフォルトの保持機能は常に ResultSet.CLOSE_CURSORS_AT_COMMIT であるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, c.getMetaData().getResultSetHoldability());

		c.close();
	}

	// DatabaseMetaData.getDatabaseMajorVersion() のテスト
	public void test_getDatabaseMajorVersion() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getDatabaseMajorVersion() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getDatabaseMajorVersion();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getDatabaseMinorVersion() のテスト
	public void test_getDatabaseMinorVersion() throws Exception
	{
		Connection	c = getConnection();

		// 現状では getDatabaseMinorVersion() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().getDatabaseMinorVersion();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.getJDBCMajorVersion() のテスト
	public void test_getJDBCMajorVersion() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(CURRENT_JDBC_MAJOR_VERSION, c.getMetaData().getJDBCMajorVersion());

		c.close();
	}

	// DatabaseMetaData.getJDBCMinorVersion() のテスト
	public void test_getJDBCMinorVersion() throws Exception
	{
		Connection	c = getConnection();

		assertEquals(CURRENT_JDBC_MINOR_VERSION, c.getMetaData().getJDBCMinorVersion());

		c.close();
	}

	// DatabaseMetaData.getSQLStateType() のテスト
	public void test_getSQLStateType() throws Exception
	{
		Connection	c = getConnection();

		// SQLException.getSQLState() は SQL99 を返すはず
		assertEquals(DatabaseMetaData.sqlStateSQL99, c.getMetaData().getSQLStateType());

		c.close();
	}

	// DatabaseMetaData.locatorsUpdateCopy() のテスト
	public void test_locatorsUpdateCopy() throws Exception
	{
		Connection	c = getConnection();

		// 現状では locatorsUpdateCopy() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData().locatorsUpdateCopy();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// DatabaseMetaData.supportsStatementPooling() のテスト
	public void test_supportsStatementPooling() throws Exception
	{
		Connection	c = getConnection();

		// 現状では supportsStatementPooling() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			boolean result = c.getMetaData().supportsStatementPooling();
			assertEquals(false, result);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		// assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// データベースを準備する
	public void setUp() throws Exception
	{
		super.setUp();
	}

	// データベースを削除する
	public void tearDown() throws Exception
	{
		super.tearDown();
	}

	// DatabaseMetaData.getTables() の結果をチェック
	private void assertTableResults(	ResultSet	rs,
										String[]	tables) throws Exception
	{
		for (int i = 0; i < tables.length; i++) {

			assertTrue(rs.next());
			assertNull(rs.getString("TABLE_CAT"));					assertTrue(rs.wasNull());
			assertNull(rs.getString("TABLE_SCHEM"));				assertTrue(rs.wasNull());
			assertEquals(tables[i], rs.getString("TABLE_NAME"));	assertFalse(rs.wasNull());
			assertEquals("TABLE", rs.getString("TABLE_TYPE"));		assertFalse(rs.wasNull());
			assertEquals("", rs.getString("REMARKS"));				assertFalse(rs.wasNull());
			assertNull(rs.getString("TYPE_CAT"));					assertTrue(rs.wasNull());
			assertNull(rs.getString("TYPE_SCHEM"));					assertTrue(rs.wasNull());
			assertNull(rs.getString("TYPE_NAME"));					assertTrue(rs.wasNull());
			assertNull(rs.getString("SELF_REFERENCING_COL_NAME"));	assertTrue(rs.wasNull());
			assertNull(rs.getString("REF_GENERATION"));				assertTrue(rs.wasNull());
		}
		assertFalse(rs.next());
	}

	// DatabaseMetaData.getColumns() の結果をチェック
	private void assertColumnResults(	ResultSet		rs,
										ColumnInfo[]	columnInfos) throws Exception
	{
		for (int i = 0; i < columnInfos.length; i++) {
			assertTrue(rs.next());
			assertNull(rs.getString("TABLE_CAT"));														assertTrue(rs.wasNull());
			assertNull(rs.getString("TABLE_SCHEM"));													assertTrue(rs.wasNull());
			assertEquals(columnInfos[i].tableName, rs.getString("TABLE_NAME"));							assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].columnName, rs.getString("COLUMN_NAME"));						assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].dataType, rs.getInt("DATA_TYPE"));								assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].typeName, rs.getString("TYPE_NAME"));							assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].columnSize, rs.getInt("COLUMN_SIZE"));							assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].decimalDigits, rs.getInt("DECIMAL_DIGITS"));					assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].numPrecRadix, rs.getInt("NUM_PREC_RADIX"));						assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].nullable, rs.getInt("NULLABLE"));								assertFalse(rs.wasNull());
				if (columnInfos[i].remarks == null) {
					assertNull(rs.getString("REMARKS"));												assertTrue(rs.wasNull());
				} else {
					assertEquals(columnInfos[i].remarks, rs.getString("REMARKS"));						assertFalse(rs.wasNull());
				}
				if (columnInfos[i].columnDefault == null) {
					assertNull(rs.getString("COLUMN_DEF"));												assertTrue(rs.wasNull());
				} else {
					assertEquals(columnInfos[i].columnDefault, rs.getString("COLUMN_DEF").trim());		assertFalse(rs.wasNull());
				}
				if (columnInfos[i].checkBytes == ColumnInfo.CHECK_BYTES) {
					assertEquals(columnInfos[i].columnBytes, rs.getInt("CHAR_OCTET_LENGTH"));			assertFalse(rs.wasNull());
				} else {
					assertNull(rs.getObject("CHAR_OCTET_LENGTH"));										assertTrue(rs.wasNull());
				}
				assertEquals(columnInfos[i].isNullable, rs.getString("IS_NULLABLE"));					assertFalse(rs.wasNull());
			assertEquals(columnInfos[i].index, rs.getInt("ORDINAL_POSITION"));							assertFalse(rs.wasNull());
			assertNull(rs.getString("SCOPE_CATLOG"));													assertTrue(rs.wasNull());
			assertNull(rs.getString("SCOPE_SCHEMA"));													assertTrue(rs.wasNull());
			assertNull(rs.getString("SCOPE_TABLE"));													assertTrue(rs.wasNull());
			assertNull(rs.getObject("SOURCE_DATA_TYPE"));												assertTrue(rs.wasNull());
		}
		assertFalse(rs.next());
	}

	// DatabaseMetaData.getBestRowIdentifier() の結果をチェック
	private void assertBestRowID(ResultSet	rs) throws Exception
	{
		assertTrue(rs.next());
		assertEquals(DatabaseMetaData.bestRowSession, rs.getShort("SCOPE"));		assertFalse(rs.wasNull());
		assertEquals("ROWID", rs.getString("COLUMN_NAME"));							assertFalse(rs.wasNull());
		assertEquals(Types.INTEGER, rs.getShort("DATA_TYPE"));						assertFalse(rs.wasNull());
		assertEquals("int", rs.getString("TYPE_NAME"));								assertFalse(rs.wasNull());
		assertEquals(4, rs.getInt("COLUMN_SIZE"));								assertFalse(rs.wasNull());
		assertEquals(0, rs.getShort("DECIMAL_DIGITS"));							assertFalse(rs.wasNull());
		assertEquals(4, rs.getInt("COLUMN_SIZE"));								assertFalse(rs.wasNull());
		assertEquals(0, rs.getShort("DECIMAL_DIGITS"));							assertFalse(rs.wasNull());
		assertEquals(DatabaseMetaData.bestRowPseudo, rs.getShort("PSEUDO_COLUMN"));	assertFalse(rs.wasNull());
		assertFalse(rs.next());
	}

	// DatabaseMetaData.getPrimaryKeys() の結果をチェック
	private void assertPrimaryKeys(	ResultSet	rs,
									String		tableName,
									String		columnName,
									short		sequentialNumber,
									String		keyName) throws Exception
	{
		assertTrue(rs.next());
		assertNull(rs.getString("TABLE_CAT"));					assertTrue(rs.wasNull());
		assertNull(rs.getString("TABLE_SCHEM"));				assertTrue(rs.wasNull());
		assertEquals(tableName, rs.getString("TABLE_NAME"));	assertFalse(rs.wasNull());
		assertEquals(columnName, rs.getString("COLUMN_NAME"));	assertFalse(rs.wasNull());
		assertEquals((short)0, rs.getShort("KEY_SEQ"));			assertFalse(rs.wasNull());
		String	primaryKeyName = tableName + "_$$PrimaryKey";
		assertEquals(primaryKeyName, rs.getString("PK_NAME"));	assertFalse(rs.wasNull());
		assertFalse(rs.next());
	}

	// DatabaseMetaData.getTypeInfo() の結果をチェック
	private void assertTypeInfo(ResultSet	rs) throws Exception
	{
		short	predNone	= DatabaseMetaData.typePredNone;
		short	predChar	= DatabaseMetaData.typePredChar;
		short	predBasic	= DatabaseMetaData.typePredBasic;
		short	searchable	= DatabaseMetaData.typeSearchable;
		String[]	typeNames		= {	"BINARY",		"CHAR",			"NCHAR",		"INT",			"BIGINT",		"DECIMAL",		"FLOAT",		"VARCHAR",		"DATETIME"	};
		short[]		dataTypes		= {	Types.BINARY,	Types.CHAR,		Types.CHAR,		Types.INTEGER,	Types.BIGINT,	Types.DECIMAL,	Types.DOUBLE,	Types.VARCHAR,	Types.TIME	};
		short[]		searchables		= {	predNone,		searchable,		searchable,		predBasic,		predBasic,		predBasic,		predBasic,		searchable,		predBasic	};
		boolean[]	fixedPrecScales	= {	false,			false,			false,			true,			true,			false,			false,			false,			false		};

		for (int i = 0; i < typeNames.length; i++) {

			assertTrue(rs.next());
			assertEquals(typeNames[i], rs.getString("TYPE_NAME"));							assertFalse(rs.wasNull());
			assertEquals(dataTypes[i], rs.getShort("DATA_TYPE"));							assertFalse(rs.wasNull());
			assertEquals(0, rs.getInt("PRECISION"));										assertFalse(rs.wasNull());
			assertNull(rs.getString("LITERAL_PREFIX"));										assertTrue(rs.wasNull());
			assertNull(rs.getString("LITERAL_SUFFIX"));										assertTrue(rs.wasNull());
			assertNull(rs.getString("CREATE_PARAMS"));										assertTrue(rs.wasNull());
			assertEquals(DatabaseMetaData.typeNullableUnknown, rs.getShort("NULLABLE"));	assertFalse(rs.wasNull());
			assertFalse(rs.getBoolean("CASE_SENSITIVE"));									assertFalse(rs.wasNull());
			assertEquals(searchables[i], rs.getShort("SEARCHABLE"));						assertFalse(rs.wasNull());
			assertFalse(rs.getBoolean("UNSIGNED_ATTRIBUTE"));								assertFalse(rs.wasNull());
			assertEquals(fixedPrecScales[i], rs.getBoolean("FIXED_PREC_SCALE"));			assertFalse(rs.wasNull());
			assertFalse(rs.getBoolean("AUTO_INCREMENT"));									assertFalse(rs.wasNull());
		}
		assertFalse(rs.next());
	}

	// DatabaseMetaData.getIndexInfo() の結果をチェック
	private void assertIndexInfo(	ResultSet	rs,
									String		tableName,
									boolean[]	nonUniques,
									String[]	indexNames,
									short[]		ordinalPositions,
									String[]	columnNames)	throws Exception
	{
		for (int i = 0; i < indexNames.length; i++) {

			assertTrue(rs.next());
			assertNull(rs.getString("TABLE_CAT"));									assertTrue(rs.wasNull());
			assertNull(rs.getString("TABLE_SCHEM"));								assertTrue(rs.wasNull());
			assertEquals(tableName, rs.getString("TABLE_NAME"));					assertFalse(rs.wasNull());
			assertEquals(nonUniques[i], rs.getBoolean("NON_UNIQUE"));				assertFalse(rs.wasNull());
			assertNull(rs.getString("INDEX_QUALIFIER"));							assertTrue(rs.wasNull());
			assertEquals(indexNames[i], rs.getString("INDEX_NAME"));				assertFalse(rs.wasNull());
			assertEquals(DatabaseMetaData.tableIndexOther, rs.getShort("TYPE"));	assertFalse(rs.wasNull());
			assertEquals(ordinalPositions[i], rs.getShort("ORDINAL_POSITION"));		assertFalse(rs.wasNull());
			assertEquals(columnNames[i], rs.getString("COLUMN_NAME"));				assertFalse(rs.wasNull());
			assertNull(rs.getString("ASC_OR_DESC"));								assertTrue(rs.wasNull());
			rs.getInt("CARDINALITY");												assertFalse(rs.wasNull());
			rs.getInt("PAGES");														assertTrue(rs.wasNull());
			assertNull(rs.getString("FILTER_CONDITION"));							assertTrue(rs.wasNull());
		}
	}
}

//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
