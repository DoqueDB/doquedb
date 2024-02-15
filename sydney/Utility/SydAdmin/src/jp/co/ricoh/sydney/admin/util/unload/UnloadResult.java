// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnloadResult.java -- 
// 
// Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.unload;

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactoryConfigurationError;

import jp.co.ricoh.sydney.admin.util.model.ColumnType;
import jp.co.ricoh.sydney.admin.util.Res;
import jp.co.ricoh.sydney.admin.util.StringSplit;
import jp.co.ricoh.sydney.admin.util.log.Log;
import jp.co.ricoh.sydney.admin.util.xml.XMLUtil;

import org.w3c.dom.DOMException;
import org.w3c.dom.Element;

public class UnloadResult
{
	// 外部列情報を格納するインナークラス
	private class ExternModel
	{
		private String	item = null;
		private String	ext = null;	// 拡張子

		public ExternModel(	String	item_,
							String	ext_)
		{
			this.item = item_;
			this.ext = ext_;
		}

		public String getExt()
		{
			return this.ext;
		}

		public String getItem()
		{
			return this.item;
		}
	}

	private static final String	LINE_SEPARATOR = System.getProperty("line.separator");
	private static final String	COLUMN_SEPARATOR = "\t";
	private static final String	ELEMENT_SEPARATOR = ",";
	private static final String	RECORD_SEPARATOR_STR = "\\n";//default

	private Statement			state = null;
 
	private String				dir = null;
	private String				fileName = null;
	private String				tableName = null;
 
	private boolean			unloadType = false;	// true:xml false:text
	private String				unloadExt = ".dat";
 
	// 列情報用
	private ArrayList<ColumnType>			columnList = null;
	// 別列情報
	private ArrayList<ExternModel>			externList = null;

	private ResultSet			resultSet = null;

	// テキスト用
	private FileOutputStream	fos = null;
	private OutputStreamWriter	osw = null;
	private BufferedWriter		bw = null;
	// XML用
	private XMLUtil				xml = null;

	// アンロード用のカウンター
	private int					recordCount = 1;

	// 
	private String				xmlRootName = "sydney";
	private String				xmlHeader = "header";
	private String				xmlRecord = "record";
	private String				xmlElement = "element";

	private String				unloadCode = null;
	private final static String	_defaultUnloadCode = "UTF-8"; // default charset

	private int					unloadCnt = 1;

 	/**
	 * アンロード先や出力方式を決定する。
	 * @param state_
	 * @param dir_
	 * @param unloadType_
	 */
	public UnloadResult(Statement	state_,
						String		dir_,
						boolean		unloadType_)
	{
		this.state = state_;
		this.dir = dir_;
		this.unloadType = unloadType_;
		this.columnList = new ArrayList<ColumnType>();
		this.externList = new ArrayList<ExternModel>();

		if(this.dir.equalsIgnoreCase("/")!=true){
			String s = System.getProperty("file.separator");
			if (this.dir.endsWith(s) != true) {
				this.dir = this.dir + s;
			}
		}

		// XML方式の場合拡張子をxmlにする。
		// テキストはdat。
		if (this.unloadType) {
			this.unloadExt = ".xml";
		}
		this.unloadCode = _defaultUnloadCode;
	}

	/**
	 * アンロード先や出力方式を決定する。
	 * @param state_
	 * @param dir_
	 * @param unloadType_
	 * @param unloadCode_
	 */
	public UnloadResult(Statement	state_,
						String		dir_,
						boolean		unloadType_,
						String		unloadCode_)
	{
		this(state_, dir_, unloadType_);

		this.unloadCode = unloadCode_;
 	}

	public String getTableName()
	{
		return this.tableName;
	}

	public String getFileName()
	{
		return this.tableName + "." + this.unloadCnt + this.unloadExt;
	}

	/**
	 * アンロードを行う為の初期化処理
	 * @param table_
	 * @return true:成功 false:失敗
	 * @throws DOMException
	 * @throws ParserConfigurationException
	 * @throws FactoryConfigurationError
	 * @throws FileNotFoundException
	 * @throws SQLException
	 * @throws IOException
	 * @throws TransformerFactoryConfigurationError
	 * @throws TransformerException
	 */
	public int open(String	table_)
		throws
			DOMException, ParserConfigurationException, FactoryConfigurationError,
			FileNotFoundException, SQLException, IOException,
			TransformerFactoryConfigurationError, TransformerException
	{
		int		count = 0;
		boolean	status = false;

		Log.trace(Res.get("TRACE_START"));

		// 初期化処理
		this.recordCount = 1;

		try{
			// 別列情報の取得
			this.tableName = getTableExtern(table_);
			this.unloadCnt = getUnloadCounter();
			this.fileName = this.dir + this.tableName + "." + this.unloadCnt + this.unloadExt;
			// 列情報の取得
			// ここでレコード数を取得する。
			count = getTableColumn();
			// ヘッダー情報の書き込み
			if (this.unloadType == false) {
				writeTableHeader();
			} else {
				writeTableXMLHeader();
			}
			status = true;
		} finally {
 			if (status == false) {
 				close();
 			}
		}

		Log.trace(Res.get("TRACE_END"));

		return count;
	}

	private class UnloadFileFilter implements java.io.FileFilter
	{
		String	tableName = null;

		public UnloadFileFilter(String	tableName_)
		{
			super();
			this.tableName = tableName_.toLowerCase() + ".";
		}

		public boolean accept(java.io.File	f_)
		{
			if (f_.isDirectory()) return false;

			String	fileName = f_.getName().toLowerCase();
			return fileName.startsWith(this.tableName) && (fileName.endsWith(".dat") || fileName.endsWith(".xml"));
		}
	}

	private int getUnloadCounter()
	{
		int	cnt = 1;

		java.io.File	outputDir = new java.io.File(this.dir);
		java.io.File[]	files = outputDir.listFiles(new UnloadFileFilter(this.tableName));

		for (int i = 0; i < files.length; i++) {
			String	fn = files[i].getName();
			int	startIndex = fn.indexOf(".", 0);
			if (startIndex > 0 && startIndex < fn.length()) {
				int	endIndex = fn.indexOf(".", startIndex + 1);
				if (endIndex > 0) {
					try {
						String	cntStr = files[i].getName().substring(startIndex + 1, endIndex);
						int	c = Integer.parseInt(cntStr);
						if (c >= cnt) cnt = c + 1;
					} catch (NumberFormatException	nfe_) {

						; // 数値でないんなら無視していいファイル
					}
				}
			}
		}
		return cnt;
	}

	/**
	 * １レコードを出力（UNLOAD）する。
	 * @return true:成功 false:失敗
	 * @throws FileNotFoundException
	 * @throws SQLException
	 * @throws IOException
	 * @throws TransformerFactoryConfigurationError
	 * @throws TransformerException
	 */
	public boolean writeRecord()
		throws
			FileNotFoundException, SQLException, IOException,
			TransformerFactoryConfigurationError, TransformerException
	{
		boolean	ret = this.resultSet.next();

		if (ret != true) {
 			return ret;
		}

		if (this.unloadType == false) {
 			writeRecordTable();
 			
		}else{
 			writeRecordTableXML();
		}

		return ret;
	}

	public void close()
		throws
			SQLException, IOException, FileNotFoundException,
			TransformerFactoryConfigurationError, TransformerException
	{
		Log.trace(Res.get("TRACE_START"));
		
		for (int i = 0; i < this.columnList.size(); i++) {
			((ColumnType)this.columnList.get(i)).closeWriter();
		}

		// UNLOAD用リザルトセット
		if (this.resultSet != null) {
			this.resultSet.close();
			this.resultSet = null;
		}

		// テキスト用
		if (this.bw != null) {
			this.bw.close();
			this.bw = null;
		}
		if (this.osw != null) {
			this.osw.close();
			this.osw = null;
		}
		if (this.fos != null) {
			this.fos.close();
			this.fos = null;
		}

		// XML用
		if (this.xml != null) {
			this.xml.write(this.fileName, this.unloadCode);
		}

		Log.trace(Res.get("TRACE_END"));
	}

	/**
	 * 外部列情報を解析しリストに設定する。
	 * TABLE(COL1:JPG,COL2:PDF)
	 * @param table_ テーブル名
	 * @return テーブル名
	 */
	private String getTableExtern(String	table_)
	{
		String	table = null;

		Log.trace(Res.get("TRACE_START"));

		// テーブル名のセット
		table = table_;

		// 別列情報のリセット
		this.externList.clear();

		// table名の調整
		int		startIndex;
		int		endIndex;
		String	temp = table_;
		startIndex = temp.indexOf("(");
		if (startIndex > 0) {
			endIndex = temp.indexOf(")");
			if (endIndex > 0) {
				table = table_.substring(0, startIndex); 
				temp = temp.substring(startIndex + 1, endIndex);
				String[]	st = StringSplit.split(temp, ",");
				for (int j = 0; j < st.length; j++) {
					int			index = st[j].indexOf(":");
					String		item = st[j].substring(0, index);
					String		ext = st[j].substring(index + 1);
					ExternModel	model = new ExternModel(item, ext);
					this.externList.add(model);
				}
			}
		}

		Log.trace(Res.get("TRACE_END"));

		return table;
	}
 
	/**
	 * 列情報や外部列情報を取得する。
	 * @return レコード数
	 * @throws SQLException
	 */
	private int getTableColumn()
		throws SQLException
	{
		int	count = 0;

		Log.trace(Res.get("TRACE_START")); //$NON-NLS-1$
		Log.trace("table=" + this.tableName);

		// 列情報のリセット
		this.columnList.clear();

		String	sql = "SELECT COUNT(*) FROM \"" + this.tableName + "\"";
		this.resultSet = this.state.executeQuery(sql);
		this.resultSet.next();
		count = this.resultSet.getInt(1);

		// 外部データ列情報の取得
		sql = "SELECT * FROM \"" + this.tableName + "\"";
		Log.trace("writeTable sql=" + sql);
		this.resultSet = this.state.executeQuery(sql);
		ResultSetMetaData	rsmeta = this.resultSet.getMetaData();

		for (int i = 1; i <= rsmeta.getColumnCount(); i++) {

			ColumnType	columnType = new ColumnType();
			String		cName = rsmeta.getColumnName(i);
			String		cTypeName = rsmeta.getColumnTypeName(i);
			int			type = rsmeta.getColumnType(i);
			columnType.setName(cName);
			columnType.setTypeName(cTypeName);
			columnType.setType(type);
			// 外部データが存在する場合
			for (int j = 0; j < this.externList.size(); j++) {
				ExternModel	model = (ExternModel)this.externList.get(j);
				if (cName.equalsIgnoreCase(model.getItem())) {
					columnType.setSeparate(true);
					columnType.setExt(model.getExt());
					break;
				}
			}

			// バイナリ型の列値は自動的に外部データとする Version 1.0.8
			switch (type) {
			case java.sql.Types.BINARY:
			case java.sql.Types.BLOB:
			case java.sql.Types.LONGVARBINARY:
			case java.sql.Types.VARBINARY:
				if (columnType.isSeparate() == false) {
					columnType.setSeparate(true);
					columnType.setExt("dat");
				}
				break;
			}

			// 要素がバイナリ型の列値も自動的に外部データとする
			if (type == java.sql.Types.ARRAY && columnType.isSeparate() == false) {
				String	elementType = cTypeName.substring(0, cTypeName.indexOf(" ")).toLowerCase();
				if (elementType.indexOf("binary") >= 0) {
					columnType.setSeparate(true);
					columnType.setExt("dat");
				}
			}

			// 列データ追加とSQL文の構築
			this.columnList.add(columnType);
		}

		Log.trace(Res.get("TRACE_END")); //$NON-NLS-1$

		return count;
	}
 
	/**
	 * ヘッダー部を出力する。
	 * @throws IOException
	 */
	public void writeTableHeader()
		throws IOException
	{
		this.fos = new FileOutputStream(this.fileName);
		this.osw = new OutputStreamWriter(fos, this.unloadCode);
		this.bw = new BufferedWriter(osw);

		this.bw.write("<header>" + LINE_SEPARATOR);
		this.bw.write("table=" + this.tableName + LINE_SEPARATOR);
		this.bw.write("column_separator=" + COLUMN_SEPARATOR + LINE_SEPARATOR);
		this.bw.write("arrayelement_separator=" + ELEMENT_SEPARATOR + LINE_SEPARATOR);
		StringBuffer	externBuff = new StringBuffer();
		for (int i = 0; i < this.columnList.size(); i++) {
			ColumnType	columnType = (ColumnType)this.columnList.get(i);
			if (columnType.isSeparate()) {
				externBuff.append(columnType.getName() + ",");
			}
		}
		String	externStr = externBuff.toString();
		if (externStr.length() > 0) {
			externStr = externStr.substring(0, externStr.length() - 1);
		}
		this.bw.write("extern=" + externStr + LINE_SEPARATOR);
		this.bw.write("record_separator=" + RECORD_SEPARATOR_STR + LINE_SEPARATOR);
		this.bw.write("</header>" + LINE_SEPARATOR);
	}
 
	private void writeTableXMLHeader()
		throws DOMException, ParserConfigurationException, FactoryConfigurationError
	{
		// カラム数の取得
		int	column = this.columnList.size();

		// 初期化とルートの設定
		this.xml = null;
		this.xml = new XMLUtil(this.xmlRootName);
		Element	rootEle = xml.getRootElement();

		// header部の作成
		Element	headerEle = this.xml.getElementNode(this.xmlHeader);
		this.xml.addElement(rootEle, headerEle);
		// headerタグのtable作成
		Element	tableEle = this.xml.getElementNode("table");
		this.xml.setTextNode(tableEle, this.tableName);
		this.xml.addElement(headerEle, tableEle);
		// headerタグのextern作成
		Element	externEle = null;
		for (int i = 0; i < column; i++) {
			ColumnType	columnType = (ColumnType)this.columnList.get(i);
			if (columnType.isSeparate()) {
				externEle = this.xml.getElementNode("extern");
				this.xml.setTextNode(externEle, columnType.getName());
				this.xml.addElement(headerEle, externEle);
			}
		}
	}

	private void writeRecordTable()
		throws SQLException, IOException
	{
		String	str = null;

		StringBuffer	recordData = new StringBuffer();
		for (int i = 1; i <= this.columnList.size(); i++) {
			if ((str = getItemData(i)) == null) {
				continue;
			}
			recordData.append(str + COLUMN_SEPARATOR);
		}

		// 最後の区切り文字を消去する。
		String	writeDate = recordData.toString();
		if (writeDate.length() > 0) {
			writeDate = writeDate.substring(0, writeDate.length() - 1);
		}

		// データ書き込み
		this.bw.write(writeDate + LINE_SEPARATOR);
		this.bw.flush();

		// カウンター加算
		this.recordCount++;
	}

	private void writeRecordTableXML()
		throws SQLException, IOException
	{
		String	str = null;

		// ルートの取得
		Element	rootEle = xml.getRootElement();
		// record部の作成
		Element	recordEle = xml.getElementNode(this.xmlRecord);
		this.xml.addElement(rootEle, recordEle);
		
		Element	columnDataEle = null;
		for (int i = 1; i <= this.columnList.size(); i++) {
			ColumnType	columnType = (ColumnType)this.columnList.get(i - 1);
			if (columnType.getType() == java.sql.Types.ARRAY) {
				this.addArrayColumnElements(recordEle, columnType, i);
			} else {
				if ((str = getItemData(i)) == null) {
		    		continue;
				}
				columnDataEle = xml.getElementNode(getItemName(i));
				this.xml.setTextNode(columnDataEle, str);
				this.xml.addElement(recordEle, columnDataEle);
			}
		}

		this.recordCount++;
	}

	// for XML
	private void addArrayColumnElements(Element		xmlRecordElement_,
										ColumnType	columnType_,
										int			columnIndex_)
		throws SQLException, IOException
	{
		Element	xmlColumnElement = this.xml.getElementNode(getItemName(columnIndex_));

		java.sql.Array	columnValue = this.resultSet.getArray(columnIndex_);
		if (this.resultSet.wasNull() == false) {

			// 列値が非 null

			java.sql.ResultSet	columnResultSet = columnValue.getResultSet();

			if (columnType_.isSeparate()) {

				// 外部ファイル
				addArrayColumnExternElements(columnType_, columnResultSet, xmlColumnElement);

			} else {

				while (columnResultSet.next()) {
					String	elementString = columnResultSet.getString(2);
					if (columnResultSet.wasNull()) {
						elementString = "";
					}
					Element	xmlElementElement = this.xml.getElementNode(xmlElement);
					this.xml.setTextNode(xmlElementElement, elementString);
					this.xml.addElement(xmlColumnElement, xmlElementElement);
				}
			}
			columnResultSet.close();

		} else {

			// 列値が null

			this.xml.setTextNode(xmlColumnElement, "");
		}

		this.xml.addElement(xmlRecordElement_, xmlColumnElement);
	}

	// for XML
	private void addArrayColumnExternElements(	ColumnType			columnType_,
												java.sql.ResultSet	columnResultSet_,
												Element				xmlColumnElement_)
		throws SQLException, IOException
	{
		InputStream			inStream = null;

		String				externFileName = this.dir + this.tableName + "." + this.unloadCnt + "." + columnType_.getName() + "." + this.recordCount + ".";	// 拡張子はここではつけない
		FileOutputStream	externFile = null;

		try {
			String	columnDataType = columnType_.getTypeName();
			String	elementDataType = columnDataType.substring(0, columnDataType.indexOf(" ")).toLowerCase();
			boolean	isBinaryArray = (elementDataType.indexOf("binary") >= 0);
			int	elementCnt = 1;
			if (isBinaryArray) {
				// 要素がバイト列
				while (columnResultSet_.next()) {
					String	externFilePath = externFileName + elementCnt + "." + columnType_.getExt();
					inStream = columnResultSet_.getBinaryStream(2);
					if (columnResultSet_.wasNull()) {
						externFilePath = "";
					} else {
						byte[]	buff = new byte[1024];
						int		len;
						externFile = new FileOutputStream(externFilePath);
						while ((len = inStream.read(buff)) != -1) externFile.write(buff, 0, len);
						externFile.close();
					}
					Element	xmlElementElement = this.xml.getElementNode(xmlElement);
					this.xml.setTextNode(xmlElementElement, externFilePath);
					this.xml.addElement(xmlColumnElement_, xmlElementElement);
					elementCnt++;
					if (inStream != null) {
						inStream.close();
						inStream = null;
					}
				}
			} else {
				// 要素がバイト列以外
				while (columnResultSet_.next()) {
					String	externFilePath = externFileName + elementCnt + "." + columnType_.getExt();
					String	elementString = columnResultSet_.getString(2);
					if (columnResultSet_.wasNull()) {
						externFilePath = "";
					} else {
						externFile = new FileOutputStream(externFilePath);
						externFile.write(elementString.getBytes());
						externFile.close();
					}
					Element	xmlElementElement = this.xml.getElementNode(xmlElement);
					this.xml.setTextNode(xmlElementElement, externFilePath);
					this.xml.addElement(xmlColumnElement_, xmlElementElement);
					elementCnt++;
				}
			}
		} finally {

			if (inStream != null) inStream.close();
			if (externFile != null) externFile.close();
		}
	}

	/**
	 * 列名を取得する。
	 * @param index_
	 * @return 列名
	 */
	private String getItemName(int	index_)
	{
		ColumnType	columnType = (ColumnType)this.columnList.get(index_ - 1);

		return columnType.getName();
	}

	/**
	 * 列データを取得する。
	 * @param index_
	 * @return 列データ。外部列方式が選択された場合ファイル名。
	 * @throws SQLException
	 * @throws IOException
	 */
	private String getItemData(int	index_)
		throws SQLException, IOException
	{
		String		str = null;
		
		ColumnType	columnType = (ColumnType)this.columnList.get(index_ - 1);
		// 外部ファイルに出力
	    if (columnType.isSeparate()) {
	    	// 「テーブル名」+「-」+「列名」+「現レコード番号」+「.」+「指定拡張子」
	    	// 現在のバージョンはパスが指定できない。
	        //str = this.dir + this.tableName + "-" + columnType.getName() + "-" + recordCount + "." + columnType.getExt();
			// Version 1.1.1 で仕様変更
			// <表名>.<アンロードカウンタ>.<列名>.<現レコード番号>.<指定拡張子>
			//str = this.dir + this.tableName + "." + this.unloadCnt + "." + columnType.getName() + "." + this.recordCount + "." + columnType.getExt();
			//if ((str = outputSeparateColumn(index_, str, columnType)) == null) {
			//	str = "";
			//}
	    	//<表明>.<アンロードカウンタ>.<列名>.<指定拡張子>
	    	str = this.dir + this.tableName + "." + this.unloadCnt + "." + columnType.getName() + "."  + columnType.getExt();
	    	str = outputSeparateColumn(index_, str, columnType,this.recordCount);
	    } else {
	    	// 全て文字列として処理する。
			if (columnType.getType() == java.sql.Types.ARRAY) {
				str = "";
				java.sql.Array	columnValue = this.resultSet.getArray(index_);
				if (this.resultSet.wasNull() == false) {

					String	columnDataType = columnType.getTypeName();
					String	elementType = columnDataType.substring(0, columnDataType.indexOf(" ")).toLowerCase();
					boolean	isString = (elementType.indexOf("char") >= 0);
					java.sql.ResultSet	columnResultSet = columnValue.getResultSet();
					while (columnResultSet.next()) {
						String	elementString = columnResultSet.getString(2);
						if (columnResultSet.wasNull()) {
							str = str + "null";
						} else {
							if (isString) 
								str += addEscape(elementString);
							else
								str = str + elementString;
						}
						if (columnResultSet.isLast() == false)
							str = str + ELEMENT_SEPARATOR;
					}
					columnResultSet.close();
				}
			} else {
				String s = (String)this.resultSet.getString(index_);
				if (s != null) {
					s = s.trim();
					// 文字列はシングルクォートで括り、文字列中のシングルクォートは重ねてエスケープする
					switch(columnType.getType())
					{
						case java.sql.Types.CHAR:
						case java.sql.Types.LONGVARCHAR:
						case java.sql.Types.VARCHAR:
							str = addEscape(s);
							break;
						default:
							str = s;
					}
				} else {
					str = "";
				}
			}
	    }

	    return str;
	}

	//	index_	1 〜
	//	filePath_	別ファイルパス
	private String outputSeparateColumn(	int			index_,
											String		filePath_,
											ColumnType	columnType_,
											int			recordCnt_)
		throws SQLException, IOException
	{
		InputStream			inStream = null;
		FileOutputStream	outFile = null;
		BufferedWriter bWriter = null;
		String	filePath = filePath_;
		int	lastPeriod = filePath.lastIndexOf(".");
		String	fileName = filePath.substring(0, lastPeriod + 1);
		String	fileExt = filePath.substring(lastPeriod);
		filePath = "";
		try {

			switch (columnType_.getType()) {
			case java.sql.Types.ARRAY:
				// nullの場合
				java.sql.Array	columnValue = this.resultSet.getArray(index_);
				if (this.resultSet.wasNull()) {
					filePath = "null";
				} else {

					String	columnDataType = columnType_.getTypeName();
					String	elementType = columnDataType.substring(0, columnDataType.indexOf(" ")).toLowerCase();
					java.sql.ResultSet	columnResultSet = columnValue.getResultSet();
					int	elementCnt = 1;
					if (elementType.indexOf("binary") >= 0) {
						// 要素がバイト列
						while (columnResultSet.next()) {
							inStream = columnResultSet.getBinaryStream(2);
							if (columnResultSet.wasNull()) {
								filePath = filePath + "null";
							} else {
								byte[]	buff = new byte[1024];
								int		len;
								filePath = filePath + fileName + elementCnt + fileExt;
								outFile = new FileOutputStream(fileName + elementCnt + fileExt);
								while ((len = inStream.read(buff)) != -1) outFile.write(buff, 0, len);
								outFile.close(); outFile = null;
							}
							if (columnResultSet.isLast() == false) 
								filePath = filePath + ELEMENT_SEPARATOR;
							elementCnt++;
							if (inStream != null) {
								inStream.close();
								inStream = null;
							}
						}
					} else {
						// 要素がバイト列以外
						// <ELEMENT><ELEMENT_SEPARATOR><ELEMENT>・・・

						// 別ファイルはレコード毎に作成するように変更 2009/8/18
//						filePath = filePath_
						
						// <表名>.<アンロードカウンタ>.<列名>.<現レコード番号>.<指定拡張子>
						filePath = fileName + String.valueOf(recordCnt_) + fileExt;
						columnType_.setUnloadCode(this.unloadCode);
//						columnType_.setWriter(filePath_);
						columnType_.setWriter(filePath);
						bWriter = columnType_.getWriter();
						while (columnResultSet.next()) {
							String	elementString = columnResultSet.getString(2);
							if (columnResultSet.wasNull()) {
								bWriter.write("");
							} else {
								elementString = elementString.trim();
								if (elementType.indexOf("char") >= 0)
									elementString = addEscape(elementString);
								bWriter.write(elementString);
							}
							if (columnResultSet.isLast() == false) 
								bWriter.write(ELEMENT_SEPARATOR);
						}
					}
					columnResultSet.close();
				}
				break;
			case java.sql.Types.BINARY:
			case java.sql.Types.BLOB:
			case java.sql.Types.LONGVARBINARY:
			case java.sql.Types.VARBINARY:
				inStream = this.resultSet.getBinaryStream(index_);
				if (this.resultSet.wasNull()) {
					filePath = "null";
				} else {
					byte[]	buff = new byte[1024];
					int		len;
					//<表名>.<アンロードカウンタ>.<列名>.<現レコード番号>.<指定拡張子>
					filePath = fileName + String.valueOf(recordCnt_) + fileExt;
					outFile = new FileOutputStream(filePath);
					while ((len = inStream.read(buff)) != -1) 
						outFile.write(buff, 0, len);
				}
				break;
			default:
				// 別ファイルはレコード毎に作成するように変更 2009/5/21
//				filePath = filePath_
				
				// <表名>.<アンロードカウンタ>.<列名>.<現レコード番号>.<指定拡張子>
				filePath = fileName + String.valueOf(recordCnt_) + fileExt;
				String	str = this.resultSet.getString(index_);
				str = str.trim();
				
//				// 文字列はシングルクォートで括り、文字列中のシングルクォートは重ねてエスケープする
//				switch(columnType_.getType())
//				{
//					case java.sql.Types.CHAR:
//					case java.sql.Types.LONGVARCHAR:
//					case java.sql.Types.VARCHAR:
//					str = addEscape(str);
//					break;
//				}
				// <VALUE>
				columnType_.setUnloadCode(this.unloadCode);
//				columnType_.setWriter(filePath_);
				columnType_.setWriter(filePath);
				bWriter = columnType_.getWriter();
				if (this.resultSet.wasNull()) {
					bWriter.write("null");
				} else {
					bWriter.write(str);
				}
				break;
			}

			return filePath;

		} finally {
			if (inStream != null) inStream.close();
			if (outFile != null) outFile.close();
		}
	}
	private String addEscape(String s)
	{
		if (this.unloadType) {
			return s;
		}
		char [] cValue = s.toCharArray();
		StringBuffer buff = new StringBuffer("'");
		for ( int i = 0 ; i < cValue.length ; i++ )
		{
			if ( String.valueOf(cValue[i]).equals("'") ) 
				buff.append("'");
			buff.append(cValue[i]);
		}
		buff.append("'");
		return buff.toString();
	}
}

//
// Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
