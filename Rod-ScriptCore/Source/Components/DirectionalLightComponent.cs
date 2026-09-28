using Rod.Internal;

namespace Rod;

public sealed class DirectionalLightComponent : Component
{
    public Vector3 Direction
    {
        get => NativeApi.DirectionalLightGetDirection(Entity.ID);
        set => NativeApi.DirectionalLightSetDirection(Entity.ID, value);
    }

    public Vector3 Color
    {
        get => NativeApi.DirectionalLightGetColor(Entity.ID);
        set => NativeApi.DirectionalLightSetColor(Entity.ID, value);
    }

    public float Intensity
    {
        get => NativeApi.DirectionalLightGetIntensity(Entity.ID);
        set => NativeApi.DirectionalLightSetIntensity(Entity.ID, value);
    }
}
