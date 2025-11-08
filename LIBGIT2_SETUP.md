# libgit2 Setup for AstroChicken Save System

The AstroChicken save system uses **libgit2** to provide git-based version control for game saves. This enables advanced features like save branching, save trees, and time-traveling through your gameplay.

## Why libgit2?

- **Native Branching**: Perfect for the game's save tree and branching narrative
- **Automatic History**: Every save is a commit with full history tracking
- **Visualize Playthroughs**: See your entire decision tree
- **Time Travel**: Go back to any point and create new branches
- **Data Integrity**: SHA hashes ensure saves aren't corrupted
- **Compression**: Git automatically compresses save data

## Installation

### Linux (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install libgit2-dev
```

### Linux (Fedora/RHEL)

```bash
sudo dnf install libgit2-devel
```

### Linux (Arch)

```bash
sudo pacman -S libgit2
```

### Verify Installation

```bash
# Check if library is installed
ldconfig -p | grep libgit2

# Check header files
ls /usr/include/git2.h

# Check pkg-config
pkg-config --modversion libgit2
```

## Build Configuration

The build system is already configured to link against libgit2 on Linux:

**Source/astrochicken/astrochicken.Build.cs:**
```csharp
if (Target.Platform == UnrealTargetPlatform.Linux)
{
    PublicAdditionalLibraries.Add("git2");
    PublicSystemIncludePaths.Add("/usr/include");
}
```

## Save System Architecture

### Repository Structure

```
SaveGames/
└── {GameUUID}/              # One repository per game run
    ├── .git/                # Git repository data
    ├── Metadata.json        # Save metadata
    ├── RaceSelection.json   # Race and colony info
    ├── TechTree.json        # Tech progression
    ├── PlayerProgress.json  # Story/events
    └── GameState.json       # Core game state
```

### Git Branching Model

- **main**: Primary playthrough timeline
- **auto-save**: Rotating auto-saves (keeps last 3)
- **gamerun/{UUID}/branch-{N}**: Player-created save branches

### Save Types

1. **Manual Saves**: User-named saves with timestamps
2. **Auto Saves**: Automatic saves on the auto-save branch (rotating, keeps 3)
3. **Checkpoint Saves**: Story-critical saves, tagged for easy access
   - NewGame, Prologue, Act1-5, Ending

### Example Git History

```
* checkpoint/Act1 (tag: checkpoint/Act1)
|
* manual-save "Before attacking asteroid field"
|
|\
| * branch-001 "Tried aggressive expansion" (alternate timeline)
|
* checkpoint/Prologue (tag: checkpoint/Prologue)
|
* checkpoint/NewGame (tag: checkpoint/NewGame) - "Chordata - Colony: New Hope"
```

## Features

### Core Operations

- ✅ **CreateNewGame()**: Initialize git repo for new game
- ✅ **ManualSave()**: Commit with user message
- ✅ **AutoSave()**: Rotating commits on auto-save branch
- ✅ **CheckpointSave()**: Tagged commits for story milestones
- 🚧 **LoadSave()**: Checkout specific commit
- 🚧 **CreateSaveBranch()**: Create new timeline from any save
- 🚧 **GetSaveTree()**: Visualize entire save history

### Save Data

All game state is serialized to JSON files:
- Race selection and colony name
- Tech tree progression and unlocked techs
- Player progress and story state
- Full game state snapshot

### Integration

The save system automatically integrates with:
- **RaceManager**: Saves race selection and colony info
- **TechTreeManager**: Saves tech points and unlocked technologies
- **GameStartFlow**: Creates initial "NewGame" checkpoint

## Troubleshooting

### "libgit2 not found" error during compilation

Install libgit2-dev:
```bash
sudo apt-get install libgit2-dev
```

### Runtime errors about git operations

Check that the SaveGames directory is writable:
```bash
mkdir -p SaveGames
chmod 755 SaveGames
```

### "Failed to initialize libgit2"

Ensure the library is in your library path:
```bash
export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH
```

## Future Enhancements

- [ ] Save tree visualization UI
- [ ] Save description editing
- [ ] Save thumbnail screenshots
- [ ] Cloud sync via git remote
- [ ] Save compression optimization
- [ ] Multi-game save browser

## Credits

- **libgit2**: https://libgit2.org/
- Git-based save systems inspired by: Braid, The Talos Principle

---

*For more information about the save system implementation, see:*
- `Source/astrochicken/SaveSystem/SaveSystem.h`
- `Source/astrochicken/SaveSystem/GitRepository.h`
- `Source/astrochicken/SaveSystem/SaveData.h`
