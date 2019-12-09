#include "lua_bridge.h"

#include "machine.h"

#include <lua.hpp>
#include <iostream>

using namespace lua;

extern retro8::Machine machine;

int pset(lua_State* L)
{
  int args = lua_gettop(L);
  //TODO: check validity of arguments

  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int c;

  if (args == 3)
    c = lua_tointeger(L, 3);
  else
    c = machine.memory().penColor()->low();

  machine.pset(x, y, static_cast<retro8::color_t>(c));

  return 0;
}

int pget(lua_State* L)
{
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);

  lua_pushinteger(L, machine.pget(x, y));

  return 1;
}

int color(lua_State* L)
{
  int c = lua_tointeger(L, 1);

  machine.color(static_cast<retro8::color_t>(c));

  return 0;
}

int line(lua_State* L)
{
  int x0 = lua_tointeger(L, 1);
  int y0 = lua_tointeger(L, 2);
  int x1 = lua_tointeger(L, 3);
  int y1 = lua_tointeger(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.memory().penColor()->low();

  machine.line(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

  return 0;
}

int rect(lua_State* L)
{
  int x0 = lua_tointeger(L, 1);
  int y0 = lua_tointeger(L, 2);
  int x1 = lua_tointeger(L, 3);
  int y1 = lua_tointeger(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.memory().penColor()->low();

  machine.rect(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

  return 0;
}

int cls(lua_State* L)
{
  int c = lua_gettop(L) == 1 ? lua_tointeger(L, -1) : 0;

  machine.cls(static_cast<retro8::color_t>(c));

  return 0;
}

int spr(lua_State* L)
{
  assert(lua_isnumber(L, 2) && lua_isnumber(L, 3));

  int idx = lua_tointeger(L, 1);
  int x = lua_tonumber(L, 2);
  int y = lua_tonumber(L, 3);

  machine.spr(idx, x, y);

  return 0;
}

int pal(lua_State* L)
{
  int c0 = lua_tonumber(L, 1);
  int c1 = lua_tonumber(L, 2);

  machine.pal(static_cast<retro8::color_t>(c0), static_cast<retro8::color_t>(c1));

  return 0;
}

int print(lua_State* L)
{
  //TODO: optimize and use const char*?
  std::string text = lua_tostring(L, 1);
  int x = lua_tonumber(L, 2); //TODO: these are optional
  int y = lua_tonumber(L, 3);
  int c = lua_gettop(L) == 4 ? lua_tointeger(L, 4) : machine.memory().penColor()->low();

  machine.print(text, x, y, static_cast<retro8::color_t>(c));

  return 0;
}

namespace math
{
  using real_t = float;
  static constexpr float PI = 3.14159265358979323846;

  int cos(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t angle = lua_tonumber(L, 1);
    real_t value = ::cos(angle * 2 * PI);
    lua_pushnumber(L, value);

    return 1;
  }
}

void lua::registerFunctions(lua_State* L)
{
  lua_register(L, "pset", pset);
  lua_register(L, "pget", pget);
  lua_register(L, "pal", pal);
  lua_register(L, "color", color);
  lua_register(L, "line", line);
  lua_register(L, "rect", rect);
  lua_register(L, "cls", cls);
  lua_register(L, "spr", spr);
  lua_register(L, "print", print);

  lua_register(L, "cos", math::cos);

}

Code::~Code()
{
  if (L)
    lua_close(L);
}

void Code::initFromSource(const std::string& code)
{
  L = luaL_newstate();

  registerFunctions(L);

  if (luaL_loadstring(L, code.c_str()))
  {
    const char* message = lua_tostring(L, -1);
    std::cout << "Error: on loadstring: " << message << std::endl;
  }

  int error = lua_pcall(L, 0, 0, 0);

  if (error)
  {
    const char* message = lua_tostring(L, -1);

    std::cout << "Error: script not loaded " << message << std::endl;
  }
}

void Code::callVoidFunction(const char* name)
{
  lua_getglobal(L, name);
  int error = lua_pcall(L, 0, 0, 0);

  if (error)
  {
    const char* message = lua_tostring(L, -1);

    std::cout << "Error in _draw function: " << message << std::endl;
  }
}