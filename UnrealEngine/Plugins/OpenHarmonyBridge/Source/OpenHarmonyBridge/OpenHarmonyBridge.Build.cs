// Copyright byq-eki. All Rights Reserved.

using UnrealBuildTool;

public class OpenHarmonyBridge : ModuleRules
{
    public OpenHarmonyBridge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
        });

        if (Target.Platform == UnrealTargetPlatform.OpenHarmony)
        {
            // Link against the HarmonyOS NAPI runtime library.
            // libace_napi.z.so is provided by the OS and must NOT be packaged
            // with the app — it is loaded from the device at runtime.
            PublicSystemLibraries.Add("ace_napi.z");

            // Path to the OpenHarmony NDK headers supplied by the toolchain.
            // When building with the official HarmonyOS UE fork the NDK root
            // is available via OHOS_NDK_ROOT; adjust if needed.
            string OhosNdkRoot = System.Environment.GetEnvironmentVariable("OHOS_NDK_ROOT") ?? "";
            if (!string.IsNullOrEmpty(OhosNdkRoot))
            {
                PublicIncludePaths.Add(System.IO.Path.Combine(OhosNdkRoot, "sysroot", "usr", "include"));
            }
        }
    }
}
