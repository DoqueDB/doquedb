/* -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
 * vi:set ts=4 sw=4:
 *
 * jp.co.ricoh.sydney.build.MessageGenerator.java --
 */

package jp.co.ricoh.sydney.build;

import java.io.*;
import java.util.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;

/**
 * parse message/exception definition XML file and generate .h, .cpp files for the messages/exceptions
 */
public class MessageGenerator
{
	private final static String _USAGE = "MessageGenerator moduleName definitionFileName charSet";
	private final static int _NUMARGS = 3;
	private final static String[] _languages =
	{
		"Japanese",
		"English",
	};

	private static String _moduleName;
	private static boolean _isException;
	private static Document _document;
	private static Document _oldDocument;
	private static String _charSet;
	private static File _file;
	private static File _oldFile;
	private static HashMap _differentMap;
	private static BufferedWriter _allNumberFileWriter;
	private static BufferedWriter _allClassFileWriter;
	private static BufferedWriter _throwClassFileWriter;
	private static BufferedWriter _argumentFileWriter;
	private static BufferedWriter[] _formatFileWriter;
	private static BufferedWriter _makeFileObjectWriter;
	private static BufferedWriter _makeFileHeaderWriter;
	private static BufferedWriter _makeFileNumberWriter;

	private static boolean _writeCppCvsignore;
	private static boolean _writeHCvsignore;
	private static BufferedWriter _cppCvsignoreWriter;
	private static BufferedWriter _hCvsignoreWriter;

	private final static String _nameTagName = "Name";
	private final static String _numberTagName = "Number";
	private final static String _descriptionTagName = "Description";
	private final static String _stateCodeTagName = "StateCode";
	private final static String _levelTagName = "Level";

	private final static String _makeFileObjectName = "../c/Makefile_object.h";
	private final static String _makeFileHeaderName = "../c/Makefile_header.h";
	private final static String _makeFileNumberName = "../c/Makefile_number.h";
	private final static int _makeFileListLength = 40;

	private static String _nodeTagName;
	private static String _formatTagName;
	private static String _argumentTagName;
	private static String _aliasTagName;
	private static String _convertTagName;
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

		final String cvsignoreBaseFileName = ".cvsignore_base";
		_writeCppCvsignore = new File(".." + File.separatorChar
									  + cvsignoreBaseFileName).exists();
		_writeHCvsignore = new File(".." + File.separatorChar
									+ moduleName + File.separatorChar
									+ cvsignoreBaseFileName).exists();

		_file = new File(fileName);
		_oldFile = new File(fileName + "_");
		if (!_oldFile.exists()) {
			_oldFile = null;
		}

		_nodeTagName = (_isException ? "Error" : "Message");

		_formatTagName = (_isException ? "Message" : "Format");
		_argumentTagName = (_isException ? "MessageArgument" : "Argument");
		_aliasTagName = "Alias";
		_convertTagName = "Convert";
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

		String[] arguments = getArguments(element_);
		String description = getDescription(element_);
		String statecode = getStateCode(element_);
		String level = getLevel(element_);
		String[] aliases = getAliases(element_);
		String convert = getConvert(element_);
		String number = getNumber(element_);
		String[] format = getFormat(element_);

		String[] oldArguments = getArguments(oldElement);
		String oldDescription = getDescription(oldElement);
		String oldStatecode = getStateCode(oldElement);
		String oldLevel = getLevel(oldElement);
		String[] oldAliases = getAliases(oldElement);
		String oldConvert = getConvert(oldElement);
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
			compareString(convert, oldConvert)
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
		final String allNumberFileName = (_isException ? "AllNumberFiles.h" : "MessageAll_Number.h");
		final String allClassFileName = (_isException ? "AllExceptionFiles.h" : "MessageAll_Class.h");
		final String throwClassFileName = (_isException ? "ThrowClassInstance.h" : null);
		final String argumentFileName = "MessageArgument.h";
		final String formatFilePrefix = (_isException ? "MessageFormat" : "MessageFormat_");
		final String cvsignoreFileName = ".cvsignore_out";

		if (_differentMap != null && _differentMap.isEmpty()) {
			return;
		}

		try {
			/* prepare allXXX files */
			_allNumberFileWriter = createWriter(_moduleName, allNumberFileName);
			_allClassFileWriter = createWriter(_moduleName, allClassFileName);
			_throwClassFileWriter = createWriter(_moduleName, throwClassFileName);
			_argumentFileWriter = createWriter(_moduleName, argumentFileName);
			if (_isException) {
				_makeFileObjectWriter = createWriter(_moduleName, _makeFileObjectName);
				_makeFileHeaderWriter = createWriter(_moduleName, _makeFileHeaderName);
				_makeFileNumberWriter = createWriter(_moduleName, _makeFileNumberName);
			}
			if (_writeCppCvsignore) {
				_cppCvsignoreWriter = createWriter(null, cvsignoreFileName);
				_cppCvsignoreWriter.write(cvsignoreFileName);
				_cppCvsignoreWriter.newLine();
			}
			if (_writeHCvsignore) {
				_hCvsignoreWriter = createWriter(_moduleName, cvsignoreFileName);
				_hCvsignoreWriter.write(cvsignoreFileName);
				_hCvsignoreWriter.newLine();
			}

			_formatFileWriter = new BufferedWriter[_languages.length];
			for (int i = 0; i < _languages.length; ++i) {
				String formatFileName = formatFilePrefix + _languages[i] + ".h";
				_formatFileWriter[i] = createWriter(_moduleName, formatFileName);
			}

			NodeList nodeList = _document.getElementsByTagName(_nodeTagName);
			int nodeLength = nodeList.getLength();

			for (int i = 0; i < nodeLength; ++i) {
				createFile((Element)nodeList.item(i), i);
			}

			if (_isException) {
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.write("GENERATED_HDRS_NOTEXPORT = \\");
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.write("\t$(HDRDIR)/" + allClassFileName);
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.write("GENERATED_HDRS_EXPORT = \\");
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.write("\t$(HDRDIR)/" + allNumberFileName + " \\");
				_makeFileHeaderWriter.newLine();
				for (int i = 0; i < _languages.length; ++i) {
					String formatFileName = formatFilePrefix + _languages[i] + ".h";
					_makeFileHeaderWriter.write("\t$(HDRDIR)/" +  formatFileName + " \\");
					_makeFileHeaderWriter.newLine();
				}
				_makeFileHeaderWriter.write("\t$(HDRDIR)/" + argumentFileName);
				_makeFileHeaderWriter.newLine();

				writeMakeFileTail(nodeLength);
			}
			if (_writeHCvsignore) {
				_hCvsignoreWriter.write(allClassFileName);
				_hCvsignoreWriter.newLine();
				_hCvsignoreWriter.write(allNumberFileName);
				_hCvsignoreWriter.newLine();
				for (int i = 0; i < _languages.length; ++i) {
					String formatFileName = formatFilePrefix + _languages[i] + ".h";
					_hCvsignoreWriter.write(formatFileName);
					_hCvsignoreWriter.newLine();
				}
				_hCvsignoreWriter.write(argumentFileName);
				_hCvsignoreWriter.newLine();
				_hCvsignoreWriter.write(throwClassFileName);
				_hCvsignoreWriter.newLine();
			}
			/* terminate allXXX files */
			terminateWriter(_allNumberFileWriter);
			terminateWriter(_allClassFileWriter);
			terminateWriter(_throwClassFileWriter);
			terminateWriter(_argumentFileWriter);
			if (_isException) {
				terminateWriter(_makeFileObjectWriter);
				terminateWriter(_makeFileHeaderWriter);
				terminateWriter(_makeFileNumberWriter);
			}
			if (_writeCppCvsignore) {
				terminateWriter(_cppCvsignoreWriter);
			}
			if (_writeHCvsignore) {
				terminateWriter(_hCvsignoreWriter);
			}
			for (int i = 0; i < _languages.length; ++i) {
				terminateWriter(_formatFileWriter[i]);
			}
		} catch (java.io.IOException e) {
			throw new RuntimeException("Can't create file: '" + e + "'");
		}
	}

	/** create files according to a message/error definition */
	private static void createFile(Element node, int index)
	{
		Node nameNode = node.getElementsByTagName(_nameTagName).item(0);
		String name = nameNode.getChildNodes().item(0).getNodeValue().trim();
		String[] arguments = getArguments(node);
		String description = getDescription(node);
		String statecode = getStateCode(node);
		String level = getLevel(node);
		String[] aliases = getAliases(node);
		String convert = getConvert(node);
		String number = getNumber(node);

		if (_isException && statecode == null) {
			throw new RuntimeException("StateCode is required (" + name + ").");
		}

		try {
			writeSharedFiles(node, name, arguments, index);
			if (_differentMap == null || _differentMap.get(name) != null) {
				createNumberFile(number, name, arguments, description, statecode);
				createHeaderFile(name, arguments, description, statecode, level, aliases, convert);
				createClassFile(name, arguments, description, statecode, level, convert);
			}
		} catch (java.io.IOException e) {
			throw new RuntimeException("Can't create file: '" + e + "'");
		}
	}

	/** write to allXXX files */
	private static void writeSharedFiles(Element node, String name, String[] arguments, int index)
		throws java.io.IOException
	{
		Element formatList = (Element)node.getElementsByTagName(_formatTagName).item(0);

		String numberFileName = _moduleName + "/" + _numberFilePrefix + name + ".h";
		String headerFileName = _moduleName + "/" + _headerFilePrefix + name + ".h";

		/* add to allXXX files */
		_allNumberFileWriter.write("#include \"" + numberFileName + "\"");
		_allNumberFileWriter.newLine();
		if (_allClassFileWriter != null) {
			_allClassFileWriter.write("#include \"" + headerFileName + "\"");
			_allClassFileWriter.newLine();
		}
		if (_throwClassFileWriter != null) {
			_throwClassFileWriter.write("\tcase " + _nameSpace + "::" + name + ":");
			_throwClassFileWriter.newLine();
			_throwClassFileWriter.write("\t\tthrow " + _moduleName + "::" + name + "(cObject_);");
			_throwClassFileWriter.newLine();
		}

		String formatKey = "\t{" + _moduleName + "::" + _nameSpace + "::" + name +",";
		for (int i = 0; i < _languages.length; ++i) {
			NodeList formatNodeList = formatList.getElementsByTagName(_languages[i]);
			if (formatNodeList.getLength() > 0) {
				Node formatNode = formatNodeList.item(0);
				String message = formatNode.getChildNodes().item(0).getNodeValue().trim();

				_formatFileWriter[i].write(formatKey);
				_formatFileWriter[i].newLine();
				_formatFileWriter[i].write("\t\"" + message + "\"},");
				_formatFileWriter[i].newLine();
			}
		}

		_argumentFileWriter.write(formatKey);
		_argumentFileWriter.write("\t\"");

		if (arguments == null) {
			_argumentFileWriter.write("\\000");

		} else {
			for (int i = 0; i < arguments.length; ++i) {
				_argumentFileWriter.write(arguments[i] + "\\000");
			}
		}
		_argumentFileWriter.write("\"},");
		_argumentFileWriter.newLine();

		if (_isException) {
			// write Makefile_XX.h
			if ((index % _makeFileListLength) == 0) {
				String postfix = String.valueOf((index / _makeFileListLength) + 1);
				_makeFileObjectWriter.newLine();
				_makeFileObjectWriter.newLine();
				_makeFileObjectWriter.write("GENERATED_OBJS" + postfix + " = \\");
				_makeFileObjectWriter.newLine();

				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.newLine();
				_makeFileHeaderWriter.write("GENERATED_HDRS" + postfix + " = \\");
				_makeFileHeaderWriter.newLine();

				_makeFileNumberWriter.newLine();
				_makeFileNumberWriter.newLine();
				_makeFileNumberWriter.write("GENERATED_NUMBER_HDRS" + postfix + " = \\");
				_makeFileNumberWriter.newLine();
			} else {
				_makeFileObjectWriter.write(" \\");
				_makeFileObjectWriter.newLine();

				_makeFileHeaderWriter.write(" \\");
				_makeFileHeaderWriter.newLine();

				_makeFileNumberWriter.write(" \\");
				_makeFileNumberWriter.newLine();
			}
			_makeFileObjectWriter.write("\t" + name + "$O");
			_makeFileHeaderWriter.write("\t$(HDRDIR)/" + _headerFilePrefix + name + ".h");
			_makeFileNumberWriter.write("\t$(HDRDIR)/" + _numberFilePrefix + name + ".h");
		}
		if (_writeCppCvsignore) {
			// write cvsignore
			_cppCvsignoreWriter.write(name + ".cpp");
			_cppCvsignoreWriter.newLine();
		}
		if (_writeHCvsignore) {
			_hCvsignoreWriter.write(_headerFilePrefix + name + ".h");
			_hCvsignoreWriter.newLine();
			_hCvsignoreWriter.write(_numberFilePrefix + name + ".h");
			_hCvsignoreWriter.newLine();
		}
	}

	/** write the last part of makefile */
	private static void writeMakeFileTail(int length)
		throws java.io.IOException
	{
		_makeFileObjectWriter.newLine();
		_makeFileHeaderWriter.newLine();
		_makeFileNumberWriter.newLine();

		int n = ((length - 1) / _makeFileListLength) + 1;

		_makeFileObjectWriter.newLine();
		_makeFileObjectWriter.write("ObjectListTarget" + (n+1) +"($(TARGET), $(OBJS)");
		for (int i = 1; i <= n; ++i) {
			_makeFileObjectWriter.write(", $(GENERATED_OBJS" + i + ")");
		}
		_makeFileObjectWriter.write(", $(TOP_INSTALL_DIR))");
		_makeFileObjectWriter.newLine();

		_makeFileHeaderWriter.newLine();
		_makeFileNumberWriter.newLine();
		_makeFileObjectWriter.newLine();
		_makeFileHeaderWriter.write("InstallHeaderTarget($(GENERATED_HDRS_EXPORT), $(TOP_EXPORT_HDRDIR))");
		_makeFileHeaderWriter.newLine();
		for (int i = 1; i <= n; ++i) {
			_makeFileHeaderWriter.write("InstallHeaderTarget($(GENERATED_HDRS" + i + "), $(TOP_EXPORT_HDRDIR))");
			_makeFileHeaderWriter.newLine();
			_makeFileNumberWriter.write("InstallHeaderTarget($(GENERATED_NUMBER_HDRS" + i + "), $(TOP_EXPORT_HDRDIR))");
			_makeFileNumberWriter.newLine();
			_makeFileObjectWriter.write("CleanTarget($(GENERATED_OBJS" + i + "))");
			_makeFileObjectWriter.newLine();
		}

		_makeFileHeaderWriter.newLine();
		for (int i = 1; i <= n; ++i) {
			_makeFileHeaderWriter.write("TapeHeaderTarget($(GENERATED_HDRS" + i + "), $(PACKAGE_HDRDIR))");
			_makeFileHeaderWriter.newLine();
		}
	}

	/** create a number file */
	private static void createNumberFile(String number, String name, String[] arguments, String description, String statecode)
		throws java.io.IOException
	{
		String fileNameBase = _numberFilePrefix + name;
		String fileName = fileNameBase + ".h";

		BufferedWriter writer = createWriter(_moduleName, fileName);

		writer.write("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// " + fileName + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#ifndef __TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write("#define __TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#include \"" + _moduleName + "/Module.h\""); writer.newLine();
		writer.write("#include \"Exception/ErrorNumber.h\""); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_BEGIN"); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_" + _moduleName.toUpperCase() + "_BEGIN"); writer.newLine();
		writer.write(""); writer.newLine();
		if (_nameSpace1 != null) {
			writer.write("namespace " + _nameSpace1 + " {"); writer.newLine();
		}
		if (_nameSpace2 != null) {
			writer.write("namespace " + _nameSpace2 + " {"); writer.newLine();
		}
		writer.write("//	CONST"); writer.newLine();
		writer.write("//	" + _moduleName + "::" + _nameSpace + "::" + name + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		if (statecode != null) {
			writer.write("// StateCode(" + statecode + ")"); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("//	NOTES"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("const " + (_isException ? "" : "Exception::ErrorNumber::") + "Type " + name + " = " + number + ";"); writer.newLine();
		if (_nameSpace2 != null) {
			writer.write("}"); writer.newLine();
		}
		if (_nameSpace1 != null) {
			writer.write("}"); writer.newLine();
		}
		writer.write(""); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_" + _moduleName.toUpperCase() + "_END"); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_END"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#endif //__TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("//	Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("//	All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();

		terminateWriter(writer);
	}

	/** create a header file */
	private static void createHeaderFile(String name,
										 String[] arguments,
										 String description,
										 String statecode,
										 String level,
										 String[] aliases,
										 String convert)
		throws java.io.IOException
	{
		String fileNameBase = _headerFilePrefix + name;
		String fileName = fileNameBase + ".h";

		BufferedWriter writer = createWriter(_moduleName, fileName);
		writer.write("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// " + fileName + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#ifndef __TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write("#define __TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#include \"" + _moduleName + "/Module.h\""); writer.newLine();
		writer.write("#include \"Exception/" + level + ".h\""); writer.newLine();
		if ("ModLibraryError".equals(name)) {
			writer.write("#include \"ModException.h\""); writer.newLine();
		}
		writer.write(""); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_BEGIN"); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_" + _moduleName.toUpperCase() + "_BEGIN"); writer.newLine();
		writer.write(""); writer.newLine();
		if (_nameSpace1 != null) {
			writer.write("namespace " + _nameSpace1 + " {"); writer.newLine();
		}
		writer.write("//	CLASS"); writer.newLine();
		writer.write("//	" + _moduleName + "::" + (_nameSpace1 == null ? "" : _nameSpace1 + "::") + name + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		if (statecode != null) {
			writer.write("// StateCode(" + statecode + ")"); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("//	NOTES"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("class " + (_isException ? "SYD_EXCEPTION_FUNCTION" : "") + " " + name + " : public Exception::" + level); writer.newLine();
		writer.write("{"); writer.newLine();
		writer.write("public:"); writer.newLine();

		writeHeaderFile(writer, name, arguments, "");
		writeHeaderFile(writer, name, arguments, "Object");
		writeHeaderFile(writer, name, arguments, "Argument");
		if (convert != null) {
			writeHeaderFile(writer, name, arguments, "UnicodeArgument");
		}

		writer.write("};"); writer.newLine();
		if (_nameSpace1 != null) {
			writer.write("}"); writer.newLine();
		}
		writer.write(""); writer.newLine();
		if (aliases != null) {
			for (int i = 0; i < aliases.length; ++i) {
				writer.write("typedef " + name + " " + aliases[i]); writer.newLine();
			}
		}
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_" + _moduleName.toUpperCase() + "_END"); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_END"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#endif //__TRMEISTER_" + _moduleName.toUpperCase() + "_" + fileNameBase.toUpperCase() + "_H"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("//	All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();

		terminateWriter(writer);
	}

	/** write constructor declaration to a header file */
	private static void writeHeaderFile(BufferedWriter writer, String name, String[] arguments, String type)
		throws java.io.IOException
	{
		if (!_isException && "Argument".equals(type) && arguments == null) return;
		writer.write("\t//コンストラクタ"); writer.newLine();
		writer.write("\t" + name + "(");
		if ("Object".equals(type)) {
			writer.write("const Exception::Object& cObject_");

		} else if ("Argument".equals(type)) {

			if (_isException) {
				writer.write("const char* pszModuleName_,"); writer.newLine();
				writer.write("\t\tconst char* pszFileName_,"); writer.newLine();
				writer.write("\t\tint iLineNumber_");
				if (arguments != null) {
					writer.write(",");
					writer.newLine();
					writer.write("\t\t");
				}
			}

			if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					if (i > 0) {
						writer.write(",");
						writer.newLine();
						writer.write("\t\t");
					}
					else if ("ModLibraryError".equals(name)) {
						writer.write("const ModException& cException0_");
						continue;
					}
					writer.write(createType(arguments[i]) + " " + createVariable(arguments[i], i));
				}
			}

		} else if ("UnicodeArgument".equals(type)) {

			if (_isException) {
				writer.write("const ModUnicodeChar* pszModuleName_,"); writer.newLine();
				writer.write("\t\tconst ModUnicodeChar* pszFileName_,"); writer.newLine();
				writer.write("\t\tint iLineNumber_");
				if (arguments != null) {
					writer.write(",");
					writer.newLine();
					writer.write("\t\t");
				}
			}

			if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					if (i > 0) {
						writer.write(",");
						writer.newLine();
						writer.write("\t\t");
					}
					else if ("ModLibraryError".equals(name)) {
						writer.write("const ModException& cException0_");
						continue;
					}
					writer.write(createType(arguments[i]) + " " + createVariable(arguments[i], i));
				}
			}
		}
		writer.write(");"); writer.newLine();
	}

	/** create a class file */
	private static void createClassFile(String name,
										String[] arguments,
										String description,
										String statecode,
										String level,
										String convert)
		throws java.io.IOException
	{
		String numberFileNameBase = _numberFilePrefix + name;
		String fileNameBase = _headerFilePrefix + name;
		String fileName = fileNameBase + ".cpp";

		BufferedWriter writer = createWriter(null, fileName);

		writer.write("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-"); writer.newLine();
		writer.write("// vi:set ts=4 sw=4:"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// " + fileName + " --"); writer.newLine();
		if (description != null) {
			writer.write("//\t\t" + description); writer.newLine();
		}
		writer.write("//"); writer.newLine();
		writer.write("// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("// All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();
		/*
		writer.write("//	$Author$"); writer.newLine();
		writer.write("//	$Date$"); writer.newLine();
		writer.write("//	$Revision$"); writer.newLine();
		writer.write("//"); writer.newLine();
		*/
		writer.write(""); writer.newLine();
		writer.write("namespace {"); writer.newLine();
		writer.write("const char moduleName[] = \"Exception\";"); writer.newLine();
		writer.write("const char srcFile[] = __FILE__;"); writer.newLine();
		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write("#include \"SyDefault.h\""); writer.newLine();
		writer.write("#include \"SyReinterpretCast.h\""); writer.newLine();
		writer.write("#include \"" + _moduleName + "/Module.h\""); writer.newLine();
		writer.write("#include \"" + _moduleName + "/" + fileNameBase + ".h\""); writer.newLine();
		writer.write("#include \"" + _moduleName + "/" + numberFileNameBase + ".h\""); writer.newLine();
		writer.write("#include \"Exception/ErrorMessage.h\""); writer.newLine();
		if ("ModLibraryError".equals(name)) {
			writer.write("#include \"ModKanjiCode.h\""); writer.newLine();
			writer.write("#include \"ModUnicodeChar.h\""); writer.newLine();
			writer.write("#include \"ModUnicodeString.h\""); writer.newLine();
			writer.write("namespace {"); writer.newLine();
			writer.write("#ifdef	OS_WINDOWSNT4_0"); writer.newLine();
			writer.write("const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::shiftJis;"); writer.newLine();
			writer.write("#else"); writer.newLine();
			writer.write("const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::euc;"); writer.newLine();
			writer.write("#endif"); writer.newLine();
			writer.write("}"); writer.newLine();
		}
		writer.write(""); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_USING"); writer.newLine();
		writer.write("_" + (_isException ? "TRMEISTER" : "SYDNEY") + "_" + _moduleName.toUpperCase() + "_USING"); writer.newLine();
		writer.write(""); writer.newLine();

		writeClassFile(writer, name, arguments, statecode, level, "");
		writeClassFile(writer, name, arguments, statecode, level, "Object");
		writeClassFile(writer, name, arguments, statecode, level, "Argument");
		if (convert != null) {
			writeClassFile(writer, name, arguments, statecode, level, "UnicodeArgument");
		}

		writer.write("//"); writer.newLine();
		writer.write("//	Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007 Ricoh Company, Ltd."); writer.newLine();
		writer.write("//	All rights reserved."); writer.newLine();
		writer.write("//"); writer.newLine();

		terminateWriter(writer);
	}

	/** write constructor implementation to a class file */
	private static void writeClassFile(BufferedWriter writer,
									   String name,
									   String[] arguments,
									   String statecode,
									   String level,
									   String type)
		throws java.io.IOException
	{
		if (!_isException && "Argument".equals(type) && arguments == null) return;

		writer.write("// FUNCTION public"); writer.newLine();
		writer.write("// " + _moduleName + "::" + (_nameSpace1 == null ? "" : _nameSpace1 + "::") + name + "::" + name + " --"); writer.newLine();
		writer.write("//\t\tコンストラクタ"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// NOTES"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// ARGUMENTS"); writer.newLine();
		if ("".equals(type)) {
			writer.write("//\tなし"); writer.newLine();
		} else if ("Object".equals(type)) {
			writer.write("//\tconst Exception::Object& cObject_"); writer.newLine();
			writer.write("//\t\t内容をコピーする基底クラス"); writer.newLine();
		} else if ("Argument".equals(type)) {
			if (_isException) {
				writer.write("//\tconst char* pszModuleName_"); writer.newLine();
				writer.write("//\t\t例外が発生したモジュール名"); writer.newLine();
				writer.write("//\tconst char* pszFileName_"); writer.newLine();
				writer.write("//\t\t例外が発生したファイル名"); writer.newLine();
				writer.write("//\tint iLineNumber_"); writer.newLine();
				writer.write("//\t\t例外が発生した行番号"); writer.newLine();
			}
			writer.write("//\t(このクラス固有の引数)"); writer.newLine();;
		} else if ("UnicodeArgument".equals(type)) {
			if (_isException) {
				writer.write("//\tconst ModUnicodeChar* pszModuleName_"); writer.newLine();
				writer.write("//\t\t例外が発生したモジュール名"); writer.newLine();
				writer.write("//\tconst ModUnicodeChar* pszFileName_"); writer.newLine();
				writer.write("//\t\t例外が発生したファイル名"); writer.newLine();
				writer.write("//\tint iLineNumber_"); writer.newLine();
				writer.write("//\t\t例外が発生した行番号"); writer.newLine();
			}
			writer.write("//\t(このクラス固有の引数)"); writer.newLine();;
		} else {
			throw new RuntimeException("Unknown class generate type: " + type);
		}
		writer.write("//"); writer.newLine();
		writer.write("// RETURN"); writer.newLine();
		writer.write("//\tなし"); writer.newLine();
		writer.write("//"); writer.newLine();
		writer.write("// EXCEPTIONS"); writer.newLine();
		writer.write("//\tなし"); writer.newLine();
		writer.write(""); writer.newLine();
		writer.write((_nameSpace1 == null ? "" : _nameSpace1 + "::") + name + "::"); writer.newLine();
		writer.write(name + "(");
		if ("Object".equals(type)) {
			writer.write("const Exception::Object& cObject_");
		} else if ("Argument".equals(type)) {
			writer.write(""); writer.newLine();
			if (_isException) {
				writer.write("\tconst char* pszModuleName_,"); writer.newLine();
				writer.write("\tconst char* pszFileName_,"); writer.newLine();
				writer.write("\tint iLineNumber_");
				if (arguments != null) writer.write(","); writer.newLine();
			}
			if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					if (i > 0) {
						writer.write(","); writer.newLine();
					}
					else if ("ModLibraryError".equals(name)) {
						writer.write("\tconst ModException& cException0_");
						continue;
					}
					writer.write("\t" + createType(arguments[i]) + " " + createVariable(arguments[i], i));
				}
			}
		} else if ("UnicodeArgument".equals(type)) {
			writer.write(""); writer.newLine();
			if (_isException) {
				writer.write("\tconst ModUnicodeChar* pszModuleName_,"); writer.newLine();
				writer.write("\tconst ModUnicodeChar* pszFileName_,"); writer.newLine();
				writer.write("\tint iLineNumber_");
				if (arguments != null) writer.write(","); writer.newLine();
			}
			if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					if (i > 0) {
						writer.write(","); writer.newLine();
					}
					else if ("ModLibraryError".equals(name)) {
						writer.write("\tconst ModException& cException0_");
						continue;
					}
					writer.write("\t" + createType(arguments[i]) + " " + createVariable(arguments[i], i));
				}
			}
		}
		writer.write(")"); writer.newLine();
		writer.write(": Exception::" + level + "(");
		if ("Object".equals(type)) {
			writer.write("cObject_, \"" + statecode + "\"");
		} else {
			writer.write((_nameSpace2 == null ? "" : _nameSpace2 + "::") + name);
			if (!"".equals(type) && _isException) {
				writer.write(","); writer.newLine();
				writer.write("\t\tpszModuleName_, pszFileName_, iLineNumber_, \"" + statecode + "\"");
			}
		}
		writer.write(")"); writer.newLine();
		writer.write("{"); writer.newLine();
		if ("Argument".equals(type) || "UnicodeArgument".equals(type)) {
			writer.write("\t//メッセージ引数を作成する"); writer.newLine();
			if ("ModLibraryError".equals(name)) {
				writer.write("\tModException& cException = const_cast<ModException&>(cException0_);"); writer.newLine();
				writer.write("\tModUnicodeString strMsg(cException.setMessage(), 0, _eLiteralCode);"); writer.newLine();
			}
			writer.write("\tException::ErrorMessage::makeMessageArgument(getErrorMessageArgument(), getErrorNumber()");
			if ("ModLibraryError".equals(name)) {
				writer.write(", (const ModUnicodeChar*)strMsg");
			} else if (arguments != null) {
				for (int i = 0; i < arguments.length; ++i) {
					writer.write(", " + createVariable(arguments[i], i));
				}
			}
			writer.write(");"); writer.newLine();
		}
		writer.write("}"); writer.newLine();
		writer.write(""); writer.newLine();
	}

	private static java.util.HashMap _mapType;
	private static java.util.HashMap _mapVariable;

	/** create type string from %d or %s */
	private static String createType(String argument)
	{
		if (_mapType == null) {
			_mapType = new java.util.HashMap();
			_mapType.put("%s", "const ModUnicodeChar*");
			_mapType.put("%hd", "short");
			_mapType.put("%hi", "short");
			_mapType.put("%hu", "unsigned short");
			_mapType.put("%ho", "unsigned short");
			_mapType.put("%hx", "unsigned short");
			_mapType.put("%hX", "unsigned short");
			_mapType.put("%lld", "ModInt64");
			_mapType.put("%lli", "ModInt64");
			_mapType.put("%llu", "ModInt64");
			_mapType.put("%llo", "ModInt64");
			_mapType.put("%llx", "ModInt64");
			_mapType.put("%llX", "ModInt64");
			_mapType.put("%ld", "long");
			_mapType.put("%li", "long");
			_mapType.put("%lu", "unsigned long");
			_mapType.put("%lo", "unsigned long");
			_mapType.put("%lx", "unsigned long");
			_mapType.put("%lX", "unsigned long");
			_mapType.put("%d", "int");
			_mapType.put("%i", "int");
			_mapType.put("%u", "unsigned int");
			_mapType.put("%o", "unsigned int");
			_mapType.put("%x", "unsigned int");
			_mapType.put("%X", "unsigned int");
		}
		String result = (String)_mapType.get(argument);
		if (result == null) {
			throw new RuntimeException("Unknown format string: " + argument);
		}
		return result;
	}

	/** create variable name from %d or %s */
	private static String createVariable(String argument, int i)
	{
		if (_mapVariable == null) {
			_mapVariable = new java.util.HashMap();
			_mapVariable.put("%s", "pszStrArg");
			_mapVariable.put("%hd", "iShortArg");
			_mapVariable.put("%hi", "iShortArg");
			_mapVariable.put("%hu", "uShortArg");
			_mapVariable.put("%ho", "uShortArg");
			_mapVariable.put("%hx", "uShortArg");
			_mapVariable.put("%hX", "uShortArg");
			_mapVariable.put("%lld", "iInt64Arg");
			_mapVariable.put("%lli", "iInt64Arg");
			_mapVariable.put("%llu", "iInt64Arg");
			_mapVariable.put("%llo", "iInt64Arg");
			_mapVariable.put("%llx", "iInt64Arg");
			_mapVariable.put("%llX", "iInt64Arg");
			_mapVariable.put("%ld", "iLongArg");
			_mapVariable.put("%li", "iLongArg");
			_mapVariable.put("%lu", "uLongArg");
			_mapVariable.put("%lo", "uLongArg");
			_mapVariable.put("%lx", "uLongArg");
			_mapVariable.put("%lX", "uLongArg");
			_mapVariable.put("%d", "iIntArg");
			_mapVariable.put("%i", "iIntArg");
			_mapVariable.put("%u", "uIntArg");
			_mapVariable.put("%o", "uIntArg");
			_mapVariable.put("%x", "uIntArg");
			_mapVariable.put("%X", "uIntArg");
		}
		String result = (String)_mapVariable.get(argument);
		if (result == null) {
			throw new RuntimeException("Unknown format string: " + argument);
		}
		return result + i + "_";
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

	/** get convert */
	private static String getConvert(Element node)
	{
		NodeList nodeList = node.getElementsByTagName(_convertTagName);
		if (nodeList.getLength() > 0) {
			Node convertNode = nodeList.item(0);
			return convertNode.getChildNodes().item(0).getNodeValue().trim();
		}
		return null;
	}

	/** get number */
	private static String getNumber(Element node)
	{
		Node numberNode = node.getElementsByTagName(_numberTagName).item(0);
		return numberNode.getChildNodes().item(0).getNodeValue().trim();
	}
}

/*
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
 * All rights reserved.
 */
