// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PreparedStatementTest.java -- jp.co.ricoh.doquedb.jdbc.PreparedStatement クラスのテスト
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
import junit.framework.*;

import java.sql.*;
import java.io.InputStream;
import java.io.Reader;
import java.math.BigDecimal;

import jp.co.ricoh.doquedb.exception.*;
import jp.co.ricoh.doquedb.common.*;

class DummyRef implements Ref
{
	public DummyRef() { ; }
	public String getBaseTypeName() throws SQLException { return null; }
	public Object getObject() throws SQLException { return null; }
	public Object getObject(java.util.Map map) throws SQLException { return null; }
	public void setObject(Object value) throws SQLException { ; }
}

class DummyBlob implements Blob
{
	public DummyBlob() { ; }
	public java.io.InputStream getBinaryStream() { return null; }
	public byte[] getBytes(long pos, int length) { return null; }
	public long length() { return 0; }
	public long position(Blob pattern, long start) { return 0; }
	public long position(byte[] pattern, long start) { return 0; }
	public java.io.OutputStream setBinaryStream(long pos) { return null; }
	public int setBytes(long pos, byte[] bytes) { return 0; }
	public int setBytes(long pos, byte[] bytes, int offset, int len) { return 0; }
	public void truncate(long len) { ; }
	@Override
	public void free() throws SQLException {
		// TODO 自動生成されたメソッド・スタブ

	}
	@Override
	public InputStream getBinaryStream(long pos, long length)
			throws SQLException {
		// TODO 自動生成されたメソッド・スタブ
		return null;
	}
}

class DummyClob implements Clob
{
	public DummyClob() { ; }
	public java.io.InputStream getAsciiStream() { return null; }
	public java.io.Reader getCharacterStream() { return null; }
	public String getSubString(long pos, int length) { return ""; }
	public long length() { return 0; }
	public long position(Clob searchstr, long start) { return 0; }
	public long position(String searchstr, long start) { return 0; }
	public java.io.OutputStream setAsciiStream(long pos) { return null; }
	public java.io.Writer setCharacterStream(long pos) { return null; }
	public int setString(long pos, String str) { return 0; }
	public int setString(long pos, String str, int offset, int len) { return 0; }
	public void truncate(long len) { ; }
	@Override
	public void free() throws SQLException {
		// TODO 自動生成されたメソッド・スタブ

	}
	@Override
	public Reader getCharacterStream(long pos, long length) throws SQLException {
		// TODO 自動生成されたメソッド・スタブ
		return null;
	}
}

class TestArray implements Array
{
	private Object[]	array;

	public TestArray(Object[] src) { array = src; }
	public Object getArray() throws SQLException { return array; }
	public Object getArray(long index, int count) throws SQLException { throw new NotSupported(); }
	public Object getArray(long index, int count, java.util.Map map) throws SQLException { throw new NotSupported(); }
	public Object getArray(java.util.Map map) throws SQLException { throw new NotSupported(); }
	public int getBaseType() throws SQLException { throw new NotSupported(); }
	public String getBaseTypeName() throws SQLException { throw new NotSupported(); }
	public ResultSet getResultSet() throws SQLException { throw new NotSupported(); }
	public ResultSet getResultSet(long index, int count) throws SQLException { throw new NotSupported(); }
	public ResultSet getResultSet(long index, int count, java.util.Map map) throws SQLException { throw new NotSupported(); }
	public ResultSet getResultSet(java.util.Map map) throws SQLException { throw new NotSupported(); }
	@Override
	public void free() throws SQLException {
		// TODO 自動生成されたメソッド・スタブ

	}
}

class SetObjectInfo
{
	private int				sqlType;
	private Object			object;
	private SQLException	sqlException;
	private int				whenCatchException;

	public final static int	AT_SET_OBJECT = 1;
	public final static int	AT_EXECUTE_UPDATE = AT_SET_OBJECT + 1;

	public SetObjectInfo(int	sqlType_)
	{
		sqlType = sqlType_;
		object = null;
		sqlException = null;
		whenCatchException = 0;
	}

	public SetObjectInfo(	int		sqlType_,
							Object	object_)
	{
		sqlType = sqlType_;
		object = object_;
		sqlException = null;
		whenCatchException = 0;
	}

	public SetObjectInfo(	int				sqlType_,
							Object			object_,
							SQLException	sqlException_)
	{
		sqlType = sqlType_;
		object = object_;
		sqlException = sqlException_;
		java.sql.SQLException	notSupported = new NotSupported();
		if (notSupported.getSQLState() == sqlException_.getSQLState()) {
			whenCatchException = AT_SET_OBJECT;
		} else {
			whenCatchException = AT_EXECUTE_UPDATE;
		}
	}

	public int getSQLType()
	{
		return sqlType;
	}

	public Object getObject()
	{
		return object;
	}

	public SQLException getSQLException()
	{
		return sqlException;
	}

	public String getSQLState()
	{
		if (sqlException == null)	return "";
		else						return sqlException.getSQLState();
	}

	public int getExceptionAt()
	{
		return whenCatchException;
	}
}

class SetObjectColumnInfo
{
	private String			columnName;
	private SetObjectInfo[]	objectInfos;

	public SetObjectColumnInfo(	String			columnName_,
								SetObjectInfo[]	objectInfos_)
	{
		try {
			columnName = columnName_;

			java.sql.SQLException	notSupported = new NotSupported();
			SetObjectInfo[]	notSupportedObjectInfos = {
				new SetObjectInfo(java.sql.Types.BOOLEAN,		new Boolean(true),									notSupported),
				new SetObjectInfo(java.sql.Types.TIME,			java.sql.Time.valueOf("19:22:03"),					notSupported),
				new SetObjectInfo(java.sql.Types.BLOB,			new DummyBlob(),									notSupported),
				new SetObjectInfo(java.sql.Types.CLOB,			new DummyClob(),									notSupported),
				new SetObjectInfo(java.sql.Types.REF,			new DummyRef(),										notSupported),
				new SetObjectInfo(java.sql.Types.DATALINK,		new java.net.URL("http://doquedb.src.ricoh.co.jp/"),	notSupported),
				new SetObjectInfo(java.sql.Types.JAVA_OBJECT,	new java.beans.Beans(),								notSupported),
				new SetObjectInfo(java.sql.Types.JAVA_OBJECT,	new java.applet.Applet(),							notSupported)
			};

			int	idx = 0;
			int	totalElements = objectInfos_.length + notSupportedObjectInfos.length;
			objectInfos = new SetObjectInfo[totalElements];
			for (int i = 0; i < objectInfos_.length; i++) objectInfos[idx++] = objectInfos_[i];
			for (int i = 0; i < notSupportedObjectInfos.length; i++) objectInfos[idx++] = notSupportedObjectInfos[i];

		} catch (Exception	e) {
			;
		}
	}

	public String getColumnName()
	{
		return columnName;
	}

	public SetObjectInfo[] getObjectInfos()
	{
		return objectInfos;
	}
}

public class PreparedStatementTest extends TestBase
{
	// 警告メッセージ WM_*****

	private final static String	WM_DIRECTION =
		"direction of ResultSet currently supported is only java.sql.ResultSet.FETCH_FORWARD.";

	private final static String	WM_FETCH_SIZE =
		"hint about the number of rows which needs to be taken out from a database is not supported other than zero.";

	private final static String	WM_CURSOR_NOT_SUPPORT =
		"cursor is not supporting.";

	public PreparedStatementTest(String	nickname)
	{
		super(nickname);
	}

	// PreparedStatement.executeQuery() のテスト
	public void test_executeQuery1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		assertSetInt(c, "f_int2", 6300);
		assertSetString(c, "f_char8", "abcd");

		// これでふたつ insert されたはず

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select f_int2 from t order by f_int1"));
		ResultSet	rs = null;
		assertNotNull(rs = ps.executeQuery());
		// ひとつめは 6300
		assertTrue(rs.next());
		assertEquals(6300, rs.getInt(1));
		assertFalse(rs.wasNull());
		// ふたつめは null
		assertTrue(rs.next());
		rs.getInt(1);
		assertTrue(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		ps.close();

		// f_int2 が 6300 なのはひとつ
		assertNotNull(ps = c.prepareStatement("select count(*) from t where f_int2 = ?"));
		ps.setInt(1, 6300);
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertEquals(1, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		// f_int2 が 6400 なのはひとつもない
		ps.setInt(1, 6400);
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertEquals(0, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		ps.close();

		// f_char8 が "abcd    " なのはひとつ
		assertNotNull(ps = c.prepareStatement("select count(*) from t where f_char8 = ?"));
		ps.setString(1, "abcd    "); // v14.0 では末尾に空白文字が追加されない
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertEquals(1, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		// v15.0 だと "abcd" でもヒットするはず
		ps.setString(1, "abcd");
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertEquals(1, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		// f_char8 が "xyz     " なのはひとつもない
		ps.setString(1, "xyz     "); // v14.0 では末尾に空白文字が追加されない
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertEquals(0, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.executeUpdate() のテスト
	public void test_executeUpdate1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 自動コミットモード
		//

		String	query = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_id) values (?, ?, ?, ?)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		// 102 1 "hogehoge" "61C35A0C-3006-493C-8072-C8AED78B2F75"
		ps.setInt(1, 102);											// f_int_not_null
		ps.setInt(2, 1);											// f_int1 (primary key)
		ps.setString(3, "hogehoge");								// f_char8_not_null
		ps.setString(4, "61C35A0C-3006-493C-8072-C8AED78B2F75");	// f_id
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		assertEquals(expected, ps.executeUpdate());

		// 102 2 "abc     " "232B88B6-3EB5-49BB-8A5F-FF28735C7D20"
		ps.setInt(1, 102);											// f_int_not_null
		ps.setInt(2, 2);											// f_int1 (primary key)
		ps.setString(3, "abc");										// f_char8_not_null
		ps.setString(4, "232B88B6-3EB5-49BB-8A5F-FF28735C7D20");	// f_id
		assertEquals(expected, ps.executeUpdate());

		// 105 3 "aaabbb  " "A6AED6F2-868C-4C4A-A4B1-C54D5CE65A64"
		ps.setInt(1, 103);											// f_int_not_null
		ps.setInt(2, 3);											// f_int1 (primary key)
		ps.setString(3, "aaabbb");									// f_char8_not_null
		ps.setString(4, "A6AED6F2-868C-4C4A-A4B1-C54D5CE65A64");	// f_id
		assertEquals(expected, ps.executeUpdate());

		// 106 4 "update  " "07B004CD-60A5-496D-AEEB-41B3C1E8BEFD"
		ps.setInt(1, 106);											// f_int_not_null
		ps.setInt(2, 4);											// f_int1 (primary key)
		ps.setString(3, "update");									// f_char8_not_null
		ps.setString(4, "07B004CD-60A5-496D-AEEB-41B3C1E8BEFD");	// f_id
		assertEquals(expected, ps.executeUpdate());

		ps.close();

		// f_int_not_null が 102 なのはふたつ
		query = "update t set f_id = '00000000-0000-0000-0000-000000000000' where f_int_not_null = ?";
		assertNotNull(ps = c.prepareStatement(query));
		ps.setInt(1, 102);
		expected = 2;
		assertEquals(expected, ps.executeUpdate());

		// f_int_not_null が 106 なのはひとつ
		ps.setInt(1, 106);
		expected = 1;
		assertEquals(expected, ps.executeUpdate());

		// f_int_not_null が 200 なのはひとつもない
		ps.setInt(1, 200);
		assertZero(ps.executeUpdate());

		ps.close();

		// 一旦全部（よっつ）消しましょう
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		expected = 4;
		assertEquals(expected, s.executeUpdate("delete from t"));
		s.close();

		//
		// 手動コミットモード
		//

		c.setAutoCommit(false);

		query = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_nchar6) values (?, ?, ?, ?)";
		assertNotNull(ps = c.prepareStatement(query));

		// 102, 1, "prepare " "abc   "
		ps.setInt(1, 102);			// f_int_not_null
		ps.setInt(2, 1);			// f_int1 (primary key)
		ps.setString(3, "prepare");	// f_char8_not_null
		ps.setString(4, "abc");		// f_nchar6
		expected = 1;
		assertEquals(expected, ps.executeUpdate());

		// 105, 2, "execute " "efgh  "
		ps.setInt(1, 105);			// f_int_not_null
		ps.setInt(2, 2);			// f_int1 (primary key)
		ps.setString(3, "execute");	// f_char8_not_null
		ps.setString(4, "efgh");	// f_nchar6
		assertEquals(expected, ps.executeUpdate());

		c.rollback();

		// ロールバックしたんだからなんにもないはず
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertZero(rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		// 104, 1, "test    ", "opqr  "
		ps.setInt(1, 104);			// f_int_not_null
		ps.setInt(2, 1);			// f_int1 (primary key)
		ps.setString(3, "test");	// f_char8_not_null
		ps.setString(4, "opqr");	// f_nchar6
		assertEquals(expected, ps.executeUpdate());

		// 104, 2, "insert  ", "check "
		ps.setInt(1, 104);			// f_int_not_null
		ps.setInt(2, 2);			// f_int1 (primary key)
		ps.setString(3, "insert");	// f_char8_not_null
		ps.setString(4, "check");	// f_nchar6
		assertEquals(expected, ps.executeUpdate());

		ps.close();

		// f_int_not_null が 104 なのはふたつ
		query = "update t set f_nchar6 = 'change' where f_int_not_null = ?";
		assertNotNull(ps = c.prepareStatement(query));
		ps.setInt(1, 104);
		expected = 2;
		assertEquals(expected, ps.executeUpdate());

		// f_int_not_null が 106 なのはひとつもない
		ps.setInt(1, 106);
		assertZero(ps.executeUpdate());

		ps.close();

		c.setAutoCommit(true);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setNull(int, int) と PreparedStatement.setNull(int, int, String) のテスト
	public void test_setNull() throws Exception
	{
		//
		// ※ 現状では、第２引数と第３引数は常に無視される
		//

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	_3rdPrm = "第３引数に変な文字列を渡しても現状では無視されるはず";

		int	f_int1 = 5;

		for (int numPrm = 2; numPrm < 4; numPrm++, f_int1++) {

			String	query = null;
			PreparedStatement	ps = null;
			{
				query = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_int2) values (3, " + f_int1 + ", 'hoge1234', ?)";
				assertNotNull(ps = c.prepareStatement(query));
				if (numPrm == 2)	ps.setNull(1, Types.INTEGER);
				else				ps.setNull(1, Types.INTEGER, _3rdPrm);
				ps.executeUpdate();
				ps.close();

				Statement	s = null;
				assertNotNull(s = c.createStatement());
				ResultSet	rs = null;
				assertNotNull(rs = s.executeQuery("select f_int2 from t"));
				assertTrue(rs.next());
				assertZero(rs.getInt(1));
				assertTrue(rs.wasNull());
				if (numPrm == 3) {
					assertTrue(rs.next());
					assertZero(rs.getInt(1));
					assertTrue(rs.wasNull());
				}
				assertFalse(rs.next());
				rs.close();
				s.close();
			}

			//
			// NOT NULL の列に設定すると v14.0 では例外 IntegrityViolation が、v15.0 以降では例外 NullabilityViolation が throw されるはず
			// ※ PRIMARY KEY も NOT NULL 制約が付く
			//

			java.sql.SQLException	nullabilityViolation = new NullabilityViolation("");
			java.sql.SQLException	integrityViolation = new IntegrityViolation();

			//
			// まずは insert
			//

			// f_int_not_null

			query = "insert into t (f_int_not_null, f_int1, f_char8_not_null) values (?, 1, '00112233')";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.INTEGER);
			else				ps.setNull(1, Types.INTEGER, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();

			// f_int1

			query = "insert into t (f_int_not_null, f_int1, f_char8_not_null) values (2, ?, 'abcdefgh')";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.INTEGER);
			else				ps.setNull(1, Types.INTEGER, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();

			// f_char8_not_null

			query = "insert into t (f_int_not_null, f_int1, f_char8_not_null) values (4, 2, ?)";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.CHAR);
			else				ps.setNull(1, Types.CHAR, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();

			//
			// update も
			//

			// f_int_not_null

			query = "update t set f_int_not_null = ? where f_int1 = 5";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.INTEGER);
			else				ps.setNull(1, Types.INTEGER, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();

			// f_int1

			query = "update t set f_int1 = ? where f_int_not_null = 3";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.CHAR);
			else				ps.setNull(1, Types.CHAR, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();

			// f_char8_not_null

			query = "update t set f_char8_not_null = ? where f_int1 = 5";
			assertNotNull(ps = c.prepareStatement(query));
			if (numPrm == 2)	ps.setNull(1, Types.CHAR);
			else				ps.setNull(1, Types.CHAR, _3rdPrm);
			assertExecuteUpdateCatchException(ps, nullabilityViolation);
			ps.close();
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setByte() のテスト
	public void test_setByte() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setByte() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetByte(c, columnName, (byte)7);
		assertGetInt(c, columnName, 7);

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		assertSetByte(c, columnName, (byte)2);
		assertGetLong(c, columnName, 2L);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		assertSetByte(c, columnName, (byte)3);
		assertGetLong(c, columnName, 3L);

		// CHAR
		columnName = "f_char8";
		assertSetByte(c, columnName, (byte)8);
		assertGetString(c, columnName, "8       "); // v14.0 では末尾に空白文字が追加されない

		// FLOAT
		columnName = "f_float";
		assertSetByte(c, columnName, (byte)4);
		assertGetFloat(c, columnName, (float)4);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetByte(c, columnName, (byte)5);
		assertGetString(c, columnName, "5                                   "); // v14.0 では末尾に空白文字が追加されない

		// NCHAR
		columnName = "f_nchar6";
		assertSetByte(c, columnName, (byte)9);
		assertGetString(c, columnName, "9     "); // v14.0 では末尾に空白文字が追加されない

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetByte(c, columnName, (byte)6);
		assertGetString(c, columnName, "6");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetByte(c, columnName, (byte)4);
		assertGetString(c, columnName, "4");

		// NTEXT
		columnName = "f_ntext";
		assertSetByte(c, columnName, (byte)2);
		assertGetString(c, columnName, "2");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetByte(c, columnName, (byte)5);
		assertGetString(c, columnName, "5");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetByte(c, columnName, (byte)4);
		assertGetString(c, columnName, "4");

		// NCLOB
		/* 動かない is not nullが効かない
		columnName = "f_nclob";
		assertSetByte(c, columnName, (byte)9);
		assertGetString(c, columnName, "9");
		*/

		//
		// 以下、setByte() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetByte(c, columnNames[i], (byte)0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetByte(c, "af_bigint", (byte)0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetByte(c, "af_decimal", (byte)0, classCast);	// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setShort() のテスト
	public void test_setShort() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setShort() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetShort(c, columnName, (short)36);
		assertGetInt(c, columnName, 36);

		// bigint 列は v15.0 からサポート
		// BINGINT
		columnName = "f_bigint";
		assertSetShort(c, columnName, (short)89);
		assertGetLong(c, columnName, 89L);

		// decimal 列は v16.1 からサポート
		// BINGINT
		columnName = "f_decimal";
		assertSetShort(c, columnName, (short)68);
		assertGetLong(c, columnName, 68L);

		// CHAR
		columnName = "f_char8";
		assertSetShort(c, columnName, (short)62);
		assertGetString(c, columnName, "62      "); // v14.0 では末尾に空白文字が追加されない

		// FLOAT
		columnName = "f_float";
		assertSetShort(c, columnName, (short)30);
		assertGetFloat(c, columnName, (float)30);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetShort(c, columnName, (short)960);
		assertGetString(c, columnName, "960                                 ");// v14.0 では末尾に空白文字が追加されない

		// NCHAR
		columnName = "f_nchar6";
		assertSetShort(c, columnName, (short)54);
		assertGetString(c, columnName, "54    ");// v14.0 では末尾に空白文字が追加されない

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetShort(c, columnName, (short)121);
		assertGetString(c, columnName, "121");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetShort(c, columnName, (short)14);
		assertGetString(c, columnName, "14");

		// NTEXT
		columnName = "f_ntext";
		assertSetShort(c, columnName, (short)80);
		assertGetString(c, columnName, "80");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetShort(c, columnName, (short)59);
		assertGetString(c, columnName, "59");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetShort(c, columnName, (short)77);
		assertGetString(c, columnName, "77");

		// NCLOB
		/* 動かない
		columnName = "f_nclob";
		assertSetShort(c, columnName, (short)53);
		assertGetString(c, columnName, "53");
		 */

		//
		// 以下、setShort() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetShort(c, columnNames[i], (short)0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetShort(c, "af_bigint", (short)0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetShort(c, "af_decimal", (short)0, classCast);// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setInt() のテスト
	public void test_setInt() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setInt() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetInt(c, columnName, 3080);
		assertGetInt(c, columnName, 3080);

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		assertSetInt(c, columnName, 25824);
		assertGetLong(c, columnName, 25824L);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		assertSetInt(c, columnName, 65987);
		assertGetLong(c, columnName, 65987L);

		// CHAR
		columnName = "f_char8";
		assertSetInt(c, columnName, 1081);
		assertGetString(c, columnName, "1081    ");// v14.0 では末尾に空白文字が追加されない

		// FLOAT
		columnName = "f_float";
		assertSetInt(c, columnName, 590);
		assertGetFloat(c, columnName, (float)590);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetInt(c, columnName, 1415);
		assertGetString(c, columnName, "1415                                ");// v14.0 では末尾に空白文字が追加されない

		// NCHAR
		columnName = "f_nchar6";
		assertSetInt(c, columnName, 316);
		assertGetString(c, columnName, "316   ");// v14.0 では末尾に空白文字が追加されない

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetInt(c, columnName, 18);
		assertGetString(c, columnName, "18");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetInt(c, columnName, 2178);
		assertGetString(c, columnName, "2178");

		// NTEXT
		columnName = "f_ntext";
		assertSetInt(c, columnName, 518);
		assertGetString(c, columnName, "518");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetInt(c, columnName, 3948);
		assertGetString(c, columnName, "3948");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetInt(c, columnName, 63);
		assertGetString(c, columnName, "63");

		// NCLOB
		/* 動かない
		columnName = "f_nclob";
		assertSetInt(c, columnName, 728);
		assertGetString(c, columnName, "728");
		*/

		//
		// 以下、setInt() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50",			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetInt(c, columnNames[i], 0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetInt(c, "af_bigint", 0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetInt(c, "af_decimal", 0, classCast);	// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setInt() のテスト
	public void test_setInt2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// int に設定可能な最小値／最大値のテスト
		//

		// 最小値
		String	columnName = "f_int2";
		assertSetInt(c, columnName, Integer.MIN_VALUE);
		assertGetInt(c, columnName, Integer.MIN_VALUE);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int	expected = 1;
		assertEquals(expected, s.executeUpdate("delete from t"));
		s.close();

		// 最大値
		assertSetInt(c, columnName, Integer.MAX_VALUE);
		assertGetInt(c, columnName, Integer.MAX_VALUE);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setLong() のテスト
	public void test_setLong() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setLong() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetLong(c, columnName, 608389);
		assertGetInt(c, columnName, 608389);

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		assertSetLong(c, columnName, 258362938788L);
		assertGetLong(c, columnName, 258362938788L);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		assertSetLong(c, columnName, 98543L);
		assertGetLong(c, columnName, 98543L);

		// CHAR
		columnName = "f_char8";
		assertSetLong(c, columnName, 74637826);
		assertGetString(c, columnName, "74637826");

		// FLOAT
		columnName = "f_float";
		assertSetLong(c, columnName, 43782674);
		assertGetFloat(c, columnName, (float)43782674);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetLong(c, columnName, 9874327);
		assertGetString(c, columnName, "9874327                             ");// v14.0 では末尾に空白文字が追加されない

		// NCHAR
		columnName = "f_nchar6";
		assertSetLong(c, columnName, 463926);
		assertGetString(c, columnName, "463926");

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetLong(c, columnName, 6598478);
		assertGetString(c, columnName, "6598478");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetLong(c, columnName, 4987465);
		assertGetString(c, columnName, "4987465");

		// NTEXT
		columnName = "f_ntext";
		assertSetLong(c, columnName, 9267635);
		assertGetString(c, columnName, "9267635");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetLong(c, columnName, 4982681);
		assertGetString(c, columnName, "4982681");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetLong(c, columnName, 1748657284);
		assertGetString(c, columnName, "1748657284");

		// NCLOB
		/* 動かない
		columnName = "f_nclob";
		assertSetLong(c, columnName, 490742187);
		assertGetString(c, columnName, "490742187");
		*/

		//
		// 以下、setLong() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetLong(c, columnNames[i], (long)0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetLong(c, "af_bigint", (long)0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetLong(c, "af_decimal", (long)0, classCast);	// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setLong() のテスト
	public void test_setLong2() throws Exception
	{
		// bigint 列は v15.0 からサポート
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// long に設定可能な最小値／最大値のテスト
		//

		// 最小値
		String	columnName = "f_bigint";
		assertSetLong(c, columnName, Long.MIN_VALUE);
		assertGetLong(c, columnName, Long.MIN_VALUE);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		assertEquals(1, s.executeUpdate("delete from t"));
		s.close();

		// 最大値
		assertSetLong(c, columnName, Long.MAX_VALUE);
		assertGetLong(c, columnName, Long.MAX_VALUE);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setFloat() のテスト
	public void test_setFloat() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setFloat() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetFloat(c, columnName, (float)4.5878267);
		assertGetInt(c, columnName, 4);

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		assertSetFloat(c, columnName, (float)95437.87);
		assertGetLong(c, columnName, 95437L);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		assertSetFloat(c, columnName, (float)8531.092);
		assertGetLong(c, columnName, 8531L);

		// CHAR
		columnName = "f_char8";
		assertSetFloat(c, columnName, (float)0.31);
		assertGetString(c, columnName, "3.1E-1  ");
		// ※ 文字列をはみ出すこともある。
		// 　 例えば、PreparedStatement.setFloat() に 5178.478 を渡すと、"5.178478E3" という文字列になり、
		// 　 列 f_char8 は 8 文字までという制限なので、v15.0 以降では例外 StringRightTruncation が throw される。
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		assertSetFloat(c, columnName, (float)5178.478, stringRightTruncation);

		// FLOAT
		columnName = "f_float";
		assertSetFloat(c, columnName, (float)478.59781);
		assertGetFloat(c, columnName, (float)478.59781);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetFloat(c, columnName, (float)1.578329);
		assertGetString(c, columnName, "1.578329E0                          ");

		// NCHAR
		columnName = "f_nchar6";
		assertSetFloat(c, columnName, (float)0.1);
		assertGetString(c, columnName, "1E-1  ");

		// ※ 文字列をはみ出すこともある（ v15.0 以降では例外 StringRightTruncation が throw される。）
		assertSetFloat(c, columnName, (float)0.02177, stringRightTruncation);

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetFloat(c, columnName, (float)5178.478);
		assertGetString(c, columnName, "5.178478E3");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetFloat(c, columnName, (float)3.78986);
		assertGetString(c, columnName, "3.78986E0");

		// NTEXT
		columnName = "f_ntext";
		assertSetFloat(c, columnName, (float)5874.8433);
		assertGetString(c, columnName,"5.8748433E3");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetFloat(c, columnName, (float)437297);
		assertGetString(c, columnName, "4.37297E5");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetFloat(c, columnName, (float)1.25);
		assertGetString(c, columnName, "1.25E0");

		// NCLOB
		/* 動かない
		columnName = "f_nclob";
		assertSetFloat(c, columnName, (float)68.498703);
		assertGetString(c, columnName, "6.8498703E1");
		*/

		//
		// 以下、setFloat() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetFloat(c, columnNames[i], (float)0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetFloat(c, "af_bigint", (float)0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetFloat(c, "af_decimal", (float)0, classCast);// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setDouble() のテスト
	public void test_setDouble() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 以下、setDouble() できるデータ型
		//

		// INT
		String	columnName = "f_int2";
		assertSetDouble(c, columnName, 19843.0873);
		assertGetInt(c, columnName, 19843);

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		assertSetDouble(c, columnName, 91872.98729);
		assertGetLong(c, columnName, 91872L);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		assertSetDouble(c, columnName, 46802.57924);
		assertGetLong(c, columnName, 46802L);

		// CHAR
		columnName = "f_char8";
		assertSetDouble(c, columnName, 13.873);
		assertGetString(c, columnName, "1.3873E1");

		// ※ setFloat() と同様に、文字列をはみ出すこともある。（ v15.0 以降では例外 StringRightTruncation が throw される。）
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		assertSetDouble(c, columnName, 54982.743879, stringRightTruncation);

		// FLOAT
		columnName = "f_float";
		assertSetDouble(c, columnName, 982364.8125);
		assertGetFloat(c, columnName, (float)982364.8125);

		// UNIQUEIDENTIFIER
		columnName = "f_id";
		assertSetDouble(c, columnName, 342987.5837);
		assertGetString(c, columnName, "3.429875837E5                       ");

		// NCHAR
		columnName = "f_nchar6";
		assertSetDouble(c, columnName, 2.89);
		assertGetString(c, columnName, "2.89E0");

		// ※ 文字列をはみ出すこともある（ v15.0 以降では例外 StringRightTruncation が throw される。）
		assertSetDouble(c, columnName, 52837.877, stringRightTruncation);

		// NVARCHAR
		columnName = "f_nvarchar256";
		assertSetDouble(c, columnName, 0.904387);
		assertGetString(c, columnName, "9.04387E-1");

		// VARCHAR
		columnName = "f_varchar128";
		assertSetDouble(c, columnName, 3297.8438);
		assertGetString(c, columnName, "3.2978438E3");

		// NTEXT
		columnName = "f_ntext";
		assertSetDouble(c, columnName, 0.87439);
		assertGetString(c, columnName, "8.7439E-1");

		// NTEXT(compressed)
		columnName = "f_ntext_compressed";
		assertSetDouble(c, columnName, 532987.877);
		assertGetString(c, columnName, "5.32987877E5");

		// FULLTEXT
		columnName = "f_fulltext";
		assertSetDouble(c, columnName, 6.18963878);
		assertGetString(c, columnName, "6.18963878E0");

		// NCLOB
		/* 動かない
		columnName = "f_nclob";
		assertSetDouble(c, columnName, 35.43987);
		assertGetString(c, columnName, "3.543987E1");
		*/

		//
		// 以下、setDouble() できないデータ型 → 例外 ClassCast が throw されるはず
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_datetime",			// DATETIME
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50",			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++)
			assertSetDouble(c, columnNames[i], (double)0, classCast);

		// bigint 列は v15.0 からサポート
		assertSetDouble(c, "af_bigint", (double)0, classCast);	// BIGINT 配列

		// decimal 列は v16.1 からサポート
		assertSetDouble(c, "af_decimal", (double)0, classCast);	// DECIMAL 配列

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setString() のテスト
	public void test_setString() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	columnName = null;
		java.util.Vector	anss = null;

		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	badArgument = new BadArgument();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();
		java.sql.SQLException	modLibraryError = new ModLibraryError(new ExceptionData());

		// INT
		{
			columnName = "f_int2";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetInt(c, columnName);

			// 空文字列 → 0 になるはず
			assertSetString(c, columnName, "");
			anss.add(new Integer(0));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列 → ちゃんと数字になるはず
			assertSetString(c, columnName, "365");
			anss.add(new Integer(365));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "0x48", invalidCharacter);

			// ちゃんと数字列、だけど float → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "3.0", invalidCharacter);

			// 数字以外も含む文字列 → ひとつでも数字以外の文字が含まれていれば、v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "165h4", invalidCharacter);
		}

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";
		anss = new java.util.Vector();

		// null → null になるはず
		assertSetString(c, columnName, null);
		assertGetLong(c, columnName);

		// 空文字列 → 0 になるはず
		assertSetString(c, columnName, "");
		anss.add(new Long(0L));
		assertGetLong(c, columnName, anss);

		// ちゃんと数字列 →ちゃんと数字になるはず
		assertSetString(c, columnName, "-66184329786");
		anss.add(new Long(-66184329786L));
		assertGetLong(c, columnName, anss);

		// ちゃんと数字列、だけど 16 進数 → 例外 InvalidCharacter が throw されるはず
		assertSetString(c, columnName, "0x4C8DFE657F2F6", invalidCharacter);

		// ちゃんと数字列、だけど float → 例外 InvalidCharacter が throw されるはず
		assertSetString(c, columnName, "19843287.87983", invalidCharacter);

		// 数字以外も含む文字列 → ひとつでも数字以外の文字が含まれていれば、例外 InvalidCharacter が throw されるはず
		assertSetString(c, columnName, "59826y809", invalidCharacter);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";
		anss = new java.util.Vector();

		// null → null になるはず
		assertSetString(c, columnName, null);
		assertGetLong(c, columnName);

		// 空文字列 → invalidcharacter
		assertSetString(c, columnName, "", invalidCharacter);

		// ちゃんと数字列 →ちゃんと数字になるはず
		assertSetString(c, columnName, "-88776.55443");
		anss.add(new BigDecimal("-88776.55443"));
		assertGetBigDecimal(c, columnName, anss);

		// ちゃんと数字列、だけど 16 進数 → 例外 InvalidCharacter が throw されるはず
		assertSetString(c, columnName, "0x4C8DFE657F2F6", invalidCharacter);

		// 数字以外も含む文字列 → ひとつでも数字以外の文字が含まれていれば、例外 InvalidCharacter が throw されるはず
		assertSetString(c, columnName, "59826y809", invalidCharacter);

		// CHAR
		{
			columnName = "f_char8";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_char8 は 8 文字）
			// ↑ではなく、空の文字列はfloatにキャストできないという仕様なので例外になるらしい
			//assertSetString(c, columnName, "");
			//anss.add("        ");
			//assertGetString(c, columnName, anss);

			// 短い文字列 → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetString(c, columnName, "abcdefg");
			anss.add("abcdefg ");
			assertGetString(c, columnName, anss);

			// 最大長超過 → v15.0 以降では超過分は切れているはず（キャスト不要なので例外 StringRightTruncation は throw されない）
			assertSetString(c, columnName, "123456789");
			anss.add("12345678");
			assertGetString(c, columnName, anss);
		}

		// FLOAT
		{
			columnName = "f_float";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetFloat(c, columnName);

			// 空文字列 → v16.1 以降では例外 InvalidCharacter が throw される、それ以外は 0 になるはず
			// ↑ではなく、空の文字列はfloatにキャストできないという仕様なので例外になるらしい
			assertSetString(c, columnName, "", invalidCharacter);

			// ちゃんと数字列（その１） → ちゃんと数字になるはず
			assertSetString(c, columnName, "37.5673");
			anss.add(new Float((float)37.5673));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列（その２） → ちゃんと数字になるはず
			assertSetString(c, columnName, "3387E2");
			anss.add(new Float((float)3387E2));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列（その３） → ちゃんと数字になるはず
			assertSetString(c, columnName, "587E+3");
			anss.add(new Float((float)587E+3));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列（その４） → ちゃんと数字になるはず
			assertSetString(c, columnName, "48652E-4");
			anss.add(new Float((float)48652E-4));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "0x5ABCD", invalidCharacter);

			// 数字以外も含む文字列 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "8743.3acag4", invalidCharacter);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetTimestamp(c, columnName);

			// 空文字列 → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetString(c, columnName, "", invalidDatetimeFormat);

			// ちゃんと暦を示す文字列 → ちゃんと暦になるはず
			assertSetString(c, columnName, "2004-12-01 15:48:08.832");
			assertGetTimestamp(c, columnName, "2004-12-01 15:48:08.832");

			// 暦になれない文字列 → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetString(c, columnName, "hogehoge", invalidDatetimeFormat);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → v15.0 以降では GUID を示す文字数分すべて空白文字になるはず
			assertSetString(c, columnName, "");
			anss.add("                                    ");
			assertGetString(c, columnName, anss);

			// ちゃんと GUID を示す文字列 → ちゃんと GUID になるはず
			assertSetString(c, columnName, "A36A0A80-C014-4005-999E-9042AD9A9C26");
			anss.add("A36A0A80-C014-4005-999E-9042AD9A9C26");
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（短い）→ v15.0 以降では末尾が空白文字で埋まるはず
			assertSetString(c, columnName, "hoge");
			anss.add("hoge                                ");
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（長い）→ v15.0 以降では超過分は切れているはず
			assertSetString(c, columnName, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
			anss.add("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
			assertGetString(c, columnName, anss);
		}

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空文字列 → 0 バイトのバイト列になるはず
			assertSetString(c, columnName, "");
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列 → 文字列をそのままバイト列にしたものになるはず
			assertSetString(c, columnName, "hogehoge");
			anss.add(stringToBytes("hogehoge"));
			assertGetBytes(c, columnName, anss);
		}

		// LANGUAGE
		{
			columnName = "f_language";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// ちゃんと言語を示す文字列 → ちゃんと言語になるはず
			assertSetString(c, columnName, "en+ja");
			anss.add("en+ja");
			assertGetString(c, columnName, anss);

			// 言語になれない文字列 → v14.0 では例外 ModLibraryError が、v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetString(c, columnName, "hogehoge", invalidCharacter);
		}

		// NCHAR
		{
			columnName = "f_nchar6";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_nchar6 は 6 文字）
			assertSetString(c, columnName, "");
			anss.add("      ");
			assertGetString(c, columnName, anss);

			// 短い文字列 → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetString(c, columnName, "foo");
			anss.add("foo   ");
			assertGetString(c, columnName, anss);

			// 最大長超過 → v15.0 以降では超過分は切れているはず（キャスト不要なので例外 StringRightTruncation は throw されない）
			assertSetString(c, columnName, "abcdefghijklmn");
			anss.add("abcdef");
			assertGetString(c, columnName, anss);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "jdbc driver");
			anss.add("jdbc driver");
			assertGetString(c, columnName, anss);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "prepared statement test");
			anss.add("prepared statement test");
			assertGetString(c, columnName, anss);
		}

		// NTEXT
		{
			columnName = "f_ntext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "PreparedStatementTest.java");
			anss.add("PreparedStatementTest.java");
			assertGetString(c, columnName, anss);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "Java プラットフォームでは、ロケールは、言語と地域の特定の組み合わせに対する単なる識別子です。ロケール固有の属性の集合ではありません。ロケール固有の情報は、ロケールの影響を受けるクラスがそれぞれ独自に保持しています。このような設計のため、ロケール固有のリソースを保持する方法に関し、ユーザオブジェクトとシステムオブジェクトで違いはありません。");
			anss.add("Java プラットフォームでは、ロケールは、言語と地域の特定の組み合わせに対する単なる識別子です。ロケール固有の属性の集合ではありません。ロケール固有の情報は、ロケールの影響を受けるクラスがそれぞれ独自に保持しています。このような設計のため、ロケール固有のリソースを保持する方法に関し、ユーザオブジェクトとシステムオブジェクトで違いはありません。");
			assertGetString(c, columnName, anss);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_fulltext) values (1, 44, 'hogehoge', ?)");
			anss.add("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_fulltext) values (1, 44, 'hogehoge', ?)");
			assertGetString(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空文字列 → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetString(c, columnName, "");
			anss.add(stringToBytes(null, 50));
			assertGetBytes(c, columnName, anss);

			// 文字列 → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetString(c, columnName, "binary column test");
			anss.add(stringToBytes("binary column test", 50));
			assertGetBytes(c, columnName, anss);

			// 最大長超過 → v15.0 以降では例外 StringRightTruncation が throw されるはず
			// キャストが不要の場合には例外は throw されないが、ここでは文字列を BINARY 型の列に挿入しようとしてキャストが発生する
			assertSetString(c, columnName, "123456789012345678901234567890", stringRightTruncation);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空文字列 → 0 バイトのバイト列になるはず
			assertSetString(c, columnName, "");
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列 → 文字列をそのままバイト列にしたものになるはず
			assertSetString(c, columnName, "blob column test");
			anss.add(stringToBytes("blob column test"));
			assertGetBytes(c, columnName, anss);
		}
		*/

		// NCLOB
		/* 動かない
		{
			columnName = "f_nclob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetString(c, columnName);

			// 空文字列 → 空文字列になるはず
			assertSetString(c, columnName, "");
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列 → 挿入した文字列になるはず
			assertSetString(c, columnName, "nclob test!");
			anss.add("nclob test!");
			assertGetString(c, columnName, anss);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50",			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetString(c, columnName, null);
			assertGetArray(c, columnName);

			// 空文字列 → 例外 ClassCast が throw されるはず
			assertSetString(c, columnName, "", classCast);

			// 文字列 → 例外 ClassCast が throw されるはず
			assertSetString(c, columnName, "hogehoge", classCast);
		}

		// bigint 列は v15.0 からサポート
		columnName = "af_bigint";	// BIGINT 配列

		// null → null になるはず
		assertSetString(c, columnName, null);
		assertGetArray(c, columnName);

		// 空文字列 → 例外 ClassCast が throw されるはず
		assertSetString(c, columnName, "", classCast);

		// 文字列 → 例外 ClassCast が throw されるはず
		assertSetString(c, columnName, "hogehoge", classCast);

		// decimal 列は v16.1 からサポート
		columnName = "af_decimal";	// DECIMAL 配列

		// null → null になるはず
		assertSetString(c, columnName, null);
		assertGetArray(c, columnName);

		// 空文字列 → 例外 ClassCast が throw されるはず
		assertSetString(c, columnName, "", classCast);

		// 文字列 → 例外 ClassCast が throw されるはず
		assertSetString(c, columnName, "hogehoge", classCast);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setBytes() のテスト
	public void test_setBytes() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		byte[]	emptyBytes = new byte[0];
		byte[]	testBytes = stringToBytes("hogehoge");
		byte[]	test50Bytes = stringToBytes("hogehoge", 50);
		byte[]	over50Bytes = stringToBytes("trmeister JDBC driver PreparedStatement test");
		byte[]	full50Bytes = stringToBytes("trmeister JDBC driver Pre", 50);

		String	columnName = null;
		java.util.Vector	anss = null;

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBytes(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のバイト列 → 0 バイトのバイト列になるはず
			assertSetBytes(c, columnName, emptyBytes);
			anss.add(emptyBytes);
			assertGetBytes(c, columnName, anss);

			// バイト列 → 挿入したバイト列になるはず
			assertSetBytes(c, columnName, testBytes);
			anss.add(testBytes);
			assertGetBytes(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBytes(c, columnName, null);
			// image 列への is not null は v15.0 から
			assertGetBytes(c, columnName);

			// 空のバイト列 → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetBytes(c, columnName, emptyBytes);
			anss.add(stringToBytes(null, 50));
			assertGetBytes(c, columnName, anss);

			// バイト列 → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetBytes(c, columnName, testBytes);
			anss.add(test50Bytes);
			assertGetBytes(c, columnName, anss);

			// 最大長超過 → 超過分は切れているはず（キャスト不要なので例外 StringRightTruncation は throw されない）
			assertSetBytes(c, columnName, over50Bytes);
			anss.add(full50Bytes);
			assertGetBytes(c, columnName, anss);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBytes(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のバイト列 → 0 バイトのバイト列になるはず
			assertSetBytes(c, columnName, emptyBytes);
			anss.add(emptyBytes);
			assertGetBytes(c, columnName, anss);

			// バイト列 → 挿入したバイト列になるはず
			assertSetBytes(c, columnName, testBytes);
			anss.add(testBytes);
			assertGetBytes(c, columnName, anss);
		}
		*/

		//
		// 以下（の型）の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_int2",				// INT
			"f_bigint",				// BIGINT
			"f_decimal",			// DECIMAL
			"f_char8",				// CHAR
			"f_float",				// FLOAT
			"f_datetime",			// DATETIME
			"f_id",					// UNIQUEIDENTIFIER
			"f_language",			// LANGUAGE
			"f_nchar6",				// NCHAR
			"f_nvarchar256",		// NVARCHAR
			"f_varchar128",			// VARCHAR
			"f_ntext",				// NTEXT
			"f_ntext_compressed",	// NTEXT(compressed)
			"f_fulltext",			// FULLTEXT
			// "f_nclob",			// NCLOB //動かない
			"af_int",				// INT 配列
			"af_bigint",			// BIGINT 配列
			"af_decimal",			// DECIMAL 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			if (columnName.equals("f_bigint") || columnName.equals("af_bigint")) continue;

			if (columnName.equals("f_decimal") || columnName.equals("af_decimal")) continue;

			// null → null になるはず
			assertSetBytes(c, columnName, null);
			if (columnName.equals("f_int2")) {
				assertGetInt(c, columnName);
			} else if (columnName.equals("f_bigint")) {
				assertGetLong(c, columnName);
			} else if (columnName.equals("f_decimal")) {
				assertGetBigDecimal(c, columnName);
			} else if (columnName.equals("f_float")) {
				assertGetFloat(c, columnName);
			} else if (columnName.equals("f_datetime")) {
				assertGetTimestamp(c, columnName);
			} else if (	columnName.equals("f_char8")			||
						columnName.equals("f_id")				||
						columnName.equals("f_language")			||
						columnName.equals("f_nchar6")			||
						columnName.equals("f_nvarchar256")		||
						columnName.equals("f_varchar128")		||
						columnName.equals("f_ntext")			||
						columnName.equals("f_ntext_compressed")	||
						columnName.equals("f_fulltext")			||
						columnName.equals("f_nclob")) {
				assertGetString(c, columnName);
			} else {
				assertGetArray(c, columnName);
			}

			// 空のバイト列 → 例外 ClassCast が throw されるはず
			assertSetBytes(c, columnName, emptyBytes, classCast);

			// バイト列 → 例外 ClassCast が throw されるはず
			assertSetBytes(c, columnName, testBytes, classCast);
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setDate(int, java.sql.Date) のテスト
	public void test_setDate1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		java.sql.Date	testDate = java.sql.Date.valueOf("2004-12-07");
		String			testDateString = "2004-12-07 00:00:00.000";
		String			testDateGUID = "2004-12-07 00:00:00.000             ";

		String	columnName = null;

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();

		// INT
		{
			columnName = "f_int2";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetInt(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";

		// null → null になるはず
		assertSetDate(c, columnName, null);
		assertGetLong(c, columnName);

		// 暦 → 例外 ClassCast が throw されるはず
		assertSetDate(c, columnName, testDate, classCast);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";

		// null → null になるはず
		assertSetDate(c, columnName, null);
		assertGetLong(c, columnName);

		// 暦 → 例外 ClassCast が throw されるはず
		assertSetDate(c, columnName, testDate, classCast);

		// CHAR
		{
			columnName = "f_char8";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 列 f_char8 は 8 文字までという制限なので、v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetDate(c, columnName, testDate, stringRightTruncation);
		}

		// FLOAT
		{
			columnName = "f_float";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetFloat(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetDate(c, columnName);

			// 暦 → ちゃんと暦になるはず
			assertSetDate(c, columnName, testDate);
			assertGetDate(c, columnName, testDate);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず（ v15.0 以降では末尾が空白文字で埋まるはず）…？？？
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateGUID);//doquedb用
		}

		// IMAGE
		{
			columnName = "f_image";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetBytes(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// LANGUAGE
		{
			columnName = "f_language";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// NCHAR
		{
			columnName = "f_nchar6";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 列 f_nchar6 は 6 文字までという制限なので、v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetDate(c, columnName, testDate, stringRightTruncation);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}

		// NTEXT
		{
			columnName = "f_ntext";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}

		// BINARY
		{
			columnName = "f_binary50";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetBytes(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetBytes(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}
		*/

		// NCLOB
		/* 動かない
		{
			columnName = "f_nclob";

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetString(c, columnName);

			// 暦 → 日時を示す文字列になるはず
			assertSetDate(c, columnName, testDate);
			assertGetString(c, columnName, testDateString);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed)
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetDate(c, columnName, null);
			assertGetArray(c, columnName);

			// 暦 → 例外 ClassCast が throw されるはず
			assertSetDate(c, columnName, testDate, classCast);
		}

		// bigint 列は v15.0 からサポート
		columnName = "af_bigint";	// BIGINT 配列

		// null → null になるはず
		assertSetDate(c, columnName, null);
		assertGetArray(c, columnName);

		// 暦 → 例外 ClassCast が throw されるはず
		assertSetDate(c, columnName, testDate, classCast);

		// decimal 列は v16.1 からサポート
		columnName = "af_decimal";	// DECIMAL 配列

		// null → null になるはず
		assertSetDate(c, columnName, null);
		assertGetArray(c, columnName);

		// 暦 → 例外 ClassCast が throw されるはず
		assertSetDate(c, columnName, testDate, classCast);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setTimestamp(int, java.sql.Timestamp) のテスト
	public void test_setTimestamp1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		java.sql.Timestamp	testTimestamp = java.sql.Timestamp.valueOf("2004-12-07 19:18:53.487");
		String				testTimestampString = "2004-12-07 19:18:53.487";
		String				testTimestampGUID = "2004-12-07 19:18:53.487             ";

		String	columnName = null;

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();

		// INT
		{
			columnName = "f_int2";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetInt(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// BIGINT
		// bigint 列は v15.0 からサポート
		columnName = "f_bigint";

		// null → null になるはず
		assertSetTimestamp(c, columnName, null);
		assertGetLong(c, columnName);

		// 日時 → 例外 ClassCast が throw されるはず
		assertSetTimestamp(c, columnName, testTimestamp, classCast);

		// DECIMAL
		// decimal 列は v16.1 からサポート
		columnName = "f_decimal";

		// null → null になるはず
		assertSetTimestamp(c, columnName, null);
		assertGetLong(c, columnName);

		// 日時 → 例外 ClassCast が throw されるはず
		assertSetTimestamp(c, columnName, testTimestamp, classCast);

		// CHAR
		{
			columnName = "f_char8";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 列 f_char8 は 8 文字までという制限なので、v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, stringRightTruncation);
		}

		// FLOAT
		{
			columnName = "f_float";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetFloat(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetTimestamp(c, columnName);

			// 日時 → ちゃんと暦になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetTimestamp(c, columnName, testTimestampString);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず（ v15.0 以降では末尾が空白文字で埋まるはず）…？？？
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampGUID);//doquedb用
		}

		// IMAGE
		{
			columnName = "f_image";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetBytes(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// LANGUAGE
		{
			columnName = "f_language";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// NCHAR
		{
			columnName = "f_nchar6";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 列 f_nchar6 は 6 文字までという制限なので、v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, stringRightTruncation);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}

		// NTEXT
		{
			columnName = "f_ntext";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}

		// BINARY
		{
			columnName = "f_binary50";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetBytes(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetBytes(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}
		*/

		// NCLOB
		/* 動かない
		{
			columnName = "f_nclob";

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetString(c, columnName);

			// 日時 → 日時を示す文字列になるはず
			assertSetTimestamp(c, columnName, testTimestamp);
			assertGetString(c, columnName, testTimestampString);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetTimestamp(c, columnName, null);
			assertGetArray(c, columnName);

			// 日時 → 例外 ClassCast が throw されるはず
			assertSetTimestamp(c, columnName, testTimestamp, classCast);
		}

		// bigint 列は v15.0 からサポート
		columnName = "af_bigint";	// BIGINT 配列

		// null → null になるはず
		assertSetTimestamp(c, columnName, null);
		assertGetArray(c, columnName);

		// 日時 → 例外 ClassCast が throw されるはず
		assertSetTimestamp(c, columnName, testTimestamp, classCast);

		// decimal 列は v16.1 からサポート
		columnName = "af_decimal";	// DECIMAL 配列

		// null → null になるはず
		assertSetTimestamp(c, columnName, null);
		assertGetArray(c, columnName);

		// 日時 → 例外 ClassCast が throw されるはず
		assertSetTimestamp(c, columnName, testTimestamp, classCast);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setAsciiStream() のテスト
	public void test_setAsciiStream() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	testDir = "test" + java.io.File.separator;
		String	emptyDataFile		= testDir + "empty.dat";
		String	decDataFile			= testDir + "ascii_dec.dat";
		String	hexDataFile			= testDir + "ascii_hex.dat";
		String	floatDataFile		= testDir + "ascii_float.dat";		// XXX.XXX
		String	floatEDataFile		= testDir + "ascii_float_e.dat";	// XXXEX
		String	floatEPDataFile		= testDir + "ascii_float_ep.dat";	// XXXE+X
		String	floatEMDataFile		= testDir + "ascii_float_em.dat";	// XXXE-X
		String	charDataFile		= testDir + "ascii_char.dat";
		String	shortCharDataFile	= testDir + "ascii_short_char.dat";
		String	largeCharDataFile	= testDir + "ascii_large_char.dat";
		String	calendarDataFile	= testDir + "ascii_calendar.dat";
		String	guidDataFile		= testDir + "ascii_guid.dat";
		String	languageDataFile	= testDir + "ascii_lang.dat";

		String	columnName = null;
		java.util.Vector	anss = null;

		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	badArgument = new BadArgument();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();
		java.sql.SQLException	modLibraryError = new ModLibraryError(new ExceptionData());

		// INT
		{
			columnName = "f_int2";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetInt(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add(new Integer(0));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, decDataFile);
			anss.add(Integer.valueOf(readString(decDataFile)));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, invalidCharacter);
		}

		// BIGINT
		{
			// bigint 列は v15.0 からサポート
			columnName = "f_bigint";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetLong(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add(new Long(0L));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, decDataFile);
			anss.add(Long.valueOf(readString(decDataFile)));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DECIMAL
		{
			// decimal 列は v16.1 からサポート
			columnName = "f_decimal";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetBigDecimal(c, columnName);

			// 空のストリーム → InvalidCharacter
			assertSetAsciiStream(c, columnName, emptyDataFile, invalidCharacter);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, decDataFile);
			anss.add(new BigDecimal(readString(decDataFile)).setScale(5));
			assertGetBigDecimal(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, invalidCharacter);
		}

		// CHAR
		{
			columnName = "f_char8";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_char8 は 8 文字）
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("        ");//doquedb用
			assertGetString(c, columnName, anss);

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetAsciiStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 8));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 8));
			assertGetString(c, columnName, anss);
		}

		// FLOAT
		{
			columnName = "f_float";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetFloat(c, columnName);

			// 空のストリーム → 0 になるはず
			// ↑ではなく、空の文字列はfloatにキャストできないという仕様なので例外になるらしい
			assertSetAsciiStream(c, columnName, emptyDataFile, invalidCharacter);
//			anss.add(new Float((float)0));
//			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その１ XXX.XXX ） → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, floatDataFile);
			anss.add(Float.valueOf(readString(floatDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その２ XXXEX） → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, floatEDataFile);
			String	f = readString(floatEDataFile);
			anss.add(Float.valueOf(f));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その３ XXXE+X） → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, floatEPDataFile);
			anss.add(Float.valueOf(readString(floatEPDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その４ XXXE-X） → ちゃんと数字になるはず
			assertSetAsciiStream(c, columnName, floatEMDataFile);
			anss.add(Float.valueOf(readString(floatEMDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム、だけど 16 進数 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetTimestamp(c, columnName);

			// 空のストリーム → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetAsciiStream(c, columnName, emptyDataFile, invalidDatetimeFormat);

			// ちゃんと暦を示す文字列をもつストリーム → ちゃんと暦になるはず
			assertSetAsciiStream(c, columnName, calendarDataFile);
			assertGetTimestamp(c, columnName, readString(calendarDataFile));

			// 暦になれない文字列をもつストリーム → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, invalidDatetimeFormat);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では GUID を示す文字数分すべて空白文字になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("                                    ");
			assertGetString(c, columnName, anss);

			// ちゃんと GUID を示す文字列をもつストリーム → ちゃんと GUID になるはず
			assertSetAsciiStream(c, columnName, guidDataFile);
			anss.add(readString(guidDataFile));
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（短い）をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetAsciiStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 36));
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（長い）をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 36));
			assertGetString(c, columnName, anss);
		}

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readString(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}

		// LANGUAGE
		{
			columnName = "f_language";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// ちゃんと言語を示す文字列をもつストリーム → ちゃんと言語になるはず
			assertSetAsciiStream(c, columnName, languageDataFile);
			anss.add(readString(languageDataFile));
			assertGetString(c, columnName, anss);

			// 言語になれない文字列をもつストリーム → v14.0 では例外 ModLibraryError が、v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetAsciiStream(c, columnName, shortCharDataFile, invalidCharacter);
		}

		// NCHAR
		{
			columnName = "f_nchar6";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_nchar6 は 6 文字）
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("      ");
			assertGetString(c, columnName, anss);

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetAsciiStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 6));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 6));
			assertGetString(c, columnName, anss);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT
		{
			columnName = "f_ntext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add(stringToBytes(null, 50));
			assertGetBytes(c, columnName, anss);

			// 短い文字列をもつストリーム → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetAsciiStream(c, columnName, shortCharDataFile);
			anss.add(stringToBytes(readString(shortCharDataFile), 50));
			assertGetBytes(c, columnName, anss);

			// バイト列に変換すると最大長を超過する文字列をもつストリーム → v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, stringRightTruncation);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetAsciiStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readString(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}
		*/

		// NCLOB
		/* 動かない
		{
			columnName = "f_nclob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetAsciiStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetAsciiStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetAsciiStream(c, columnName, null);
			assertGetArray(c, columnName);

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetAsciiStream(c, columnName, emptyDataFile, classCast);

			// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
			assertSetAsciiStream(c, columnName, charDataFile, classCast);
		}

		// bigint 列は v15.0 からサポート
		columnName = "af_bigint";	// BIGINT 配列

		// null → null になるはず
		assertSetAsciiStream(c, columnName, null);
		assertGetArray(c, columnName);

		// 空のストリーム → 例外 ClassCast が throw されるはず
		assertSetAsciiStream(c, columnName, emptyDataFile, classCast);

		// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
		assertSetAsciiStream(c, columnName, charDataFile, classCast);

		// decimal 列は v16.1 からサポート
		columnName = "af_decimal";	// DECIMAL 配列

		// null → null になるはず
		assertSetAsciiStream(c, columnName, null);
		assertGetArray(c, columnName);

		// 空のストリーム → 例外 ClassCast が throw されるはず
		assertSetAsciiStream(c, columnName, emptyDataFile, classCast);

		// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
		assertSetAsciiStream(c, columnName, charDataFile, classCast);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setUnicodeStream() のテスト
	public void test_setUnicodeStream() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	testDir = "test" + java.io.File.separator;
		String	emptyDataFile			= testDir + "empty.dat";
		String	decDataFile				= testDir + "utf8_dec.dat";
		String	hexDataFile				= testDir + "utf8_hex.dat";
		String	floatDataFile			= testDir + "utf8_float.dat";		// XXX.XXX
		String	floatEDataFile			= testDir + "utf8_float_e.dat";		// XXXEX
		String	floatEPDataFile			= testDir + "utf8_float_ep.dat";	// XXXE+X
		String	floatEMDataFile			= testDir + "utf8_float_em.dat";	// XXXE-X
		String	charDataFile			= testDir + "utf8_char.dat";
		String	shortCharDataFile		= testDir + "utf8_short_char.dat";
		String	largeCharDataFile		= testDir + "utf8_large_char.dat";
		String	calendarDataFile		= testDir + "utf8_calendar.dat";
		String	guidDataFile			= testDir + "utf8_guid.dat";
		String	languageDataFile		= testDir + "utf8_lang.dat";
		String	asciiCharDataFile		= testDir + "ascii_char.dat";
		String	asciiShortCharDataFile	= testDir + "ascii_short_char.dat";

		String	columnName = null;
		java.util.Vector	anss = null;

		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	badArgument = new BadArgument();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();
		java.sql.SQLException	modLibraryError = new ModLibraryError(new ExceptionData());

		// INT
		{
			columnName = "f_int2";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetInt(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add(new Integer(0));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, decDataFile);
			anss.add(Integer.valueOf(readUTF8(decDataFile)));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, invalidCharacter);
		}

		// BIGINT
		{
			// bigint 列は v15.0 からサポート
			columnName = "f_bigint";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetLong(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add(new Long(0L));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, decDataFile);
			anss.add(Long.valueOf(readUTF8(decDataFile)));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DECIMAL
		{
			// decimal 列は v16.1 からサポート
			columnName = "f_decimal";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetBigDecimal(c, columnName);

			// 空のストリーム → InvalidCharacter
			assertSetUnicodeStream(c, columnName, emptyDataFile, invalidCharacter);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, decDataFile);
			anss.add(new BigDecimal(readUTF8(decDataFile)).setScale(5));
			assertGetBigDecimal(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, invalidCharacter);
		}

		// CHAR
		{
			columnName = "f_char8";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_char8 は 8 文字）
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("        ");
			assertGetString(c, columnName, anss);

			// [ 注意! ] Sydney の仕様
			// CHAR 型の列には ASCII しか入れられない。
			// ASCII 以外を入れたいのであれば NCHAR 型の列に入れる。

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetUnicodeStream(c, columnName, asciiShortCharDataFile);
			anss.add(appendSpace(readUTF8(asciiShortCharDataFile), 8));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetUnicodeStream(c, columnName, asciiCharDataFile);
			anss.add(readUTF8(asciiCharDataFile, 8));
			assertGetString(c, columnName, anss);
		}

		// FLOAT
		{
			columnName = "f_float";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetFloat(c, columnName);

			// 空のストリーム → 0 になるはず
			// ↑ではなく、空の文字列はfloatにキャストできないという仕様なので例外になるらしい
			assertSetUnicodeStream(c, columnName, emptyDataFile, invalidCharacter);
//			anss.add(new Float((float)0));
//			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その１ XXX.XXX） → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, floatDataFile);
			anss.add(Float.valueOf(readUTF8(floatDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その２ XXXEX） → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, floatEDataFile);
			anss.add(Float.valueOf(readUTF8(floatEDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その３ XXXE+X） → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, floatEPDataFile);
			anss.add(Float.valueOf(readUTF8(floatEPDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その４ XXXE-X） → ちゃんと数字になるはず
			assertSetUnicodeStream(c, columnName, floatEMDataFile);
			anss.add(Float.valueOf(readUTF8(floatEMDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム、だけど 16 進数 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetTimestamp(c, columnName);

			// 空のストリーム → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile, invalidDatetimeFormat);

			// ちゃんと暦を示す文字列をもつストリーム → ちゃんと暦になるはず
			assertSetUnicodeStream(c, columnName, calendarDataFile);
			assertGetTimestamp(c, columnName, readUTF8(calendarDataFile));

			// 暦になれない文字列をもつストリーム → v14.0 では例外 BadArgument が、v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, invalidDatetimeFormat);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では GUID を示す文字数分すべて空白文字になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("                                    ");
			assertGetString(c, columnName, anss);

			// ちゃんと GUID を示す文字列をもつストリーム → ちゃんと GUID になるはず
			assertSetUnicodeStream(c, columnName, guidDataFile);
			anss.add(readUTF8(guidDataFile));
			assertGetString(c, columnName, anss);

			// [ 注意! ] Sydney の仕様
			// CHAR 型の列には ASCII しか入れられない。
			// ASCII 以外を入れたいのであれば NCHAR 型の列に入れる。
			// UNIQUEIDENTIFIER 型も内部的には CHAR 型。

			// GUID になれない文字列（短い）をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetUnicodeStream(c, columnName, asciiShortCharDataFile);
			anss.add(appendSpace(readString(asciiShortCharDataFile), 36));
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（長い）をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetUnicodeStream(c, columnName, asciiCharDataFile);
			anss.add(readString(asciiCharDataFile, 36));
			assertGetString(c, columnName, anss);
		}

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetUnicodeStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readUTF8(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}

		// LANGUAGE
		{
			columnName = "f_language";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// ちゃんと言語を示す文字列をもつストリーム → ちゃんと言語になるはず
			assertSetUnicodeStream(c, columnName, languageDataFile);
			anss.add(readUTF8(languageDataFile));
			assertGetString(c, columnName, anss);

			// 言語になれない文字列をもつストリーム → v14.0 では例外 ModLibraryError が、v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetUnicodeStream(c, columnName, shortCharDataFile, invalidCharacter);
		}

		// NCHAR
		{
			columnName = "f_nchar6";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_nchar6 は 6 文字）
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("      ");
			assertGetString(c, columnName, anss);

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetUnicodeStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readUTF8(shortCharDataFile), 6));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetUnicodeStream(c, columnName, charDataFile);
			anss.add(readUTF8(charDataFile, 6));
			assertGetString(c, columnName, anss);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, charDataFile);
			anss.add(readUTF8(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// [ 注意! ] Sydney の仕様
			// VARCHAR 型の列には ASCII しか入れられない。
			// ASCII 以外を入れたいのであれば NVARCHAR 型の列に入れる。

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, asciiCharDataFile);
			anss.add(readString(asciiCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT
		{
			columnName = "f_ntext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, charDataFile);
			anss.add(readUTF8(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, largeCharDataFile);
			anss.add(readUTF8(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, largeCharDataFile);
			anss.add(readUTF8(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add(stringToBytes(null, 50));
			assertGetBytes(c, columnName, anss);

			// 短い文字列をもつストリーム → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetUnicodeStream(c, columnName, shortCharDataFile);
			anss.add(stringToBytes(readUTF8(shortCharDataFile), 50));
			assertGetBytes(c, columnName, anss);

			// バイト列に変換すると最大長を超過する文字列をもつストリーム → v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, stringRightTruncation);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず。
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetUnicodeStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readUTF8(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}
		*/

		// NCLOB
		/* 動かない
		{
			columnName = "f_nclob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetUnicodeStream(c, columnName, largeCharDataFile);
			anss.add(readUTF8(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetUnicodeStream(c, columnName, null);
			assertGetArray(c, columnName);

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetUnicodeStream(c, columnName, emptyDataFile, classCast);

			// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
			assertSetUnicodeStream(c, columnName, charDataFile, classCast);
		}

		// bigint 列は v15.0 からサポート
		columnName = "af_bigint";	// BIGINT 配列

		// null → null になるはず
		assertSetUnicodeStream(c, columnName, null);
		assertGetArray(c, columnName);

		// 空のストリーム → 例外 ClassCast が throw されるはず
		assertSetUnicodeStream(c, columnName, emptyDataFile, classCast);

		// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
		assertSetUnicodeStream(c, columnName, charDataFile, classCast);

		// decimal 列は v16.1 からサポート
		columnName = "af_decimal";	// DECIMAL 配列

		// null → null になるはず
		assertSetUnicodeStream(c, columnName, null);
		assertGetArray(c, columnName);

		// 空のストリーム → 例外 ClassCast が throw されるはず
		assertSetUnicodeStream(c, columnName, emptyDataFile, classCast);

		// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
		assertSetUnicodeStream(c, columnName, charDataFile, classCast);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setBinaryStream() のテスト
	public void test_setBinaryStream() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		String	testDir = "test" + java.io.File.separator;
		String	emptyDataFile		= testDir + "empty.dat";
		String	binaryDataFile	 	= testDir + "binary.dat";
		String	shortBinaryDataFile	= testDir + "short_binary.dat";

		// 下準備
		createTestTable(c);

		String	columnName = null;
		java.util.Vector	anss = null;

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBinaryStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetBinaryStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// データをもつストリーム → 挿入したバイト列になるはず
			assertSetBinaryStream(c, columnName, binaryDataFile);
			anss.add(readBinary(binaryDataFile));
			assertGetBytes(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBinaryStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetBinaryStream(c, columnName, emptyDataFile);
			anss.add(readBinary(emptyDataFile, 50));
			assertGetBytes(c, columnName, anss);

			// 短いデータをもつストリーム → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetBinaryStream(c, columnName, shortBinaryDataFile);
			anss.add(readBinary(shortBinaryDataFile, 50));
			assertGetBytes(c, columnName, anss);

			// 最大長超過 → 超過分は切れているはず（キャスト不要なので例外 StringRightTruncation は throw されない）
			assertSetBinaryStream(c, columnName, binaryDataFile);
			anss.add(readBinary(binaryDataFile, 50));
			assertGetBytes(c, columnName, anss);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetBinaryStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetBinaryStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// データをもつストリーム → 挿入したバイト列になるはず
			assertSetBinaryStream(c, columnName, binaryDataFile);
			anss.add(readBinary(binaryDataFile));
			assertGetBytes(c, columnName, anss);
		}
		*/

		//
		// 以下（の型）の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"f_int2",				// INT
			"f_bigint",				// BIGINT
			"f_decimal",			// DECIMAL
			"f_char8",				// CHAR
			"f_float",				// FLOAT
			"f_datetime",			// DATETIME
			"f_id",					// UNIQUEIDENTIFIER
			"f_language",			// LANGUAGE
			"f_nchar6",				// NCHAR
			"f_nvarchar256",		// NVARCHAR
			"f_varchar128",			// VARCHAR
			"f_ntext",				// NTEXT
			"f_ntext_compressed",	// NTEXT(compressed)
			"f_fulltext",			// FULLTEXT
			// "f_nclob",			// NCLOB //動かない
			"af_int",				// INT 配列
			"af_bigint",			// BIGINT 配列
			"af_decimal",			// DECIMAL 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// bigint 列は v15.0 からサポート
			if (columnName.equals("f_bigint") || columnName.equals("af_bigint")) continue;

			// decimal 列は v16.1 からサポート
			if (columnName.equals("f_decimal") || columnName.equals("af_decimal")) continue;

			// null → null になるはず
			assertSetBinaryStream(c, columnName, null);
			if (columnName.equals("f_int2")) {
				assertGetInt(c, columnName);
			} else if (columnName.equals("f_bigint")) {
				assertGetLong(c, columnName);
			} else if (columnName.equals("f_decimal")) {
				assertGetBigDecimal(c, columnName);
			} else if (columnName.equals("f_float")) {
				assertGetFloat(c, columnName);
			} else if (columnName.equals("f_datetime")) {
				assertGetTimestamp(c, columnName);
			} else if (	columnName.equals("f_char8")			||
						columnName.equals("f_id")				||
						columnName.equals("f_language")			||
						columnName.equals("f_nchar6")			||
						columnName.equals("f_nvarchar256")		||
						columnName.equals("f_varchar128")		||
						columnName.equals("f_ntext")			||
						columnName.equals("f_ntext_compressed")	||
						columnName.equals("f_fulltext")			||
						columnName.equals("f_nclob")) {
				assertGetString(c, columnName);
			} else {
				assertGetArray(c, columnName);
			}

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetBinaryStream(c, columnName, emptyDataFile, classCast);

			// データをもつストリーム → 例外 ClassCast が throw されるはず
			assertSetBinaryStream(c, columnName, binaryDataFile, classCast);
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.clearParameters() のテスト
	public void test_clearParameters() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// clearParameters したあとに executeUpdate したら v14.0 では例外 BadArgument が、v15.0 以降では例外 SQLSyntaxError が throw されるはず
		//

		String	query = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_int2, f_nchar6) values (101, ?, 'test', ?, ?)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		ps.setInt(1, 1);			// f_int1 (primary key)
		ps.setInt(2, 53);			// f_int2
		ps.setString(3, "hoge");	// f_nchar6
		ps.clearParameters();

		boolean	caught = false;
		try {
			ps.executeUpdate();
		} catch (SQLException	sqle) {
			caught = true;
			assertDynamicParameterNotMatch(sqle);
		}
		assertTrue(caught);

		//
		// clearParameters しないかぎり前の setter メソッドでの設定が残っているはず
		//

		ps.setInt(1, 1);			// f_int1 (primary key)
		ps.setInt(2, 801);			// f_int2
		ps.setString(3, "before");	// f_nchar6
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int	expected = 1;
		assertEquals(expected, ps.executeUpdate());

		ps.setInt(1, 2);	// f_int1 (primary key)
		ps.setInt(2, 659);	// f_int2
		assertEquals(expected, ps.executeUpdate()); // ← f_int2 だけ設定して、f_nchar6 は前の設定のまま

		ps.close();

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select f_int2, f_nchar6 from t order by f_int1"));
		// ひとつめの確認
		assertTrue(rs.next());
		assertEquals(801, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertEquals("before", rs.getString(2));
		assertFalse(rs.wasNull());
		// ふたつめの確認
		assertTrue(rs.next());
		assertEquals(659, rs.getInt(1));
		assertFalse(rs.wasNull());
		assertEquals("before", rs.getString(2));	// ← ひとつめの挿入後に clearParameters をせず、パラメータも設定しないままふたつめを挿入したので、ひとつめと同じ文字列が挿入されているはず
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}
	/* TODO: 要修正
	//以下でエラーが出た場合は export DISPLAY=:0.0 と設定する
	// PreparedStatement.setObject(int, Object) のテスト
	public void test_setObject1() throws Exception
	{
		checkSetObject(2);
	}

	// PreparedStatement.setObject(int, Object, int) のテスト
	public void test_setObject2() throws Exception
	{
		checkSetObject(3);
	}

	// PreparedStatement.setObject(int, Object, int, int) のテスト
	public void test_setObject3() throws Exception
	{
		checkSetObject(4);
	}
	*/

	// PreparedStatement.addBatch() と PreparedStatement.executeBatch() のテスト
	public void test_addAndExecuteBatch() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// 自動コミットモード
		//

		String	insertQuery = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_ntext) values (?, ?, ?, ?)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(insertQuery));

		java.util.Vector	f_int_not_nulls = new java.util.Vector();
		java.util.Vector	f_int1s = new java.util.Vector();
		java.util.Vector	f_char8_not_nulls = new java.util.Vector();
		java.util.Vector	f_ntexts = new java.util.Vector();
		java.util.Vector	batchResults = new java.util.Vector();

		int	f_int1 = 1;

		// ひとつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_ntexts.add(			"public boolean booleanValue() - この Boolean オブジェクトの値をプリミティブ boolean 型として返します。");
		batchResults.add(new Integer(1));
		// ふたつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_ntexts.add(			"public static Boolean valueOf(boolean) - 指定された boolean 値を表す Boolean インスタンスを返します。");
		batchResults.add(new Integer(1));
		// みっつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_ntexts.add(			"public static Boolean valueOf(String) - 指定された String が表す値を持つ Boolean を返します。");
		batchResults.add(new Integer(1));
		// よっつ
		f_int_not_nulls.add(	new Integer(102));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Byte    ");
		f_ntexts.add(			"public byte byteValue() - この Byte の値を byte として返します。");
		batchResults.add(new Integer(1));
		// いつつ
		f_int_not_nulls.add(	new Integer(102));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Byte    ");
		f_ntexts.add(			"public short shortValue() - この Byte の値を short として返します。");
		batchResults.add(new Integer(1));
		// むっつ
		f_int_not_nulls.add(	new Integer(106));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Class   ");
		f_ntexts.add(			"public boolean desiredAssertionStatus() - このメソッドの呼び出し時にこのクラスを初期化する場合、クラスに割り当てられる宣言ステータスを返します。");
		batchResults.add(new Integer(1));

		for (int i = 0; i < f_int_not_nulls.size(); i++) {

			ps.setInt(		1, ((Integer)f_int_not_nulls.elementAt(i)).intValue());
			ps.setInt(		2, ((Integer)f_int1s.elementAt(i)).intValue());
			ps.setString(	3, (String)f_char8_not_nulls.elementAt(i));
			ps.setString(	4, (String)f_ntexts.elementAt(i));
			ps.addBatch();
		}

		// addBatch しただけでは反映されていないはず
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertZero(rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		assertBatchResults(batchResults, ps.executeBatch());
		batchResults.clear();

		// addBatch したので反映されたはず
		assertNotNull(s = c.createStatement());
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertEquals(f_int_not_nulls.size(), rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();
		assertGetInt(c, "f_int_not_null", f_int_not_nulls);
		assertGetInt(c, "f_int1", f_int1s);
		assertGetString(c, "f_char8_not_null", f_char8_not_nulls);
		assertGetString(c, "f_ntext", f_ntexts);

		ps.close();

		//
		// 手動コミットモード
		//

		// rollback

		c.setAutoCommit(false);
		assertNotNull(ps = c.prepareStatement(insertQuery));

		ps.setInt(		1, 109);
		ps.setInt(		2, f_int1);
		ps.setString(	3, "Double  ");
		ps.setString(	4, "public byte byteValue() - この Double の値を (byte にキャストすることにより) byte として返します。");
		batchResults.add(new Integer(1));
		ps.addBatch();
		ps.setInt(		1, 109);
		ps.setInt(		2, f_int1 + 1);
		ps.setString(	3, "Double  ");
		ps.setString(	4, "public short shortValue() - この Double の値を (short にキャストすることにより) short として返します。");
		batchResults.add(new Integer(1));
		ps.addBatch();
		assertBatchResults(batchResults, ps.executeBatch());
		batchResults.clear();

		c.rollback();

		// addBatch して executeBatch したけど rollback したので反映されていないはず
		assertGetInt(c, "f_int_not_null", f_int_not_nulls);
		assertGetInt(c, "f_int1", f_int1s);
		assertGetString(c, "f_char8_not_null", f_char8_not_nulls);
		assertGetString(c, "f_ntext", f_ntexts);

		// 今度は commit

		// ななつ
		f_int_not_nulls.add(	new Integer(110));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Float   ");
		f_ntexts.add(			"public String toString() - この Float オブジェクトの文字列表現を返します。このオブジェクトが表すプリミティブ float 値は、1 つの引数をとる toString メソッドを実行した場合と同じ String に変換されます。");

		int	bottom = f_int_not_nulls.size() - 1;

		ps.setInt(		1, ((Integer)f_int_not_nulls.elementAt(bottom)).intValue());
		ps.setInt(		2, ((Integer)f_int1s.elementAt(bottom)).intValue());
		ps.setString(	3, (String)f_char8_not_nulls.elementAt(bottom));
		ps.setString(	4, (String)f_ntexts.elementAt(bottom));
		batchResults.add(new Integer(1));
		ps.addBatch();
		assertBatchResults(batchResults, ps.executeBatch());
		batchResults.clear();

		c.commit();

		// addBatch して executeBatch して commit したので反映されたはず
		assertGetInt(c, "f_int_not_null", f_int_not_nulls);
		assertGetInt(c, "f_int1", f_int1s);
		assertGetString(c, "f_char8_not_null", f_char8_not_nulls);
		assertGetString(c, "f_ntext", f_ntexts);

		c.setAutoCommit(true);

		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.addBach(String) のテスト
	// ※ PreparedStatement の基本クラスである Statement の addBatch(String) のテスト
	public void test_addBatch() throws Exception
	{
		// Statement.addBatch(String) を呼ぶことになるが、害はないはず

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	insertQuery = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_nvarchar256) values (?, ?, ?, ?)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(insertQuery));

		java.util.Vector	f_int_not_nulls = new java.util.Vector();
		java.util.Vector	f_int1s = new java.util.Vector();
		java.util.Vector	f_char8_not_nulls = new java.util.Vector();
		java.util.Vector	f_nvarchar256s = new java.util.Vector();

		int	f_int1 = 1;

		// ひとつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_nvarchar256s.add(		"public static String toString(boolean) - 指定された boolean を表す String オブジェクトを返します。");
		// ふたつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_nvarchar256s.add(		"public int hashCode() - この Boolean オブジェクトのハッシュコードを返します。");

		for (int i = 0; i < f_int_not_nulls.size(); i++) {

			ps.setInt(		1, ((Integer)f_int_not_nulls.elementAt(i)).intValue());
			ps.setInt(		2, ((Integer)f_int1s.elementAt(i)).intValue());
			ps.setString(	3, (String)f_char8_not_nulls.elementAt(i));
			ps.setString(	4, (String)f_nvarchar256s.elementAt(i));
			ps.addBatch();
		}

		ps.addBatch("どんな文字列を渡しても害はないはず…");

		ps.executeBatch();

		assertGetInt(c, "f_int_not_null", f_int_not_nulls);
		assertGetInt(c, "f_int1", f_int1s);
		assertGetString(c, "f_char8_not_null", f_char8_not_nulls);
		assertGetString(c, "f_nvarchar256", f_nvarchar256s);

		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.clearBatch() のテスト
	public void test_clearBatch() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	insertQuery = "insert into t (f_int_not_null, f_int1, f_char8_not_null, f_float) values (?, ?, ?, ?)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(insertQuery));

		java.util.Vector	f_int_not_nulls = new java.util.Vector();
		java.util.Vector	f_int1s = new java.util.Vector();
		java.util.Vector	f_char8_not_nulls = new java.util.Vector();
		java.util.Vector	f_floats = new java.util.Vector();

		int	f_int1 = 1;

		// ひとつ
		f_int_not_nulls.add(	new Integer(101));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Boolean ");
		f_floats.add(			new Float(3.8));
		// ふたつ
		f_int_not_nulls.add(	new Integer(102));
		f_int1s.add(			new Integer(f_int1++));
		f_char8_not_nulls.add(	"Byte    ");
		f_floats.add(			new Float(6800.0));

		for (int i = 0; i < f_int_not_nulls.size(); i++) {

			ps.setInt(		1, ((Integer)f_int_not_nulls.elementAt(i)).intValue());
			ps.setInt(		2, ((Integer)f_int1s.elementAt(i)).intValue());
			ps.setString(	3, (String)f_char8_not_nulls.elementAt(i));
			ps.setFloat(	4, ((Float)f_floats.elementAt(i)).floatValue());
			ps.addBatch();
		}

		ps.clearBatch();
		ps.executeBatch();

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertZero(rs.getInt(1));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.cancel() のテスト
	// ※ PreparedStatement の基本クラスである Statement の cancel() のテスト
	public void test_cancel() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		int	numberOfTuples = 1000;
		for (int i = 0; i < numberOfTuples; i++)
			s.executeUpdate("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_int2) values (1, " + (i + 1) + ", 'hogehoge',  1)");
		s.close();

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select f_int2 from t where f_int2 = ?"));
		ps.setInt(1, 1);
		ResultSet	rs = null;
		assertNotNull(rs = ps.executeQuery());

		//
		// ※ cancel() したからといって、すぐに ResultSet.next() が
		// 　 false を返すわけではない。
		// 　 若干のブランクがある。
		//

		// YET! キャンセルされたら警告が積まれる仕様となったので、ちゃんと getWarnings() してチェックする！

		int	t = 0;
		assertTrue(rs.next());
		t++;
		ps.cancel();
		while (rs.next()) t++;
		assertFalse(rs.next());
		System.out.println("inserted number of tuples = " + numberOfTuples);
		System.out.println("got number of tuples      = " + t);
		assertTrue(t < numberOfTuples); // でも全件取得する前にキャンセルされるはず

		rs.close();
		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getWarnings() と PreparedStatement.clearWarnings() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getWarnings() と clearWarnings() のテスト
	public void test_getAndClearWarnings() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// getWarnings() と clearWarnings() は、
		// setFetchDirection() などのテストで
		// 散々やっているので、ここでは省く。

		ps.close();

		//
		// 閉じた PreparedStatement に対して getWarnings() を呼び出すと
		// 例外 SessionNotAvailable が発生するはず
		//

		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			ps.getWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.executeQuery(String) のテスト
	// ※ PreparedStatement の基本クラスである Statement の executeQuery(String) のテスト
	public void test_executeQuery2() throws Exception
	{
		// String を引数とする executeQuery() はサポート外のはず
		// （ NotSupported が throw されるはず）

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.executeQuery("select * from t where id = ?");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.executeUpdate(String) のテスト
	// ※ PreparedStatement の基本クラスである Statement の executeUpdate(String) のテスト
	public void test_executeUpdate2() throws Exception
	{
		// NotSupported が throw されるはず

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		String	query = "insert into t values (3)";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		boolean	caught = false;
		try {
			ps.executeUpdate(query);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		caught = false;
		try {
			ps.executeUpdate(query, Statement.NO_GENERATED_KEYS);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		int[]	columnIndexes = { 1 };
		caught = false;
		try {
			ps.executeUpdate(query, columnIndexes);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		String[]	columnNames = { "id" };
		caught = false;
		try {
			ps.executeUpdate(query, columnNames);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getConnection() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getConnection() のテスト
	public void test_getConnection() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		assertEquals(c, ps.getConnection());

		ps.close();

		//
		// 閉じた PreparedStatement に対して getConnection() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(c, ps.getConnection());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getMoreResults() と PreparedStatement.getMoreResults(int) のテスト
	// ※ PreparedStatement の基本クラスである Statement の getMoreResults() と getMoreResults(int) のテスト
	public void test_getMoreResults() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		//
		// 現状では getMoreResults() をサポートしていないはず
		//

		boolean	caught = false;
		boolean result = false;
		try {
			result = ps.getMoreResults();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		// getMoreResult has been supported
		assertFalse(caught);
		assertFalse(result);

		caught = false;
		try {
			ps.getMoreResults(Statement.CLOSE_CURRENT_RESULT);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getResultSet() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getResultSet() のテスト
	public void test_getResultSet() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		//
		// 現状では execute() をサポートしていないので
		// getResultSet() もサポートしていないはず
		//

		boolean	caught = false;
		ResultSet r = null;
		try {
			r = ps.getResultSet();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		// getResultSet has been supported
		assertFalse(caught);
		assertNull(r);

		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getResultSetConcurrency() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getResultSetConcurrency() のテスト
	public void test_getResultSetConcurrency() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		// 常に ResultSet.CONCUR_READ_ONLY が得られるはず
		assertEquals(ResultSet.CONCUR_READ_ONLY, ps.getResultSetConcurrency());

		ps.close();

		//
		// 閉じた PreparedStatement に対して getResultSetConcurrency() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.CONCUR_READ_ONLY, ps.getResultSetConcurrency());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getResultSetHoldability() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getResultSetHoldability() のテスト
	public void test_getResultSetHoldability() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		// 常に ResultSet.CLOSE_CUSORS_AT_COMMIT が得られるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, ps.getResultSetHoldability());

		ps.close();

		//
		// 閉じた PreparedStatement に対して getResultSetHoldability() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, ps.getResultSetHoldability());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getResultSetType() のテスト
	// ※ PreparedStatement の基本クラスである Statement の getResultSetType() のテスト
	public void test_getResultSetType() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		// 常に ResultSet.TYPE_FORWARD_ONLY が得られるはず
		assertEquals(ResultSet.TYPE_FORWARD_ONLY, ps.getResultSetType());

		ps.close();

		//
		// 閉じた PreparedStatement に対して getResultSetType() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.TYPE_FORWARD_ONLY, ps.getResultSetType());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getUpdateCount() のテスト
	public void test_getUpdateCount() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		//
		// 現状では getUpdateCount() をサポートしていないはず
		//

		boolean	caught = false;
		try {
			ps.getUpdateCount();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertFalse(caught);
		
		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setCursorName() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setCursorName() のテスト
	public void test_setCursorName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_id) values (?, ?, ?, ?)"));

		SQLWarning	w = null;

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 現状ではカーソルをサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		ps.setCursorName("hogehoge");
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// カーソル名に null を指定しても、未サポートの警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		ps.setCursorName(null);
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// カーソル名に空文字列を指定しても、未サポートの警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		ps.setCursorName("");
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		ps.clearWarnings();

		ps.close();

		//
		// 閉じた PreparedStatement に対して setCursorName() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setCursorName("foo");

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setFetchDirection() と PreparedStatement.getFetchDirection() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setFetchDirection() と getFetchDirection() のテスト
	public void test_setAndGetFetchDirection() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// デフォルト状態でも FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		// FETCH_FORWARD では警告は何もないはず
		ps.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertNull(ps.getWarnings());

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// FETCH_REVERSE はサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_DIRECTION);
		ps.setFetchDirection(ResultSet.FETCH_REVERSE);
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		// FETCH_UNKNOWN はサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_DIRECTION);
		ps.setFetchDirection(ResultSet.FETCH_UNKNOWN);
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		// 警告をクリア
		ps.clearWarnings();
		assertNull(ps.getWarnings());

		// -1 では例外 BadArgument が throw されるはず
		boolean	caught = false;
		try {
			ps.setFetchDirection(-1);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		ps.close();

		//
		// 閉じた PreparedStatement に対して setFetchDirection() と getFetchDirection() を
		// 呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertEquals(ResultSet.FETCH_FORWARD, ps.getFetchDirection());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setFetchSize() と PreparedStatement.getFetchSize() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setFetchSize() と getFetchSize() のテスト
	public void test_setAndGetFetchSize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// デフォルト状態でも 0 が得られるはず
		assertZero(ps.getFetchSize());

		// 0 では警告は何もないはず
		ps.setFetchSize(0);
		assertNull(ps.getWarnings());

		// 常に 0 が得られるはず
		assertZero(ps.getFetchSize());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 0 以外はサポートしていないので警告が出るはず − その１
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE);
		ps.setFetchSize(-1);
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// 常に 0 が得られるはず
		assertZero(ps.getFetchSize());

		// 0 以外はサポートしていないので警告が出るはず − その２
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE);
		ps.setFetchSize(1);
		// 警告のチェック
		assertSQLWarning(ps.getWarnings(), wEC, wSS, wMS);

		// 常に 0 が得られるはず
		assertZero(ps.getFetchSize());

		// 警告をクリア
		ps.clearWarnings();
		assertNull(ps.getWarnings());

		// 常に 0 が得られるはず
		assertZero(ps.getFetchSize());

		ps.close();

		//
		// 閉じた PreparedStatement に対して setFetchSize() と getFetchSize() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setFetchSize(0);
		assertZero(ps.getFetchSize());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setMaxFieldSize() と PreparedStatement.getMaxFieldSize() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setMaxFieldSize() と getMaxFieldSize() のテスト
	public void test_setAndGetMaxFieldSize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(ps.getMaxFieldSize());

		// 現状では setMaxFieldSize() をサポートしておらず何もしないはず
		ps.setMaxFieldSize(0);

		// 現状では getMaxFieldSize() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(ps.getMaxFieldSize());

		// 同上

		ps.setMaxFieldSize(256);
		assertZero(ps.getMaxFieldSize());

		ps.setMaxFieldSize(100000);
		assertZero(ps.getMaxFieldSize());

		// 負数では例外 BadArgument が throw されるはず
		boolean	caught = false;
		try {
			ps.setMaxFieldSize(-1);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);

		ps.close();

		//
		// 閉じた PreparedStatement に対して setMaxFieldSize() と getMaxFieldSize() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setMaxFieldSize(100);
		assertZero(ps.getMaxFieldSize());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setMaxRows() と PreparedStatement.getMaxRows() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setMaxRows() と getMaxRows() のテスト
	public void test_setAndGetMaxRows() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(ps.getMaxRows());

		// 現状では setMaxRows() をサポートしておらず何もしないはず
		ps.setMaxFieldSize(0);

		// 現状では getMaxRows() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(ps.getMaxRows());

		// 同上

		ps.setMaxRows(256);
		assertZero(ps.getMaxRows());

		ps.setMaxRows(100000);
		assertZero(ps.getMaxRows());

		// 負数では例外 BadArgument が throw されるはず
		boolean	caught = false;
		try {
			ps.setMaxRows(-1);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);

		ps.close();

		//
		// 閉じた PreparedStatement に対して setMaxRows() と getMaxRows() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setMaxRows(100);
		assertZero(ps.getMaxRows());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setQueryTimeout() と PreparedStatement.getQueryTimeout() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setQueryTimeout() と getQueryTimeout() のテスト
	public void test_setAndGetQueryTimeout() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(ps.getQueryTimeout());

		// 現状では setQueryTimeout() をサポートしておらず何もしないはず
		ps.setQueryTimeout(10);

		// 現状では getQueryTimeout() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(ps.getQueryTimeout());

		// 同上

		ps.setQueryTimeout(10000);
		assertZero(ps.getQueryTimeout());

		ps.setQueryTimeout(100000);
		assertZero(ps.getQueryTimeout());

		// 負数では例外 BadArgument が throw されるはず
		boolean	caught = false;
		try {
			ps.setQueryTimeout(-1);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);

		ps.close();

		//
		// 閉じた PreparedStatement に対して setQueryTimeout() と getQueryTimeout()
		// を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		ps.setQueryTimeout(60);
		assertZero(ps.getQueryTimeout());

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.close() のテスト
	public void test_close() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (?)"));

		int	numberOfTuples = 3;
		ps.setInt(1, 1);
		ps.executeUpdate();
		ps.setInt(1, 2);
		ps.executeUpdate();
		ps.setInt(1, 2);
		ps.executeUpdate();

		//
		// ResultSet のない状態で PreparedStatement を close
		//

		ps.close();

		//
		// ResultSet のある状態で PreparedStatement を close
		//

		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));
		ps.setInt(1, 2);
		ResultSet	rs = null;
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		ps.close();

		// PreparedStatement を閉じると ResultSet も閉じられるので false が返されるはず
		assertFalse(rs.next());

		rs.close();

		//
		// close した ResultSet のある状態で PreparedStatement を close
		//

		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));
		ps.setInt(1, 1);
		assertNotNull(rs = ps.executeQuery());
		assertTrue(rs.next());
		assertFalse(rs.next());
		rs.close();

		ps.close();

		// 既に閉じた Statement に対して再度 close() を呼び出しても問題ないはず
		ps.close();

		// 何回呼び出したって平気
		for (int i = 0; i < 10; i++) ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setCharacterStream() のテスト
	public void test_setCharacterStream() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	testDir = "test" + java.io.File.separator;
		String	emptyDataFile		= testDir + "empty.dat";
		String	decDataFile			= testDir + "ascii_dec.dat";
		String	hexDataFile			= testDir + "ascii_hex.dat";
		String	floatDataFile		= testDir + "ascii_float.dat";		// XXX.XXX
		String	floatEDataFile		= testDir + "ascii_float_e.dat";	// XXXEX
		String	floatEPDataFile		= testDir + "ascii_float_ep.dat";	// XXXE+X
		String	floatEMDataFile		= testDir + "ascii_float_em.dat";	// XXXE-X
		String	charDataFile		= testDir + "ascii_char.dat";
		String	shortCharDataFile	= testDir + "ascii_short_char.dat";
		String	largeCharDataFile	= testDir + "ascii_large_char.dat";
		String	calendarDataFile	= testDir + "ascii_calendar.dat";
		String	guidDataFile		= testDir + "ascii_guid.dat";
		String	languageDataFile	= testDir + "ascii_lang.dat";

		String	columnName = null;
		java.util.Vector	anss = null;

		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	badArgument = new BadArgument();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();
		java.sql.SQLException	modLibraryError = new ModLibraryError(new ExceptionData());

		// INT
		{
			columnName = "f_int2";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetInt(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add(new Integer(0));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, decDataFile);
			anss.add(Integer.valueOf(readString(decDataFile)));
			assertGetInt(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, invalidCharacter);
		}

		// BIGINT
		// bigint 列は v15.0 からサポート
		{
			columnName = "f_bigint";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetLong(c, columnName);

			// 空のストリーム → 0 になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add(new Long(0L));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, decDataFile);
			anss.add(Long.valueOf(readString(decDataFile)));
			assertGetLong(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DECIMAL
		// decimal 列は v16.1 からサポート
		{
			columnName = "f_decimal";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetBigDecimal(c, columnName);

			// 空のストリーム → InvalidCharacter
			assertSetCharacterStream(c, columnName, emptyDataFile, invalidCharacter);

			// ちゃんと数字列（ 10 進数）をもつストリーム → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, decDataFile);
			anss.add(new BigDecimal(readString(decDataFile)).setScale(5));
			assertGetBigDecimal(c, columnName, anss);

			// ちゃんと数字列、だけど 16 進数をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, invalidCharacter);
		}

		// CHAR
		{
			columnName = "f_char8";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_char8 は 8 文字）
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("        ");
			assertGetString(c, columnName, anss);

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetCharacterStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 8));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 8));
			assertGetString(c, columnName, anss);
		}

		// FLOAT
		{
			columnName = "f_float";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetFloat(c, columnName);

			// 空のストリーム → 0 になるはず
			// ↑ではなく、空の文字列はfloatにキャストできないという仕様なので例外になるらしい
			assertSetCharacterStream(c, columnName, emptyDataFile, invalidCharacter);
			//anss.add(new Float((float)0));
			//assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その１ XXX.XXX） → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, floatDataFile);
			anss.add(Float.valueOf(readString(floatDataFile)));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その２ XXXEX） → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, floatEDataFile);
			String	f = readString(floatEDataFile);
			anss.add(Float.valueOf(f));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その３ XXXE+X） → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, floatEPDataFile);
			f = readString(floatEPDataFile);
			anss.add(Float.valueOf(f));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム（その４ XXXE-X） → ちゃんと数字になるはず
			assertSetCharacterStream(c, columnName, floatEMDataFile);
			f = readString(floatEMDataFile);
			anss.add(Float.valueOf(f));
			assertGetFloat(c, columnName, anss);

			// ちゃんと数字列をもつストリーム、だけど 16 進数 → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, hexDataFile, invalidCharacter);

			// 数字以外も含む文字列をもつストリーム → v15.0 以降では例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, invalidCharacter);
		}

		// DATETIME
		{
			columnName = "f_datetime";

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetTimestamp(c, columnName);

			// 空のストリーム → v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetCharacterStream(c, columnName, emptyDataFile, invalidDatetimeFormat);

			// ちゃんと暦を示す文字列をもつストリーム → ちゃんと暦になるはず
			assertSetCharacterStream(c, columnName, calendarDataFile);
			assertGetTimestamp(c, columnName, readString(calendarDataFile));

			// 暦になれない文字列をもつストリーム → v15.0 以降では例外 InvalidDatetimeFormat が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, invalidDatetimeFormat);
		}

		// UNIQUEIDENTIFIER
		{
			columnName = "f_id";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では GUID を示す文字数分すべて空白文字になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("                                    ");
			assertGetString(c, columnName, anss);

			// ちゃんと GUID を示す文字列をもつストリーム → ちゃんと GUID になるはず
			assertSetCharacterStream(c, columnName, guidDataFile);
			anss.add(readString(guidDataFile));
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（短い）をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetCharacterStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 36));
			assertGetString(c, columnName, anss);

			// GUID になれない文字列（長い）をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 36));
			assertGetString(c, columnName, anss);
		}

		// IMAGE
		{
			columnName = "f_image";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readString(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}

		// LANGUAGE
		{
			columnName = "f_language";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// ちゃんと言語を示す文字列をもつストリーム → ちゃんと言語になるはず
			assertSetCharacterStream(c, columnName, languageDataFile);
			anss.add(readString(languageDataFile));
			assertGetString(c, columnName, anss);

			// 言語になれない文字列をもつストリーム → 例外 InvalidCharacter が throw されるはず
			assertSetCharacterStream(c, columnName, shortCharDataFile, invalidCharacter);
		}

		// NCHAR
		{
			columnName = "f_nchar6";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → v15.0 以降では create table で定義された文字数分すべて空白文字になるはず（ f_nchar6 は 6 文字）
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("      ");
			assertGetString(c, columnName, anss);

			// 短い文字列をもつストリーム → v15.0 以降では末尾が空白文字で埋まるはず
			assertSetCharacterStream(c, columnName, shortCharDataFile);
			anss.add(appendSpace(readString(shortCharDataFile), 6));
			assertGetString(c, columnName, anss);

			// 最大長を超過する文字列をもつストリーム → v15.0 以降では超過分は切れているはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile, 6));
			assertGetString(c, columnName, anss);
		}

		// NVARCHAR
		{
			columnName = "f_nvarchar256";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// VARCHAR
		{
			columnName = "f_varchar128";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT
		{
			columnName = "f_ntext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(readString(charDataFile));
			assertGetString(c, columnName, anss);
		}

		// NTEXT(compressed)
		{
			columnName = "f_ntext_compressed";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// FULLTEXT
		{
			columnName = "f_fulltext";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}

		// BINARY
		{
			columnName = "f_binary50";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → create table で定義されたバイト数のゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add(stringToBytes(null, 50));
			assertGetBytes(c, columnName, anss);

			// 短い文字列をもつストリーム → create table で定義されたバイト数の末尾がゼロ埋めされたバイト列になるはず（ f_binary50 は 50 バイト）
			assertSetCharacterStream(c, columnName, shortCharDataFile);
			anss.add(stringToBytes(readString(shortCharDataFile), 50));
			assertGetBytes(c, columnName, anss);

			// バイト列に変換すると最大長を超過する文字列をもつストリーム → v15.0 以降では例外 StringRightTruncation が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, stringRightTruncation);
		}

		// BLOB
		/* 動かない
		{
			columnName = "f_blob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetBytes(c, columnName);

			// 空のストリーム → 0 バイトのバイト列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add(new byte[0]);
			assertGetBytes(c, columnName, anss);

			// 文字列をもつストリーム → 文字列をそのままバイト列にしたものになるはず
			assertSetCharacterStream(c, columnName, charDataFile);
			anss.add(stringToBytes(readString(charDataFile)));
			assertGetBytes(c, columnName, anss);
		}
		 */

		/* 動かない
		// NCLOB
		{
			columnName = "f_nclob";
			anss = new java.util.Vector();

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetString(c, columnName);

			// 空のストリーム → 空文字列になるはず
			assertSetCharacterStream(c, columnName, emptyDataFile);
			anss.add("");
			assertGetString(c, columnName, anss);

			// 文字列をもつストリーム → ストリームから挿入した文字列になるはず
			assertSetCharacterStream(c, columnName, largeCharDataFile);
			anss.add(readString(largeCharDataFile));
			assertGetString(c, columnName, anss);
		}
		*/

		//
		// 配列型の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		java.sql.SQLException	classCast = new ClassCast();

		String[]	columnNames = {
			"af_int",				// INT 配列
			"af_char8",				// CHAR 配列
			"af_float",				// FLOAT 配列
			"af_datetime",			// DATETIME 配列
			"af_id",				// UNIQUEIDENTIFIER 配列
			"af_image",				// IMAGE 配列
			"af_language",			// LANGUAGE 配列
			"af_nchar6",			// NCHAR 配列
			"af_nvarchar256",		// NVARCHAR 配列
			"af_varchar128",		// VARCHAR 配列
			"af_ntext",				// NTEXT 配列
			"af_ntext_compressed",	// NTEXT(compressed) 配列
			"af_fulltext",			// FULLTEXT 配列
			"af_binary50"			// BINARY 配列
		};

		for (int i = 0; i < columnNames.length; i++) {

			columnName = columnNames[i];

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetArray(c, columnName);

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, emptyDataFile, classCast);

			// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, classCast);
		}

		// bigint 列は v15.0 からサポート
		{

			columnName = "af_bigint";	// BIGINT 配列

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetArray(c, columnName);

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, emptyDataFile, classCast);

			// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, classCast);
		}

		// decimal 列は v16.1 からサポート
		{

			columnName = "af_decimal";	// DECIMAL 配列

			// null → null になるはず
			assertSetCharacterStream(c, columnName, null);
			assertGetArray(c, columnName);

			// 空のストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, emptyDataFile, classCast);

			// 文字列をもつストリーム → 例外 ClassCast が throw されるはず
			assertSetCharacterStream(c, columnName, charDataFile, classCast);
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setArray() のテスト
	public void test_setArray() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// INT 配列
		checkSetIntArray(c);

		// BIGINT 配列
		// bigint 列は v15.0 からサポート
		checkSetBigIntArray(c);

		// DECIMAL 配列
		// decimal 列は v16.1 からサポート
		checkSetDecimalArray(c);

		// CHAR 配列
		checkSetCharArray(c);

		// FLOAT 配列
		checkSetFloatArray1(c);
		checkSetFloatArray2(c);

		// DATETIME 配列
		checkSetDatetimeArray(c);

		// UNIQUEIDENTIFIER 配列
		checkSetUniqueidentifierArray(c);

		// IMAGE 配列
		checkSetImageArray(c);

		// LANGUAGE 配列
		checkSetLanguageArray(c);

		// NCHAR 配列
		checkSetNcharArray(c);

		// NVARCHAR 配列
		checkSetNvarcharArray(c);

		// VARCHAR 配列
		checkSetVarcharArray(c);

		// NTEXT 配列
		checkSetNtextArray(c);

		// NTEXT(compressed) 配列
		checkSetNtextCompressedArray(c);

		// FULLTEXT 配列
		checkSetFulltextArray(c);

		// BINARY 配列
		checkSetBinaryArray(c);

		// null の要素を含む配列オブジェクトで setArray → NotSupported

		// 型の異なる要素が格納されている配列オブジェクトで setArray

		//
		// 以下（の型）の列は null のみ許されるはず
		// （ null 以外では例外 ClassCast が throw されるはず）
		//

		String[]	columnNames = {
			"f_int2",				// INT
			"f_char8",				// CHAR
			"f_float",				// FLOAT
			"f_datetime",			// DATETIME
			"f_id",					// UNIQUEIDENTIFIER
			"f_image",				// IMAGE
			"f_language",			// LANGUAGE
			"f_nchar6",				// NCHAR
			"f_nvarchar256",		// NVARCHAR
			"f_varchar128",			// VARCHAR
			"f_ntext",				// NTEXT
			"f_ntext_compressed",	// NTEXT(compressed)
			"f_fulltext",			// FULLTEXT
			"f_binary50",			// BINARY
			"f_blob",				// BLOB
			"f_nclob",				// NCLOB
		};

		Object[]	ary = { "hogehoge", new Integer(3) };
		java.sql.SQLException	classCast = new ClassCast();
		for (int i = 0; i < columnNames.length; i++) {

			String	columnName = columnNames[i];

			assertSetArray(c, columnName, false, ary, classCast);
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// 手動コミットモードでのテスト
	public void test_manualTransaction() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		c.setAutoCommit(false);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));

		ps.executeUpdate();
		c.rollback();
		ps.close();

		assertNotNull(ps = c.prepareStatement("select count(*) from t"));
		ResultSet	rs = null;
		assertNotNull(rs = ps.executeQuery());
		rs.next();
		assertZero(rs.getInt(1));
		ps.close();

		assertNotNull(ps = c.prepareStatement("insert into t values (1)"));
		ps.executeUpdate();
		c.commit();
		ps.close();

		assertNotNull(ps = c.prepareStatement("select count(*) from t"));
		assertNotNull(rs = ps.executeQuery());
		rs.next();
		assertEquals(1, rs.getInt(1));

		ps.close();

		c.setAutoCommit(true);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setEscapeProcessing() のテスト
	// ※ PreparedStatement の基本クラスである Statement の setEscapeProcessing() のテスト
	public void test_setEscapeProcessing() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setEscapeProcessing(false);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setBoolean() のテスト
	public void test_setBoolean() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setBoolean(1, false);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setBigDecimal() のテスト
	public void test_setBigDecimal() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setBigDecimal(1, new BigDecimal("12345"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertFalse(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setTime(int, java.sql.Time) と
	// PreparedStatement.setTime(int, java.sql.Time, java.util.Calendar) のテスト
	public void test_setTime() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		String	query = "select * from t where id = ?";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		boolean	caught = false;
		try {
			ps.setTime(1, new java.sql.Time(0));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		assertNotNull(ps = c.prepareStatement(query));

		caught = false;
		try {
			ps.setTime(1, new java.sql.Time(0), new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.execute() のテスト
	public void test_execute() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		String	query = "insert into t values (1), (2), (3);";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		boolean	caught = false;
		boolean result = false;
		try {
			result = ps.execute();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		
		// execute has been supported
		assertFalse(caught);
		assertTrue(result);
		ResultSet r = null;
		assertNotNull(r = ps.getResultSet());
		while (r.next());
		assertEquals(3, ps.getUpdateCount());
		assertEquals(-1, ps.getUpdateCount());
		assertFalse(ps.getMoreResults());
		assertEquals(-1, ps.getUpdateCount());

		// PreparedStatement の親クラスである Statement の execute(String) のテスト
		query = "insert into t values (1), (2), (3); select * from t where id = 1";
		caught = false;
		result = false;
		try {
			result = ps.execute(query);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		// execute has been supported
		assertFalse(caught);
		assertTrue(result);
		r = null;
		assertNotNull(r = ps.getResultSet());
		while (r.next());
		assertEquals(3, ps.getUpdateCount());
		assertEquals(-1, ps.getUpdateCount());
		assertTrue(ps.getMoreResults());
		assertNotNull(r = ps.getResultSet());
		while (r.next());
		assertEquals(2, ps.getUpdateCount());
		assertEquals(-1, ps.getUpdateCount());
		assertFalse(ps.getMoreResults());
		assertEquals(-1, ps.getUpdateCount());

		// 同 execute(String, int) のテスト
		caught = false;
		try {
			ps.execute(query, java.sql.Statement.RETURN_GENERATED_KEYS);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// 同 execute(String, int[]) のテスト
		caught = false;
		try {
			int[]	columnIndexes = { 1, 2 };
			ps.execute(query, columnIndexes);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// 同 execute(String, String[]) のテスト
		caught = false;
		try {
			String[]	columnNames = { "f_int2", "f_id" };
			ps.execute(query, columnNames);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setRef() のテスト
	public void test_setRef() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setRef(1, new DummyRef());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setBlob() のテスト
	public void test_setBlob() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setBlob(1, new DummyBlob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setClob() のテスト
	public void test_setClob() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setClob(1, new DummyClob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getMetaData() のテスト
	public void test_getMetaData() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.getMetaData();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setDate(int, java.sql.Date, java.util.Calendar) のテスト
	public void test_setDate2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setDate(1, new java.sql.Date(10000), new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setTimestamp(int, java.sql.Timestamp, java.util.Calendar) のテスト
	public void test_setTimestamp2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.setTimestamp(1, new java.sql.Timestamp(10000), new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.setURL() のテスト
	public void test_setURL() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		java.net.URL	url = new java.net.URL("http://doquedb.src.ricoh.co.jp/");
		boolean	caught = false;
		try {
			ps.setURL(1, url);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getParameterMetaData() のテスト
	public void test_getParameterMetaData() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.getParameterMetaData();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// PreparedStatement.getGeneratedKeys() のテスト
	public void test_getGeneratedKeys() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));

		boolean	caught = false;
		try {
			ps.getGeneratedKeys();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		ps.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// BINARY 列への最大長超過キャストのテスト
	public void test_BinaryStringRightTruncation() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	prms = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";

		byte[]	prmb = stringToBytes(prms);
		// BINARY 型の列へ文字列を挿入するのだから、キャストが必要となる。
		// キャストが必要なとき、列の最大長を超過してしまうのであれば v15.0 以降では例外 StringRightTruncation が throw されるはず
		assertSetString(c, "f_binary50", prms, new StringRightTruncation());

		// BINARY 型の列へバイト列を挿入するのだから、キャストは不要。
		// キャストが不要なとき、列の最大長を超過してしまうのであれば例外が throw されることなく超過分が切られるはず
		assertSetBytes(c, "f_binary50", prmb);

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// Sydney 障害管理−障害処理票No.0326『JDBCのPreparedStatementを使用するとSocketがリークする』の再現
	public void test_0326() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t values (1)");
		s.executeUpdate("insert into t values (2)");
		s.executeUpdate("insert into t values (3)");
		s.close();
		c.close();

		int	i = 0;
		try {
			for (i = 0; i < 10000; i++) {
				assertNotNull(c = getConnection());
				PreparedStatement	ps = null;
				assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));
				ps.setInt(1, 2);
				ResultSet	rs = null;
				assertNotNull(rs = ps.executeQuery());
				while (rs.next());
				rs.close();
				ps.close();
				c.close();
			}
		} catch (SQLException	sqle) {
			System.err.println("counter = " + i);
			sqle.printStackTrace();
			throw sqle;
		}

		assertNotNull(c = getConnection());

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// データベースを準備する
	public void setUp()	throws Exception
	{
		super.setUp();
	}

	// データベースを削除する
	public void tearDown() throws Exception
	{
		super.tearDown();
	}

	private static int	_f_int1 = 1;

	private void createTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		String	query =
			"create table t (																	" +
			"	f_int_not_null		int not null,												" +
			"	f_int1				int,														" +
			"	f_int2				int,														";

		// bigint 列は v15.0 からサポート
		query = query +
			"	f_bigint			bigint,														";

		// decimal 列は v16.1 からサポート
		query = query +
			"	f_decimal			decimal(15,5),												";


		query = query +
			"	f_char8_not_null	char(8) not null,											" +
			"	f_char8				char(8),													" +
			"	f_float				float,														" +
			"	f_datetime			datetime,													" +
			"	f_id				uniqueidentifier,											" +
			"	f_image				image,														" +
			"	f_language			language,													" +
			"	f_nchar6			nchar(6),													" +
			"	f_nvarchar256		nvarchar(256),												" +
			"	f_varchar128		varchar(128),												" +
			"	f_ntext				ntext,														" +
			"	f_ntext_compressed	ntext hint heap 'compressed',								" +
			"	f_fulltext			fulltext,													" +
			"	f_binary50			binary(50),													" +
			"	f_blob				blob,														" +
			"	f_nclob				nclob,														" +
			"	af_int				int					array[no limit],						";

		// bigint 列は v15.0 からサポート
		query = query +
			"	af_bigint			bigint				array[no limit],						";

		// decimal 列は v16.1 からサポート
		query = query +
			"	af_decimal			decimal(15,5)		array[no limit],						";

		query = query +
			"	af_char8			char(8)				array[no limit],						" +
			"	af_float			float				array[no limit],						" +
			"	af_datetime			datetime			array[no limit],						" +
			"	af_id				uniqueidentifier	array[no limit],						" +
			"	af_image			image				array[no limit],						" +
			"	af_language			language			array[no limit],						" +
			"	af_nchar6			nchar(6)			array[no limit],						" +
			"	af_nvarchar256		nvarchar(256)		array[no limit],						" +
			"	af_varchar128		varchar(128)		array[no limit],						" +
			"	af_ntext			ntext				array[no limit],						" +
			"	af_ntext_compressed	ntext				array[no limit] hint heap 'compressed',	" +
			"	af_fulltext			fulltext			array[no limit],						" +
			"	af_binary50			binary(50)			array[no limit],						" +
			"	primary key(f_int1)																" +
			")";
		s.executeUpdate(query);

		s.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		Statement	s = c.createStatement();
		s.executeUpdate("drop table t");
		s.close();
	}

	// INT 配列への PreparedStatement.setArray() のテスト
	private void checkSetIntArray(Connection	c) throws Exception
	{
		String				columnName = "af_int";
		int					elementType = DataType.INTEGER;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			//
			// 以下は要素を INT 型の値に変換可能なはず
			//

			// only Integer
			{
				Integer[]	ary = { new Integer(4), new Integer(8376), new Integer(63) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = { new Integer(4398), new Long(59L), new Integer(60), new Integer(5001) };
				Integer[]	ans = { new Integer(4398), new Integer(59), new Integer(60), new Integer(5001) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = { new Integer(87), new Integer(0), new Float(48.629), new Integer(438) };
				Integer[]	ans = { new Integer(87), new Integer(0), new Integer(48), new Integer(438) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = { new Integer(82), new Double(7483.8) };
				Integer[]	ans = { new Integer(82), new Integer(7483) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				Object[]	ary = { new Double(3.8742), new Integer(14) };
				Integer[]	ans = { new Integer(3), new Integer(14) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 数値に変換可能ならば正常に INT の値に変換されるはず

				Object[]	ary = { new Integer(85), new Integer(38), "32" };
				Integer[]	ans = { new Integer(85), new Integer(38), new Integer(32) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// 数値に変換不可能ならば v15.0 以降では例外 InvalidCharacter が throw されるはず

				Object[]	ary = { new Integer(3), "hogehoge" };
				assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
			}

			// null 混じり
			{
				Integer[]	ary = { new Integer(864), null, new Integer(467) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, DataType.INTEGER);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new Integer(1908), new byte[3], new Integer(1) },							// byte[] 混じり
				{ new Integer(7987), java.sql.Date.valueOf("2004-12-15") },					// java.sql.Date 混じり
				{ new Integer(3), java.sql.Timestamp.valueOf("2004-12-15 13:48:03.472") },	// java.sql.Timestamp 混じり
				{ new Integer(5897), new LanguageData("ja"), new Integer(0) }				// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// BIGINT 配列への PreparedStatement.setArray() のテスト
	private void checkSetBigIntArray(Connection	c) throws Exception
	{
		// bigint 列は v15.0 からサポート
		String				columnName = "af_bigint";
		int					elementType = DataType.INTEGER64;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			//
			// 以下は要素を BIGINT 型に変換可能なはず

			// only Integer
			{
				Integer[]	ary = { new Integer(2983), new Integer(198), new Integer(-5800), new Integer(4) };
				Long[]		ans = { new Long(2983L), new Long(198L), new Long(-5800L), new Long(4L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// only Long
			{
				Long[]	ary  = { new Long(198432768787L), new Long(2987764332L), new Long(32L), new Long(43978168736L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer + Long
			{
				Object[]	ary = { new Integer(502), new Integer(9810), new Long(9158736878L), new Integer(39100), new Long(5198347762837L), new Long(229878762837L) };
				Long[]		ans = { new Long(502L), new Long(9810L), new Long(9158736878L), new Long(39100L), new Long(5198347762837L), new Long(229878762837L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				// Integer に Float 混じり

				Object[]	ary = { new Integer(6189), new Float(30.58278), new Integer(41), new Integer(2081) };
				Long[]		ans = { new Long(6189L), new Long(30L), new Long(41L), new Long(2081L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に Float 混じり

				Object[]	ary = { new Long(19847321767L), new Long(24387L), new Float(5110.48), new Long(-18734673621874L), new Long(48374176873L) };
				Long[]		ans = { new Long(19847321767L), new Long(24387L), new Long(5110L), new Long(-18734673621874L), new Long(48374176873L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Long + Float

				Object[]	ary = { new Integer(2), new Integer(77918), new Long(2987658734628L), new Float(819838.438), new Long(4387928734234L), new Integer(319), new Long(791346768721736413L) };
				Long[]		ans = { new Long(2L), new Long(77918L), new Long(2987658734628L), new Long(819838L), new Long(4387928734234L), new Long(319L), new Long(791346768721736413L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				// Integer に Double 混じり

				Object[]	ary = { new Integer(6692), new Integer(32987), new Double(98273.18734), new Integer(331) };
				Long[]		ans = { new Long(6692L), new Long(32987L), new Long(98273L), new Long(331L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に Double 混じり

				Object[]	ary = { new Long(247938678432L), new Double(762874.877653), new Long(24987347265872L), new Long(198328237948L), new Long(729467623498741L)};
				Long[]		ans = { new Long(247938678432L), new Long(762874L), new Long(24987347265872L), new Long(198328237948L), new Long(729467623498741L)};
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Log + Double

				Object[]	ary = { new Long(9872876523L), new Integer(-42983), new Integer(5), new Long(247748298778L), new Double(59871.158), new Long(1159385267876L) };
				Long[]		ans = { new Long(9872876523L), new Long(-42983L), new Long(5L), new Long(247748298778L), new Long(59871L), new Long(1159385267876L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 数値に変換可能ならば正常に BIGINT の値に変換されるはず

				{
					// Integer に String 混じり

					Object[]	ary = { new Integer(5429), new Integer(3283), "29", new Integer(5110), new Integer(68) };
					Long[]		ans = { new Long(5429L), new Long(3283L), new Long(29L), new Long(5110L), new Long(68L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
				{
					// Long に String 混じり

					Object[]	ary = { new Long(72948361732L), "298341767328", new Long(14398676321L), new Long(-96857681763L) };
					Long[]		ans = { new Long(72948361732L), new Long(298341767328L), new Long(14398676321L), new Long(-96857681763L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
				{
					// Integer + Long + String
					Object[]	ary = { new Integer(22839), new Long(24985768175L), new Integer(7971), "349873891583", new Integer(29078), new Long(8874361876451L) };
					Long[]		ans = { new Long(22839L), new Long(24985768175L), new Long(7971L), new Long(349873891583L), new Long(29078L), new Long(8874361876451L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
			}
			{
				// 数値に変換不可能ならば例外 InvalidCharacter が throw されるはず

				{
					// Integer に String 混じり
					Object[]	ary = { new Integer(9385), new Integer(211), "foo", new Integer(3984) };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
				{
					// Long に String 混じり
					Object[]	ary = { new Long(71984378165L), "xyz" };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
				{
					// Integer + Long + String
					Object[]	ary = { new Long(81953247987L), new Integer(4398), new Long(6165372686L), "a39x", new Integer(54883) };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
			}

			// null 混じり
			{
				// Integer に null 混じり
				Integer[]	ary = { new Integer(339), null, new Integer(5480), new Integer(6) };
				Long[]		ans = { new Long(339L), null, new Long(5480L), new Long(6L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に null 混じり
				Long[]	ary = { new Long(8018937910L), new Long(77109753276L), null, new Long(10329871362L), new Long(990625381042L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Long + null
				Object[]	ary = { new Long(7258397276L), new Integer(33809), null, new Integer(2561), new Integer(9801), new Long(2429877763426L) };
				Long[]		ans = { new Long(7258397276L), new Long(33809L), null, new Long(2561L), new Long(9801L), new Long(2429877763426L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new Integer(40), new byte[10], new Integer(30), new Integer(20) },													// Integer に byte[] 混じり
				{ new Long(5L), new Long(66318746768L), new byte[2], new Long(148246987134872L) },										// Long に byte[] 混じり
				{ new Integer(39), java.sql.Date.valueOf("2005-03-14"), new Integer(4938) },											// Integer に java.sql.Date 混じり
				{ new Long(358462874381L), new Long(7918734767L), java.sql.Date.valueOf("2005-03-14"), new Long(881737629874100L) },	// Long に java.sql.Date 混じり
				{ new Integer(403), java.sql.Timestamp.valueOf("2004-03-14 15:08:31.498"), new Integer(61) },							// Integer に java.sql.Timestamp 混じり
				{ new Long(7781376875177760L), new Long(43298L), java.sql.Timestamp.valueOf("2004-03-14 15:09:06.501") },				// Long に java.sql.Timestamp 混じり
				{ new Integer(60081), new LanguageData("ka"), new Integer(9) },															// Integer に jp.co.ricoh.doquedb.common.LanguageData 混じり
				{ new Long(41983247531087L), new LanguageData("rw"), new Long(227856636498L), new Long(0L) }							// Long に jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// DECIMAL 配列への PreparedStatement.setArray() のテスト
	private void checkSetDecimalArray(Connection	c) throws Exception
	{
		// decimal 列は v16.1 からサポート
		String				columnName = "af_decimal";
		int					elementType = DataType.DECIMAL;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			//
			// 以下は要素を DECIMAL 型に変換可能なはず

			// only Integer
			{
				Integer[]	ary = { new Integer(2983), new Integer(198), new Integer(-5800), new Integer(4) };
				BigDecimal[]	ans = { new BigDecimal(2983L), new BigDecimal(198L), new BigDecimal(-5800L), new BigDecimal(4L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// only Long
			{
				Long[]	ary  = { new Long(19843L), new Long(2987L), new Long(32L), new Long(43978L) };
				BigDecimal[]	ans  = { new BigDecimal(19843L), new BigDecimal(2987L), new BigDecimal(32L), new BigDecimal(43978L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer + Long
			{
				Object[]	ary = { new Integer(502), new Integer(9810), new Long(91587L), new Integer(39100), new Long(5198L), new Long(22987L) };
				BigDecimal[]	ans = { new BigDecimal(502L), new BigDecimal(9810L), new BigDecimal(91587L), new BigDecimal(39100L), new BigDecimal(5198L), new BigDecimal(22987L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				// Integer に Float 混じり

				Object[]	ary = { new Integer(6189), new Float(30.58278), new Integer(41), new Integer(2081) };
				BigDecimal[] ans = { new BigDecimal(6189L), new BigDecimal(30L), new BigDecimal(41L), new BigDecimal(2081L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に Float 混じり

				Object[]	ary = { new Long(19847L), new Long(24387L), new Float(5110.48), new Long(-18734L), new Long(48374L) };
				BigDecimal[]		ans = { new BigDecimal("19847"), new BigDecimal(24387L), new BigDecimal("5110.48"), new BigDecimal(-18734L), new BigDecimal(48374L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Long + Float

				Object[]	ary = { new Integer(2), new Integer(77918), new Long(29876L), new Float(81983.8438), new Long(43879L), new Integer(319), new Long(79134L) };
				BigDecimal[]		ans = { new BigDecimal(2L), new BigDecimal(77918L), new BigDecimal(29876L), new BigDecimal("81983.8438"), new BigDecimal(43879L), new BigDecimal(319L), new BigDecimal(79134L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				// Integer に Double 混じり

				Object[]	ary = { new Integer(6692), new Integer(32987), new Double(98273.18734), new Integer(331) };
				BigDecimal[]		ans = { new BigDecimal(6692L), new BigDecimal(32987L), new BigDecimal("98273.18734"), new BigDecimal(331L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に Double 混じり

				Object[]	ary = { new Long(24793L), new Double(76287.48776), new Long(24987L), new Long(19832L), new Long(72946L)};
				BigDecimal[]		ans = { new BigDecimal(24793L), new BigDecimal("76287.48776"), new BigDecimal(24987L), new BigDecimal(19832L), new BigDecimal(72946L)};
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Log + Double

				Object[]	ary = { new Long(98728L), new Integer(-42983), new Integer(5), new Long(24774L), new Double(59871.158), new Long(11593L) };
				BigDecimal[]		ans = { new BigDecimal(98728L), new BigDecimal(-42983L), new BigDecimal(5L), new BigDecimal(24774L), new BigDecimal("59871.158"), new BigDecimal(11593L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 数値に変換可能ならば正常に DECIMAL の値に変換されるはず

				{
					// Integer に String 混じり

					Object[]	ary = { new Integer(5429), new Integer(3283), "29", new Integer(5110), new Integer(68) };
					BigDecimal[]		ans = { new BigDecimal(5429L), new BigDecimal(3283L), new BigDecimal(29L), new BigDecimal(5110L), new BigDecimal(68L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
				{
					// Long に String 混じり

					Object[]	ary = { new Long(72948L), "29834.17673", new Long(14398L), new Long(-96857L) };
					BigDecimal[]		ans = { new BigDecimal(72948L), new BigDecimal("29834.17673"), new BigDecimal(14398L), new BigDecimal(-96857L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
				{
					// Integer + Long + String
					Object[]	ary = { new Integer(22839), new Long(24985L), new Integer(7971), "34987.38915", new Integer(29078), new Long(88743L) };
					BigDecimal[]		ans = { new BigDecimal(22839L), new BigDecimal(24985L), new BigDecimal(7971L), new BigDecimal("34987.38915"), new BigDecimal(29078L), new BigDecimal(88743L) };
					assertSetArray(c, columnName, isTestArray, ary);
					anss.add(ans);
					assertGetArray(c, columnName, anss, elementType);
				}
			}
			{
				// 数値に変換不可能ならば例外 InvalidCharacter が throw されるはず

				{
					// Integer に String 混じり
					Object[]	ary = { new Integer(9385), new Integer(211), "foo", new Integer(3984) };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
				{
					// Long に String 混じり
					Object[]	ary = { new Long(71984L), "xyz" };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
				{
					// Integer + Long + String
					Object[]	ary = { new Long(81953), new Integer(4398), new Long(61653), "a39x", new Integer(54883) };
					assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
				}
			}

			// null 混じり
			{
				// Integer に null 混じり
				Integer[]	ary = { new Integer(339), null, new Integer(5480), new Integer(6) };
				BigDecimal[]		ans = { new BigDecimal(339L), null, new BigDecimal(5480L), new BigDecimal(6L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Long に null 混じり
				Long[]	ary = { new Long(80189L), new Long(77109L), null, new Long(10329L), new Long(99062L) };
				BigDecimal[] ans = { new BigDecimal(80189L), new BigDecimal(77109L), null, new BigDecimal(10329L), new BigDecimal(99062L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Integer + Long + null
				Object[]	ary = { new Long(72583L), new Integer(33809), null, new Integer(2561), new Integer(9801), new Long(24298L) };
				BigDecimal[]		ans = { new BigDecimal(72583L), new BigDecimal(33809L), null, new BigDecimal(2561L), new BigDecimal(9801L), new BigDecimal(24298L) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new Integer(40), new byte[10], new Integer(30), new Integer(20) },													// Integer に byte[] 混じり
				{ new Long(5L), new Long(66318L), new byte[2], new Long(14824L) },										// Long に byte[] 混じり
				{ new Integer(39), java.sql.Date.valueOf("2005-03-14"), new Integer(4938) },											// Integer に java.sql.Date 混じり
				{ new Long(3584), new Long(7918), java.sql.Date.valueOf("2005-03-14"), new Long(88173) },	// Long に java.sql.Date 混じり
				{ new Integer(403), java.sql.Timestamp.valueOf("2004-03-14 15:08:31.498"), new Integer(61) },							// Integer に java.sql.Timestamp 混じり
				{ new Long(77813), new Long(43298L), java.sql.Timestamp.valueOf("2004-03-14 15:09:06.501") },				// Long に java.sql.Timestamp 混じり
				{ new Integer(60081), new LanguageData("ka"), new Integer(9) },															// Integer に jp.co.ricoh.doquedb.common.LanguageData 混じり
				{ new Long(41983), new LanguageData("rw"), new Long(22785), new Long(0L) }							// Long に jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// CHAR 配列への PreparedStatement.setArray() のテスト
	private void checkSetCharArray(Connection	c) throws Exception
	{
		String				columnName = "af_char8";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = { "hoge", "abcdef", "foo" };
				String[]	ans = { "hoge    ", "abcdef  ", "foo     " };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// データ型のキャストは不要なので、最大長を超える部分は切られるはず
				String[]	ary = { "string element array overflow", "x" };
				String[]	ans = { "string e", "x       " };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = { "xxxy", new Integer(4287) };
				String[]	ans = { "xxxy    ", "4287    " };
				String[]	ans14 = { "xxxy", "4287" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = { "point", new Long(5108L), "batch" };
				String[]	ans = { "point   ", "5108    ", "batch   " };
				String[]	ans14 = { "point", "5108", "batch" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				// 文字列に変換しても文字列長が af_char8 の要素の最大長 (8) よりも短ければ文字列に変換されるはず

				Object[]	ary = { "doquedb", new Float(3), "jdbc" };
				String[]	ans = { "doquedb ", "3E0     ", "jdbc    " };
				String[]	ans14 = { "doquedb", "3.000000", "jdbc" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// 文字列に変換すると文字列長が af_char8 の要素の最大長 (8) を超える場合には v15.0 以降では例外 StringRightTruncation が throw されるはず

				Object[]	ary = { "abc", new Float(3843.87729) };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}

			// Double 混じり
			{
				// Float に同じ

				Object[]	ary = { "text", new Double(30000.0) };
				String[]	ans = { "text    ", "3E4     " };
				String[]	ans14 = { "text", "30000.000000" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// Float に同じ

				Object[]	ary = { "document", new Double(30.0523) };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}

			// null 混じり
			{
				String[]	ary = { "not null", null, "null?" };
				String[]	ans = { "not null", null, "null?   " };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず

				Object[]	ary = { "binary", new byte[3], "byte" };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			// 文字列に変換すると文字列長が af_char8 の要素の最大長 (8) を超えるので v15.0 以降では例外 StringRightTruncation が throw されるはず
			{
				Object[]	ary = { "datetime", java.sql.Date.valueOf("2004-12-15") };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// java.sql.Timestamp 混じり
			// java.sql.Date に同じ
			{
				Object[]	ary = { "time", java.sql.Timestamp.valueOf("2004-12-15 14:38:18.018") };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = { "japan", new LanguageData("ja") };
				String[]	ans = { "japan   ", "ja      " };
				String[]	ans14 = { "japan", "ja" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	//  FLOAT 配列への PreparedStatement.setArray() のテスト
	private void checkSetFloatArray1(Connection	c) throws Exception
	{
		String				columnName = "af_float";
		int					elementType = DataType.FLOAT;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

		// 2007.01.31
		// float 型で登録しようとすると内部で double にキャストされてから登録される
		// float から double にキャストすると間延び？してしまう（仕様）例：float(0.1) → double(0.10000000149011612)
		// その為にテスト側を修正

			// only Float
			{
				Float[]		ary = { new Float(0.1), new Float(3.873), new Float(0.00387) };
//				Double[]	ans = { new Double(0.1), new Double(3.873), new Double(0.00387) };
				Double[]	ans = { new Double( (new Float(0.1)).doubleValue()), new Double( (new Float(3.873)).doubleValue()), new Double( (new Float(0.00387).doubleValue()) ) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = { new Float(378432.8), new Float(0.087432), new Integer(378) };
//				Double[]	ans = { new Double(378432.8), new Double(0.087432), new Double(378) };
				Double[]	ans = { new Double((new Float(378432.8)).doubleValue()), new Double((new Float(0.087432)).doubleValue()), new Double((new Float(378)).doubleValue()) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = { new Float(0.5389), new Long(7127039L), new Float(319.98) };
//				Double[]	ans = { new Double(0.5389), new Double(7127039.0), new Double(319.98) };
				Double[]	ans = { new Double((new Float(0.5389)).doubleValue()), new Double((new Float(7127039.0)).doubleValue()), new Double((new Float(319.98)).doubleValue()) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = { new Float(0.0987), new Double(4398.0), new Float(9.37) };
//				Double[]	ans = { new Double(0.0987), new Double(4398.0), new Double(9.37) };
				Double[]	ans = { new Double((new Float(0.0987)).doubleValue()), new Double((new Float(4398.0)).doubleValue()), new Double((new Float(9.37)).doubleValue()) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 数値に変換可能ならば正常に FLOAT の値に変換されるはず

//				Object[]	ary = { new Float(39.7), "4.099999904632568", new Float(0.013) };
//				Float[]		ans = { new Float(39.7), new Float(4.099999904632568), new Float(0.013) };
				Object[]	ary = { new Float(39.7), "4.09999990463256", new Float(0.013) };
//				Double[]	ans = { new Double(39.7), new Double(4.09999990463256), new Double(0.013) };
				Double[]	ans = { new Double((new Float(39.7)).doubleValue()), new Double((new Float(4.09999990463256)).doubleValue()), new Double((new Float(0.013)).doubleValue()) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			// 数値に変換不可能ならば v15.0 以降では例外 InvalidCharacter が throw されるはず
			{
				Object[]	ary = { new Float(0.85), "abcdefg" };
				assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
			}
			// null 混じり
			{
				Float[]		ary = { new Float(0.01), null, new Float(379) };
//				Double[]	ans = { new Double(0.01), null, new Double(379) };
				Double[]	ans = { new Double((new Float(0.01)).doubleValue()), null, new Double((new Float(379)).doubleValue()) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new Float(0.3), new byte[2], new Float(589732.39) },							// byte[] 混じり
				{ new Float(38742.878), java.sql.Date.valueOf("2004-12-15"), new Float(0.3) },	// java.sql.Date 混じり
				{ new Float(3.478298), java.sql.Timestamp.valueOf("2004-12-15 16:56:19.378") },	// java.sql.Timestamp 混じり
				{ new Float(5873.87), new LanguageData("en"), new Float(0.00878) }				// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	//  FLOAT 配列への PreparedStatement.setArray() のテスト
	private void checkSetFloatArray2(Connection	c) throws Exception
	{
		// checkSetFloatArray1() で af_float にいろいろと値が挿入されているので消す
		Statement	s = c.createStatement();
		s.executeUpdate("delete from t where af_float is not null");
		s.close();

		String				columnName = "af_float";
		int					elementType = DataType.DOUBLE;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only Double
			{
				Double[]	ary = { new Double(43879234.7879), new Double(4837294.0), new Double(7.87798273), new Double(0.097872) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = { new Double(59873.7872), new Integer(9873), new Double(0.098743), new Double(0.387) };
				Double[]	ans = { new Double(59873.7872), new Double(9873), new Double(0.098743), new Double(0.387) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = { new Double(8.915029), new Double(0.389015), new Long(30198787632L), new Double(5987.0898) };
				Double[]	ans = { new Double(8.915029), new Double(0.389015), new Double(30198787632.0), new Double(5987.0898) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = { new Double(4987.287849), new Double(98.3472), new Float(3.0799999237060547) };
				Double[]	ans = { new Double(4987.287849), new Double(98.3472), new Double(3.0799999237060547) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 数値に変換可能ならば正常に FLOAT の値に変換されるはず

				Object[]	ary = { new Double(3.287439), new Double(0.009999873), new Double(3982.89877), "0.5782678" };
				Double[]	ans = { new Double(3.287439), new Double(0.009999873), new Double(3982.89877), new Double(0.5782678) };
				Double[]	ans14 = { new Double(3.287439), new Double(0.009999873), new Double(3982.89877), new Double(0.5782678000000002) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			// 数値に変換不可能ならば v15.0 以降では例外 InvalidCharacter が throw されるはず
			{
				Object[]	ary = { new Double(5987.837982), "hoge" };
				assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
			}
			// null 混じり
			{
				Double[]	ary = { new Double(98472.87987), new Double(0), null };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new Double(5987342.87), new byte[5] },																// byte[] 混じり
				{ new Double(0.0008783), java.sql.Date.valueOf("2004-12-15"), new Double(847387.8) },					// java.sql.Date 混じり
				{ new Double(387.878), java.sql.Timestamp.valueOf("2004-12-15 17:54:38.976"), new Double(98.29387) },	// java.sql.Timestamp 混じり
				{ new Double(0.8784398), new LanguageData("fr"), new Double(6148.87) }									// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// DATETIME 配列への PreparedStatement.setArray() のテスト
	private void checkSetDatetimeArray(Connection	c) throws Exception
	{
		String				columnName = "af_datetime";
		int					elementType = DataType.DATE_TIME;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	badArgument = new BadArgument();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only java.sql.Timestamp
			{
				java.sql.Timestamp[]	ary = {
					java.sql.Timestamp.valueOf("2004-12-15 18:20:38.481"),
					java.sql.Timestamp.valueOf("2001-11-29 13:18:41.036")
				};
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// 日時に変換可能ならば正常に DATETIME の値に変換されるはず

				Object[]	ary = { java.sql.Timestamp.valueOf("2004-03-05 14:49:52.739"), "2002-12-30 04:58:43.008" };
				java.sql.Timestamp[]	ans = {
					java.sql.Timestamp.valueOf("2004-03-05 14:49:52.739"),
					java.sql.Timestamp.valueOf("2002-12-30 04:58:43.008")
				};
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// 日時に変換不可能ならば例外 BadArgument が throw されるはず

				Object[]	ary = { "text", java.sql.Timestamp.valueOf("1970-02-04 06:50:38.301") };
				assertSetArray(c, columnName, isTestArray, ary, invalidDatetimeFormat);
			}

			// null 混じり
			{
				java.sql.Timestamp[]	ary = { java.sql.Timestamp.valueOf("2000-11-05 06:39:48.719"), null };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					java.sql.Timestamp.valueOf("2004-01-03 04:21:39.004"),
					java.sql.Date.valueOf("2003-09-28"),
					java.sql.Timestamp.valueOf("2003-08-08 15:49:21.390")
				};
				java.sql.Timestamp[]	ans = {
					java.sql.Timestamp.valueOf("2004-01-03 04:21:39.004"),
					java.sql.Timestamp.valueOf("2003-09-28 00:00:00.000"),
					java.sql.Timestamp.valueOf("2003-08-08 15:49:21.390")
				};
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ java.sql.Timestamp.valueOf("1970-03-21 19:09:10.583"), new Integer(3) },			// Integer 混じり
				{ java.sql.Timestamp.valueOf("2005-03-14 15:27:05.309"), new Long(70819843287L) },	// Long 混じり
				{ java.sql.Timestamp.valueOf("1997-10-05 21:59:32.008"), new Float(0.03) },			// Float 混じり
				{ new Double(53987.087), java.sql.Timestamp.valueOf("2003-11-09 05:43:20.538") },	// Double 混じり
				{ new byte[1], java.sql.Timestamp.valueOf("2000-10-05 21:49:05.331") },				// byte[] 混じり
				// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// UNIQUEIDENTIFIER 配列への PreparedStatement.setArray() のテスト
	private void checkSetUniqueidentifierArray(Connection	c) throws Exception
	{
		String				columnName = "af_id";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"9819CE92-60F8-4d0c-BEEC-6CCA36EDD4A1",
					"161F520E-90BE-4465-9276-A3CA12338475",
					"hoge" };
				String[]	ans = {
					"9819CE92-60F8-4d0c-BEEC-6CCA36EDD4A1",
					"161F520E-90BE-4465-9276-A3CA12338475",
					"hoge                                " };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"5C3BF8C3-B1BE-420e-B0FF-15637B4C971F",
					new Integer(882),
					"8EEB89AB-40D9-42f3-8123-20858BED13FB" };
				String[]	ans = {
					"5C3BF8C3-B1BE-420e-B0FF-15637B4C971F",
					"882                                 ",
					"8EEB89AB-40D9-42f3-8123-20858BED13FB" };
				String[]	ans14 = {
					"5C3BF8C3-B1BE-420e-B0FF-15637B4C971F",
					"882",
					"8EEB89AB-40D9-42f3-8123-20858BED13FB" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					"EEDEFC55-770C-406e-A6EF-CBE153705893",
					"0C97D89E-D579-428a-8CEC-737015055A23",
					new Long(18739824234L),
					"C2FE4FE8-4394-4284-97D7-49203B2AB354" };
				String[]	ans = {
					"EEDEFC55-770C-406e-A6EF-CBE153705893",
					"0C97D89E-D579-428a-8CEC-737015055A23",
					"18739824234                         ",
					"C2FE4FE8-4394-4284-97D7-49203B2AB354" };
				String[]	ans14 = {
					"EEDEFC55-770C-406e-A6EF-CBE153705893",
					"0C97D89E-D579-428a-8CEC-737015055A23",
					"18739824234",
					"C2FE4FE8-4394-4284-97D7-49203B2AB354" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					"D33F5231-3332-42b7-844B-9D973DF868B7",
					"62988155-9CEF-4b76-B66D-6590AF5EF454",
					new Float(0.0829999968409538) };
				String[]	ans = {
					"D33F5231-3332-42b7-844B-9D973DF868B7",
					"62988155-9CEF-4b76-B66D-6590AF5EF454",
					"8.29999968409538E-2                 " };
				String[]	ans14 = {
					"D33F5231-3332-42b7-844B-9D973DF868B7",
					"62988155-9CEF-4b76-B66D-6590AF5EF454",
					"0.083000" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					"43C28144-EA22-405e-9319-2FEDCC6263F7",
					new Double(53872.8789),
					"0F36650A-0B12-4702-91BB-43874D6E52D3" };
				String[]	ans = {
					"43C28144-EA22-405e-9319-2FEDCC6263F7",
					"5.38728789E4                        ",
					"0F36650A-0B12-4702-91BB-43874D6E52D3" };
				String[]	ans14 = {
					"43C28144-EA22-405e-9319-2FEDCC6263F7",
					"53872.878900",
					"0F36650A-0B12-4702-91BB-43874D6E52D3" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					"C33E0CA8-A61F-4a3b-A8BE-EC36645BCED5",
					"6C8D00EF-404B-443a-AEF6-15BEB5C6479E",
					null };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から GUID に変換できないので例外 ClassCast が throw されるはず
				Object[]	ary = {
					"A59FCE50-463F-49b9-B52B-3EAA77532E8B",
					new byte[3] };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					"295955A5-DBF6-472f-9E67-D45439D5D66B",
					java.sql.Date.valueOf("2004-12-20") };
				String[]	ans = {
					"295955A5-DBF6-472f-9E67-D45439D5D66B",
					"2004-12-20                          " };
				String[]	ans14 = {
					"295955A5-DBF6-472f-9E67-D45439D5D66B",
					"2004-12-20" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					"8041094F-CBCF-4171-8AA3-59DF8E85EB02",
					"DE147EF4-5902-42be-BF08-F4F0473AAAB2",
					java.sql.Timestamp.valueOf("2004-12-20 11:54:41.739") };
				String[]	ans = {
					"8041094F-CBCF-4171-8AA3-59DF8E85EB02",
					"DE147EF4-5902-42be-BF08-F4F0473AAAB2",
					"2004-12-20 11:54:41.739             " };
				String[]	ans14 = {
					"8041094F-CBCF-4171-8AA3-59DF8E85EB02",
					"DE147EF4-5902-42be-BF08-F4F0473AAAB2",
					"2004-12-20 11:54:41.739" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = {
					new LanguageData("fr"),
					"02164D46-34FF-4f6f-80CF-48C707C2B527" };
				String[]	ans = {
					"fr                                  ",
					"02164D46-34FF-4f6f-80CF-48C707C2B527" };
				String[]	ans14 = {
					"fr",
					"02164D46-34FF-4f6f-80CF-48C707C2B527" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// IMAGE 配列への PreparedStatement.setArray() のテスト
	private void checkSetImageArray(Connection	c) throws Exception
	{
		String				columnName = "af_image";
		int					elementType = DataType.BINARY;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only byte[]
			{
				byte[]	elm1 = { 0x31, 0x58, 0x6E };
				byte[]	elm2 = { 0x0A, 0x38, 0x0D, 0x6F };
				byte[]	elm3 = { 0x00, 0x08, 0x3D };
				byte[][]	ary = { elm1, elm2, elm3 };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				byte[]	elm1 = { 0x47, 0x49, 0x6D };
				byte[]	elm2 = { 0x15, 0x2F };
				Object[]	ary = { elm1, "hogehoge", elm2 };
				assertSetArray(c, columnName, isTestArray, ary);
				byte[][]	ans = { elm1, stringToBytes("hogehoge"), elm2 };
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				byte[]	elm1 = { 0x1E, 0x27, 0x44, 0x1A };
				byte[]	elm2 = { 0x0A, 0x0E, 0x11, 0x5D, 0x5A };
				byte[][]	ary = { elm1, null, elm2 };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			byte[]	elm1 = { 0x00, 0x2A, 0x78, 0x6D };
			byte[]	elm2 = { 0x38, 0x44, 0x2F, 0x53, 0x0F };
			byte[]	elm3 = { 0x22, 0x6A };
			byte[]	elm4 = { 0x1A, 0x1A, 0x1A };
			byte[]	elm5 = { 0x46, 0x23, 0x70, 0x77 };
			Object[][]	arys = {
				{ elm1, new Integer(3), elm2 },										// Integer 混じり
				{ elm2, elm4, new Long(7019342701L), elm1 },						// Long 混じり
				{ elm3, elm2, new Float(43987.09) },								// Float 混じり
				{ elm5, elm1, new Double(0.0098787432), elm2 },						// Double 混じり
				{ elm2, java.sql.Date.valueOf("2004-12-20"), elm1 },				// java.sql.Date 混じり
				{ java.sql.Timestamp.valueOf("2004-12-20 13:17:05.469"), elm3 },	// java.sql.Timestamp 混じり
				{ elm1, elm3, new LanguageData("it"), elm2 }						// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) assertSetArray(c, columnName, isTestArray, arys[j], classCast);

			isTestArray = true;
		}
	}

	// LANGUAGE 配列への PreparedStatement.setArray() のテスト
	private void checkSetLanguageArray(Connection	c) throws Exception
	{
		String				columnName = "af_language";
		int					elementType = DataType.LANGUAGE;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	modLibraryError = new ModLibraryError(new ExceptionData());

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only jp.co.ricoh.doquedb.common.LanguageData
			{
				LanguageData[]	ary = { new LanguageData("en"), new LanguageData("ja"), new LanguageData("de"), new LanguageData("ko") };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				// ちゃんと言語を示す文字列混じり

				Object[]	ary = { new LanguageData("sw"), new LanguageData("gu"), "kn" };
				assertSetArray(c, columnName, isTestArray, ary);
				LanguageData[]	ans = { new LanguageData("sw"), new LanguageData("gu"), new LanguageData("kn") };
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// 言語になれない文字列混じり

				Object[]	ary = { new LanguageData("da"), "xx", new LanguageData("es") };
				assertSetArray(c, columnName, isTestArray, ary, invalidCharacter);
			}

			// null 混じり
			{
				LanguageData[]	ary = { new LanguageData("et"), null, new LanguageData("ky"), new LanguageData("pt") };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			Object[][]	arys = {
				{ new LanguageData("fr"), new LanguageData("zh"), new Integer(309) },										// Integer 混じり
				{ new LanguageData("no"), new Long(81387431078L), new LanguageData("yo") },									// Long 混じり
				{ new LanguageData("sv"), new Float(0.3987), new LanguageData("nl") },										// Float 混じり
				{ new LanguageData("ja"), new Double(539287.87432) },														// Double 混じり
				{ new LanguageData("ml"), new LanguageData("pl"), new byte[3] },											// byte[] 混じり
				{ new LanguageData("ro"), java.sql.Date.valueOf("2004-12-20") },											// java.sql.Date 混じり
				{ new LanguageData("sr"), java.sql.Timestamp.valueOf("2004-12-20 13:40:09.762"), new LanguageData("kn") }	// java.sql.Timestamp 混じり
			};
			for (int j = 0; j < arys.length; j++) {
				Object[]	ary = arys[j];
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			isTestArray = true;
		}
	}

	// NCHAR 配列への PreparedStatement.setArray() のテスト
	private void checkSetNcharArray(Connection	c) throws Exception
	{
		String				columnName = "af_nchar6";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = { "sql", "ｔｅｓｔ", "　" };
				String[]	ans = { "sql   ", "ｔｅｓｔ  ", "　     " };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			{
				// データ型のキャストは不要なので、最大長を越える部分は切られるはず
				String[]	ary = { "この文字列は途中で切られるはず", "koremo途中でちょん切られるはず" };
				String[]	ans = { "この文字列は", "koremo" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = { new Integer("498"), "134-00" };
				String[]	ans = { "498   ", "134-00" };
				String[]	ans14 = { "498", "134-00" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				// 文字列に変換しても文字列長が af_nchar6 の要素の最大長 (6) よりも短ければ文字列に変換されるはず

				Object[]	ary = { "049310", new Long(3878L), "63901" };
				String[]	ans = { "049310", "3878  ", "63901 " };
				String[]	ans14 = { "049310", "3878", "63901" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			// 文字列に変換すると文字列長が af_nchar6 の要素の最大長 (6) を超える場合には v15.0 以降では例外 StringRightTruncation が throw されるはず
			{
				Object[]	ary = { "実践", "sql", new Long(33019380138L), "プロ" };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// Float 混じり
			{
				// 文字列に変換しても文字列長が af_nchar6 の要素の最大長 (6) よりも短ければ文字列に変換されるはず

				Object[]	ary = { "hogeho", new Float(4E2), "0.5896" };
				String[]	ans = { "hogeho", "4E2   ", "0.5896" };
				String[]	ans14 = { "hogeho", "400.000000", "0.5896" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			// 文字列に変換すると文字列長が af_nchar6 の要素の最大長 (6) を超える場合には例外 StringRightTruncation が throw されるはず
			{
				Object[]	ary = { new Float(0.087843), "4837.6" };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// Double 混じり
			{
				// Float に同じ

				Object[]	ary = { new Double(0.1), "abcdef", "xyz!!!" };
				String[]	ans = { "1E-1  ", "abcdef", "xyz!!!" };
				String[]	ans14 = { "0.100000", "abcdef", "xyz!!!" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}
			// Float に同じ
			{
				Object[]	ary = { "333", "444", new Double(555.666) };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// null 混じり
			{
				String[]	ary = { "ｎｏｔ", "ｎｕｌｌ", null };
				String[]	ans = { "ｎｏｔ   ", "ｎｕｌｌ  ", null };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず
				Object[]	ary = { "string", new byte[5] };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			// 文字列に変換すると文字列長が af_nchar6 の要素の最大長 (6) を超えるので例外 StringRightTruncation が throw されるはず
			{
				Object[]	ary = { java.sql.Date.valueOf("2004-12-21"), "041221" };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// java.sql.Timestamp 混じり
			// java.sql.Date に同じ
			{
				Object[]	ary = { "101203", java.sql.Timestamp.valueOf("2004-12-21 10:12:03.548") };
				assertSetArray(c, columnName, isTestArray, ary, stringRightTruncation);
			}
			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = { "Tonga", new LanguageData("to"), "Polish", new LanguageData("pl") };
				String[]	ans = { "Tonga ", "to    ", "Polish", "pl    " };
				String[]	ans14 = { "Tonga", "to", "Polish", "pl" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// NVARCHAR 配列への PreparedStatement.setArray() のテスト
	private void checkSetNvarcharArray(Connection	c) throws Exception
	{
		String				columnName = "af_nvarchar256";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"public java.sql.ParameterMetaData getParameterMetaData() throws java.sql.SQLException",
					"public java.sql.ResultSet executeQuery(java.lang.String sql_) throws java.sql.SQLException" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"public int executeUpdate(java.lang.String sql_) throws java.sql.SQLException",
					new Integer(3),
					"指定された SQL 文を実行します。" };
				String[]	ans = {
					"public int executeUpdate(java.lang.String sql_) throws java.sql.SQLException",
					"3",
					"指定された SQL 文を実行します。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					"このボタンから始めます",
					"デスクトップ",
					new Long(2019583700083729L),
					"スタート" };
				String[]	ans = {
					"このボタンから始めます",
					"デスクトップ",
					"2019583700083729",
					"スタート" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					"public void close() throws java.sql.SQLException",
					"自動的にクローズされるときに java.sql.Statement オブジェクトのデータベースと JDBC リソースが解放されるのを待つのではなく、",
					new Float(987432.987) };
				String[]	ans = {
					"public void close() throws java.sql.SQLException",
					"自動的にクローズされるときに java.sql.Statement オブジェクトのデータベースと JDBC リソースが解放されるのを待つのではなく、",
					"9.87433E5" };
				String[]	ans14 = {
					"public void close() throws java.sql.SQLException",
					"自動的にクローズされるときに java.sql.Statement オブジェクトのデータベースと JDBC リソースが解放されるのを待つのではなく、",
					"987433.000000" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					"指定されたパラメータを SQL NULL に設定します。",
					new Double(9872934.8979832),
					"このオブジェクトへの参照がないと、ガベージコレクションによって判断されたときに、ガベージコレクタによって呼び出されます。" };
				String[]	ans = {
					"指定されたパラメータを SQL NULL に設定します。",
					"9.8729348979832E6",
					"このオブジェクトへの参照がないと、ガベージコレクションによって判断されたときに、ガベージコレクタによって呼び出されます。" };
				String[]	ans14 = {
					"指定されたパラメータを SQL NULL に設定します。",
					"9872934.897983",
					"このオブジェクトへの参照がないと、ガベージコレクションによって判断されたときに、ガベージコレクタによって呼び出されます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					"public void setObject(int parameterIndex_, java.lang.Object x_) throws java.sql.SQLException",
					"指定されたパラメータの値を、指定されたオブジェクトを使用して設定します",
					null,
					"parameterIndex_ - 最初のパラメータは 1、2 番目のパラメータは 2、などとする。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず

				Object[]	ary = {
					"指定されたパラメータを、指定されたバイト数を持つ指定された入力ストリームに設定します。",
					new byte[10] };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = { "2004/12/21", "２００４・１２・２１", java.sql.Date.valueOf("2004-12-21") };
				String[]	ans = { "2004/12/21", "２００４・１２・２１", "2004-12-21" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					"2004/12/21-10:44:52.439",
					"２００４・１２・２１−１０：４４：５２．４３９",
					java.sql.Timestamp.valueOf("2004-12-21 10:44:52.439") };
				String[]	ans = {
					"2004/12/21-10:44:52.439",
					"２００４・１２・２１−１０：４４：５２．４３９",
					"2004-12-21 10:44:52.439" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = { "Slovenian", new LanguageData("sl"), "Swedish", new LanguageData("sv") };
				String[]	ans = { "Slovenian", "sl", "Swedish", "sv" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// VARCHAR 配列への PreparedStatement.setArray() のテスト
	private void checkSetVarcharArray(Connection	c) throws Exception
	{
		String				columnName = "af_varchar128";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"addBatch, checkParameter, clearBatch, clearParameters, close, execute, executeBatch, executeQuery",
					"executeUpdate, finalize, getMetaData, getParameterMetaData, setArray, setAsciiStream, setBigDecimal",
					"setBinaryStream, setBlob, setBoolean, setByte, setBytes, setCharacterStream, setClob, setDate",
					"setDouble, setFloat, setInt, setLong, setNull, setObject, setParameter, setRef, setShort, setString",
					"setTime, setTimestamp, setUnicodeStream, setURL" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"getArray, getBigDecimal, getBlob, getBoolean, getByte, getBytes, getClob, getDate, getDouble",
					"getFloat, getInt, getLong, getObject, getRef, getShort, getString, getTime, getTimestamp, getURL",
					new Integer(901),
					"registerOutParameter, setAsciiStream, setBigDecimal, setBinaryStreawm, setBoolean, setByte, setBytes" };
				String[]	ans = {
					"getArray, getBigDecimal, getBlob, getBoolean, getByte, getBytes, getClob, getDate, getDouble",
					"getFloat, getInt, getLong, getObject, getRef, getShort, getString, getTime, getTimestamp, getURL",
					"901",
					"registerOutParameter, setAsciiStream, setBigDecimal, setBinaryStreawm, setBoolean, setByte, setBytes" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					"The class loader to use for accessing our XML parser.",
					new Long(43826810187L),
					"Parse the specified XML document, and return a TreeNode that corresponds to the root node of the document tree.",
					"Construct a logger that writes output to the servlet context log for the current web application." };
				String[]	ans = {
					"The class loader to use for accessing our XML parser.",
					"43826810187",
					"Parse the specified XML document, and return a TreeNode that corresponds to the root node of the document tree.",
					"Construct a logger that writes output to the servlet context log for the current web application." };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					new Float(0.390154987573624),
					"clearWarnings, close, commit, createStatement, finalize, getAutoCommit, getCatalog, getHoldability",
					"getMetaData, getSession, getTransactionIsolation, getTypeMap, getWarnings, isClosed, isReadOnly, nativeSQL",
					"prepareCall, prepareStatement, releaseSavepoint, rollback, setAutoCommit, setCatalog, setHoldability" };
				String[]	ans = {
					"3.90154987573624E-1",
					"clearWarnings, close, commit, createStatement, finalize, getAutoCommit, getCatalog, getHoldability",
					"getMetaData, getSession, getTransactionIsolation, getTypeMap, getWarnings, isClosed, isReadOnly, nativeSQL",
					"prepareCall, prepareStatement, releaseSavepoint, rollback, setAutoCommit, setCatalog, setHoldability" };
				String[]	ans14 = {
					"0.390155",
					"clearWarnings, close, commit, createStatement, finalize, getAutoCommit, getCatalog, getHoldability",
					"getMetaData, getSession, getTransactionIsolation, getTypeMap, getWarnings, isClosed, isReadOnly, nativeSQL",
					"prepareCall, prepareStatement, releaseSavepoint, rollback, setAutoCommit, setCatalog, setHoldability" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					"allProceduresAreCallable, allTablesAreSelectable, dataDefinitionCausesTransactionCommit",
					new Double(6.108767),
					"dataDefinitionIgnoredInTransactions, deletesAreDetected, doesMaxRowSizeIncludeBlobs, getAttributes",
					"getBestRowIdentifier, getCatalogs, getCatalogSeparator, getCatalogTerm, getColumnPrivileges, getColumns" };
				String[]	ans = {
					"allProceduresAreCallable, allTablesAreSelectable, dataDefinitionCausesTransactionCommit",
					"6.108767E0",
					"dataDefinitionIgnoredInTransactions, deletesAreDetected, doesMaxRowSizeIncludeBlobs, getAttributes",
					"getBestRowIdentifier, getCatalogs, getCatalogSeparator, getCatalogTerm, getColumnPrivileges, getColumns" };
				String[]	ans14 = {
					"allProceduresAreCallable, allTablesAreSelectable, dataDefinitionCausesTransactionCommit",
					"6.108767",
					"dataDefinitionIgnoredInTransactions, deletesAreDetected, doesMaxRowSizeIncludeBlobs, getAttributes",
					"getBestRowIdentifier, getCatalogs, getCatalogSeparator, getCatalogTerm, getColumnPrivileges, getColumns" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					"getConnection, getCrossReference, getDatabaseMajorVersion, getDatabaseMinorVersion, getDatabaseProductName",
					"getDatabaseProductVersion, getDefaultTransactionIsolation, getDriverMajorVersion, getDriverMinorVersion",
					null,
					null,
					"getDriverName, getDriverVersion, getExportedKeys, getExtraNameCharacters, getIdentifierQuoteString",
					"getImportedKeys, getIndexInfo, getJDBCMajorVersion, getJDBCMinorVersiongetMaxBinaryLiteralLength" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず

				Object[]	ary = {
					"getmaxCharLiteralLength, getMaxColumnNameLength, getMaxColumnsInGroupBy, getMaxColumnsInIndex, getMaxColumnsInOrderBy",
					"getMaxColumnsInSelect, getMaxColumnsInTable, getMaxConnections, getMaxCursorNameLength, getMaxIndexLength",
					"getMaxProcedureNameLength, getMaxRowSize, getMaxSchemaNameLength, getMaxStatementLength, getMaxStatements",
					new byte[53] };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					"getMaxTableNameLength, getMaxTablesInSelect, getMaxUserNameLength, getNumericFunctions, getPrimaryKeyInfo",
					java.sql.Date.valueOf("2004-12-21"),
					"getPrimaryKeyInfoColumnNames, getPrimaryKeys, getProcedureColumns, getProcedures, getProcedureTerm",
					"getResultSetHoldability, getSchemas, getSchemaTerm, getSearchStringEscape, getSQLKeywords, getSQLStateType" };
				String[]	ans = {
					"getMaxTableNameLength, getMaxTablesInSelect, getMaxUserNameLength, getNumericFunctions, getPrimaryKeyInfo",
					"2004-12-21",
					"getPrimaryKeyInfoColumnNames, getPrimaryKeys, getProcedureColumns, getProcedures, getProcedureTerm",
					"getResultSetHoldability, getSchemas, getSchemaTerm, getSearchStringEscape, getSQLKeywords, getSQLStateType" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					java.sql.Timestamp.valueOf("2004-12-21 11:47:31.006"),
					"getStringFunctions, getSuperTables, getSuperTypes, getSystemFunctions, getTablePrivileges, getTables",
					"getTimeDateFunctions, getTypeInfo, getURDs, getURL, getUserName, getVersionColumns, insertsAreDetected" };
				String[]	ans = {
					"2004-12-21 11:47:31.006",
					"getStringFunctions, getSuperTables, getSuperTypes, getSystemFunctions, getTablePrivileges, getTables",
					"getTimeDateFunctions, getTypeInfo, getURDs, getURL, getUserName, getVersionColumns, insertsAreDetected" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = {
					"isCatalogAtStart, isReadOnly, locatorsUpdateCopy, nullPlusNonNullIsNull, nullsAreSortedAtEnd",
					"nullsAreSortedAtStart, nullsAreSortedHigh, nullsAreSortedLow, othersDeletesAreVisible, othersInsertsAreVisible",
					"othersUpdatesAreVisible, ownDeletesAreVisible, ownInsertsAreVisible, ownUpdatesAreVisible",
					new LanguageData("en"),
					"storesLowerCaseIdentifiers, storesLowerCaseQuotedIdentifiers, storesMixedCaseIdentifiers" };
				String[]	ans = {
					"isCatalogAtStart, isReadOnly, locatorsUpdateCopy, nullPlusNonNullIsNull, nullsAreSortedAtEnd",
					"nullsAreSortedAtStart, nullsAreSortedHigh, nullsAreSortedLow, othersDeletesAreVisible, othersInsertsAreVisible",
					"othersUpdatesAreVisible, ownDeletesAreVisible, ownInsertsAreVisible, ownUpdatesAreVisible",
					"en",
					"storesLowerCaseIdentifiers, storesLowerCaseQuotedIdentifiers, storesMixedCaseIdentifiers" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// NTEXT 配列への PreparedStatement.setArray() のテスト
	private void checkSetNtextArray(Connection	c) throws Exception
	{
		String				columnName = "af_ntext";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();
		String				testDir = "test" + java.io.File.separator;
		String				charDataFile = testDir + "ascii_char.dat";
		String				largeCharDataFile = testDir + "ascii_large_char.dat";

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"XML 処理：XML 処理用の Java API が Java 2 プラットフォームに追加されました。 これにより、Java プラットフォーム API の標準的なセットで XML ドキュメントの基本処理を行うことができます。",
					"New I/O API：New I/O (NIO) API は、バッファ管理、文字セットのサポート、正規表現マッチング、ファイル入出力、およびスケーラブルなネットワーク入出力の分野で新しい機能を提供し、パフォーマンスを向上させます。",
					readString(charDataFile),
					"セキュリティ：・次の 2 つのセキュリティ機能が新しくなりました。①Java GSS-API は、アプリケーション間の通信に Kerberos V5 機構を使って、メッセージを安全に交換するための機能です。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"輸入管理制限があるため、J2SDK v1.4 に同梱された JCE 管轄ポリシーファイルは「強固」ですが、暗号化の使用には制限があります。 暗号化強度に対する制限がない「無制限」バージョンのファイルもあります。",
					"このリリースの JSSE 実装には、強固な暗号群が付属しています。 ただし、米国輸出規制により、このリリースではその他の「プラグ可能」SSL/TLS 実装を使用することはできません。 詳細については、「JSSE リファレンスガイド」を参照してください。",
					new Integer(509),
					"JAAS が J2SDK に統合されているので、java.security.Policy API はプリンシパルベースのクエリーを扱い、デフォルト Policy の実装はプリンシパルベースの grant エントリをサポートしています。 このように、どのコードが実行されているかだけではなく、どの「ユーザ」が実行しているかに基づいて、アクセス制御を実行できるようになりました。",
					"動的ポリシーへのサポートが追加されました。 バージョン 1.4 より前の J2SDK では、アクセス権とクラスはクラスのロード中にセキュリティポリシーを問い合わせることで静的にバインドされていました。 このバインドのライフタイムは、クラスローダのライフタイムの範囲内でした。 バージョン 1.4 では、このバインドはセキュリティチェックが必要になるまで延長されます。" };
				String[]	ans = {
					"輸入管理制限があるため、J2SDK v1.4 に同梱された JCE 管轄ポリシーファイルは「強固」ですが、暗号化の使用には制限があります。 暗号化強度に対する制限がない「無制限」バージョンのファイルもあります。",
					"このリリースの JSSE 実装には、強固な暗号群が付属しています。 ただし、米国輸出規制により、このリリースではその他の「プラグ可能」SSL/TLS 実装を使用することはできません。 詳細については、「JSSE リファレンスガイド」を参照してください。",
					"509",
					"JAAS が J2SDK に統合されているので、java.security.Policy API はプリンシパルベースのクエリーを扱い、デフォルト Policy の実装はプリンシパルベースの grant エントリをサポートしています。 このように、どのコードが実行されているかだけではなく、どの「ユーザ」が実行しているかに基づいて、アクセス制御を実行できるようになりました。",
					"動的ポリシーへのサポートが追加されました。 バージョン 1.4 より前の J2SDK では、アクセス権とクラスはクラスのロード中にセキュリティポリシーを問い合わせることで静的にバインドされていました。 このバインドのライフタイムは、クラスローダのライフタイムの範囲内でした。 バージョン 1.4 では、このバインドはセキュリティチェックが必要になるまで延長されます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					"データベースのストアドプロシージャを呼び出すための java.sql.CallableStatement オブジェクトを生成します。 java.sql.CallableStatement オブジェクトは、 その IN と OUT パラメータを設定するメソッドと ストアドプロシージャの呼び出しを実行するメソッドを提供します。",
					new Long(83922053719L),
					"指定された型と並行処理で java.sql.ResultSet オブジェクトを 生成する java.sql.Statement オブジェクトを生成します。 このメソッドは上記の createStatement メソッドと同じですが、 デフォルトの結果セットの型および並行処理をオーバーライドできます。",
					"このメソッドは上記の prepareStatement メソッドと 同じですが、デフォルトの結果セットの型、並行処理、および保持機能を オーバーライドできます。" };
				String[]	ans = {
					"データベースのストアドプロシージャを呼び出すための java.sql.CallableStatement オブジェクトを生成します。 java.sql.CallableStatement オブジェクトは、 その IN と OUT パラメータを設定するメソッドと ストアドプロシージャの呼び出しを実行するメソッドを提供します。",
					"83922053719",
					"指定された型と並行処理で java.sql.ResultSet オブジェクトを 生成する java.sql.Statement オブジェクトを生成します。 このメソッドは上記の createStatement メソッドと同じですが、 デフォルトの結果セットの型および並行処理をオーバーライドできます。",
					"このメソッドは上記の prepareStatement メソッドと 同じですが、デフォルトの結果セットの型、並行処理、および保持機能を オーバーライドできます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					"グラフィカルな Policy Tool ユーティリティが拡張され、指定したアクセス制御権をどのユーザに付与するかを示す Principal フィールドが指定できるようになりました。",
					new Float(439287),
					"Java 2D テクノロジ：Java 2D には、パフォーマンスの向上、オフスクリーンイメージ用ハードウェア高速化のサポート、プラグ可能イメージ入出力フレームワーク、新しい印刷サービス API、および新しいフォント機能など、数多くの新機能が含まれています。" };
				String[]	ans = {
					"グラフィカルな Policy Tool ユーティリティが拡張され、指定したアクセス制御権をどのユーザに付与するかを示す Principal フィールドが指定できるようになりました。",
					"4.39287E5",
					"Java 2D テクノロジ：Java 2D には、パフォーマンスの向上、オフスクリーンイメージ用ハードウェア高速化のサポート、プラグ可能イメージ入出力フレームワーク、新しい印刷サービス API、および新しいフォント機能など、数多くの新機能が含まれています。" };
				String[]	ans14 = {
					"グラフィカルな Policy Tool ユーティリティが拡張され、指定したアクセス制御権をどのユーザに付与するかを示す Principal フィールドが指定できるようになりました。",
					"439287.000000",
					"Java 2D テクノロジ：Java 2D には、パフォーマンスの向上、オフスクリーンイメージ用ハードウェア高速化のサポート、プラグ可能イメージ入出力フレームワーク、新しい印刷サービス API、および新しいフォント機能など、数多くの新機能が含まれています。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					new Double(0.0098743),
					"Image I/O Framework：Java Image I/O Framework は、ファイルに保存されネットワーク経由でアクセスされるイメージを扱うプラグイン可能なアーキテクチャを提供します。 この API は、イメージをロードしたり保存したりするための現行の API に比べて、柔軟性がかなり高く、機能も強力です。",
					readString(largeCharDataFile),
					"・印刷サービス機能に基づいて、印刷サービスを検出および選択する・印刷データの形式を指定する・印刷するドキュメントがサポートされるサービスに印刷ジョブを送信する" };
				String[]	ans = {
					"9.8743E-3",
					"Image I/O Framework：Java Image I/O Framework は、ファイルに保存されネットワーク経由でアクセスされるイメージを扱うプラグイン可能なアーキテクチャを提供します。 この API は、イメージをロードしたり保存したりするための現行の API に比べて、柔軟性がかなり高く、機能も強力です。",
					readString(largeCharDataFile),
					"・印刷サービス機能に基づいて、印刷サービスを検出および選択する・印刷データの形式を指定する・印刷するドキュメントがサポートされるサービスに印刷ジョブを送信する" };
				String[]	ans14 = {
					"0.009874",
					"Image I/O Framework：Java Image I/O Framework は、ファイルに保存されネットワーク経由でアクセスされるイメージを扱うプラグイン可能なアーキテクチャを提供します。 この API は、イメージをロードしたり保存したりするための現行の API に比べて、柔軟性がかなり高く、機能も強力です。",
					readString(largeCharDataFile),
					"・印刷サービス機能に基づいて、印刷サービスを検出および選択する・印刷データの形式を指定する・印刷するドキュメントがサポートされるサービスに印刷ジョブを送信する" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					"AWT：AWT パッケージの変更は、グラフィカルユーザインタフェースを表示するプログラムの堅牢さ、動作、およびパフォーマンスの向上に重点が置かれています。 これまでの実装は、新しい「フォーカスアーキテクチャ」に置き換わりました。",
					"ここではプラットフォームが異なるために生じるフォーカス関連のバグや、AWT コンポーネントと Swing コンポーネント間の非互換性について説明します。 新しい「フルスクリーン排他モード API」は、ウィンドウシステムを一時停止して画面に直接描画できる高性能グラフィックスをサポートします。",
					null,
					"ゲームやレンダリングを多用するアプリケーションでは便利な機能です。 ディスプレイ、キーボード、およびマウスがグラフィック環境でサポートされるかどうかを示す新しいグラフィック環境メソッドにより、「ヘッドレスサポート」が利用できるようになりました。",
					"フレームの表示方法を完全に制御する必要があるアプリケーションで、ネイティブなフレーム装飾を無効にする機能が利用できるようになりました。",
					"この機能を使うと、ネイティブなタイトルバー、システムメニュー、ボーダ、またはネイティブなオペレーティングシステムに依存する画面コンポーネントがレンダリングされないようにすることができます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず

				Object[]	ary = {
					new byte[103],
					readString(largeCharDataFile),
					"新しい「スピナ」コンポーネントは 1 行の入力フィールドで、ユーザは小さな上下の矢印ボタンを押して数字を変え、値を選択することができます。",
					"新しい「書式付きテキストフィールド」コンポーネントでは、日付、数字、および文字列を書式化することができます (十進数の通貨だけを入力できるテキストフィールドなど)。" };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					"「Windows の Look & Feel」の実装は、2000/98 バージョンの機能も利用できるように更新されました。",
					"新しい「ドラッグ＆ドロップアーキテクチャ」は、コンポーネント間のシームレスなドラッグ＆ドロップをサポートします。また、カスタマイズされた Swing コンポーネントにドラッグ & ドロップを簡単に実装することができます。",
					"これは、データモデルの詳細を記述するメソッドをいくつか書くだけで十分です。",
					java.sql.Date.valueOf("2004-12-21"),
					"Swing の進捗バーコンポーネントが拡張され、不確定状態をサポートするようになりました。" };
				String[]	ans = {
					"「Windows の Look & Feel」の実装は、2000/98 バージョンの機能も利用できるように更新されました。",
					"新しい「ドラッグ＆ドロップアーキテクチャ」は、コンポーネント間のシームレスなドラッグ＆ドロップをサポートします。また、カスタマイズされた Swing コンポーネントにドラッグ & ドロップを簡単に実装することができます。",
					"これは、データモデルの詳細を記述するメソッドをいくつか書くだけで十分です。",
					"2004-12-21",
					"Swing の進捗バーコンポーネントが拡張され、不確定状態をサポートするようになりました。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					"「不確定進捗バー」は、完了度を表すのではなく、一定のアニメーションを使って時間のかかる操作が発生していることを表します。",
					java.sql.Timestamp.valueOf("2004-12-21 18:25:59.870"),
					readString(largeCharDataFile),
					"この機能により、すべてのタブが 1 つのタブ並びに収まらない場合でも、タブ付きペインコンポーネントでは、複数のタブ並びではなく、単一のスクロール可能なタブ並びが表示されます。 「ポップアップとポップアップファクトリ」の各クラスは、以前は package private でしたが、このバージョンでは public になったため、プログラマは独自のポップアップを作成できます。" };
				String[]	ans = {
					"「不確定進捗バー」は、完了度を表すのではなく、一定のアニメーションを使って時間のかかる操作が発生していることを表します。",
					"2004-12-21 18:25:59.870",
					readString(largeCharDataFile),
					"この機能により、すべてのタブが 1 つのタブ並びに収まらない場合でも、タブ付きペインコンポーネントでは、複数のタブ並びではなく、単一のスクロール可能なタブ並びが表示されます。 「ポップアップとポップアップファクトリ」の各クラスは、以前は package private でしたが、このバージョンでは public になったため、プログラマは独自のポップアップを作成できます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = {
					"Accessibility：新しい機能には、次のサポートが含まれています。",
					"JTabbedPane でのニーモニックタブナビゲーション",
					readString(largeCharDataFile),
					new LanguageData("fr"),
					"HTML コンポーネントのユーザ補助機能",
					"Swing アクションのユーザ補助機能",
					"リスト項目の頭文字を使ったリストナビゲーション",
					"コンポーネントロール DATE_EDITOR、FONT_CHOOSER、GROUP_BOX、SPIN_BOX、STATUS_BAR",
					"画面拡大機能や画面読み取り機能の存在を示すプロパティ、または Java 仮想マシンにロードされる補助テクノロジを指定するプロパティ" };
				String[]	ans = {
					"Accessibility：新しい機能には、次のサポートが含まれています。",
					"JTabbedPane でのニーモニックタブナビゲーション",
					readString(largeCharDataFile),
					"fr",
					"HTML コンポーネントのユーザ補助機能",
					"Swing アクションのユーザ補助機能",
					"リスト項目の頭文字を使ったリストナビゲーション",
					"コンポーネントロール DATE_EDITOR、FONT_CHOOSER、GROUP_BOX、SPIN_BOX、STATUS_BAR",
					"画面拡大機能や画面読み取り機能の存在を示すプロパティ、または Java 仮想マシンにロードされる補助テクノロジを指定するプロパティ" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}


			isTestArray = true;
		}
	}

	// NTEXT(compressed) 配列への PreparedStatement.setArray() のテスト
	private void checkSetNtextCompressedArray(Connection	c) throws Exception
	{
		String				columnName = "af_ntext_compressed";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();
		String				testDir = "test" + java.io.File.separator;
		String				charDataFile = testDir + "ascii_char.dat";
		String				largeCharDataFile = testDir + "ascii_large_char.dat";

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"Java プログラムには、単一のグローバルロケールは割り当てられていません。ロケールに依存するすべての操作には、引数として明示的にロケールを渡すことができます。これにより、多言語プログラムが非常に簡単になります。グローバルロケールが強制的に使われることはありませんが、ロケールを明示的に管理する必要のないプログラムでは、デフォルトのロケールを利用できます。デフォルトロケールを使うと、全体的な表示形式を一度の選択で変えることができます。",
					"Java のロケールは、別のオブジェクトから出された、ある動作に対する要求として機能します。たとえば、カレンダオブジェクトにフランス語系カナダ用ロケールを渡すと、ケベック州の慣習に従って正しく動作するようカレンダに要求したことになります。ロケールを受け取ったオブジェクトは、適切な処理を行わなければなりません。オブジェクトが特定のロケールに対して地域対応されていない場合、オブジェクトは、地域対応されているロケールでよく似たものを探します。",
					"Locale オブジェクトは、地理的、政治的、または文化的に特定の地域を表しています。動作するためにロケールを必要とする操作のことを、ロケール依存操作と呼びます。ロケール依存操作は、Locale オブジェクトを使って、ユーザに合うように情報を加工します。たとえば、数値の表示はロケールに依存する操作です。数値の書式は、ユーザに固有の国、地域、文化などの慣習やきまりに従って設定する必要があります。",
					"すべてのロケール依存クラスは、サポートするロケール用にカスタマイズされたリソースにアクセスできなければなりません。地域対応の処理を容易にするには、ロケールごとにリソースをグループ化し、プログラムのロケールに依存しない部分から切り離すことが有用です。",
					"Java プラットフォームでは、クラスごとに独自の地域対応版が保持されるので、サポートされるロケールの集合は 1 種類だけでなくてもかまいません。ただし、Java 2 プラットフォームのクラスとしては、一貫性のある地域対応をサポートしています。Java プラットフォームのほかの実装では、異なるロケールをサポートする場合があります。Java 2 SDK がサポートするロケールの一覧は、「サポートされているロケール」を参照してください。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"ResourceBundle クラスは、リソースのコンテナを表す抽象基底クラスです。プログラムでは、特定のロケールのためのリソースを含む ResourceBundle のサブクラスを作成します。使用するコードを変更しなくても、ResourceBundle のインスタンスに新しいリソースを追加したり、ResourceBundle の新しいインスタンスをシステムに追加したりできます。リソースをクラスとしてパッケージ化することで、Java のクラスローディング機構を利用してリソースを探すことができます。",
					"リソースのバンドルには、ロケール固有のオブジェクトが含まれています。String オブジェクトなど、ロケール固有のリソースが必要な場合は、現在のユーザのロケールに適したリソースバンドルからリソースをロードします。このようにして、リソースのバンドルに含まれているロケール固有の情報のすべて、または少なくとも大部分を切り離し、ユーザのロケールとの依存関係が弱いコードを作成できます。",
					"JDK ソフトウェアの Version 1.0 では、日付と時刻を表すために java.util.Date クラスが導入されました。java.util.Date クラスを使うと、年、月、日、時、分、秒の値として日付を解釈でき、日付の文字列の書式指定と構文解析が可能でした。しかし、これらの機能の API は国際化の対象にはなりませんでした。このクラスの「表現」部分だけが、JDK ソフトウェアの Version 1.1 に引き継がれています。",
					new Integer(3071),
					"JDK ソフトウェア 1.1 のリリースでは、Date クラスは、日付または時刻のラッパーとしてだけ使う必要があります。つまり、Date オブジェクトは、ミリ秒の単位で時刻の具体的なインスタンスを表します。代わりに、日付と時刻のフィールドの間の変換には Calendar クラスを、また日付の文字列の書式指定と構文解析には DateFormat クラスを、それぞれ使う必要があります。JDK ソフトウェアの Version 1.0 で使われていた Date クラスの対応するメソッドは、推奨されなくなっています。" };
				String[]	ans = {
					"ResourceBundle クラスは、リソースのコンテナを表す抽象基底クラスです。プログラムでは、特定のロケールのためのリソースを含む ResourceBundle のサブクラスを作成します。使用するコードを変更しなくても、ResourceBundle のインスタンスに新しいリソースを追加したり、ResourceBundle の新しいインスタンスをシステムに追加したりできます。リソースをクラスとしてパッケージ化することで、Java のクラスローディング機構を利用してリソースを探すことができます。",
					"リソースのバンドルには、ロケール固有のオブジェクトが含まれています。String オブジェクトなど、ロケール固有のリソースが必要な場合は、現在のユーザのロケールに適したリソースバンドルからリソースをロードします。このようにして、リソースのバンドルに含まれているロケール固有の情報のすべて、または少なくとも大部分を切り離し、ユーザのロケールとの依存関係が弱いコードを作成できます。",
					"JDK ソフトウェアの Version 1.0 では、日付と時刻を表すために java.util.Date クラスが導入されました。java.util.Date クラスを使うと、年、月、日、時、分、秒の値として日付を解釈でき、日付の文字列の書式指定と構文解析が可能でした。しかし、これらの機能の API は国際化の対象にはなりませんでした。このクラスの「表現」部分だけが、JDK ソフトウェアの Version 1.1 に引き継がれています。",
					"3071",
					"JDK ソフトウェア 1.1 のリリースでは、Date クラスは、日付または時刻のラッパーとしてだけ使う必要があります。つまり、Date オブジェクトは、ミリ秒の単位で時刻の具体的なインスタンスを表します。代わりに、日付と時刻のフィールドの間の変換には Calendar クラスを、また日付の文字列の書式指定と構文解析には DateFormat クラスを、それぞれ使う必要があります。JDK ソフトウェアの Version 1.0 で使われていた Date クラスの対応するメソッドは、推奨されなくなっています。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					"readObject public void readObject(jp.co.ricoh.doquedb.common.InputStream input_) throws java.io.IOExceptionストリームから読み込む 定義: インタフェース Serializable 内の readObject オーバーライド: クラス StringData 内の readObject パラメータ: input_ - 入力用のストリーム 例外: java.io.IOException - 入出力関係の例外が発生した 関連項目: readObject",
					"ReadOnlyTransaction public ReadOnlyTransaction(jp.co.ricoh.doquedb.common.ExceptionData e_) 新たに例外オブジェクトを作成する。",
					new Long(-391675819873L) };
				String[]	ans = {
					"readObject public void readObject(jp.co.ricoh.doquedb.common.InputStream input_) throws java.io.IOExceptionストリームから読み込む 定義: インタフェース Serializable 内の readObject オーバーライド: クラス StringData 内の readObject パラメータ: input_ - 入力用のストリーム 例外: java.io.IOException - 入出力関係の例外が発生した 関連項目: readObject",
					"ReadOnlyTransaction public ReadOnlyTransaction(jp.co.ricoh.doquedb.common.ExceptionData e_) 新たに例外オブジェクトを作成する。",
					"-391675819873" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					"Java ロギング API は java.util.logging パッケージの形で導入され、エンドユーザ、システム管理者、フィールドサービスエンジニア、およびソフトウェアの開発チームが分析するためのログレポートを作成し、顧客サイトでのソフトウェアのサービスや保守を簡単にできるようにするものです。 Logging API では、アプリケーションまたはプラットフォームで発生したセキュリティの障害、構成エラー、パフォーマンスのボトルネック、バグなどの情報を取り込みます。",
					new Float(43892.0),
					readString(largeCharDataFile),
					"主要パッケージでは、プレーンテキストまたは XML 形式のログレコードをメモリ、出力ストリーム、コンソール、ファイル、およびソケットに配信します。 Logging API には、ホストオペレーティングシステム上の既存のログサービスと対話する機能があります。" };
				String[]	ans = {
					"Java ロギング API は java.util.logging パッケージの形で導入され、エンドユーザ、システム管理者、フィールドサービスエンジニア、およびソフトウェアの開発チームが分析するためのログレポートを作成し、顧客サイトでのソフトウェアのサービスや保守を簡単にできるようにするものです。 Logging API では、アプリケーションまたはプラットフォームで発生したセキュリティの障害、構成エラー、パフォーマンスのボトルネック、バグなどの情報を取り込みます。",
					"4.3892E4",
					readString(largeCharDataFile),
					"主要パッケージでは、プレーンテキストまたは XML 形式のログレコードをメモリ、出力ストリーム、コンソール、ファイル、およびソケットに配信します。 Logging API には、ホストオペレーティングシステム上の既存のログサービスと対話する機能があります。" };
				String[]	ans14 = {
					"Java ロギング API は java.util.logging パッケージの形で導入され、エンドユーザ、システム管理者、フィールドサービスエンジニア、およびソフトウェアの開発チームが分析するためのログレポートを作成し、顧客サイトでのソフトウェアのサービスや保守を簡単にできるようにするものです。 Logging API では、アプリケーションまたはプラットフォームで発生したセキュリティの障害、構成エラー、パフォーマンスのボトルネック、バグなどの情報を取り込みます。",
					"43892.000000",
					readString(largeCharDataFile),
					"主要パッケージでは、プレーンテキストまたは XML 形式のログレコードをメモリ、出力ストリーム、コンソール、ファイル、およびソケットに配信します。 Logging API には、ホストオペレーティングシステム上の既存のログサービスと対話する機能があります。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					readString(largeCharDataFile),
					new Double(43297.987882),
					"java.math パッケージでは、任意精度の数値演算がサポートされます。このパッケージには、次の 2 つのクラスがあります。BigInteger の数値は、固定の任意精度の整数型です。BigInteger クラスは、Java 言語のプリミティブ整数演算子すべてと同等のメソッドを提供します。さらに、このクラスでは、モジュラー算術演算、GCD 計算、素数の生成と判定、単一ビット操作、およびその他の演算もサポートされます。",
					"BigDecimal の数値は、固定の任意精度の符号付き 10 進数で、金融計算に適しています。BigDecimal クラスでは、基本算術演算、スケール操作、比較、フォーマット変換、およびハッシングの操作がサポートされます。",
					"リフレクションによって Java コードは、ロードしたクラスのフィールド、メソッド、およびコンストラクタに関する情報を検出し、リフレクトされたフィールド、メソッド、およびコンストラクタを使って、オブジェクト上でこれらの基にある対応部分を操作することが、セキュリティの制約のもとで可能になります。",
					readString(charDataFile) };
				String[]	ans = {
					readString(largeCharDataFile),
					"4.3297987882E4",
					"java.math パッケージでは、任意精度の数値演算がサポートされます。このパッケージには、次の 2 つのクラスがあります。BigInteger の数値は、固定の任意精度の整数型です。BigInteger クラスは、Java 言語のプリミティブ整数演算子すべてと同等のメソッドを提供します。さらに、このクラスでは、モジュラー算術演算、GCD 計算、素数の生成と判定、単一ビット操作、およびその他の演算もサポートされます。",
					"BigDecimal の数値は、固定の任意精度の符号付き 10 進数で、金融計算に適しています。BigDecimal クラスでは、基本算術演算、スケール操作、比較、フォーマット変換、およびハッシングの操作がサポートされます。",
					"リフレクションによって Java コードは、ロードしたクラスのフィールド、メソッド、およびコンストラクタに関する情報を検出し、リフレクトされたフィールド、メソッド、およびコンストラクタを使って、オブジェクト上でこれらの基にある対応部分を操作することが、セキュリティの制約のもとで可能になります。",
					readString(charDataFile) };
				String[]	ans14 = {
					readString(largeCharDataFile),
					"43297.987882",
					"java.math パッケージでは、任意精度の数値演算がサポートされます。このパッケージには、次の 2 つのクラスがあります。BigInteger の数値は、固定の任意精度の整数型です。BigInteger クラスは、Java 言語のプリミティブ整数演算子すべてと同等のメソッドを提供します。さらに、このクラスでは、モジュラー算術演算、GCD 計算、素数の生成と判定、単一ビット操作、およびその他の演算もサポートされます。",
					"BigDecimal の数値は、固定の任意精度の符号付き 10 進数で、金融計算に適しています。BigDecimal クラスでは、基本算術演算、スケール操作、比較、フォーマット変換、およびハッシングの操作がサポートされます。",
					"リフレクションによって Java コードは、ロードしたクラスのフィールド、メソッド、およびコンストラクタに関する情報を検出し、リフレクトされたフィールド、メソッド、およびコンストラクタを使って、オブジェクト上でこれらの基にある対応部分を操作することが、セキュリティの制約のもとで可能になります。",
					readString(charDataFile) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					"Java 2 プラットフォームでは、デフォルトのリフレクトアクセス制御をプログラムで抑制することができます。",
					"Java Core Reflection API の高度なクライアントの中には、リフレクトされたメンバおよびコンストラクタの「使用中」に、デフォルトの Java 言語のアクセス制御検査を抑止する手段を必要とするものがあります。public、デフォルト (パッケージ) アクセス、protected、および private メンバの検査は、フィールド、メソッド、またはコンストラクタがフィールドの設定または取得、メソッドの呼び出し、またはクラスの新しいインスタンスの生成に使用される時に実行されます。",
					null,
					readString(largeCharDataFile) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 ClassCast が throw されるはず

				Object[]	ary = {
					readString(largeCharDataFile),
					readString(charDataFile),
					"このようなクライアントの例として、Java オブジェクト直列化サービス、オブジェクトインスペクタおよびデバッガなどの開発ツール、データベース持続機構などのアプリケーションがあります。",
					new byte[50] };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					java.sql.Date.valueOf("2004-12-22"),
					"Java 1.2 では、フィールド、メソッド、またはコンストラクタオブジェクトは、デフォルトの Java 言語アクセス制御を抑制するよう明示的にフラグがたてられています。リフレクトされたオブジェクトを使用する場合、このフラグ (新しいインスタンスフィールド) をアクセス検査の一部として調べます。",
					"フラグが true の場合、アクセス検査は無効にされ、要求された操作が続行されます。false の場合には、Java 1.1 で行われたような標準アクセス検査が実行されます。リフレクトされたメンバまたはコンストラクタでは、デフォルトのフラグは false に設定されています。" };
				String[]	ans = {
					"2004-12-22",
					"Java 1.2 では、フィールド、メソッド、またはコンストラクタオブジェクトは、デフォルトの Java 言語アクセス制御を抑制するよう明示的にフラグがたてられています。リフレクトされたオブジェクトを使用する場合、このフラグ (新しいインスタンスフィールド) をアクセス検査の一部として調べます。",
					"フラグが true の場合、アクセス検査は無効にされ、要求された操作が続行されます。false の場合には、Java 1.1 で行われたような標準アクセス検査が実行されます。リフレクトされたメンバまたはコンストラクタでは、デフォルトのフラグは false に設定されています。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					"フラグ設定は、新しい Java 1.2 のセキュリティ機構によりセキュリティチェックが行われます。この操作を実行するには、クライアントのコンテキスト呼び出しに十分な特権が与えられている必要があります。",
					readString(charDataFile),
					java.sql.Timestamp.valueOf("2004-12-22 09:35:18.306"),
					"このため、Java 1.2 では、実際にリフレクトされたクラス (フィールド、メソッド、およびコンストラクタ) は、次で説明する新しい基底クラスである AccessibleObject クラスを継承します。このクラスは、リフレクトされたオブジェクトに accessible フラグを設定および取得するために必要な状態およびメソッドを提供します。次で説明する新しい ReflectPermission クラスは、セキュリティポリシーファイルを使用して必要な権限を与える能力を提供します。" };
				String[]	ans = {
					"フラグ設定は、新しい Java 1.2 のセキュリティ機構によりセキュリティチェックが行われます。この操作を実行するには、クライアントのコンテキスト呼び出しに十分な特権が与えられている必要があります。",
					readString(charDataFile),
					"2004-12-22 09:35:18.306",
					"このため、Java 1.2 では、実際にリフレクトされたクラス (フィールド、メソッド、およびコンストラクタ) は、次で説明する新しい基底クラスである AccessibleObject クラスを継承します。このクラスは、リフレクトされたオブジェクトに accessible フラグを設定および取得するために必要な状態およびメソッドを提供します。次で説明する新しい ReflectPermission クラスは、セキュリティポリシーファイルを使用して必要な権限を与える能力を提供します。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = {
					"Core Reflection API を使うと、次のような 2 種類のアプリケーションを作成できるようになります。",
					new LanguageData("ik"),
					"1 つは、実行時のクラスに基づくターゲットオブジェクトの、すべての public メンバを見つけ出して使う必要があるアプリケーションの集合です。これらのアプリケーションは、オブジェクトのすべての public フィールド、メソッド、およびコンストラクタに実行時にアクセスする必要があります。このカテゴリに入るアプリケーションには、Java Beans[1]、あるいはオブジェクトインスペクタなどの簡易ツールがあります。" };
				String[]	ans = {
					"Core Reflection API を使うと、次のような 2 種類のアプリケーションを作成できるようになります。",
					"ik",
					"1 つは、実行時のクラスに基づくターゲットオブジェクトの、すべての public メンバを見つけ出して使う必要があるアプリケーションの集合です。これらのアプリケーションは、オブジェクトのすべての public フィールド、メソッド、およびコンストラクタに実行時にアクセスする必要があります。このカテゴリに入るアプリケーションには、Java Beans[1]、あるいはオブジェクトインスペクタなどの簡易ツールがあります。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// FULLTEXT 配列への PreparedStatement.setArray() のテスト
	private void checkSetFulltextArray(Connection	c) throws Exception
	{
		String				columnName = "af_fulltext";
		int					elementType = DataType.STRING;
		java.util.Vector	anss = new java.util.Vector();
		String				testDir = "test" + java.io.File.separator;
		String				charDataFile = testDir + "utf8_char.dat";
		String				largeCharDataFile = testDir + "utf8_large_char.dat";

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only String
			{
				String[]	ary = {
					"全世界に広がるインターネットでは、世界中どこでも使えるソフトウェアが必要になります。つまり、ユーザの国や言語を意識しないで開発でき、さまざまな国や地域に合わせて地域対応できるソフトウェアです。",
					"Java 2 プラットフォームでは、グローバルアプリケーションを開発するための豊富な API 群が提供されています。",
					"このような国際化のための API は、Unicode 2.1 の文字エンコーディングに基づいており、テキスト、数値、日付、通貨、およびユーザ定義のオブジェクトを任意の国の慣習に合わせる機能を備えています。",
					readUTF8(largeCharDataFile),
					"このドキュメントでは、Java 2 プラットフォームの国際化に関する API と機能の概要を説明します。コーディング例と詳しい手順の説明については、『Java チュートリアル』を参照してください。API の詳細については、Java プラットフォーム API 仕様を参照してください。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Integer 混じり
			{
				Object[]	ary = {
					"もう 1 つは、特定のクラスが宣言したメンバを見つけ出して使う必要のある、複雑なアプリケーションの集合です。これらのアプリケーションは、class ファイルが指定したレベルのクラス実装への実行時アクセスが必要です。このカテゴリに入るアプリケーションには、インタプリタ、インスペクタ、クラスブラウザなどの開発ツールや、Java オブジェクト直列化 [2] などの実行サービスがあります。",
					new Integer(36),
					readUTF8(charDataFile),
					readUTF8(largeCharDataFile),
					"これらのアプリケーションは、クラス Class のメソッド getDeclaredField、getDeclaredMethod、getDeclaredConstructor、getDeclaredFields、getDeclaredMethods、および getDeclaredConstructors から取得したクラス Field、Method、および Constructor のインスタンスを使います。" };
				String[]	ans = {
					"もう 1 つは、特定のクラスが宣言したメンバを見つけ出して使う必要のある、複雑なアプリケーションの集合です。これらのアプリケーションは、class ファイルが指定したレベルのクラス実装への実行時アクセスが必要です。このカテゴリに入るアプリケーションには、インタプリタ、インスペクタ、クラスブラウザなどの開発ツールや、Java オブジェクト直列化 [2] などの実行サービスがあります。",
					"36",
					readUTF8(charDataFile),
					readUTF8(largeCharDataFile),
					"これらのアプリケーションは、クラス Class のメソッド getDeclaredField、getDeclaredMethod、getDeclaredConstructor、getDeclaredFields、getDeclaredMethods、および getDeclaredConstructors から取得したクラス Field、Method、および Constructor のインスタンスを使います。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Long 混じり
			{
				Object[]	ary = {
					readUTF8(largeCharDataFile),
					"SystemCall public SystemCall(java.lang.String arg0_, int arg1_) 新たに例外オブジェクトを作成する。 -------------------------------------------------------------------------------- SystemCall public SystemCall(jp.co.ricoh.doquedb.common.ExceptionData e_) 新たに例外オブジェクトを作成する。",
					"データ型をあらわすクラスを新たに作成する",
					new Long(43297163820L) };
				String[]	ans = {
					readUTF8(largeCharDataFile),
					"SystemCall public SystemCall(java.lang.String arg0_, int arg1_) 新たに例外オブジェクトを作成する。 -------------------------------------------------------------------------------- SystemCall public SystemCall(jp.co.ricoh.doquedb.common.ExceptionData e_) 新たに例外オブジェクトを作成する。",
					"データ型をあらわすクラスを新たに作成する",
					"43297163820" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Float 混じり
			{
				Object[]	ary = {
					"Method オブジェクトは、基本フィールドを反映したメソッドを表します。基本メソッドは、abstract メソッド、インスタンスメソッド、クラス (static) メソッドのどれでも構いません。",
					"クラス Method のメソッドを使って、基本メソッドの仮パラメータの型、戻り値の型、および確認済み例外の型を取得します。また、クラス Method の invoke メソッドを使って、ターゲットオブジェクトの基本メソッドを呼び出します。インスタンスおよび abstract メソッドの呼び出しでは、ターゲットオブジェクトの実行クラスおよびリフレクトされたメソッドの宣言クラス、名前、および仮パラメータの型に基づいた動的なメソッドの解決を使います。",
					"このため、インタフェースを実装するクラスのインスタンスであるオブジェクトについて、リフレクトされたインタフェースメソッドを呼び出すことができます。メソッド呼び出しでは、メソッドの宣言クラスの基本 static メソッドを使います。",
					new Float(3),
					readUTF8(charDataFile) };
				String[]	ans = {
					"Method オブジェクトは、基本フィールドを反映したメソッドを表します。基本メソッドは、abstract メソッド、インスタンスメソッド、クラス (static) メソッドのどれでも構いません。",
					"クラス Method のメソッドを使って、基本メソッドの仮パラメータの型、戻り値の型、および確認済み例外の型を取得します。また、クラス Method の invoke メソッドを使って、ターゲットオブジェクトの基本メソッドを呼び出します。インスタンスおよび abstract メソッドの呼び出しでは、ターゲットオブジェクトの実行クラスおよびリフレクトされたメソッドの宣言クラス、名前、および仮パラメータの型に基づいた動的なメソッドの解決を使います。",
					"このため、インタフェースを実装するクラスのインスタンスであるオブジェクトについて、リフレクトされたインタフェースメソッドを呼び出すことができます。メソッド呼び出しでは、メソッドの宣言クラスの基本 static メソッドを使います。",
					"3E0",
					readUTF8(charDataFile) };
				String[]	ans14 = {
					"Method オブジェクトは、基本フィールドを反映したメソッドを表します。基本メソッドは、abstract メソッド、インスタンスメソッド、クラス (static) メソッドのどれでも構いません。",
					"クラス Method のメソッドを使って、基本メソッドの仮パラメータの型、戻り値の型、および確認済み例外の型を取得します。また、クラス Method の invoke メソッドを使って、ターゲットオブジェクトの基本メソッドを呼び出します。インスタンスおよび abstract メソッドの呼び出しでは、ターゲットオブジェクトの実行クラスおよびリフレクトされたメソッドの宣言クラス、名前、および仮パラメータの型に基づいた動的なメソッドの解決を使います。",
					"このため、インタフェースを実装するクラスのインスタンスであるオブジェクトについて、リフレクトされたインタフェースメソッドを呼び出すことができます。メソッド呼び出しでは、メソッドの宣言クラスの基本 static メソッドを使います。",
					"3.000000",
					readUTF8(charDataFile) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// Double 混じり
			{
				Object[]	ary = {
					new Double(0.00009249071),
					readUTF8(largeCharDataFile),
					"Constructor オブジェクトは、リフレクトされたコンストラクタを表します。クラス Constructor のメソッドを使って、基本コンストラクタの仮パラメータ型と確認済みの例外の型を取得します。さらに、クラスがインスタンスを実行できる場合は、クラス Constructor の newInstance メソッドを使って、コンストラクタを宣言するクラスの新規インスタンスを生成して初期化します。",
					"Array クラスはインスタンスを生成できないクラスです。 クラスメソッドをエクスポートして、プリミティブ型またはクラス型コンポーネントを持つ Java 配列を生成します。クラス Array のメソッドを使って、配列を構成する値の取得と設定も行います。" };
				String[]	ans = {
					"9.249071E-5",
					readUTF8(largeCharDataFile),
					"Constructor オブジェクトは、リフレクトされたコンストラクタを表します。クラス Constructor のメソッドを使って、基本コンストラクタの仮パラメータ型と確認済みの例外の型を取得します。さらに、クラスがインスタンスを実行できる場合は、クラス Constructor の newInstance メソッドを使って、コンストラクタを宣言するクラスの新規インスタンスを生成して初期化します。",
					"Array クラスはインスタンスを生成できないクラスです。 クラスメソッドをエクスポートして、プリミティブ型またはクラス型コンポーネントを持つ Java 配列を生成します。クラス Array のメソッドを使って、配列を構成する値の取得と設定も行います。" };
				String[]	ans14 = {
					"0.000092",
					readUTF8(largeCharDataFile),
					"Constructor オブジェクトは、リフレクトされたコンストラクタを表します。クラス Constructor のメソッドを使って、基本コンストラクタの仮パラメータ型と確認済みの例外の型を取得します。さらに、クラスがインスタンスを実行できる場合は、クラス Constructor の newInstance メソッドを使って、コンストラクタを宣言するクラスの新規インスタンスを生成して初期化します。",
					"Array クラスはインスタンスを生成できないクラスです。 クラスメソッドをエクスポートして、プリミティブ型またはクラス型コンポーネントを持つ Java 配列を生成します。クラス Array のメソッドを使って、配列を構成する値の取得と設定も行います。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				String[]	ary = {
					readUTF8(largeCharDataFile),
					"Modifier クラスはインスタンスを生成できないクラスです。 クラスメソッドをエクスポートして、クラスやメンバの Java 言語修飾子を復号化します。言語修飾子は、Java 仮想マシン仕様で定義されたコード化定数を使って整数型に符号化します。",
					"Class オブジェクトが 9 つあります。 これらのオブジェクトを使って、8 つのプリミティブ Java 型と void を実行時に表現します。(これらは Class オブジェクトであって、クラスではないことに注意してください。)Core Reflection API はこれらの Class オブジェクトを使って、次のものを識別します。",
					null,
					"システムマネージャがメンバに対する最初のリフレクティブなアクセスを取得すると任意のコードを使ってリフレクトされたメンバの識別情報を問い合わせることができる。ただし、protected、デフォルトの (パッケージ) アクセス、および private クラスとメンバについては、個々のリフレクトされたメンバを使ってオブジェクトの基本メンバを操作する (つまり、フィールド値の取得や設定、メソッドの呼び出し、新規オブジェクトの生成と初期化を行う) ときに、標準の Java 言語アクセス制御チェックが実行される。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ary);
				assertGetArray(c, columnName, anss, elementType);
			}

			// byte[] 混じり
			{
				// バイト列から文字列に変換できないので例外 Class Cast が throw されるはず

				Object[]	ary = {
					"アプリケーションに関する Java 言語セキュリティポリシーは、任意のコードが、リンクする先の任意のクラスのすべてのメンバおよびコンストラクタ (public でないメンバとコンストラクタを含む) への反映的なアクセスを取得できるということです。",
					new byte[1],
					"デフォルトでは、メンバまたはコンストラクタへの反映的なアクセスを取得するアプリケーションコードは、標準の Java 言語アクセス制御とともにリフレクトされたメンバまたはコンストラクタを使うことだけができます。",
					"標準ポリシーは、リフレクトされたメンバの setAccessible メソッドを呼び出すとオーバーライドされます。そして、setAccessible メソッドの呼び出しは、アクセス権 ReflectPermission の suppressAccessChecks ターゲットによって制御されます。",
					readUTF8(largeCharDataFile) };
				assertSetArray(c, columnName, isTestArray, ary, classCast);
			}

			// java.sql.Date 混じり
			{
				Object[]	ary = {
					"リフレクションパッケージの特定メソッドは、プリミティブ型とクラス型オブジェクト間のデータ変換を自動的に実行します。フィールドおよび配列のコンポーネント値を取得したり、設定したりするための汎用メソッド、およびメソッドとコンストラクタ呼び出しのためのメソッドがあります。",
					java.sql.Date.valueOf("2004-12-22"),
					"自動データ変換には次の 2 つのタイプがあります。「ラッピング変換」は、プリミティブ型からクラス型オブジェクトの値に変換します。「アンラッピング変換」は、クラス型オブジェクトからプリミティブ型の値に変換します。これらの変換に関する規則は、「ラッピング変換とアンラッピング変換」で定義します。",
					readUTF8(charDataFile),
					"さらに、フィールドアクセスとメソッド呼び出しは、プリミティブ型および参照型について「拡張変換」を可能にします。これらの変換については、Java 言語仕様の項 5 を参照してください。 より詳細な情報は、「拡張変換」を参照してください。" };
				String[]	ans = {
					"リフレクションパッケージの特定メソッドは、プリミティブ型とクラス型オブジェクト間のデータ変換を自動的に実行します。フィールドおよび配列のコンポーネント値を取得したり、設定したりするための汎用メソッド、およびメソッドとコンストラクタ呼び出しのためのメソッドがあります。",
					"2004-12-22",
					"自動データ変換には次の 2 つのタイプがあります。「ラッピング変換」は、プリミティブ型からクラス型オブジェクトの値に変換します。「アンラッピング変換」は、クラス型オブジェクトからプリミティブ型の値に変換します。これらの変換に関する規則は、「ラッピング変換とアンラッピング変換」で定義します。",
					readUTF8(charDataFile),
					"さらに、フィールドアクセスとメソッド呼び出しは、プリミティブ型および参照型について「拡張変換」を可能にします。これらの変換については、Java 言語仕様の項 5 を参照してください。 より詳細な情報は、「拡張変換」を参照してください。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// java.sql.Timestamp 混じり
			{
				Object[]	ary = {
					"ダイナミックプロキシクラスとは、実行時に指定されたインタフェースのリストを実装するクラスのことです。 ダイナミックプロキシクラスを使用すると、メソッドが呼び出されるときに、そのインスタンスのいずれかのインタフェースを介して符号化され、統一インタフェースを介して別のオブジェクトにディスパッチされます。",
					"このため、インタフェースのリストに対して型保証されたプロキシオブジェクトを作成できます。 コンパイル時にツールを使用するなど、プロキシクラスを事前に生成する必要がなくなります。",
					java.sql.Timestamp.valueOf("2004-12-22 10:05:10.541"),
					"ダイナミックプロキシクラスのインスタンス上でメソッドを呼び出すと、インスタンスの呼び出しハンドラ内の 1 つのメソッドにディスパッチされ、呼び出されたメソッドを識別する java.lang.reflect.Method オブジェクト、および引数を含む Object 型の配列を使用して符号化されます。" };
				String[]	ans = {
					"ダイナミックプロキシクラスとは、実行時に指定されたインタフェースのリストを実装するクラスのことです。 ダイナミックプロキシクラスを使用すると、メソッドが呼び出されるときに、そのインスタンスのいずれかのインタフェースを介して符号化され、統一インタフェースを介して別のオブジェクトにディスパッチされます。",
					"このため、インタフェースのリストに対して型保証されたプロキシオブジェクトを作成できます。 コンパイル時にツールを使用するなど、プロキシクラスを事前に生成する必要がなくなります。",
					"2004-12-22 10:05:10.541",
					"ダイナミックプロキシクラスのインスタンス上でメソッドを呼び出すと、インスタンスの呼び出しハンドラ内の 1 つのメソッドにディスパッチされ、呼び出されたメソッドを識別する java.lang.reflect.Method オブジェクト、および引数を含む Object 型の配列を使用して符号化されます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// jp.co.ricoh.doquedb.common.LanguageData 混じり
			{
				Object[]	ary = {
					readUTF8(largeCharDataFile),
					"ダイナミックプロキシクラスは、インタフェース API を提供するオブジェクト上で呼び出しを行うときに、アプリケーションまたはライブラリから型保証されたリフレクトディスパッチを行う必要がある場合に使用します。",
					new LanguageData("it"),
					"たとえば、ダイナミックプロキシクラスをアプリケーションで使用すると、複数の任意のイベントリスナーインタフェース (java.util.EventListener を継承するインタフェース) を実装するオブジェクトを作成し、すべてのイベントログをファイルに記録するなど、さまざまなタイプのイベントを統一された方式で処理できます。" };
				String[]	ans = {
					readUTF8(largeCharDataFile),
					"ダイナミックプロキシクラスは、インタフェース API を提供するオブジェクト上で呼び出しを行うときに、アプリケーションまたはライブラリから型保証されたリフレクトディスパッチを行う必要がある場合に使用します。",
					"it",
					"たとえば、ダイナミックプロキシクラスをアプリケーションで使用すると、複数の任意のイベントリスナーインタフェース (java.util.EventListener を継承するインタフェース) を実装するオブジェクトを作成し、すべてのイベントログをファイルに記録するなど、さまざまなタイプのイベントを統一された方式で処理できます。" };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			isTestArray = true;
		}
	}

	// BINARY 配列への PreparedStatement.setArray() のテスト
	private void checkSetBinaryArray(Connection	c) throws Exception
	{
		String	columnName = "af_binary50";
		int	elementType = DataType.BINARY;
		java.util.Vector	anss = new java.util.Vector();

		java.sql.SQLException	classCast = new ClassCast();

		boolean	isTestArray = false;
		for (int i = 0; i < 2; i++) {

			// only byte[]
			{
				byte[]	elm1 = { 0x0C, 0x1F, 0x60, 0x3C, 0x3A, 0x34, 0x44, 0x41, 0x08, 0x2B, 0x4A, 0x26, 0x1F, 0x29, 0x6F, 0x66, 0x48, 0x1E, 0x0F, 0x48, 0x1F };
				byte[]	elm2 = { 0x1D, 0x37, 0x5A, 0x6C, 0x08, 0x1A, 0x24, 0x45, 0x3C, 0x28, 0x67, 0x0C, 0x3F, 0x3F, 0x1D, 0x2E, 0x23, 0x5F, 0x6E, 0x29, 0x1A, 0x0B };
				byte[]	elm3 = { 0x5D, 0x1E, 0x2B, 0x3F, 0x19, 0x6E, 0x1D, 0x73, 0x1A, 0x4F, 0x2B, 0x06, 0x18, 0x2B, 0x6C, 0x70, 0x1B, 0x1B, 0x19, 0x6A, 0x08, 0x6D, 0x1B, 0x2E, 0x53 };
				byte[]	elm4 = { 0x00, 0x0E, 0x07, 0x19, 0x3D, 0x2A, 0x0B, 0x2D, 0x3F, 0x29, 0x14, 0x74, 0x59, 0x61, 0x56, 0x6C, 0x1D, 0x07, 0x29, 0x28, 0x6E, 0x3E };
				byte[]	elm5 = { 0x61, 0x19, 0x1A, 0x1D, 0x1B, 0x41, 0x7B, 0x41, 0x4F, 0x5D, 0x18, 0x23, 0x77, 0x0E, 0x78, 0x26, 0x24, 0x78, 0x73, 0x01 };
				byte[]	elm6 = { 0x4A, 0x3A, 0x6B, 0x19, 0x35, 0x1E, 0x0F, 0x7F, 0x64, 0x19, 0x4A, 0x2B, 0x29, 0x1F, 0x56, 0x09, 0x2F, 0x23, 0x0F, 0x08, 0x09, 0x4F, 0x2E, 0x1B };
				byte[][]	ary = { elm1, elm2, elm3, elm4, elm5, elm6 };
				byte[][]	ans = { appendNull(elm1, 50), appendNull(elm2, 50), appendNull(elm3, 50), appendNull(elm4, 50), appendNull(elm5, 50), appendNull(elm6, 50) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// String 混じり
			{
				byte[]	elm1 = { 0x3D, 0x18, 0x1A, 0x0E, 0x3B, 0x74, 0x48, 0x0C, 0x6B, 0x4D, 0x14, 0x18, 0x0C, 0x7A, 0x45, 0x3C, 0x7C, 0x59, 0x1C, 0x0C };
				byte[]	elm2 = { 0x22, 0x6E, 0x3F, 0x2C, 0x18, 0x36, 0x74, 0x16, 0x5B, 0x79, 0x26, 0x5E, 0x2C, 0x1C, 0x18, 0x0F, 0x58, 0x1B };
				byte[]	elm3 = { 0x0D, 0x3A, 0x17, 0x51, 0x10, 0x26, 0x1F, 0x47, 0x48, 0x5A, 0x7D, 0x2C, 0x6A, 0x47, 0x1C, 0x1A, 0x09, 0x39, 0x28, 0x1E, 0x68, 0x2D, 0x05 };
				Object[]	ary = { elm1, elm2, "binary array", elm3 };
				byte[][]	ans = { appendNull(elm1, 50), appendNull(elm2, 50), appendNull(stringToBytes("binary array"), 50), appendNull(elm3, 50) };
				byte[][]	ans14 = {elm1, elm2, stringToBytes("binary array"), elm3 };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			// null 混じり
			{
				byte[]	elm1 = { 0x29, 0x02, 0x49, 0x1B, 0x27, 0x1C, 0x5E, 0x49, 0x1C, 0x47, 0x1B, 0x29, 0x0A, 0x3A, 0x79, 0x1D, 0x18, 0x1F, 0x28, 0x5E, 0x56, 0x7A };
				byte[]	elm2 = { 0x1E, 0x0B, 0x02, 0x1F, 0x52, 0x46, 0x56, 0x40, 0x5E, 0x19, 0x2E, 0x78, 0x53, 0x18, 0x75, 0x27, 0x38, 0x2A, 0x07 };
				byte[]	elm3 = { 0x30, 0x18, 0x78, 0x06, 0x2D, 0x5F, 0x5D, 0x3C, 0x6B, 0x49, 0x7B, 0x1A, 0x3C, 0x2A, 0x70, 0x52, 0x54, 0x18, 0x6C, 0x7A, 0x28, 0x1C, 0x07 };
				byte[]	elm4 = { 0x03, 0x0D, 0x3A, 0x7F, 0x5F, 0x53, 0x19, 0x09, 0x64, 0x34, 0x28, 0x1B, 0x2B, 0x0D, 0x2C, 0x29, 0x0E, 0x38, 0x33, 0x1A, 0x10, 0x39, 0x33 };
				byte[][]	ary = { elm1, elm2, null, elm3, elm4 };
				byte[][]	ans = { appendNull(elm1, 50), appendNull(elm2, 50), null, appendNull(elm3, 50), appendNull(elm4, 50) };
				assertSetArray(c, columnName, isTestArray, ary);
				anss.add(ans);
				assertGetArray(c, columnName, anss, elementType);
			}

			//
			// 以下は例外 ClassCast が throw されるはず
			//

			byte[]	elm1 = { 0x67, 0x13, 0x32, 0x7A, 0x5F, 0x00, 0x1E, 0x27, 0x31, 0x79, 0x3C, 0x44, 0x7B, 0x1B, 0x21, 0x53, 0x64, 0x18, 0x1F, 0x1F, 0x2A };
			byte[]	elm2 = { 0x51, 0x13, 0x6A, 0x44, 0x48, 0x18, 0x59, 0x46, 0x47, 0x27, 0x4E, 0x4E, 0x10, 0x67, 0x21, 0x1F, 0x3D };
			byte[]	elm3 = { 0x50, 0x1C, 0x02, 0x26, 0x6D, 0x28, 0x3D, 0x4F, 0x2A, 0x46, 0x19, 0x38, 0x18, 0x2C, 0x3E, 0x18, 0x3E, 0x70, 0x44, 0x58, 0x59, 0x2C, 0x28, 0x5F };
			byte[]	elm4 = { 0x6C, 0x16, 0x6E, 0x1D, 0x53, 0x7C, 0x74, 0x19, 0x1A, 0x4A, 0x68, 0x18, 0x5C, 0x5D, 0x7C, 0x10, 0x73, 0x04 };
			byte[]	elm5 = { 0x7A, 0x59, 0x2A, 0x0F, 0x1E, 0x0F, 0x3F, 0x3D, 0x40, 0x1A, 0x3B, 0x51, 0x0C, 0x17, 0x2D, 0x2B, 0x6C, 0x70, 0x00, 0x19, 0x52 };
			Object[][]	arys = {
				{ elm2, new Integer(81), elm1 },												// Integer 混じり
				{ elm3, elm5, elm1, new Long(14098758710003L), elm2 },							// Long 混じり
				{ new Float(6.07), elm1, elm3 },												// Float 混じり
				{ elm1, elm4, new Double(938274.087) },											// Double 混じり
				{ elm4, elm1, java.sql.Date.valueOf("2004-12-21") },							// java.sql.Date 混じり
				{ elm5, elm4, elm2, java.sql.Timestamp.valueOf("2004-12-21 11:05:53.491") },	// java.sql.Timestamp 混じり
				{ elm3, elm5, new LanguageData("jw"), elm1 }									// jp.co.ricoh.doquedb.common.LanguageData 混じり
			};
			for (int j = 0; j < arys.length; j++) assertSetArray(c, columnName, isTestArray, arys[j], classCast);

			isTestArray = true;
		}
	}

	// PreparedStatement.setObject(int, Object),
	// PreparedStatement.setObject(int, Object, int),
	// PreparedStatement.setObject(int, Object, int, int) のテスト
	private void checkSetObject(int	numPrm) throws Exception
	{
		Integer					smallIntValue = new Integer(3);
		Long					intValue = new Long(439872);
		Long					bigIntValue = new Long(4682390178872L);
		BigDecimal				decimalValue = new BigDecimal("98765.4321");
		Float					miniFloatValue = new Float(3.5);	// 文字列に変換したときに短い Float 値
		Float					floatValue = new Float(3.32895);
		Double					miniDoubleValue = new Double(2.1);	// 文字列に変換したときに短い Double 値
		Double					doubleValue = new Double(917437.877832);
		String					decCharValue = "30";
		String					hexCharValue = "0x10";
		String					miniRealCharValue = "3.9";
		String					realCharValue = "394872.8973";
		String					dateCharValue = "2004-12-22";
		String					timeCharValue = "17:16:04";
		String					timestampCharValue = "2004-12-27 17:20:19.317";
		String					langCharValue = "ja";
		String					shortCharValue = "hoge";
		String					charValue = "abcdefghijklmnopqrstuvwxyz";
		String					idValue = "D6BE37FB-352A-4d90-B7FD-1288B27639C5";
		java.sql.Date			dateValue = java.sql.Date.valueOf("2004-12-22");
		java.sql.Timestamp		timestampValue = java.sql.Timestamp.valueOf("2004-12-22 19:23:14.386");
		byte[]					binaryValue = new byte[30];
		LanguageData			languageValue = new LanguageData("it");

		Integer[]		integerArray = { new Integer(1), new Integer(2), new Integer(3) };
		TestArray		integerArrayValue = new TestArray(integerArray);

		Long[]			longArray = { new Long(70189773610L), new Long(3671937641000L), new Long(834198760001378L) };
		TestArray		longArrayValue = new TestArray(longArray);

		Float[]			floatArray = { new Float(4398.2), new Float(9.878432) };
		TestArray		floatArrayValue = new TestArray(floatArray);

		Double[]		doubleArray = { new Double(987992.987432), new Double(0.000874890082), new Double(45877.8) };
		TestArray		doubleArrayValue = new TestArray(doubleArray);

		Date[]			dateArray = { java.sql.Date.valueOf("2004-12-22"), java.sql.Date.valueOf("1980-10-21") };
		TestArray		dateArrayValue = new TestArray(dateArray);

		Timestamp[]		timestampArray = { java.sql.Timestamp.valueOf("2004-12-22 20:31:08.441"), java.sql.Timestamp.valueOf("2003-05-01 10:49:08.001") };
		TestArray		timestampArrayValue = new TestArray(timestampArray);

		LanguageData[]	languageArray = { new LanguageData("ja"), new LanguageData("zh"), new LanguageData("it") };
		TestArray		languageArrayValue = new TestArray(languageArray);

		String[]		idArray = { "C15B4B39-7CCE-4b2f-9132-15CB59647074", "728571B1-2865-45bf-941C-2E78B2417D84", "70A2924F-7455-4966-BFBC-06AC786234BB" };
		TestArray		idArrayValue = new TestArray(idArray);

		String[]		decCharArray = { "100", "200", "300" };
		TestArray		decCharArrayValue = new TestArray(decCharArray);
		String[]		hexCharArray = { "0x47", "0x30", "0x0A", "0x5F" };
		TestArray		hexCharArrayValue = new TestArray(hexCharArray);
		String[]		miniRealCharArray = { "3.1", "2", "5.0" };
		TestArray		miniRealCharArrayValue = new TestArray(miniRealCharArray);
		String[]		realCharArray = { "0.009832", "43897.2", "343293E-3" };
		TestArray		realCharArrayValue = new TestArray(realCharArray);
		String[]		langCharArray = { "en", "ja", "it", "fr", "de" };
		TestArray		langCharArrayValue = new TestArray(langCharArray);
		String[]		dateCharArray = { "2004-12-22", "2004-10-03" };
		TestArray		dateCharArrayValue = new TestArray(dateCharArray);
		String[]		shortCharArray = { "abc", "hijk", "x" };
		TestArray		shortCharArrayValue = new TestArray(shortCharArray);
		String[]		charArray = { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "01234567890012345678900123456789001234567890012345678900123456789001234567890" };
		TestArray		charArrayValue = new TestArray(charArray);

		byte[]			bytesElm1 = { 0x0C, 0x1F, 0x60, 0x3C, 0x3A, 0x34, 0x44, 0x41, 0x08, 0x2B, 0x4A, 0x26, 0x1F, 0x29, 0x6F, 0x66, 0x48, 0x1E, 0x0F, 0x48, 0x1F };
		byte[]			bytesElm2 = { 0x1D, 0x37, 0x5A, 0x6C, 0x08, 0x1A, 0x24, 0x45, 0x3C, 0x28, 0x67, 0x0C, 0x3F, 0x3F, 0x1D, 0x2E, 0x23, 0x5F, 0x6E, 0x29, 0x1A, 0x0B };
		byte[][]		bytesArray = { bytesElm1, bytesElm2 };
		TestArray		bytesArrayValue = new TestArray(bytesArray);

		java.sql.SQLException	classCast = new ClassCast();
		java.sql.SQLException	invalidCharacter = new InvalidCharacter();
		java.sql.SQLException	stringRightTruncation = new StringRightTruncation();
		java.sql.SQLException	invalidDatetimeFormat = new InvalidDatetimeFormat();
		java.sql.SQLException	numericValueOutOfRange = new NumericValueOutOfRange();

		SetObjectInfo[]	forIntInfos = {
			new SetObjectInfo(java.sql.Types.NULL),															// INT に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),										// INT に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),											// INT に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			numericValueOutOfRange),	// INT に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),										// INT に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),										// INT に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),										// INT に Double
			new SetObjectInfo(java.sql.Types.CHAR,		decCharValue),										// INT に String (整数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		realCharValue,			invalidCharacter),			// INT に String (実数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		hexCharValue,			invalidCharacter),			// INT に String - 16 進数
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue,			invalidCharacter),			// INT に String - 数字列ではない
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),					// INT に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),					// INT に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),					// INT に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),					// INT に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),					// INT に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),					// INT に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)					// INT に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forIntColumnInfo = new SetObjectColumnInfo("f_int2", forIntInfos);

		SetObjectInfo[]	forBigIntInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// BIGINT に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),								// BIGINT に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),									// BIGINT に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),								// BIGINT に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),								// BIGINT に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),								// BIGINT に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),								// BIGINT に Double
			new SetObjectInfo(java.sql.Types.CHAR,		decCharValue),								// BIGINT に String (整数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		realCharValue,			invalidCharacter),	// BIGINT に String (実数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		hexCharValue,			invalidCharacter),	// BIGINT に String - 16 進数
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue,			invalidCharacter),	// BIGINT に String - 数字列ではない
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// BIGINT に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// BIGINT に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// BIGINT に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// BIGINT に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),			// BIGINT に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),			// BIGINT に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)			// BIGINT に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forBigIntColumnInfo = new SetObjectColumnInfo("f_bigint", forBigIntInfos);

		SetObjectInfo[]	forDecimalInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// DECIMAL に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),								// DECIMAL に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),									// DECIMAL に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			numericValueOutOfRange), // DECIMAL に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),								// DECIMAL に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),								// DECIMAL に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),								// DECIMAL に Double
			new SetObjectInfo(java.sql.Types.CHAR,		decCharValue),								// DECIMAL に String (整数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		realCharValue),								// DECIMAL に String (実数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		hexCharValue,			invalidCharacter),	// DECIMAL に String - 16 進数
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue,			invalidCharacter),	// DECIMAL に String - 数字列ではない
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// DECIMAL に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// DECIMAL に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// DECIMAL に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// DECIMAL に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),			// DECIMAL に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),			// DECIMAL に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)			// DECIMAL に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forDecimalColumnInfo = new SetObjectColumnInfo("f_decimal", forDecimalInfos);

		SetObjectInfo[]	forCharInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// CHAR に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),									// CHAR に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),										// CHAR に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			stringRightTruncation),	// CHAR に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			stringRightTruncation),	// CHAR に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		miniFloatValue),								// CHAR に Float (文字列に変換したときに短い Float 値)
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				stringRightTruncation),	// CHAR に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	miniDoubleValue),								// CHAR に Double (文字列に変換したときに短い Double 値)
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			stringRightTruncation),	// CHAR に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),								// CHAR に String (short)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),										// CHAR に String (long)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				stringRightTruncation),	// CHAR に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			stringRightTruncation),	// CHAR に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// CHAR に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),									// CHAR に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),				// CHAR に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),				// CHAR に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)				// CHAR に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forCharColumnInfo = new SetObjectColumnInfo("f_char8", forCharInfos);

		SetObjectInfo[]	forFloatInfos = {
			new SetObjectInfo(java.sql.Types.NULL),												// FLOAT に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),							// FLOAT に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),								// FLOAT に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),							// FLOAT に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),							// FLOAT に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),							// FLOAT に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),							// FLOAT に Double
			new SetObjectInfo(java.sql.Types.CHAR,		realCharValue),							// FLOAT に String (実数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		decCharValue),							// FLOAT に String (整数を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,			invalidCharacter),	// FLOAT に String (String)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,			classCast),			// FLOAT に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,		classCast),			// FLOAT に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,		classCast),			// FLOAT に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,		classCast),			// FLOAT に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		floatArrayValue,	classCast),			// FLOAT に java.sql.Array (Float 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue,	classCast),			// FLOAT に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast),			// FLOAT に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,	classCast)			// FLOAT に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forFloatColumnInfo = new SetObjectColumnInfo("f_float", forFloatInfos);

		SetObjectInfo[]	forDateTimeInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// DATETIME に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),				// DATETIME に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),				// DATETIME に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),				// DATETIME に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),				// DATETIME に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),				// DATETIME に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),				// DATETIME に Double
			new SetObjectInfo(java.sql.Types.CHAR,		dateCharValue),									// DATETIME に String (暦を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		timestampCharValue),							// DATETIME に String (日時を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		timeCharValue,			invalidDatetimeFormat),	// DATETIME に String (時間を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				invalidDatetimeFormat),	// DATETIME に String (String)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),										// DATETIME に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),								// DATETIME に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// DATETIME に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),				// DATETIME に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue,			classCast),				// DATETIME に java.sql.Array
			new SetObjectInfo(java.sql.Types.ARRAY,		timestampArrayValue,	classCast),				// DATETIME に java.sql.Array (Timestamp 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateCharArrayValue,		classCast),				// DATETIME に java.sql.Array (暦を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),				// DATETIME に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast)				// DATETIME に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forDateTimeColumnInfo = new SetObjectColumnInfo("f_datetime", forDateTimeInfos);

		SetObjectInfo[]	forIDInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// UNIQUEIDENTIFIER に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// UNIQUEIDENTIFIER に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// UNIQUEIDENTIFIER に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// UNIQUEIDENTIFIER に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// UNIQUEIDENTIFIER に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// UNIQUEIDENTIFIER に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// UNIQUEIDENTIFIER に Double
			new SetObjectInfo(java.sql.Types.CHAR,		idValue),							// UNIQUEIDENTIFIER に String (GUIID を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// UNIQUEIDENTIFIER に String (String)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// UNIQUEIDENTIFIER に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// UNIQUEIDENTIFIER に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// UNIQUEIDENTIFIER に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// UNIQUEIDENTIFIER に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		idArrayValue,			classCast),	// UNIQUEIDENTIFIER に java.sql.Array (GUID (String) 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),	// UNIQUEIDENTIFIER に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue,			classCast),	// UNIQUEIDENTIFIER に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast)	// UNIQUEIDENTIFIER に java.sql.Array (短い文字列の要素の String 配列)
		};
		SetObjectColumnInfo	forIDColumnInfo = new SetObjectColumnInfo("f_id", forIDInfos);

		SetObjectInfo[]	forImageInfos = {
			new SetObjectInfo(java.sql.Types.NULL),										// IMAGE に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,		classCast),	// IMAGE に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,			classCast),	// IMAGE に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,		classCast),	// IMAGE に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,		classCast),	// IMAGE に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,			classCast),	// IMAGE に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,		classCast),	// IMAGE に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),				// IMAGE に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,			classCast),	// IMAGE に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,		classCast),	// IMAGE に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue),					// IMAGE に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,		classCast),	// IMAGE に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,	classCast),	// IMAGE に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast),	// IMAGE に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,		classCast)	// IMAGE に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forImageColumnInfo = new SetObjectColumnInfo("f_image", forImageInfos);

		SetObjectInfo[]	forLanguageInfos = {
			new SetObjectInfo(java.sql.Types.NULL),												// LANGUAGE に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,		classCast),			// LANGUAGE に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,			classCast),			// LANGUAGE に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,		classCast),			// LANGUAGE に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,		classCast),			// LANGUAGE に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,			classCast),			// LANGUAGE に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,		classCast),			// LANGUAGE に Double
			new SetObjectInfo(java.sql.Types.CHAR,		langCharValue),							// LANGUAGE に String (言語を示す String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,			invalidCharacter),	// LANGUAGE に String (String)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,			classCast),			// LANGUAGE に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,		classCast),			// LANGUAGE に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,		classCast),			// LANGUAGE に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),							// LANGUAGE に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue,	classCast),			// LANGUAGE に java.sql.Array (LanguageData 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		langCharArrayValue,	classCast),			// LANGUAGE に java.sql.Array (言語を示す文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast),			// LANGUAGE に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,		classCast)			// LANGUAGE に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forLanguageColumnInfo = new SetObjectColumnInfo("f_language", forLanguageInfos);

		SetObjectInfo[]	forNcharInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// NCHAR に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),									// NCHAR に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),										// NCHAR に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			stringRightTruncation),	// NCHAR に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			stringRightTruncation),	// NCHAR に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		miniFloatValue),								// NCHAR に Float (文字列に変換したときに短い Float 値)
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				stringRightTruncation),	// NCHAR に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	miniDoubleValue),								// NCHAR に Double (文字列に変換したときに短い Double 値)
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			stringRightTruncation),	// NCHAR に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),								// NCHAR に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),										// NCHAR に String (String)
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				stringRightTruncation),	// NCHAR に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			stringRightTruncation),	// NCHAR に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// NCHAR に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),									// NCHAR に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),				// NCHAR に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),				// NCHAR に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)				// NCHAR に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forNcharColumnInfo = new SetObjectColumnInfo("f_nchar6", forNcharInfos);

		SetObjectInfo[]	forNvarcharInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NVARCHAR に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// NVARCHAR に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// NVARCHAR に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// NVARCHAR に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// NVARCHAR に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// NVARCHAR に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// NVARCHAR に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),					// NVARCHAR に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// NVARCHAR に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// NVARCHAR に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// NVARCHAR に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NVARCHAR に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// NVARCHAR に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),	// NVARCHAR に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),	// NVARCHAR に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// NVARCHAR に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forNvarcharColumnInfo = new SetObjectColumnInfo("f_nvarchar256", forNvarcharInfos);

		SetObjectInfo[]	forVarcharInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// VARCHAR に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// VARCHAR に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// VARCHAR に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// VARCHAR に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// VARCHAR に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// VARCHAR に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// VARCHAR に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),					// VARCHAR に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// VARCHAR に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// VARCHAR に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// VARCHAR に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// VARCHAR に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// VARCHAR に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),	// VARCHAR に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),	// VARCHAR に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// VARCHAR に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forVarcharColumnInfo = new SetObjectColumnInfo("f_varchar128", forVarcharInfos);

		SetObjectInfo[]	forNtextInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NTEXT に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// NTEXT に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// NTEXT に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// NTEXT に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// NTEXT に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// NTEXT に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// NTEXT に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),					// NTEXT に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// NTEXT に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// NTEXT に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// NTEXT に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NTEXT に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// NTEXT に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),	// NTEXT に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),	// NTEXT に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// NTEXT に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forNtextColumnInfo = new SetObjectColumnInfo("f_ntext", forNtextInfos);

		SetObjectInfo[]	forNtextCompressedInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NTEXT(compressed) に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// NTEXT(compressed) に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// NTEXT(compressed) に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// NTEXT(compressed) に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// NTEXT(compressed) に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// NTEXT(compressed) に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// NTEXT(compressed) に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),					// NTEXT(compressed) に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// NTEXT(compressed) に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// NTEXT(compressed) に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// NTEXT(compressed) に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NTEXT(compressed) に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// NTEXT(compressed) に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),	// NTEXT(compressed) に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),	// NTEXT(compressed) に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// NTEXT(compressed) に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forNtextCompressedColumnInfo = new SetObjectColumnInfo("f_ntext_compressed", forNtextCompressedInfos);

		SetObjectInfo[]	forFulltextInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// FULLTEXT に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),						// FULLTEXT に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),							// FULLTEXT に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),						// FULLTEXT に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),						// FULLTEXT に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),						// FULLTEXT に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),						// FULLTEXT に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),					// FULLTEXT に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),							// FULLTEXT に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),							// FULLTEXT に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),					// FULLTEXT に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// FULLTEXT に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),						// FULLTEXT に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue,	classCast),	// FULLTEXT に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			classCast),	// FULLTEXT に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// FULLTEXT に java.sql.Array (Integer 配列) (Integer 配列)
		};
		SetObjectColumnInfo	forFulltextColumnInfo = new SetObjectColumnInfo("f_fulltext", forFulltextInfos);

		SetObjectInfo[]	forBinaryInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// BINARY に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,		classCast),				// BINARY に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,			classCast),				// BINARY に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,		classCast),				// BINARY に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,		classCast),				// BINARY に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,			classCast),				// BINARY に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,		classCast),				// BINARY に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),							// BINARY に String (短い String)
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,			stringRightTruncation),	// BINARY に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,			classCast),				// BINARY に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,		classCast),				// BINARY に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue),								// BINARY に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,		classCast),				// BINARY に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,	classCast),				// BINARY に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,		classCast),				// BINARY に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast)				// BINARY に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forBinaryColumnInfo = new SetObjectColumnInfo("f_binary50", forBinaryInfos);

		SetObjectInfo[]	forBlobInfos = {
			new SetObjectInfo(java.sql.Types.NULL),										// BLOB に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,		classCast),	// BLOB に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,			classCast),	// BLOB に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,		classCast),	// BLOB に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,		classCast),	// BLOB に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,			classCast),	// BLOB に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,		classCast),	// BLOB に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),						// BLOB に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,			classCast),	// BLOB に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,		classCast),	// BLOB に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue),					// BLOB に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,		classCast),	// BLOB に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,	classCast),	// BLOB に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,		classCast),	// BLOB に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast)	// BLOB に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forBlobColumnInfo = new SetObjectColumnInfo("f_blob", forBlobInfos);

		SetObjectInfo[]	forNclobInfos = {
			new SetObjectInfo(java.sql.Types.NULL),										// NCLOB に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue),					// NCLOB に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue),						// NCLOB に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue),					// NCLOB に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue),					// NCLOB に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue),					// NCLOB に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue),					// NCLOB に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue),				// NCLOB に String
			new SetObjectInfo(java.sql.Types.CHAR,		charValue),						// NCLOB に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue),						// NCLOB に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue),				// NCLOB に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,		classCast),	// NCLOB に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue),					// NCLOB に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,		classCast),	// NCLOB に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,	classCast),	// NCLOB に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,	classCast)	// NCLOB に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forNclobColumnInfo = new SetObjectColumnInfo("f_nclob", forNclobInfos);

		SetObjectInfo[]	forIntArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// INT 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),			// INT 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),			// INT 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),			// INT 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),			// INT 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),			// INT 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),			// INT 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),			// INT 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// INT 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// INT 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// INT 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// INT 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),							// INT 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		floatArrayValue),							// INT 配列に java.sql.Array (Float 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),							// INT 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		decCharArrayValue),							// INT 配列に java.sql.Array (10 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		hexCharArrayValue,		invalidCharacter),	// INT 配列に java.sql.Array (16 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		realCharArrayValue,		invalidCharacter),	// INT 配列に java.sql.Array (実数を示す要素の String 配列
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidCharacter)	// INT 配列に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forIntArrayColumnInfo = new SetObjectColumnInfo("af_int", forIntArrayInfos);

		SetObjectInfo[]	forBigIntArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// BIGINT 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),			// BIGINT 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),			// BIGINT 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),			// BIGINT 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),			// BIGINT 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),			// BIGINT 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),			// BIGINT 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),			// BIGINT 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// BIGINT 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// BIGINT 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// BIGINT 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// BIGINT 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),							// BIGINT 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		floatArrayValue),							// BIGINT 配列に java.sql.Array (Float 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),							// BIGINT 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		decCharArrayValue),							// BIGINT 配列に java.sql.Array (10 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		hexCharArrayValue,		invalidCharacter),	// BIGINT 配列に java.sql.Array (16 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		realCharArrayValue,		invalidCharacter),	// BIGINT 配列に java.sql.Array (実数を示す要素の String 配列
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidCharacter)	// BIGINT 配列に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forBigIntArrayColumnInfo = new SetObjectColumnInfo("af_bigint", forBigIntArrayInfos);

		SetObjectInfo[]	forDecimalArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// DECIMAL 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),			// DECIMAL 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),			// DECIMAL 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),			// DECIMAL 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),			// DECIMAL 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),			// DECIMAL 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),			// DECIMAL 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),			// DECIMAL 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// DECIMAL 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// DECIMAL 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// DECIMAL 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// DECIMAL 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),							// DECIMAL 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		floatArrayValue),							// DECIMAL 配列に java.sql.Array (Float 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),							// DECIMAL 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		decCharArrayValue),							// DECIMAL 配列に java.sql.Array (10 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		hexCharArrayValue,		invalidCharacter),	// DECIMAL 配列に java.sql.Array (16 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		realCharArrayValue),						// DECIMAL 配列に java.sql.Array (実数を示す要素の String 配列
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidCharacter)	// DECIMAL 配列に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forDecimalArrayColumnInfo = new SetObjectColumnInfo("af_decimal", forDecimalArrayInfos);

		SetObjectInfo[]	forCharArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// CHAR 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),				// CHAR 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),				// CHAR 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),				// CHAR 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),				// CHAR 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),				// CHAR 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),				// CHAR 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),				// CHAR 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),				// CHAR 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),				// CHAR 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// CHAR 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),				// CHAR 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue),							// CHAR 配列に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),								// CHAR 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		decCharArrayValue),								// CHAR 配列に java.sql.Array (10 進数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		miniRealCharArrayValue),						// CHAR 配列に java.sql.Array (文字列に変換すると短い実数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),								// CHAR 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),							// CHAR 配列に java.sql.Array (LanguageData 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue,		stringRightTruncation),	// CHAR 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		timestampArrayValue,	stringRightTruncation)	// CHAR 配列に java.sql.Array (Timestamp 配列)
		};
		SetObjectColumnInfo	forCharArrayColumnInfo = new SetObjectColumnInfo("af_char8", forCharArrayInfos);

		SetObjectInfo[]	forFloatArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// FLOAT 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),			// FLOAT 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),			// FLOAT 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),			// FLOAT 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),			// FLOAT 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),			// FLOAT 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),			// FLOAT 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),			// FLOAT 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// FLOAT 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// FLOAT 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// FLOAT 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// FLOAT 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		floatArrayValue),							// FLOAT 配列に java.sql.Array (Float 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),							// FLOAT 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),							// FLOAT 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		realCharArrayValue),						// FLOAT 配列に java.sql.Array (実数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		decCharArrayValue),							// FLOAT 配列に java.sql.Array (整数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidCharacter)	// FLOAT 配列に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forFloatArrayColumnInfo = new SetObjectColumnInfo("af_float", forFloatArrayInfos);

		SetObjectInfo[]	forDatetimeArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// DATETIME 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),				// DATETIME 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),				// DATETIME 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),				// DATETIME 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),				// DATETIME 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),				// DATETIME 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),				// DATETIME 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),				// DATETIME 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),				// DATETIME 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),				// DATETIME 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// DATETIME 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),				// DATETIME 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),								// DATETIME 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),				// DATETIME 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		timestampArrayValue),							// DATETIME 配列に java.sql.Array (Timestamp 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateCharArrayValue),							// DATETIME 配列に java.sql.Array (暦を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidDatetimeFormat)	// DATETIME 配列に java.sql.Array (String 配列)
		};
		SetObjectColumnInfo	forDatetimeArrayColumnInfo = new SetObjectColumnInfo("af_datetime", forDatetimeArrayInfos);

		SetObjectInfo[]	forIDArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// UNIQUEIDENTIFIER 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// UNIQUEIDENTIFIER 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// UNIQUEIDENTIFIER 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// UNIQUEIDENTIFIER 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// UNIQUEIDENTIFIER 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// UNIQUEIDENTIFIER 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// UNIQUEIDENTIFIER 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// UNIQUEIDENTIFIER 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// UNIQUEIDENTIFIER 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// UNIQUEIDENTIFIER 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// UNIQUEIDENTIFIER 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// UNIQUEIDENTIFIER 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		idArrayValue),						// UNIQUEIDENTIFIER 配列に java.sql.Array (GUID (String) 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue),				// UNIQUEIDENTIFIER 配列に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// UNIQUEIDENTIFIER 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// UNIQUEIDENTIFIER 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// UNIQUEIDENTIFIER 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast),	// UNIQUEIDENTIFIER 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forIDArrayColumnInfo = new SetObjectColumnInfo("af_id", forIDArrayInfos);

		SetObjectInfo[]	forImageArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// IMAGE 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// IMAGE 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// IMAGE 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// IMAGE 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// IMAGE 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// IMAGE 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// IMAGE 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// IMAGE 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// IMAGE 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// IMAGE 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// IMAGE 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// IMAGE 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue),					// IMAGE 配列に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// IMAGE 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),	// IMAGE 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue,		classCast),	// IMAGE 配列に java.sql.Array (LanguageData 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		timestampArrayValue,	classCast)	// IMAGE 配列に java.sql.Array (Timestamp 配列)
		};
		SetObjectColumnInfo	forImageArrayColumnInfo = new SetObjectColumnInfo("af_image", forImageArrayInfos);

		SetObjectInfo[]	forLanguageArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),													// LANGUAGE 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),			// LANGUAGE 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),			// LANGUAGE 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),			// LANGUAGE 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),			// LANGUAGE 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),			// LANGUAGE 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),			// LANGUAGE 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),			// LANGUAGE 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),			// LANGUAGE 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),			// LANGUAGE 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),			// LANGUAGE 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),			// LANGUAGE 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),						// LANGUAGE 配列に java.sql.Array (LanguageData 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		langCharArrayValue),						// LANGUAGE 配列に java.sql.Array (言語を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue,			invalidCharacter),	// LANGUAGE 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast),			// LANGUAGE 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)			// LANGUAGE 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forLanguageArrayColumnInfo = new SetObjectColumnInfo("af_language", forLanguageArrayInfos);

		SetObjectInfo[]	forNcharArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),														// NCHAR 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),				// NCHAR 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),				// NCHAR 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),				// NCHAR 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),				// NCHAR 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),				// NCHAR 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),				// NCHAR 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),				// NCHAR 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),				// NCHAR 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),				// NCHAR 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),				// NCHAR 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),				// NCHAR 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue),							// NCHAR 配列に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),								// NCHAR 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),							// NTEXT(compressed) 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		miniRealCharArrayValue),						// NCHAR 配列に java.sql.Array (文字列に変換すると短い実数を示す要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		realCharArrayValue),							// NCHAR 配列に java.sql.Array (実数を示す要素の String 配列
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue,		stringRightTruncation),	// NTEXT(compressed) 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue,			stringRightTruncation)	// NTEXT(compressed) 配列に java.sql.Array (Date 配列)
		};
		SetObjectColumnInfo	forNcharArrayColumnInfo = new SetObjectColumnInfo("af_nchar6", forNcharArrayInfos);

		SetObjectInfo[]	forNvarcharArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NVARCHAR 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// NVARCHAR 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// NVARCHAR 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// NVARCHAR 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// NVARCHAR 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// NVARCHAR 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// NVARCHAR 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// NVARCHAR 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// NVARCHAR 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// NVARCHAR 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NVARCHAR 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// NVARCHAR 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// NVARCHAR 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// NVARCHAR 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),				// NTEXT(compressed) 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)	// NVARCHAR 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forNvarcharArrayColumnInfo = new SetObjectColumnInfo("af_nvarchar256", forNvarcharArrayInfos);

		SetObjectInfo[]	forVarcharArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// VARCHAR 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// VARCHAR 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER, 	intValue,				classCast),	// VARCHAR 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// VARCHAR 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// VARCHAR 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// VARCHAR 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// VARCHAR 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// VARCHAR 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// VARCHAR 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// VARCHAR 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// VARCHAR 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// VARCHAR 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// VARCHAR 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// VARCHAR 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),				// NTEXT(compressed) 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)	// VARCHAR 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forVarcharArrayColumnInfo = new SetObjectColumnInfo("af_varchar128", forVarcharArrayInfos);

		SetObjectInfo[]	forNtextArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NTEXT 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// NTEXT 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// NTEXT 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// NTEXT 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// NTEXT 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// NTEXT 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// NTEXT 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// NTEXT 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// NTEXT 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// NTEXT 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NTEXT 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// NTEXT 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// NTEXT 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// NTEXT 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),				// NTEXT(compressed) 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)	// NTEXT 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forNtextArrayColumnInfo = new SetObjectColumnInfo("af_ntext", forNtextArrayInfos);

		SetObjectInfo[]	forNtextCompressedArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// NTEXT(compressed) 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// NTEXT(compressed) 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// NTEXT(compressed) 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// NTEXT(compressed) 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// NTEXT(compressed) 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// NTEXT(compressed) 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// NTEXT(compressed) 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// NTEXT(compressed) 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// NTEXT(compressed) 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// NTEXT(compressed) 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// NTEXT(compressed) 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// NTEXT(compressed) 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),					// NTEXT(compressed) 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),				// NTEXT(compressed) 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)	// NTEXT(compressed) 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forNtextCompressedArrayColumnInfo = new SetObjectColumnInfo("af_ntext_compressed", forNtextCompressedArrayInfos);

		SetObjectInfo[]	forFulltextArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// FULLTEXT 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// FULLTEXT 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// FULLTEXT 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// FULLTEXT 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// FULLTEXT 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// FULLTEXT 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// FULLTEXT 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		charValue,				classCast),	// FULLTEXT 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// FULLTEXT 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// FULLTEXT 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// FULLTEXT 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// FULLTEXT 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		charArrayValue),					// FULLTEXT 配列に java.sql.Array (String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue),					// FULLTEXT 配列に java.sql.Array (Integer 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		doubleArrayValue),					// FULLTEXT 配列に java.sql.Array (Double 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		dateArrayValue),					// FULLTEXT 配列に java.sql.Array (Date 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		languageArrayValue),				// FULLTEXT 配列に java.sql.Array (Language 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue,		classCast)	// FULLTEXT 配列に java.sql.Array (byte[] 配列)
		};
		SetObjectColumnInfo	forFulltextArrayColumnInfo = new SetObjectColumnInfo("af_fulltext", forFulltextArrayInfos);

		SetObjectInfo[]	forBinaryArrayInfos = {
			new SetObjectInfo(java.sql.Types.NULL),											// BINARY 配列に null
			new SetObjectInfo(java.sql.Types.SMALLINT,	smallIntValue,			classCast),	// BINARY 配列に Integer
			new SetObjectInfo(java.sql.Types.INTEGER,	intValue,				classCast),	// BINARY 配列に Long - 1
			new SetObjectInfo(java.sql.Types.BIGINT,	bigIntValue,			classCast),	// BINARY 配列に Long - 2
			new SetObjectInfo(java.sql.Types.DECIMAL,	decimalValue,			classCast),	// BINARY 配列に Decimal
			new SetObjectInfo(java.sql.Types.FLOAT,		floatValue,				classCast),	// BINARY 配列に Float
			new SetObjectInfo(java.sql.Types.DOUBLE,	doubleValue,			classCast),	// BINARY 配列に Double
			new SetObjectInfo(java.sql.Types.CHAR,		shortCharValue,			classCast),	// BINARY 配列に String
			new SetObjectInfo(java.sql.Types.DATE,		dateValue,				classCast),	// BINARY 配列に java.sql.Date
			new SetObjectInfo(java.sql.Types.TIMESTAMP,	timestampValue,			classCast),	// BINARY 配列に java.sql.Timestamp
			new SetObjectInfo(java.sql.Types.BINARY,	binaryValue,			classCast),	// BINARY 配列に byte[]
			new SetObjectInfo(java.sql.Types.OTHER,		languageValue,			classCast),	// BINARY 配列に jp.co.ricoh.doquedb.common.LanguageData
			new SetObjectInfo(java.sql.Types.ARRAY,		bytesArrayValue),					// BINARY 配列に java.sql.Array (byte[] 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		shortCharArrayValue),				// BINARY 配列に java.sql.Array (短い文字列の要素の String 配列)
			new SetObjectInfo(java.sql.Types.ARRAY,		integerArrayValue,		classCast)	// BINARY 配列に java.sql.Array (Integer 配列)
		};
		SetObjectColumnInfo	forBinaryArrayColumnInfo = new SetObjectColumnInfo("af_binary50", forBinaryArrayInfos);

		SetObjectColumnInfo[]	columnInfos = {
			forIntColumnInfo,
			forBigIntColumnInfo,
			forDecimalColumnInfo,
			forCharColumnInfo,
			forFloatColumnInfo,
			forDateTimeColumnInfo,
			forIDColumnInfo,
			forImageColumnInfo,
			forLanguageColumnInfo,
			forNcharColumnInfo,
			forNvarcharColumnInfo,
			forVarcharColumnInfo,
			forNtextColumnInfo,
			forNtextCompressedColumnInfo,
			forFulltextColumnInfo,
			forBinaryColumnInfo,
			forBlobColumnInfo,
			forNclobColumnInfo,
			forIntArrayColumnInfo,
			forBigIntArrayColumnInfo,
			forDecimalArrayColumnInfo,
			forCharArrayColumnInfo,
			forFloatArrayColumnInfo,
			forDatetimeArrayColumnInfo,
			forIDArrayColumnInfo,
			forImageArrayColumnInfo,
			forLanguageArrayColumnInfo,
			forNcharArrayColumnInfo,
			forNvarcharArrayColumnInfo,
			forVarcharArrayColumnInfo,
			forNtextArrayColumnInfo,
			forNtextCompressedArrayColumnInfo,
			forFulltextArrayColumnInfo,
			forBinaryArrayColumnInfo
		};

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		for (int i = 0; i < columnInfos.length; i++) {

			SetObjectColumnInfo	columnInfo = columnInfos[i];
			String				columnName = columnInfo.getColumnName();
			// bigint 列は v15.0 からサポート
			if (columnName.compareTo("f_bigint") == 0 || columnName.compareTo("af_bigint") == 0) continue;

			// decimal 列は v16.1 からサポート
			if (columnName.compareTo("f_decimal") == 0 || columnName.compareTo("af_decimal") == 0) continue;
			SetObjectInfo[]		objectInfos = columnInfo.getObjectInfos();

			int	exceptionAt = 0;

			String	query = "insert into t (f_int_not_null, f_int1, f_char8_not_null, " + columnName + ") values (1, ?, 'hogehoge', ?)";
			PreparedStatement	ps = null;
			assertNotNull(ps = c.prepareStatement(query));

			for (int j = 0; j < objectInfos.length; j++) {

				SetObjectInfo	objectInfo = objectInfos[j];

				exceptionAt = SetObjectInfo.AT_SET_OBJECT;

				ps.setInt(1, _f_int1++);	// primary key

				String	SQLState = "";
				try {
					switch (numPrm) {
					case 2:
						ps.setObject(2, objectInfo.getObject());
						break;
					case 3:
						ps.setObject(2, objectInfo.getObject(), objectInfo.getSQLType());
						break;
					case 4:
						ps.setObject(2, objectInfo.getObject(), objectInfo.getSQLType(), 1);
						break;
					default:
						break;
					}

					exceptionAt = SetObjectInfo.AT_EXECUTE_UPDATE;
					ps.executeUpdate();

				} catch (SQLException	sqle) {
					SQLState = sqle.getSQLState();
					if (objectInfo.getExceptionAt() != exceptionAt) {
						System.out.println("#1 COLUMNNAME = " + columnName + "[" + j + "] : SQLTYPE = " + objectInfo.getSQLType() + " : INFO_SQLSTATE = " + objectInfo.getSQLState() + " : SQLSTATE = " + SQLState);
					}
					assertEquals(objectInfo.getExceptionAt(), exceptionAt);
				}
				if (objectInfo.getSQLState().equals(SQLState) == false) {
					System.out.println("#2 COLUMNNAME = " + columnName + "[" + j + "] : SQLTYPE = " + objectInfo.getSQLType() + " : INFO_SQLSTATE = " + objectInfo.getSQLState() + " : SQLSTATE = " + SQLState);
				}
				assertEquals(objectInfo.getSQLState(), SQLState);
			}

			ps.close();
		}

		// 後始末
		dropTestTable(c);

		c.close();
	}

	private String getInsertQuery(String	columnName)
	{
		return "insert into t (f_int_not_null, f_int1, f_char8_not_null, " + columnName + ") values (1, " + _f_int1++ + ", 'hogehoge', ?)";
	}

	private PreparedStatement getInsertPreparedStatement(	Connection	c,
															String		columnName) throws Exception
	{
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(getInsertQuery(columnName)));
		return ps;
	}

	private void assertSetByte(	Connection		c,
								String			columnName,
								byte			columnValue,
								SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setByte(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetByte(	Connection		c,
								String			columnName,
								byte			columnValue) throws Exception
	{
		assertSetByte(c, columnName, columnValue, null);
	}

	private void assertSetShort(	Connection		c,
									String			columnName,
									short			columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setShort(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetShort(	Connection		c,
									String			columnName,
									short			columnValue) throws Exception
	{
		assertSetShort(c, columnName, columnValue, null);
	}

	private void assertSetInt(	Connection		c,
								String			columnName,
								int				columnValue,
								SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setInt(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetInt(	Connection		c,
								String			columnName,
								int				columnValue) throws Exception
	{
		assertSetInt(c, columnName, columnValue, null);
	}

	private void assertSetLong(	Connection		c,
								String			columnName,
								long			columnValue,
								SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setLong(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetLong(	Connection	c,
								String		columnName,
								long		columnValue) throws Exception
	{
		assertSetLong(c, columnName, columnValue, null);
	}

	private void assertSetFloat(	Connection		c,
									String			columnName,
									float			columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setFloat(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetFloat(	Connection	c,
									String		columnName,
									float		columnValue) throws Exception
	{
		assertSetFloat(c, columnName, columnValue, null);
	}

	private void assertSetDouble(	Connection		c,
									String			columnName,
									double			columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setDouble(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetDouble(	Connection	c,
									String		columnName,
									double		columnValue) throws Exception
	{
		assertSetDouble(c, columnName, columnValue, null);
	}

	private void assertSetString(	Connection		c,
									String			columnName,
									String			columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setString(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetString(	Connection	c,
									String		columnName,
									String		columnValue) throws Exception
	{
		assertSetString(c, columnName, columnValue, null);
	}

	private void assertSetBytes(	Connection		c,
									String			columnName,
									byte[]			columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setBytes(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetBytes(	Connection	c,
									String		columnName,
									byte[]		columnValue) throws Exception
	{
		assertSetBytes(c, columnName, columnValue, null);
	}

	private void assertSetDate(	Connection		c,
								String			columnName,
								java.sql.Date	columnValue,
								SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setDate(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetDate(	Connection		c,
								String			columnName,
								java.sql.Date	columnValue) throws Exception
	{
		assertSetDate(c, columnName, columnValue, null);
	}

	private void assertSetTimestamp(	Connection			c,
										String				columnName,
										java.sql.Timestamp	columnValue,
										SQLException		e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		ps.setTimestamp(1, columnValue);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetTimestamp(	Connection			c,
										String				columnName,
										java.sql.Timestamp	columnValue) throws Exception
	{
		assertSetTimestamp(c, columnName, columnValue, null);
	}

	private int getFileLength(String	fileName) throws Exception
	{
		java.io.File	f = new java.io.File(fileName);
		return (int)f.length();
	}

	private void assertSetAsciiStream(	Connection		c,
										String			columnName,
										String			fileName,
										SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		java.io.InputStream	stream = null;
		int	flen = 0;
		if (fileName != null) {
			stream = new java.io.FileInputStream(fileName);
			flen = getFileLength(fileName);
		}
		ps.setAsciiStream(1, stream, flen);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
		if (stream != null) stream.close();
	}

	private void assertSetAsciiStream(	Connection	c,
										String		columnName,
										String		fileName) throws Exception
	{
		assertSetAsciiStream(c, columnName, fileName, null);
	}

	private void assertSetUnicodeStream(	Connection		c,
											String			columnName,
											String			fileName,
											SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		java.io.InputStream	stream = null;
		int	flen = 0;
		if (fileName != null) {
			stream = new java.io.FileInputStream(fileName);
			flen = getFileLength(fileName);
		}
		ps.setUnicodeStream(1, stream, flen);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
		if (stream != null) stream.close();
	}

	private void assertSetUnicodeStream(	Connection		c,
											String			columnName,
											String			fileName) throws Exception
	{
		assertSetUnicodeStream(c, columnName, fileName, null);
	}

	private void assertSetCharacterStream(	Connection		c,
											String			columnName,
											String			fileName,
											SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		java.io.Reader	reader = null;
		int	flen = 0;
		if (fileName != null) {
			reader = getFileReader(fileName);
			flen = getFileLength(fileName);
		}
		ps.setCharacterStream(1, reader, flen);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
		if (reader != null) reader.close();
	}

	private void assertSetCharacterStream(	Connection	c,
											String		columnName,
											String		fileName) throws Exception
	{
		assertSetCharacterStream(c, columnName, fileName, null);
	}

	private void assertSetBinaryStream(	Connection		c,
										String			columnName,
										String			fileName,
										SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		java.io.InputStream	stream = null;
		int	flen = 0;
		if (fileName != null) {
			stream = new java.io.FileInputStream(fileName);
			flen = getFileLength(fileName);
		}
		ps.setBinaryStream(1, stream, flen);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
		if (stream != null) stream.close();
	}

	private void assertSetBinaryStream(	Connection	c,
										String		columnName,
										String		fileName) throws Exception
	{
		assertSetBinaryStream(c, columnName, fileName, null);
	}

	private void assertSetArray(	Connection		c,
									String			columnName,
									boolean			isTestArray,
									Object[]		columnValue,
									SQLException	e) throws Exception
	{
		PreparedStatement	ps = getInsertPreparedStatement(c, columnName);
		java.sql.Array	val = null;
		if (isTestArray) {
			val = new TestArray(columnValue);
		} else {
			val = new jp.co.ricoh.doquedb.jdbc.Array(columnValue);
		}
		ps.setArray(1, val);
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int expected = 1;
		if (e == null)	assertEquals(expected, ps.executeUpdate());
		else			assertExecuteUpdateCatchException(ps, e);
		ps.close();
	}

	private void assertSetArray(	Connection	c,
									String		columnName,
									boolean		isTestArray,
									Object[]	columnValue) throws Exception
	{
		assertSetArray(c, columnName, isTestArray, columnValue, null);
	}

	private String getSelectQuery(String	columnName)
	{
		return "select " + columnName + " from t where " + columnName + " is not null order by f_int1";
	}

	private void assertGetInt(	Connection			c,
								String				columnName,
								java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			assertEquals(((Integer)anss.elementAt(i)).intValue(), rs.getInt(1));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetInt(	Connection	c,
								String		columnName,
								int			ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(new Integer(ans));
		assertGetInt(c, columnName, anss);
	}

	private void assertGetInt(	Connection	c,
								String		columnName) throws Exception
	{
		assertGetInt(c, columnName, new java.util.Vector());
	}

	private void assertGetLong(	Connection			c,
								String				columnName,
								java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			assertEquals(((Long)anss.elementAt(i)).longValue(), rs.getLong(1));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetLong(	Connection	c,
								String		columnName,
								long		ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(new Long(ans));
		assertGetLong(c, columnName, anss);
	}

	private void assertGetLong(	Connection	c,
								String		columnName) throws Exception
	{
		assertGetLong(c, columnName, new java.util.Vector());
	}

	private void assertGetBigDecimal(	Connection			c,
										String				columnName,
										java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			assertEquals(((BigDecimal)anss.elementAt(i)), rs.getBigDecimal(1));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetBigDecimal(	Connection	c,
										String		columnName,
										BigDecimal	ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(ans);
		assertGetBigDecimal(c, columnName, anss);
	}

	private void assertGetBigDecimal(	Connection	c,
										String		columnName) throws Exception
	{
		assertGetBigDecimal(c, columnName, new java.util.Vector());
	}

	private void assertGetString(	Connection			c,
									String				columnName,
									java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			assertEquals((String)anss.elementAt(i), rs.getString(1));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetString(	Connection	c,
									String		columnName,
									String		ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(ans);
		assertGetString(c, columnName, anss);
	}

	private void assertGetString(	Connection	c,
									String		columnName) throws Exception
	{
		assertGetString(c, columnName, new java.util.Vector());
	}

	private void assertGetFloat(	Connection			c,
									String				columnName,
									java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			assertEquals((Float)anss.elementAt(i), new Float(rs.getFloat(1)));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetFloat(	Connection	c,
									String		columnName,
									float		ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(new Float(ans));
		assertGetFloat(c, columnName, anss);
	}

	private void assertGetFloat(	Connection	c,
									String		columnName) throws Exception
	{
		assertGetFloat(c, columnName, new java.util.Vector());
	}

	private void assertGetTimestamp(	Connection			c,
										String				columnName,
										java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			String	ans = (String)anss.elementAt(i);
			assertZero(rs.getTimestamp(1).compareTo(java.sql.Timestamp.valueOf(ans)));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetTimestamp(	Connection	c,
										String		columnName,
										String		ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(ans);
		assertGetTimestamp(c, columnName, anss);
	}

	private void assertGetTimestamp(	Connection	c,
										String		columnName) throws Exception
	{
		assertGetTimestamp(c, columnName, new java.util.Vector());
	}

	private void assertGetBytes(	Connection			c,
									String				columnName,
									java.util.Vector	anss) throws Exception
	{
		// バイト列の列への is not null 検索は v15.0 から
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			byte[]	ans = (byte[])anss.elementAt(i);
			assertEquals(ans, rs.getBytes(1));
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetBytes(	Connection	c,
									String		columnName,
									byte[]		ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(ans);
		assertGetBytes(c, columnName, anss);
	}

	private void assertGetBytes(	Connection	c,
									String		columnName) throws Exception
	{
		assertGetBytes(c, columnName, new java.util.Vector());
	}

	private void assertGetArray(	Connection			c,
									String				columnName,
									java.util.Vector	anss,
									int					doquedbDataType) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			java.sql.Array	ary = rs.getArray(1);
			switch (doquedbDataType) {
			case DataType.INTEGER:
				{
					Integer[]	ans = (Integer[])anss.elementAt(i);
					Integer[]	intAry = (Integer[])ary.getArray();
					assertEquals(ans.length, intAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(intAry[j]);
						} else {
							assertNotNull(intAry[j]);
							assertEquals(ans[j].intValue(), intAry[j].intValue());
						}
					}
				}
				break;
			case DataType.INTEGER64:
				{
					Long[]	ans = (Long[])anss.elementAt(i);
					Long[]	longAry = (Long[])ary.getArray();
					assertEquals(ans.length, longAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(longAry[j]);
						} else {
							assertNotNull(longAry[j]);
							assertEquals(ans[j].longValue(), longAry[j].longValue());
						}
					}
				}
				break;
			case DataType.STRING:
				{
					String[]	ans = (String[])anss.elementAt(i);
					String[]	strAry = (String[])ary.getArray();
					assertEquals(ans.length, strAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(strAry[j]);
						} else {
							assertNotNull(strAry[j]);
							assertEquals(ans[j], strAry[j]);
						}
					}
				}
				break;
			case DataType.FLOAT:
				{
					Double[]	ans = (Double[])anss.elementAt(i);
					Double[]	dblAry = (Double[])ary.getArray();
					assertEquals(ans.length, dblAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(dblAry[j]);
						} else {
							assertNotNull(dblAry[j]);
//							assertEquals(ans[j].doubleValue(), dblAry[j].doubleValue());
							assertEquals(ans[j].doubleValue(), dblAry[j].floatValue());
						}
					}
				}
				break;
			case DataType.DOUBLE:
				{
					Double[]	ans = (Double[])anss.elementAt(i);
					Double[]	dblAry = (Double[])ary.getArray();
					assertEquals(ans.length, dblAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(dblAry[j]);
						} else {
							assertNotNull(dblAry[j]);
							assertEquals(ans[j].doubleValue(), dblAry[j].doubleValue());
						}
					}
				}
				break;
			case DataType.DATE_TIME:
				{
					java.sql.Timestamp[]	ans = (java.sql.Timestamp[])anss.elementAt(i);
					java.sql.Timestamp[]	tsAry = (java.sql.Timestamp[])ary.getArray();
					assertEquals(ans.length, tsAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(tsAry[j]);
						} else {
							assertNotNull(tsAry[j]);
							assertEquals(ans[j].toString(), tsAry[j].toString());
						}
					}
				}
				break;
			case DataType.BINARY:
				{
					byte[][]	ans = (byte[][])anss.elementAt(i);
					byte[][]	btAry = (byte[][])ary.getArray();
					assertEquals(ans.length, btAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(btAry[j]);
						} else {
							assertNotNull(btAry[j]);
							byte[]	ansElm = ans[j];
							byte[]	btElm = btAry[j];
							assertEquals(ansElm.length, btElm.length);
							for (int k = 0; k < ansElm.length; k++) {
								assertEquals(ansElm[k], btElm[k]);
							}
						}
					}
				}
				break;
			case DataType.LANGUAGE:
				{
					LanguageData[]	ans = (LanguageData[])anss.elementAt(i);
					LanguageData[]	langAry = (LanguageData[])ary.getArray();
					assertEquals(ans.length, langAry.length);
					for (int j = 0; j < ans.length; j++) {
						if (ans[j] == null) {
							assertNull(langAry[j]);
						} else {
							assertNotNull(langAry[j]);
							assertTrue(ans[j].equals(langAry[j]));
						}
					}
				}
				break;
			default:
				break;
			}
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetArray(	Connection	c,
									String		columnName) throws Exception
	{
		assertGetArray(c, columnName, new java.util.Vector(), DataType.UNDEFINED);
	}

	private void assertGetDate(	Connection			c,
								String				columnName,
								java.util.Vector	anss) throws Exception
	{
		Statement	s = c.createStatement();
		ResultSet	rs = s.executeQuery(getSelectQuery(columnName));
		for (int i = 0; i < anss.size(); i++) {
			assertTrue(rs.next());
			java.sql.Date	ans = (java.sql.Date)anss.elementAt(i);
			assertEquals(ans.toString(), rs.getDate(1).toString());
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertGetDate(	Connection		c,
								String			columnName,
								java.sql.Date	ans) throws Exception
	{
		java.util.Vector	anss = new java.util.Vector();
		anss.add(ans);
		assertGetDate(c, columnName, anss);
	}

	private void assertGetDate(	Connection	c,
								String		columnName) throws Exception
	{
		assertGetDate(c, columnName, new java.util.Vector());
	}

	private String readString(String	fileName) throws Exception
	{
		java.io.File	f = new java.io.File(fileName);
		int	flen = (int)f.length();
		java.io.FileReader	reader = new java.io.FileReader(fileName);
		char[]	buff = new char[flen];
		reader.read(buff);
		String	result = String.valueOf(buff);
		reader.close();
		return result;
	}

	private String readString(	String	fileName,
								int		len) throws Exception
	{
		java.io.File	f = new java.io.File(fileName);
		int	flen = (int)f.length();
		if (len > flen) len = flen;
		java.io.FileReader	reader = new java.io.FileReader(fileName);
		char[]	buff = new char[len];
		reader.read(buff, 0, len);
		String	result = String.valueOf(buff);
		reader.close();
		return result;
	}

	private String readString(	String	fileName,
								String	charSet) throws Exception
	{
		java.io.File	f = new java.io.File(fileName);
		int	flen = (int)f.length();
		java.io.InputStream	stream = new java.io.FileInputStream(fileName);
		java.io.InputStreamReader	reader = new java.io.InputStreamReader(stream, charSet);
		char[]	buff = new char[flen];
		int	rlen = reader.read(buff, 0, flen);
		String	result = String.valueOf(buff, 0, rlen);
		reader.close();
		return result;
	}

	private String readUTF8(String	fileName) throws Exception
	{
		return readString(fileName, "UTF-8");
	}

	private String readUTF8(	String	fileName,
								int		len) throws Exception
	{
		String	result = readString(fileName, "UTF-8");
		if (len < result.length()) result = result.substring(0, len);
		return result;
	}

	private byte[] readBinary(	String	fileName,
								int		len) throws Exception
	{
		byte[]	result = new byte[len];
		for (int i = 0; i < len; i++) result[i] = (byte)0;
		java.io.File	f = new java.io.File(fileName);
		int	flen = (int)f.length();
		if (flen == 0) return result;
		if (len > flen) len = flen;
		java.io.InputStream	stream = new java.io.FileInputStream(fileName);
		stream.read(result, 0, len);
		stream.close();
		return result;
	}

	private byte[] readBinary(String	fileName) throws Exception
	{
		java.io.File	f = new java.io.File(fileName);
		int	flen = (int)f.length();
		byte[]	result = new byte[flen];
		java.io.InputStream	stream = new java.io.FileInputStream(fileName);
		stream.read(result);
		stream.close();
		return result;
	}

	private java.io.Reader getFileReader(String	fileName) throws Exception
	{
		java.io.InputStream	stream = new java.io.FileInputStream(fileName);
		return new java.io.InputStreamReader(stream);
	}

	private byte[] stringToBytes(String	src) throws Exception
	{
		return stringToBytes(src, src.length() * 2);
	}

	private byte[] stringToBytes(	String	src,
									int		len) throws Exception
	{
		byte[]	result = new byte[len];
		for (int i = 0; i < len; i++) result[i] = (byte)0;
		if (src == null) return result;
		char[]	chars = src.toCharArray();
		int	idx = 0;
		for (int i = 0; i < chars.length; i++) {
			if (idx >= len) break;
			int	chr = (int)chars[i];
			result[idx++] = (byte)chr;
			if (idx >= len) break;
			chr >>= 8;
			result[idx++] = (byte)chr;
		}
		return result;
	}

	private String appendSpace(	String	src,
								int		newLen) throws Exception
	{
		int	len = src.length();
		if (len >= newLen) return src;
		StringBuffer	result = new StringBuffer(src);
		for (int i = len; i < newLen; i++) result.append(" ");
		return result.toString();
	}

	private byte[] appendNull(	byte[]	src,
								int		newLen) throws Exception
	{
		int	len = src.length;
		if (len >= newLen) return src;
		byte[]	result = new byte[newLen];
		int	i;
		for (i = 0; i < src.length; i++) result[i] = src[i];
		for (     ; i < newLen    ; i++) result[i] = 0x00;
		return result;
	}

	private void createSimpleTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("create table t (id int)");
		s.close();
	}

	private void assertExecuteUpdateCatchException(	PreparedStatement	ps,
													SQLException		e) throws Exception
	{
		String	expected = e.getSQLState();
		String	actual = "";
		try {
			ps.executeUpdate();
		} catch (SQLException	sqle) {
			actual = sqle.getSQLState();
		}
		assertEquals(expected, actual);
	}

	private void assertEquals(byte[] expected, byte[] actual) throws Exception
	{
		assertEquals(expected.length, actual.length);
		for (int i = 0; i < expected.length; i++) assertEquals(expected[i], actual[i]);
	}

	private void assertEquals(int len, byte[] actual) throws Exception
	{
		assertEquals(len, actual.length);
		for (int i = 0; i < len; i++) assertZero(actual[i]);
	}

	private void assertBatchResults(java.util.Vector expected, int[] actual) throws Exception
	{
		assertEquals(expected.size(), actual.length);
		for (int i = 0; i < expected.size(); i++) {
			int	exp = ((Integer)expected.elementAt(i)).intValue();
			assertEquals(exp, actual[i]);
		}
	}

	private void assertNotSupported(SQLException	e) throws Exception
	{
		assertEquals((new NotSupported()).getSQLState(), e.getSQLState());
	}

	private void assertUnexpected(SQLException	e) throws Exception
	{
		assertEquals((new Unexpected()).getSQLState(), e.getSQLState());
	}

	private void assertBadArgument(SQLException	e) throws Exception
	{
		assertEquals((new BadArgument()).getSQLState(), e.getSQLState());
	}

	private void assertSQLSyntaxError(SQLException	e) throws Exception
	{
		assertEquals((new SQLSyntaxError("")).getSQLState(), e.getSQLState());
	}

	private void assertDynamicParameterNotMatch(SQLException e) throws Exception
	{
		assertEquals((new DynamicParameterNotMatch(1,2)).getSQLState(), e.getSQLState());
	}
}

//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
