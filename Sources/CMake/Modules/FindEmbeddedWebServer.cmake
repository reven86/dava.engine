
if (EMBEDDED_WEB_SERVER_FOUND)
    return ()
endif ()

set (EMBEDDED_WEB_SERVER_FOUND true)

include (GlobalVariables)

add_module_subdirectory( EmbeddedWebServer  "${DAVA_MODULES_DIR}/EmbeddedWebServer" )
