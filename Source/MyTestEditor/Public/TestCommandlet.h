#pragma once 

#include "Commandlets\Commandlet.h"

#include "TestCommandlet.generated.h"

//G:\workspace\UnrealEngine\Game\FuGuang\Plugins\EngineInside\Source\MyTest\Private\TestCommandlet.h
// MyTest.TestCommandlet
UCLASS(config = Editor)
class UTestCommandlet
	: public UCommandlet
{
	GENERATED_BODY()
public:
	virtual int32 Main(const FString& CmdLineParams) override;
};