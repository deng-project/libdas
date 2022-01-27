--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: SubstringSearchTest.lua - SubstringSearchTest test program build configuration
--- author: Karl-Mihkel Ott

project "SubstringSearchTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/KMP.cpp"
    }

    links { "das" }
