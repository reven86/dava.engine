function (generate_clang_complete_config)
   set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/.clang_complete")
   message ("writing clang_complete config ${config_file}")
   file (REMOVE "${config_file}")

   # includes
   get_directory_property (project_includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} INCLUDE_DIRECTORIES)
   foreach (item ${project_includes})
      file (APPEND "${config_file}" "-I${item}\n")
   endforeach ()

   # defines
   get_directory_property (project_defines DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
   foreach (item ${project_defines})
      file (APPEND "${config_file}" "-D${item}\n")
   endforeach ()

endfunction (generate_clang_complete_config)

