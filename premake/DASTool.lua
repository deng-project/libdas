--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: DASTool.lua - DASTool cli utility build configuration
--- author: Karl-Mihkel Ott

project "DASTool"
	basedir("..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

	includedirs { "../include" }
    files {
		"../include/*",
		"../src/DASTool.cpp"
	}
	
	links { "das" }

