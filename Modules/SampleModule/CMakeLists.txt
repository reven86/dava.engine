
set( NAME_MODULE SampleModule )                  # именуем модуль

set( MODULE_TYPE STATIC )                        # указываем тип модуля [INLINE, STATIC, DYNAMIC]

set( INCLUDES "." )                              # указываем путь поиска хидеров модуля для всего проекта
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )       # указываем путь поиска хидеров внутри модуля

set( MODULE_INITIALIZATION_CODE SampleModule.h ) # указываем хидер где объявляется модуль
set( HPP_FILES *.h )                             # 
set( CPP_FILES Private/*.cpp )                   # указываем где живут исходники

setup_main_module()                              # вызываем макрос формирования модуля
