#pragma once

#include <queue>
#include <vpad/input.h>

class InputSystem
{
private:
    std::queue<VPADStatus> list;

public:
    void push(const VPADStatus& status);
    std::vector<VPADStatus> consumeAll();
};