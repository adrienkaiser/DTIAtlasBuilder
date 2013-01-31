#include "itkTestMain.h"

#ifdef WIN32
#define MODULE_IMPORT// __declspec(dllimport) // "dllimport" -> Dll = Dynamic library on windows -> DTIAtlasBuilderLib is static, so just set MODULE_IMPORT to nothing: Fails to link (linkage error) if dllimport because library is static and tries to link as dynamic
#else
#define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int ModuleEntryPoint(int, char * []); // " extern "C" " so the function is linked in the lib like in C language i.e. the name is kept as is and not modified like in C++

void RegisterTests()
{
  StringToTestFunctionMap["ModuleEntryPoint"] = ModuleEntryPoint;
}
