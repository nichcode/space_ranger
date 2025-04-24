
#pragma once

#include <map>

struct PersistData;
class State;
class StateMachine
{
private:
    std::map<const char*, State*> m_States;
    State* m_ActiveState;

public:
    ~StateMachine();
    void AddState(const char* name, State* state);
    void RemoveState(const char* state_name);

    void Destroy();

    void MakeActive(const char* state_name);
    void Update(PersistData* data);
};