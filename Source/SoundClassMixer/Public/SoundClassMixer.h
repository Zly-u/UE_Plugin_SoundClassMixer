#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_CLASS(LogSoundClassMixerModule, Display, All);

class FSoundClassMixerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
