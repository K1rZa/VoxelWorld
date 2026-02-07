# Быстрый старт - VoxelWorld

## После открытия проекта в UE 5.4.4, выполните эти шаги:

### 1. Создайте Input Actions в Content/Input/

```
Правый клик → Input → Input Action → создайте:
├── IA_Move     (Value Type: Axis2D)
├── IA_Look     (Value Type: Axis2D)
├── IA_Jump     (Value Type: Digital)
└── IA_Crouch   (Value Type: Digital)
```

### 2. Создайте Input Mapping Context

```
Правый клик → Input → Input Mapping Context → IMC_Default

Добавьте маппинги:
├── IA_Move:
│   ├── W → Modifiers: [Swizzle YXZ, Negate]
│   ├── S → Modifiers: [Swizzle YXZ]
│   ├── A → Modifiers: [Negate]
│   └── D
├── IA_Look:
│   └── Mouse XY → Modifiers: [Negate Y только]
├── IA_Jump:
│   └── Space Bar
└── IA_Crouch:
    └── Left Ctrl
```

### 3. Создайте Blueprints в Content/Blueprints/

```
BP_VoxelPlayerCharacter (родитель: VoxelPlayerCharacter)
├── Default Mapping Context: IMC_Default
├── Move Action: IA_Move
├── Look Action: IA_Look
├── Jump Action: IA_Jump
└── Crouch Action: IA_Crouch

BP_VoxelWorldGameMode (родитель: VoxelWorldGameMode)
└── Default Pawn Class: BP_VoxelPlayerCharacter
```

### 4. Создайте уровень Content/Maps/VoxelWorld

```
File → New Level → Empty Level
Добавьте:
├── Directional Light
├── Sky Atmosphere (или BP_Sky_Sphere)
└── Exponential Height Fog (опционально)

World Settings:
└── GameMode Override: BP_VoxelWorldGameMode
```

### 5. Запустите игру (Play)!

Управление:
- WASD - движение
- Мышь - камера
- Space - прыжок
- Ctrl - присед
