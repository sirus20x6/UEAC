# AstroChicken - Space Colonization Game

## Project Overview
**Genre**: Oregon Trail in Space - Space Colonization Strategy Game
**Engine**: Unreal Engine 5.6
**Language**: C++ (Blueprint minimal usage)
**Platform**: Linux Development
**Current Build Target**: Linux Development

---

## Core Design Philosophy

### Primary Principles
- **Pure C++ Implementation**: All functionality written in C++ rather than Blueprints
- **JSON-Driven Configuration**: UI layouts and data defined in external JSON files
- **Dynamic Runtime Generation**: UI elements created at runtime from configuration
- **Professional Architecture**: Clean, maintainable code following UE5 best practices
- **Leverage Engine Features**: Extend existing UE functionality rather than rebuilding

---

## Game Concept

**AstroChicken** is a space colonization game inspired by Oregon Trail, where players:
1. Select an alien species to play as
2. Name and customize their colony
3. Allocate research points across tech disciplines
4. Journey through space facing challenges
5. Manage resources and technology progression
6. Engage in turret defense minigames

---

## Current Systems

### 1. Race Selection System ✅

**Location**: `/Source/astrochicken/Race/`

**Status**: Fully Implemented

**Features**:
- 11 unique phylums with distinct attributes
- 4 playable races (unlocked by default):
  - **Chordata (Vertebranes)**: Social leaders, intelligence-focused
  - **Arthropoda (Chitinoids)**: Resilient, efficient, resource extraction specialists
  - **Cnidaria (Medusians)**: Highly adaptable, space-adapted
  - **Mollusca (Cephalopods)**: Intelligent, technology-focused
- 7 unlockable races (future content)
- Custom colony naming
- Race-specific attribute bonuses (8 attributes per race)
- Race icons (1024x1024 images, scaled to 256x256 in UI)

**Attributes**:
- Adaptability
- Resilience
- Intelligence
- Social
- Efficiency
- Space Adaptation
- Resource Extraction
- Tech Affinity

**Integration**:
- Race bonuses affect starting tech points
- Full save/load support
- UUID-based game runs

---

### 2. Tech Tree System ✅

**Location**: `/Source/astrochicken/TechTree/`

**Status**: Fully Implemented

**Categories**:
- Physics
- Engineering
- Energy
- Computing
- Biology
- Sensors

**Features**:
- Multi-discipline prerequisites
- Race-based starting bonuses
- Point allocation system
- Research progression
- Tech unlocking system
- Full save/load support

**Tech Point Allocation**:
- Base allocation: 10 points per category
- Race bonuses modify starting points
- Players allocate additional points during game start

---

### 3. Save System ✅

**Location**: `/Source/astrochicken/GameState/`

**Status**: Fully Implemented

**Save Types**:
1. **Manual Saves**: Player-named with ISO timestamps
2. **Auto Saves**: 3 rotating slots per game UUID
3. **Checkpoint Saves**: Story-critical saves
   - NewGame
   - Prologue
   - Act 1-5

**Features**:
- UUID-based game runs
- Save tree visualization support
- Branch/fork functionality (planned)
- ISO timestamp naming
- Full game state persistence
- Race data preservation
- Tech tree state saving

**Save Tree Structure**:
- Parent-child relationships
- Branch tracking
- Sequence numbering
- Custom branch descriptions

---

### 4. Dynamic Menu System ✅

**Location**: `/Source/astrochicken/UI/`

**Status**: Fully Functional

**Components**:
- `MainMenuWidget`: Primary navigation with widget switcher
- `DynamicSettingsWidget`: JSON-driven settings interface
- `GameSettingsManager`: Settings persistence
- `MeshButtonMenu`: 3D mesh-based menu navigation

**Settings Categories**:
- **Graphics**: Resolution, window mode, quality presets, VSync, frame rate limits
- **Advanced Graphics**: Anisotropic filtering, anti-aliasing (method & quality), texture quality, shadow quality, effects quality, post-processing
- **Audio**: Master, Music, SFX, Ambient, Voice volumes
- **Gameplay**: (Future)

**Features**:
- Runtime UI generation from JSON
- Real-time value display
- Mouse calibration system (fixes offset in windowed mode)
- Proper UE5 GameUserSettings integration
- Full save/load persistence
- Tab-based organization
- Responsive design with resolution-aware spacing

**JSON Configuration**: `/Content/Data/SettingsLayout.json`

---

### 5. Game Start Flow ✅

**Location**: `/Source/astrochicken/UI/GameStart/`

**Status**: Fully Implemented

**Flow Phases**:
1. **Race Selection**: Choose species and name colony
2. **Tech Point Allocation**: Distribute bonus research points
3. **Game Ready**: Initialization complete, checkpoint save created

**Widgets**:
- `GameStartController`: Flow management
- `RaceSelectionWidget`: Species selection UI
- `RaceButtonWidget`: Individual race display
- `TechPointAllocationWidget`: Point distribution interface

**Features**:
- Widget switcher for phase transitions
- Automatic "NewGame" checkpoint save
- Race bonus preview
- Custom colony naming
- UUID generation for new game runs

---

### 6. Turret Defense Minigame 🔧

**Location**: `/Source/astrochicken/MiniGames/Turrets/`

**Status**: Core Features Complete, Refinement Ongoing

**Components**:
- `Turret.h/.cpp`: Main turret class with dual-axis rotation
- `TurretAIController.h/.cpp`: AI targeting and firing logic
- `Asteroid.h/.cpp`: Target system with fragmentation
- `Projectile.h/.cpp`: Bullet/projectile system
- `AsteroidSpawner.h/.cpp`: Enemy spawning system
- `Hardpoint.h/.cpp`: Turret mounting and power system

**Turret Features**:
- Dual-axis rotation (yaw and pitch)
- Independent rotation ranges
- AI-controlled or player-controlled
- Power management system
- Audio integration (movement, firing, power)
- Lead targeting for moving objects
- Multiple turret types: Light, Medium, Heavy, Point Defense

**Hardpoint System**:
- Small, Medium, Large sizes
- Power delivery from electrical grid
- Installation/removal mechanics
- State management (Operational, Damaged, Destroyed)
- Hy21_mesh docking component
- Compatibility checking

**Asteroid System**:
- Health and damage system
- Fragmentation (breaks into smaller pieces)
- Size progression: Massive → Large → Medium → Small
- Repair functionality
- Movement and rotation
- Multi-generation fragments

**Known Issues Being Addressed**:
- Turret firing synchronization (staggering implemented)
- AI targeting reliability
- Rotation speed calculations
- Projectile visual representation

---

### 7. Audio System 🔧

**Location**: `/Source/astrochicken/Audio/`

**Status**: Basic Implementation

**Components**:
- Environmental context manager (leverages UE5 built-ins)
- Turret sound integration
- Volume control through settings

**Audio Types**:
- Turret movement sounds
- Turret firing sounds
- Power system sounds
- Ambient/environmental audio

---

### 8. Infrastructure System 🚧

**Location**: `/Source/astrochicken/Infrastructure/`

**Status**: Hardpoint Complete, Electrical Grid Planned

**Current**:
- Hardpoint system fully implemented
- Power connection visualization

**Planned**:
- Electrical grid network
- Reactor system (one to many)
- Branching wire network
- Power distribution management
- Load balancing

---

## Menu System Architecture (Technical Deep Dive)

### 📁 File Structure
```
Source/astrochicken/
├── UI/
│   ├── MainMenuWidget.h/.cpp          # Main menu navigation with widget switcher
│   ├── DynamicSettingsWidget.h/.cpp   # JSON-driven settings interface with working functionality
│   └── GameSettingsManager.h/.cpp     # Settings persistence manager
├── GameModes/
│   └── MenuGameMode.h/.cpp             # Menu-specific game mode
└── Player/
    └── MenuPlayerController.h/.cpp     # Menu input handling

Content/
└── Data/
    └── SettingsLayout.json             # Settings UI configuration
```

### 🎯 System Components

#### 1. **MainMenuWidget**
- **Purpose**: Primary menu navigation using Widget Switcher
- **Features**:
  - Seamless switching between main menu and settings
  - Automatic C++ event binding for buttons
  - Container-based dynamic widget population
  - Proper input mode management

#### 2. **DynamicSettingsWidget** ✨ **FULLY FUNCTIONAL**
- **Purpose**: Generates complete settings interface from JSON configuration with working functionality
- **Features**:
  - **✅ Working Resolution Changes**: Instant resolution switching with automatic mouse calibration
  - **✅ Working Window Mode**: Fullscreen/Windowed/Borderless transitions that actually work
  - **✅ Automatic Mouse Fix**: Intelligent mouse coordinate recalibration after resolution changes
  - **✅ Graphics Quality Settings**: All quality sliders apply and save properly
  - **✅ VSync & Frame Rate Limiting**: Working performance settings
  - **✅ Settings Persistence**: All settings save and load correctly
  - **Runtime UI Generation**: Creates sliders, combo boxes, checkboxes from JSON
  - **Container Population**: Directly populates parent containers instead of creating widget trees
  - **Real-time Value Display**: Shows formatted values as users adjust settings
  - **UE5 Integration**: Hooks into GameUserSettings for graphics options
  - **Extensible Design**: Easy to add new setting types via JSON

#### 3. **JSON Configuration System**
- **File**: `Content/Data/SettingsLayout.json`
- **Capabilities**:
  - Define UI sections (Graphics, Audio, etc.)
  - Specify control types (Slider, ComboBox, CheckBox, Button)
  - Set ranges, steps, formatting options
  - Configure button layouts and styles

#### 4. **✨ Intelligent Mouse Calibration System**
- **Automatic Detection**: Identifies windowed vs fullscreen mode
- **Smart Recalibration**: Multiple-stage mouse coordinate fixing
- **Auto-Toggle Fix**: Automatically toggles fullscreen/windowed to fix mouse offset
- **User Feedback**: Clear messages about calibration progress
- **Fallback Options**: Manual toggle suggestions if auto-fix fails

### 🚀 Startup Flow
1. **MenuGameMode** spawns **MenuPlayerController**
2. **MenuPlayerController** creates and displays **MainMenuWidget**
3. **MainMenuWidget** initializes with **Widget Switcher** containing:
   - Slot 0: Main menu buttons (Start Game, Settings, Quit)
   - Slot 1: **SettingsContainer** (VerticalBox for dynamic population)

### ⚙️ Settings Generation Process
1. **MainMenuWidget** creates **DynamicSettingsWidget** instance
2. When user clicks Settings → **PopulateContainer()** called
3. **DynamicSettingsWidget** loads `SettingsLayout.json`
4. **JSON Parser** converts configuration into C++ structs
5. **UI Generator** creates widgets directly in parent container:
   ```cpp
   // Fixed approach: Direct container population
   void UDynamicSettingsWidget::PopulateContainer(UVerticalBox* Container)
   {
       Container->ClearChildren();
       // Create widgets directly in container
       Container->AddChild(CreateSliderWithLabel(Control));
   }
   ```
6. **Event Binding** for real-time interaction

### 🎮 User Interaction Flow
1. User clicks **Settings** → `OnSettingsClicked()` → `PopulateContainer(SettingsContainer)`
2. **Widget Switcher** switches to Slot 1 showing populated settings
3. User adjusts slider → `OnSliderValueChanged()` → Real-time value display updates
4. User clicks **Back** → `OnBackToMainMenu.Broadcast()` → Return to main menu

---

## Technical Implementation Details

### 🔧 Key C++ Patterns Used

#### **Automatic Widget Binding**
```cpp
// Header declaration
UPROPERTY(meta = (BindWidget))
UVerticalBox* SettingsContainer;  // Container for dynamic population

// Automatic binding in NativeConstruct()
if (SettingsContainer)
{
    // Container ready for dynamic population
}
```

#### **Container-Based Dynamic Generation**
```cpp
// NEW PATTERN: Direct container population
void UDynamicSettingsWidget::PopulateContainer(UVerticalBox* Container)
{
    TargetContainer = Container;
    TargetContainer->ClearChildren();

    // Load JSON and create widgets directly in container
    if (LoadSettingsLayout())
    {
        GenerateSettingsUI();  // Populates TargetContainer
    }
}
```

#### **JSON-to-Struct Serialization**
```cpp
USTRUCT(BlueprintType)
struct FSettingsControl
{
    GENERATED_BODY()

    UPROPERTY()
    FString Type;           // "Slider", "ComboBox", "CheckBox"

    UPROPERTY()
    FString Name;           // Widget identifier

    UPROPERTY()
    FString Label;          // Display text

    // ... additional properties
};
```

### 📊 Settings Categories Implemented

#### **Enhanced Graphics Settings**
- Resolution (1280x720 to 3840x2160)
- Window Mode (Fullscreen, Windowed, Borderless)
- Quality Presets (Low, Medium, High, Epic, Cinematic)
- Individual Quality Sliders:
  - View Distance, Shadow Quality, Texture Quality
  - Anti-Aliasing, Post Processing, Effects
  - Foliage Quality, Shading Quality
- Performance Options: VSync, Frame Rate Limiting

#### **Enhanced Audio Settings**
- Master Volume (0-100%)
- Music Volume (0-100%)
- SFX Volume (0-100%)

### 🎨 Enhanced UI Design System

#### **Responsive Layout Engine**
```cpp
struct FUILayoutConfig
{
    float MinWindowWidth = 1280.0f;     // Minimum supported resolution
    float MaxContentWidth = 800.0f;     // Content width constraint
    float ControlHeight = 40.0f;        // Base control height
    float ControlSpacing = 12.0f;       // Space between controls
    float SectionSpacing = 24.0f;       // Space between sections
    float SidePadding = 32.0f;          // Left/right margins
    float TopBottomPadding = 24.0f;     // Top/bottom margins
};
```

**Key Features:**
- **Dynamic Scaling**: UI elements scale from 0.8x to 1.5x based on screen width
- **Resolution Detection**: Automatically detects viewport size using UWidgetLayoutLibrary
- **Constraint-Based Layout**: Content width clamped between 600px and 1000px
- **Minimum Size Enforcement**: Ensures usability on 1280x720 displays

#### **Professional Color Scheme**
```cpp
// Color Constants
FLinearColor GetPrimaryColor()   { return FLinearColor(0.1f, 0.4f, 0.8f, 1.0f); }  // Blue
FLinearColor GetSecondaryColor() { return FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); }  // Dark Gray
FLinearColor GetBackgroundColor(){ return FLinearColor(0.05f, 0.05f, 0.05f, 0.9f); } // Near Black
FLinearColor GetTextColor()      { return FLinearColor(0.9f, 0.9f, 0.9f, 1.0f); }  // Light Gray
FLinearColor GetAccentColor()    { return FLinearColor(0.3f, 0.6f, 1.0f, 1.0f); }  // Light Blue
```

#### **Typography System**
- **Title Text**: Roboto Bold, 24px (scaled)
- **Section Headers**: Roboto Bold, 18px (scaled)
- **Regular Text**: Roboto Regular, 14px (scaled)
- **Error Text**: Roboto Regular, 14px, Red color

#### **Layout Structure**
```
TargetContainer (VerticalBox)
└── MainBorder (styled background + padding)
    └── ContentBox (VerticalBox)
        ├── TitleText (styled header)
        ├── VerticalSpacer (section spacing)
        ├── ScrollBox (scrollable content area)
        │   ├── SectionHeader ("Graphics Settings")
        │   ├── ControlWidget (slider/combo/checkbox)
        │   ├── VerticalSpacer (control spacing)
        │   └── ... (more controls)
        └── ActionButtonsRow (Apply, Reset, Back)
```

#### **Smart Value Display**
```cpp
FString FormatSliderValue(float Value, const FSettingsControl& Control)
{
    // Handle special zero text (e.g., "Unlimited" for 0 FPS limit)
    if (Value == 0.0f && !Control.ZeroText.IsEmpty())
        return Control.ZeroText;

    // Apply multiplier (e.g., 0.5 * 100 = "50" for percentages)
    float DisplayValue = Value * Control.ValueMultiplier;

    // Format precision based on step size
    // Add suffix (e.g., "75%", "144 FPS")
}
```

### 📝 Adding New Settings
Modify `Content/Data/SettingsLayout.json`:
```json
{
  "Type": "Slider",
  "Name": "NewSettingSlider",
  "Label": "My New Setting",
  "Min": 0,
  "Max": 10,
  "Step": 1,
  "ShowValue": true,
  "ValueSuffix": " units"
}
```

No C++ recompilation required - changes take effect immediately!

---

## Major Debugging & Fixes

### 🐛 **Critical Issue Resolved: Widget Tree Integration**

**Problem**: DynamicSettingsWidget was creating its own `WidgetTree->RootWidget` which failed to integrate with the parent Blueprint hierarchy, resulting in black screens despite successful widget creation.

**Root Cause**:
```cpp
// BROKEN APPROACH - Creates isolated widget tree
WidgetTree->RootWidget = MyCustomCanvas;
// Widgets created but not visible in parent hierarchy
```

**Solution**:
```cpp
// FIXED APPROACH - Direct container population
void PopulateContainer(UVerticalBox* Container)
{
    Container->ClearChildren();
    Container->AddChild(CreateSliderWithLabel(Control));
    // Widgets properly integrated with parent hierarchy
}
```

**Key Insights**:
- ✅ **Widget Creation** was always working correctly
- ❌ **Widget Tree Integration** was the real issue
- ✅ **Direct Container Population** solves hierarchy problems
- ✅ **Visibility Debugging** helped identify the core issue

### 🔍 **Debugging Process**
1. **Visibility Analysis**: Discovered `ESlateVisibility::Visible = 0`, not visibility issue
2. **Simple Widget Test**: Confirmed basic dynamic creation works
3. **Direct Container Test**: Proved container population approach works
4. **Architecture Refactor**: Moved from widget tree creation to container population

---

## Current Issues & Debugging

### Active Issues

1. **Turret Synchronization** 🔧
   - Turrets fire at the same time
   - Solution implemented: Individual AI controller timing with unique offsets
   - Status: Testing required

2. **Race Widget Visibility** ⚠️
   - Race buttons created but not visible in some cases
   - Issue: Blueprint widget binding mismatch
   - Required widgets in `WBP_RaceButtonWidget`:
     - `RaceButton` (Button)
     - `RaceBorder` (Border)
     - `RaceImage` (Image)
     - `RaceNameText` (Text Block)
     - `RacePhylumText` (Text Block)

3. **Settings Window Resolution Changes** ✅ FIXED
   - Resolution and window mode changes now work properly
   - Mouse calibration fixes cursor offset in windowed mode

4. **Disabled Plugins** ⚙️
   - ChaosCloth, MLDeformerFramework disabled due to ISPC issues on Linux
   - Not needed for core gameplay

---

## TODO List

### High Priority

- [ ] **Test turret staggered firing** - Verify AI timing fixes work
- [ ] **Complete electrical grid system** - Power distribution network
- [ ] **Implement reactor system** - Power generation sources
- [ ] **Add gameplay phase** - Move from game start to actual gameplay
- [ ] **Story/narrative system** - Oregon Trail-style events and decisions
- [ ] **Resource management** - Track and manage colony resources

### Medium Priority

- [ ] **Save tree UI** - Visualize save branching/forking
- [ ] **Save tree branching** - Allow loading from any point and creating branches
- [ ] **Unlock system for races** - Gameplay achievements unlock new phylums
- [ ] **Race-specific scenarios** - Starting situations based on species
- [ ] **Turret upgrade system** - Tech tree integration
- [ ] **Ship customization** - Hardpoint placement and configuration
- [ ] **Projectile visuals** - Proper mesh and texture for bullets
- [ ] **Asteroid visual variety** - Different meshes for sizes/types

### Low Priority

- [ ] **Random race name generator** - Auto-generate colony names per phylum
- [ ] **Hybrid races** - Combine phylum attributes
- [ ] **Custom race creation** - Point-buy system
- [ ] **Achievement system** - Track player accomplishments
- [ ] **Difficulty selection** - Starting condition modifiers
- [ ] **More minigames** - Additional gameplay variety
- [ ] **Multiplayer consideration** - Future expansion possibility

### Polish & Quality of Life

- [ ] **Tutorial system** - Teach game mechanics
- [ ] **Tooltips** - Explain UI elements and mechanics
- [ ] **Sound effects** - Complete audio pass
- [ ] **Music system** - Dynamic soundtrack
- [ ] **Particle effects** - Visual polish
- [ ] **Loading screens** - Better transitions
- [ ] **Localization support** - Multiple languages

---

## Development Guidelines

### Code Style
- Always use C++ over Blueprints
- Break edits into small, manageable chunks
- Request user confirmation before proceeding with large changes
- Use `edit_file` instead of `write_file` (write_file fails for large files)
- Keep files under 100 lines when possible
- For larger files, use iterative editing

### Best Practices
- Don't program features not explicitly requested
- Don't duplicate built-in engine functionality
- Leverage and extend existing UE5 features
- Don't leave test code or fallbacks permanently
- Leave compilation to the user
- Save checkpoints frequently (Claude outages are common)

### File Conventions
- First line comment: Full path and filename
- Use clear, descriptive variable names
- Document complex logic
- Keep functions focused and single-purpose

### Communication
- Request to proceed before editing after reading files
- Check in after a few edits before moving on
- Ask user when in doubt
- Provide full file paths when referencing files

---

## Known Working Features

✅ Race selection with 4 playable species
✅ Tech tree with 6 disciplines
✅ UUID-based save system with 3 types
✅ Dynamic settings menu (JSON-driven)
✅ Resolution and window mode changes
✅ Mouse calibration in windowed mode
✅ Audio volume controls
✅ Graphics quality settings
✅ Game start flow (3 phases)
✅ Turret AI targeting
✅ Asteroid spawning and fragmentation
✅ Hardpoint installation system
✅ Dual-axis turret rotation
✅ Power management framework
✅ Container-based UI population
✅ JSON-to-struct serialization
✅ Responsive UI scaling
✅ Professional styling system

---

## Build Information

**Engine Version**: Unreal Engine 5.6
**Build System**: GNU Make

**Disabled Plugins**:
- ChaosCloth
- ChaosClothEditor
- MLDeformerFramework
- NearestNeighborModel
- NeuralMorphModel
- VertexDeltaModel

**Build Notes**:
- First compile can take 10-30 minutes
- Use `make -j4` for fewer cores if needed
- ISPC issues on Linux handled via plugin disabling

---

## Lessons Learned

### 🎓 **Key Insights**
1. **Widget Tree vs Container Population**: Custom widget trees don't integrate well with Blueprint hierarchies
2. **Debugging Approach**: Start with simplest possible test case and build complexity
3. **Visibility Enum Values**: `ESlateVisibility::Visible = 0`, not intuitive but correct
4. **Dynamic Widget Lifecycle**: `NativeConstruct()` not always called for `CreateWidget<>()`
5. **Architecture Flexibility**: Container-based approach more maintainable than widget tree replacement

### 🛠️ **Best Practices Established**
- **Always test widget creation with simplest possible UI first**
- **Use container population rather than widget tree replacement**
- **Implement comprehensive debug logging for complex UI systems**
- **Design for integration with existing Blueprint hierarchies**
- **Separate concerns: widget creation vs layout management**

---

## Recent Changes (Chronological)

1. Fixed dynamic settings persistence and loading
2. Added race selection system with 11 phylums
3. Implemented tech tree with 6 categories
4. Added UUID-based save system
5. Created checkpoint save types
6. Implemented game start flow controller
7. Added race-based tech point bonuses
8. Fixed resolution change functionality
9. Added mouse calibration system
10. Implemented turret AI with targeting
11. Created asteroid fragmentation system
12. Added hardpoint mounting system
13. Implemented dual-axis turret rotation
14. Fixed turret firing synchronization
15. Added Hy21_mesh docking component
16. Enhanced responsive UI with professional styling
17. Implemented smart value display formatting

---

## Next Session Priorities

1. **Verify race widget visibility** - Ensure Blueprint bindings correct
2. **Test turret staggered firing** - Confirm AI timing works
3. **Begin electrical grid** - Start power distribution system
4. **Gameplay phase transition** - Move from start to actual game

---

*This project demonstrates advanced Unreal Engine C++ programming, JSON-driven UI systems, and the importance of proper architecture when building complex dynamic systems. The combination of strategic gameplay, procedural generation, and minigame mechanics creates a unique space colonization experience.*
