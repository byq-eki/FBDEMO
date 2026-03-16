# CMakeLists.txt for the Unreal Engine ↔ HarmonyOS NAPI bridge
#
# Build with the OpenHarmony NDK toolchain:
#   cmake -DCMAKE_TOOLCHAIN_FILE=$OHOS_NDK/build/cmake/ohos.toolchain.cmake \
#         -DOHOS_ARCH=arm64-v8a \
#         -B build && cmake --build build

cmake_minimum_required(VERSION 3.10)  # 3.10+ is widely available in HarmonyOS NDK toolchains
project(ue_harmony_bridge)

# HarmonyOS NDK provides napi/native_api.h under:
#   $OHOS_NDK/sysroot/usr/include/
# The toolchain file sets this up automatically when using the official NDK.

add_library(ue_harmony_bridge SHARED napi_init.cpp)

target_compile_features(ue_harmony_bridge PRIVATE cxx_std_17)

target_link_libraries(
    ue_harmony_bridge
    libace_napi.z.so   # HarmonyOS NAPI runtime (provided by the OS)
    libc++_shared.so
)
