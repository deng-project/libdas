--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: JSONParser.lua - JSONParser class test build configuration
--- author: Karl-Mihkel Ott

project "JSONParserTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/JSONParserTest.cpp"
    }

    links { "das" }
