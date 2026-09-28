namespace Rod;

public sealed class TagComponent : Component
{
    public string Tag
    {
        get => Entity.Name;
        set => Entity.SetName(value);
    }
}
