--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: DasReaderCore.lua - DasReaderCore class test build configuration
--- author: Karl-Mihkel Ott

local DasReaderCore = {}

function DasReaderCore.build()
    project "DasReaderCoreTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "tests/DasReaderCoreTest.cpp"
        }

        links { "das" }
end

return DasReaderCore
