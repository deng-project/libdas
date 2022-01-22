--- DENG: dynamic engine - powerful 3D game engine
--- licence: Apache, see LICENCE file
--- file: libdas-static.lua - static libdas library build configuration
--- author: Karl-Mihkel Ott

project "das"
	basedir("..")
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	
	includedirs { "../include" }
	files {
		"../include/*.h",
		"../src/*.cpp"
	}
	
	removefiles {
		"../src/DASTool.cpp",
		"../src/GLTFCompiler.cpp"
	}
	
	defines { 
		"LIBDAS_EXPORT_LIBRARY",
		"LIBDAS_STATIC"
	}