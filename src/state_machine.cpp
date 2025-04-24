
#include "state_machine.h"
#include "state.h"

StateMachine::~StateMachine()
{
    if (m_States.size() > 0) {
        for (std::pair<const char*, State*> state : m_States) {
            delete state.second;
            state.second = nullptr;
        }

        m_ActiveState = nullptr;
        m_States.clear();
    }
}

void StateMachine::Destroy()
{
    if (m_States.size() > 0) {
        for (std::pair<const char*, State*> state : m_States) {
            delete state.second;
            state.second = nullptr;
        }

        m_ActiveState = nullptr;
        m_States.clear();
    }
}

void StateMachine::AddState(const char* name, State* state)
{
    if (state) {
        state->m_Machine = this;
        m_States.emplace(std::pair<const char*, State*>(name, state));
    }

    if (m_States.size() == 1) {
        m_ActiveState = state;
    }
}

void StateMachine::MakeActive(const char* state_name)
{
    State* state = m_States.at(state_name);
    if (state) {
        m_ActiveState = state;
    }
}

void StateMachine::RemoveState(const char* state_name)
{
    State* state = m_States.at(state_name);
    if (state) {
        m_States.erase(state_name);
        if (state == m_ActiveState) {
            m_ActiveState = nullptr;
        }

        state->m_Machine = nullptr;
        delete state;
    }
}

void StateMachine::Update(PersistData* data)
{
    if (m_ActiveState) {
        m_ActiveState->Update(data);
    }
}
