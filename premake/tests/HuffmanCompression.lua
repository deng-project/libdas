--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: HuffmanCompression.lua - Huffman compression classes test build config
--- author: Karl-Mihkel Ott

local HuffmanCompression = {}

function HuffmanCompression.build()
    project "HuffmanCompressionTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/HuffmanCompressionTest.cpp"
        }

        links { "das" }
end


return HuffmanCompression
