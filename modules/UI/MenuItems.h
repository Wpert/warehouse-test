#ifndef MENUITEMS_H
#define MENUITEMS_H

#include <string>
#include <functional>

struct MenuItem {
    std::string name;
    std::function<void()> handler;
};

#endif
