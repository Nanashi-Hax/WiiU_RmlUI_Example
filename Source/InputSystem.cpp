#include "InputSystem.hpp"
#include "vpad/input.h"

void InputSystem::push(const VPADStatus& status)
{
    list.push(std::move(status));
}

std::vector<VPADStatus> InputSystem::consumeAll()
{
    std::vector<VPADStatus> result;
    result.reserve(list.size());

    while (!list.empty())
    {
        result.push_back(std::move(list.front()));
        list.pop();
    }
    return result;
}