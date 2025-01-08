# Bermuda
An expedition team never returned from exploring an unknown area at the bottom of the ocean. You are dispatched to go investigate. While exploring in your submarine, you’re attacked by a mysterious creature and the submarine is torn apart. You find yourself surrounded by the debris of the expedition team’s submarines and equipment trailing off towards a cave-like entrance
All you have left is an oxygen tank, an atmospheric diving suit, and a harpoon gun to explore through these caves and figure out what happened.
  
Bermuda is an underwater roguelike where survival hinges on managing your oxygen supply. Dive deep into procedurally generated challenges, evade dangers, and explore the depths. Built entirely on a custom game engine developed from scratch using `C++`, `OpenGL`, `CMake`, and `SDL2`.

### Credits:
- Andy Li [@andyyyli](https://github.com/AndyyyLi)
- Bob Pham [@bob-pham](https://github.com/bob-pham)
- Brandon Yih
- Caeleb Koharjo [@caelebk](https://github.com/caelebk)
- David Sopheap [@gmdvdsp](https://github.com/gmdvdsp)
- Tony Zhang [@TonyZhang1112](https://github.com/TonyZhang1112)

# Requirements
Most of the requirements have been included as header libaries. The follow may or may not need to be installed

### General requirements:
- `CMake`
- `C++14`

### Windows
Hitting `Build` within Visual Studio should be enough, but otherwise you may have to install [C++ CMake Tools for Windows](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

### MacOS
The following packages (may be) required:
- `pkg-config`
- `glfw3`
- `sdl2`
- `sdl2_mixer`

### Linux
The following packages are required:
- `ibglfw3-dev`
- `libsdl2-dev`
- `libsdl2-mixer-dev`

# Building
### Windows
Clone the repository, then open and compile using Visual Studio's tools.

### MacOS/Linux
1. Run CMake
```shell
cmake .
```
2. Build the Game
```shell
make
```
3. Run the game
```shell
./bermuda
```

* Not for MacOS: Do not build/run using Rosetta

# Gallery
**Starting Screen**
![image](https://github.com/user-attachments/assets/f2d7a982-de29-437f-857f-dd4b0e133ea5)

**Tutorial**
![image](https://github.com/user-attachments/assets/2211f5b1-410b-4163-8a5d-d625b16be160)

**Misc Level 1 Room**
![image](https://github.com/user-attachments/assets/f68b2d4e-2d61-4bcb-9291-38585fd4cbab)

**Misc Level 2 Room**
![image](https://github.com/user-attachments/assets/f4d77dea-9ad7-4a1b-96ed-0cfe79727c16)

**Misc Level 3 Room**
![image](https://github.com/user-attachments/assets/15ffc6ed-b511-45f5-9925-bce0790ac845)

**MiniBoss 1**
![image](https://github.com/user-attachments/assets/31403840-1f42-4aab-9fab-60a84bdb5d85)

**MiniBoss 2**
![image](https://github.com/user-attachments/assets/c3624e29-0629-4ddd-a721-d816e4a286c3)
![image](https://github.com/user-attachments/assets/c78edd5b-6fb1-4037-8f51-ce6003b615a2)

**Final Boss**
![image](https://github.com/user-attachments/assets/bc396f90-0c6a-466b-bf7a-eeb033f3421e)
![image](https://github.com/user-attachments/assets/2f74971a-17b1-443d-93b6-f1802d2fc02f)



