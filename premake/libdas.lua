--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: libdas.lua - libdas library build configuration
--- author: Karl-Mihkel Ott

local Libdas = {}

function Libdas.build() 
    project "das"
        kind "SharedLib"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "src/*.cpp",
        }

        -- Most of these files are for legacy references --
        removefiles {
            "src/dam.cpp",
            "src/hashmap.cpp",
            "src/hf_comp.cpp",
            "src/huf.cpp",
            "src/objdump.cpp",
            "src/tex_loader.cpp",
            "src/uuid.cpp",
            -- This list is incomplete, you can help by expanding it :)))
        }

end

return Libdas
