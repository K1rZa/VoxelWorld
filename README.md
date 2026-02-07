# VoxelWorld - Процедурная генерация мира из блоков

## Описание проекта

Тестовый проект на Unreal Engine 5.4.4 с процедурной генерацией воксельного мира.

### Параметры мира:
- **Размер блока:** 4x4x4 метра (400x400x400 см в UE)
- **Размер игрока:** 4x4x8 метров
- **Размер мира:** 512x512x32 блока
- **Размер чанка:** 16x16x32 блока
- **Дальность прорисовки:** 4 чанка вокруг игрока
- **Типы блоков:**
  - Камень (серый)
  - Трава (зелёный)
  - Песок (жёлтый)

### Управление:
- **WASD** - движение
- **Мышь** - поворот камеры
- **SPACE** - прыжок
- **CTRL** - присесть

---

## Установка и настройка

### Шаг 1: Откройте проект в Unreal Engine 5.4.4

1. Запустите Unreal Engine 5.4.4
2. Откройте файл `VoxelWorld.uproject`
3. Дождитесь компиляции шейдеров и C++ кода

### Шаг 2: Создайте Input Actions (Enhanced Input)

В папке `/Content/Input/` создайте следующие ассеты:

#### 1. Input Action: IA_Move
- Right-click → Input → Input Action
- Назовите: `IA_Move`
- Откройте и установите:
  - Value Type: `Axis2D (Vector2D)`

#### 2. Input Action: IA_Look
- Right-click → Input → Input Action
- Назовите: `IA_Look`
- Откройте и установите:
  - Value Type: `Axis2D (Vector2D)`

#### 3. Input Action: IA_Jump
- Right-click → Input → Input Action
- Назовите: `IA_Jump`
- Откройте и установите:
  - Value Type: `Digital (bool)`

#### 4. Input Action: IA_Crouch
- Right-click → Input → Input Action
- Назовите: `IA_Crouch`
- Откройте и установите:
  - Value Type: `Digital (bool)`

#### 5. Input Mapping Context: IMC_Default
- Right-click → Input → Input Mapping Context
- Назовите: `IMC_Default`
- Откройте и добавьте маппинги:

**Для IA_Move:**
- Нажмите "+" и выберите `IA_Move`
- Добавьте клавиши:
  - W (добавьте модификатор Swizzle Input Axis Values: YXZ, затем Negate)
  - S (добавьте модификатор Swizzle Input Axis Values: YXZ)
  - A (добавьте модификатор Negate)
  - D

**Для IA_Look:**
- Нажмите "+" и выберите `IA_Look`
- Добавьте:
  - Mouse XY 2D-Axis (добавьте модификатор Negate для Y)

**Для IA_Jump:**
- Нажмите "+" и выберите `IA_Jump`
- Добавьте клавишу: Space Bar

**Для IA_Crouch:**
- Нажмите "+" и выберите `IA_Crouch`
- Добавьте клавишу: Left Ctrl

### Шаг 3: Создайте Blueprint персонажа

1. В папке `/Content/Blueprints/` создайте Blueprint:
   - Right-click → Blueprint Class → выберите `VoxelPlayerCharacter` как родительский класс
   - Назовите: `BP_VoxelPlayerCharacter`

2. Откройте Blueprint и в панели Details найдите секцию "Input":
   - **Default Mapping Context:** выберите `IMC_Default`
   - **Move Action:** выберите `IA_Move`
   - **Look Action:** выберите `IA_Look`
   - **Jump Action:** выберите `IA_Jump`
   - **Crouch Action:** выберите `IA_Crouch`

3. Скомпилируйте и сохраните Blueprint

### Шаг 4: Создайте Blueprint GameMode

1. В папке `/Content/Blueprints/` создайте Blueprint:
   - Right-click → Blueprint Class → выберите `VoxelWorldGameMode` как родительский класс
   - Назовите: `BP_VoxelWorldGameMode`

2. Откройте Blueprint и установите:
   - **Default Pawn Class:** `BP_VoxelPlayerCharacter`

3. Скомпилируйте и сохраните Blueprint

### Шаг 5: Создайте уровень

1. Создайте новую папку `/Content/Maps/`
2. Создайте новый уровень: File → New Level → Empty Level
3. Сохраните как `/Content/Maps/VoxelWorld`
4. Добавьте Directional Light и Sky Sphere для освещения

### Шаг 6: Настройте World Settings

1. Откройте уровень VoxelWorld
2. Откройте Window → World Settings
3. Установите:
   - **GameMode Override:** `BP_VoxelWorldGameMode`

### Шаг 7: Настройте Project Settings

1. Откройте Edit → Project Settings
2. В разделе Maps & Modes:
   - **Editor Startup Map:** `/Game/Maps/VoxelWorld`
   - **Game Default Map:** `/Game/Maps/VoxelWorld`
   - **Default GameMode:** `BP_VoxelWorldGameMode`

3. В разделе Input:
   - **Default Player Input Class:** `EnhancedPlayerInput`
   - **Default Input Component Class:** `EnhancedInputComponent`

---

## Структура проекта

```
VoxelWorld/
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   └── DefaultInput.ini
├── Content/
│   ├── Blueprints/
│   │   ├── BP_VoxelPlayerCharacter (создать)
│   │   └── BP_VoxelWorldGameMode (создать)
│   ├── Input/
│   │   ├── IA_Move (создать)
│   │   ├── IA_Look (создать)
│   │   ├── IA_Jump (создать)
│   │   ├── IA_Crouch (создать)
│   │   └── IMC_Default (создать)
│   └── Maps/
│       └── VoxelWorld (создать)
├── Source/
│   ├── VoxelWorld/
│   │   ├── VoxelChunk.h/.cpp
│   │   ├── VoxelPlayerCharacter.h/.cpp
│   │   ├── VoxelWorld.h/.cpp
│   │   ├── VoxelWorld.Build.cs
│   │   ├── VoxelWorldConstants.h
│   │   ├── VoxelWorldGameMode.h/.cpp
│   │   └── VoxelWorldManager.h/.cpp
│   ├── VoxelWorld.Target.cs
│   └── VoxelWorldEditor.Target.cs
└── VoxelWorld.uproject
```

---

## Описание классов

### AVoxelChunk
Отвечает за хранение и отрисовку одного чанка мира. Использует `ProceduralMeshComponent` для генерации геометрии блоков. Оптимизирует отрисовку, показывая только видимые грани блоков (greedy meshing не включен для простоты).

### AVoxelWorldManager
Управляет загрузкой и выгрузкой чанков в зависимости от позиции игрока. Отслеживает активные чанки и обновляет их при перемещении игрока.

### AVoxelPlayerCharacter
Персонаж игрока с поддержкой Enhanced Input System. Реализует движение, прыжок, приседание и управление камерой от первого лица.

### AVoxelWorldGameMode
Игровой режим, который спавнит VoxelWorldManager при старте игры.

---

## Генерация террейна

Террейн генерируется с использованием простой функции шума на основе синусоид:

```cpp
float Height = 8.0f + 
    sin(WorldX * 0.5f) * 3.0f + 
    sin(WorldY * 0.5f) * 3.0f +
    sin(WorldX * 0.2f + WorldY * 0.3f) * 5.0f;
```

- Нижние слои (глубже 3 блоков от поверхности) — камень
- Верхние слои — трава (на возвышенностях) или песок (в низинах)

---

## Возможные улучшения

1. **Perlin Noise** — заменить синусоиды на настоящий шум Perlin для более реалистичного террейна
2. **Greedy Meshing** — оптимизация объединения соседних граней одинаковых блоков
3. **LOD система** — разный уровень детализации для дальних чанков
4. **Многопоточная генерация** — асинхронная генерация чанков
5. **Сохранение мира** — сериализация изменений блоков
6. **Разрушение/строительство блоков** — интерактивность

---

## Требования

- Unreal Engine 5.4.4
- Visual Studio 2022 или Rider
- Windows 10/11

---

## Лицензия

Этот проект предоставляется как есть для образовательных целей.
