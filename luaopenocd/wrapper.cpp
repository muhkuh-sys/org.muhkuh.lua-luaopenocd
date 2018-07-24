#include "wrapper.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "shared_library.h"

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#       define OPENOCD_SHARED_LIBRARY_FILENAME "openocd.dll"
#       include <windows.h>

#elif defined(__GNUC__)
#       define OPENOCD_SHARED_LIBRARY_FILENAME "openocd.so"
#       include <dlfcn.h>
#       include <unistd.h>
#endif

/* NOTE: this must end with a slash. */
#define OPENOCD_SUBFOLDER "openocd/"



luaopenocd::luaopenocd(void)
 : m_tState(STATE_Uninitialized)
 , m_pcPluginPath(NULL)
 , m_pcOpenocdDataPath(NULL)
 , m_pcOpenocdSharedObjectPath(NULL)
{
	memset(&m_tDevice, 0, sizeof(ROMLOADER_JTAG_DEVICE_T));

	get_plugin_path();
	get_openocd_path();
}



luaopenocd::~luaopenocd(void)
{
	if( m_tState==STATE_Open )
	{
		openocd_close();
	}

	if( m_pcPluginPath!=NULL )
	{
		free(m_pcPluginPath);
		m_pcPluginPath = NULL;
	}
	if( m_pcOpenocdDataPath!=NULL )
	{
		free(m_pcOpenocdDataPath);
		m_pcOpenocdDataPath = NULL;
	}
	if( m_pcOpenocdSharedObjectPath!=NULL )
	{
		free(m_pcOpenocdSharedObjectPath);
		m_pcOpenocdSharedObjectPath = NULL;
	}
}



void luaopenocd::get_plugin_path(void)
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
	char *pcPath;
	DWORD dwFlags;
	BOOL fResult;
	LPCTSTR pfnMember;
	HMODULE hModule;
	DWORD dwBufferSize;
	DWORD dwResult;
	char *pcSlash;
	size_t sizPath;


	pcPath = NULL;

	/* Get the module by an address, but do not increase the refcount. */
	dwFlags =   GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
	          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	/* Use this function to identify the module. */
	pfnMember = (LPCTSTR)(luaopenocd::atOpenOcdResolve);
	fResult = GetModuleHandleEx(dwFlags, pfnMember, &hModule);
	if( fResult==0 )
	{
		fprintf(stderr, "Failed to get the module handle: %d\n", GetLastError());
	}
	else
	{
		dwBufferSize = 0x00010000;
		pcPath = (char*)malloc(dwBufferSize);
		if( pcPath==NULL )
		{
			fprintf(stderr, "Failed to allocate %d bytes for the path buffer.\n", dwBufferSize);
		}
		else
		{
			dwResult = GetModuleFileName(hModule, pcPath, dwBufferSize);
			/* NOTE: dwResult contains the length of the string without the terminating 0.
			 *       If the buffer is too small, the function returns the provided size of
			 *       the buffer, in this case "dwBufferSize".
			 *       Therefore the function failed if the result is dwBufferSize.
			 */
			if( dwResult>0 && dwResult<dwBufferSize )
			{
				fprintf(stderr, "Module path: '%s'\n", pcPath);

				/* Find the last backslash in the path. */
				pcSlash = strrchr(pcPath, '\\');
				if( pcSlash==NULL )
				{
					fprintf(stderr, "Failed to find the end of the path!\n");
					free(pcPath);
					pcPath = NULL;
				}
				else
				{
					/* Terminate the string after the last slash. */
					pcSlash[1] = 0;

					/* Allocate the new buffer. */
					sizPath = strlen(pcPath);
					m_pcPluginPath = (char*)malloc(sizPath + 1);
					if( m_pcPluginPath==NULL )
					{
						fprintf(stderr, "Failed to allocate a buffer for the plugin path.\n");
					}
					else
					{
						memcpy(m_pcPluginPath, pcPath, sizPath + 1);
					}

					free(pcPath);
					pcPath = NULL;
				}
			}
			else
			{
				fprintf(stderr, "Failed to get the module file name: %d\n", dwResult);
				free(pcPath);
				pcPath = NULL;
			}
		}
	}
#elif defined(__GNUC__)
	Dl_info tDlInfo;
	int iResult;
	size_t sizPath;
	const char *pcSlash;
	size_t sizCwdBufferSize;
	size_t sizCwd;
	char *pcCwd;
	char *pcGetCwdResult;
	int iCwdAddSlash;


	iResult = dladdr(luaopenocd::atOpenOcdResolve, &tDlInfo);
	if( iResult==0 )
	{
		fprintf(stderr, "Failed to get information about the shared object.\n");
	}
	else
	{
		if( tDlInfo.dli_fname!=NULL )
		{
			fprintf(stderr, "Path to the shared object: '%s'\n", tDlInfo.dli_fname);

			/* Is this an absolute path? */
			iResult = -1;
			if( tDlInfo.dli_fname[0]=='/' )
			{
				/* Yes -> no need to prepend the current working directory. */
				sizCwd = 0;
				pcCwd = NULL;
				iCwdAddSlash = 0;
				iResult = 0;
			}
			else
			{
				/* No, prepend the current working folder. */
				sizCwdBufferSize = 65536;
				pcCwd = (char*)malloc(sizCwdBufferSize);
				if( pcCwd==NULL )
				{
					fprintf(stderr, "Failed to allocate a buffer for the current working folder.\n");
				}
				else
				{
					pcGetCwdResult = getcwd(pcCwd, sizCwdBufferSize);
					if( pcGetCwdResult==NULL )
					{
						fprintf(stderr, "Failed to get the current working folder.\n");
					}
					else
					{
						sizCwd = strlen(pcCwd);
						iCwdAddSlash = 0;
						if( sizCwd>0 && pcCwd[sizCwd-1]!='/' )
						{
							iCwdAddSlash = 1;
						}
						iResult = 0;
					}
				}
			}

			if( iResult==0 )
			{
				/* Find the last backslash in the path. */
				pcSlash = strrchr(tDlInfo.dli_fname, '/');
				if( pcSlash==NULL )
				{
					fprintf(stderr, "Failed to find the end of the path!\n");
					if( pcCwd!=NULL )
					{
						free(pcCwd);
						pcCwd = NULL;
						sizCwd = 0;
					}
				}
				else
				{
					sizPath = (size_t)(pcSlash - tDlInfo.dli_fname) + 1;
					m_pcPluginPath = (char*)malloc(sizCwd + iCwdAddSlash + sizPath + 1);
					if( m_pcPluginPath==NULL )
					{
						fprintf(stderr, "Failed to allocate a buffer for the path.\n");
						if( pcCwd!=NULL )
						{
							free(pcCwd);
							pcCwd = NULL;
							sizCwd = 0;
						}
					}
					else
					{
						if( pcCwd!=NULL && sizCwd!=0 )
						{
							memcpy(m_pcPluginPath, pcCwd, sizCwd);
						}
						if( iCwdAddSlash!=0 )
						{
							m_pcPluginPath[sizCwd] = '/';
						}
						memcpy(m_pcPluginPath + sizCwd + iCwdAddSlash, tDlInfo.dli_fname, sizPath);
						m_pcPluginPath[sizCwd + iCwdAddSlash + sizPath] = 0;
					}
				}
			}
		}
	}
#endif
}



void luaopenocd::get_openocd_path(void)
{
	size_t sizOpenOcdSo;
	size_t sizPluginPath;
	size_t sizOpenOcdSubfolder;


	sizOpenOcdSo = sizeof(OPENOCD_SHARED_LIBRARY_FILENAME);

	if( m_pcPluginPath==NULL )
	{
		m_pcOpenocdDataPath = (char*)malloc(2);
		if( m_pcOpenocdDataPath!=NULL )
		{
			m_pcOpenocdDataPath[0] = '.';
			m_pcOpenocdDataPath[1] = 0;

			m_pcOpenocdSharedObjectPath = (char*)malloc(sizOpenOcdSo + 1);
			if( m_pcOpenocdSharedObjectPath==NULL )
			{
				free(m_pcOpenocdDataPath);
				m_pcOpenocdDataPath = NULL;
			}
			else
			{
				/* Initialize the path with the name of the shared object only. */
				memcpy(m_pcOpenocdSharedObjectPath, OPENOCD_SHARED_LIBRARY_FILENAME, sizOpenOcdSo);
				/* Terminate the name with a 0. */
				m_pcOpenocdSharedObjectPath[sizOpenOcdSo] = 0;
			}
		}
	}
	else
	{
		sizPluginPath = strlen(m_pcPluginPath);
		sizOpenOcdSubfolder = strlen(OPENOCD_SUBFOLDER);
		m_pcOpenocdDataPath = (char*)malloc(sizPluginPath + sizOpenOcdSubfolder + 1);
		if( m_pcOpenocdDataPath!=NULL )
		{
			/* Copy the path to this module to the start of the OpenOCD path. */
			memcpy(m_pcOpenocdDataPath, m_pcPluginPath, sizPluginPath);
			/* Append the subfolder. */
			memcpy(m_pcOpenocdDataPath + sizPluginPath, OPENOCD_SUBFOLDER, sizOpenOcdSubfolder);
			/* Terminate the path. */
			m_pcOpenocdDataPath[sizPluginPath + sizOpenOcdSubfolder] = 0;

			m_pcOpenocdSharedObjectPath = (char*)malloc(sizPluginPath + sizOpenOcdSubfolder + sizOpenOcdSo + 1);
			if( m_pcOpenocdSharedObjectPath==NULL )
			{
				free(m_pcOpenocdDataPath);
				m_pcOpenocdDataPath = NULL;
			}
			else
			{
				/* Copy the path to this module to the start of the OpenOCD path. */
				memcpy(m_pcOpenocdSharedObjectPath, m_pcPluginPath, sizPluginPath);
				/* Append the subfolder. */
				memcpy(m_pcOpenocdSharedObjectPath + sizPluginPath, OPENOCD_SUBFOLDER, sizOpenOcdSubfolder);
				/* Append the name of the OpenOCD shared object. */
				memcpy(m_pcOpenocdSharedObjectPath + sizPluginPath + sizOpenOcdSubfolder, OPENOCD_SHARED_LIBRARY_FILENAME, sizOpenOcdSo);
				/* Terminate the name with a 0. */
				m_pcOpenocdSharedObjectPath[sizPluginPath + sizOpenOcdSubfolder + sizOpenOcdSo] = 0;
			}
		}
	}

	if( m_pcOpenocdDataPath!=NULL && m_pcOpenocdSharedObjectPath!=NULL )
	{
		fprintf(stderr, "The path to the OpenOCD data files is:    '%s'\n", m_pcOpenocdDataPath);
		fprintf(stderr, "The path to the OpenOCD shared object is: '%s'\n", m_pcOpenocdSharedObjectPath);

		m_tState = STATE_Initialized;
	}
	else
	{
		fprintf(stderr, "Failed to get the path to the OpenOCD shared object.\n");
		m_tState = STATE_Error;
	}
}



const luaopenocd::OPENOCD_NAME_RESOLVE_T luaopenocd::atOpenOcdResolve[5] =
{
	{
		"muhkuh_openocd_init",
		offsetof(luaopenocd::MUHKUH_OPENOCD_FUNCTION_POINTERS_T, pfnInit) / sizeof(void*)
	},
	{
		"muhkuh_openocd_get_result",
		offsetof(luaopenocd::MUHKUH_OPENOCD_FUNCTION_POINTERS_T, pfnGetResult) / sizeof(void*)
	},
	{
		"muhkuh_openocd_get_result_alloc",
		offsetof(luaopenocd::MUHKUH_OPENOCD_FUNCTION_POINTERS_T, pfnGetResultAlloc) / sizeof(void*)
	},
	{
		"muhkuh_openocd_command_run_line",
		offsetof(luaopenocd::MUHKUH_OPENOCD_FUNCTION_POINTERS_T, pfnCommandRunLine) / sizeof(void*)
	},
	{
		"muhkuh_openocd_uninit",
		offsetof(luaopenocd::MUHKUH_OPENOCD_FUNCTION_POINTERS_T, pfnUninit) / sizeof(void*)
	}
};



/*
   Try to open the shared library.
   If successful, resolve method names and initialize the shared library.
 */
int luaopenocd::openocd_open(void)
{
	int iResult;
	void *pvSharedLibraryHandle;
	const OPENOCD_NAME_RESOLVE_T *ptCnt;
	const OPENOCD_NAME_RESOLVE_T *ptEnd;
	void *pvFn;
	void *pvOpenocdContext;
	char acError[1024];


	/* Be pessimistic. */
	iResult = -1;

	switch( m_tState )
	{
	case STATE_Uninitialized:
		printf("Failed to open: not initialized. Was openocd found?\n");
		break;

	case STATE_Initialized:
		iResult = 0;
		break;

	case STATE_Open:
		printf("Failed to open: already open.\n");
		break;

	case STATE_Error:
		printf("Failed to open: in error state. Was openocd found?\n");
		break;
	}

	if( iResult==0 )
	{
		/* Try to open the shared library. */
		pvSharedLibraryHandle = sharedlib_open(m_pcOpenocdSharedObjectPath);
		if( pvSharedLibraryHandle==NULL )
		{
			/* Failed to open the shared library. */
			sharedlib_get_error(acError, sizeof(acError));
			fprintf(stderr, "Failed to open the shared library %s.\n", m_pcOpenocdSharedObjectPath);
			fprintf(stderr, "Error: %s\n", acError);
			iResult = -1;
		}
		else
		{
			m_tDevice.pvSharedLibraryHandle = pvSharedLibraryHandle;

			/* Try to resolve all symbols. */
			ptCnt = atOpenOcdResolve;
			ptEnd = atOpenOcdResolve + (sizeof(atOpenOcdResolve)/sizeof(atOpenOcdResolve[0]));
			while( ptCnt<ptEnd )
			{
				pvFn = sharedlib_resolve_symbol(pvSharedLibraryHandle, ptCnt->pstrSymbolName);
				if( pvFn==NULL )
				{
					iResult = -1;
					break;
				}
				else
				{
					m_tDevice.tFunctions.pv[ptCnt->sizPointerOffset] = pvFn;
					++ptCnt;
				}
			}

			if( iResult==0 )
			{
				/* Call the init function and pass the data path as a search path for scripts. */
				pvOpenocdContext = m_tDevice.tFunctions.tFn.pfnInit(m_pcOpenocdDataPath);
				if( pvOpenocdContext==NULL )
				{
					fprintf(stderr, "Failed to initialize the OpenOCD device context.\n");
					iResult = -1;
				}
				else
				{
					m_tDevice.pvOpenocdContext = pvOpenocdContext;
					m_tState = STATE_Open;
				}
			}

			if( iResult!=0 )
			{
				/* Close the shared library. */
				sharedlib_close(pvSharedLibraryHandle);
				m_tDevice.pvSharedLibraryHandle = NULL;
			}
		}
	}

	return iResult;
}


/* Uninitialize and cose the shared library. */
void luaopenocd::openocd_close(void)
{
	int iResult;


	/* Be pessimistic. */
	iResult = -1;

	switch(m_tState)
	{
	case STATE_Uninitialized:
		printf("Failed to close: uninitialized.\n");
		break;

	case STATE_Initialized:
		printf("Not open, no need to close.\n");
		iResult = 0;
		break;

	case STATE_Open:
		iResult = 0;
		break;

	case STATE_Error:
		printf("Failed to open: in error state.\n");
		break;
	}

	if( iResult==0 && m_tState==STATE_Open )
	{
		if( m_tDevice.pvSharedLibraryHandle!=NULL )
		{
			if( m_tDevice.pvOpenocdContext!=NULL )
			{
				m_tDevice.tFunctions.tFn.pfnUninit(m_tDevice.pvOpenocdContext);
				m_tDevice.pvOpenocdContext=NULL;
			}

			/* Close the shared library. */
			sharedlib_close(m_tDevice.pvSharedLibraryHandle);
			m_tDevice.pvSharedLibraryHandle=NULL;
		}
	}
}



/* The initialize function opens the OpenOCD shared library,
   executes the "version" command and closes the library.

   This is a test if the shared library can be used.
*/
int luaopenocd::initialize(void)
{
	int iResult;
	char acResult[256];


	/* Be pessimistic. */
	iResult = -1;

	if( m_tState==STATE_Initialized )
	{
		/* Initialize the device handle. */
		memset(&m_tDevice, 0, sizeof(ROMLOADER_JTAG_DEVICE_T));

		iResult = openocd_open();
		if( iResult==0 )
		{
			/* Run the version command. */
			iResult = m_tDevice.tFunctions.tFn.pfnCommandRunLine(m_tDevice.pvOpenocdContext, "version\n");
			if( iResult!=0 )
			{
				fprintf(stderr, "Failed to run the version command!\n");
			}
			else
			{
				iResult = m_tDevice.tFunctions.tFn.pfnGetResult(m_tDevice.pvOpenocdContext, acResult, sizeof(acResult));
				if( iResult!=0 )
				{
					fprintf(stderr, "Failed to get the result for the version command.\n");
				}
				else
				{
					fprintf(stderr, "OpenOCD version %s\n", acResult);
				}
			}

			if( iResult!=0 )
			{
				openocd_close();
			}
		}
	}

	return iResult;
}



int luaopenocd::run(char *strLine)
{
	int iResult;


	/* Be pessimistic. */
	iResult = -1;
	if( m_tState==STATE_Open )
	{
		iResult = m_tDevice.tFunctions.tFn.pfnCommandRunLine(m_tDevice.pvOpenocdContext, strLine);
		if( iResult!=0 )
		{
			fprintf(stderr, "Failed to run the command!\n");
		}
	}

	return iResult;
}



void luaopenocd::get_result(char **ppcBUFFER_OUT, size_t *psizBUFFER_OUT)
{
	int iResult;
	char *pcBuffer;
	size_t sizBuffer;


	/* Be pessimistic. */
	iResult = -1;
	pcBuffer = NULL;
	sizBuffer = 0;

	if( m_tState==STATE_Open )
	{
		iResult = m_tDevice.tFunctions.tFn.pfnGetResultAlloc(m_tDevice.pvOpenocdContext, &pcBuffer, &sizBuffer);
		if( iResult!=0 )
		{
			fprintf(stderr, "Failed to get the result for the version command.\n");
		}
	}

	*ppcBUFFER_OUT = pcBuffer;
	*psizBUFFER_OUT = sizBuffer;
}



void luaopenocd::uninit(void)
{
#if 0
	struct command_context *ptCmdCtx;


	if( ptJtagDevice!=NULL )
	{
		ptCmdCtx = (struct command_context *)(ptJtagDevice->pvOpenocdContext);
		if( ptCmdCtx!=NULL )
		{
			unregister_all_commands(ptCmdCtx, NULL);

			/* free commandline interface */
			command_done(ptCmdCtx);

			adapter_quit();

			ptJtagDevice->pvOpenocdContext = NULL;
		}
	}
#endif
}

