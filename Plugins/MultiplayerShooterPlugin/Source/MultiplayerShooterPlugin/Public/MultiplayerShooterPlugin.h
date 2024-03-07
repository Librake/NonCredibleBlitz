// Copyright 2022 Vster

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define SURFACE_Head SurfaceType1
#define SURFACE_Chest SurfaceType2
#define SURFACE_Torso SurfaceType3
#define SURFACE_Leg SurfaceType4

class FMultiplayerShooterPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};