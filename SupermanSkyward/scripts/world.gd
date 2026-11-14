extends Node3D

# Detailed, mobile-conscious procedural city. Decorative surfaces have no collision.
const ROAD_STEP := 38.0
var rng := RandomNumberGenerator.new()
var facade_materials: Array[StandardMaterial3D] = []
var glass_material: StandardMaterial3D
var lit_material: StandardMaterial3D
var pavement_material: StandardMaterial3D
var asphalt_material: StandardMaterial3D

func _ready() -> void:
    rng.seed = 20261115
    _setup_materials()
    _lighting()
    _build_city()
    _hud()

func _mat(color: Color, roughness: float = 0.8, metallic: float = 0.0) -> StandardMaterial3D:
    var m := StandardMaterial3D.new()
    m.albedo_color = color
    m.roughness = roughness
    m.metallic = metallic
    return m

func _setup_materials() -> void:
    for c in [Color("#a8a9a5"), Color("#c5b8a5"), Color("#777f8b"), Color("#b4b9bd"), Color("#978e86")]:
        facade_materials.append(_mat(c))
    glass_material = _mat(Color("#23485d"), 0.17, 0.55)
    lit_material = _mat(Color("#f4d99a"), 0.32)
    lit_material.emission_enabled = true
    lit_material.emission = Color("#d9a65b")
    lit_material.emission_energy_multiplier = 0.4
    pavement_material = _mat(Color("#9a9a93"))
    asphalt_material = _mat(Color("#333941"))

func _lighting() -> void:
    var sun := DirectionalLight3D.new()
    sun.rotation_degrees = Vector3(-48, -34, 0)
    sun.light_energy = 1.25
    sun.shadow_enabled = true
    sun.directional_shadow_max_distance = 95.0
    add_child(sun)
    var world_env := WorldEnvironment.new()
    var env := Environment.new()
    env.background_mode = Environment.BG_COLOR
    env.background_color = Color("#88b9e3")
    env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    env.ambient_light_color = Color("#b2c7df")
    env.ambient_light_energy = 0.75
    env.tonemap_mode = Environment.TONE_MAPPER_FILMIC
    world_env.environment = env
    add_child(world_env)

func _block(pos: Vector3, size: Vector3, material: Material, collide: bool = false) -> void:
    var mesh := MeshInstance3D.new()
    var box := BoxMesh.new()
    box.size = size
    mesh.mesh = box
    mesh.material_override = material
    mesh.position = pos
    add_child(mesh)
    if collide:
        var body := StaticBody3D.new()
        body.position = pos
        var shape := CollisionShape3D.new()
        var box_shape := BoxShape3D.new()
        box_shape.size = size
        shape.shape = box_shape
        body.add_child(shape)
        add_child(body)

func _build_city() -> void:
    _block(Vector3(0, -0.6, 0), Vector3(420, 1.2, 420), _mat(Color("#718b70")), true)
    for i in range(-5, 6):
        var p := float(i) * ROAD_STEP
        _block(Vector3(p, 0.025, 0), Vector3(13.5, 0.06, 420), asphalt_material)
        _block(Vector3(0, 0.03, p), Vector3(420, 0.06, 13.5), asphalt_material)
        for j in range(-5, 6):
            var q := float(j) * ROAD_STEP
            if abs(i) < 5:
                _block(Vector3(p + 19, 0.065, q), Vector3(1.6, 0.012, 0.14), _mat(Color("#eee1b0")))
                _block(Vector3(q, 0.07, p + 19), Vector3(0.14, 0.012, 1.6), _mat(Color("#eee1b0")))
    for x in range(-4, 4):
        for z in range(-4, 4):
            var cx := float(x) * ROAD_STEP + ROAD_STEP * 0.5
            var cz := float(z) * ROAD_STEP + ROAD_STEP * 0.5
            _block(Vector3(cx, 0.13, cz), Vector3(23, 0.26, 23), pavement_material)
            if abs(x) <= 1 and abs(z) <= 1:
                _park(cx, cz)
            else:
                _building(cx, cz, rng.randf_range(17, 65), (x * 7 + z * 3) % facade_materials.size())
    for i in range(-3, 4):
        _street_light(float(i) * ROAD_STEP + 7, 8)
        _street_light(8, float(i) * ROAD_STEP + 7)

func _building(cx: float, cz: float, height: float, material_index: int) -> void:
    var width := rng.randf_range(15, 19)
    var depth := rng.randf_range(15, 19)
    var material := facade_materials[material_index]
    _block(Vector3(cx, height * 0.5 + 0.26, cz), Vector3(width, height, depth), material, true)
    _block(Vector3(cx, height + 0.42, cz), Vector3(width + 0.8, 0.55, depth + 0.8), _mat(Color("#353e48")))
    _block(Vector3(cx, 1.6, cz + depth * 0.5 + 0.025), Vector3(2.5, 2.9, 0.09), glass_material)
    var floors := mini(int(height / 3.6), 16)
    var columns := 4
    for floor_idx in range(1, floors):
        var y := 2.3 + float(floor_idx) * 3.55
        for col in range(columns):
            var u := (float(col) + 0.5) / float(columns) - 0.5
            var x := cx + u * width * 0.88
            var z := cz + u * depth * 0.88
            var window_material := lit_material if rng.randf() > 0.82 else glass_material
            _block(Vector3(x, y, cz + depth * 0.5 + 0.025), Vector3(width * 0.15, 1.9, 0.055), window_material)
            _block(Vector3(x, y, cz - depth * 0.5 - 0.025), Vector3(width * 0.15, 1.9, 0.055), window_material)
            _block(Vector3(cx + width * 0.5 + 0.025, y, z), Vector3(0.055, 1.9, depth * 0.15), glass_material)
            _block(Vector3(cx - width * 0.5 - 0.025, y, z), Vector3(0.055, 1.9, depth * 0.15), glass_material)
        _block(Vector3(cx, y - 1.16, cz + depth * 0.5 + 0.07), Vector3(width, 0.16, 0.17), _mat(Color("#50545a")))

func _park(cx: float, cz: float) -> void:
    _block(Vector3(cx, 0.3, cz), Vector3(19, 0.09, 19), _mat(Color("#59855e")))
    for i in range(3):
        _tree(cx - 6 + i * 6, cz - 6)
        _tree(cx - 6 + i * 6, cz + 6)

func _tree(x: float, z: float) -> void:
    _block(Vector3(x, 1.5, z), Vector3(0.35, 3, 0.35), _mat(Color("#66462d")))
    var crown := MeshInstance3D.new()
    var sphere := SphereMesh.new()
    sphere.radius = 2.2
    sphere.height = 4.1
    crown.mesh = sphere
    crown.position = Vector3(x, 4.2, z)
    crown.material_override = _mat(Color("#3d6b43"))
    add_child(crown)

func _street_light(x: float, z: float) -> void:
    var metal := _mat(Color("#505961"), 0.3, 0.6)
    _block(Vector3(x, 3.2, z), Vector3(0.13, 6.4, 0.13), metal)
    _block(Vector3(x + 0.8, 6.4, z), Vector3(1.7, 0.12, 0.12), metal)
    _block(Vector3(x + 1.5, 6.3, z), Vector3(0.5, 0.08, 0.35), lit_material)

func _hud() -> void:
    var layer := CanvasLayer.new()
    add_child(layer)
    var label := Label.new()
    label.text = "SKYWARD  |  WASD: move  F: fly  SPACE/CTRL: height  SHIFT: boost  MOUSE: camera"
    label.position = Vector2(14, 12)
    layer.add_child(label)
