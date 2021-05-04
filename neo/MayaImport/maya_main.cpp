#include "precompiled.h"
#pragma hdrstop

#include "../sys/sys_local.h"
#pragma hdrstop

idStr Maya_Error = "";

bool dllEntry(int version, idCommon* common, idSys* sys);

idCVar g_exportMask("g_exportMask", "", CVAR_INTEGER, "");

idSession* session = NULL;
idEventLoop* eventLoop = NULL;

const char* Maya_ConvertModel(const char* ospath, const char* commandline);

int idEventLoop::JournalLevel(void) const { return 0; }

#define STDIO_PRINT( pre, post )	\
	va_list argptr;					\
	va_start( argptr, fmt );		\
	printf( pre );					\
	vprintf( fmt, argptr );			\
	printf( post );					\
	va_end( argptr )

int Sys_Milliseconds(void) {
	return 0;
}

class idCommonLocal : public idCommon {
public:
	idCommonLocal(void) {}

	virtual void			Init(int argc, const char** argv, const char* cmdline) {}
	virtual void			Shutdown(void) {}
	virtual void			Quit(void) {}
	virtual bool			IsInitialized(void) const { return true; }
	virtual void			Frame(void) {}
	virtual void			GUIFrame(bool execCmd, bool network) {}
	virtual void			Async(void) {}
	virtual void			StartupVariable(const char* match, bool once) {}
	virtual void			InitTool(const toolFlag_t tool, const idDict* dict) {}
	virtual void			ActivateTool(bool active) {}
	virtual void			WriteConfigToFile(const char* filename) {}
	virtual void			WriteFlaggedCVarsToFile(const char* filename, int flags, const char* setCmd) {}
	virtual void			BeginRedirect(char* buffer, int buffersize, void (*flush)(const char*)) {}
	virtual void			EndRedirect(void) {}
	virtual void			SetRefreshOnPrint(bool set) {}
	virtual void			Printf(const char* fmt, ...) { STDIO_PRINT("", ""); }
	virtual void			VPrintf(const char* fmt, va_list arg) { vprintf(fmt, arg); }
	virtual void			DPrintf(const char* fmt, ...) { /*STDIO_PRINT( "", "" );*/ }
	virtual void			Warning(const char* fmt, ...) { STDIO_PRINT("WARNING: ", "\n"); }
	virtual void			DWarning(const char* fmt, ...) { /*STDIO_PRINT( "WARNING: ", "\n" );*/ }
	virtual void			PrintWarnings(void) {}
	virtual void			ClearWarnings(const char* reason) {}
	virtual void			Error(const char* fmt, ...) { STDIO_PRINT("ERROR: ", "\n"); exit(0); }
	virtual void			FatalError(const char* fmt, ...) { STDIO_PRINT("FATAL ERROR: ", "\n"); exit(0); }
	virtual const idLangDict* GetLanguageDict() { return NULL; }
	virtual const char* KeysFromBinding(const char* bind) { return NULL; }
	virtual const char* BindingFromKey(const char* key) { return NULL; }
	virtual int				ButtonState(int key) { return 0; }
	virtual int				KeyState(int key) { return 0; }
};


idCVar com_developer("developer", "0", CVAR_BOOL | CVAR_SYSTEM, "developer mode");

idCommonLocal		commonLocal;
idCommon* common = &commonLocal;

/*
==============================================================

	idSys

==============================================================
*/

void			Sys_Mkdir(const char* path) {}
ID_TIME_T			Sys_FileTimeStamp(FILE* fp) { return 0; }

#ifdef _WIN32

#include <io.h>
#include <direct.h>

const char* Sys_Cwd(void) {
	static char cwd[1024];

	_getcwd(cwd, sizeof(cwd) - 1);
	cwd[sizeof(cwd) - 1] = 0;

	int i = idStr::FindText(cwd, CD_BASEDIR, false);
	if (i >= 0) {
		cwd[i + strlen(CD_BASEDIR)] = '\0';
	}

	return cwd;
}

const char* Sys_DefaultCDPath(void) {
	return "";
}

const char* Sys_DefaultBasePath(void) {
	return Sys_Cwd();
}

const char* Sys_DefaultSavePath(void) {
	return cvarSystem->GetCVarString("fs_basepath");
}

const char* Sys_EXEPath(void) {
	return "";
}

int Sys_ListFiles(const char* directory, const char* extension, idStrList& list) {
	idStr		search;
	struct _finddata_t findinfo;
	int			findhandle;
	int			flag;

	if (!extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		flag = 0;
	}
	else {
		flag = _A_SUBDIR;
	}

	sprintf(search, "%s\\*%s", directory, extension);

	// search
	list.Clear();

	findhandle = _findfirst(search, &findinfo);
	if (findhandle == -1) {
		return -1;
	}

	do {
		if (flag ^ (findinfo.attrib & _A_SUBDIR)) {
			list.Append(findinfo.name);
		}
	} while (_findnext(findhandle, &findinfo) != -1);

	_findclose(findhandle);

	return list.Num();
}

#else

const char* Sys_DefaultCDPath(void) { return ""; }
const char* Sys_DefaultBasePath(void) { return ""; }
const char* Sys_DefaultSavePath(void) { return ""; }
int				Sys_ListFiles(const char* directory, const char* extension, idStrList& list) { return 0; }

#endif

xthreadInfo* g_threads[MAX_THREADS];
int				g_thread_count;

void			Sys_CreateThread(xthread_t function, void* parms, xthreadPriority priority, xthreadInfo& info, const char* name, xthreadInfo* threads[MAX_THREADS], int* thread_count) {}
void			Sys_DestroyThread(xthreadInfo& info) {}

void			Sys_EnterCriticalSection(int index) {}
void			Sys_LeaveCriticalSection(int index) {}

void			Sys_WaitForEvent(int index) {}
void			Sys_TriggerEvent(int index) {}

/*
==============
idSysLocal stub
==============
*/
void			idSysLocal::DebugPrintf(const char* fmt, ...) {}
void			idSysLocal::DebugVPrintf(const char* fmt, va_list arg) {}

double			idSysLocal::GetClockTicks(void) { return 0.0; }
double			idSysLocal::ClockTicksPerSecond(void) { return 1.0; }
cpuid_t			idSysLocal::GetProcessorId(void) { return (cpuid_t)0; }
const char* idSysLocal::GetProcessorString(void) { return ""; }
const char* idSysLocal::FPU_GetState(void) { return ""; }
bool			idSysLocal::FPU_StackIsEmpty(void) { return true; }
void			idSysLocal::FPU_SetFTZ(bool enable) {}
void			idSysLocal::FPU_SetDAZ(bool enable) {}

bool			idSysLocal::LockMemory(void* ptr, int bytes) { return false; }
bool			idSysLocal::UnlockMemory(void* ptr, int bytes) { return false; }

void			idSysLocal::GetCallStack(address_t* callStack, const int callStackSize) { memset(callStack, 0, callStackSize * sizeof(callStack[0])); }
const char* idSysLocal::GetCallStackStr(const address_t* callStack, const int callStackSize) { return ""; }
const char* idSysLocal::GetCallStackCurStr(int depth) { return ""; }
void			idSysLocal::ShutdownSymbols(void) {}

int				idSysLocal::DLL_Load(const char* dllName) { return 0; }
void* idSysLocal::DLL_GetProcAddress(int dllHandle, const char* procName) { return NULL; }
void			idSysLocal::DLL_Unload(int dllHandle) { }
void			idSysLocal::DLL_GetFileName(const char* baseName, char* dllName, int maxLength) { }

sysEvent_t		idSysLocal::GenerateMouseButtonEvent(int button, bool down) { sysEvent_t ev; memset(&ev, 0, sizeof(ev)); return ev; }
sysEvent_t		idSysLocal::GenerateMouseMoveEvent(int deltax, int deltay) { sysEvent_t ev; memset(&ev, 0, sizeof(ev)); return ev; }

void			idSysLocal::OpenURL(const char* url, bool quit) { }
void			idSysLocal::StartProcess(const char* exeName, bool quit) { }

void			idSysLocal::FPU_EnableExceptions(int exceptions) { }

idSysLocal		sysLocal;
idSys* sys = &sysLocal;

/*
==============================================================================================

	idModelExport

==============================================================================================
*/

class idModelExport {
private:
	void					Reset(void);
	bool					ParseOptions(idLexer& lex);
	int						ParseExportSection(idParser& parser);

	static bool				CheckMayaInstall(void);
	static void				LoadMayaDll(void);

	bool					ConvertMayaToMD5(void);
	static bool				initialized;

public:
	idStr					commandLine;
	idStr					src;
	idStr					dest;
	bool					force;

	idModelExport();

	static void				Shutdown(void);

	int						ExportDefFile(const char* filename);
	bool					ExportModel(const char* model);
	bool					ExportAnim(const char* anim);
	int						ExportModels(const char* pathname, const char* extension);
};


bool idModelExport::initialized = false;


/*
====================
idModelExport::idModelExport
====================
*/
idModelExport::idModelExport() {
	Reset();
}

/*
====================
idModelExport::Shutdown
====================
*/
void idModelExport::Shutdown(void) {
	
	Maya_Error.Clear();
	initialized = false;
}

/*
=====================
idModelExport::CheckMayaInstall

Determines if Maya is installed on the user's machine
=====================
*/
bool idModelExport::CheckMayaInstall(void) {
	return true;
}

/*
=====================
idModelExport::LoadMayaDll

Checks to see if we can load the Maya export dll
=====================
*/
void idModelExport::LoadMayaDll(void) {
	
}

/*
=====================
idModelExport::ConvertMayaToMD5

Checks if a Maya model should be converted to an MD5, and converts if if the time/date or
version number has changed.
=====================
*/
bool idModelExport::ConvertMayaToMD5(void) {
	ID_TIME_T		sourceTime;
	ID_TIME_T		destTime;
	int			version;
	idToken		cmdLine;
	idStr		path;

	// check if our DLL got loaded
	if (initialized) {
		Maya_Error = "MayaImport dll not loaded.";
		return false;
	}

	// if idAnimManager::forceExport is set then we always reexport Maya models
	//if (idAnimManager::forceExport) {
	//	force = true;
	//}

	// get the source file's time
	if (fileSystem->ReadFile(src, NULL, &sourceTime) < 0) {
		// source file doesn't exist
		return true;
	}

	// get the destination file's time
	if (!force && (fileSystem->ReadFile(dest, NULL, &destTime) >= 0)) {
		idParser parser(LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS);

		parser.LoadFile(dest);

		// read the file version
		if (parser.CheckTokenString(MD5_VERSION_STRING)) {
			version = parser.ParseInt();

			// check the command line
			if (parser.CheckTokenString("commandline")) {
				parser.ReadToken(&cmdLine);

				// check the file time, scale, and version
				if ((destTime >= sourceTime) && (version == MD5_VERSION) && (cmdLine == commandLine)) {
					// don't convert it
					return true;
				}
			}
		}
	}

	// if this is the first time we've been run, check if Maya is installed and load our DLL
	if (!initialized) {
		initialized = true;

		if (!CheckMayaInstall()) {
			Maya_Error = "Maya not installed in registry.";
			return false;
		}

		LoadMayaDll();

	}

	// we need to make sure we have a full path, so convert the filename to an OS path
	// _D3XP :: we work out of the cdpath, at least until we get Alienbrain
	src = fileSystem->RelativePathToOSPath(src, "fs_cdpath");
	dest = fileSystem->RelativePathToOSPath(dest, "fs_cdpath");

	dest.ExtractFilePath(path);
	if (path.Length()) {
		fileSystem->CreateOSPath(path);
	}

	// get the os path in case it needs to create one
	path = fileSystem->RelativePathToOSPath("", "fs_cdpath" /* _D3XP */);

	common->SetRefreshOnPrint(true);
	Maya_Error = Maya_ConvertModel(path, commandLine);
	common->SetRefreshOnPrint(false);
	if (Maya_Error != "Ok") {
		return false;
	}

	// conversion succeded
	return true;
}

/*
====================
idModelExport::Reset
====================
*/
void idModelExport::Reset(void) {
	force = false;
	commandLine = "";
	src = "";
	dest = "";
}

/*
====================
idModelExport::ExportModel
====================
*/
bool idModelExport::ExportModel(const char* model) {
	const char* game = cvarSystem->GetCVarString("fs_game");
	if (strlen(game) == 0) {
		game = BASE_GAMEDIR;
	}

	Reset();
	src = model;
	dest = model;
	dest.SetFileExtension(MD5_MESH_EXT);

	sprintf(commandLine, "mesh %s -dest %s -game %s", src.c_str(), dest.c_str(), game);
	if (!ConvertMayaToMD5()) {
		common->Printf("Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str());
		return false;
	}

	return true;
}

/*
====================
idModelExport::ExportAnim
====================
*/
bool idModelExport::ExportAnim(const char* anim) {
	const char* game = cvarSystem->GetCVarString("fs_game");
	if (strlen(game) == 0) {
		game = BASE_GAMEDIR;
	}

	Reset();
	src = anim;
	dest = anim;
	dest.SetFileExtension(MD5_ANIM_EXT);

	sprintf(commandLine, "anim %s -dest %s -game %s", src.c_str(), dest.c_str(), game);
	if (!ConvertMayaToMD5()) {
		common->Printf("Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str());
		return false;
	}

	return true;
}

/*
====================
idModelExport::ParseOptions
====================
*/
bool idModelExport::ParseOptions(idLexer& lex) {
	idToken	token;
	idStr	destdir;
	idStr	sourcedir;

	if (!lex.ReadToken(&token)) {
		lex.Error("Expected filename");
		return false;
	}

	src = token;
	dest = token;

	while (lex.ReadToken(&token)) {
		if (token == "-") {
			if (!lex.ReadToken(&token)) {
				lex.Error("Expecting option");
				return false;
			}
			if (token == "sourcedir") {
				if (!lex.ReadToken(&token)) {
					lex.Error("Missing pathname after -sourcedir");
					return false;
				}
				sourcedir = token;
			}
			else if (token == "destdir") {
				if (!lex.ReadToken(&token)) {
					lex.Error("Missing pathname after -destdir");
					return false;
				}
				destdir = token;
			}
			else if (token == "dest") {
				if (!lex.ReadToken(&token)) {
					lex.Error("Missing filename after -dest");
					return false;
				}
				dest = token;
			}
			else {
				commandLine += va(" -%s", token.c_str());
			}
		}
		else {
			commandLine += va(" %s", token.c_str());
		}
	}

	if (sourcedir.Length()) {
		src.StripPath();
		sourcedir.BackSlashesToSlashes();
		sprintf(src, "%s/%s", sourcedir.c_str(), src.c_str());
	}

	if (destdir.Length()) {
		dest.StripPath();
		destdir.BackSlashesToSlashes();
		sprintf(dest, "%s/%s", destdir.c_str(), dest.c_str());
	}

	return true;
}

/*
====================
idModelExport::ParseExportSection
====================
*/
int idModelExport::ParseExportSection(idParser& parser) {
	idToken	command;
	idToken	token;
	idStr	defaultCommands;
	idLexer lex;
	idStr	temp;
	idStr	parms;
	int		count;

	const char* game = cvarSystem->GetCVarString("fs_game");

	if (strlen(game) == 0) {
		game = BASE_GAMEDIR;
	}

	// only export sections that match our export mask
	if (g_exportMask.GetString()[0]) {
		if (parser.CheckTokenString("{")) {
			parser.SkipBracedSection(false);
			return 0;
		}

		parser.ReadToken(&token);
		if (token.Icmp(g_exportMask.GetString())) {
			parser.SkipBracedSection();
			return 0;
		}
		parser.ExpectTokenString("{");
	}
	else if (!parser.CheckTokenString("{")) {
		// skip the export mask
		parser.ReadToken(&token);
		parser.ExpectTokenString("{");
	}

	count = 0;

	lex.SetFlags(LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT);

	while (1) {

		if (!parser.ReadToken(&command)) {
			parser.Error("Unexpoected end-of-file");
			break;
		}

		if (command == "}") {
			break;
		}

		if (command == "options") {
			parser.ParseRestOfLine(defaultCommands);
		}
		else if (command == "addoptions") {
			parser.ParseRestOfLine(temp);
			defaultCommands += " ";
			defaultCommands += temp;
		}
		else if ((command == "mesh") || (command == "anim") || (command == "camera")) {
			if (!parser.ReadToken(&token)) {
				parser.Error("Expected filename");
			}

			temp = token;
			parser.ParseRestOfLine(parms);

			if (defaultCommands.Length()) {
				sprintf(temp, "%s %s", temp.c_str(), defaultCommands.c_str());
			}

			if (parms.Length()) {
				sprintf(temp, "%s %s", temp.c_str(), parms.c_str());
			}

			lex.LoadMemory(temp, temp.Length(), parser.GetFileName());

			Reset();
			if (ParseOptions(lex)) {
				const char* game = cvarSystem->GetCVarString("fs_game");
				if (strlen(game) == 0) {
					game = BASE_GAMEDIR;
				}

				if (command == "mesh") {
					dest.SetFileExtension(MD5_MESH_EXT);
				}
				else if (command == "anim") {
					dest.SetFileExtension(MD5_ANIM_EXT);
				}
				else if (command == "camera") {
					dest.SetFileExtension(MD5_CAMERA_EXT);
				}
				else {
					dest.SetFileExtension(command);
				}
				idStr back = commandLine;
				sprintf(commandLine, "%s %s -dest %s -game %s%s", command.c_str(), src.c_str(), dest.c_str(), game, commandLine.c_str());
				if (ConvertMayaToMD5()) {
					count++;
				}
				else {
					parser.Warning("Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str());
				}
			}
			lex.FreeSource();
		}
		else {
			parser.Error("Unknown token: %s", command.c_str());
			parser.SkipBracedSection(false);
			break;
		}
	}

	return count;
}

/*
================
idModelExport::ExportDefFile
================
*/
int idModelExport::ExportDefFile(const char* filename) {
	idParser	parser(LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT);
	idToken		token;
	int			count;

	count = 0;

	if (!parser.LoadFile(filename)) {
		common->Printf("Could not load '%s'\n", filename);
		return 0;
	}

	while (parser.ReadToken(&token)) {
		if (token == "export") {
			count += ParseExportSection(parser);
		}
		else {
			parser.ReadToken(&token);
			parser.SkipBracedSection();
		}
	}

	return count;
}

/*
================
idModelExport::ExportModels
================
*/
int idModelExport::ExportModels(const char* pathname, const char* extension) {
	int	count;

	count = 0;

	idFileList* files;
	int			i;

	if (!CheckMayaInstall()) {
		// if Maya isn't installed, don't bother checking if we have anims to export
		return 0;
	}

	common->Printf("--------- Exporting models --------\n");
	if (!g_exportMask.GetString()[0]) {
		common->Printf("  Export mask: '%s'\n", g_exportMask.GetString());
	}

	count = 0;

	files = fileSystem->ListFiles(pathname, extension);
	for (i = 0; i < files->GetNumFiles(); i++) {
		count += ExportDefFile(va("%s/%s", pathname, files->GetFile(i)));
	}
	fileSystem->FreeFileList(files);

	common->Printf("...%d models exported.\n", count);
	common->Printf("-----------------------------------\n");

	return count;
}


int main(int argc, char** argv) {
	static idCommonLocal commonLocal;

	dllEntry(MD5_VERSION, &commonLocal, NULL);
	cmdSystem->Init();
	cvarSystem->Init();
	idCVar::RegisterStaticVars();
	fileSystem->Init();
	declManager->Init();

	idModelExport	exporter;
	idStr			name;

	name = argv[1];
	name = "def/" + name;
	name.DefaultFileExtension(".def");
	exporter.ExportDefFile(name);

	return 0;
}