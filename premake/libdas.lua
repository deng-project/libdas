--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: libdas.lua - libdas library build configuration
--- author: Karl-Mihkel Ott

local Libdas = {}

function Libdas.build() 
    project "das"
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"

        files {
            "include/*",
            "src/*.cpp",
        }

        -- Most of these files are for legacy references --
        removefiles {
            "src/DASTool.cpp"
            -- This list is incomplete, you can help by expanding it :)))
        }

end

return Libdas
