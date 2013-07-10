#include "plu_lua.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "plu_debug.h"
#include "plu_global_state.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

lua_State *
plu_new_lua_state(pTHX)
{
  lua_State *lua;

  /* XXX the Lua state should probably use Perl's memory allocator */
  lua = luaL_newstate();

  /* C equivalent of Lua 'perl = {}' */
  lua_newtable(lua);
  lua_setglobal(lua, "perl");

  luaL_openlibs(PLU_lua_int);

  return lua;
}

SV *
plu_get_lua_errmsg(pTHX)
{
  const char *str;
  size_t len;
  SV *errmsg;
  /* XXX this should probably use luaL_checklstring */
  str = lua_tolstring(PLU_lua_int, -1, &len);
  errmsg = sv_2mortal(newSVpv(str, (STRLEN)len));
  lua_pop(PLU_lua_int, -1);
  return errmsg;
}

int
plu_call_lua_func(pTHX_ const char *lua_func_name)
{
  /* XXX why not lua_getglobal? */
  lua_getfield(PLU_lua_int, LUA_GLOBALSINDEX, lua_func_name);
  return lua_pcall(PLU_lua_int, 0, 0, 0);
}


int
plu_compile_lua_block_or_croak(pTHX_ char *code, STRLEN len)
{
  int status;
  /* XXX can we include filename/line number information here? */
  /* XXX IIRC, we would have to use a patched Lua to tell it to use Perl's line numbers */
  status = luaL_loadbuffer(PLU_lua_int, code, len, "_INLINED_LUA");
  switch (status) {
    case 0:
      break;
    case LUA_ERRMEM:
      croak("Memory allocation problem compiling Lua code. This is dire.");
      break;
    case LUA_ERRSYNTAX:
    default:
      {
        char *str;
        STRLEN len;
        SV *err = plu_get_lua_errmsg(aTHX);
        str = SvPV(err, len);
        croak("Error compiling inline Lua code: %*s", len, str);
        break;
      }
  };
  return status;
}

