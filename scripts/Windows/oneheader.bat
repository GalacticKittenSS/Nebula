@echo off
setlocal EnableDelayedExpansion

oneheader.exe ../../Nebula/src/Nebula/Core/ > Core.txt
type Core.txt > ../../Nebula/src/Nebula/Core/Core.txt

oneheader.exe ../../Nebula/src/Nebula/Debug/ > Debug.txt
type Debug.txt > ../../Nebula/src/Nebula/Debug/Debug.txt

oneheader.exe ../../Nebula/src/Nebula/Events/ > Events.txt
type Events.txt > ../../Nebula/src/Nebula/Events/Events.txt

oneheader.exe ../../Nebula/src/Nebula/imgui/ > imgui.txt
type imgui.txt > ../../Nebula/src/Nebula/imgui/imgui.txt

oneheader.exe ../../Nebula/src/Nebula/Renderer/ > Renderer.txt
type Renderer.txt > ../../Nebula/src/Nebula/Renderer/Renderer.txt

set "String="
set "Bool=false"

for /f %%a in (words.txt) do (
  set "line=%%a"
  
  if "!line!"=="--Order" (
    set "Bool=true"
  )
  
  if "!line!"=="--Excluded" (
    set "Bool=false"
  )
  
   if "!Bool!"=="true" (
   if not "!line!"=="--Order" set "String=!String!%%a end.txt ")
)

type End.txt > ../../Nebula/OneHeader_Temp/End.txt
type Words.txt > ../../Nebula/OneHeader_Temp/Words.txt

cd ../../Nebula/src/Nebula/Core

for /f %%a in (Core.txt) do (
  set "file=%%a"
  set "file=!file:../../Nebula/src/Nebula/Core/=!"
  echo !file!
  type !file! > ../../../OneHeader_Temp/!file!
)

del Core.txt

cd ../Debug

for /f %%a in (Debug.txt) do (
  set "file=%%a"
  set "file=!file:../../Nebula/src/Nebula/Debug/=!"
  echo !file!
  type !file! > ../../../OneHeader_Temp/!file!
)

del Debug.txt

cd ../Events

for /f %%a in (Events.txt) do (
  set "file=%%a"
  set "file=!file:../../Nebula/src/Nebula/Events/=!"
  echo !file!
  type !file! > ../../../OneHeader_Temp/!file!
)

del Events.txt

cd ../imgui

for /f %%a in (imgui.txt) do (
  set "file=%%a"
  set "file=!file:../../Nebula/src/Nebula/imgui/=!"
  echo !file!
  type !file! > ../../../OneHeader_Temp/!file!
)

del imgui.txt

cd ../Renderer

for /f %%a in (Renderer.txt) do (
  set "file=%%a"
  set "file=!file:../../Nebula/src/Nebula/Renderer/=!"
  echo !file!
  type !file! > ../../../OneHeader_Temp/!file!
)

del Renderer.txt

cd ../..

type nbpch.h > ../OneHeader_Temp/nbpch.h

PAUSE

cd ../OneHeader_Temp

type !string! > OneHeader_Nebula.h

del End.txt

set "bool=false"

for /f "usebackqdelims=" %%w in (Words.txt) do (
  set "line=%%w"
  if "!line!"=="--Excluded" set "bool=true"
  if "!line!"=="--Order" set "bool=false"
  
  if "!bool!"=="true" ( if not "!line!"=="--Excluded" (

  (for /f "usebackqdelims=" %%o in (OneHeader_Nebula.h) do (
    set "line=%%o"
	set "line=!line:%%w=!"
    set "ns=!line: =!"
    if not "!ns!"=="" (
	  echo !line! )
	)) > out.txt
  
  type out.txt > OneHeader_Nebula.h

  echo Removed Keyword %%w
)))
  
(for /f "usebackqdelims=" %%a in (OneHeader_Nebula.h) do (
    set "line=%%a"
    set "ns=!line: =!"
  if not "!ns!"=="ECHOisoff." (
    set "line=!line:  =!"
    echo !line! )
  )) > out.txt

type out.txt > ../OneHeader_Nebula.h

del out.txt
del words.txt

cd ../../scripts/Windows

del Core.txt
del Debug.txt
del Events.txt
del imgui.txt
del Renderer.txt
del Core.txt