-- premake5.lua

--test
--[[
print("------->",path.getabsolute("DomiEngine/network"))
print("------->",os.matchdirs("DomiEngine/*"))

for k,v in pairs(os.matchdirs("DomiEngine/common/*")) do
	print(v,path.getdirectory(v),path.getname(v),os.getcwd(v))
end
--]]

-- solution
solution("Domi")
	configurations { "Debug", "Release" }
	location "build"

	--消除vs警告
	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS","WIN32" }

-- game server
project "gameServer"
	kind "ConsoleApp"
	language "C++"
	targetdir "Output/%{cfg.buildcfg}"
	targetname "gameserver"

	-- 包含目录
	includedirs {
		"DomiServer",
		"DomiEngine",
		"3rd/include/libevent",
		"3rd/include/libevent/include",
		"3rd/include/libevent/compat",
		"3rd/include/libevent/WIN32-Code",
		"3rd/include",
	}

	-- 库目录
	libdirs {"3rd/lib"}
	
	files { "DomiServer/**.h", "DomiServer/**.cpp" ,"DomiServer/**.cc"}
	if os.get() == "windows" then	-- windows
		--链接库
		links {"domiEngine","libevent", "libprotobuf","ws2_32", "wsock32"}
		
		vpaths {
			-- main
			["main"] = {"DomiServer/main/*.h","DomiServer/main/*.cpp","DomiServer/main/*.cc"},
			
			-- logic
			["logic/logicnet"] = {"DomiServer/logic/logicnet/*.*"},
		}
	else -- linux
		links {"domiEngine","libevent", "libprotobuf"}
	end

	-- 筛选器
	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"		

-- engine lib
project "domiEngine"
	kind "StaticLib"
	language "C++"
	targetdir "Output/%{cfg.buildcfg}"
	targetname "domiengine"

	-- 包含目录
	includedirs {
		"DomiEngine",
		"3rd/include/libevent",
		"3rd/include/libevent/include",
		"3rd/include/libevent/compat",
		"3rd/include/libevent/WIN32-Code",
		"3rd/include",
	}

	files { "DomiEngine/**.h", "DomiEngine/**.cpp" ,"DomiEngine/**.cc"}
	if os.get() == "windows" then	-- windows
		local vpathsTable = {}
		
		-- common
		for k,v in pairs(os.matchdirs("DomiEngine/common/*")) do
			local vpathName = string.sub(v,#"DomiEngine/"+1)
			if vpathName then
				vpathsTable[vpathName] = {v.."*/*"}
			end
		end
		
		-- network
		vpathsTable["network"] = {"DomiEngine/network/*.*"}
		
		vpaths(vpathsTable)
	end

	-- 筛选器
	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"