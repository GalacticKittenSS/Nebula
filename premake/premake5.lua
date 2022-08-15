project "Premake"
	kind "Utility"
	location ".."

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"../**premake5.lua"
	}

	postbuildmessage "Regenerating project files with Premake5!"
	postbuildcommands {
		"\"%{prj.location}premake/premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
	}