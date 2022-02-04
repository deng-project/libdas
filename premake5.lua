--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: premake5.lua - Main premake5 configuration file
--- author: Karl-Mihkel Ott


workspace "libdas"
    cleancommands { "make clean %{cfg.buildcfg}" }
    configurations { "Debug", "Release" }
    platforms { "Win32", "Linux" }
    includedirs { "./include" }

    warnings "Extra"
    architecture "x86_64"
    pic "On"

    -- Ignore safety warnings that MSVC gives
    filter "platforms:Win32"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    -- Enable debug symbols if requested
    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        targetdir "build/debug"
        defines { "_DEBUG" }

    filter "configurations:Release"
        symbols "Off"
        optimize "Speed"
        targetdir "build/release"

    filter {}

--- Add testing flags ---
newoption {
    trigger = "tests",
    value = "CONF",
    default = "none",
    description = "Specify the test application building",
    allowed = {
        { "none", "Do not build any unit tests (default)" },
        { "WavefrontObjParser", "Build WavefrontObjParser class test" },
        { "AsciiSTLParser", "Build AsciiSTLParser class test" },
        { "BinarySTLParser", "Build BinarySTLParser class test" },
        { "HuffmanCompression", "Build Huffman compression class test application" },
        { "WavefrontObjCompiler", "Build a Wavefront OBJ compiler test application" },
        { "STLCompiler", "Build an STL compiler test application" },
        { "Base64Decoder", "Build a Base64Decoder class test application" },
        { "JSONParser", "Build a JSONParser class test application" },
        { "GLTFParser", "Build a GLTFParser class test application" },
        -- tmp --
        { "TextureReader", "Build TextureReader class test" },
        { "DasReaderCoreTest", "Build DasReaderCore class test" },
        { "SubstringSearchTest", "Build substring search test programm" },

        -- misc --
        { "all", "Build all available unit tests" }
    }
}


newoption {
    trigger = "shared",
    description = "Build a shared libdas library"
}


-- Define an option to clean --
cleancommands {
    "make clean %{cfg.buildcfg}"
}



-- Check if given operating system is supported --
function OsCheck()
    if not os.istarget("linux") and not os.istarget("windows") then
        local host = os.target()
        host = host:gsub("%a", string.upper, 1)
        print(host .. " is not supported by Libdas :(")
        os.exit()
    end
end


-- Check which test configs to include --
function LoadTests() 
    -- Test options checking
    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "WavefrontObjParser" then
        print("Including WavefrontObjParser test config")
        require "premake/tests/WavefrontObjParser"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "AsciiSTLParser" then
        print("Including AsciiSTLParser test config")
        require "premake/tests/AsciiSTLParser"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "BinarySTLParser" then
        print("Including BinarySTLParser test config")
        require "premake/tests/BinarySTLParser"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "HuffmanCompression" then
        print("Including HuffmanCompression test config");
        require "premake/tests/HuffmanCompression"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "WavefrontObjCompiler" then
        print("Including WavefrontObjCompiler test config")
        require "premake/tests/WavefrontObjCompiler"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "STLCompiler" then
        print("Including STLCompiler test config")
        require "premake/tests/STLCompiler"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "Base64Decoder" then
        print("Including Base64Decoder test config")
        require "premake/tests/Base64Decoder"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "JSONParser" then
        print("Including JSONParser test config")
        require "premake/tests/JSONParser"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "GLTFParser" then
        print("Including GLTFParser test config")
        require "premake/tests/GLTFParserTest"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "GLTFCompiler" then
        print("Including GLTFCompiler test config")
        require "premake/tests/GLTFCompilerTest"
    end

    -- tmp --
    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "TextureReader" then
        print("Including TextureReader test config");
        require "premake/tests/TextureReader"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "DasReaderCore" then
        print("Including DasReaderCore test config")
        require "premake/tests/DasReaderCore"
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "SubstringSearchTest" then
        print("Including SubstringSearchTest test config")
        require "premake/tests/SubstringSearchTest"
    end
end


-- Load all required build configs --
function LoadBuildConfigs()
    -- Load libdas build configuration --
    if not _OPTIONS["shared"] then
        require "premake/libdas-shared"
    else
        require "premake/libdas-static"
    end

    LoadTests()
    require "premake/DASTool"
end


if not _OPTIONS["help"] then
    OsCheck()
    LoadBuildConfigs()
end
