' -*-Mode: C++; tab-width: 4; c-basic-offset:4;-*-
'
' MessageGenerator.vbs -- メッセージクラスを自動生成するVBScript
'
Option Explicit

'------------------------
'実行スクリプトの種類
'------------------------
Dim scriptIsWScript
If Right(UCase(WScript.FullName), 11) = "WSCRIPT.EXE" Then
	scriptIsWScript = 1
Else
	scriptIsWScript = 0
End If

'------------------------
'コマンドライン引数の解析
'------------------------

Dim definitionFile		'メッセージ定義ファイルのファイル名
Dim arguments
Set arguments = WScript.Arguments
If arguments.Count > 0 Then
	definitionFile = arguments(0)
Else
	definitionFile = "MessageDefinition.xml"
End If

Dim msdevDir			'MSDEVのあるパス名
If arguments.Count > 1 Then
	msdevDir = arguments(1)
Else
	msdevDir = "."
End If

Dim sydFunction			'SYD_FUNCTIONの代わりに使うシンボル
If arguments.Count > 2 Then
	sydFunction = arguments(2)
Else
	sydFunction = "SYD_FUNCTION"
End If

Dim Languages
Languages = Array("Japanese", "English")
Const maxLanguage = 1

Dim MessageFormat(1)
Dim MessageArgument
Dim AllNumberFiles
Dim AllObjectFiles
Dim ThrowClassInstance

Dim IgnoredError
IgnoredError = -2147220728

Dim Targets
Targets = Array("Debug", "Release", "Purify", "Quantify")
Const maxTargets = 3

'--------------------------------------------------------
'作業ディレクトリの取得とヘッダを置くディレクトリーの作成
'--------------------------------------------------------
Dim fileSystem
Set fileSystem = createObject("Scripting.FileSystemObject")
Dim topDir				'.cppと.hを作るディレクトリのトップ
topDir = fileSystem.GetAbsolutePathName(".")
Dim moduleName
moduleName = fileSystem.GetFileName(topDir)
Dim folderName
folderName = topDir & "/" & moduleName
If fileSystem.FolderExists(folderName) = False Then
	fileSystem.CreateFolder(folderName)
End If

'------------------------------------------
'ファイルを追加するべきワークスペースの検査
'------------------------------------------
Dim projectFileIsChanged		'プロジェクトファイルに変更があったか
projectFileIsChanged = False
Dim projectFileIsReadOnly		'プロジェクトファイルが読み込み専用か
Dim projectFileName
projectFileName = topDir & "/" & moduleName & ".dsp"
Dim projectFile
Set projectFile = fileSystem.GetFile(projectFileName)
If projectFile.Attributes and 1 Then
	'メッセージダイアログを出すとうざいのでコメントアウト(2001/01/10)
	'MsgBox moduleName & "のプロジェクトファイルが読み込み専用です。" & vbNewLine & "新しいメッセージを追加していたらまずプロジェクトファイルをチェックアウトしてください。" & vbNewLine & "メッセージを追加していなければそのままで結構です。", vbExclamation, "注意"
	projectFileIsReadOnly = True
Else
	projectFileIsReadOnly = False
End If

'--------------------------------
'例外定義のエントリーを表すクラス
'--------------------------------
Class ErrorDefinition

	Public name
	Public number
	Public argument()
	Public maxArgument
	Public message()
	Public description()
	Public solution()

	Sub Class_Initialize()
		ReDim message(maxLanguage)
		ReDim description(maxLanguage)
		ReDim solution(maxLanguage)
	End Sub
	Function parse(node_, bakList_)
		Dim nameNode
		Set nameNode = node_.selectSingleNode("Name")
		name = nameNode.text

		Dim currNode
		Set currNode = node_.selectSingleNode("Number")
		If Not currNode Is Nothing Then
			number = currNode.text
		End If

		'------------------------------
		'すべてのNumberファイルを並べる
		'------------------------------
		AllNumberFiles = AllNumberFiles & "#include """ & moduleName & "/MessageNumber_" & name & ".h""" & vbNewLine

		'------------------------------------
		'すべてのメッセージクラスをインクルードする
		'------------------------------------
		AllObjectFiles = AllObjectFiles & "#include """ & moduleName & "/Message_" & name & ".h""" & vbNewLine

		Dim prefix
		prefix = vbTab & "{" & moduleName & "::Message::Number::" & name & "," & vbNewLine

		Set currNode = node_.selectSingleNode("Format")
		If Not currNode Is Nothing Then
			Dim childNode
			Dim l
			For l = 0 To maxLanguage
				Set childNode = currNode.selectSingleNode(Languages(l))
				If Not childNode Is Nothing Then
					message(l) = childNode.text
					'-------------------
					'MessageFormatも作る
					'-------------------
					MessageFormat(l) = MessageFormat(l) & prefix & vbTab & """" & message(l) & """}," & vbNewLine
				End If
			Next
		End If

		Dim nodeList
		Set nodeList = node_.selectNodes("Argument")
		'-------------------
		'MessageArgumentも作る
		'-------------------
		MessageArgument = MessageArgument & prefix & vbTab & """"
		If nodeList.length = 0 Then
			maxArgument = 0
			Redim argument(0)
			argument(0) = ""
			MessageArgument = MessageArgument & "\000"
		Else
			maxArgument = nodeList.length - 1
			Redim argument(maxArgument)
			Dim i
			For i = 0 To maxArgument
				Set childNode = nodeList.nextNode
				argument(i) = childNode.text
				MessageArgument = MessageArgument & argument(i) & "\000"
			Next
		End If
		MessageArgument = MessageArgument & """}," & vbNewLine

		'------------------------
		'ここでチェックして抜ける
		'------------------------
		If checkNode(nameNode, bakList_) = False Then
			parse = False
			Exit Function
		End If

		parse = True
	End Function

	Function checkNode(node_, bakList_)
		Dim node
		Dim bakNode
		If IsEmpty(bakList_) = False Then
			Dim i
			For i = 0 To bakList_.length - 1
				Set bakNode = bakList_.nextNode
				If bakNode.text = node_.text Then
					If bakNode.parentNode.text = node_.parentNode.text Then
						Set node = node_
					End If
				End If
			Next
			bakList_.reset()
		End If

		CheckNode = IsEmpty(node)
	End Function

	Sub write(project_)
		'--------------------------
		'メッセージ内容定義クラスの.hと.cppを作る
		'--------------------------
		createNumberFile project_, folderName & "/MessageNumber_" & name & ".h"
		createHeaderFile project_, folderName & "/Message_" & name & ".h"
		createCppFile project_, topDir & "/Message_" & name & ".cpp"

	End Sub

	Sub createNumberFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName_

		Dim headerFile
		Set headerFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		headerFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		headerFile.WriteLine("// vi:set ts=4 sw=4:")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// MessageNumber_" & name & ".h -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Copyright (c) 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("// All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.WriteLine("")
		headerFile.WriteLine("#ifndef __SYDNEY_" & UCase(moduleName) & "_MESSAGENUMBER_" & UCase(name) & "_H")
		headerFile.WriteLine("#define __SYDNEY_" & UCase(moduleName) & "_MESSAGENUMBER_" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("#include """ & moduleName & "/Module.h""")
		headerFile.WriteLine("#include ""Exception/ErrorNumber.h""")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_BEGIN")
		headerFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_BEGIN")
		headerFile.WriteLine("")
		headerFile.WriteLine("namespace Message {")
		headerFile.WriteLine("namespace Number {")
		headerFile.WriteLine("//	CONST")
		headerFile.WriteLine("//	" & moduleName & "::Message::Number::" & name & " -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	NOTES")
		headerFile.WriteLine("")
		headerFile.WriteLine("const Exception::ErrorNumber::Type " & name & " = " & number & ";")
		headerFile.WriteLine("}")
		headerFile.WriteLine("}")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_END")
		headerFile.WriteLine("_SYDNEY_END")
		headerFile.WriteLine("")
		headerFile.WriteLine("#endif //__SYDNEY_" & UCase(moduleName) & "_MESSAGENUMBER_" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	Copyright (c) 2001 Ricoh Company, Ltd.")
		headerFile.WriteLine("//	All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.Close()
	End Sub

	Sub createHeaderFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName_

		Dim headerFile
		Set headerFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		headerFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		headerFile.WriteLine("// vi:set ts=4 sw=4:")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Message_" & name & ".h -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("// Copyright (c) 2001, 2002 Ricoh Company, Ltd.")
		headerFile.WriteLine("// All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.WriteLine("")
		headerFile.WriteLine("#ifndef __SYDNEY_" & UCase(moduleName) & "_MESSAGE_" & UCase(name) & "_H")
		headerFile.WriteLine("#define __SYDNEY_" & UCase(moduleName) & "_MESSAGE_" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("#include """ & moduleName & "/Module.h""")
		headerFile.WriteLine("#include ""Exception/Object.h""")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_BEGIN")
		headerFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_BEGIN")
		headerFile.WriteLine("")
		headerFile.WriteLine("namespace Message {")
		headerFile.WriteLine("//	CLASS")
		headerFile.WriteLine("//	" & moduleName & "::Message::" & name & " -- " & notes)
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	NOTES")
		headerFile.WriteLine("")
		'headerFile.WriteLine("class " & sydFunction & " " & name & " : public Exception::Object")
		headerFile.WriteLine("class " & " " & name & " : public Exception::Object")
		headerFile.WriteLine("{")
		headerFile.WriteLine("public:")
		writeHeaderFunction headerFile, ""
		writeHeaderFunction headerFile, "Object"
		writeHeaderFunction headerFile, "Argument"
		headerFile.WriteLine("};")
		headerFile.WriteLine("}")
		headerFile.WriteLine("")
		headerFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_END")
		headerFile.WriteLine("_SYDNEY_END")
		headerFile.WriteLine("")
		headerFile.WriteLine("#endif //__SYDNEY_" & UCase(moduleName) & "_MESSAGE_" & UCase(name) & "_H")
		headerFile.WriteLine("")
		headerFile.WriteLine("//")
		headerFile.WriteLine("//	Copyright (c) 2001, 2002 Ricoh Company, Ltd.")
		headerFile.WriteLine("//	All rights reserved.")
		headerFile.WriteLine("//")
		headerFile.Close()
	End Sub

	Sub createCppFile(project_, fileName_)
		'プロジェクトにファイルを加える
		AddCppFile project_, fileName_

		Dim cppFile
		Set cppFile = fileSystem.OpenTextFile(fileName_, 2, True, 0)

		Dim notes
		If IsNull(description(0)) = False Then
			notes = Replace(description(0), vbLf, vbLf & "//")
		Else
			notes = ""
		End If

		cppFile.WriteLine("// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-")
		cppFile.WriteLine("// vi:set ts=4 sw=4:")
		cppFile.WriteLine("//")
		cppFile.WriteLine("// !!!This file is AUTOMATICALY GENERATED. DO NOT EDIT!!!")
		cppFile.WriteLine("//")
		cppFile.WriteLine("// Message_" & name & ".cpp -- " & notes)
		cppFile.WriteLine("//")
		cppFile.WriteLine("// Copyright (c) 2001 Ricoh Company, Ltd.")
		cppFile.WriteLine("// All rights reserved.")
		cppFile.WriteLine("//")
		cppFile.WriteLine("//	$Author$")
		cppFile.WriteLine("//	$Date$")
		cppFile.WriteLine("//	$Revision$")
		cppFile.WriteLine("//")
		cppFile.WriteLine("")
		cppFile.WriteLine("namespace {")
		cppFile.WriteLine("const char rcsid[] = ""$Header$"";")
		cppFile.WriteLine("}")
		cppFile.WriteLine("")
		cppFile.WriteLine("#include ""SyDefault.h""")
		cppFile.WriteLine("#include """ & moduleName & "/Module.h""")
		cppFile.WriteLine("#include """ & moduleName & "/Message_" & name & ".h""")
		cppFile.WriteLine("#include """ & moduleName & "/MessageNumber_" & name & ".h""")
		cppFile.WriteLine("#include ""Exception/ErrorMessage.h""")
		cppFile.WriteLine("")
		cppFile.WriteLine("_SYDNEY_USING")
		cppFile.WriteLine("_SYDNEY_" & UCase(moduleName) & "_USING")
		cppFile.WriteLine("")
		writeCppFunction cppFile, ""
		writeCppFunction cppFile, "Object"
		writeCppFunction cppFile, "Argument"
		cppFile.WriteLine("//")
		cppFile.WriteLine("//	Copyright (c) 2001 Ricoh Company, Ltd.")
		cppFile.WriteLine("//	All rights reserved.")
		cppFile.WriteLine("//")
	End Sub

	Sub writeHeaderFunction(file_, stringType_)
		If stringType_ <> "" and stringType_ <> "Object" and stringType_ <> "Argument" Then
			Exit Sub
		End If
		If stringType_ = "Argument" and argument(0) = "" Then
			Exit Sub
		End If

		file_.WriteLine("	//コンストラクタ")
		file_.Write("	" & name & "(")
		If stringType_ = "Object" Then
			file_.Write("const Exception::Object& cObject_")
		ElseIf stringType_ <> "" Then
			Dim i
			For i = 0 To maxArgument
				If i > 0 Then
					file_.WriteLine(",")
				End If

				Dim extraArgument
				extraArgument = createType(argument(i), i) & " " & createVariable(argument(i), i)
				file_.Write("						   " & extraArgument)
			Next
		End If
		file_.WriteLine(");")
	End Sub

	Sub writeCppFunction(file_, stringType_)
		If stringType_ <> "" and stringType_ <> "Object" and stringType_ <> "Argument" Then
			Exit Sub
		End If
		If stringType_ = "Argument" and argument(0) = "" Then
			Exit Sub
		End If

		file_.WriteLine("//	FUNCTION public")
		file_.WriteLine("//	" & moduleName & "::Message::" & name & "::" & name & " --")
		file_.WriteLine("//		コンストラクタ")
		file_.WriteLine("//")
		file_.WriteLine("//	NOTES")
		file_.WriteLine("//")
		file_.WriteLine("//	ARGUMENTS")
		If stringType_ = "" Then
			file_.WriteLine("//	なし")
		ElseIf stringType_ = "Object" Then
			file_.WriteLine("// const Exception::Object& cObject_")
			file_.WriteLine("//		内容をコピーする基底クラス")
		Else
			file_.WriteLine("//	(このメッセージ固有の引数)")
		End If
		file_.WriteLine("//")
		file_.WriteLine("//	RETURN")
		file_.WriteLine("//	なし")
		file_.WriteLine("//")
		file_.WriteLine("//	EXCEPTIONS")
		file_.WriteLine("//	なし")
		file_.WriteLine("")
		file_.WriteLine("Message::" & name & "::")
		file_.Write(name & "(")
		If stringType_ = "Object" Then
			file_.Write("const Exception::Object& cObject_")
		ElseIf stringType_ = "Argument" Then
			file_.WriteLine("")
			If argument(0) <> "" Then
				Dim i
				For i = 0 To maxArgument
					If i > 0 Then
						file_.WriteLine(",")
					End If

					Dim extraArgument
					extraArgument = createType(argument(i), i) & " " & createVariable(argument(i), i)
					file_.Write("	" & extraArgument)
				Next
			End If
		End If
		file_.WriteLine(")")
		file_.Write(": Exception::Object(")
		If stringType_ = "Object" Then
			file_.Write("cObject_")
		Else
			file_.Write("Number::" & name)
		End If
		file_.WriteLine(")")
		file_.WriteLine("{")
		If stringType_ = "Argument" Then
			file_.WriteLine("	//エラーメッセージ引数を作成する")
			file_.WriteLine("	Exception::ErrorMessage::makeMessageArgument(getErrorMessageArgument(),")
			file_.Write("									  getErrorNumber()")
			If argument(0) <> "" Then
				For i = 0 To maxArgument
					file_.WriteLine(",")
					Dim extraVariable
					extraVariable = createVariable(argument(i), i)
					file_.Write("									  " & extraVariable)
				Next
			End If
			file_.WriteLine(");")
		End If
		file_.WriteLine("}")
		file_.WriteLine("")
	End Sub

	Function createType(argument_, index_)

		Dim result
		Dim digit, digit2, digit3
		digit = Right(argument_, 1)
		digit2 = Left(Right(argument_, 2), 1)
		digit3 = Left(Right(argument_, 3), 1)

		If digit = "s" Then
			result = "const ModUnicodeChar* "
		Else
			If digit2 = "h" Then
				If digit = "d" or digit = "i" Then
					result = "short "
				ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
					result = "unsigned short "
				Else
					WScript.Echo definitionFile & " : error: Unknown format: " & argument_
					createType = ""
					Exit Function
				End If
			ElseIf digit2 = "l" Then
				If digit3 = "l" Then
					If digit = "d" or digit = "i" Then
						result = "ModInt64 "
					ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
						result = "ModInt64 "
					Else
						WScript.Echo definitionFile & " : error: Unknown format: " & argument_
						createType = ""
						Exit Function
					End If
				Else
					If digit = "d" or digit = "i" Then
						result = "long "
					ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
						result = "unsigned long "
					Else
						WScript.Echo definitionFile & " : error: Unknown format: " & argument_
						createType = ""
						Exit Function
					End If
				End If
			Else
				If digit = "d" or digit = "i" Then
					result = "int "
				ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
					result = "unsigned int "
				Else
					WScript.Echo definitionFile & " : error: Unknown format: " & argument_
					createType = ""
					Exit Function
				End If
   			End If
		End If
		createType = result
	End Function

	Function createVariable(argument_, index_)

		Dim result
		Dim digit
		digit = Right(argument_, 1)

		If digit = "s" Then
			result = "pszStrArg" & index_ & "_"
		Else
			If digit = "d" or digit = "i" Then
				result = "iIntArg" & index_ & "_"
			ElseIf digit = "u" or digit = "o" or digit = "x" or digit = "X" Then
				result = "uIntArg" & index_ & "_"
			Else
				WScript.Echo definitionFile & " : error: Unknown format: " & argument_
				createVariable = ""
				Exit Function
			End If
		End If
		createVariable = result
	End Function

End Class

Sub WriteMessageFile(project_)
	Dim file, fileName

	fileName = folderName & "/MessageAll_Number.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(AllNumberFiles)
	file.Close

	fileName = folderName & "/MessageAll_Class.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(AllObjectFiles)
	file.Close

	fileName = folderName & "/MessageArgument.h"

	'プロジェクトにファイルを加える
	AddHeaderFile project_, fileName

	Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
	file.WriteLine(MessageArgument)
	file.Close

	Dim l
	For l = 0 To maxLanguage

		fileName = folderName & "/MessageFormat_" & Languages(l) & ".h"

		'プロジェクトにファイルを加える
		AddHeaderFile project_, fileName

		Set file = fileSystem.OpenTextFile(fileName, 2, True, 0)
		file.WriteLine(MessageFormat(l))
		file.Close
	Next
End Sub


'==================
'ここから処理の開始
'==================

Dim ErrorCode
ErrorCode = 0

'---------------------------------------
'例外を定義しているXMLファイルを読み込む
'---------------------------------------
Dim xmlDoc
Set xmlDoc = CreateObject("microsoft.xmldom")
xmlDoc.async = False

On Error Resume Next
xmlDoc.load(definitionFile)
If xmlDoc.parseError.errorCode <> 0 Then
	On Error Goto 0
	ErrorCode = 17
	Err.Raise ErrorCode, ": error: メッセージ定義ファイルに誤りがあります。" & vbNewLine & xmlDoc.parseError.reason
End If
Err.Clear

'-------------------------------------------------
'XMLファイルの内容をチェックして正しければ実行する
'-------------------------------------------------
If CheckDefinition(xmlDoc) = False Then
	On Error Goto 0
	ErrorCode = 17
	Err.Raise ErrorCode, ": error: メッセージ定義ファイルに誤りがあります。"
Else
	On Error Goto 0
	Dim application
	Set application = WScript.CreateObject("msdev.application")
	application.Documents.Open projectFileName
	Dim project
	Set project = application.Projects("" & moduleName)

	'-----------------------------------------------------
	'前回生成に使用したXMLファイルのバックアップを読み込む
	'-----------------------------------------------------
	Dim xmlBak
	Set xmlBak = CreateObject("microsoft.xmldom")
	xmlDoc.async = False
	Dim bakList
	If xmlBak.load(definitionFile & "_") Then
		Set bakList = xmlBak.selectNodes("MessageDefine/Message/Name")
	End If

	'---------------------------------
	'<Message>タグごとに例外の定義がある
	'---------------------------------
	Dim nodeList
	Set nodeList = xmlDoc.getElementsByTagName("Message")

	'------------------------------------------------------------------
	' generated: 生成した例外の名称を並べる
	' noNeed:    前回から変更がないので生成しなかった例外の名称を並べる
	'------------------------------------------------------------------
	Dim generated
	Dim noNeed
	Dim i
	For i = 0 To nodeList.length - 1
		Dim errorNode
		Set errorNode = nodeList.nextNode

		Dim errorDef
		Set errorDef = New ErrorDefinition

		If errorDef.parse(errorNode, bakList) = False Then
			noNeed = noNeed & " " & errorDef.name
		Else
			generated = generated & " " & errorDef.name
			errorDef.write(project)
		End If
	Next
	'------------------------------------------------------------------
	'すべて読み終わったらMessageFormatとMessageArgumentをファイルに書く
	'------------------------------------------------------------------
	WriteMessageFile(project)
	application.Documents.SaveAll
	application.Quit

	'メッセージダイアログを出すとうざいのでコメントアウト(2001/01/10)
	'If noNeed <> "" Then
		'MsgBox "以下のクラスは変更がありませんでした。" & vbNewLine & noNeed, vbInformation, "変更のなかったクラス"
	'End If
	If scriptIsWScript = 0 and generated <> "" Then
		WScript.Echo "以下のクラスについてファイルを作成しました。" & vbNewLine & generated
	End If
	'タイムスタンプを新しくする
	xmlDoc.Save(definitionFile & "_")

	'アプリケーションをdisconnectする
	WScript.DisconnectObject application

	WScript.Quit ErrorCode
End If

'-------------------------------
'XMLファイルの内容をチェックする
'-------------------------------
Function CheckDefinition(doc_)
	Dim nameList
	Set nameList = doc_.selectNodes("MessageDefine/Message/Name")
	Dim numberList
	Set numberList = doc_.selectNodes("MessageDefine/Message/Number")

	If nameList.length <> numberList.length Then
		WScript.Echo definitionFile & " : error: <Name>タグと<Number>タグの数が合いません"
		CheckDefinition = False
		Exit Function
	End If

	Dim i
	For i = 0 To nameList.length - 1
		Dim nameNode
		Set nameNode = nameList.item(i)
		Dim numberNode
		Set numberNode = numberList.item(i)

		Dim j
		For j = i + 1 To nameList.length - 1
			Dim otherName
			Set otherName = nameList.item(j)
			Dim otherNumber
			Set otherNumber = numberList.item(j)
			If nameNode.text = otherName.text Then
				WScript.Echo definitionFile & " : error: 同じ名前のメッセージが複数あります: " & nameNode.text
				CheckDefinition = False
				Exit Function
			End If
			If numberNode.text = otherNumber.text Then
				WScript.Echo definitionFile & " : error: 同じ番号のメッセージが複数あります: " & numberNode.text
				CheckDefinition = False
				Exit Function
			End If
		Next
	Next
	CheckDefinition = True
End Function

Sub AddHeaderFile(project_, fileName_)
	On Error Resume Next
	project_.AddFile fileName_
	If Err.Number = 0 Then

		'ファイルが追加された

		If projectFileIsReadOnly Then

			'読み込み専用なのにファイルが追加されたらエラー

			On Error Goto 0
			ErrorCode = 17
			project_.Application.Quit
			Err.Raise ErrorCode, ": error : 新しいメッセージを追加したらまずプロジェクトファイルをチェックアウトしてください。"
		End If

		'---------------------------------------
		'Installの構成にカスタムビルドを設定する
		'---------------------------------------
		Dim configuration
		Set configuration = project_.Configurations(project_.Name & " - Win32 Install")
		configuration.AddCustomBuildStepToFile fileName_, "copy $(InputPath) ..\..\include\$(InputPath)", "..\..\include\$(InputPath)", "ヘッダーファイルのインストール中 - $(InputPath)"

		projectFileIsChanged = True

	ElseIf Err.Number <> -2147220728 Then	'すでにあることによる失敗(-2147220728)は無視する
		If Err.Number <> IgnoredError Then
			MsgBox "ファイルの追加に失敗しました(" & Err.Number & "): " & Err.Description & vbNewLine & "このエラーは無視します。 " & vbNewLine & "メッセージを追加したのにこのエラーが出たときは正しくビルドできません。", vbExclamation, "注意"
			IgnoredError = Err.Number
		End If
	End If
	Err.Clear
	On Error Goto 0
End Sub

Sub AddCppFile(project_, fileName_)
	On Error Resume Next
	project_.AddFile fileName_
	If Err.Number = 0 Then
		If projectFileIsReadOnly Then

			'読み込み専用なのにファイルが追加されたらエラー

			On Error Goto 0
			ErrorCode = 17
			project_.Application.Quit
			Err.Raise ErrorCode, ": error: 新しいメッセージを追加したらまずプロジェクトファイルをチェックアウトしてください。"
		End If

		projectFileIsChanged = True

	ElseIf Err.Number <> -2147220728 Then	'すでにあることによる失敗(-2147220728)は無視する
		If Err.Number <> IgnoredError Then
			MsgBox "ファイルの追加に失敗しました(" & Err.Number & "): " & Err.Description & vbNewLine & "このエラーは無視します。 " & vbNewLine & "メッセージを追加したのにこのエラーが出たときは正しくビルドできません。", vbExclamation, "注意"
			IgnoredError = Err.Number
		End If
	End If
	Err.Clear
End Sub

'
' Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
' All rights reserved.
'
