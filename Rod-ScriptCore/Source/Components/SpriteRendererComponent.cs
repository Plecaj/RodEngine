using Rod.Internal;

namespace Rod;

public sealed class SpriteRendererComponent : Component
{
    public Vector4 Color
    {
        get => NativeApi.SpriteRendererGetColor(Entity.ID);
        set => NativeApi.SpriteRendererSetColor(Entity.ID, value);
    }

    public float TilingFactor
    {
        get => NativeApi.SpriteRendererGetTilingFactor(Entity.ID);
        set => NativeApi.SpriteRendererSetTilingFactor(Entity.ID, value);
    }

    public string TexturePath
    {
        get => NativeApi.SpriteRendererGetTexturePath(Entity.ID);
        set => NativeApi.SpriteRendererSetTexturePath(Entity.ID, value);
    }
}
