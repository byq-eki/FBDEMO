// Copyright byq-eki. All Rights Reserved.

#include "OpenHarmonyBridgeModule.h"
#include "Modules/ModuleManager.h"
#include "Async/TaskGraphInterfaces.h"

// ---------------------------------------------------------------------------
// Platform guard: the NAPI declarations only exist on OpenHarmony.
// On other platforms the module compiles but all bridge calls are no-ops so
// that game code can reference the module unconditionally.
// ---------------------------------------------------------------------------
#if PLATFORM_OPENHARMONY
// Forward-declare the C functions exported by napi_init.cpp
// (compiled into libue_harmony_bridge.so on the device).
extern "C"
{
    void UEHarmonyBridge_RegisterCallback(
        void (*callback)(const char *message, int length));
    void UEHarmonyBridge_NotifyArkTS(const char *message, int length);
}
#endif // PLATFORM_OPENHARMONY

// Static pointer used by the plain-C callback to reach the module instance.
// A plain function pointer cannot capture a `this` pointer, so we store the
// active module here when it starts up and clear it on shutdown.
static FOpenHarmonyBridgeModule *GOpenHarmonyBridgeModule = nullptr;

// ---------------------------------------------------------------------------
// Module lifecycle
// ---------------------------------------------------------------------------

void FOpenHarmonyBridgeModule::StartupModule()
{
    GOpenHarmonyBridgeModule = this;

#if PLATFORM_OPENHARMONY
    // Register the C callback so that whenever ArkTS calls bridge.sendToUE()
    // (napi_init.cpp: SendToUE → g_ueCallback) the message reaches UE game
    // code on the Game Thread.
    //
    // NOTE: plain function pointer — no lambda capture allowed.
    UEHarmonyBridge_RegisterCallback([](const char *message, int /*length*/)
    {
        if (GOpenHarmonyBridgeModule == nullptr) return;

        // Marshal to FString and dispatch to the Game Thread.
        FString MessageStr = FString(UTF8_TO_TCHAR(message));

        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [MessageStr]()
            {
                // Re-check inside the lambda: ShutdownModule() clears the pointer
                // on the Game Thread, so this check and the call below are
                // effectively atomic from the Game Thread's perspective.
                if (GOpenHarmonyBridgeModule)
                {
                    GOpenHarmonyBridgeModule->DispatchMessageToHandler(MessageStr);
                }
            },
            TStatId{},
            nullptr,
            ENamedThreads::GameThread);
    });
#endif // PLATFORM_OPENHARMONY
}

void FOpenHarmonyBridgeModule::ShutdownModule()
{
#if PLATFORM_OPENHARMONY
    // Deregister the callback to avoid dangling references after module unload.
    UEHarmonyBridge_RegisterCallback(nullptr);
#endif
    GOpenHarmonyBridgeModule = nullptr;
}

// ---------------------------------------------------------------------------
// IOpenHarmonyBridgeModule interface
// ---------------------------------------------------------------------------

void FOpenHarmonyBridgeModule::DispatchMessageToHandler(const FString &Message)
{
    if (MessageHandler)
    {
        MessageHandler(Message);
    }
}

void FOpenHarmonyBridgeModule::SetMessageHandler(
    TFunction<void(const FString &Message)> Handler)
{
    MessageHandler = MoveTemp(Handler);
}

void FOpenHarmonyBridgeModule::CallArkTSFunction(
    const FString &EventName, const FString &Payload)
{
#if PLATFORM_OPENHARMONY
    // Compose a simple JSON envelope:  {"event":"<EventName>","data":<Payload>}
    FString Envelope = FString::Printf(
        TEXT("{\"event\":\"%s\",\"data\":%s}"), *EventName, *Payload);

    // Convert to UTF-8 for the C API.
    FTCHARToUTF8 Utf8(*Envelope);
    UEHarmonyBridge_NotifyArkTS(Utf8.Get(), Utf8.Length());
#else
    UE_LOG(LogTemp, Warning,
        TEXT("OpenHarmonyBridge: CallArkTSFunction called on non-HarmonyOS platform "
             "(event=%s). No-op."),
        *EventName);
#endif
}

// ---------------------------------------------------------------------------
// Module registration
// ---------------------------------------------------------------------------
IMPLEMENT_MODULE(FOpenHarmonyBridgeModule, OpenHarmonyBridge)
