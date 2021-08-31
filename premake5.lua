workspace "netlemon"
    architecture "x64"
    
    configurations
    {
        "Debug",
        "Release"
    }
    
-- Bin / Bin-obj directory paths defined as global variables
tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"

-- Define all external dependencies
externals = {}
externals["asio"] = "external/asio-1.18.2"

project "netlemon"
    location "netlemon"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/include/**.h"
    }

    sysincludedirs
    {
        "%{prj.name}/include",
        "%{externals.asio}/include",
    }

    -- Windows
    filter {"system:windows", "configurations:*"}
        systemversion "latest" -- target latest windows upgrade when "cli gensln"
        defines { "ASIO_STANDALONE" } 

    -- [CONFIGURATION DEFINES]: usually used to turn on optimization
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on" -- pdb symbols / for release we don't want them

    filter "configurations:Release"
        runtime "Release"
        symbols "off"
        optimize "on"

project "netservereditor"
    location "netservereditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "netlemon"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/src/**.cpp"
    }

    sysincludedirs 
    { 
        "netlemon/include",
        "%{externals.asio}/include",
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest" -- target latest windows upgrade when "cli gensln"
        defines { "ASIO_STANDALONE" } 

    -- [CONFIGURATION DEFINES]: usually used to turn on optimization
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on" -- pdb symbols / for release we don't want them

    filter "configurations:Release"
        runtime "Release"
        symbols "off"
        optimize "on"

project "netclienteditor"
    location "netclienteditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "netlemon"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/src/**.cpp"
    }

    sysincludedirs 
    { 
        "netlemon/include",
        "%{externals.asio}/include",
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest" -- target latest windows upgrade when "cli gensln"
        defines { "ASIO_STANDALONE" } 

    -- [CONFIGURATION DEFINES]: usually used to turn on optimization
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on" -- pdb symbols / for release we don't want them

    filter "configurations:Release"
        runtime "Release"
        symbols "off"
        optimize "on"