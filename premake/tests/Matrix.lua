--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: Matrix.lua - Matrix classes test build configuration
--- author: Karl-Mihkel Ott


local Matrix = {}

function Matrix.build()
    project "MatrixTest"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++14"

        files {
            "include/*",
            "tests/MatrixIterationTest.cpp"
        }

        links { "das" }
end

return Matrix
