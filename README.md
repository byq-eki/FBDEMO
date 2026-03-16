# FBDEMO
fb sdk test demo

---

## Unreal Engine ↔ HarmonyOS (OpenHarmony) 通信方案

### 背景

| 引擎 | 与 Harmony 原生通信方式 |
|------|------------------------|
| **团结引擎（Unity）** | `OpenHarmonyJSObject`：由引擎内置，通过该对象调用 ArkTS 函数或接收 JS 回调 |
| **Unreal Engine** | **NAPI（Node API）**：HarmonyOS NDK 提供的标准 C/C++ ↔ ArkTS 互操作机制 |

### 架构说明

```
┌──────────────────────────┐           ┌──────────────────────────────────┐
│   Unreal Engine (C++)    │           │   HarmonyOS ArkTS (.ets)         │
│                          │           │                                  │
│  IOpenHarmonyBridgeModule│──────────▶│  bridge.receiveFromUE(callback)  │
│  ::CallArkTSFunction()   │   NAPI    │    → callback(message)           │
│                          │  tsfn     │                                  │
│  SetMessageHandler(fn)   │◀──────────│  bridge.sendToUE(message)        │
└──────────────────────────┘           └──────────────────────────────────┘
         ▲                                          ▲
         │ libue_harmony_bridge.so (共享库)          │
         └──────────────── NAPI module ─────────────┘
```

- **ArkTS → UE**：ArkTS 调用 `bridge.sendToUE(msg)`，NAPI 模块通过已注册的 C 回调
  (`UEHarmonyBridge_RegisterCallback`) 将消息传递到 UE 的 Game Thread。
- **UE → ArkTS**：UE 调用 `CallArkTSFunction(event, payload)`，NAPI 模块通过
  `napi_threadsafe_function` (`UEHarmonyBridge_NotifyArkTS`) 安全地跨线程调用
  ArkTS 层注册的 `receiveFromUE` 回调。

### 文件结构

```
HarmonyOS/
└── entry/src/main/
    ├── cpp/
    │   ├── napi_init.cpp      # NAPI 模块：注册 sendToUE / receiveFromUE
    │   └── CMakeLists.txt     # 编译为 libue_harmony_bridge.so
    └── ets/pages/
        └── Index.ets          # ArkTS 页面：导入并使用 bridge

UnrealEngine/
└── Plugins/OpenHarmonyBridge/
    ├── OpenHarmonyBridge.uplugin
    └── Source/OpenHarmonyBridge/
        ├── OpenHarmonyBridge.Build.cs
        ├── Public/
        │   └── IOpenHarmonyBridgeModule.h   # UE 插件公共接口
        └── Private/
            ├── OpenHarmonyBridgeModule.h
            └── OpenHarmonyBridgeModule.cpp  # 模块实现
```

### 快速上手

#### 1. 编译 NAPI 共享库（HarmonyOS 侧）

```bash
# 需要安装 OpenHarmony NDK
export OHOS_NDK_ROOT=/path/to/ohos-ndk

cmake -DCMAKE_TOOLCHAIN_FILE=$OHOS_NDK_ROOT/build/cmake/ohos.toolchain.cmake \
      -DOHOS_ARCH=arm64-v8a \
      -B build/harmony \
      HarmonyOS/entry/src/main/cpp

cmake --build build/harmony
# 产物：libue_harmony_bridge.so
```

#### 2. 集成 UE 插件

将 `UnrealEngine/Plugins/OpenHarmonyBridge` 复制到你的 UE 项目的 `Plugins/` 目录，
然后在 `.uproject` 中启用该插件：

```json
{
  "Plugins": [
    { "Name": "OpenHarmonyBridge", "Enabled": true }
  ]
}
```

#### 3. UE C++ 使用示例

```cpp
#include "IOpenHarmonyBridgeModule.h"

// 注册接收 ArkTS 消息的回调
IOpenHarmonyBridgeModule::Get().SetMessageHandler(
    [](const FString& Message)
    {
        UE_LOG(LogTemp, Log, TEXT("从 ArkTS 收到: %s"), *Message);
    });

// 向 ArkTS 发送消息
IOpenHarmonyBridgeModule::Get().CallArkTSFunction(
    TEXT("updateScore"),
    TEXT("{\"score\":100}"));
```

#### 4. ArkTS 使用示例（Index.ets）

```typescript
import bridge from 'libue_harmony_bridge.so';

// 注册接收 UE 消息的回调
bridge.receiveFromUE((message: string) => {
  console.log('从 UE 收到:', message);
});

// 向 UE 发送消息
bridge.sendToUE('{"action":"pause"}');
```

---

## iOS Facebook SDK Demo

Original iOS demo using Facebook SDK (login / share).

