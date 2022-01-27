--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: AsciiSTLParser.lua - AsciiSTLParser class test build configuration
--- author: Karl-Mihkel Ott

project "AsciiSTLParserTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/AsciiSTLParseTest.cpp"
    }

    links { "das" }
