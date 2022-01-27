--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: WavefrontObjCompiler.lua - WavefrontObjCompiler class test build configuration
--- author: Karl-Mihkel Ott

project "WavefrontObjCompilerTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/WavefrontObjCompilerTest.cpp"
    }

    links { "das" }
