// Copyright byq-eki. All Rights Reserved.
#pragma once

#include "IOpenHarmonyBridgeModule.h"

/**
 * Concrete UE module class — do not use directly.
 * Access the module through IOpenHarmonyBridgeModule::Get().
 */
class FOpenHarmonyBridgeModule : public IOpenHarmonyBridgeModule
{
public:
    // IModuleInterface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // IOpenHarmonyBridgeModule
    virtual void SetMessageHandler(TFunction<void(const FString& Message)> Handler) override;
    virtual void CallArkTSFunction(const FString& EventName, const FString& Payload) override;

    /** Called from the static C callback to deliver a message on the Game Thread. */
    void DispatchMessageToHandler(const FString& Message);

private:
    TFunction<void(const FString& Message)> MessageHandler;
};
