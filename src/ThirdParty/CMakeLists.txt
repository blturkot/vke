cmake_minimum_required(VERSION 2.6)

project(ThirdParty)

add_subdirectory(glslang)
add_subdirectory(DevIL/DevIL/src-IL)
add_subdirectory(DevIL/DevIL/src-ILU)
add_subdirectory(DevIL/DevIL/src-ILUT)

set_target_properties(IL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../../lib)
set_target_properties(IL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../../lib)
set_target_properties(ILU PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../../lib)
set_target_properties(ILU PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../../lib)
set_target_properties(ILUT PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../../lib)
set_target_properties(ILUT PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../../lib)

set_target_properties(glslang PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../lib)
set_target_properties(glslang PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../lib)
set_target_properties(HLSL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../lib)
set_target_properties(HLSL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../lib)
set_target_properties(SPIRV PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../lib)
set_target_properties(SPIRV PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../lib)
set_target_properties(OGLCompiler PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../lib)
set_target_properties(OGLCompiler PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../lib)
set_target_properties(OSDependent PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../../../../../../lib)
set_target_properties(OSDependent PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../../../lib)

set_target_properties(zlib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../../../../../../lib)