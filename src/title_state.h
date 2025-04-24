
#pragma once 

#include "state.h"

class TitleState : public State
{
public:
    void Update(PersistData* data) override;
};