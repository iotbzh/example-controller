--[[
  Copyright (C) 2016 "IoT.bzh"
  Author Fulup Ar Foll <fulup@iot.bzh>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.


  Provide sample LUA routines to be used with AGL control "lua_docall" API
--]]

function _Onload_Lua_Function(source, args, query)
	AFB:info(source, "Plugin controller-example-lua-plugin : _Onload_Lua_Function function called correctly")
end

function _Control_Lua_Function(request, args, query)
	AFB:info(request, "Plugin controller-example-lua-plugin : _Onload_Lua_Function function called correctly")
	AFB:success(request, "Plugin controller-example-lua-plugin : _Onload_Lua_Function function called correctly")
end

function _Event_Lua_Function(source, args, event)
	AFB:info(source, "Plugin controller-example-lua-plugin : _Onload_Lua_Function function called correctly")
end