using System.Runtime.InteropServices;

namespace Rod.Internal;

internal static unsafe class NativeApi
{
    private static readonly Dictionary<string, IntPtr> s_Functions = new();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void LogMessageFn(int level, [MarshalAs(UnmanagedType.LPUTF8Str)] string message);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] [return: MarshalAs(UnmanagedType.I1)] private delegate bool InputIsKeyPressedFn(int keycode);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] [return: MarshalAs(UnmanagedType.I1)] private delegate bool InputIsMouseButtonPressedFn(int button);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate Vector2 InputGetMousePositionFn();
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate ulong EntityCreateFn([MarshalAs(UnmanagedType.LPUTF8Str)] string name);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void EntityDestroyFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate ulong EntityFindByNameFn([MarshalAs(UnmanagedType.LPUTF8Str)] string name);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate IntPtr EntityGetNameFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void EntitySetNameFn(ulong entityID, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] [return: MarshalAs(UnmanagedType.I1)] private delegate bool EntityHasComponentFn(ulong entityID, int componentType);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void EntityAddComponentFn(ulong entityID, int componentType);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void EntityRemoveComponentFn(ulong entityID, int componentType);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] [return: MarshalAs(UnmanagedType.I1)] private delegate bool GetBoolFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetBoolFn(ulong entityID, [MarshalAs(UnmanagedType.I1)] bool value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate int GetIntFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetIntFn(ulong entityID, int value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate Vector3 TransformGetVector3Fn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void TransformSetVector3Fn(ulong entityID, Vector3 value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate Vector4 GetVector4Fn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetVector4Fn(ulong entityID, Vector4 value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate float GetFloatFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetFloatFn(ulong entityID, float value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate IntPtr GetStringFn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetStringFn(ulong entityID, [MarshalAs(UnmanagedType.LPUTF8Str)] string value);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate Vector3 GetVector3Fn(ulong entityID);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate void SetVector3Fn(ulong entityID, Vector3 value);

    public static void Register(string name, IntPtr function)
    {
        s_Functions[name] = function;
    }

    public static void LogMessage(int level, string message)
    {
        Get<LogMessageFn>("Log.Message")(level, message);
    }

    public static bool InputIsKeyPressed(int keycode) => Get<InputIsKeyPressedFn>("Input.IsKeyPressed")(keycode);
    public static bool InputIsMouseButtonPressed(int button) => Get<InputIsMouseButtonPressedFn>("Input.IsMouseButtonPressed")(button);
    public static Vector2 InputGetMousePosition() => Get<InputGetMousePositionFn>("Input.GetMousePosition")();

    public static ulong EntityCreate(string name) => Get<EntityCreateFn>("Entity.Create")(name);
    public static void EntityDestroy(ulong entityID) => Get<EntityDestroyFn>("Entity.Destroy")(entityID);
    public static ulong EntityFindByName(string name) => Get<EntityFindByNameFn>("Entity.FindByName")(name);
    public static string EntityGetName(ulong entityID) => PtrToStringAndFree(Get<EntityGetNameFn>("Entity.GetName")(entityID));
    public static void EntitySetName(ulong entityID, string name) => Get<EntitySetNameFn>("Entity.SetName")(entityID, name);
    public static bool EntityHasComponent(ulong entityID, int componentType) => Get<EntityHasComponentFn>("Entity.HasComponent")(entityID, componentType);
    public static void EntityAddComponent(ulong entityID, int componentType) => Get<EntityAddComponentFn>("Entity.AddComponent")(entityID, componentType);
    public static void EntityRemoveComponent(ulong entityID, int componentType) => Get<EntityRemoveComponentFn>("Entity.RemoveComponent")(entityID, componentType);

    public static bool CameraGetPrimary(ulong entityID) => Get<GetBoolFn>("Camera.GetPrimary")(entityID);
    public static void CameraSetPrimary(ulong entityID, bool value) => Get<SetBoolFn>("Camera.SetPrimary")(entityID, value);
    public static bool CameraGetFixedAspectRatio(ulong entityID) => Get<GetBoolFn>("Camera.GetFixedAspectRatio")(entityID);
    public static void CameraSetFixedAspectRatio(ulong entityID, bool value) => Get<SetBoolFn>("Camera.SetFixedAspectRatio")(entityID, value);
    public static int CameraGetProjectionType(ulong entityID) => Get<GetIntFn>("Camera.GetProjectionType")(entityID);
    public static void CameraSetProjectionType(ulong entityID, int value) => Get<SetIntFn>("Camera.SetProjectionType")(entityID, value);
    public static float CameraGetPerspectiveVerticalFOV(ulong entityID) => Get<GetFloatFn>("Camera.GetPerspectiveVerticalFOV")(entityID);
    public static void CameraSetPerspectiveVerticalFOV(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetPerspectiveVerticalFOV")(entityID, value);
    public static float CameraGetPerspectiveNearClip(ulong entityID) => Get<GetFloatFn>("Camera.GetPerspectiveNearClip")(entityID);
    public static void CameraSetPerspectiveNearClip(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetPerspectiveNearClip")(entityID, value);
    public static float CameraGetPerspectiveFarClip(ulong entityID) => Get<GetFloatFn>("Camera.GetPerspectiveFarClip")(entityID);
    public static void CameraSetPerspectiveFarClip(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetPerspectiveFarClip")(entityID, value);
    public static float CameraGetOrthographicSize(ulong entityID) => Get<GetFloatFn>("Camera.GetOrthographicSize")(entityID);
    public static void CameraSetOrthographicSize(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetOrthographicSize")(entityID, value);
    public static float CameraGetOrthographicNearClip(ulong entityID) => Get<GetFloatFn>("Camera.GetOrthographicNearClip")(entityID);
    public static void CameraSetOrthographicNearClip(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetOrthographicNearClip")(entityID, value);
    public static float CameraGetOrthographicFarClip(ulong entityID) => Get<GetFloatFn>("Camera.GetOrthographicFarClip")(entityID);
    public static void CameraSetOrthographicFarClip(ulong entityID, float value) => Get<SetFloatFn>("Camera.SetOrthographicFarClip")(entityID, value);

    public static Vector3 TransformGetTranslation(ulong entityID) => Get<TransformGetVector3Fn>("Transform.GetTranslation")(entityID);
    public static void TransformSetTranslation(ulong entityID, Vector3 value) => Get<TransformSetVector3Fn>("Transform.SetTranslation")(entityID, value);
    public static Vector3 TransformGetRotation(ulong entityID) => Get<TransformGetVector3Fn>("Transform.GetRotation")(entityID);
    public static void TransformSetRotation(ulong entityID, Vector3 value) => Get<TransformSetVector3Fn>("Transform.SetRotation")(entityID, value);
    public static Vector3 TransformGetScale(ulong entityID) => Get<TransformGetVector3Fn>("Transform.GetScale")(entityID);
    public static void TransformSetScale(ulong entityID, Vector3 value) => Get<TransformSetVector3Fn>("Transform.SetScale")(entityID, value);

    public static Vector4 SpriteRendererGetColor(ulong entityID) => Get<GetVector4Fn>("SpriteRenderer.GetColor")(entityID);
    public static void SpriteRendererSetColor(ulong entityID, Vector4 value) => Get<SetVector4Fn>("SpriteRenderer.SetColor")(entityID, value);
    public static float SpriteRendererGetTilingFactor(ulong entityID) => Get<GetFloatFn>("SpriteRenderer.GetTilingFactor")(entityID);
    public static void SpriteRendererSetTilingFactor(ulong entityID, float value) => Get<SetFloatFn>("SpriteRenderer.SetTilingFactor")(entityID, value);
    public static string SpriteRendererGetTexturePath(ulong entityID) => PtrToStringAndFree(Get<GetStringFn>("SpriteRenderer.GetTexturePath")(entityID));
    public static void SpriteRendererSetTexturePath(ulong entityID, string value) => Get<SetStringFn>("SpriteRenderer.SetTexturePath")(entityID, value);

    public static string MeshGetPath(ulong entityID) => PtrToStringAndFree(Get<GetStringFn>("Mesh.GetPath")(entityID));
    public static void MeshSetPath(ulong entityID, string value) => Get<SetStringFn>("Mesh.SetPath")(entityID, value);
    public static Vector4 MeshGetAlbedo(ulong entityID) => Get<GetVector4Fn>("Mesh.GetAlbedo")(entityID);
    public static void MeshSetAlbedo(ulong entityID, Vector4 value) => Get<SetVector4Fn>("Mesh.SetAlbedo")(entityID, value);
    public static Vector3 MeshGetEmissive(ulong entityID) => Get<GetVector3Fn>("Mesh.GetEmissive")(entityID);
    public static void MeshSetEmissive(ulong entityID, Vector3 value) => Get<SetVector3Fn>("Mesh.SetEmissive")(entityID, value);
    public static float MeshGetRoughness(ulong entityID) => Get<GetFloatFn>("Mesh.GetRoughness")(entityID);
    public static void MeshSetRoughness(ulong entityID, float value) => Get<SetFloatFn>("Mesh.SetRoughness")(entityID, value);
    public static float MeshGetMetallic(ulong entityID) => Get<GetFloatFn>("Mesh.GetMetallic")(entityID);
    public static void MeshSetMetallic(ulong entityID, float value) => Get<SetFloatFn>("Mesh.SetMetallic")(entityID, value);

    public static Vector3 DirectionalLightGetDirection(ulong entityID) => Get<GetVector3Fn>("DirectionalLight.GetDirection")(entityID);
    public static void DirectionalLightSetDirection(ulong entityID, Vector3 value) => Get<SetVector3Fn>("DirectionalLight.SetDirection")(entityID, value);
    public static Vector3 DirectionalLightGetColor(ulong entityID) => Get<GetVector3Fn>("DirectionalLight.GetColor")(entityID);
    public static void DirectionalLightSetColor(ulong entityID, Vector3 value) => Get<SetVector3Fn>("DirectionalLight.SetColor")(entityID, value);
    public static float DirectionalLightGetIntensity(ulong entityID) => Get<GetFloatFn>("DirectionalLight.GetIntensity")(entityID);
    public static void DirectionalLightSetIntensity(ulong entityID, float value) => Get<SetFloatFn>("DirectionalLight.SetIntensity")(entityID, value);

    public static string ScriptGetClassName(ulong entityID) => PtrToStringAndFree(Get<GetStringFn>("Script.GetClassName")(entityID));
    public static void ScriptSetClassName(ulong entityID, string value) => Get<SetStringFn>("Script.SetClassName")(entityID, value);

    private static T Get<T>(string name) where T : Delegate
    {
        return Marshal.GetDelegateForFunctionPointer<T>(s_Functions[name]);
    }

    private static string PtrToStringAndFree(IntPtr value)
    {
        if (value == IntPtr.Zero)
            return string.Empty;

        string result = Marshal.PtrToStringUTF8(value) ?? string.Empty;
        Marshal.FreeCoTaskMem(value);
        return result;
    }
}
