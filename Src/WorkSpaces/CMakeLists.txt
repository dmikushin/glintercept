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
	../Common/ConfigParser.cpp
	../Common/ErrorLog.cpp
	../Common/FileUtils.cpp
	../Common/InputUtils.cpp
	../Common/MiscUtils.cpp
	../Common/NetworkUtils.cpp)

# TODO Take care of ../Common/prefix.cpp on Linux

set(MAINLIB_SOURCES
	../MainLib/BuiltInFunction.cpp
	../MainLib/ConfigData.cpp
	../MainLib/DisplayListManager.cpp
	../MainLib/DisplayListSaveManager.cpp
	../MainLib/DLLLoader.cpp
	../MainLib/EnumData.cpp
	../MainLib/ExtensionFunction.cpp
	../MainLib/ExtensionFunctionWrapper.cpp
	../MainLib/FunctionData.cpp
	../MainLib/FunctionParser.cpp 
	../MainLib/FunctionTable.cpp
	../MainLib/GLContext.cpp
	../MainLib/GLCore1_1.cpp
	../MainLib/GLCore1_2.cpp
	../MainLib/GLDriver.cpp 
	../MainLib/GLIntercept.cpp
	../MainLib/GLStubs.cpp
	../MainLib/GLStubsWGL.cpp
	../MainLib/GLWindows.cpp
	../MainLib/ImageManager.cpp 
	../MainLib/ImageSaveManager.cpp
	../MainLib/InterceptData.cpp
	../MainLib/InterceptFrame.cpp
	../MainLib/InterceptImage.cpp 
	../MainLib/InterceptLog.cpp
	../MainLib/InterceptLogText.cpp
	../MainLib/InterceptLogXML.cpp
	../MainLib/InterceptPluginDLLInstance.cpp 
	../MainLib/InterceptPluginInstance.cpp
	../MainLib/InterceptPluginManager.cpp
	../MainLib/InterceptShader.cpp 
	../MainLib/InterceptShaderGLSL.cpp
	../MainLib/MovieMaker.cpp
	../MainLib/ParameterData.cpp
	../MainLib/ShaderGLSLManager.cpp 
	../MainLib/ShaderGLSLSaveManager.cpp
	../MainLib/ShaderManager.cpp
	../MainLib/ShaderSaveManager.cpp
	../MainLib/InterceptDisplayList.cpp)

file(GLOB_RECURSE CORONA_SOURCES
	"../../3rdParty/Corona-1.0.2-Static/src/*.c"
	"../../3rdParty/Corona-1.0.2-Static/src/*.cpp")

if ("${CMAKE_GENERATOR}" MATCHES "(Win64)")

set(GENERATED_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/ExtensionFunctionStubs_64.asm)

set(STUBS_SCRIPT
	../MainLib/generate_64b_asm_stubs.py)

add_custom_command(
	OUTPUT ${GENERATED_SOURCES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
	COMMAND ${PYTHON_EXECUTABLE} ${STUBS_SCRIPT} ${GENERATED_SOURCES}
	COMMENT "Generating extension function stubs"
	DEPENDS ../MainLib/generate_64b_asm_stubs.py)
set_source_files_properties("${GENERATED_SOURCES}" PROPERTIES GENERATED TRUE)

endif()

include_directories(
	..
	../Common
	../../3rdParty/Corona-1.0.2-Static/src
	../../3rdParty/Corona-1.0.2-Static/src/jpeg-6b
	../../3rdParty/Corona-1.0.2-Static/src/libpng-1.2.1
	../../3rdParty/Corona-1.0.2-Static/src/libungif-4.1.0
	../../3rdParty/Corona-1.0.2-Static/src/zlib-1.1.4)

add_library(GLIntercept SHARED ${COMMON_SOURCES} ${MAINLIB_SOURCES} ${CORONA_SOURCES} ${GENERATED_SOURCES})
target_include_directories(GLIntercept PUBLIC "../MainLib")

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
	"../Plugins/GLCStrike/*.cpp"
	"../Plugins/GLCStrike/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLCStrike SHARED ${GLCStrike_SOURCES})
target_include_directories(GLCStrike PUBLIC "../Plugins/GLCStrike")
if (WIN32)
target_link_libraries(GLCStrike DbgHelp)
endif()

file(GLOB GLDebugContext_SOURCES
	"../Plugins/GLDebugContext/*.cpp"
	"../Plugins/GLDebugContext/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/ErrorLog.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLDebugContext SHARED ${GLDebugContext_SOURCES})
target_include_directories(GLDebugContext PUBLIC "../Plugins/GLDebugContext")
if (WIN32)
target_link_libraries(GLDebugContext DbgHelp)
endif()

file(GLOB GLExtOverride_SOURCES
	"../Plugins/GLExtOverride/*.cpp"
	"../Plugins/GLExtOverride/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLExtOverride SHARED ${GLExtOverride_SOURCES})
target_include_directories(GLExtOverride PUBLIC "../Plugins/GLExtOverride")
if (WIN32)
target_link_libraries(GLExtOverride DbgHelp)
endif()

file(GLOB GLFramePing_SOURCES
	"../Plugins/GLFramePing/*.cpp"
	"../Plugins/GLFramePing/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLFramePing SHARED ${GLFramePing_SOURCES})
target_include_directories(GLFramePing PUBLIC "../Plugins/GLFramePing")
if (WIN32)
target_link_libraries(GLFramePing DbgHelp)
endif()

file(GLOB GLFreeCam_SOURCES
	"../Plugins/GLFreeCam/*.cpp"
	"../Plugins/GLFreeCam/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLFreeCam SHARED ${GLFreeCam_SOURCES})
target_include_directories(GLFreeCam PUBLIC "../Plugins/GLFreeCam")
if (WIN32)
target_link_libraries(GLFreeCam DbgHelp)
endif()

file(GLOB GLFuncStats_SOURCES
	"../Plugins/GLFuncStats/*.cpp"
	"../Plugins/GLFuncStats/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLFuncStats SHARED ${GLFuncStats_SOURCES})
target_include_directories(GLFuncStats PUBLIC "../Plugins/GLFuncStats")
if (WIN32)
target_link_libraries(GLFuncStats DbgHelp)
endif()

file(GLOB TestPlugin_SOURCES
	"../Plugins/TestPlugin/*.cpp"
	"../Plugins/TestPlugin/*.def"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(TestPlugin SHARED ${TestPlugin_SOURCES})
target_include_directories(TestPlugin PUBLIC "../Plugins/TestPlugin")
if (WIN32)
target_link_libraries(TestPlugin DbgHelp)
endif()

file(GLOB GLShaderEdit_SOURCES
	"../Plugins/GLShaderEdit/*.cpp"
	"../Plugins/GLShaderEdit/*.def"
	"../Plugins/GLShaderEdit/PreProcessor/*.c"
	"../Common/ConfigParser.cpp"
	"../Common/InputUtils.cpp"
	"../Common/MiscUtils.cpp")
add_library(GLShaderEdit SHARED ${GLShaderEdit_SOURCES})
target_include_directories(GLShaderEdit PUBLIC "../Plugins/GLShaderEdit")
if (WIN32)
target_link_libraries(GLShaderEdit DbgHelp)
endif()

file(GLOB scintilla_SOURCES
	"../Plugins/GLShaderEdit/scintilla/src/*.cxx")
if (WIN32)
file(GLOB scintilla_PLATFORM
	"../Plugins/GLShaderEdit/scintilla/win32/*.cxx"
	"../Plugins/GLShaderEdit/scintilla/win32/ScintRes.rc")
else()
file(GLOB scintilla_PLATFORM
	"../Plugins/GLShaderEdit/scintilla/gtk/*.cxx")
endif()
add_library(scintilla STATIC ${scintilla_SOURCES} ${scintilla_PLATFORM})
target_compile_definitions(scintilla PUBLIC SCI_LEXER=1)
target_include_directories(scintilla PUBLIC "../Plugins/GLShaderEdit/scintilla/include")
target_include_directories(scintilla PUBLIC "../Plugins/GLShaderEdit/scintilla/src")
if (WIN32)
target_include_directories(scintilla PUBLIC "../Plugins/GLShaderEdit/scintilla/win32")
endif()

file(GLOB_RECURSE glslang_SOURCES
	"../../3rdParty/OpenGLCompilerSept202005/*.c"
	"../../3rdParty/OpenGLCompilerSept202005/*.cpp")

if (WIN32)
set(FLEX ./../3rdParty/OpenGLCompilerSept202005/tools/flex.exe)
else()
set(FLEX flex)
endif()
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Gen_glslang.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${FLEX} ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent/glslang.l
    COMMENT "Running Flex for file glslang.l"
    DEPENDS ${FLEX} ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent/glslang.l)
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
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/OpenGLCompilerSept202005/tools
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${BISON} -t -v -d --no-lines ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent/glslang.y -o ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp
    COMMENT "Running Bison for file glslang.y"
    DEPENDS ${BISON} ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent/glslang.y)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp" PROPERTIES GENERATED TRUE)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp.h" PROPERTIES GENERATED TRUE)
# Submit the resulting source files for compilation
LIST(APPEND glslang_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/glslang.cpp)

add_library(glslang STATIC ${glslang_SOURCES})
target_compile_definitions(glslang PUBLIC _ALT_NS=1)
if (NOT WIN32)
target_compile_definitions(glslang PUBLIC linux=1)
endif()
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang")
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/OGLCompilersDLL")
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent")
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang/MachineIndependent/preprocessor")
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang/Public")
if (WIN32)
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang/OSDependent/Windows")
else()
target_include_directories(glslang PUBLIC "../../3rdParty/OpenGLCompilerSept202005/glslang/OSDependent/Linux")
endif()

file(GLOB wxstedit_SOURCES
	"../Plugins/GLShaderEdit/wxstedit/src/*.cpp")
add_library(wxstedit STATIC ${wxstedit_SOURCES})
target_compile_definitions(wxstedit PRIVATE _LIB=1)
target_compile_definitions(wxstedit PRIVATE __WXMSW__=1)
target_compile_definitions(wxstedit PRIVATE __WX__=1)
target_include_directories(wxstedit PUBLIC "../Plugins/GLShaderEdit/wxstedit/include")
target_include_directories(wxstedit PUBLIC "../../3rdParty/wxWidgets/include")
target_include_directories(wxstedit PUBLIC "../../3rdParty/wxWidgets/contrib/include")
if (MSVC)
target_include_directories(wxstedit PUBLIC "../../3rdParty/wxWidgets/include/msvc")
endif(MSVC)

if (WIN32)
add_subdirectory(../../3rdParty/lua ${CMAKE_CURRENT_BINARY_DIR}/lua)
endif()

file(GLOB scite_SOURCES
	"../Plugins/GLShaderEdit/scite/src/*.cxx")
if (WIN32)
file(GLOB_RECURSE scite_PLATFORM
	"../Plugins/GLShaderEdit/scite/win32/*.cpp"
	"../Plugins/GLShaderEdit/scite/win32/*.cxx"
	"../Plugins/GLShaderEdit/scite/win32/SciTERes.rc")
else()
file(GLOB scite_PLATFORM
	"../Plugins/GLShaderEdit/scite/gtk/*.cxx")
endif()
add_executable(GLISciTE ${scite_SOURCES} ${scite_PLATFORM} ../Common/NetworkUtils.cpp)
target_compile_definitions(GLISciTE PUBLIC GLI_CHANGES=1)
target_compile_definitions(GLISciTE PUBLIC STATIC_BUILD=1)
target_compile_definitions(GLISciTE PUBLIC NO_LUA=1)
target_compile_definitions(GLISciTE PUBLIC GLI_BUILD_WINDOWS=1)
target_include_directories(GLISciTE PUBLIC "../Plugins/GLShaderEdit/scite/src")
target_include_directories(GLISciTE PUBLIC "../Plugins/GLShaderEdit/scintilla/include")
if (WIN32)
target_compile_definitions(GLISciTE PUBLIC _AFXDLL=1)
target_include_directories(GLISciTE PUBLIC "../../3rdParty/lua/src")
target_include_directories(GLISciTE PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/lua")
set_target_properties(GLISciTE PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFEST:NO")
endif()

if (WIN32)
target_link_libraries(GLISciTE scintilla wxstedit glslang Comctl32 Imm32 Rpcrt4 liblua
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/expatd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/jpegd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/pngd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/regexd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/tiffd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/wxmswd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/wxWidgets/lib/zlibd.lib
)
else()
target_link_libraries(GLISciTE scintilla wxstedit glslang lua)
endif()

add_custom_command(TARGET GLISciTE
	POST_BUILD COMMAND
	"mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\..\\Plugins\\GLShaderEdit\\scite\\win32\\SciTE.exe.manifest\" -outputresource:\"${CMAKE_CURRENT_BINARY_DIR}\\Debug\\GLISciTE.exe\"\;\#1
	COMMENT "Adding custom manifest file...")

