--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: Base64DecoderTest.lua - Base64Decoder class test build configuration
--- author: Karl-Mihkel Ott

local Base64Decoder = {}

function Base64Decoder.build()
    project "Base64Decoder"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/Base64DecoderTest.cpp"
        }

        links { "das" }
end

return Base64Decoder
