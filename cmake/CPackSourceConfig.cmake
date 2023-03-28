
# generator
set(CPACK_GENERATOR "TXZ")

# source generator
set(CPACK_SOURCE_GENERATOR "TXZ")
set(CPACK_SOURCE_IGNORE_FILES
  /\\.git.*
  /build.*/
  /\\.vs.*/
  \\.swp
)
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_THREADS YES)

if(CPack_CMake_INCLUDED)
include(CPack)
endif()