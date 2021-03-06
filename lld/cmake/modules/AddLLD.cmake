macro(add_lld_library name)
  add_llvm_library(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "lld libraries")
endmacro(add_lld_library)

macro(add_lld_executable name)
  add_llvm_executable(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "lld executables")
endmacro(add_lld_executable)

macro(add_lld_tool name)
  if (NOT LLD_BUILD_TOOLS)
    set(EXCLUDE_FROM_ALL ON)
  endif()

  add_lld_executable(${name} ${ARGN})

  if (LLD_BUILD_TOOLS)
    if(${name} IN_LIST LLVM_DISTRIBUTION_COMPONENTS OR
        NOT LLVM_DISTRIBUTION_COMPONENTS)
      set(export_to_lldtargets EXPORT lldTargets)
      set_property(GLOBAL PROPERTY LLD_HAS_EXPORTS True)
    endif()

    install(TARGETS ${name}
      ${export_to_lldtargets}
      RUNTIME DESTINATION bin
      COMPONENT ${name})

    if(NOT CMAKE_CONFIGURATION_TYPES)
      add_custom_target(install-${name}
        DEPENDS ${name}
        COMMAND "${CMAKE_COMMAND}"
        -DCMAKE_INSTALL_COMPONENT=${name}
        -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif()
    set_property(GLOBAL APPEND PROPERTY LLD_EXPORTS ${name})
  endif()
endmacro()

macro(add_lld_symlink name dest)
  add_llvm_tool_symlink(${name} ${dest} ALWAYS_GENERATE)
  # Always generate install targets
  llvm_install_symlink(${name} ${dest} ALWAYS_GENERATE)
endmacro()
