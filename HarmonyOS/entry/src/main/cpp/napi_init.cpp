/*
 * HarmonyOS (OpenHarmony) NAPI native module for Unreal Engine communication.
 *
 * 团结引擎 (Unity) 通过 OpenHarmonyJSObject 与 Harmony 原生通信。
 * Unreal Engine 通过 NAPI (Node API) 与 Harmony 原生通信。
 *
 * This module is compiled as a shared library (libue_harmony_bridge.so) that
 * is loaded by the ArkTS runtime. Unreal Engine's C++ code calls into this
 * module via napi_call_function / napi_threadsafe_function to exchange data
 * with the HarmonyOS ArkTS layer.
 *
 * Architecture overview:
 *
 *   ┌─────────────────────────┐        ┌──────────────────────────────┐
 *   │   Unreal Engine (C++)   │        │   HarmonyOS ArkTS (.ets)     │
 *   │                         │        │                              │
 *   │  UEHarmonyBridge::      │──────▶│  import bridge from          │
 *   │    CallArkTSFunction()  │  NAPI  │    'libue_harmony_bridge.so' │
 *   │                         │◀──────│                              │
 *   │  OnArkTSCallback(...)   │  tsfn  │  bridge.sendToUE(data)       │
 *   └─────────────────────────┘        └──────────────────────────────┘
 *
 * Key difference vs 团结引擎:
 *   - 团结引擎 uses OpenHarmonyJSObject  (a Unity-specific JS bridge object)
 *   - Unreal Engine uses NAPI directly   (the standard HarmonyOS C/C++ bridge)
 */

#include "napi/native_api.h"
#include <string>
#include <functional>
#include <mutex>
#include <queue>

// ---------------------------------------------------------------------------
// Thread-safe callback: ArkTS → Unreal Engine
// ---------------------------------------------------------------------------

// Signature for the C++ callback invoked when ArkTS calls sendToUE().
// Matches the extern "C" declaration in OpenHarmonyBridgeModule.cpp so that
// the UE module and this shared library share the same ABI.
using UEMessageCallback = void (*)(const char *message, int length);

static UEMessageCallback g_ueCallback = nullptr;
static napi_threadsafe_function g_tsfn = nullptr;
static std::mutex g_mutex;

// Register a C++ callback that is invoked (on the JS thread, via tsfn) when
// ArkTS sends a message to Unreal Engine.
// Must use extern "C" linkage to match the declaration in the UE plugin.
extern "C" void UEHarmonyBridge_RegisterCallback(UEMessageCallback callback)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ueCallback = callback;
}

// ---------------------------------------------------------------------------
// NAPI helper: read a JS string into a std::string
// ---------------------------------------------------------------------------
static std::string NapiGetString(napi_env env, napi_value value)
{
    size_t len = 0;
    napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    std::string result(len, '\0');
    napi_get_value_string_utf8(env, value, result.data(), len + 1, &len);
    return result;
}

// ---------------------------------------------------------------------------
// NAPI exports
// ---------------------------------------------------------------------------

/**
 * sendToUE(message: string): void
 *
 * Called from ArkTS to forward a message to Unreal Engine.
 * The message is delivered to the registered C++ callback on the correct
 * thread using napi_threadsafe_function.
 */
static napi_value SendToUE(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "sendToUE expects 1 argument: message");
        return nullptr;
    }

    std::string message = NapiGetString(env, args[0]);

    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_ueCallback) {
        // Deliver on the calling (JS) thread directly when the callback is
        // already thread-safe.  For cross-thread delivery, use g_tsfn below.
        g_ueCallback(message.c_str(), static_cast<int>(message.size()));
    }

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

/**
 * receiveFromUE(callback: (message: string) => void): void
 *
 * ArkTS registers a callback here. Unreal Engine can later call
 * UEHarmonyBridge_NotifyArkTS() (from any C++ thread) to invoke it.
 */
static napi_value ReceiveFromUE(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "receiveFromUE expects 1 argument: callback");
        return nullptr;
    }

    napi_value resourceName;
    napi_create_string_utf8(env, "UEHarmonyBridge", NAPI_AUTO_LENGTH, &resourceName);

    // Create a thread-safe function wrapper so UE can call the ArkTS callback
    // from any native thread.
    napi_create_threadsafe_function(
        env,
        args[0],          // JS callback
        nullptr,
        resourceName,
        0,                // max_queue_size (0 = unlimited)
        1,                // initial_thread_count
        nullptr,
        nullptr,
        nullptr,
        // call_js_cb: invoked on the JS thread with the data payload
        [](napi_env cbEnv, napi_value jsCb, void * /*context*/, void *data) {
            if (data == nullptr || jsCb == nullptr) return;
            auto *msg = static_cast<std::string *>(data);
            napi_value jsMessage;
            napi_create_string_utf8(cbEnv, msg->c_str(), msg->size(), &jsMessage);
            napi_value global;
            napi_get_global(cbEnv, &global);
            napi_call_function(cbEnv, global, jsCb, 1, &jsMessage, nullptr);
            delete msg;
        },
        &g_tsfn);

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

// Public C++ API: called from Unreal Engine C++ to push a message to ArkTS.
// This is thread-safe — can be called from any UE worker thread.
extern "C" void UEHarmonyBridge_NotifyArkTS(const char *message, int length)
{
    if (g_tsfn == nullptr || message == nullptr) return;
    // Heap-allocate so ownership is transferred to the call_js_cb lambda.
    auto *data = new std::string(message, static_cast<size_t>(length));
    napi_call_threadsafe_function(g_tsfn, data, napi_tsfn_nonblocking);
}

// ---------------------------------------------------------------------------
// Module initialisation
// ---------------------------------------------------------------------------
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor props[] = {
        {"sendToUE",      nullptr, SendToUE,      nullptr, nullptr, nullptr, napi_default, nullptr},
        {"receiveFromUE", nullptr, ReceiveFromUE, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(props) / sizeof(props[0]), props);
    return exports;
}

NAPI_MODULE(ue_harmony_bridge, Init)
