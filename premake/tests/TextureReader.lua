--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: TextureReader.lua - TextureReader class test build configuration
--- author: Karl-Mihkel Ott

project "TextureReaderTest"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/TextureReaderTest.cpp"
    }

    links { "das" }
