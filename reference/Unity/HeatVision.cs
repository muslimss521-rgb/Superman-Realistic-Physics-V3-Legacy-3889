using UnityEngine;

public class HeatVision : MonoBehaviour
{
    public Transform eyesPoint;        // Точка между глаз персонажа
    public float maxDistance = 100f;   // Дальность луча
    public float pushForce = 50f;      // Сила физического толчка луча
    public LineRenderer laserVisual;   // Визуальный лазер (Line Renderer)

    void Start() => laserVisual.enabled = false;

    void Update()
    {
        if (Input.GetKey(KeyCode.Mouse0)) // Левая кнопка мыши
        {
            ActivateHeatVision();
        }
        else
        {
            laserVisual.enabled = false;
        }
    }

    void ActivateHeatVision()
    {
        laserVisual.enabled = true;
        laserVisual.SetPosition(0, eyesPoint.position);

        RaycastHit hit;
        Vector3 targetPosition = eyesPoint.position + eyesPoint.forward * maxDistance;

        if (Physics.Raycast(eyesPoint.position, eyesPoint.forward, out hit, maxDistance))
        {
            targetPosition = hit.point;

            Rigidbody targetRb = hit.collider.GetComponent<Rigidbody>();
            if (targetRb != null)
            {
                Vector3 pushDirection = (hit.point - eyesPoint.position).normalized;
                targetRb.AddForceAtPosition(pushDirection * pushForce, hit.point, ForceMode.Force);
            }
        }

        laserVisual.SetPosition(1, targetPosition);
    }
}