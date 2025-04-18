
target_dir = "bin/%{cfg.buildcfg}"
obj_dir = "bin-obj/%{cfg.buildcfg}"

workspace "space_ranger"
    startproject "space_ranger"

    configurations { "Debug", "Release" }
    flags { "MultiProcessorCompile" }
    staticruntime "off"

    filter {"system:windows", "configurations:*"}
        architecture "x64"
        systemversion "latest"
        cppdialect "C++17"

    filter "configurations:Debug"
        symbols "on"
        runtime "Debug"

    filter "configurations:Release"
        symbols "off"
        runtime "Release"
        optimize "full"
  
    project "space_ranger"
    language "C++"
    kind "ConsoleApp"

    targetdir(target_dir)
    objdir(obj_dir) 

    files { "src/main.cpp" }
    links { "third_party/PAL/lib/PAL" }
    
    includedirs { 
        "src",
        "third_party/PAL/include"
    }

    filter "configurations:Debug"
        postbuildcommands {   
            "robocopy third_party/PAL/lib bin/Debug PAL.dll"
        }

    filter "configurations:Release"
        postbuildcommands {   
            "robocopy third_party/PAL/lib bin/Release PAL.dll"
        }

    