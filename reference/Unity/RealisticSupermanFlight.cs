using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class RealisticSupermanFlight : MonoBehaviour
{
    [Header("Настройки скорости и сил")]
    public float flyForce = 30f;          // Сила тяги вперед
    public float maxSpeed = 50f;          // Максимальная базовая скорость
    public float boostMultiplier = 2.5f;   // Множитель супер-ускорения (Shift)
    
    [Header("Управление и маневренность")]
    public float turnSpeed = 3f;          // Скорость поворота (маневренность)
    public float airDrag = 1f;            // Сопротивление воздуха при полете
    public float stopDrag = 5f;           // Быстрое торможение, если отпустить газ

    [Header("Визуальный наклон (Крен)")]
    public float leanAmount = 35f;        // Максимальный угол наклона на виражах
    public float leanSpeed = 5f;          // Скорость наклона тела

    private Rigidbody rb;
    private float currentLean = 0f;

    void Start()
    {
        rb = GetComponent<Rigidbody>();
        rb.useGravity = true; // Физический полет борется с гравитацией
    }

    void FixedUpdate()
    {
        float forwardInput = Input.GetAxis("Vertical"); // W / S (Тяга вперед)
        float InputX = Input.GetAxis("Horizontal");    // A / D (Повороты/Смещение)
        bool isBoosting = Input.GetKey(KeyCode.LeftShift);

        Transform cam = Camera.main.transform;
        Vector3 flightDirection = cam.forward;

        // Сопротивление воздуха
        rb.drag = (forwardInput != 0 || InputX != 0) ? airDrag : stopDrag;

        // Физическая тяга вперед
        if (forwardInput > 0)
        {
            float currentMaxSpeed = isBoosting ? maxSpeed * boostMultiplier : maxSpeed;
            if (rb.velocity.magnitude < currentMaxSpeed)
            {
                rb.AddForce(flightDirection * flyForce * forwardInput, ForceMode.Acceleration);
            }
        }

        // Подъемная сила для компенсации гравитации в движении
        if (forwardInput > 0 && rb.velocity.magnitude > 5f)
        {
            rb.AddForce(Vector3.up * Mathf.Abs(Physics.gravity.y) * rb.mass * 0.9f, ForceMode.Force);
        }

        // Реалистичные наклоны (Крен)
        if (rb.velocity.magnitude > 1f)
        {
            Quaternion lookRot = Quaternion.LookRotation(rb.velocity.normalized);
            float targetLean = -InputX * leanAmount;
            currentLean = Mathf.Lerp(currentLean, targetLean, leanSpeed * Time.deltaTime);

            Quaternion finalRotation = lookRot * Quaternion.Euler(0, 0, currentLean);
            rb.MoveRotation(Quaternion.Slerp(transform.rotation, finalRotation, turnSpeed * Time.deltaTime));
        }
    }
}