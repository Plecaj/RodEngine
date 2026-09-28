namespace Rod;

public abstract class ScriptBehaviour
{
    public Entity Entity { get; internal set; }

    protected virtual void OnCreate() { }
    protected virtual void OnDestroy() { }
    protected virtual void OnUpdate(float timestep) { }

    internal void InvokeOnCreate()
    {
        OnCreate();
    }

    internal void InvokeOnDestroy()
    {
        OnDestroy();
    }

    internal void InvokeOnUpdate(float timestep)
    {
        OnUpdate(timestep);
    }
}
