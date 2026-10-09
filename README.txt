CSOPESY Desktop-Style OS Mock-up (Semi-Major Output 2)

Group
  Rainer Gonzaga
  Aaron James Gonzales
  Mariel Yasumuro
  Richmond Jose Ramos

Entry point
  src/main.cpp (main() is here)

Built with
  C++17, GLFW 3.5.1, OpenGL, Dear ImGui 1.92.9b.
  stb_image (third_party/) loads the wallpaper from assets/.
  CMake clones GLFW and Dear ImGui with git on the first build, so that build
  needs internet and git, and takes a minute longer.

Running in Visual Studio 2022
  Needs the "Desktop development with C++" workload (it includes the CMake tools).
  1. File > Open > Folder..., then pick this folder.
  2. Wait for "CMake generation finished" in the Output window.
  3. Pick CsopesyOS.exe from the startup item dropdown next to the green arrow.
  4. Press the green arrow (Run).

Running from a terminal (MinGW + Ninja, or any CMake setup)
  cmake -S . -B build -G Ninja
  cmake --build build
  build\CsopesyOS.exe
