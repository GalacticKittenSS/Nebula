project "Nebula-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	staticruntime "off"

	targetdir ("../Nebula-Storm/Resources/Scripts")
	objdir ("../Nebula-Storm/Resources/Scripts/Intermediates")

	files {
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"
		
	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"