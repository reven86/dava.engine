Physics module uses NVidia physx library for collision detection and physics simulation

Integration problems:
- physx library use C limit macroses. To compile physx or project that use physx on android there should be defined \__STDC_LIMIT_MACROS
  before stdint.h will be included (on Window and Apple's platforms limit macroses defined with out #ifdef). As DAVA includes cstdint -> stdint.h in
  Base/BaseTypes.h with out \__STDC_LIMIT_MACROS we can not define \__STDC_LIMIT_MACROS localy (stdint.h is a standard header with
  include guard) so we have to define \__STDC_LIMIT_MACROS globaly as compiler flag.
  This can produce comiler warning __'\__STDC_LIMIT_MACROS' : macro redefinition__ in some thirdparty libraries.
