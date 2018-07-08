# Copyright (c) 2010 Intel Coprporation
# Copyright (c) 2018 Dmitry Mikushin
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 2.6)

if ("${CMAKE_GENERATOR}" MATCHES "(Win64)")
enable_language(ASM_MASM)
endif()

# For Windows, we need the Resource Compiler language
if (WIN32)
enable_language(RC)
endif()

project(GLIntercept)

# Manual build type selection (for debugging purposes)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

# Find Python interpreter
find_package(PythonInterp REQUIRED)

set(COMMON_SOURCES 
	Src/Common/ConfigParser.cpp
	Src/Common/ErrorLog.cpp
	Src/Common/FileUtils.cpp
	Src/Common/InputUtils.cpp
	Src/Common/MiscUtils.cpp
	Src/Common/NetworkUtils.cpp)

# TODO Take care of Src/Common/prefix.cpp on Linux

set(MAINLIB_SOURCES
	Src/MainLib/BuiltInFunction.cpp
	Src/MainLib/ConfigData.cpp
	Src/MainLib/DisplayListManager.cpp
	Src/MainLib/DisplayListSaveManager.cpp
	Src/MainLib/DLLLoader.cpp
	Src/MainLib/EnumData.cpp
	Src/MainLib/ExtensionFunction.cpp
	Src/MainLib/ExtensionFunctionWrapper.cpp
	Src/MainLib/FunctionData.cpp
	Src/MainLib/FunctionParser.cpp 
	Src/MainLib/FunctionTable.cpp
	Src/MainLib/GLContext.cpp
	Src/MainLib/GLCore1_1.cpp
	Src/MainLib/GLCore1_2.cpp
	Src/MainLib/GLDriver.cpp 
	Src/MainLib/GLIntercept.cpp
	Src/MainLib/GLStubs.cpp
	Src/MainLib/GLStubsWGL.cpp
	Src/MainLib/GLWindows.cpp
	Src/MainLib/ImageManager.cpp 
	Src/MainLib/ImageSaveManager.cpp
	Src/MainLib/InterceptData.cpp
	Src/MainLib/InterceptFrame.cpp
	Src/MainLib/InterceptImage.cpp 
	Src/MainLib/InterceptLog.cpp
	Src/MainLib/InterceptLogText.cpp
	Src/MainLib/InterceptLogXML.cpp
	Src/MainLib/InterceptPluginDLLInstance.cpp 
	Src/MainLib/InterceptPluginInstance.cpp
	Src/MainLib/InterceptPluginManager.cpp
	Src/MainLib/InterceptShader.cpp 
	Src/MainLib/InterceptShaderGLSL.cpp
	Src/MainLib/MovieMaker.cpp
	Src/MainLib/ParameterData.cpp
	Src/MainLib/ShaderGLSLManager.cpp 
	Src/MainLib/ShaderGLSLSaveManager.cpp
	Src/MainLib/ShaderManager.cpp
	Src/MainLib/ShaderSaveManager.cpp
	Src/MainLib/InterceptDisplayList.cpp)

file(GLOB_RECURSE CORONA_SOURCES
	"3rdParty/corona/src/*.c"
	"3rdParty/corona/src/*.cpp")

if ("${CMAKE_GENERATOR}" MATCHES "(Win64)")

set(GENERATED_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/ExtensionFunctionStubs_64.asm)

set(STUBS_SCRIPT
	Src/MainLib/generate_64b_asm_stubs.py)

add_custom_command(
	OUTPUT ${GENERATED_SOURCES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
	COMMAND ${PYTHON_EXECUTABLE} ${STUBS_SCRIPT} ${GENERATED_SOURCES}
	COMMENT "Generating extension function stubs"
	DEPENDS Src/MainLib/generate_64b_asm_stubs.py)
set_source_files_properties("${GENERATED_SOURCES}" PROPERTIES GENERATED TRUE)

endif()

include_directories(
	Src/Common
	3rdParty/corona/src
	3rdParty/corona/src/jpeg-6b
	3rdParty/corona/src/libpng-1.2.1
	3rdParty/corona/src/libungif-4.1.0
	3rdParty/corona/src/zlib-1.1.4)

add_library(GLIntercept SHARED ${COMMON_SOURCES} ${MAINLIB_SOURCES} ${CORONA_SOURCES} ${GENERATED_SOURCES})
target_include_directories(GLIntercept PUBLIC "Src/MainLib")

if (WIN32)
set_target_properties(GLIntercept PROPERTIES OUTPUT_NAME "OpenGL32")
add_definitions(-DGLI_BUILD_WINDOWS=1)
else()
add_definitions(-DGLI_BUILD_LINUX=1)
add_definitions(-Wno-write-strings)
endif()

if (NOT WIN32)
target_link_libraries(GLIntercept png pthread)
else()
target_link_libraries(GLIntercept DbgHelp)
endif()

# Build plugins

file(GLOB GLCStrike_SOURCES
	"Src/Plugins/GLCStrike/*.cpp"
	"Src/Plugins/GLCStrike/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLCStrike SHARED ${GLCStrike_SOURCES})
target_include_directories(GLCStrike PUBLIC "Src/Plugins/GLCStrike")
if (WIN32)
target_link_libraries(GLCStrike DbgHelp)
endif()

file(GLOB GLDebugContext_SOURCES
	"Src/Plugins/GLDebugContext/*.cpp"
	"Src/Plugins/GLDebugContext/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/ErrorLog.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLDebugContext SHARED ${GLDebugContext_SOURCES})
target_include_directories(GLDebugContext PUBLIC "Src/Plugins/GLDebugContext")
if (WIN32)
target_link_libraries(GLDebugContext DbgHelp)
endif()

file(GLOB GLExtOverride_SOURCES
	"Src/Plugins/GLExtOverride/*.cpp"
	"Src/Plugins/GLExtOverride/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLExtOverride SHARED ${GLExtOverride_SOURCES})
target_include_directories(GLExtOverride PUBLIC "Src/Plugins/GLExtOverride")
if (WIN32)
target_link_libraries(GLExtOverride DbgHelp)
endif()

file(GLOB GLFramePing_SOURCES
	"Src/Plugins/GLFramePing/*.cpp"
	"Src/Plugins/GLFramePing/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLFramePing SHARED ${GLFramePing_SOURCES})
target_include_directories(GLFramePing PUBLIC "Src/Plugins/GLFramePing")
if (WIN32)
target_link_libraries(GLFramePing DbgHelp)
endif()

file(GLOB GLFreeCam_SOURCES
	"Src/Plugins/GLFreeCam/*.cpp"
	"Src/Plugins/GLFreeCam/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLFreeCam SHARED ${GLFreeCam_SOURCES})
target_include_directories(GLFreeCam PUBLIC "Src/Plugins/GLFreeCam")
if (WIN32)
target_link_libraries(GLFreeCam DbgHelp)
endif()

file(GLOB GLFuncStats_SOURCES
	"Src/Plugins/GLFuncStats/*.cpp"
	"Src/Plugins/GLFuncStats/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLFuncStats SHARED ${GLFuncStats_SOURCES})
target_include_directories(GLFuncStats PUBLIC "Src/Plugins/GLFuncStats")
if (WIN32)
target_link_libraries(GLFuncStats DbgHelp)
endif()

file(GLOB TestPlugin_SOURCES
	"Src/Plugins/TestPlugin/*.cpp"
	"Src/Plugins/TestPlugin/*.def"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(TestPlugin SHARED ${TestPlugin_SOURCES})
target_include_directories(TestPlugin PUBLIC "Src/Plugins/TestPlugin")
if (WIN32)
target_link_libraries(TestPlugin DbgHelp)
endif()

file(GLOB GLShaderEdit_SOURCES
	"Src/Plugins/GLShaderEdit/*.cpp"
	"Src/Plugins/GLShaderEdit/*.def"
	"Src/Plugins/GLShaderEdit/PreProcessor/*.c"
	"Src/Common/ConfigParser.cpp"
	"Src/Common/InputUtils.cpp"
	"Src/Common/MiscUtils.cpp")
add_library(GLShaderEdit SHARED ${GLShaderEdit_SOURCES})
target_include_directories(GLShaderEdit PUBLIC "Src/Plugins/GLShaderEdit")
if (WIN32)
target_link_libraries(GLShaderEdit DbgHelp)
endif()

file(GLOB scintilla_SOURCES
	"Src/Plugins/GLShaderEdit/scintilla/src/*.cxx")
if (WIN32)
file(GLOB scintilla_PLATFORM
	"Src/Plugins/GLShaderEdit/scintilla/win32/*.cxx"
	"Src/Plugins/GLShaderEdit/scintilla/win32/ScintRes.rc")
else()
file(GLOB scintilla_PLATFORM
	"Src/Plugins/GLShaderEdit/scintilla/gtk/*.cxx")
endif()
add_library(scintilla STATIC ${scintilla_SOURCES} ${scintilla_PLATFORM})
target_compile_definitions(scintilla PUBLIC SCI_LEXER=1)
target_include_directories(scintilla PUBLIC "Src/Plugins/GLShaderEdit/scintilla/include")
target_include_directories(scintilla PUBLIC "Src/Plugins/GLShaderEdit/scintilla/src")
if (WIN32)
target_include_directories(scintilla PUBLIC "Src/Plugins/GLShaderEdit/scintilla/win32")
endif()

file(GLOB_RECURSE glslang_SOURCES
	"3rdParty/glslang/*.c"
	"3rdParty/glslang/*.cpp")

if (WIN32)
set(FLEX ./../3rdParty/glslang/tools/flex.exe)
else()
set(FLEX flex)
endif()
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Gen_glslang.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${FLEX} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glslang/src/MachineIndependent/glslang.l
    COMMENT "Running Flex for file glslang.l"
    DEPENDS ${FLEX} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glslang/src/MachineIndependent/glslang.l)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/Gen_glslang.cpp" PROPERTIES GENERATED TRUE)
# Submit the resulting source files for compilation
LIST(APPEND glslang_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Gen_glslang.cpp)

if (WIN32)
set(BISON ./bison.exe)
else()
set(BISON bison)
endif()
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glslang/tools
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${BISON} -t -v -d --no-lines ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glslang/src/MachineIndependent/glslang.y -o ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp
    COMMENT "Running Bison for file glslang.y"
    DEPENDS ${BISON} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glslang/src/MachineIndependent/glslang.y)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp" PROPERTIES GENERATED TRUE)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp.h" PROPERTIES GENERATED TRUE)
# Submit the resulting source files for compilation
LIST(APPEND glslang_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp)

add_library(glslang STATIC ${glslang_SOURCES})
target_compile_definitions(glslang PUBLIC _ALT_NS=1)
if (NOT WIN32)
target_compile_definitions(glslang PUBLIC linux=1)
endif()
target_include_directories(glslang PUBLIC "3rdParty/glslang/src")
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/OGLCompilersDLL")
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/MachineIndependent")
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/MachineIndependent/preprocessor")
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/Public")
if (WIN32)
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/OSDependent/Windows")
else()
target_include_directories(glslang PUBLIC "3rdParty/glslang/src/OSDependent/Linux")
endif()

file(GLOB wxstedit_SOURCES
	"Src/Plugins/GLShaderEdit/wxstedit/src/*.cpp")
add_library(wxstedit STATIC ${wxstedit_SOURCES})
target_compile_definitions(wxstedit PRIVATE _LIB=1)
target_compile_definitions(wxstedit PRIVATE __WXMSW__=1)
target_compile_definitions(wxstedit PRIVATE __WX__=1)
target_include_directories(wxstedit PUBLIC "Src/Plugins/GLShaderEdit/wxstedit/include")
target_include_directories(wxstedit PUBLIC "3rdParty/wxWidgets/include")
target_include_directories(wxstedit PUBLIC "3rdParty/wxWidgets/contrib/include")
if (MSVC)
target_include_directories(wxstedit PUBLIC "3rdParty/wxWidgets/include/msvc")
endif(MSVC)

file(GLOB_RECURSE wxWidgets_SOURCES
	"3rdParty/wxWidgets/src/common/*.cpp"
	"3rdParty/wxWidgets/src/html/*.cpp"
	"3rdParty/wxWidgets/src/xml/*.cpp"
	"3rdParty/wxWidgets/src/xrc/*.cpp")
if (WIN32)
file(GLOB_RECURSE wxWidgets_PLATFORM
	"3rdParty/wxWidgets/src/msw/.*.cpp")
# Remove WinCE-specific sources
list(FILTER wxWidgets_SOURCES EXCLUDE REGEX
	"3rdParty/wxWidgets/src/msw/wince/*.cpp")
else()
file(GLOB_RECURSE wxWidgets_PLATFORM
	"3rdParty/wxWidgets/src/gtk/*.cpp")
endif()
add_library(wxWidgets STATIC ${wxWidgets_SOURCES} ${wxWidgets_PLATFORM})
target_compile_definitions(wxWidgets PRIVATE WINVER=0x0501)
target_compile_definitions(wxWidgets PRIVATE STRICT=1)
target_compile_definitions(wxWidgets PRIVATE wxUSE_BASE=1)
target_compile_definitions(wxWidgets PRIVATE UNICODE=1)
target_compile_definitions(wxWidgets PRIVATE wxUSE_PRINTING_ARCHITECTURE=1)
if (CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
target_compile_definitions(wxWidgets PRIVATE __WXDEBUG__=1)
endif()
target_include_directories(wxWidgets PUBLIC "3rdParty/wxWidgets/include")
target_include_directories(wxWidgets PUBLIC "3rdParty/wxWidgets/contrib/include")
target_include_directories(wxWidgets PUBLIC "3rdParty/wxWidgets/src/expat/lib")
target_include_directories(wxWidgets PUBLIC "3rdParty/wxWidgets/src/regex")
target_include_directories(wxWidgets PUBLIC "3rdParty/wxWidgets/src/tiff")

file(GLOB scite_SOURCES
	"Src/Plugins/GLShaderEdit/scite/src/*.cxx")
if (WIN32)
file(GLOB_RECURSE scite_PLATFORM
	"Src/Plugins/GLShaderEdit/scite/win32/*.cpp"
	"Src/Plugins/GLShaderEdit/scite/win32/*.cxx"
	"Src/Plugins/GLShaderEdit/scite/win32/SciTERes.rc")
else()
file(GLOB scite_PLATFORM
	"Src/Plugins/GLShaderEdit/scite/gtk/*.cxx")
endif()
add_executable(GLISciTE ${scite_SOURCES} ${scite_PLATFORM} Src/Common/NetworkUtils.cpp)
target_compile_definitions(GLISciTE PUBLIC GLI_CHANGES=1)
target_compile_definitions(GLISciTE PUBLIC STATIC_BUILD=1)
target_compile_definitions(GLISciTE PUBLIC NO_LUA=1)
target_compile_definitions(GLISciTE PUBLIC GLI_BUILD_WINDOWS=1)
target_include_directories(GLISciTE PUBLIC "Src/Plugins/GLShaderEdit/scite/src")
target_include_directories(GLISciTE PUBLIC "Src/Plugins/GLShaderEdit/scintilla/include")
if (WIN32)
target_compile_definitions(GLISciTE PUBLIC _AFXDLL=1)
set_target_properties(GLISciTE PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFEST:NO")
endif()

if (WIN32)
target_link_libraries(GLISciTE scintilla glslang Comctl32 Imm32 Rpcrt4 wxstedit wxWidgets)
else()
target_link_libraries(GLISciTE scintilla glslang wxstedit wxWidgets)
endif()

add_custom_command(TARGET GLISciTE
	POST_BUILD COMMAND
	"mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\Src\\Plugins\\GLShaderEdit\\scite\\win32\\SciTE.exe.manifest\" -outputresource:\"$<TARGET_FILE:GLISciTE>\"\;\#1
	COMMENT "Adding custom manifest file...")

