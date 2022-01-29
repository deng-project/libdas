--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: GLTFParser.lua - GLTFParser class test build configuration
--- author: Karl-Mihkel Ott

project "GLTFParserTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/GLTFParserTest.cpp"
    }

    links { "das" }
