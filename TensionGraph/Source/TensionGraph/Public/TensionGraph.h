// Tension Graph - Joseph Chittock

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTensionGraphModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
