using Rod;

namespace Game;

public sealed class Rotator : ScriptBehaviour
{
    public float Speed = 1.0f;

    protected override void OnUpdate(float timestep)
    {
        TransformComponent transform = Entity.GetComponent<TransformComponent>();
        transform.Rotation += new Vector3(0.0f, Speed * timestep, 0.0f);
    }
}
