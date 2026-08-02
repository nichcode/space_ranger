
project "SpaceRanger"
    kind "ConsoleApp"
    targetdir(targetDir)
    objdir(objDir)
    language "C++"

    includedirs {
        "src",
        "third_party/PAL/include",
        "third_party/glm/include"
    }

    files { "src/*.cpp" }
    links { "PAL2" }