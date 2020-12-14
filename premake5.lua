local solution_dir = _ACTION

function setupIncludeDirs()
   includedirs {
      "ImageSignaturer",
   }
end

function setupSlotion()
   location(solution_dir)
   solution "ImageSignaturer"
      configurations {
         "Debug", 
         "Release"
      }

      platforms { "Win64" }
      warnings "Extra"
      floatingpoint "Fast"
      symbols "On"
      cppdialect "C++17"
      rtti "On"
      characterset ("MBCS")

      configuration "Debug*"
         defines { "DEBUG", "_DEBUG" }

      configuration "Release*"
         defines { "NDEBUG" }
         optimize "On"

      filter { "platforms:Win*", "configurations:Debug*" }
         defines { "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
         system "Windows"
         architecture "x86_64"
         staticruntime "Off"

      filter { "platforms:Win*", "configurations:Release*" }
         defines { "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
         system "Windows"
         architecture "x86_64"
         staticruntime "Off"
end

function setupProj()
   project "ImageSignaturer"
   kind "ConsoleApp"
   language "C++"

   files { 
      "ImageSignaturer/*.*"
   }

   filter { "configurations:Debug*" }
      targetdir (solution_dir .. "/bin/debug")

   filter { "configurations:Release*" }
      targetdir (solution_dir .. "/bin/release")
end

setupSlotion()
setupIncludeDirs()
setupProj()