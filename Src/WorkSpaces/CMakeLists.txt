#Copyright (c) 2010 Intel Coprporation
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

enable_language(ASM_MASM)

project(GLIntercept)

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

include_directories(
	..
	../Common
	../MainLib
	../GLIConfig
	../../3rdParty/Corona-1.0.2-Static/src
	../../3rdParty/Corona-1.0.2-Static/src/jpeg-6b
	../../3rdParty/Corona-1.0.2-Static/src/libpng-1.2.1
	../../3rdParty/Corona-1.0.2-Static/src/libungif-4.1.0
	../../3rdParty/Corona-1.0.2-Static/src/zlib-1.1.4)

if (WIN32)
add_definitions(-DGLI_BUILD_WINDOWS)
else()
add_definitions(-DGLI_BUILD_LINUX -Wno-write-strings)
endif()
add_library(GLIntercept SHARED ${COMMON_SOURCES} ${MAINLIB_SOURCES} ${CORONA_SOURCES} ${GENERATED_SOURCES})

if (NOT WIN32)
set(corona_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/Corona-1.0.2-Static")
target_link_libraries(GLIntercept png pthread "${corona_dir}/src/.libs/libcorona.a")

#build corona
add_custom_command(OUTPUT "${corona_dir}/src/.libs/libcorona.a"
  COMMAND sh bootstrap
  COMMAND sh configure
  COMMAND make clean  #libcorona.a alone doesnt seem to trigger build for corona project
  COMMAND make -j 4
  WORKING_DIRECTORY ${corona_dir}
  COMMENT "Building libcorona")
add_custom_target(corona DEPENDS "${corona_dir}/src/.libs/libcorona.a")
add_dependencies(GLIntercept corona)
else()
target_link_libraries(GLIntercept DbgHelp)
endif()

