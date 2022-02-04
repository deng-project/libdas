--- DENG: dynamic engine - powerful 3D game engine
--- licence: Apache, see LICENCE file
--- file: libdas-shared.lua - shared libdas library build configuration
--- author: Karl-Mihkel Ott

project "das"
	basedir("..")
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	
	includedirs { "../include" } 
	files { 
		"../include/*.h",
		"../src/*.cpp"
	}

    filter "configurations:Release"
        removefiles { "../include/Debug.h", "../src/Debug.cpp" }
	
	removefiles { "../src/DASTool.cpp" }
	
	defines { "LIBDAS_EXPORT_LIBRARY" }
		
