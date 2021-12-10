--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: DASTool.lua - DASTool cli utility build configuration
--- author: Karl-Mihkel Ott

local DASTool = {}

function DASTool.build(use_lib)
    project "DASTool"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        if not use_lib then
            files {
                "include/*",
                "src/*.cpp"
            }
        else
            files {
                "include/*",
                "src/DASTool.cpp"
            }

            links { "das" }
        end
end


return DASTool
