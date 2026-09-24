using UnityEngine;

[RequireComponent(typeof(RealisticSupermanFlight))]
[RequireComponent(typeof(HeatVision))]
[RequireComponent(typeof(FreezeBreath))]
[RequireComponent(typeof(XRayVision))]
[RequireComponent(typeof(SupermanLanding))]
public class SupermanAbilitiesManager : MonoBehaviour
{
    // Этот компонент служит для централизованного контроля способностей.
    // Вы можете использовать его для активации или деактивации интерфейса (UI) 
    // или блокировки способностей (например, запретить тепловое зрение во время рентгена).
    
    void Start()
    {
        Debug.Log("Superman Physical Abilities System Initialized.");
    }
}