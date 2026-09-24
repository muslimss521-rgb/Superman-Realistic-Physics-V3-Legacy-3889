using UnityEngine;

public class FreezeBreath : MonoBehaviour
{
    public Transform mouthPoint;      // Точка у рта
    public float freezeRange = 15f;    // Дальность дыхания
    public float coneAngle = 30f;      // Угол конуса заморозки
    public float freezePower = 2f;     // Как быстро замерзает объект

    void Update()
    {
        if (Input.GetKey(KeyCode.Mouse1)) // Правая кнопка мыши
        {
            ApplyFreezeBreath();
        }
    }

    void ApplyFreezeBreath()
    {
        Collider[] affectedObjects = Physics.OverlapSphere(mouthPoint.position, freezeRange);

        foreach (var col in affectedObjects)
        {
            Vector3 dirToTarget = (col.transform.position - mouthPoint.position).normalized;
            float angle = Vector3.Angle(mouthPoint.forward, dirToTarget);

            if (angle < coneAngle)
            {
                Rigidbody rb = col.GetComponent<Rigidbody>();
                if (rb != null)
                {
                    rb.velocity = Vector3.Lerp(rb.velocity, Vector3.zero, freezePower * Time.deltaTime);
                    rb.angularVelocity = Vector3.Lerp(rb.angularVelocity, Vector3.zero, freezePower * Time.deltaTime);
                }
            }
        }
    }
}