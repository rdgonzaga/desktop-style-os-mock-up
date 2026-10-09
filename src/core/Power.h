#pragma once

namespace power {

enum class State { Booting, Running, Off };

inline State state = State::Booting;

}
