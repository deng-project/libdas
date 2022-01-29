--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: GLTFCompilerTest.lua - GLTFCompiler class test build configuration
--- author: Karl-Mihkel Ott

project "GLTFCompilerTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/GLTFCompilerTest.cpp"
    }

    links { "das" }
