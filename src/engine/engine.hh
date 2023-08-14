#pragma once

#include <functional>

namespace engine {

void init();
void cleanup();

// add a method to execute before deleting the gl context.
void add_cleanup_task(std::function<void()> const &);

}
