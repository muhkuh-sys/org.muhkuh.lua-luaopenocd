
#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif


#include <stdint.h>


#ifndef SWIGRUNTIME
#include <swigluarun.h>

/* swigluarun does not include the lua specific defines. Add them here. */
typedef struct
{
	lua_State* L; /* the state */
	int ref;      /* a ref in the lua global index */
} SWIGLUA_REF;
#endif



#ifndef __WRAPPER_H__
#define __WRAPPER_H__


class luaopenocd
{
public:
	luaopenocd(SWIGLUA_REF tLuaFn);
	~luaopenocd(void);


	int initialize(void);
	int run(char *strLine);
	void get_result(char **ppcBUFFER_OUT, size_t *psizBUFFER_OUT);
	void uninit(void);

/* Do not use the rest in Swig. */
#if !defined(SWIG)
	static void openocd_output_handler(void *pvUser, const char *pcLine, size_t sizLine);
	void output_handler(const char *pcLine, size_t sizLine);


private:
	typedef enum STATE_ENUM
	{
		STATE_Uninitialized = 0,
		STATE_Initialized = 1,
		STATE_Open = 2,
		STATE_Error = 3
	} STATE_T;

	typedef struct OPENOCD_NAME_RESOLVE_STRUCT
	{
		const char *pstrSymbolName;
		size_t sizPointerOffset;
	} OPENOCD_NAME_RESOLVE_T;

	typedef void (*PFN_MUHKUH_OPENOCD_OUTPUT_HANDLER_T) (void *pvUser, const char *pcLine, size_t sizLine);

	typedef void * (*PFN_MUHKUH_OPENOCD_INIT_T) (const char *pcScriptSearchDir, PFN_MUHKUH_OPENOCD_OUTPUT_HANDLER_T pfnOutputHandler, void *pvOutputHanderData);
	typedef int (*PFN_MUHKUH_OPENOCD_GET_RESULT_T) (void *pvContext, char *pcBuffer, size_t sizBufferMax);
	typedef int (*PFN_MUHKUH_OPENOCD_GET_RESULT_ALLOC_T) (void *pvContext, char **ppcBuffer, size_t *psizBuffer);
	typedef int (*PFN_MUHKUH_OPENOCD_COMMAND_RUN_LINE_T) (void *pvContext, const char *pcLine);
	typedef void (*PFN_MUHKUH_OPENOCD_UNINIT_T) (void *pvContext);

	typedef struct MUHKUH_OPENOCD_FUNCTION_POINTERS_UNION
	{
		PFN_MUHKUH_OPENOCD_INIT_T pfnInit;
		PFN_MUHKUH_OPENOCD_GET_RESULT_T pfnGetResult;
		PFN_MUHKUH_OPENOCD_GET_RESULT_ALLOC_T pfnGetResultAlloc;
		PFN_MUHKUH_OPENOCD_COMMAND_RUN_LINE_T pfnCommandRunLine;
		PFN_MUHKUH_OPENOCD_UNINIT_T pfnUninit;
	} MUHKUH_OPENOCD_FUNCTION_POINTERS_T;


	typedef union MUHKUH_OPENOCD_FUNCTIONS_UNION
	{
		MUHKUH_OPENOCD_FUNCTION_POINTERS_T tFn;
		void *pv[sizeof(MUHKUH_OPENOCD_FUNCTION_POINTERS_T)/sizeof(void*)];
	} MUHKUH_OPENOCD_FUNCTIONS_T;


	typedef struct ROMLOADER_JTAG_DEVICE_STRUCT
	{
		void *pvSharedLibraryHandle;
		MUHKUH_OPENOCD_FUNCTIONS_T tFunctions;
		void *pvOpenocdContext;
	} ROMLOADER_JTAG_DEVICE_T;


	static const OPENOCD_NAME_RESOLVE_T atOpenOcdResolve[5];

	STATE_T m_tState;
	ROMLOADER_JTAG_DEVICE_T m_tDevice;
	SWIGLUA_REF m_tLuaFn;

	/* This is the path to the folder where the romloader_jtag plugin, the
	 * openocd shared object and the TCL scripts live.
	 */
	char *m_pcPluginPath;
	/* This is the path to the openocd data files. */
	char *m_pcOpenocdDataPath;
	/* This is the full path to the openocd shared object. */
	char *m_pcOpenocdSharedObjectPath;


	void get_plugin_path(void);
	void get_openocd_path(void);

	int openocd_open(void);
	void openocd_close(void);
#endif
};


#endif  /* __WRAPPER_H__ */
