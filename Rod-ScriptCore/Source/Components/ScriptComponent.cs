using Rod.Internal;

namespace Rod;

public sealed class ScriptComponent : Component
{
    public string ClassName
    {
        get => NativeApi.ScriptGetClassName(Entity.ID);
        set => NativeApi.ScriptSetClassName(Entity.ID, value);
    }
}
