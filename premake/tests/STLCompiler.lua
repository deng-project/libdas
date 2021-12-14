--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: STLCompiler.lua - STLCompiler class test build configuration
--- author: Karl-Mihkel Ott

local STLCompiler = {}

function STLCompiler.build()
    project "STLCompilerTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/STLCompilerTest.cpp"
        }

        links { "das" }
end

return STLCompiler
