extends CharacterBody3D

const WALK_SPEED := 7.0
const FLIGHT_SPEED := 28.0
const BOOST_SPEED := 85.0
const ACCEL := 7.0
var flying := false
var yaw := 0.0
var pitch := -0.15
var pivot: Node3D
var camera: Camera3D
var visual: Node3D
var cape: MeshInstance3D
var elapsed := 0.0

func _ready() -> void:
    var collision := CollisionShape3D.new()
    var capsule := CapsuleShape3D.new()
    capsule.radius = 0.42
    capsule.height = 1.9
    collision.shape = capsule
    collision.position.y = 0.95
    add_child(collision)
    visual = Node3D.new()
    add_child(visual)
    _part(Vector3(0, 1.35, 0), Vector3(0.9, 1.0, 0.43), Color(0.04, 0.16, 0.65))
    _part(Vector3(-0.22, 0.5, 0), Vector3(0.3, 1.0, 0.34), Color(0.04, 0.16, 0.65))
    _part(Vector3(0.22, 0.5, 0), Vector3(0.3, 1.0, 0.34), Color(0.04, 0.16, 0.65))
    _part(Vector3(-0.58, 1.3, 0), Vector3(0.3, 0.9, 0.35), Color(0.04, 0.16, 0.65))
    _part(Vector3(0.58, 1.3, 0), Vector3(0.3, 0.9, 0.35), Color(0.04, 0.16, 0.65))
    _part(Vector3(0, 0.92, -0.02), Vector3(0.9, 0.22, 0.46), Color(0.8, 0.04, 0.08))
    var head := MeshInstance3D.new()
    var sphere := SphereMesh.new()
    sphere.radius = 0.25
    sphere.height = 0.5
    head.mesh = sphere
    head.position = Vector3(0, 2.08, 0)
    head.material_override = _mat(Color(0.78, 0.52, 0.38))
    visual.add_child(head)
    cape = MeshInstance3D.new()
    var cloth := PlaneMesh.new()
    cloth.size = Vector2(1.2, 1.7)
    cape.mesh = cloth
    cape.material_override = _mat(Color(0.75, 0.03, 0.08))
    cape.position = Vector3(0, 1.35, 0.36)
    cape.rotation.x = PI / 2.0
    visual.add_child(cape)
    pivot = Node3D.new()
    pivot.position.y = 1.5
    add_child(pivot)
    camera = Camera3D.new()
    camera.position = Vector3(0, 1.2, 6.5)
    camera.current = true
    pivot.add_child(camera)
    Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _mat(color: Color) -> StandardMaterial3D:
    var material := StandardMaterial3D.new()
    material.albedo_color = color
    material.roughness = 0.75
    material.cull_mode = BaseMaterial3D.CULL_DISABLED
    return material

func _part(pos: Vector3, size: Vector3, color: Color) -> void:
    var mesh := MeshInstance3D.new()
    var cube := BoxMesh.new()
    cube.size = size
    mesh.mesh = cube
    mesh.position = pos
    mesh.material_override = _mat(color)
    visual.add_child(mesh)

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
    cape.rotation.x = PI / 2.0 + sin(elapsed * (11.0 if boost else 4.0)) * (0.15 if flying else 0.035)
