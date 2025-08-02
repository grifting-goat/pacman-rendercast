# Pac-Man RenderCast

> A **3D raycasting Pac-Man** game that brings the classic arcade experience into three dimensions using old school rendering techniques.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B-orange)

## Overview

This project implements a **Pac-Man-style game** using **raycasting** technology to simulate a 3D environment in a console window. The game features physics with momentum-based movement, intelligent ghost AI with pathfinding, and interactive environment manipulation.

## System Requirements

- **OS**: Windows (cmd/PowerShell)
- **Compiler**: MSVC (cl.exe) or MinGW (g++)
- **Terminal Size**: 160x50 characters (adjustable in code)

> **Important**: Make sure your terminal window matches the screen dimensions or the display will appear distorted.

## Controls

### Mouse Controls
| Input | Action |
|-------|--------|
| **Mouse Movement** | Rotate camera/view direction |
| **Left Click** | Fire ray gun (destroy walls) |
| **Right Click** | Activate ice skating mode |

### Keyboard Controls
| Key(s) | Action |
|--------|--------|
| **W A S D** | Strafe movement |
| **← →** | Rotate view (fallback) |
| **Space** | Ice skating (reduce friction) |
| **↑** | Remove walls with ray gun |
| **↓** | Place walls with ray gun |

## Building & Running

### Using VS Code Tasks
1. Open the project in VS Code
2. Press `Ctrl+Shift+B` to build
3. Or use `Ctrl+Shift+P` → "Tasks: Run Task" → "Build and Run C++"

### Manual Compilation
```bash
# Using MSVC
cl.exe /Zi /EHsc /nologo /FepackManRenderCaster.exe pacManRenderCaster.cpp

# Using MinGW
g++ -fdiagnostics-color=always -g pacManRenderCaster.cpp -o pacManRenderCaster.exe
```

## Features

### Advanced AI
- **Breadth-First Search (BFS)** pathfinding for ghost navigation
- **Line-of-sight detection** for dynamic ghost behavior
- **Smart targeting** with different ghost personalities

### Physics Engine
- **Momentum-based movement** with realistic physics
- **Collision detection** with wall bouncing
- **Friction simulation** with variable coefficients
- **Force vectors** and acceleration

### Graphics & Rendering
- **Raycasting engine** for 3D visualization
- **Distance-based shading** for depth perception
- **Corner detection** for realistic wall rendering
- **Minimap** overlay for navigation
- **Dynamic lighting** effects

### Interactive Environment
- **Real-time wall editing** with ray gun
- **Coin collection** system
- **Power-up berries** for ghost-scaring mode
- **Lives system** with heart display

## Game Mechanics

### Collectibles
- **Coins (.)**: Increase score by 1 point each
- **Power Berries (B)**: Worth 10 points, make ghosts scared for 12 seconds

### Ghost Behavior
- **Normal Mode**: Ghosts chase the player intelligently
- **Scared Mode**: Ghosts flee from the player (eating them gives 25 points)
- **Pathfinding**: Uses BFS to navigate around walls

### Physics
- **Momentum**: Movement has realistic inertia
- **Friction**: Adjustable surface friction (ice skating mode reduces it)
- **Bouncing**: Collision with walls causes silly bouncing

## Customization

Key variables in the code for customization:

```cpp
// Screen dimensions
const int nScreenX = 160;  // Console width
const int nScreenY = 50;   // Console height

// Map dimensions  
const int nMapX = 28;      // Map width
const int nMapY = 31;      // Map height

// Mouse sensitivity
float mouseSensitivity = 0.002f;

// Physics constants
float fPMewBase = 0.65f;   // Base friction coefficient
float fForceBase = 700.0f; // Base movement force
```

