extends CharacterBody3D

# Procedural anatomical placeholder. Replace with a rigged licensed GLB later.
const WALK_SPEED := 7.0
const FLIGHT_SPEED := 28.0
const BOOST_SPEED := 85.0
const ACCEL := 7.0
var flying := false
var yaw := 0.0
var pitch := -0.15
var elapsed := 0.0
var pivot: Node3D
var camera: Camera3D
var visual: Node3D
var cape: MeshInstance3D
var left_arm: Node3D
var right_arm: Node3D
var left_leg: Node3D
var right_leg: Node3D
var cape_material: StandardMaterial3D
var blue: StandardMaterial3D
var red: StandardMaterial3D
var skin: StandardMaterial3D
var hair: StandardMaterial3D
var gold: StandardMaterial3D

func _ready() -> void:
    var collision := CollisionShape3D.new()
    var capsule := CapsuleShape3D.new()
    capsule.radius = 0.42
    capsule.height = 1.9
    collision.shape = capsule
    collision.position.y = 0.95
    add_child(collision)
    blue = _mat(Color("#153f9a"), 0.72)
    red = _mat(Color("#a20b20"), 0.62)
    skin = _mat(Color("#b77d60"), 0.85)
    hair = _mat(Color("#17171e"), 0.8)
    gold = _mat(Color("#e8b74b"), 0.42)
    visual = Node3D.new()
    add_child(visual)
    _ellipsoid(visual, Vector3(0, 1.43, 0), Vector3(0.47, 0.59, 0.26), blue)
    _ellipsoid(visual, Vector3(0, 0.96, 0), Vector3(0.37, 0.28, 0.24), blue)
    _ellipsoid(visual, Vector3(0, 2.11, 0), Vector3(0.245, 0.31, 0.23), skin)
    _ellipsoid(visual, Vector3(0, 2.34, 0.01), Vector3(0.25, 0.12, 0.235), hair)
    _ellipsoid(visual, Vector3(0, 1.98, -0.22), Vector3(0.15, 0.075, 0.05), skin)
    _ellipsoid(visual, Vector3(-0.095, 2.13, -0.22), Vector3(0.045, 0.025, 0.025), hair)
    _ellipsoid(visual, Vector3(0.095, 2.13, -0.22), Vector3(0.045, 0.025, 0.025), hair)
    _ellipsoid(visual, Vector3(0, 1.65, -0.267), Vector3(0.21, 0.17, 0.04), red)
    _ellipsoid(visual, Vector3(0, 1.65, -0.305), Vector3(0.13, 0.10, 0.025), gold)
    _ellipsoid(visual, Vector3(0, 0.86, 0), Vector3(0.4, 0.12, 0.255), red)
    left_arm = _limb(Vector3(-0.52, 1.76, 0), Vector3(0.15, 0.45, 0.15), blue, red, true)
    right_arm = _limb(Vector3(0.52, 1.76, 0), Vector3(0.15, 0.45, 0.15), blue, red, true)
    left_leg = _limb(Vector3(-0.20, 0.88, 0), Vector3(0.18, 0.43, 0.18), blue, red, false)
    right_leg = _limb(Vector3(0.20, 0.88, 0), Vector3(0.18, 0.43, 0.18), blue, red, false)
    cape_material = _mat(Color("#a4071b"), 0.95)
    cape_material.cull_mode = BaseMaterial3D.CULL_DISABLED
    cape = MeshInstance3D.new()
    cape.material_override = cape_material
    cape.position = Vector3(0, 1.98, 0.23)
    visual.add_child(cape)
    _update_cape(0.0, false)
    pivot = Node3D.new()
    pivot.position.y = 1.5
    add_child(pivot)
    camera = Camera3D.new()
    camera.position = Vector3(0, 1.25, 6.3)
    camera.current = true
    pivot.add_child(camera)
    Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _mat(color: Color, roughness: float) -> StandardMaterial3D:
    var material := StandardMaterial3D.new()
    material.albedo_color = color
    material.roughness = roughness
    return material

func _ellipsoid(parent: Node3D, pos: Vector3, scale_size: Vector3, material: Material) -> void:
    var part := MeshInstance3D.new()
    var sphere := SphereMesh.new()
    sphere.radius = 1.0
    sphere.height = 2.0
    part.mesh = sphere
    part.scale = scale_size
    part.position = pos
    part.material_override = material
    parent.add_child(part)

func _limb(pos: Vector3, size: Vector3, suit: Material, accent: Material, is_arm: bool) -> Node3D:
    var root := Node3D.new()
    root.position = pos
    visual.add_child(root)
    _ellipsoid(root, Vector3(0, -0.29, 0), size, suit)
    _ellipsoid(root, Vector3(0, -0.75, 0), size * Vector3(0.83, 0.93, 0.88), suit)
    _ellipsoid(root, Vector3(0, -1.03 if is_arm else -0.94, -0.05), Vector3(0.13, 0.18, 0.16) if is_arm else Vector3(0.19, 0.24, 0.26), accent)
    return root

func _update_cape(time: float, moving_fast: bool) -> void:
    var surface := SurfaceTool.new()
    surface.begin(Mesh.PRIMITIVE_TRIANGLES)
    var cols := 10
    var rows := 14
    for y in range(rows):
        for x in range(cols):
            var x0 := float(x) / float(cols)
            var x1 := float(x + 1) / float(cols)
            var y0 := float(y) / float(rows)
            var y1 := float(y + 1) / float(rows)
            var a := _cape_vertex(x0, y0, time, moving_fast)
            var b := _cape_vertex(x1, y0, time, moving_fast)
            var d := _cape_vertex(x0, y1, time, moving_fast)
            var e := _cape_vertex(x1, y1, time, moving_fast)
            for v in [a, b, d, b, e, d]:
                surface.add_vertex(v)
    surface.generate_normals()
    cape.mesh = surface.commit()

func _cape_vertex(u: float, v: float, time: float, fast: bool) -> Vector3:
    var width := lerpf(0.95, 1.6, v)
    var x := (u - 0.5) * width
    var wave := sin(time * (10.0 if fast else 3.5) + u * 7.0 + v * 6.0) * 0.12 * v
    var depth := 0.12 + v * (1.15 if fast else 0.26) + wave
    return Vector3(x, -v * 1.68, depth)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventKey and event.pressed and not event.echo:
        if event.keycode == KEY_F:
            flying = not flying
            if flying:
                velocity.y = 4.0
        elif event.keycode == KEY_ESCAPE:
            Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
    if event is InputEventMouseButton and event.pressed:
        Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
    if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
        yaw -= event.relative.x * 0.003
        pitch = clamp(pitch - event.relative.y * 0.003, -1.2, 1.0)

func _physics_process(delta: float) -> void:
    var forward := Vector3(sin(yaw), 0, -cos(yaw))
    var right := Vector3(cos(yaw), 0, sin(yaw))
    var direction := Vector3.ZERO
    if Input.is_key_pressed(KEY_W): direction += forward
    if Input.is_key_pressed(KEY_S): direction -= forward
    if Input.is_key_pressed(KEY_D): direction += right
    if Input.is_key_pressed(KEY_A): direction -= right
    direction = direction.normalized()
    var boost := flying and Input.is_key_pressed(KEY_SHIFT)
    var speed := BOOST_SPEED if boost else (FLIGHT_SPEED if flying else WALK_SPEED)
    var target := direction * speed
    velocity.x = move_toward(velocity.x, target.x, speed * ACCEL * delta)
    velocity.z = move_toward(velocity.z, target.z, speed * ACCEL * delta)
    if flying:
        var vertical := 0.0
        if Input.is_key_pressed(KEY_SPACE): vertical += 1.0
        if Input.is_key_pressed(KEY_CTRL): vertical -= 1.0
        velocity.y = move_toward(velocity.y, vertical * speed, speed * ACCEL * delta)
    else:
        velocity.y -= 24.0 * delta
        if is_on_floor() and Input.is_key_pressed(KEY_SPACE):
            velocity.y = 9.0
    move_and_slide()
    visual.rotation.y = lerp_angle(visual.rotation.y, yaw, minf(delta * 8.0, 1.0))
    visual.rotation.x = lerpf(visual.rotation.x, -0.55 if flying and direction.length() > 0.0 else 0.0, minf(delta * 5.0, 1.0))
    pivot.rotation.y = yaw
    pivot.rotation.x = pitch
    camera.fov = lerpf(camera.fov, 90.0 if boost else 72.0, minf(delta * 3.0, 1.0))
    elapsed += delta
    var gait := sin(elapsed * 10.0) * (0.45 if direction.length() > 0.0 and not flying else 0.0)
    left_arm.rotation.x = -gait if not flying else -1.1
    right_arm.rotation.x = gait if not flying else -1.1
    left_leg.rotation.x = gait if not flying else 0.12
    right_leg.rotation.x = -gait if not flying else 0.12
    _update_cape(elapsed, boost)
