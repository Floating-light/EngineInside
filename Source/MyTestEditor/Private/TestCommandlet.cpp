#include "TestCommandlet.h"
#include "OPPropertyTraverse.h"

int32 UTestCommandlet::Main(const FString& CmdLineParams)
{
	FOPPropertyTranverse<FTextProperty>::TEST_DirectoryAsset();
	return int32();
}
