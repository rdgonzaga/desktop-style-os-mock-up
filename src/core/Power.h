#pragma once

namespace power {

enum class State { Booting, Running, ShuttingDown, Off };

inline State state = State::Booting;

}
