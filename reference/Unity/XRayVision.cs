using UnityEngine;

public class XRayVision : MonoBehaviour
{
    public Camera mainCamera;
    public LayerMask defaultMask;    // Обычные слои
    public LayerMask xrayMask;       // Слои для рентгена
    
    private bool isXrayActive = false;

    void Start() => defaultMask = mainCamera.cullingMask;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.X)) // Клавиша X
        {
            isXrayActive = !isXrayActive;
            mainCamera.cullingMask = isXrayActive ? xrayMask : defaultMask;
        }
    }
}