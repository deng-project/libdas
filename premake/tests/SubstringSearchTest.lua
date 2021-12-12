--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: SubstringSearchTest.lua - SubstringSearchTest test program build configuration
--- author: Karl-Mihkel Ott

local SubstringSearchTest = {}

function SubstringSearchTest.build()
    project "SubstringSearchTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/KMP.cpp"
        }

        links { "das" }
end

return SubstringSearchTest
