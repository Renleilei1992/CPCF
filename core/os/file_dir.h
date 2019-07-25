#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "../rt/string_type.h"
#include "../rt/buffer_type.h"
#include "kernel.h"
#include <stdio.h>
#include "predefines.h"

#ifdef PLATFORM_WIN
#include <shellapi.h>
#endif

namespace os
{

class MemoryFileRef: public rt::_File
{
protected:
	LPBYTE	m_pData;
	SIZE_T	m_CurPos;
	SIZE_T	m_Len;
	MemoryFileRef();

public:
	MemoryFileRef(LPVOID pData, SIZE_T Len){ SetMemoryBuffer(pData,Len); }
	void SetMemoryBuffer(LPVOID pData, SIZE_T Len);

	virtual SIZE_T		Read(LPVOID lpBuf,SIZE_T nCount);
	virtual SIZE_T		Write(LPCVOID lpBuf,SIZE_T nCount);
	virtual SIZE_T		Seek(SSIZE_T offset,UINT nFrom = rt::_File::Seek_Begin); // return ULLONG_MAX for failure.
	virtual SIZE_T		GetLength() const;
};


class MemoryFile: public MemoryFileRef
{
	inline void SetMemoryBuffer(LPVOID, UINT);  // this function is disabled
public:
	MemoryFile(SIZE_T Len=0);
	~MemoryFile();
	void SetBufferSize(SIZE_T Len);

	SIZE_T Read(LPVOID pBuf, SIZE_T sz){ return MemoryFileRef::Read(pBuf,sz); }
	SIZE_T Write(LPCVOID pBuf, SIZE_T sz){ return MemoryFileRef::Write(pBuf,sz); }
	SIZE_T Seek(SSIZE_T offset=0, DWORD where = SEEK_SET){ return MemoryFileRef::Seek(offset,where); } //return INFINITE if not seekable
	INLFUNC LPBYTE GetBuffer(){ return m_pData; }
	INLFUNC LPCBYTE GetBuffer() const { return m_pData; }
};

	
class File:public rt::_File
{
protected:
	FILE*		hFile;
	bool		bErrorFlag;
	rt::String	Filename;
	static bool	_GetFileStat(LPCSTR fn_utf8, struct _stat& stat);
	bool		_GetFileStat(struct _stat& stat) const;

public:
	static const LPCSTR		Normal_Read;
	static const LPCSTR		Normal_ReadText;
	static const LPCSTR		Normal_Write;
	static const LPCSTR		Normal_WriteText;
	static const LPCSTR		Normal_ReadWrite;
	static const LPCSTR		Normal_Append;		
	static const LPCSTR		Normal_AppendText;

	enum _tagCopyPathOption
	{	CPOPT_MIRROR		= 0x0001,	// remove destination items if source don't have them
		CPOPT_OVERWRITE		= 0x0002,	// overwrite all files, otherwise only modified files will be overwritten
		CPOPT_HIDDEN_FILE	= 0x0004,	// don't ignore hidden files
	};

public:
	FILE* GetFileHandle(){ return hFile; }
	File();
	~File();
	File(LPCSTR fn_utf8, LPCSTR mode = Normal_Read, bool create_path = false);
	bool		IsOpen() const { return hFile != NULL; }

	SIZE_T		Write(const rt::String_Ref& x){ return Write(x.Begin(), x.GetLength()); }
	SIZE_T		Write(const rt::String& x){ return Write(x.Begin(), x.GetLength()); }
	SIZE_T		Write(LPCSTR str){ return Write(str, strlen(str)); }
	template<typename t_POD>
	SIZE_T		WritePOD(const t_POD& x){ return Write((LPCVOID)&x,sizeof(t_POD)); }
	SIZE_T		Write(char x){ return Write(&x,1); }
	template<typename t_StrExprJson>
	SIZE_T		Write(const t_StrExprJson& x)
	{	SIZE_T  len = x.GetLength();
		LPSTR   buf = (LPSTR)alloca(len);
		VERIFY(len == x.CopyTo(buf));
		return Write(buf, len);
	}
	bool		IsEOF();
	template<typename t_POD>
	SIZE_T		ReadPOD(t_POD& x){ return Read((LPVOID)&x,sizeof(t_POD)); }
	SIZE_T		Read(LPVOID lpBuf,SIZE_T nCount);
	SIZE_T		Write(LPCVOID lpBuf,SIZE_T nCount);
	void		Close();
	bool		Open(LPCSTR fn_utf8, LPCSTR mode = Normal_Read, bool create_path = false);
	SIZE_T		GetLength() const;
	void		Truncate(SIZE_T len);
	SIZE_T		GetCurrentPosition() const;
	SIZE_T		Seek(SSIZE_T offset,UINT nFrom = Seek_Begin);
	void		SeekToBegin();
	void		SeekToEnd();
	void        Flush();
	int			GetFD() const { return fileno(hFile); }

	const rt::String& GetFilename() const { return Filename; }
	void	ClearError(){ bErrorFlag = false; }
	bool	ErrorOccured(){ return bErrorFlag; }

	//void	SetTime_Creation(__time64_t x);
	//void	SetTime_LastAccess(__time64_t x);
	//void	SetTime_LastModify(__time64_t x);

	static bool CreateDirectory(LPCSTR path);
	static bool CreateDirectories(LPCSTR path, bool path_is_file = true);	// create intermediate subdirectories if necessary, 
																		// if file_mode is true, last segment will be treated as file name. e.g. a/b/c/d/e, the folder a/b/c/d will be created, e is regard as the filename
	static bool IsDirectory(LPCSTR path);
	static bool IsFile(LPCSTR path);
	static bool IsExist(LPCSTR fn);
	static bool ProbeAvailableFilename(LPCSTR fn, rt::String& fn_out);	// true, confliction found, false no confliction, fn_out will not be set in this case
	static bool Remove(LPCSTR fn, bool secure = false);
	static bool RemoveDirectory(LPCSTR path);
	static bool Rename(LPCSTR fn,LPCSTR new_fn);
	static void GetCurrentDirectory(rt::String& out);
	static bool SetCurrentDirectory(LPCSTR path);
	static void ResolveRelativePath(LPCSTR path, rt::String& fn_out);
	static bool	GetPathTime(LPCSTR pathname, __time64_t* creation,__time64_t* last_access,__time64_t* last_modify);	// handles file and directory, feed NULL if not interested
	static bool	SetPathTime(LPCSTR pathname, __time64_t last_access, __time64_t last_modify);	// zero for not set
	static bool	MoveFile(LPCSTR from, LPCSTR to, bool overwrite = true);	// will try move if fail, try copy & delete

	//static	bool Copy(LPCSTR fn,LPCSTR new_fn,bool no_overwrite = false){ return ::CopyFile(fn,new_fn,no_overwrite); }
	//static	bool Move(LPCSTR fn,LPCSTR new_fn,bool no_overwrite = false){ return ::MoveFileEx(fn,new_fn,MOVEFILE_COPY_ALLOWED|(no_overwrite?0:MOVEFILE_REPLACE_EXISTING)); }
	//static	DWORD GetAttributes(LPCSTR fn){ return ::GetFileAttributes(fn); }
	//static	bool SetAttributes(LPCSTR fn,DWORD attrib){ return ::SetFileAttributes(fn,attrib); }

	__time64_t	GetTime_Creation() const { __time64_t x=0; return GetFileTime(&x, NULL, NULL)?x:0; }
	__time64_t	GetTime_LastAccess() const { __time64_t x=0; return GetFileTime(NULL, &x, NULL)?x:0; }
	__time64_t	GetTime_LastModify() const { __time64_t x=0; return GetFileTime(NULL, NULL, &x)?x:0; }
	bool		GetFileTime(__time64_t* creation,__time64_t* last_access,__time64_t* last_modify) const;
	bool		SetFileTime(__time64_t last_access, __time64_t last_modify) const; // zero for not set

	static ULONGLONG	GetFileSize(LPCSTR pathname);
	ULONGLONG			GetFileSize() const;

	static bool LoadText(LPCSTR fn, rt::String& out, UINT expire_sec = rt::TypeTraits<UINT>::MaxVal());
	static bool SaveText(LPCSTR fn, const rt::String_Ref& in, bool add_utf8_signature = true, bool append = false);
	static bool LoadBinary(LPCSTR fn, rt::String& out, UINT expire_sec = rt::TypeTraits<UINT>::MaxVal());
	static bool SaveBinary(LPCSTR fn, const rt::String_Ref& in);

	static bool CopyPath(const rt::String_Ref& dest, const rt::String_Ref& src, DWORD opt = CPOPT_MIRROR);
	static bool RemovePath(const rt::String_Ref& src);
};

extern ULONGLONG GetFreeDiskSpace(LPCSTR path, ULONGLONG* pTotal = NULL); // in MB
extern void		 GetAppSandbox(rt::String& out_path, LPCSTR app_name);	// app_name not necessarily appears in the path
extern void		 SetAppSandboxAsCurrentDirectory(LPCSTR app_name);

///////////////////////////////////////////
// FileRead

template<typename T = BYTE>
class FileRead
{
	rt::Buffer<T>	_buf;
public:
	SIZE_T GetSize() const { return _buf.GetSize(); }
	operator const T* () const { return _buf; }
	FileRead(LPCSTR fn, LPCSTR mode = os::File::Normal_Read)
	{	File	_file;
		int		co;
		if(	_file.Open(fn, mode) &&
			(co = (int)_file.GetLength() / sizeof(T)) &&
			_buf.SetSize(co) &&
			_file.Read(_buf, co*sizeof(T)) == co*sizeof(T)
		){}
		else
		{	_buf.SetSize(0);
		}
	}
};

class FileReadLine
{
	bool	GetNextLine(rt::String& line); // use rt::String_Ref instead
protected:
	static const int FRL_BUFSIZE = 256*1024;	// each line should smaller than half of this
	rt::BufferEx<char>	_buf;
	UINT				_bufused;
#if defined(PLATFORM_WIN)
	HANDLE				_hFile;
#else
	File				_file;
#endif
	ULONGLONG			_lastpos;
	rt::String			_filename;
public:
	FileReadLine();
	~FileReadLine(){ Close(); }
	const rt::String& GetFilename() const { return _filename; }
	ULONGLONG	GetSize() const;
	ULONGLONG	CurPos() const { return _lastpos; }
	bool		Open(LPCSTR fn);
	void		Close();
	template<typename T>
	bool		ReadBlock(const T*& obj){ return ReadBlock((LPCVOID&)obj, sizeof(T)); }
	bool		ReadBlock(LPCVOID& p, UINT size);
	bool		GetNextLine(rt::String_Ref& line){ return GetNextLineWithQuote(line, 0); }
	bool		GetNextLineWithQuote(rt::String_Ref& line, char quote = '"');
};

class FileWriteLine
{
	static const int FRL_BUFSIZE = 64*1024;	// each line should smaller than half of this
	rt::BufferEx<char>	_buf;
	UINT				_bufused;
#if defined(PLATFORM_WIN)
	HANDLE				_hFile;
#else
	File				_file;
#endif
public:
	FileWriteLine();
	~FileWriteLine(){ Close(); }
	ULONGLONG	GetSize() const;
	bool		Open(LPCSTR fn, bool append = true);
	void		Close();
	bool		WriteLine(const rt::String_Ref& line){ return Write(line.Begin(), (UINT)line.GetLength()) && Write("\r\n",2); }
	bool		WriteLine(LPCSTR line){ return WriteLine(rt::String_Ref(line)); }
	template<typename T>
	bool		WriteLine(const T& line)
	{	SIZE_T len = line.GetLength();
		LPSTR buf = (LPSTR)alloca(len);
		VERIFY(line.CopyTo(buf) == len);
		return WriteLine(rt::String_Ref(buf,len));
	}
	bool		Write(LPCVOID p, UINT size);
	bool		Flush();
	bool		WriteUTF8Sign();
	bool		WriteHeader(LPCVOID p, UINT size);
	template<typename T>
	bool		WriteBlock(const T& obj){ return Write(&obj, sizeof(T)); }
	template<typename T>
	bool		WriteHeader(const T& obj){ return WriteHeader(&obj, sizeof(T)); }
	bool		IsOpen() const;
};

class FileMapping
{
protected:
#if defined(PLATFORM_WIN)
	HANDLE				_hFile;
	HANDLE				_hFileMapping;
	rt::String			_Filename;
#else
	File				_File;
#endif

	bool				_Readonly;
	LPVOID				_Ptr;
	SIZE_T				_Size;
public:
	FileMapping();
	bool		Open(LPCSTR filename, SIZE_T length = 0, bool readonly = true, bool create_new = true);
	void		Close(bool also_delete_file = false);

	INLFUNC bool		IsOpen() const { return _Ptr != NULL; }
	INLFUNC LPVOID		GetBasePtr(SIZE_T offset = 0){ return (offset + (LPBYTE)_Ptr); }
	INLFUNC LPCVOID		GetBasePtr(SIZE_T offset = 0) const { return (offset + (LPBYTE)_Ptr); }
	INLFUNC SIZE_T		GetSize() const { return _Size; }
	INLFUNC bool		IsReadonly() const { return _Readonly; }

	~FileMapping(){ Close(); }
	const rt::String& GetFilename() const;
};

template<typename T>
struct FileBuffer: protected FileMapping
{
	INLFUNC FileBuffer(){}
	INLFUNC FileBuffer(LPCSTR filename, SIZE_T count = 0, bool readonly = true){ Open(filename, count, readonly); }
	INLFUNC LPVOID	GetBasePtr(SIZE_T offset = 0){ return (offset + (LPBYTE)_Ptr); }
	INLFUNC LPCVOID	GetBasePtr(SIZE_T offset = 0) const { return (offset + (LPBYTE)_Ptr); }
	INLFUNC bool	Open(LPCSTR filename, SIZE_T count = 0, bool readonly = true){ return FileMapping::Open(filename, count*sizeof(T), readonly); }
	INLFUNC void	Close(bool also_delete_file = false){ FileMapping::Close(also_delete_file); }
	INLFUNC bool	IsOpen() const { return _Ptr != NULL; }
	INLFUNC bool	IsReadonly() const { return _Readonly; }
	INLFUNC SIZE_T	GetSize() const { return _Size/sizeof(T); }

	INLFUNC const rt::String&	GetFilename() const { return FileMapping::GetFilename(); }
	INLFUNC T*				Begin(){ return (T*)GetBasePtr(); }
	INLFUNC const T*		Begin() const { return (T*)GetBasePtr(); }
	INLFUNC T*				End(){ return (T*)GetBasePtr(FileMapping::GetSize()); }
	INLFUNC const T*		End() const { return (T*)GetBasePtr(FileMapping::GetSize()); }
	INLFUNC					operator T*(){ return Begin(); }
	INLFUNC					operator const T*() const { return Begin(); }
};

namespace _details
{	
#pragma pack(1)
struct FilePackedEntry
{	ULONGLONG		FileName;	// named by a ULONGLONG
	UINT			Offset;
	UINT			Size;
};
#pragma pack()
static const DWORD	FilePackedMagic = 0x50444b43;
}

class FilePacked_Reader
{	ULONGLONG	_FileSearchName;
	ULONGLONG	_FileSearchMask;
	int			_FileSearchLast;
protected:
	os::File	_File;
	rt::BufferEx<_details::FilePackedEntry>	_Entries;
	UINT		_FindFile(ULONGLONG name);
public:
	bool		Open(LPCSTR fn);
	void		Close();
	UINT		FindFirstFile(ULONGLONG name, ULONGLONG mask = 0xffffffffffffffffLL);	// return idx, INFINITE if not found
	UINT		FindNextFile(); // return idx, INFINITE if not found
	UINT		GetFileSize(UINT idx);
	ULONGLONG	GetFileName(UINT idx);
	UINT		GetFileCount() const;
	bool		Read(UINT idx, LPVOID pout);
	bool		Load(ULONGLONG name, rt::String& out);
	template<typename T>
	bool		Load(ULONGLONG name, rt::Buffer<T>& out)
	{	int idx;
		return	(idx = _FindFile(name)) != INFINITE &&
				GetFileSize(idx) % sizeof(T) == 0 &&
				out.SetSize(GetFileSize(idx)/sizeof(T)) &&
				Read(idx, out.Begin());
	}
	template<typename T>
	bool		Load(ULONGLONG name, T* out)
	{	int idx;
		return	(idx = _FindFile(name)) != INFINITE &&
				sizeof(T) == GetFileSize(idx) &&
				Read(idx, out);
	}
};

class FilePacked_Writer
{	os::File	_File;
public:
	void Close(){ _File.Close(); }
	bool Open(LPCSTR fn); // Read only, or write only
	bool Write(ULONGLONG name, LPCVOID data, UINT size);
	bool Copy(FilePacked_Reader& reader, ULONGLONG filename_desired, ULONGLONG filename_mask);
	void Flush(){ _File.Flush(); }
};


class FileList
{
	struct _File
	{	rt::String		Name;
		bool			IsDirectory;
		_File(){ IsDirectory = false; }
	};
	rt::BufferEx<_File>		_Filenames;
	rt::String				_Root;
	rt::String				_TempString;
public:
	enum
	{	FLAG_ONLYFIRSTFILE	= 0x1,
		FLAG_NODIRECTORY	= 0x2,
		FLAG_DIRECTORYONLY	= 0x4,
		FLAG_SKIPHIDDEN		= 0x8,	// File with name begin with '.', or file with hidden attribute
		FLAG_RECURSIVE		= 0x10,
	};
protected:
	UINT	_Populate(const rt::String_Ref& directory, LPCSTR suffix_filter, DWORD flag);	// suffix_filter = ".bmp|.jpg|.png", up to 64 suffixies, each individul filter should short than 16 characters

public:
	void	AddFile(const rt::String_Ref& fn, bool IsDir = false);
	UINT	Populate(LPCSTR directory, LPCSTR suffix_filter = NULL, DWORD flag = FLAG_SKIPHIDDEN|FLAG_NODIRECTORY);	// suffix_filter = ".bmp|.jpg|.png", up to 64 suffixies, each individul filter should short than 16 characters

#ifdef PLATFORM_WIN
	UINT	PopulateDropList(HDROP hfile, LPCSTR suffix_filter = NULL, DWORD flag = FLAG_SKIPHIDDEN|FLAG_NODIRECTORY);
#endif

	UINT	GetCount() const;
	
	const rt::String& GetFilename(UINT idx) const;	// return the relative path including the filenname
	
	bool	IsDirectory(UINT idx) const;
	void	GetFullpath(UINT idx, rt::String& fn) const;
	//void	GetFilenameAndExt(UINT idx,rt::String& filename, rt::String& ext) const; // e.g. for relative path ".\a\pic.png", returned filename is "pic", ext is "png"
	const rt::String& GetFullpath(UINT idx){ GetFullpath(idx,_TempString); return _TempString; }
};

#if defined(PLATFORM_WIN) //||  defined(PLATFORM_MAC)

class FolderChangingMonitor
{
public:
enum _tagFolderChangedEvent
{
#if defined(PLATFORM_WIN)
	FCE_FILE_OVERWRITTEN = FILE_NOTIFY_CHANGE_LAST_WRITE,
	FCE_FILE_RENAMED = FILE_NOTIFY_CHANGE_FILE_NAME,
	FCE_DIRECTORY_RENAMED = FILE_NOTIFY_CHANGE_DIR_NAME,
	FCE_FILE_ACCESSED = FILE_NOTIFY_CHANGE_LAST_ACCESS,
	FCE_FILE_CREATED = FILE_NOTIFY_CHANGE_CREATION
#else
	ASSERT_STATIC(0);
#endif
};
protected:
	virtual void OnFolderChanged() = 0;
	int			_CoalescingInterval;	 // msec

#if defined(PLATFORM_WIN)
	static DWORD WINAPI _WorkingThreadFunc(LPVOID p);
	HANDLE	m_WaitingHandle;
	HANDLE	m_WorkingThread;
#else
	ASSERT_STATIC(0);
#endif

public:
	FolderChangingMonitor();
	~FolderChangingMonitor(){ Destroy(); }
	bool Create(LPCSTR FolderName, bool IncludeSubTree = false, DWORD Filter =	FCE_FILE_OVERWRITTEN );
	void Destroy();
	bool IsStarted();
};
#endif // #if defined(PLATFORM_WIN) ||  defined(PLATFORM_MAC)


///////////////////////////////////////////////////
// Cmdline parser ( option indicator is '/' '-' )
class CommandLine
{
	struct _opt
	{	rt::String	Name;
		rt::String	Value;
	};
protected:
	rt::String					_CommandLine;
	rt::BufferEx<rt::String>	_Arguments; // Text
	rt::BufferEx<_opt>			_Options;
	void						_Parse(int argc, char* argv[]);	// for _tmain

public:
	CommandLine();
	~CommandLine();

	void	Empty();

#if defined(PLATFORM_WIN)
	void	Parse(int argc, WCHAR* argv[]);	// for _tmain
	void	Parse(LPCWSTR pCmdLine);		// for _twmain
	explicit CommandLine(int argc, WCHAR* argv[]){ Parse(argc, argv); }
	explicit CommandLine(LPCWSTR pCmdLine){ Parse(pCmdLine); }
#endif

	void	ParseURI(const rt::String_Ref& path, const rt::String_Ref& query);
	void	Parse(int argc, char* argv[]);	// for _tmain
	void	Parse(LPCSTR pCmdLine);			// for _twmain
	explicit CommandLine(int argc, char* argv[]){ Parse(argc, argv); }
	explicit CommandLine(LPCSTR pCmdLine){ Parse(pCmdLine); }

	void	SubstituteOptions(rt::String& string, const rt::String_Ref& prefix = rt::SS("%"), const rt::String_Ref& suffix = rt::SS("%")) const;

	template<typename T>
	INLFUNC T GetOptionAs(const rt::String_Ref& opt_name, T default_val) const
	{	rt::String_Ref str = GetOption(opt_name);
		if(!str.IsEmpty())str.ToNumber(default_val);
		return default_val;
	}
	INLFUNC bool GetOptionAs(const rt::String_Ref& opt_name, bool default_val) const
	{	rt::String_Ref str = GetOption(opt_name);
		if(str.IsEmpty())return default_val;
		return str[0] == 't' || str[0] == '1' || str[0] == 'T';
	}
	template<typename T>
	INLFUNC T GetOptionAs(const rt::String_Ref& opt_name) const { return GetOptionAs<T>(opt_name, 0); }
	template<class VEC, int chan>
	INLFUNC VEC GetOptionAs(const rt::String_Ref& opt_name, const VEC& default_val = 0) const
	{	rt::String_Ref str = GetOption(opt_name);
		rt::String_Ref f[chan];
		VEC ret;
		UINT co;
		if(!str.IsEmpty() && (co = str.Split(f,chan,",x|:/*")))
		{	for(UINT i=0;i<chan;i++)
				f[rt::min(co,i)].ToNumber(ret[i]);
			return ret;
		}
		return default_val;
	}
	LPCSTR	SearchOptionEx(const rt::String_Ref& option_substring) const;	//search an option contains this name (in lower-case)	, if found return the remaining text of the option
	rt::String_Ref GetOption(const rt::String_Ref& option_name, const rt::String_Ref& def_val = NULL) const;
	bool	HasOption(const rt::String_Ref& option_name) const;

	UINT	GetTextCount()const{ return (UINT)_Arguments.GetSize(); }
	LPCSTR	GetText(UINT idx, LPCSTR default_val = NULL)const{ return _Arguments.GetSize()>idx?(LPCSTR)_Arguments[idx]:default_val; }

	LPCSTR	GetOriginalLine() const { return _CommandLine; }

	UINT	GetOptionCount()const{ return (UINT)_Options.GetSize(); }
	LPCSTR	GetOptionName(UINT idx)const{ return _Options[idx].Name; }
	LPCSTR	GetOptionValue(UINT idx)const{ return _Options[idx].Value; }
	void	SecureClear();

	void	LoadEnvironmentVariablesAsOptions();
	void	SetOptionDefault(const rt::String_Ref& opt_name, const rt::String_Ref& value);
	void	SetOption(const rt::String_Ref& opt_name, const rt::String_Ref& value);
	static const CommandLine& Get();
	static CommandLine& GetMutable();
};

//////////////////////////////////////////////
// Process Management
class Process
{
protected:
#if defined(PLATFORM_WIN)
	HANDLE			m_hProcess;
#else
	DWORD			m_PID;
#endif
	int				m_ExitCode;
	UINT			m_ExecutionTime;		// in msec
	os::Timestamp	m_ExitTime;				// available after call IsRunning and it returns false

public:
	Process();
	bool	Launch(LPCSTR cmdline, LPCSTR pWorkDirectory = NULL, LPVOID pEnvVariable = NULL);
	bool	WaitForEnding(DWORD timeout = INFINITE); // return false when timeout
	void	Terminate();
	bool	IsRunning();

	UINT	GetExecutionTime() const { return m_ExecutionTime; }		// available after IsRunning() returns false!	
	int		GetExitCode() const { return m_ExitCode; }					// available after IsRunning() returns false!	
	void	SetExitCode(int c){ m_ExitCode = c; }
	const os::Timestamp& GetExitTime() const { return m_ExitTime; }	// available after IsRunning() returns false!
    
public:
    struct Info
    {
        UINT            PID;
		UINT			ParentPID;
        rt::String      Name;
        os::Timestamp   StartTime;
    };
    static void		Populate(rt::Buffer<Info>& list_out);
	static bool		Search(Info& out, const rt::String_Ref& process_substr);
	static UINT		CurrentId();
	static UINT		CurrentParentId();
	static bool		IsRunning(UINT pid);
	static void		Terminate(UINT pid);
};

#if defined(PLATFORM_WIN)
namespace _details
{	
	typedef void (*FUNC_PostCrashDump)(LPCSTR fn);
	extern void _SetPostCrashDumpHook(os::_details::FUNC_PostCrashDump hook);
}

extern void EnableCrashDump(LPCSTR dump_filename, bool full_memory);
#endif

}

