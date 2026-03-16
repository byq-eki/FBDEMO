/**
 * Index.ets — HarmonyOS ArkTS entry page
 *
 * Demonstrates how to communicate with Unreal Engine through the NAPI bridge
 * (libue_harmony_bridge.so).
 *
 * Comparison:
 *   团结引擎 (Unity)  → uses OpenHarmonyJSObject to call JS from native
 *   Unreal Engine    → uses NAPI (napi_threadsafe_function) to call ArkTS
 *                       from native C++, and napi_call_function for the
 *                       reverse direction (ArkTS → UE)
 */

// Import the native module compiled from napi_init.cpp
import bridge from 'libue_harmony_bridge.so';

@Entry
@Component
struct Index {
  @State messageFromUE: string = '(waiting for Unreal Engine…)';
  @State inputMessage: string = '';

  aboutToAppear() {
    // Register the ArkTS callback that Unreal Engine will invoke
    // from its native C++ thread via UEHarmonyBridge_NotifyArkTS().
    bridge.receiveFromUE((message: string) => {
      this.messageFromUE = message;
    });
  }

  build() {
    Column({ space: 16 }) {
      Text('Unreal Engine ↔ HarmonyOS Bridge Demo')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      // ── UE → ArkTS ──────────────────────────────────────────────────────
      Text('Message from Unreal Engine:')
        .fontSize(14)
        .fontColor(Color.Gray)
      Text(this.messageFromUE)
        .fontSize(16)
        .border({ width: 1, color: Color.Blue })
        .padding(8)
        .width('90%')

      Divider()

      // ── ArkTS → UE ──────────────────────────────────────────────────────
      Text('Send message to Unreal Engine:')
        .fontSize(14)
        .fontColor(Color.Gray)
      TextInput({ placeholder: 'Type a message…', text: this.inputMessage })
        .width('90%')
        .onChange((value: string) => {
          this.inputMessage = value;
        })
      Button('Send to UE')
        .width('90%')
        .onClick(() => {
          // Calls SendToUE() in napi_init.cpp which forwards the message
          // to the registered C++ callback inside Unreal Engine.
          bridge.sendToUE(this.inputMessage);
          this.inputMessage = '';
        })
    }
    .width('100%')
    .height('100%')
    .padding(24)
    .justifyContent(FlexAlign.Center)
  }
}
