--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: WriterCore.lua - DasWriterCore class test build configuration
--- author: Karl-Mihkel Ott

local WriterCore = {}

function WriterCore.build()
    project "WriterCoreTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "tests/WriterCoreTest.cpp"
        }

        links { "das" }
end

return WriterCore
