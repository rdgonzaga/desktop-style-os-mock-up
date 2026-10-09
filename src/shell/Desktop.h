#pragma once

// The base layer of the OS: drawn first every frame, behind every window.
namespace desktop {

void init();
void draw();
void shutdown();

}
