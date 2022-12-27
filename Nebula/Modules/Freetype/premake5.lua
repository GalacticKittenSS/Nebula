project "Freetype"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/base/ftbase.c",
		"src/base/ftbbox.c",
		"src/base/ftbdf.c",
		"src/base/ftbitmap.c",
		"src/base/ftcid.c",
		"src/base/ftdebug.c",
		"src/base/ftfstype.c",
		"src/base/ftgasp.c",
		"src/base/ftglyph.c",
		"src/base/ftgxval.c",
		"src/base/ftinit.c",
		"src/base/ftmm.c",
		"src/base/ftotval.c",
		"src/base/ftpatent.c",
		"src/base/ftpfr.c",
		"src/base/ftstroke.c",
		"src/base/ftsynth.c",
		"src/base/ftsystem.c",
		"src/base/fttype1.c",
		"src/base/ftwinfnt.c",
		
		"src/autofit/autofit.c",
		"src/bdf/bdf.c",
		"src/cff/cff.c",
		"src/lzw/ftlzw.c",
		"src/pcf/pcf.c",
		"src/pfr/pfr.c",
		"src/psaux/psaux.c",
		"src/pshinter/pshinter.c",
		"src/psnames/psnames.c",
		"src/raster/raster.c",
		"src/sdf/sdf.c",
		"src/sfnt/sfnt.c",
		"src/smooth/smooth.c",
		"src/svg/svg.c",
		"src/truetype/truetype.c",
		"src/type1/type1.c",
		"src/cid/type1cid.c",
		"src/type42/type42.c",
		"src/winfonts/winfnt.c"
	}

	includedirs {
		"include"
	}

	defines {
		"FT2_BUILD_LIBRARY"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
