// Copyright byq-eki. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * IOpenHarmonyBridgeModule
 *
 * Public interface for the OpenHarmonyBridge Unreal Engine plugin.
 *
 * Background
 * ----------
 * On HarmonyOS, 团结引擎 (the Unity variant for China) communicates with
 * ArkTS native code via OpenHarmonyJSObject.  Unreal Engine achieves the same
 * result through NAPI (Node API), which is the standard C/C++ ↔ ArkTS interop
 * mechanism provided by the HarmonyOS NDK.
 *
 * Architecture
 * ------------
 *
 *   Unreal Engine (C++)                 HarmonyOS ArkTS (.ets)
 *   ──────────────────────              ───────────────────────────────
 *   CallArkTSFunction("key", "data") ──▶  bridge.sendToUE callback
 *                                         registered in Index.ets
 *
 *   OnNativeMessage callback         ◀──  bridge.sendToUE("data")
 *   (registered via SetMessageHandler)     called from ArkTS button / logic
 *
 * The native side is implemented in:
 *   HarmonyOS/entry/src/main/cpp/napi_init.cpp
 *
 * Usage (C++ game code)
 * ----------------------
 *   // 1. Get the module
 *   IOpenHarmonyBridgeModule& Bridge =
 *       FModuleManager::LoadModuleChecked<IOpenHarmonyBridgeModule>(
 *           "OpenHarmonyBridge");
 *
 *   // 2. Register a handler for messages coming FROM ArkTS
 *   Bridge.SetMessageHandler([](const FString& Message) {
 *       UE_LOG(LogTemp, Log, TEXT("From ArkTS: %s"), *Message);
 *   });
 *
 *   // 3. Send a message TO ArkTS
 *   Bridge.CallArkTSFunction(TEXT("myEvent"), TEXT("{\"score\":42}"));
 */
class OPENHARMONYBRIDGE_API IOpenHarmonyBridgeModule : public IModuleInterface
{
public:
    static IOpenHarmonyBridgeModule& Get()
    {
        return FModuleManager::LoadModuleChecked<IOpenHarmonyBridgeModule>("OpenHarmonyBridge");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("OpenHarmonyBridge");
    }

    /**
     * Register a C++ handler that is invoked whenever ArkTS calls
     * bridge.sendToUE(message) on the HarmonyOS side.
     *
     * @param Handler  Callable that receives the UTF-8 message string.
     *                 Invoked on the Game Thread.
     */
    virtual void SetMessageHandler(TFunction<void(const FString& Message)> Handler) = 0;

    /**
     * Send a message from Unreal Engine to the ArkTS layer.
     *
     * Internally this calls UEHarmonyBridge_NotifyArkTS() which uses a
     * napi_threadsafe_function to safely cross the thread boundary and invoke
     * the receiveFromUE callback registered in Index.ets.
     *
     * @param EventName  Logical event name (passed as a JSON key).
     * @param Payload    Arbitrary string payload (e.g. serialised JSON).
     */
    virtual void CallArkTSFunction(const FString& EventName, const FString& Payload) = 0;
};
