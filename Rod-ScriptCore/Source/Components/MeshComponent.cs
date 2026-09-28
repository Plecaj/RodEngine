using Rod.Internal;

namespace Rod;

public sealed class MeshComponent : Component
{
    public string Path
    {
        get => NativeApi.MeshGetPath(Entity.ID);
        set => NativeApi.MeshSetPath(Entity.ID, value);
    }

    public Vector4 Albedo
    {
        get => NativeApi.MeshGetAlbedo(Entity.ID);
        set => NativeApi.MeshSetAlbedo(Entity.ID, value);
    }

    public Vector3 Emissive
    {
        get => NativeApi.MeshGetEmissive(Entity.ID);
        set => NativeApi.MeshSetEmissive(Entity.ID, value);
    }

    public float Roughness
    {
        get => NativeApi.MeshGetRoughness(Entity.ID);
        set => NativeApi.MeshSetRoughness(Entity.ID, value);
    }

    public float Metallic
    {
        get => NativeApi.MeshGetMetallic(Entity.ID);
        set => NativeApi.MeshSetMetallic(Entity.ID, value);
    }
}
