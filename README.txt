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
  Edit config.txt or processes.txt in this folder, save it, then press Run
  again. The build step only copies them next to the exe, so nothing gets
  recompiled. When you run the exe directly instead, edit the copies next to
  the exe.

  window_width, window_height  window size in pixels
  fullscreen                   true/false, fills the main monitor
  os_name                      shown on the desktop, terminal and shutdown screen
  user_name                    start menu, terminal prompt and whoami
  wallpaper                    image path next to the exe, a missing one falls back to a gradient
  desktop_icons                true/false, the desktop shortcuts
  taskbar_position             top or bottom
  taskbar_centered             true/false, centers the start and app buttons
  clock_position               taskbar, top_left, top_right, bottom_left or bottom_right
  clock_24h                    true/false
  clock_seconds                true/false
  date_format                  strftime format of the long date, e.g. %A, %B %d, %Y
  volume                       starting volume, 0 to 100
  brightness                   starting brightness, 10 to 100
  wifi, bluetooth, night_light starting quick settings, true/false
  terminal_size                starting window size, e.g. 640x400
  file_explorer_size           same, for File Explorer
  task_manager_size            same, for Task Manager
  process_file                 Task Manager's background processes, see below
  total_memory_mb              RAM used for Task Manager's memory percentages
  update_speed                 Task Manager refresh: high, normal, low or paused
  boot_screens                 true/false, BIOS and splash screens before the desktop
  cpu_name                     BIOS screen and Task Manager's Performance tab
  bios_memory_kb               the BIOS memory test count
  fun_fact                     last BIOS line, empty hides it
  splash_seconds               how long the splash screen stays, 0 to 60

  processes.txt has one background process per line:
    name, cpu %, memory MB, disk MB/s, network Mbps[, running or suspended]
  A missing file keeps the built-in list. An empty one shows no background
  processes.

  An unknown key or a bad value or line prints a warning in the console and
  keeps the default.

Using it
  - Any key or click skips the BIOS and splash screens.
  - The taskbar buttons (or start) open Terminal, File Explorer and Task Manager.
    Clicking the button of the app in front minimizes it.
  - Terminal commands: help, clear, echo, date, ver, whoami, ps, open <app>,
    ping <host>, ipconfig, exit.
  - The tray's speaker and Wi-Fi icons open volume and quick settings, and the
    tray clock opens a calendar.
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
