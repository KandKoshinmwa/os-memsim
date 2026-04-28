#include <iostream>
#include <algorithm>
#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    std::vector<Process*>::iterator it = std::find_if(_processes.begin(), _processes.end(), [pid](Process* p)
    { 
        return p != nullptr && p->pid == pid; 
    });

   if (it != _processes.end())
    {
        proc = *it; // Assign proc here!
        Variable *var = new Variable();
        var->name = var_name;
        var->type = type;
        var->virtual_address = address;
        var->size = size;
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            Variable* var = _processes[i]->variables[j];
            // TODO: print all variables (excluding those of type DataType::FreeSpace)
            if (var->type != DataType::FreeSpace)
            {
                printf("%-5d | %-13s |   0x%08x | %10d\n", 
                       _processes[i]->pid, 
                       var->name.c_str(),     // Remember .c_str() for std::string!
                       var->virtual_address, 
                       var->size);
            }
            //using printf
        }
    }
}
Process* Mmu::getProcess(uint32_t pid)
{
    std::vector<Process*>::iterator it = std::find_if(_processes.begin(), _processes.end(), [pid](Process* p)
    { 
      return p != nullptr && p->pid == pid;
    });
    if (it != _processes.end())
    {
        return *it;
    }
    return nullptr;
}

std::vector<std::string> Mmu::getVariableNamesForProcess(uint32_t pid)
{
    std::vector<std::string> names;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                names.push_back(_processes[i]->variables[j]->name);
            }
            break;
        }
    }
    return names;
}

void Mmu::removeProcess(uint32_t pid)
{
    for (auto it = _processes.begin(); it != _processes.end(); ++it)
    {
        if ((*it)->pid == pid)
        {
            // Free the memory allocated for the variables array
            for (int i = 0; i < (*it)->variables.size(); i++) {
                delete (*it)->variables[i];
            }
            // Free the process itself
            delete *it;
            // Remove the pointer from the vector
            _processes.erase(it);
            break;
        }
    }
}

//helper method for getting processes
std::vector<Process*> Mmu::getProcesses()
{
    return _processes;
}

