#include <iostream>
#include <algorithm>
#include "mmu.h"
#include <iomanip>

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

    if (proc != NULL)//it != _processes.end()
    {
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
                std::cout << std::left << std::setw(6) << _processes[i]->pid << "|"
                          << " " << std::left << std::setw(14) << var->name << "|"
                          << "   0x" << std::right << std::setfill('0') << std::setw(8) << std::hex << var->virtual_address << std::setfill(' ') << " |"
                          << std::right << std::setw(12) << std::dec << var->size << std::endl;
            }
            //using printf
        }
    }
}
