/* -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
 * vi:set ts=4 sw=4:
 *
 * jp.co.ricoh.sydney.build.MessageGeneratorForPerl.java --
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
public class MessageGeneratorForPerl
{
	private final static String _USAGE = "MessageGeneratorForPerl moduleName definitionFileName charSet";
	private final static int _NUMARGS = 3;
	private final static String[] _languages =
	{
		"Japanese",
		"English",
	};
	private final static String _dirName = "Perl";

	private static String _moduleName;
	private static boolean _isException;
	private static Document _document;
	private static String _charSet;
	private static File _file;

	private final static String _numberTagName = "Number";

	private static String _nodeTagName;
	private static String _formatTagName;

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

		_nodeTagName = (_isException ? "Error" : "Message");

		_formatTagName = (_isException ? "Message" : "Format");

		load();
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

		} catch (ParserConfigurationException e) {
			throw new RuntimeException("Can't create new DocumentBuilder: '" + e + "'");

		} catch (org.xml.sax.SAXException e) {
			throw new RuntimeException("Parse error of " + _file + ": '" + e + "'");

		} catch (IOException e) {
			throw new RuntimeException("File read error of " + _file + ": '" + e + "'");
		}
	}

	/** create files */
	private static void createFiles()
	{
		final String fileName = (_isException ? "ErrorDef.pl" : null);

		try {
			/* prepare file */
			BufferedWriter fileWriter = createWriter(_dirName, fileName);

			fileWriter.write("########################################################"); fileWriter.newLine();
			fileWriter.write("# Net::ErrorDef -- error messages definition table"); fileWriter.newLine();
			fileWriter.write("########################################################"); fileWriter.newLine();
			fileWriter.write("package Net::TRMeister;"); fileWriter.newLine();
			fileWriter.newLine();
			fileWriter.write("our %ErrorDef ="); fileWriter.newLine();
			fileWriter.write("("); fileWriter.newLine();

			NodeList nodeList = _document.getElementsByTagName(_nodeTagName);
			int nodeLength = nodeList.getLength();


			for (String language: _languages) {
				fileWriter.write("\t\"" + language + "\" =>"); fileWriter.newLine();
				fileWriter.write("\t{"); fileWriter.newLine();

				for (int i = 0; i < nodeLength; ++i) {
					createFile(fileWriter, (Element)nodeList.item(i), language);
				}
				fileWriter.write("\t},"); fileWriter.newLine();
			}

			fileWriter.write(")"); fileWriter.newLine();

			/* terminate files */
			terminateWriter(fileWriter);
		} catch (java.io.IOException e) {
			throw new RuntimeException("Can't create file: '" + e + "'");
		}
	}

	/** create files according to a message/error definition */
	private static void createFile(BufferedWriter fileWriter,
								   Element node,
								   String language)
		throws java.io.IOException
	{
		Node numberNode = node.getElementsByTagName(_numberTagName).item(0);
		String number = numberNode.getChildNodes().item(0).getNodeValue().trim();
		Element formatList = (Element)node.getElementsByTagName(_formatTagName).item(0);
		NodeList formatNodeList = formatList.getElementsByTagName(language);
		if (formatNodeList.getLength() > 0) {
			Node formatNode = formatNodeList.item(0);
			String message = formatNode.getChildNodes().item(0).getNodeValue().trim();

			fileWriter.write("\t\t" + number + " => \"" + message + "\","); fileWriter.newLine();
		}
	}

	private static java.util.HashMap _mapType;
	private static java.util.HashMap _mapConvert;

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
						java.nio.charset.Charset.forName("utf-8")));

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
}

/*
 * Copyright (c) 2011, 2023 Ricoh Company, Ltd.
 * All rights reserved.
 */
