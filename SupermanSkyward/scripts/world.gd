extends Node3D

func _ready() -> void:
    var sun := DirectionalLight3D.new()
    sun.rotation_degrees = Vector3(-55, -35, 0)
    sun.light_energy = 1.4
    sun.shadow_enabled = true
    add_child(sun)
    var world_env := WorldEnvironment.new()
    var env := Environment.new()
    env.background_mode = Environment.BG_COLOR
    env.background_color = Color(0.48, 0.71, 0.95)
    env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    env.ambient_light_color = Color(0.75, 0.84, 1.0)
    env.ambient_light_energy = 0.8
    world_env.environment = env
    add_child(world_env)
    _box(Vector3(0, -0.5, 0), Vector3(420, 1, 420), Color(0.23, 0.36, 0.27))
    for i in range(-4, 5):
        _box(Vector3(i * 38, 0.015, 0), Vector3(13, 0.03, 420), Color(0.14, 0.15, 0.17), false)
        _box(Vector3(0, 0.02, i * 38), Vector3(420, 0.04, 13), Color(0.14, 0.15, 0.17), false)
    var rng := RandomNumberGenerator.new()
    rng.seed = 2026
    for x in range(-4, 4):
        for z in range(-4, 4):
            if abs(x) <= 1 and abs(z) <= 1:
                continue
            var height := rng.randf_range(12, 62)
            var pos := Vector3(x * 38 + 19, height * 0.5, z * 38 + 19)
            var tone := rng.randf_range(0.32, 0.6)
            _box(pos, Vector3(17, height, 17), Color(tone, tone * 1.05, tone * 1.12))
            _box(pos + Vector3(0, height * 0.5 + 0.35, 0), Vector3(18, 0.7, 18), Color(0.12, 0.15, 0.19), false)
    var layer := CanvasLayer.new()
    add_child(layer)
    var label := Label.new()
    label.text = "SUPERMAN SKYWARD  |  WASD move  |  F flight  |  Space up  |  Ctrl down  |  Shift boost  |  Mouse camera"
    label.position = Vector2(14, 12)
    layer.add_child(label)

func _box(pos: Vector3, size: Vector3, color: Color, collide := true) -> void:
    var mesh := MeshInstance3D.new()
    var cube := BoxMesh.new()
    cube.size = size
    mesh.mesh = cube
    mesh.position = pos
    var mat := StandardMaterial3D.new()
    mat.albedo_color = color
    mat.roughness = 0.83
    mesh.material_override = mat
    add_child(mesh)
    if collide:
        var body := StaticBody3D.new()
        body.position = pos
        var shape := CollisionShape3D.new()
        var box := BoxShape3D.new()
        box.size = size
        shape.shape = box
        body.add_child(shape)
        add_child(body)
