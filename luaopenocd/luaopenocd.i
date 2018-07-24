%module luaopenocd

%include "stdint.i"
%include "typemaps.i"

%{
	#include "wrapper.h"
%}


/* This typemap adds "SWIGTYPE_" to the name of the input parameter to
 * construct the swig typename. The parameter name must match the definition
 * in the wrapper.
 */
%typemap(in, numinputs=0) swig_type_info *
%{
	$1 = SWIGTYPE_$1_name;
%}


/* This typemap passes Lua state to the function. The function must create one
 * lua object on the stack. This is passes as the return value to lua.
 * No further checks are done!
 */
%typemap(in, numinputs=0) lua_State *MUHKUH_SWIG_OUTPUT_CUSTOM_OBJECT
%{
	$1 = L;
	++SWIG_arg;
%}


/* This typemap passes Lua state to the function. The function must create 0
 * or more lua object on the stack. The function returns the number of objects
 * created. They are passed as the return value to lua.
 * No further checks are done!
 */
%typemap(in, numinputs=0) (lua_State *MUHKUH_SWIG_OUTPUT_CUSTOM_OBJECT_LIST, unsigned int *uiNUMBER_OF_CREATED_OBJECTS)
%{
	unsigned int uiObjects;
	$1 = L;
	$2 = &uiObjects;
%}
%typemap(argout) (lua_State *MUHKUH_SWIG_OUTPUT_CUSTOM_OBJECT_LIST, unsigned int *uiNUMBER_OF_CREATED_OBJECTS)
%{
	SWIG_arg += uiObjects;
%}


%typemap(in) (const char *pcBUFFER_IN, size_t sizBUFFER_IN)
{
        size_t sizBuffer;
        $1 = (char*)lua_tolstring(L, $argnum, &sizBuffer);
        $2 = sizBuffer;
}


%typemap(in, numinputs=0) (char **ppcBUFFER_OUT, size_t *psizBUFFER_OUT)
%{
	char *pcOutputData;
	size_t sizOutputData;
	$1 = &pcOutputData;
	$2 = &sizOutputData;
%}

/* NOTE: This "argout" typemap can only be used in combination with the above "in" typemap. */
%typemap(argout) (char **ppcBUFFER_OUT, size_t *psizBUFFER_OUT)
%{
	if( pcOutputData!=NULL && sizOutputData!=0 )
	{
		lua_pushlstring(L, pcOutputData, sizOutputData);
		free(pcOutputData);
	}
	else
	{
		lua_pushnil(L);
	}
	++SWIG_arg;
%}


%typemap(in, numinputs=0) (unsigned char *pucARGUMENT_OUT)
%{
	unsigned char ucArgument;
	$1 = &ucArgument;
%}
%typemap(argout) (unsigned char *pucARGUMENT_OUT)
%{
	lua_pushnumber(L, ucArgument);
	++SWIG_arg;
%}



%typemap(in, numinputs=0) (unsigned short *pusARGUMENT_OUT)
%{
	unsigned short usArgument;
	$1 = &usArgument;
%}
%typemap(argout) (unsigned short *pusARGUMENT_OUT)
%{
	lua_pushnumber(L, usArgument);
	++SWIG_arg;
%}



%typemap(in, numinputs=0) (int *piARGUMENT_OUT)
%{
	int iArgument;
	$1 = &iArgument;
%}
%typemap(argout) (int *piARGUMENT_OUT)
%{
	lua_pushnumber(L, iArgument);
	++SWIG_arg;
%}



%typemap(in, numinputs=0) (unsigned int *puiARGUMENT_OUT)
%{
	unsigned int uiArgument;
	$1 = &uiArgument;
%}
%typemap(argout) (unsigned int *puiARGUMENT_OUT)
%{
	lua_pushnumber(L, uiArgument);
	++SWIG_arg;
%}



%typemap(out) RESULT_INT_TRUE_OR_NIL_WITH_ERR
%{
	if( $1>=0 )
	{
		lua_pushboolean(L, 1);
		SWIG_arg = 1;
	}
	else
	{
		lua_pushnil(L);
		lua_pushstring(L, arg1->get_error_string());
		SWIG_arg = 2;
	}
%}



%typemap(out) RESULT_INT_NOTHING_OR_NIL_WITH_ERR
%{
	if( $1<0 )
	{
		lua_pushnil(L);
		lua_pushstring(L, arg1->get_error_string());
		SWIG_arg = 2;
	}
	else
	{
%}
%typemap(ret) RESULT_INT_NOTHING_OR_NIL_WITH_ERR
%{
	}
%}



%typemap(out) RESULT_INT_INT_OR_NIL_WITH_ERR
%{
	if( $1>=0 )
	{
		lua_pushnumber(L, $1);
		SWIG_arg = 1;
	}
	else
	{
		lua_pushnil(L);
		lua_pushstring(L, arg1->get_error_string());
		SWIG_arg = 2;
	}
%}

%include "wrapper.h"
