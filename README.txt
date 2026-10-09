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

Changing parameters without recompiling
  Edit config.txt in this folder, save it, then press Run again. The build step
  only copies config.txt next to the exe, so nothing gets recompiled. When you
  run the exe directly instead, edit the config.txt next to the exe.

  window_width, window_height  window size in pixels
  fullscreen                   true/false, fills the main monitor
  os_name                      shown on the desktop, terminal and shutdown screen
  wallpaper                    image path next to the exe, a missing one falls back to a gradient
  taskbar_position             top or bottom
  clock_24h                    true/false
  clock_seconds                true/false
  boot_screens                 true/false, BIOS and splash screens before the desktop

  An unknown key or a bad value prints a warning in the console and keeps the default.

Using it
  - Any key or click skips the BIOS and splash screens.
  - The taskbar buttons (or start) open Terminal, File Explorer and Task Manager.
    Clicking the button of the app in front minimizes it.
  - Terminal commands: help, clear, echo, date, ver, whoami, ps, open <app>, exit.
  - Task Manager: click a column header to sort, pick a row, then End task.
  - The window's X button and Alt+F4 do nothing. Shut down with PWR (or
    start > Turn Off Computer), then Turn Off.

Source layout
  src/main.cpp   window, frame loop, boot -> desktop -> shutdown states
  src/Config.*   config.txt loader
  src/core/      app table, clock, icons, power state, textures, theme
  src/shell/     boot and shutdown screens, desktop, taskbar
  src/apps/      Terminal, File Explorer, Task Manager
  Adding an app means writing its draw function and adding one row to the
  table in src/core/Apps.cpp.
