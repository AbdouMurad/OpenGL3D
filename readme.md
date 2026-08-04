# OpenGL3D Setup Guide

This guide explains how to set up the development environment for OpenGL3D.

## Requirements

Install the following:

* Visual Studio 2022 Community (or higher)
* C++ development tools
* CMake
* vcpkg

---

# 1. Install Visual Studio 2022

Download and install:

https://visualstudio.microsoft.com/vs/community/

During installation, select:

```
Desktop development with C++
```

Make sure these components are installed:

```
✓ MSVC v143 - VS 2022 C++ x64/x86 build tools
✓ C++ CMake tools for Windows
✓ Windows 10/11 SDK
```

---

# 2. Install vcpkg

Clone and install vcpkg:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
```

Build vcpkg:

```powershell
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

Enable Visual Studio integration:

```powershell
.\vcpkg integrate install
```

Verify installation:

```powershell
vcpkg version
```

---

# 3. Open the Project

Open Visual Studio 2022.

Select:

```
File
 → Open
 → Folder
```

Select the OpenGL3D project folder.

The project should contain:

```
OpenGL3D/
├── CMakeLists.txt
├── vcpkg.json
├── src/
└── assets/
```

Visual Studio will automatically detect the CMake project and configure the required dependencies.

---

# 4. Build the Project

Select:

```
x64-Debug
```

from the configuration dropdown.

Build using:

```
Build
 → Build All
```

or:

```
Ctrl + Shift + B
```

---

# 5. Run the Project

Start debugging:

```
F5
```

Run without debugging:

```
Ctrl + F5
```

---

# Troubleshooting

## CMake Configuration Issues

Delete the generated build folder:

```
OpenGL3D/build
```

Then reopen Visual Studio and allow CMake to configure again.

---

## vcpkg Not Found

Verify vcpkg is installed:

```
C:\vcpkg\vcpkg.exe
```

The CMake toolchain file should exist:

```
C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

---

## Wrong Visual Studio Version

This project is configured for:

```
Visual Studio 2022
MSVC v143 toolset
```

If another Visual Studio version is detected, delete the build folder and reopen the project using Visual Studio 2022.

---

# Dependencies

Dependencies are managed using vcpkg.

Packages are defined in:

```
vcpkg.json
```

Dependencies should be installed automatically when CMake configures the project.
