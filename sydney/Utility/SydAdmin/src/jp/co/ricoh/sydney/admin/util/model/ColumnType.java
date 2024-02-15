// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnType.java -- 
// 
// Copyright (c) 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.model;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import jp.co.ricoh.sydney.admin.util.log.Log;

public class ColumnType
{
	private String	name = null;
	private int		type = java.sql.Types.NULL;	// java.sql.Types で示される JDBC 型（ SQL データ型）の定数
	private String	typeName = null;
	private int		size = 0;	// 文字列長など
	private int		scale = 0;	// scale value of decimal type (precision is represented by size)
	private int		numElements = 0;	// 配列型での最大要素数
	private int		nullable = java.sql.ResultSetMetaData.columnNullableUnknown;	// java.sql.ResultSetMetaData の定数 columnNoNulls or columnNullable or columnNullableUnknown
	private boolean	separate = false;	// 外部ファイルへ出力する列かどうか
	private String	ext = null;			// ファイル拡張子
	private	String	hint = "";			// 列ヒント
	private	String	defaultvalue = "";		// DEFAULT VALUE
	private BufferedReader reader = null;
	private String loadCode = "UTF-8";
	private String recordSeparator = "\n";
	private FileOutputStream	outFile = null;
	private OutputStreamWriter	outWriter = null;
	private BufferedWriter bWriter = null;
	private String unloadCode = "UTF-8";
	public ColumnType()
	{
	}
	public ColumnType(ColumnType other_)
	{
		name = other_.name;
		type = other_.type;
		typeName = other_.typeName;
		size = other_.size;
		scale = other_.scale;
		numElements = other_.numElements;
		nullable = other_.nullable;
		separate = other_.separate;
		hint = other_.hint;
		defaultvalue = other_.defaultvalue;
		reader = other_.reader;
		loadCode = other_.loadCode;
		recordSeparator = other_.recordSeparator;
		outFile = other_.outFile;
		outWriter = other_.outWriter;
		bWriter = other_.bWriter;
		
	}
	public void setWriter(String filePath_) throws IOException
	{
		closeWriter();
		String	charsetName = unloadCode;
		if (charsetName == null || charsetName.length() == 0) {
			charsetName = "JISAutoDetect";
		} else if (java.nio.charset.Charset.isSupported(charsetName) == false) {
			Log.warning("CHARSET = \"" + charsetName + "\" is not supported. (\"JISAutoDetect\" use.)");
			charsetName = "JISAutoDetect";
		}
		this.outFile = new FileOutputStream(filePath_);
		this.outWriter = new OutputStreamWriter(outFile, charsetName);
		this.bWriter = new BufferedWriter(outWriter);
	}
	public void closeWriter() throws IOException
	{
		if ( bWriter != null ) {
			bWriter.flush();
			bWriter.close();
		}
		
	}
	public BufferedWriter getWriter()
	{
		return bWriter;
	}
	public void setUnloadCode(String code)
	{
		this.unloadCode = code;
	}
	public void setRecordSeparator(String sep)
	{
		this.recordSeparator = sep;
	}
	public String getRecordSerparator()
	{
		return this.recordSeparator;
	}
	public BufferedReader getReader()
	{
		return reader;
	}
	public void setLoadCode(String code)
	{
		this.loadCode = code;
	}
	public void setReader(String filePath_) throws IOException
	{
		if ( reader != null )
			reader.close();
		
		String	charsetName = loadCode;
		if (charsetName == null || charsetName.length() == 0) {
			charsetName = "JISAutoDetect";
		} else if (java.nio.charset.Charset.isSupported(charsetName) == false) {
			Log.warning("CHARSET = \"" + charsetName + "\" is not supported. (\"JISAutoDetect\" use.)");
			charsetName = "JISAutoDetect";
		}
		reader = new BufferedReader(new InputStreamReader(new FileInputStream(filePath_), charsetName));
	}
	public String getName()
	{
		return name;
	}

	public void setName(String	name_)
	{
		this.name = name_;
	}

	// java.sql.Types で示される JDBC 型（ SQL データ型）の定数
	public int getType()
	{
		return type;
	}

	public void setType(int	type_)
	{
		this.type = type_;
	}

	public String getTypeName()
	{
		return typeName;
	}

	public void setTypeName(String	typeName_)
	{
		this.typeName = typeName_;
	}

	public int getSize()
	{
		return this.size;
	}

	public void setSize(int	size_)
	{
		this.size = size_;
	}

	public int getScale()
	{
		return this.scale;
	}

	public void setScale(int	scale_)
	{
		this.scale = scale_;
	}

	public int getNumElements()
	{
		return this.numElements;
	}

	public void setNumElements(int	numElements_)
	{
		this.numElements = numElements_;
	}

	// java.sql.ResultSetMetaData の定数 columnNoNulls or columnNullable or columnNullableUnknown
	public int getNullable()
	{
		return this.nullable;
	}

	public void setNullable(int	nullable_)
	{
		this.nullable = nullable_;
	}

	// 外部ファイルへ出力する列かどうか
	public boolean isSeparate()
	{
		return separate;
	}

	public void setSeparate(boolean	separate_)
	{
		this.separate = separate_;
	}

	// ファイル拡張子
	public String getExt()
	{
		return this.ext;
	}

	public void setExt(String	ext_)
	{
		this.ext = ext_;
	}

	public String getHint()
	{
		return this.hint;
	}

	public void setHint(String	hint_)
	{
		this.hint = hint_;
	}

	public String getDefault()
	{
		return this.defaultvalue;
	}

	public void setDefault(String	default_)
	{
		this.defaultvalue = default_;
	}
}

//
// Copyright (c) 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
