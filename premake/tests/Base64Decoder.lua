--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: Base64DecoderTest.lua - Base64Decoder class test build configuration
--- author: Karl-Mihkel Ott

project "Base64Decoder"
    basedir("../..")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files {
        "../../include/*",
        "../../tests/Base64DecoderTest.cpp"
    }

    links { "das" }
