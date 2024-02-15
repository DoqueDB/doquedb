// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadInsert.java -- 
// 
// Copyright (c) 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.load;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.StringTokenizer;

import jp.co.ricoh.sydney.admin.util.*;
import jp.co.ricoh.sydney.admin.util.exception.AdminException;
import jp.co.ricoh.sydney.admin.util.load.Separator;
import jp.co.ricoh.sydney.admin.util.log.Log;
import jp.co.ricoh.sydney.admin.util.model.ColumnType;

public class LoadInsert
{
	private java.sql.Connection		_conn = null;
	private java.sql.Statement			state = null;
	private java.sql.PreparedStatement	psmt = null;

	private String						_elementSeparator = null;	// ロードファイル内の配列型の列値の要素の区切り文字

	// 列情報用
	private ArrayList<ColumnType>		_columnTypes = null;	// admin.util.model.ColumnType の列

	private String						_loadDir = null;


	public LoadInsert(	Connection	conn_,
						Statement	state_)
	{
		this._conn = conn_;
		this.state = state_;

		this._columnTypes = new ArrayList<ColumnType>();
	}

	public void commit()
		throws SQLException
	{
		this._conn.commit();
	}

	public void rollback()
		throws SQLException
	{
		this._conn.rollback();
	}

	public void loadOpen(	String loadFilePath_,
							String loadTable_,
							String columnSeparator_,
							String elementSeparator_,
							String extern_,
							String charsetName_,
							String insertColumns_)
		throws SQLException, IOException
	{
		ResultSet resultSet = null;
		ResultSetMetaData rsmeta = null;


		try {
			// 2006.07.18
			// 以前は管理ツールの操作全てが「読み書きモード」だったが
			// 問題があり「ReadOnly」に修正した副作用で
			// 「ロード」の場合は「読み書きモード」でないと動作しない為の修正
			this._conn.setReadOnly(false);

			this._conn.setAutoCommit(false);

			java.io.File	f = new File(loadFilePath_);
			this._loadDir = f.getParent();

			this._elementSeparator = elementSeparator_;

			// 列情報のリセット
			this._columnTypes.clear();

			String	sql = "SELECT * FROM \"" + loadTable_ + "\" LIMIT 1";
			Log.trace("sql = \"" + sql + "\"");
			resultSet = this.state.executeQuery(sql);
			rsmeta = resultSet.getMetaData();

			// INSERT INTO TABLE VALUES(?,.....,?)を作成する。
			// 列情報を取得する。
			// insertするカラムを指定する 2010/4/23 対応
			String	loadSql = "insert into \"" + loadTable_ + "\"";
			if ( insertColumns_ != null && insertColumns_.length() > 0 ) {
				loadSql += "(" + insertColumns_ + ") values (";
				ArrayList<String>	columnList = getInsertColumnList(insertColumns_);
				for ( int i = 0 ; i < columnList.size() ; i++ ) {
					for ( int j = 1 ; j <= rsmeta.getColumnCount() ; j++ ) {
						String	cName = rsmeta.getColumnName(j);
						String	cTypeName = rsmeta.getColumnTypeName(j);
						int		type = rsmeta.getColumnType(j);
						if ( cName.equalsIgnoreCase(columnList.get(i)) ) {
							setColumnType(cName, cTypeName, type, extern_, charsetName_);
							loadSql = loadSql + "?,";
							break;
						} else {
							continue;
						}
					}
				}
			} else {
				loadSql +=  "values (";
				for ( int i = 1 ; i <= rsmeta.getColumnCount() ; i++ ) {
					String	cName = rsmeta.getColumnName(i);
					String	cTypeName = rsmeta.getColumnTypeName(i);
					int		type = rsmeta.getColumnType(i);
					setColumnType(cName, cTypeName, type, extern_, charsetName_);
					loadSql = loadSql + "?,";
				}
			}
			loadSql = loadSql.substring(0, loadSql.length() - 1);
			loadSql = loadSql + ")";
			Log.trace("prepare sql = \"" + loadSql + "\"");
			this.psmt = this._conn.prepareStatement(loadSql);

		} finally {

			if (resultSet != null) {
				resultSet.close();
			}
		}

		Log.trace(Res.get("TRACE_END"));
	}
	private ArrayList<String> getInsertColumnList(String insertColumns_)
	{
		ArrayList<String>	columnList = new ArrayList<String>();
		String[] columns = insertColumns_.split(",");
		String buffer = "";
		for ( int i = 0 ; i < columns.length ; i++ ) {
			if ( buffer.length() == 0 ) {
				if ( columns[i].charAt(0) == '"' ) {
					buffer = columns[i].substring(1);
				} else {
					columnList.add(columns[i].trim());
				}
			}
			if ( buffer.length() > 0 ) {
				if ( columns[i].charAt(columns[i].length() - 1) == '"' ) {
					buffer += columns[i].substring(0, columns[i].length() - 2);
					columnList.add(buffer.trim());
					buffer = "";
				}
			}
		}
		return columnList;
	}
	private void setColumnType(String cName, String cTypeName, int type, String extern_, String charsetName_) {
		ColumnType	columnType = new ColumnType();
		columnType.setName(cName);
		columnType.setTypeName(cTypeName);
		columnType.setType(type);
		// 別列情報を取得。
		if (extern_ != null && extern_.length() > 0) {
			StringTokenizer	st = new StringTokenizer(extern_, ",");
			while (st.hasMoreTokens()) {
				String	eName = st.nextToken();
				if (eName.equalsIgnoreCase(cName)) {
					columnType.setSeparate(true);
					columnType.setLoadCode(charsetName_);
					break;
				}
			}
		}
		// 列データ追加とSQL文の構築
		this._columnTypes.add(columnType);
	}
	public void loadClose()
		throws SQLException
	{
		Log.trace(Res.get("TRACE_START"));

		if (psmt != null) {

			try {

				psmt.close();

			} catch (SQLException sqle_) {

				Log.printStackTrace(sqle_);
				Log.warning(sqle_.toString());
			}
		}

		this._conn.setAutoCommit(true);

		Log.trace(Res.get("TRACE_END"));
	}

	public void insertTextRecord(	ArrayList<String>	columnValues)
		throws SQLException, IOException, AdminException
	{
		ColumnType	columnType = null;
		String		value = null;

		for (int j = 0; j < this._columnTypes.size(); j++) {

			columnType = (ColumnType)this._columnTypes.get(j);

			value = columnValues.get(j).toString().trim();

			// 外部データをオブジェクトとして取得
			if (columnType.isSeparate()) {
				externInsert(j + 1, value.toString(), columnType);
			} else {

				if ((value != null) && value.equals("") == true)
					this.psmt.setNull(j + 1, columnType.getType());
				else if ( value.equalsIgnoreCase("null") == true)
					this.psmt.setNull(j + 1, columnType.getType());
				else if (columnType.getType() == java.sql.Types.ARRAY) {
					if (this._elementSeparator == null	|| this._elementSeparator.length() == 0)
						throw new AdminException(ErrorReason.NO_ARRAY_ELEMENT_SEPARATOR);
					this.setArray(j + 1, StringSplit.split(value, this._elementSeparator), columnType);
				} else {
					this.psmt.setObject(j + 1, value, columnType.getType());
				}
			}
		} // end for

		this.psmt.executeUpdate();
	}

	public void insertXMLRecord(ArrayList<LoadItem>	list_,
								String		charsetName_)
		throws SQLException, IOException, AdminException
	{
		ColumnType	columnType = null;
		LoadItem	item = null;
		String		value = null;

		int	itemSkipNum = 0;

		for (int j = 0; j < this._columnTypes.size(); j++) {

			columnType = (ColumnType)this._columnTypes.get(j);

			item = (LoadItem)list_.get(j + itemSkipNum);
			value = item.getValue();

			int	columnDataType = columnType.getType();	// java.sql.Types.*
			if (columnDataType == java.sql.Types.ARRAY) {

				if (columnType.getName().equals("element") == false) {

					// element という列名だったら以下のチェックはできないが、
					// そうでないのならちゃんと配列型の列への挿入用のフォーマットになっているか
					// チェックする。

					if (columnType.getName().equals(item.getName()) && value != null && value.length() > 0) {

						AdminException	adminException = new AdminException(ErrorReason.NOT_SET_ELEMENT);
						adminException.setCauseObject(columnType.getName());
						throw adminException;
					}
				}

			} else {

				if (columnType.getName().equals("element") == false) {

					// 同上

					if (item.getName().equals("element")) {

						AdminException	adminException = new AdminException(ErrorReason.NOT_ARRAY_COLUMN);
						adminException.setCauseObject(columnType.getName());
						throw adminException;
					}
				}
			}

			if (columnType.isSeparate()) {

				if (columnDataType == java.sql.Types.ARRAY) {

					itemSkipNum += this.externInsert(list_, j + 1, j + itemSkipNum, columnType, charsetName_);

				} else {

					this.externInsert(j + 1, value, columnType);
				}

			} else {

				if (columnDataType == java.sql.Types.ARRAY) {

					itemSkipNum += this.setArray(list_, j + 1, j + itemSkipNum, columnType);

				} else {

					if ((value == null) || (value.length() == 0)) {
						this.psmt.setNull(j + 1, columnDataType);
					} else {
						this.psmt.setObject(j + 1, value, columnDataType);
					}
				}
			}
		}

		this.psmt.executeUpdate();
	}

	private int setArray(	ArrayList<LoadItem>	loadItems_,
							int			columnIndex_,
							int			loadItemStartIndex_,
							ColumnType	columnType_)
		throws SQLException, AdminException
	{
		LoadItem	loadItem = (LoadItem)loadItems_.get(loadItemStartIndex_);
		String	elementItemName = loadItem.getName();

		if (elementItemName.equals("element") == false) {
			this.psmt.setNull(columnIndex_, java.sql.Types.ARRAY);
			return 0;
		}
		// count elements
		int	i = loadItemStartIndex_;
		int	n = 0;
		while (loadItem.getName().equals("element")) {
			++n;
			loadItem = (LoadItem)loadItems_.get(++i);
		}
		// prepare string array
		String[] columnValue = new String[n];
		// scan again
		loadItem = (LoadItem)loadItems_.get(loadItemStartIndex_);
		i = loadItemStartIndex_;
		n = 0;

		while (loadItem.getName().equals("element")) {

			String	elementValue = loadItem.getValue();
			if (elementValue == null || elementValue.length() == 0) elementValue = "null";
			columnValue[n++] = elementValue;

			loadItem = (LoadItem)loadItems_.get(++i);
		}
		this.setArray(columnIndex_, columnValue, columnType_);

		return i - loadItemStartIndex_;
	}

	private void setArray(	int			columnIndex_,	// 1 〜
							String[]	elementValue_,
							ColumnType	columnType_)
		throws SQLException, AdminException
	{
		String	elementTypeName = columnType_.getTypeName();
		String	elementType = elementTypeName.substring(0, elementTypeName.indexOf(" ")).toLowerCase();

		Object[]	columnValue = null;
		if (elementType.equals("int")) {
			columnValue = getIntegerArray(elementValue_);
		} else if (elementType.equals("bigint")) {
			columnValue = getLongArray(elementValue_);
		} else if (elementType.equals("float")) {
			columnValue = getDoubleArray(elementValue_);
		} else if (elementType.equals("datetime")) {
			columnValue = getDateTimeArray(elementValue_);
		} else if (elementType.equals("language")) {
			columnValue = getLanguageArray(elementValue_);
		} else if (elementType.indexOf("char") >= 0) {
			columnValue = getStringArray(elementValue_);
		} else if (elementType.equals("decimal")) {
			columnValue = getDecimalArray(elementValue_);
		} else {
			AdminException	adminException = new AdminException(ErrorReason.NOT_SUPPORTED_ELEMENT_DATA_TYPE);
			adminException.setCauseObject(elementType);
			throw adminException;
		}

		this.psmt.setArray(columnIndex_, new jp.co.ricoh.doquedb.jdbc.Array(columnValue));
	}

	private Object[] getIntegerArray(String[]	elements_)
	{
		Integer[]	elements = new Integer[elements_.length];

		for (int i = 0; i < elements_.length; i++) {
			if (elements_[i].compareToIgnoreCase("null") == 0) {
				elements[i] = null;
			} else {
				elements[i] = new Integer(elements_[i]);
			}
		}

		return elements;
	}

	private Object[] getLongArray(String[]	elements_)
	{
		Long[]	elements = new Long[elements_.length];

		for (int i = 0; i < elements_.length; i++) {
			if (elements_[i].compareToIgnoreCase("null") == 0) {
				elements[i] = null;
			} else {
				elements[i] = new Long(elements_[i]);
			}
		}

		return elements;
	}

	private Object[] getDoubleArray(String[]	elements_)
	{
		Double[]	elements = new Double[elements_.length];

		for (int i = 0; i < elements_.length; i++) {
			if (elements_[i].compareToIgnoreCase("null") == 0) {
				elements[i] = null;
			} else {
				elements[i] = new Double(elements_[i]);
			}
		}

		return elements;
	}

	private Object[] getDateTimeArray(String[]	elements_)
	{
		return this.getStringArray(elements_);
	}

	private Object[] getLanguageArray(String[]	elements_)
	{
		return this.getStringArray(elements_);
	}

	private Object[] getStringArray(String[]	elements_)
	{
		String[]	elements = new String[elements_.length];

		for (int i = 0; i < elements_.length; i++) {
			if (elements_[i].compareToIgnoreCase("null") == 0) {
				elements[i] = null;
			} else {
				String	element = elements_[i];
				// 2008/3/5 削除
//				if (element.startsWith("'")) element = element.substring(1);
//				if (element.endsWith("'")) element = element.substring(0, element.length() - 1);
				elements[i] = new String(element);
			}
		}

		return elements;
	}

	private Object[] getDecimalArray(String[]	elements_)
	{
		BigDecimal[]	elements = new BigDecimal[elements_.length];

		for (int i = 0; i < elements_.length; i++) {
			if (elements_[i].compareToIgnoreCase("null") == 0) {
				elements[i] = null;
			} else {
				elements[i] = new BigDecimal(elements_[i]);
			}
		}

		return elements;
	}

	private int externInsert(	ArrayList<LoadItem>	loadItems_,
								int			columnIndex_,
								int			loadItemStartIndex_,
								ColumnType	columnType_,
								String		charsetName_)
		throws SQLException, IOException, AdminException
	{
		LoadItem	loadItem = (LoadItem)loadItems_.get(loadItemStartIndex_);
		String	elementItemName = loadItem.getName();

		if (elementItemName.equals("element") == false) {
			this.psmt.setNull(columnIndex_, java.sql.Types.ARRAY);
			return 0;
		}

		String	externFilePaths = "";
		int	i = loadItemStartIndex_;
		while (loadItem.getName().equals("element")) {

			String	externFilePath = loadItem.getValue();
			if (externFilePath == null || externFilePath.length() == 0) externFilePath = "null";
			externFilePaths = externFilePaths + externFilePath;

			i++;

			loadItem = (LoadItem)loadItems_.get(i);

			if (loadItem.getName().equals("element")) externFilePaths = externFilePaths + ",";
		}

		this._elementSeparator = ",";
		this.externInsert(columnIndex_, externFilePaths, columnType_);

		return i - loadItemStartIndex_;
	}

	private void externInsert(	int			columnIndex_,
								String		file_,
								ColumnType	columnType_)
		throws SQLException, IOException, AdminException
	{
		int	dataType = columnType_.getType();	// java.sql.Types で定義されている定数の値
		if (file_ == null || file_.length() == 0) {
			this.psmt.setNull(columnIndex_, dataType);
		} else {
			switch (dataType) {
			case java.sql.Types.BINARY:
			case java.sql.Types.VARBINARY:
			case java.sql.Types.BLOB:
				binaryInsert(columnIndex_, file_);
				break;
			case java.sql.Types.ARRAY:
				arrayInsert(columnIndex_, file_, columnType_);
				break;
			default:
				stringInsert(columnIndex_, file_, columnType_);
				break;
			}
		}
	}

	private void arrayInsert(	int			columnIndex_,
								String		externFilePath_,
								ColumnType	columnType_)
		throws SQLException, IOException, AdminException
	{
		if (this._elementSeparator == null || this._elementSeparator.length() == 0) throw new AdminException(ErrorReason.NO_ARRAY_ELEMENT_SEPARATOR);

		String	elementTypeName = columnType_.getTypeName();
		String	elementType = elementTypeName.substring(0, elementTypeName.indexOf(" ")).toLowerCase();
		Object[]	columnValue = null;

		if (elementType.equals("int")) {
			columnValue = getIntegerArrayExtern(columnType_, externFilePath_);
		} else if (elementType.equals("bigint")) {
			columnValue = getLongArrayExtern(columnType_, externFilePath_);
		} else if (elementType.equals("float")) {
			columnValue = getDoubleArrayExtern(columnType_, externFilePath_);
		} else if (elementType.equals("datetime")) {
			columnValue = getDateTimeArrayExtern(columnType_, externFilePath_);
		} else if (elementType.equals("language")) {
			columnValue = getLanguageArrayExtern(columnType_, externFilePath_);
		} else if (elementType.indexOf("char") >= 0) {
			columnValue = getStringArrayExtern(columnType_, externFilePath_);
		} else if (elementType.indexOf("binary") >=0) {
			columnValue = getBinaryArrayExtern(externFilePath_.split(this._elementSeparator));
		} else if (elementType.equals("decimal")) {
			columnValue = getDecimalArrayExtern(columnType_, externFilePath_);
		} else {
			AdminException	adminException = new AdminException(ErrorReason.NOT_SUPPORTED_ELEMENT_DATA_TYPE);
			adminException.setCauseObject(elementType);
			throw adminException;
		}
		for (int i = 0 ; i < columnValue.length ; i++ )
		{
			if ( ((String)columnValue[i]).length() == 0 )
				columnValue[i] = null;
		}
		jp.co.ricoh.doquedb.jdbc.Array array = new jp.co.ricoh.doquedb.jdbc.Array(columnValue);
		this.psmt.setArray(columnIndex_, array);
	}



	private byte[] readBytes(String	filePath_)
		throws IOException
	{
		byte[]	result = null;

		java.io.FileInputStream	inStream = null;

		try {

			java.io.File	f = new java.io.File(filePath_);
			int	len = (int)f.length();

			result = new byte[len];
			inStream = new java.io.FileInputStream(f);
			inStream.read(result);

		} finally {

			if (inStream != null) inStream.close();
		}

		return result;
	}
	private Object[] getElements(BufferedReader loadFileReader_) throws IOException
	{
		Separator element = new Separator(java.lang.System.getProperty("line.separator"), this._elementSeparator, loadFileReader_);
		return element.getData().toArray();
	}
	private Object[] getIntegerArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		columnType_.setReader(externFilePath_);
		BufferedReader reader = columnType_.getReader();
		return getElements(reader);
	}

	private Object[] getLongArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		return getIntegerArrayExtern(columnType_, externFilePath_);
	}

	private Object[] getDoubleArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		return getIntegerArrayExtern(columnType_, externFilePath_);
	}

	private Object[] getDateTimeArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		return getIntegerArrayExtern(columnType_, externFilePath_);
	}

	private Object[] getLanguageArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		return getIntegerArrayExtern(columnType_, externFilePath_);
	}

	private Object[] getStringArrayExtern(	ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		columnType_.setReader(externFilePath_);
		BufferedReader reader = columnType_.getReader();

		return getElements(reader);
	}

	private Object[] getBinaryArrayExtern(String[]	externFilePaths_)
		throws IOException
	{
		Object[]	columnValue = new Object[externFilePaths_.length];
		for (int i = 0; i < externFilePaths_.length; i++) {

			String	externFilePath = externFilePaths_[i];
			columnValue[i] = (externFilePath.equals("null") ? null : this.readBytes(externFilePath));
		}

		return columnValue;
	}

	private Object[] getDecimalArrayExtern(ColumnType	columnType_, String externFilePath_)
		throws IOException
	{
		return getIntegerArrayExtern(columnType_, externFilePath_);
	}

	private void binaryInsert(	int		index_,
								String	file_)
		throws SQLException, IOException
	{
		InputStream	inStream = null;

		try {

			java.io.File	f = FileUtil.getFileHandler(this._loadDir, file_);
			inStream = new java.io.FileInputStream(f);
			int len = (int)f.length();
			this.psmt.setBinaryStream(index_, inStream, len);
		} finally {

			if (inStream != null) inStream.close();
		}
	}
	private void stringInsert(	int	index_,	String	file_, ColumnType columnType_) throws SQLException, IOException
	{
		columnType_.setReader(file_);
		BufferedReader reader = columnType_.getReader();
		int	c = -1;
		StringBuffer column =  new StringBuffer();
		try {
			while( (c = reader.read()) != -1 ){
				// ファイルの先頭に EF BB BF が入ってる場合の対応 2009/6/23 
				if ( c == 65279 && column.length() == 0 )
					continue;
				column.append((char)c);
			} // end while
		} finally {
			if (reader != null && c == -1 )
				reader.close();
		}

		this.psmt.setString(index_, column.toString());
	}
}

//
// Copyright (c) 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
