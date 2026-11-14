extends CharacterBody3D

# Uses the uploaded exported-model (2).glb, preserving its rig and 178 animations.
const MODEL_PATH := "res://assets/exported-model (2).glb"
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
var animation_player: AnimationPlayer
var active_animation := ""
var current_speed := 0.0

func _ready() -> void:
    var collision := CollisionShape3D.new()
    var capsule := CapsuleShape3D.new()
    capsule.radius = 0.42
    capsule.height = 1.9
    collision.shape = capsule
    collision.position.y = 0.95
    add_child(collision)
    visual = Node3D.new()
    visual.name = "VisualRoot"
    add_child(visual)
    if ResourceLoader.exists(MODEL_PATH):
        var model_scene := load(MODEL_PATH) as PackedScene
        if model_scene != null:
            var model := model_scene.instantiate()
            visual.add_child(model)
            animation_player = _find_animation_player(model)
            if animation_player != null:
                animation_player.playback_default_blend_time = 0.22
                _play_named("Idle_A")
    else:
        push_warning("Missing GLB. Copy exported-model (2).glb into res://assets/")
    pivot = Node3D.new()
    pivot.position.y = 1.5
    add_child(pivot)
    camera = Camera3D.new()
    camera.position = Vector3(0, 1.25, 6.3)
    camera.current = true
    pivot.add_child(camera)
    Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _find_animation_player(node: Node) -> AnimationPlayer:
    if node is AnimationPlayer:
        return node as AnimationPlayer
    for child in node.get_children():
        var found := _find_animation_player(child)
        if found != null:
            return found
    return null

func _play_named(clip: String) -> void:
    if animation_player == null or clip == active_animation:
        return
    var found := ""
    if animation_player.has_animation(clip):
        found = clip
    else:
        for name in animation_player.get_animation_list():
            if name == clip or name.ends_with("/" + clip):
                found = name
                break
    if found != "":
        animation_player.play(found, 0.2)
        active_animation = clip

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
    current_speed = Vector2(velocity.x, velocity.z).length()
    if flying:
        _play_named("Flying_Forward_Super" if boost and current_speed > 2.0 else ("Flying_Forward" if current_speed > 2.0 else "Levitate_Idle"))
    elif current_speed > 0.5:
        _play_named("Walk")
    else:
        _play_named("Idle_A")
