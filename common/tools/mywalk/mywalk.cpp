// MyWalk.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#define DUMP_MEMORY 1

namespace { // from pwalk

typedef struct tagDbgSection
    {
    char	    szSection[IMAGE_SIZEOF_SHORT_NAME];
    ULONG	    uVirtualAddress;
    ULONG	    uSize;
    struct tagDbgSection*   Next;

	tagDbgSection() : uVirtualAddress(0), uSize(0), Next(0) {szSection[0]=0;}
    }SECTIONINFO;
typedef SECTIONINFO   *LPSECTIONINFO;

/* retrieve name of module from module's open file handle */
void WINAPI RetrieveModuleName (
    char      *lpszModule,
    HANDLE    hFile)
{
    HANDLE		     hMapFile;
    LPVOID		     lpFile;
    char		     *lpszName;
    int 		     nSections;
    ULONG		     VAExportDir;
    int 		     i=0;
    int 		     ImageHdrOffset;
    PIMAGE_SECTION_HEADER    psh;
    PIMAGE_FILE_HEADER	     pfh;
    PIMAGE_OPTIONAL_HEADER   poh;
    PIMAGE_EXPORT_DIRECTORY  ped;


    /* memory map handle to DLL for easy access */
    hMapFile = CreateFileMapping (hFile,
				  (LPSECURITY_ATTRIBUTES)NULL,
				  PAGE_READONLY,
				  0,
				  0,
				  NULL);

    /* map view of entire file */
    lpFile = MapViewOfFile (hMapFile, FILE_MAP_READ, 0, 0, 0);

    /* if DOS based file */
    if (*((USHORT *)lpFile) == IMAGE_DOS_SIGNATURE)
	{
	/* file image header offset exists after DOS header and nt signature */
	ImageHdrOffset = (int)((ULONG *)lpFile)[15] + sizeof (ULONG);
	if (*((ULONG *)((char *)lpFile + ImageHdrOffset - sizeof (ULONG))) !=
	    IMAGE_NT_SIGNATURE)
	    {
	    strcpy (lpszModule, "Error, no IMAGE_NT_SIGNATURE");
	    goto EXIT;
	    }
	}

    pfh = (PIMAGE_FILE_HEADER)((char *)lpFile + ImageHdrOffset);

    /* if optional header exists and exports directory exists proceed */
    if (pfh->SizeOfOptionalHeader)
	{
	/* locate virtual address for Export Image Directory in OptionalHeader */
	poh = (PIMAGE_OPTIONAL_HEADER)((char *)pfh + sizeof (IMAGE_FILE_HEADER));
	VAExportDir = poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	/* locate section where export virtual address is located */
	psh = (PIMAGE_SECTION_HEADER)((char *)poh + pfh->SizeOfOptionalHeader);
	nSections = pfh->NumberOfSections;
	while (i++<nSections)
	    {
	    if (psh->VirtualAddress <= VAExportDir &&
		psh->VirtualAddress + psh->SizeOfRawData > VAExportDir)
		break;
	    psh++;
	    }

	/* locate export image directory */
	if (i < nSections)
	    ped = (PIMAGE_EXPORT_DIRECTORY)((char *)lpFile +
		(VAExportDir - psh->VirtualAddress) + psh->PointerToRawData);
	else
	    {
	    strcpy (lpszModule, "IMAGE_EXPORT_DIRECTORY not found");
	    goto EXIT;
	    }

	/* read name from export directory */
	lpszName = (char *)lpFile + ped->Name + (psh->PointerToRawData - psh->VirtualAddress);
	strcpy (lpszModule, lpszName);
	}

    else
	strcpy (lpszModule, "Error, no IMAGE_OPTIONAL_HEADER");

EXIT:
    /* clean up before exiting */
    UnmapViewOfFile (lpFile);
    CloseHandle (hMapFile);
}




/* retieve section names from module file handle */
void WINAPI RetrieveSectionNames (
    HANDLE	  hFile,
    SECTIONINFO   **pSection)
{
    HANDLE		     hMapFile;
    LPVOID		     lpFile;
    int 		     nSections;
    int 		     i=0;
    int 		     ImageHdrOffset;
    PIMAGE_SECTION_HEADER    psh;
    PIMAGE_FILE_HEADER	     pfh;
    SECTIONINFO 	     *ps;


    /* memory map handle to DLL for easy access */
    hMapFile = CreateFileMapping (hFile,
				  (LPSECURITY_ATTRIBUTES)NULL,
				  PAGE_READONLY,
				  0,
				  0,
				  NULL);

    /* map view of entire file */
    lpFile = MapViewOfFile (hMapFile, FILE_MAP_READ, 0, 0, 0);

    /* if DOS based file */
    if (*((USHORT *)lpFile) == IMAGE_DOS_SIGNATURE)
	{
	/* file image header offset exists after DOS header and nt signature */
	ImageHdrOffset = (int)((ULONG *)lpFile)[15] + sizeof (ULONG);
	if (*((ULONG *)((char *)lpFile + ImageHdrOffset - sizeof (ULONG))) !=
	    IMAGE_NT_SIGNATURE)
	    goto EXIT;
	}

    pfh = (PIMAGE_FILE_HEADER)((char *)lpFile + ImageHdrOffset);

    /* if optional header exists, offset first section header */
    psh = (PIMAGE_SECTION_HEADER)((char *)pfh +
	      sizeof (IMAGE_FILE_HEADER) + pfh->SizeOfOptionalHeader);

    /* allocate one section header for each section */
    ps = *pSection = new SECTIONINFO;
    nSections = pfh->NumberOfSections;
    while (TRUE)
	{
	strcpy (ps->szSection, (const char*)psh[i].Name);
	ps->uVirtualAddress = psh[i].VirtualAddress;
	ps->uSize = psh[i].SizeOfRawData;

	if (++i >= nSections)
	    break;

	/* allocate heap memory for sections */
	ps->Next = new SECTIONINFO;
	ps = (SECTIONINFO *)ps->Next;
	}

EXIT:
    /* clean up before exiting */
    UnmapViewOfFile (lpFile);
    CloseHandle (hMapFile);
}

}

namespace {
	unsigned int _pid = 0;
	bool bTrigger = false;
	unsigned short _iPortNumber = 0;
	int _iSeqNo = 0;
	bool _bDump = false;
	bool bDumpTrigger = false;
	unsigned short _iDumpPortNumber = 0;
	int _iDumpSeqNo = 0;
	bool _bSum = false;

	int _nThreads = 0;
	int _nDlls = 0;
	int _aThreads = 0;
	int _aDlls = 0;

	struct sockaddr_in _service;
	SOCKET _socket;
	struct sockaddr_in _dumpService;
	SOCKET _dumpSocket;

	struct _ThreadInfo
	{
		void setData(DWORD threadID, const CREATE_THREAD_DEBUG_INFO& threadInfo)
		{
			m_threadID = threadID;
			m_threadHandle = threadInfo.hThread;
			m_stackBaseAddress = 0;
		}
		void setData(DWORD threadID, const CREATE_PROCESS_DEBUG_INFO& processInfo)
		{
			m_threadID = threadID;
			m_threadHandle = processInfo.hThread;
			m_stackBaseAddress = 0;
		}

		DWORD m_threadID;
		HANDLE m_threadHandle;
		void* m_stackBaseAddress;

	} *_threads = 0;

	struct _DllInfo
	{
		void setData(const LOAD_DLL_DEBUG_INFO& dllInfo)
		{
			m_dllHandle = dllInfo.hFile;
			m_baseAddress = dllInfo.lpBaseOfDll;
			RetrieveModuleName (m_szImageName, dllInfo.hFile);
			RetrieveSectionNames (dllInfo.hFile, &(m_lpSection));
		}

		HANDLE m_dllHandle;
		void* m_baseAddress;
		char m_szImageName[MAX_PATH];
		SECTIONINFO* m_lpSection;  /* from PEFILE.H */
	} *_dlls = 0;

	template <class _X_>
	void _adjustArray(int& alloc_, int& num_, _X_*& array_)
	{
		if (!array_) {
			alloc_ = 10;
			array_ = new _X_[alloc_];
		}
		if (num_ + 1 >= alloc_) {
			alloc_ <<= 1;
			_X_* tmp = new _X_[alloc_];
			for (int i = 0; i < num_; ++i) {
				tmp[i] = array_[i];
			}
			delete [] array_;
			array_ = tmp;
		}
	}
	void _addThread(DWORD threadID, const CREATE_THREAD_DEBUG_INFO& threadInfo)
	{
		_adjustArray(_aThreads, _nThreads, _threads);
		_threads[_nThreads++].setData(threadID, threadInfo);
	}
	void _addThread(DWORD threadID, const CREATE_PROCESS_DEBUG_INFO& processInfo)
	{
		_adjustArray(_aThreads, _nThreads, _threads);
		_threads[_nThreads++].setData(threadID, processInfo);
	}
	void _removeThread(DWORD threadID)
	{
		for (int i = 0; i < _nThreads; ++i) {
			if (_threads[i].m_threadID == threadID) {
				for (int j = i + 1; j < _nThreads; ++j) {
					_threads[j - 1] = _threads[j];
				}
				--_nThreads;
				break;
			}
		}
	}
	void _addDll(const LOAD_DLL_DEBUG_INFO& dllInfo)
	{
		_adjustArray(_aDlls, _nDlls, _dlls);
		_dlls[_nDlls++].setData(dllInfo);
	}
	void _removeDll(const LOAD_DLL_DEBUG_INFO& dllInfo)
	{
		for (int i = 0; i < _nDlls; ++i) {
			if (_dlls[i].m_baseAddress == dllInfo.lpBaseOfDll) {
				SECTIONINFO *pSection, *pNext;
				pSection = pNext = _dlls[i].m_lpSection;
				while (pNext) {
					pNext = (SECTIONINFO*)pSection->Next;
					delete pSection;
					pSection = pNext;
				}
				for (int j = i + 1; j < _nDlls; ++j) {
					_dlls[j - 1] = _dlls[j];
				}
				--_nDlls;
				break;
			}
		}
	}

	void* _stackHead(HANDLE h)
	{
		CONTEXT			ThreadContext;

		ThreadContext.ContextFlags = CONTEXT_CONTROL;
		if (!GetThreadContext (h, &ThreadContext)) {
			DWORD err = GetLastError();
			fprintf(stderr, "Can't get thread context(%d)\n", err);
			return 0;
		}

#ifdef _X86_
		return (void*)ThreadContext.Esp;
#elif defined (_PPC_)
		return (void*) ThreadContext.Gpr1;
#else /* _MIPS_ */
		return (void*) ThreadContext.IntSp;
#endif
	}

	void _setThreadStack(HANDLE h)
	{
		MEMORY_BASIC_INFORMATION mbi;
		for (int i = 0; i < _nThreads; ++i) {
			void* pStack = _stackHead(_threads[i].m_threadHandle);
			VirtualQueryEx(h, pStack, &mbi, sizeof(mbi));
			_threads[i].m_stackBaseAddress = mbi.AllocationBase; // 0でもセットする
		}
	}

	void _suspendThread()
	{
		for (int i = 0; i < _nThreads; ++i) {
			::SuspendThread(_threads[i].m_threadHandle);
		}
	}

	void _resumeThread()
	{
		for (int i = 0; i < _nThreads; ++i) {
			::ResumeThread(_threads[i].m_threadHandle);
		}
	}
}

void walk(HANDLE h)
{
    SYSTEM_INFO   si;
    GetSystemInfo(&si);
	static char path[256];
	static char path2[256];
	static char empty = 0;
	char* option;

	FILE* pFile = NULL;
#ifdef DUMP_MEMORY
	if (_bDump) {
		sprintf(path2, "dump%04d_%04d.dat", _pid, _iDumpSeqNo++);
		pFile = fopen(path2, "wb");
	} else
#endif
	if (_bSum) {
		sprintf(path2, "memory%04d_sum.txt", _pid);
		pFile = fopen(path2, "ab");
	} else {
		sprintf(path2, "memory%04d_%04d.txt", _pid, _iSeqNo++);
		pFile = fopen(path2, "wb");
	}

	// すべてのThreadを止める
	_suspendThread();

	// Threadの情報をセットする
	_setThreadStack(h);

	// Commitされているページのサイズを合計する
	DWORD committed = 0;
	MEMORY_BASIC_INFORMATION mbi;
    LPVOID	  lpMem = 0;
    while (lpMem < si.lpMaximumApplicationAddress) {
		VirtualQueryEx(h, lpMem, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		option = &empty;
		{
			// Threadをチェックする
			for (int i = 0; i < _nThreads; ++i) {
				if (mbi.AllocationBase == _threads[i].m_stackBaseAddress) {
					sprintf(path, "(Stack#%x)", _threads[i].m_threadID);
					option = &path[0];
					break;
				}
			}
		}
		if (option == &empty)
		{
			// DLLをチェックする
			for (int i = 0; i < _nDlls; ++i) {
				if (mbi.AllocationBase == _dlls[i].m_baseAddress) {
					sprintf(path, "(%s)", _dlls[i].m_szImageName);
					option = &path[0];
					break;
				}
				SECTIONINFO* pSection = (SECTIONINFO*)_dlls[i].m_lpSection;
				while (pSection) {
					if (mbi.AllocationBase == (char*)_dlls[i].m_baseAddress + pSection->uVirtualAddress) {
						sprintf(path, "(%s:%s)", _dlls[i].m_szImageName, pSection->szSection);
						option = &path[0];
						break;
					}
					pSection = (SECTIONINFO*)pSection->Next;
				}
				if (pSection) break;
			}
		}

	    lpMem = (LPVOID)((DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize);
		if (mbi.State & MEM_COMMIT) {
			committed += (DWORD)mbi.RegionSize;
#ifdef DUMP_MEMORY
			if (_bDump) {
				fprintf(pFile, "--------Adress:%08x--------\n", (DWORD)mbi.BaseAddress);
				char* buffer = new char[mbi.RegionSize];
				ReadProcessMemory(h, mbi.BaseAddress, buffer, mbi.RegionSize, NULL);
				fwrite(buffer, mbi.RegionSize, 1, pFile);
				delete [] buffer;
			} else
#endif
			if (!_bSum) {
				fprintf(pFile, "Address: 0x%08x%s Size: %d Status: %s \n",
						(DWORD)mbi.BaseAddress, option,
						mbi.RegionSize,
						((mbi.Protect & PAGE_READWRITE) ? "RW" : ((mbi.Protect & PAGE_READONLY) ? "RO" : "NA")));
			}
		}
	}

	if (_bSum && !_bDump) {
		fprintf(pFile, "%d\t%u\n", _iSeqNo++, committed);
	}

	// すべてのThreadを再開する
	_resumeThread();
	fclose(pFile);

	if (!_bSum || _iSeqNo == 1)
		fprintf(stderr, "%s\n", path2);
}

unsigned long checkTrigger(void* dummy)
{
	if (_iPortNumber == 0)
	{
		HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleMode(hStdIn, ENABLE_PROCESSED_INPUT);
		while (true) {
			if (!bTrigger) {
				FlushConsoleInputBuffer(hStdIn);
				fprintf(stderr, "Press Any Key...");
				while (true) {
					if (WaitForSingleObject(hStdIn, 100) == WAIT_OBJECT_0) {
						DWORD n;
						if (GetNumberOfConsoleInputEvents(hStdIn, &n) && n) {
							INPUT_RECORD ir;
							for (DWORD i = 0; i < n; ++i) {
								DWORD nEvents;
								ReadConsoleInput(hStdIn, &ir, sizeof(ir), &nEvents);
								switch (ir.EventType) {
								case KEY_EVENT:
									{
										if (ir.Event.KeyEvent.bKeyDown) {
											bTrigger = true;
											fprintf(stderr, "\n");
										}
										break;
									}
								default:
									break;
								}
							}
						}
						FlushConsoleInputBuffer(hStdIn);
						break;
					}
				}
			}
			Sleep(1000);
		}
	}
	else
	{
		while (true)
		{
			if (bTrigger != true)
			{
				for (;;)
				{
					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(_socket, &fds);

					timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 500*1000;

					if (select(_socket + 1, &fds, 0, 0, &tv))
						break;
				}
				int size = sizeof(_service);
				SOCKET socket = accept(_socket, (struct sockaddr*)&_service, &size);
				bTrigger = true;

				closesocket(socket);
			}
			Sleep(1000);
		}
	}

	return 0;
}

unsigned long checkDumpTrigger(void* dummy)
{
	if (_iDumpPortNumber > 0)
	{
		while (true)
		{
			if (bDumpTrigger != true)
			{
				for (;;)
				{
					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(_dumpSocket, &fds);

					timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 500*1000;

					if (select(_dumpSocket + 1, &fds, 0, 0, &tv))
						break;
				}
				int size = sizeof(_dumpService);
				SOCKET socket = accept(_dumpSocket, (struct sockaddr*)&_dumpService, &size);
				bDumpTrigger = true;

				closesocket(socket);
			}
			Sleep(1000);
		}
	}

	return 0;
}

void USAGE()
{
	cerr << "MyWalk [/port portnumber] [/dump portnumber] [/sum] process_id" << endl;
}

int main(int argc, char* argv[])
{
	int i = 1;
	while (i < argc)
	{
		char* p = argv[i];
		if (*p == '-' || *p == '/')
		{
			char* p = argv[i++];
			p++;
			if (strcmp(p, "port") == 0 && i < argc)
			{
				_iPortNumber = atoi(argv[i++]);
			}
			else if (strcmp(p, "dump") == 0 && i < argc)
			{
				_iDumpPortNumber = atoi(argv[i++]);
			}
			else if (strcmp(p, "sum") == 0 && i < argc)
			{
				_bSum = true;
			}
			else
			{
				USAGE();
				return 1;
			}
		}
		else
		{
			if (_pid == 0)
			{
				_pid = atoi(p);
			}
			else
			{
				USAGE();
				return 1;
			}
			i++;
		}
	}
	
	if (_pid == 0)
	{
		USAGE();
		return 1;
	}

#ifdef DUMP_MEMORY
	fprintf(stderr, "dump=%s\n", _iDumpPortNumber>0?"yes":"no");
#endif
	fprintf(stderr, "sum=%s\n", _bSum?"yes":"no");

	if (_iPortNumber || _iDumpPortNumber)
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(1,1), &wsaData);
	}
	if (_iPortNumber)
	{
		_service.sin_family = AF_INET;
		_service.sin_addr.s_addr = htonl(INADDR_ANY);
		_service.sin_port = htons(_iPortNumber);

		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int on = 1;
		setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

		bind(_socket, (struct sockaddr*)&_service, sizeof(_service));

		listen(_socket, 5);
	}
	if (_iDumpPortNumber)
	{
		_dumpService.sin_family = AF_INET;
		_dumpService.sin_addr.s_addr = htonl(INADDR_ANY);
		_dumpService.sin_port = htons(_iDumpPortNumber);

		_dumpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int on = 1;
		setsockopt(_dumpSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

		bind(_dumpSocket, (struct sockaddr*)&_dumpService, sizeof(_dumpService));

		listen(_dumpSocket, 5);
	}

	LUID debugName;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &debugName)) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't lookup privilege value(%d)\n", err);
		return 1;
	}
	TOKEN_PRIVILEGES priv;
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = debugName;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	HANDLE thisProcess = GetCurrentProcess();
	if (thisProcess == NULL) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't get current process(%d)\n", err);
		return 1;
	}
	HANDLE token;
	if (!OpenProcessToken(thisProcess, TOKEN_ADJUST_PRIVILEGES, &token)) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't open process token(%d)\n", err);
		return 1;
	}
	if (!AdjustTokenPrivileges(token, FALSE, &priv, 0, NULL, NULL)) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't adjust token privileges(%d)\n", err);
		return 1;
	}
	CloseHandle(token);

#if 0
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, (DWORD)_pid);
	if (h == NULL) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't get process: %d(%d)\n", _pid, err);
		return 1;
	}
	walk(h);
	CloseHandle(h);
#endif

	if (!DebugActiveProcess(_pid)) {
		// failed
		DWORD err = GetLastError();
		fprintf(stderr, "Can't attach process: %d(%d)\n", _pid, err);
		return 1;
	}

	// Create trigger thread
	if (!CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)checkTrigger, NULL, 0, NULL)) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't create thread(%d)\n", err);
		return 1;
	}

	// Create dump trigger thread
	if (!CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)checkDumpTrigger, NULL, 0, NULL)) {
		DWORD err = GetLastError();
		fprintf(stderr, "Can't create thread(%d)\n", err);
		return 1;
	}

	HANDLE targetProcess;
	DEBUG_EVENT de;
	bool bContinue = true;
	while (bContinue) {
		bool bFirstException = false;
		if (WaitForDebugEvent(&de, 100)) {
			if (de.dwProcessId == _pid) {
				switch (de.dwDebugEventCode) {
				case EXCEPTION_DEBUG_EVENT:
					if (de.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_BREAKPOINT) {
						bFirstException = (de.u.Exception.dwFirstChance != 0);
					}
					break;
				case CREATE_THREAD_DEBUG_EVENT:
					_addThread(de.dwThreadId, de.u.CreateThread);
					break;
			    case CREATE_PROCESS_DEBUG_EVENT:
					targetProcess = de.u.CreateProcessInfo.hProcess;
					_addThread(de.dwThreadId, de.u.CreateProcessInfo);
					break;
				case EXIT_THREAD_DEBUG_EVENT:
					_removeThread(de.dwThreadId);
					break;
			    case EXIT_PROCESS_DEBUG_EVENT:
					bContinue = false;
					break;
			    case LOAD_DLL_DEBUG_EVENT:
					_addDll(de.u.LoadDll);
					break;
			    case UNLOAD_DLL_DEBUG_EVENT:
					_removeDll(de.u.LoadDll);
					break;
				case OUTPUT_DEBUG_STRING_EVENT:
					// ignore
					break;
				case RIP_EVENT:
					// ignore
					break;
				default:
					break;
				}
			}
			if (!ContinueDebugEvent(de.dwProcessId, de.dwThreadId, bFirstException ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE)) {
				DWORD err = GetLastError();
				fprintf(stderr, "Can't continue debug event(%d)\n", err);
				return 1;
			}

		}
		if (bTrigger) {
			walk(targetProcess);
			bTrigger = false;
		}
		if (bDumpTrigger) {
			_bDump = true;
			walk(targetProcess);
			_bDump = false;
			bDumpTrigger = false;
		}
	}

	if (_iPortNumber)
	{
		closesocket(_socket);
	}
	if (_iDumpPortNumber)
	{
		closesocket(_dumpSocket);
	}
	if (_iPortNumber || _iDumpPortNumber)
		WSACleanup();

	return 0;
}
