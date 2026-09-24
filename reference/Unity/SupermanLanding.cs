using UnityEngine;

public class SupermanLanding : MonoBehaviour
{
    public float shockwaveRadius = 20f;
    public float shockwaveForce = 500f;
    private Rigidbody rb;
    private bool isFlying = false;

    void Start() => rb = GetComponent<Rigidbody>();

    void Update()
    {
        if (rb.velocity.y < -15f) 
        {
            isFlying = true; 
        }
    }

    void OnCollisionEnter(Collision collision)
    {
        if (isFlying)
        {
            float impactForce = Mathf.Abs(collision.relativeVelocity.y);
            if (impactForce > 15f)
            {
                TriggerGroundPound(collision.contacts.point, impactForce);
            }
            isFlying = false;
        }
    }

    void TriggerGroundPound(Vector3 point, float force)
    {
        Collider[] colliders = Physics.OverlapSphere(point, shockwaveRadius);
        foreach (Collider hit in colliders)
        {
            Rigidbody targetRb = hit.GetComponent<Rigidbody>();
            if (targetRb != null && targetRb != rb)
            {
                targetRb.AddExplosionForce(shockwaveForce * force, point, shockwaveRadius, 3.0f, ForceMode.Impulse);
            }
        }
    }
}