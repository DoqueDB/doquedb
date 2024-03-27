/* -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
 * vi:set ts=4 sw=4:
 *
 * jp.co.ricoh.sydney.build.MessageGeneratorForJava.java --
 */

package jp.co.ricoh.sydney.build;

import java.io.*;
import java.util.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;

/**
 * parse message/exception definition XML file and generate .java files for the messages/exceptions
 */
public class MessageGeneratorForJava
{
	private final static String _USAGE = "MessageGeneratorForJava moduleName definitionFileName charSet";
	private final static int _NUMARGS = 3;
	private final static String[] _languages =
	{
		"Japanese",
		"English",
	};
	private final static String _dirName = "Java";

	private static String _moduleName;
	private static boolean _isException;
	private static Document _document;
	private static Document _oldDocument;
	private static String _charSet;
	private static File _file;
	private static File _oldFile;
	private static HashMap _differentMap;
	private static BufferedWriter _throwClassFileWriter;
	private static BufferedWriter _errorCodeFileWriter;
	private static BufferedWriter[] _formatFileWriter;

	private final static String _nameTagName = "Name";
	private final static String _numberTagName = "Number";
	private final static String _descriptionTagName = "Description";
	private final static String _stateCodeTagName = "StateCode";
	private final static String _levelTagName = "Level";

	private static String _nodeTagName;
	private static String _formatTagName;
	private static String _argumentTagName;
	private static String _aliasTagName;
	private static String _nameSpace;
	private static String _nameSpace1;
	private static String _nameSpace2;
	private static String _numberFilePrefix;
	private static String _headerFilePrefix;

	/** main */
	public static void main(String[] args) throws Exception
	{
		if (args.length < _NUMARGS) {
			throw new RuntimeException("USAGE: " + _USAGE);
		}
		int i = 0;
		String moduleName = args[i++];
		String fileName = args[i++];
		String charSet = args[i++];

		_moduleName = moduleName;
		_isException = "Exception".equals(moduleName);
		_charSet = charSet;

		_file = new File(fileName);
		_oldFile = new File(fileName + "_");
		if (!_oldFile.exists()) {
			_oldFile = null;
		}

		_nodeTagName = (_isException ? "Error" : "Message");

		_formatTagName = (_isException ? "Message" : "Format");
		_argumentTagName = (_isException ? "MessageArgument" : "Argument");
		_aliasTagName = "Alias";
		_nameSpace = (_isException ? "ErrorNumber" : "Message::Number");
		_nameSpace1 = (_isException ? null : "Message");
		_nameSpace2 = (_isException ? "ErrorNumber" : "Number");

		_numberFilePrefix = (_isException ? "Number" : "MessageNumber_");
		_headerFilePrefix = (_isException ? "" : "Message_");

		load();
		check();
		createFiles();
	}

	/** load the XML file */
	private static void load()
	{
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		try {
			/* create inputSource specifying charSet */
			InputSource source = new InputSource(new FileInputStream(_file));
			source.setEncoding(_charSet);

			/* parse the XML file */
			_document = factory.newDocumentBuilder().parse(source);

			if (_oldFile != null) {
				/* create inputSource specifying charSet */
				InputSource oldSource = new InputSource(new FileInputStream(_oldFile));
				oldSource.setEncoding(_charSet);

				/* parse the XML file */
				_oldDocument = factory.newDocumentBuilder().parse(oldSource);
			}

		} catch (ParserConfigurationException e) {
			throw new RuntimeException("Can't create new DocumentBuilder: '" + e + "'");

		} catch (org.xml.sax.SAXException e) {
			throw new RuntimeException("Parse error of " + _file + ": '" + e + "'");

		} catch (IOException e) {
			throw new RuntimeException("File read error of " + _file + ": '" + e + "'");
		}
	}
	/** check the XML file */
	private static void check()
	{
		NodeList nameList = _document.getElementsByTagName(_nameTagName);
		NodeList numberList = _document.getElementsByTagName(_numberTagName);

		int nameLength = nameList.getLength();
		int numberLength = numberList.getLength();
		
		if (nameLength != numberLength) {
			throw new RuntimeException("Bad definition: the number of <Name> tags is different from that of <Number> tags");
		}

		checkNodeList(nameList, nameLength);
		checkNodeList(numberList, numberLength);

		if (_oldDocument != null) {
			createDifferentMap(createHashMap(_oldDocument));
		}
	}

	/** create hashmap from node list */
	private static HashMap createHashMap(Document document_)
	{
		HashMap result = new HashMap();

		NodeList nodeList = document_.getElementsByTagName(_nodeTagName);
		int nodeLength = nodeList.getLength();
		for (int i = 0; i < nodeLength; ++i) {
			addNode((Element)nodeList.item(i), result);
		}
		return result;
	}

	/** add a node to hashmap */
	private static void addNode(Element element_, HashMap map_)
	{
		Node nameNode = element_.getElementsByTagName(_nameTagName).item(0);
		String name = nameNode.getChildNodes().item(0).getNodeValue().trim();
		map_.put(name, element_);
	}

	/** check changed nodes */
	private static void createDifferentMap(HashMap oldMap_)
	{
		_differentMap = new HashMap();
		NodeList nodeList = _document.getElementsByTagName(_nodeTagName);
		int nodeLength = nodeList.getLength();
		for (int i = 0; i < nodeLength; ++i) {
			checkNode((Element)nodeList.item(i), oldMap_);
		}
	}
	/** check a changed node */
	private static void checkNode(Element element_, HashMap oldMap_)
	{
		Node nameNode = element_.getElementsByTagName(_nameTagName).item(0);
		String name = nameNode.getChildNodes().item(0).getNodeValue().trim();

		// get from old map
		Element oldElement = (Element)oldMap_.get(name);
		if (oldElement == null) {
			// new node
			_differentMap.put(name, element_);
			return;
		}

		// if file does not exists, it should be created
		File file = new File(_file.getParentFile(),
							 _dirName + File.separatorChar + name + ".java");
		if (!file.exists()) {
			_differentMap.put(name, element_);
			return;
		}

		String[] arguments = getArguments(element_);
		String description = getDescription(element_);
		String statecode = getStateCode(element_);
		String level = getLevel(element_);
		String[] aliases = getAliases(element_);
		String number = getNumber(element_);
		String[] format = getFormat(element_);

		String[] oldArguments = getArguments(oldElement);
		String oldDescription = getDescription(oldElement);
		String oldStatecode = getStateCode(oldElement);
		String oldLevel = getLevel(oldElement);
		String[] oldAliases = getAliases(oldElement);
		String oldNumber = getNumber(oldElement);
		String[] oldFormat = getFormat(oldElement);

		// if any element is changed, reproduce the file
		if (compareStrings(arguments, oldArguments)
			||
			compareString(description, oldDescription)
			||
			compareString(statecode, oldStatecode)
			||
			compareString(level, oldLevel)
			||
			compareStrings(aliases, oldAliases)
			||
			compareString(number, oldNumber)
			||
			compareStrings(format, oldFormat)) {
			// changed node
			_differentMap.put(name, element_);
		}
	}

	/** compare string array */
	private static boolean compareStrings(String[] new_, String[] old_)
	{
		if (new_ == null && old_ == null) {
			return false;
		}
		if (new_ == null || old_ == null) {
			return true;
		}
		if (new_.length == old_.length) {
			for (int i = 0; i < new_.length; ++i) {
				if (compareString(new_[i], old_[i])) {
					return true;
				}
			}
			return false;
		}
		return true;
	}
	/** compare string */
	private static boolean compareString(String new_, String old_)
	{
		if (new_ == null && old_ == null) {
			return false;
		}
		if (new_ == null || old_ == null) {
			return true;
		}
		return !new_.equals(old_);
	}

	/** check node list */
	private static void checkNodeList(NodeList nodeList, int length)
	{
		for (int i = 0; i < length - 1; ++i) {
			Node node = nodeList.item(i);
			String nodeName = node.getNodeName();
			String text = node.getChildNodes().item(0).getNodeValue().trim();

			/** check duplicates */
			for (int j = i + 1; j < length; ++j) {
				Node otherNode = nodeList.item(j);
				String otherText = otherNode.getChildNodes().item(0).getNodeValue().trim();

				if (text.equals(otherText)) {
					throw new RuntimeException("Bad definition: duplicate " + nodeName + ": " + text);
				}
			}
		}
	}

	/** create files */
	private static void createFiles()
	{
		final String throwClassFileName = (_isException ? "ThrowClassInstance.java" : null);
		final String errorCodeFileName = (_isException ? "ErrorCode.java" : null);
		final String formatFilePrefix = (_isException ? "MessageFormat" : "MessageFormat_");

		if (_differentMap != null && _differentMap.isEmpty()) {
			return;
		}

		try {
			/* prepare allXXX files */
			_throwClassFileWriter = createWriter(_dirName, throwClassFileName);
			writeThrowClassHeader(_throwClassFileWriter);

			_errorCodeFileWriter = createWriter(_dirName, errorCodeFileName);
			writeErrorCodeHeader(_errorCodeFileWriter);

			_formatFileWriter = new BufferedWriter[_languages.length];
			for (int i = 0; i < _languages.length; ++i) {
				String formatFileName = formatFilePrefix + _languages[i] + ".java";
				_formatFileWriter[i] = createWriter(_dirName, formatFileName);
				writeFormatHeader(_formatFileWriter[i], _languages[i]);
			}

			NodeList nodeList = _document.getElementsByTagName(_nodeTagName);
			int nodeLength = nodeList.getLength();

			for (int i = 0; i < nodeLength; ++i) {
				createFile((Element)nodeList.item(i));
			}

			writeThrowClassFooter(_throwClassFileWriter);
			writeErrorCodeFooter(_errorCodeFileWriter);
			for (int i = 0; i < _languages.length; ++i) {
				writeFormatFooter(_formatFileWriter[i]);
			}

			/* terminate files */
			terminateWriter(_throwClassFileWriter);
			terminateWriter(_errorCodeFileWriter);
			for (int i = 0; i < _languages.length; ++i) {
				terminateWriter(_formatFileWriter[i]);
			}
		} catch (java.io.IOException e) {
			throw new RuntimeException("Can't create file: '" + e + "'");
		}
	}

	/** create files according to a message/error definition */
	private static void createFile(Element node)
	{
		Node nameNode = node.getElementsByTagName(_nameTagName).item(0);
		String name = nameNode.getChildNodes().item(0).getNodeValue().trim();
		String[] arguments = getArguments(node);
		String description = getDescription(node);
		String statecode = getStateCode(node);
		String[] aliases = getAliases(node);
		String codeName = createCodeName(name);

		if (_isException && statecode == null) {
			throw new RuntimeException("StateCode is required (" + name + ").");
		}

		try {
			writeSharedFiles(node, name, arguments, description, codeName);
			if (_differentMap == null || _differentMap.get(name) != null) {
				createClassFile(name, arguments, description, statecode, codeName);
			}
		} catch (java.io.IOException e) {
			throw new RuntimeException("Can't create file: '" + e + "'");
		}
	}

	/** write to allXXX files */
	private static void writeSharedFiles(Element node, String name, String[] arguments, String description, String codeName)
		throws java.io.IOException
	{
		Element formatList = (Element)node.getElementsByTagName(_formatTagName).item(0);

		if (_throwClassFileWriter != null) {
			_throwClassFileWriter.write("\t\tcase ErrorCode." + codeName + ":");
			_throwClassFileWriter.newLine();
			_throwClassFileWriter.write("\t\t\tthrow new " + name + "(e_);");
			_throwClassFileWriter.newLine();
		}

		if (_errorCodeFileWriter != null) {
			Node numberNode = node.getElementsByTagName(_numberTagName).item(0);
			String number = numberNode.getChildNodes().item(0).getNodeValue().trim();

			_errorCodeFileWriter.write("\t/** " + description + " */");
			_errorCodeFileWriter.newLine();
			_errorCodeFileWriter.write("\tpublic final static int " + codeName + " = " + number + ";");
			_errorCodeFileWriter.newLine();
		}

		String formatKey = "\t{" + _moduleName + "::" + _nameSpace + "::" + name +",";
		for (int i = 0; i < _languages.length; ++i) {
			NodeList formatNodeList = formatList.getElementsByTagName(_languages[i]);
			if (formatNodeList.getLength() > 0) {
				Node formatNode = formatNodeList.item(0);
				String message = formatNode.getChildNodes().item(0).getNodeValue().trim();

				_formatFileWriter[i].write("\t\tnew MessageEntry(ErrorCode." + codeName + ",");
				_formatFileWriter[i].newLine();
				_formatFileWriter[i].write("\t\t\t\"" + message + "\")");
				_formatFileWriter[i].write(",");
				_formatFileWriter[i].newLine();
			}
		}
	}

	/** create a class file */
	private static void createClassFile(String name, String[] arguments,
										String description, String statecode, String codeName)
		throws java.io.IOException
	{
		String numberFileNameBase = _numberFilePrefix + name;
		String fileNameBase = _headerFilePrefix + name;
		String fileName = fileNameBase + ".java";

		BufferedWriter writer = createWriter(_dirName, fileName);

		writer.write("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// " + fileName + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2001, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		/* Automatically generated files are not needed include these parts
		writer.write("//	$Author$"); writer.newLine();
		writer.write("//	$Date$"); writer.newLine();
		writer.write("//	$Revision$"); writer.newLine();
		writer.write("//"); writer.newLine();
		*/
		writer.write(""); writer.newLine();
		writer.write("package jp.co.ricoh.doquedb.exception;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("import jp.co.ricoh.doquedb.common.*;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("/**"); writer.newLine();
		writer.write(" * " + description); writer.newLine();
		writer.write(" */"); writer.newLine();
		writer.write("public class " + name + " extends " + getSuperClazz(statecode)); writer.newLine();
		writer.write("{"); writer.newLine();

		writeClassFile(writer, name, arguments, statecode, codeName, "ExceptionData");
		writeClassFile(writer, name, arguments, statecode, codeName, "Argument");

		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("//	Copyright (c) 2001, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("//	All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();

		terminateWriter(writer);
	}

	/** write constructor implementation to a class file */
	private static void writeClassFile(BufferedWriter writer, String name, String[] arguments,
									   String stateCode, String codeName, String type)
		throws java.io.IOException
	{
		if ("ModLibraryError".equals(name) && "Argument".equals(type)) return;

		String extraArgument = "";
		if ("Argument".equals(type) && arguments != null) {
			for (int i = 0; i < arguments.length; ++i) {
				if (i > 0) {
					extraArgument = extraArgument + ", ";
				}
				extraArgument = extraArgument + createType(arguments[i]) + " arg" + i + "_";
			}
		}

		writer.write("\t/**"); writer.newLine();
		writer.write("\t * 新たに例外オブジェクトを作成する"); writer.newLine();
		writer.write("\t */"); writer.newLine();
		writer.write("\tpublic ");
		writer.write(name + "(");
		if ("ExceptionData".equals(type)) {
			writer.write("ExceptionData e_");
		} else if ("Argument".equals(type)) {
			if (arguments != null) {
				writer.write(extraArgument);
			}
		}
		writer.write(")"); writer.newLine();
		writer.write("\t{"); writer.newLine();
		writer.write("\t\tsuper(");
		if ("ExceptionData".equals(type)) {
			writer.write("e_.getErrorMessage(), ");
		} else if ("Argument".equals(type)) {
			writer.write("makeErrorMessage(");
			if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					if (i > 0) {
						writer.write(", ");
					}
					writer.write("arg" + i + "_");
				}
			}
			writer.write("), ");
		}
		writer.write("\"" + stateCode + "\", ErrorCode." + codeName);
		writer.write(");"); writer.newLine();
		writer.write("\t}"); writer.newLine();
		writer.write(""); writer.newLine();
		if ("Argument".equals(type)) {
			writer.write("\t/**"); writer.newLine();
			writer.write("\t * エラーメッセージを作成する"); writer.newLine();
			writer.write("\t */"); writer.newLine();
			writer.write("\tprivate static String makeErrorMessage(" + extraArgument + ")"); writer.newLine();
			writer.write("\t{"); writer.newLine();
			if (arguments != null && arguments.length > 0) {
				writer.write("\t\tjava.util.Vector arguments = new java.util.Vector();"); writer.newLine();
				for (int i = 0; i < arguments.length; ++i) {
					writer.write("\t\targuments.add(" + createConvert(arguments[i], i) + ");"); writer.newLine();
				}
			}
			writer.write("\t\treturn ErrorMessage.makeErrorMessage("); writer.newLine();
			writer.write("\t\t\t\tErrorCode." + codeName + ", "
						 + ((arguments != null && arguments.length > 0) ? "arguments" : "null") + ");"); writer.newLine();
			writer.write("\t}"); writer.newLine();
		}
	}

	/** write header of ThrowClassInstance.java */
	private static void writeThrowClassHeader(BufferedWriter writer)
		throws java.io.IOException
	{
		writer.write("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// ThrowClassInstance.java -- 例外をスローする"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		/* Automatically generated files are not needed include these parts
		writer.write("//	$Author$"); writer.newLine();
		writer.write("//	$Date$"); writer.newLine();
		writer.write("//	$Revision$"); writer.newLine();
		writer.write("//"); writer.newLine();
		*/
		writer.write(""); writer.newLine();
		writer.write("package jp.co.ricoh.doquedb.exception;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("import jp.co.ricoh.doquedb.common.*;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("/**"); writer.newLine();
		writer.write(" * 例外をスローする。"); writer.newLine();
		writer.write(" */"); writer.newLine();
		writer.write("public final class ThrowClassInstance"); writer.newLine();
		writer.write("{"); writer.newLine();
		writer.write("	/**"); writer.newLine();
		writer.write("	 * 例外をスローする。"); writer.newLine();
		writer.write("	 */"); writer.newLine();
		writer.write("	public static void throwException(ExceptionData e_)"); writer.newLine();
		writer.write("		throws java.sql.SQLException"); writer.newLine();
		writer.write("	{"); writer.newLine();
		writer.write("		switch (e_.getErrorNumber())"); writer.newLine();
		writer.write("		{"); writer.newLine();
	}

	/** write footer of ThrowClassInstance.java */
	private static void writeThrowClassFooter(BufferedWriter writer)
		throws java.io.IOException
	{
		writer.write("		default:"); writer.newLine();
		writer.write("			throw new UnknownException(e_.getErrorNumber());"); writer.newLine();
		writer.write("		}"); writer.newLine();
		writer.write("	}"); writer.newLine();
		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
	}

	/** write header of ErrorCodeInstance.java */
	private static void writeErrorCodeHeader(BufferedWriter writer)
		throws java.io.IOException
	{
		writer.write("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// ErrorCode.java -- 例外をスローする"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		/*
		writer.write("//	$Author$"); writer.newLine();
		writer.write("//	$Date$"); writer.newLine();
		writer.write("//	$Revision$"); writer.newLine();
		writer.write("//"); writer.newLine();
		*/
		writer.write(""); writer.newLine();
		writer.write("package jp.co.ricoh.doquedb.exception;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("/**"); writer.newLine();
		writer.write(" * エラーコード"); writer.newLine();
		writer.write(" */"); writer.newLine();
		writer.write("public final class ErrorCode"); writer.newLine();
		writer.write("{"); writer.newLine();
	}

	/** write footer of ErrorCodeInstance.java */
	private static void writeErrorCodeFooter(BufferedWriter writer)
		throws java.io.IOException
	{
		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
	}

	/** write header of MessageFormatXXX.java */
	private static void writeFormatHeader(BufferedWriter writer, String language)
		throws java.io.IOException
	{
		writer.write("// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// MessageFormat" + language + ".java -- エラーメッセージフォーマット"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015, 2023 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		/*
		writer.write("//	$Author$"); writer.newLine();
		writer.write("//	$Date$"); writer.newLine();
		writer.write("//	$Revision$"); writer.newLine();
		writer.write("//"); writer.newLine();
		*/
		writer.write(""); writer.newLine();
		writer.write("package jp.co.ricoh.doquedb.exception;"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("/**"); writer.newLine();
		writer.write(" * エラーメッセージフォーマット"); writer.newLine();
		writer.write(" */"); writer.newLine();
		writer.write("public final class MessageFormat" + language); writer.newLine();
		writer.write("{"); writer.newLine();
		writer.write("	/** フォーマット */"); writer.newLine();
		writer.write("	public final static MessageEntry _table[] ="); writer.newLine();
		writer.write("	{"); writer.newLine();
	}

	/** write footer of MessageFormatXXX.java */
	private static void writeFormatFooter(BufferedWriter writer)
		throws java.io.IOException
	{
		writer.write("	};"); writer.newLine();
		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2002, 2015 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
	}

	private static java.util.HashMap _mapType;
	private static java.util.HashMap _mapConvert;

	/** create type string from %d or %s */
	private static String createType(String argument)
	{
		if (_mapType == null) {
			_mapType = new java.util.HashMap();
			_mapType.put("%s", "String");
			_mapType.put("%hd", "short");
			_mapType.put("%hi", "short");
			_mapType.put("%hu", "short");
			_mapType.put("%ho", "short");
			_mapType.put("%hx", "short");
			_mapType.put("%hX", "short");
			_mapType.put("%lld", "long");
			_mapType.put("%lli", "long");
			_mapType.put("%llu", "long");
			_mapType.put("%llo", "long");
			_mapType.put("%llx", "long");
			_mapType.put("%llX", "long");
			_mapType.put("%ld", "int");
			_mapType.put("%li", "int");
			_mapType.put("%lu", "int");
			_mapType.put("%lo", "int");
			_mapType.put("%lx", "int");
			_mapType.put("%lX", "int");
			_mapType.put("%d", "int");
			_mapType.put("%i", "int");
			_mapType.put("%u", "int");
			_mapType.put("%o", "int");
			_mapType.put("%x", "int");
			_mapType.put("%X", "int");
		}
		String result = (String)_mapType.get(argument);
		if (result == null) {
			throw new RuntimeException("Unknown format string: " + argument);
		}
		return result;
	}

	/** create type string from %d or %s */
	private static String createConvert(String argument, int index)
	{
		if (_mapConvert == null) {
			_mapConvert = new java.util.HashMap();
			_mapConvert.put("%s", null);
			_mapConvert.put("%hd", "Integer.toString");
			_mapConvert.put("%hi", "Integer.toString");
			_mapConvert.put("%hu", "Integer.toString");
			_mapConvert.put("%ho", "Integer.toOctalString");
			_mapConvert.put("%hx", "Integer.toHexString");
			_mapConvert.put("%hX", "Integer.toHexString");
			_mapConvert.put("%lld", "Long.toString");
			_mapConvert.put("%lli", "Long.toString");
			_mapConvert.put("%llu", "Long.toString");
			_mapConvert.put("%llo", "Long.toOctalString");
			_mapConvert.put("%llx", "Long.toHexString");
			_mapConvert.put("%llX", "Long.toHexString");
			_mapConvert.put("%ld", "Integer.toString");
			_mapConvert.put("%li", "Integer.toString");
			_mapConvert.put("%lu", "Integer.toString");
			_mapConvert.put("%lo", "Integer.toOctalString");
			_mapConvert.put("%lx", "Integer.toHexString");
			_mapConvert.put("%lX", "Integer.toHexString");
			_mapConvert.put("%d", "Integer.toString");
			_mapConvert.put("%i", "Integer.toString");
			_mapConvert.put("%u", "Integer.toString");
			_mapConvert.put("%o", "Integer.toOctalString");
			_mapConvert.put("%x", "Integer.toHexString");
			_mapConvert.put("%X", "Integer.toHexString");
		}
		String result = (String)_mapConvert.get(argument);
		if (result == null) {
			result = "arg" + index + "_";
		} else {
			result = result + "(arg" + index + "_)";
		}
		if (argument.endsWith("X")) {
			result = result + ".toUpperCase()";
		}
		return result;
	}

	/** create new Writer */
	private static BufferedWriter createWriter(String dirName, String fileName)
	{
		if (fileName == null) return null;
		try {
			return new BufferedWriter(
					new OutputStreamWriter(
						new FileOutputStream(
							new File(_file.getParentFile(),
									 (dirName == null ? "" : dirName + File.separatorChar) + fileName)),
						java.nio.charset.Charset.forName(_charSet)));

		} catch (java.io.FileNotFoundException e) {
			throw new RuntimeException("Can't write to file '" + fileName + "': '" + e + "'");
		}
	}

	/** terminate Writer */
	private static void terminateWriter(BufferedWriter writer)
		throws java.io.IOException
	{
		if (writer != null) {
			writer.flush();
			writer.close();
		}
	}

	/** get format */
	private static String[] getFormat(Element node)
	{
		NodeList nodeList = node.getElementsByTagName(_formatTagName);
		if (nodeList.getLength() > 0) {
			int languageLength = _languages.length;
			String[] result = new String[languageLength];
			for (int i = 0; i < languageLength; ++i) {
				NodeList formatList = ((Element)nodeList.item(0)).getElementsByTagName(_languages[i]);
				if (formatList.getLength() > 0) {
					Node formatNode = formatList.item(0);
					String format = formatNode.getChildNodes().item(0).getNodeValue().trim();
					result[i] = formatNode.getChildNodes().item(0).getNodeValue().trim();
				}
			}
			return result;
		}
		return null;
	}

	/** get description */
	private static String getDescription(Element node)
	{
		NodeList nodeList = node.getElementsByTagName(_descriptionTagName);
		if (nodeList.getLength() > 0) {
			NodeList descriptionList = ((Element)nodeList.item(0)).getElementsByTagName(_languages[0]);
			if (descriptionList.getLength() > 0) {
				Node descriptionNode = descriptionList.item(0);
				String description = descriptionNode.getChildNodes().item(0).getNodeValue().trim();
				return description.replaceAll("[\r\n]\\s*", "");
			}
		}
		return null;
	}

	/** get state code */
	private static String getStateCode(Element node)
	{
		NodeList nodeList = node.getElementsByTagName(_stateCodeTagName);
		if (nodeList.getLength() > 0) {
			Node stateCodeNode = nodeList.item(0);
			return stateCodeNode.getChildNodes().item(0).getNodeValue().trim();
		}
		return null;
	}

	/** get level */
	private static String getLevel(Element node)
	{
		NodeList nodeList = node.getElementsByTagName(_levelTagName);
		if (nodeList.getLength() > 0) {
			Node stateCodeNode = nodeList.item(0);
			return stateCodeNode.getChildNodes().item(0).getNodeValue().trim() + "Level";
		}
		return "Object";
	}

	/** get arguments */
	private static String[] getArguments(Element node)
	{
		NodeList argumentList = node.getElementsByTagName(_argumentTagName);
		int argumentLength = argumentList.getLength();

		if (argumentLength > 0) {
			String[] result = new String[argumentLength];
			for (int i = 0; i < argumentLength; ++i) {
				Node argumentNode = argumentList.item(i);
				result[i] = argumentNode.getChildNodes().item(0).getNodeValue().trim();
			}
			return result;
		}
		return null;
	}

	/** get aliases */
	private static String[] getAliases(Element node)
	{
		NodeList aliasList = node.getElementsByTagName(_aliasTagName);
		int aliasLength = aliasList.getLength();

		if (aliasLength > 0) {
			String[] result = new String[aliasLength];
			for (int i = 0; i < aliasLength; ++i) {
				Node aliasNode = aliasList.item(i);
				result[i] = aliasNode.getChildNodes().item(0).getNodeValue().trim();
			}
			return result;
		}
		return null;
	}

	/** get number */
	private static String getNumber(Element node)
	{
		Node numberNode = node.getElementsByTagName(_numberTagName).item(0);
		return numberNode.getChildNodes().item(0).getNodeValue().trim();
	}

	/** create codeName from Exception name */
	private static String createCodeName(String name)
	{
		return name.replaceAll("([a-z]|SQL|SubString|TimeStamp)([A-Z])", "$1_$2").toUpperCase();
	}

	/** get super class */
	private static String getSuperClazz(String stateCode)
	{
		String clazz = stateCode.substring(0, 2);
		String ret = "java.sql.SQLException";
		if (clazz.equals("22") == true) {
			ret = "java.sql.SQLDataException";
		} else if (clazz.equals("0A") == true) {
			ret = "java.sql.SQLFeatureNotSupportedException";
		} else if (clazz.equals("23") == true) {
			ret = "java.sql.SQLIntegrityConstraintViolationException";
		} else if (clazz.equals("28") == true) {
			ret = "java.sql.SQLInvalidAuthorizationSpecException";
		} else if (clazz.equals("08") == true) {
			ret = "java.sql.SQLTransientConnectionException";
		} else if (clazz.equals("42") == true) {
			ret = "java.sql.SQLSyntaxErrorException";
		} else if (clazz.equals("40") == true) {
			ret = "java.sql.SQLTransactionRollbackException";
		}
		return ret;
	}
	
}

/*
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2015, 2023 Ricoh Company, Ltd.
 * All rights reserved.
 */
