#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include "mmu.h"
#include "pagetable.h"
#include <sstream>
#include <vector>


// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);
void printVariableValue(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, uint8_t *memory);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        std::cerr << "Error: you must specify the page size" << std::endl;
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' (raw array of bytes)
    uint8_t *memory = new uint8_t[PHYSICAL_MEMORY];

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(PHYSICAL_MEMORY);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline(std::cin, command);
    
    while (command != "exit")
    {
        // Parse the incoming command string
        std::stringstream ss(command);
        std::string cmd;
        ss >> cmd;

        // Command routing
        if (cmd == "create") {
            int text_size;
            int data_size;
            ss >> text_size >> data_size;
            createProcess(text_size, data_size, mmu, page_table);
            
        } else if (cmd == "allocate") {
            // allocate <PID> <var_name> <data_type> <number_of_elements>
            uint32_t pid;
            std::string var_name;
            std::string data_type_str;
            uint32_t num_elements;

            ss >> pid >> var_name >> data_type_str >> num_elements;
            
            // Map the string input to the DataType enum
            DataType type;
            if (data_type_str == "char") {
                type= DataType::Char;
            } else if (data_type_str == "short") {
                type = DataType::Short;
            } else if (data_type_str == "int") {
               type = DataType::Int;
            } else if (data_type_str == "float") {
                type = DataType::Float;
            } else if (data_type_str == "long") {
                type = DataType::Long;
            } else if (data_type_str == "double") {
                type = DataType::Double;
            } else {
                std::cout << "Invalid data type" << std::endl;
                std::cout << "> ";
                std::getline(std::cin, command);
            }
            allocateVariable(pid, var_name, type, num_elements, mmu, page_table);
            
        } else if (cmd == "set") {
            // set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N>
            uint32_t pid;
            std::string var_name;
            int offset;
            int value;
            std::vector<std::string> values;
            std::string current_value;

            ss >> pid >> var_name >> offset;
            
            // Collect all trailing value arguments
            while (ss >> current_value) {
                values.push_back(current_value);
            }
            // Sequentially set each value in memory, incrementing the offset
            for (int i = 0; i < values.size(); i++) {
                setVariable(pid, var_name, offset + i, &values[i], mmu, page_table, memory);
            }
            
        } else if (cmd == "free") {
            uint32_t pid;
            std::string var_name;
            ss >> pid >> var_name;

            freeVariable(pid, var_name, mmu, page_table);

        } else if (cmd == "terminate") {
            uint32_t pid;

            ss >> pid;
            terminateProcess(pid, mmu, page_table);

        } else if (cmd == "print") {
            // get the parameter based on how you worked stringstream
            std::string par;
            ss >> par;
            
            // Route the print command to the appropriate helper
            if (par == "mmu") {
                mmu->print();
            } else if (par == "page") {
                page_table->print();
            } else if (par == "processes") {
                std::vector<Process*> processes = mmu->getProcesses();
                for (int i = 0; i < processes.size(); i++) {
                    std::cout << processes[i]->pid << std::endl;
                }
            } else if (par.find(":") != std::string::npos) {
                // Split the <PID>:<var_name> format
                size_t colon_pos = par.find(":");
                uint32_t pidPrint = std::stoi(par.substr(0, colon_pos));
                std::string printVar_name = par.substr(colon_pos + 1);
                
                printVariableValue(pidPrint, printVar_name, mmu, page_table, memory);
            } else {
                std::cout << "Invalid print command" << std::endl;
            }
            
        } else {
            // Error handling for completely invalid top-level commands
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline(std::cin, command);
    }

    // Clean up
    delete[] memory;
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // create new process in the MMU and fetch its PID
    uint32_t pid = mmu->createProcess();
    
    // allocate new required system variables for the <TEXT>, <GLOBALS>, and <STACK>
    allocateVariable(pid, "<TEXT>", DataType::Char, text_size, mmu, page_table);
    allocateVariable(pid, "<GLOBALS>", DataType::Char, data_size, mmu, page_table);
    allocateVariable(pid, "<STACK>", DataType::Char, 65536, mmu, page_table);

    // Output only the PID for successful creation
    std::cout << pid << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
   // Validate the process exists 
   Process* proc = mmu->getProcess(pid);
   if(proc == nullptr){
        std::cout << "error: process not found" << std::endl;
        return;
    }
    
    // Prevent duplicate variable names within the same process
    for (int i = 0; i < proc->variables.size(); i++){
        if(proc->variables[i]->name == var_name){
            std::cout << "error: variable already exists" << std::endl;
            return;
        }
    }
    
    // calculate total size of variable based on data type bounds
    uint32_t size = 0;
    switch(type){
        case DataType::Char:
            size = 1;
            break;
        case DataType::Short:
            size = 2;
            break;
        case DataType::Int:
            size = 4;
            break;
        case DataType::Float:
            size = 4;
            break;
        case DataType::Long:
            size = 8;
            break;
        case DataType::Double:
            size = 8;
            break;
        default:
            size = num_elements;
            break;
    }
    size *= num_elements;

    // Track state for finding memory holes
    uint32_t allocated_address = 0;
    bool found = false;
    Variable* target_free_space = nullptr; // Track the hole so we can safely shrink it later

    // Iterate to find the first free hole (First Fit Algorithm)
    for (int i = 0; i < proc->variables.size(); i++){
        Variable* var = proc->variables[i];
        
        if(var->type == DataType::FreeSpace && var->size >= size){
            allocated_address = var->virtual_address;
            target_free_space = var;
            found = true;
            break; // First fit algorithm stops at first match
        }
    }

    // if no free hole found, we are out of memory print an error message
    if(!found){
        std::cout << "error: out of memory" << std::endl;
            return;
    }

    // Check if the allocation physically exceeds the maximum system boundary
    if (allocated_address + size > PHYSICAL_MEMORY) {
            std::cout << "error: out of memory" << std::endl;
            return; 
    }

    // Safe to Shrink the free space now that bounds are confirmed
    target_free_space->virtual_address += size;
    target_free_space->size -= size;

    // Calculate logical page boundaries for the new allocation
    uint32_t start_page = allocated_address / page_table->getPageSize();
    uint32_t end_page = (allocated_address + size - 1) / page_table->getPageSize();


    // Map necessary pages in the page table across the entire span
    for(uint32_t current_page = start_page; current_page <= end_page; current_page++){
        uint32_t physical_address = current_page * page_table->getPageSize();

        // If page does not exist yet, allocate a new one
            if(page_table->getPhysicalAddress(pid, physical_address) == -1){
            page_table->addEntry(pid, current_page);
            }
    }


    // Print and insert MMU entry tracking
    mmu->addVariableToProcess(pid, var_name, type, size, allocated_address);
    
    // Only print address for user-defined variables (hiding system vars)
    if (var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>") {
        std::cout << allocated_address << std::endl;
    }
}


void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    // Validate the process exists
    Process* proc = mmu->getProcess(pid);
    if(proc == nullptr){
        std::cout << "error: process not found" << std::endl;
        return;
    }
    
    // look up variable by name, ignoring FreeSpace holes
    Variable *var = nullptr;
    for (int i = 0; i < proc->variables.size(); i++){
        if(proc->variables[i]->name == var_name && proc->variables[i]->type != DataType::FreeSpace){
            var = proc->variables[i];
            break;
        }
    }
    if (var== nullptr){
        std::cout << "error: variable not found" << std::endl;
        return;
    }
    
    // get element size based on underlying data type for offset calculation
    uint32_t element_size = 0;
    switch(var->type){
        case DataType::Char:
            element_size = 1;
            break;
        case DataType::Short:
            element_size = 2;
            break;
        case DataType::Int:
            element_size = 4;
            break;
        case DataType::Float:
            element_size = 4;
            break;
        case DataType::Long:
            element_size = 8;
            break;
        case DataType::Double:
            element_size = 8;
            break;
        default:
            element_size = 1;
            break;
    }
    
    // Prevent writing outside the bounds of the allocated array
    uint32_t total_elements = var->size / element_size;
    if (offset >= total_elements) {
        std::cout << "error: index out of range" << std::endl;
        return;
    }

    // calculate precise virtual address with the offset
    uint32_t virtual_address = var-> virtual_address + (element_size * offset); 
    
    // Convert to physical frame mapping
    int physical_add = page_table->getPhysicalAddress(pid, virtual_address);
    if(physical_add == -1){
        std::cout << "error: invalid address" << std::endl;
        return;
    }

    // Unmask the generic void pointer back into a readable C++ string
    std::string value_str = *(static_cast<std::string*>(value));

    // Convert string to correct numeric type and copy directly into physical memory
    try {
        if (var->type == DataType::Char) {
            char val = value_str[0]; 
            memcpy(&memory[physical_add], &val, element_size);
        } else if (var->type == DataType::Short) {
            short val = (short)std::stoi(value_str); 
            memcpy(&memory[physical_add], &val, element_size);
        } else if (var->type == DataType::Int) {
            int val = std::stoi(value_str); 
            memcpy(&memory[physical_add], &val, element_size);
        } else if (var->type == DataType::Float) {
            float val = std::stof(value_str); 
            memcpy(&memory[physical_add], &val, element_size);
        } else if (var->type == DataType::Long) {
            long long val = std::stoll(value_str);
            memcpy(&memory[physical_add], &val, element_size);
        } else if (var->type == DataType::Double) {
            double val = std::stod(value_str); 
            memcpy(&memory[physical_add], &val, element_size);
        }
    } catch (...) {
        std::cout << "error: invalid data format" << std::endl;
    }
}   


void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // Validate process exists
    Process* proc = mmu->getProcess(pid);
    if (proc == nullptr){
        std::cout << "error: process not found" << std::endl;
        return;
    } 

    uint32_t var_address = 0;
    uint32_t var_size = 0;
    bool found = false;

    // Locate target variable and convert it back into recyclable FreeSpace
    for (auto i = proc->variables.begin(); i != proc->variables.end(); ++i) {
        if ((*i)->name == var_name && (*i)->type != DataType::FreeSpace) {
            var_address = (*i)->virtual_address;
            var_size = (*i)->size;
            found = true;

            (*i)->name = "<FREE_SPACE>";
            (*i)->type = DataType::FreeSpace;
            break; 
        }
    }

    // Abort if variable wasn't present
    if (!found) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    // Determine the page boundary span of the newly freed memory
    uint32_t page_size = page_table->getPageSize();
    uint32_t start_page = var_address / page_size;
    uint32_t end_page = (var_address + var_size - 1) / page_size;

    // Evaluate each freed page to see if it can be completely unmapped
    for (uint32_t current_page = start_page; current_page <= end_page; ++current_page) {
        bool page_in_use = false;

        // Check against every other active variable in the process
        for (int i = 0; i < proc->variables.size(); ++i) {
            Variable* rem_var = proc->variables[i];
            
            if (rem_var->type == DataType::FreeSpace) continue; 

            uint32_t rem_start = rem_var->virtual_address / page_size;
            uint32_t rem_end = (rem_var->virtual_address + rem_var->size - 1) / page_size;

            // If an active variable overlaps this page, flag it as in use
            if (current_page >= rem_start && current_page <= rem_end) {
                page_in_use = true;
                break; 
            }
        }

        // Only free the physical frame if it is entirely unoccupied
        if (!page_in_use) {
            page_table->removeEntry(pid, current_page); 
        }
    }
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // Validate process exists before attempting termination
    Process* proc = mmu->getProcess(pid);
    if (proc == nullptr){
        std::cout << "error: process not found" << std::endl;
        return;
    }
    
    // Fetch all active variables and sequentially free them
    std::vector<std::string> var_names = mmu->getVariableNamesForProcess(pid);
    for(int i = 0; i < var_names.size(); i++){
        freeVariable(pid, var_names[i], mmu, page_table);
    }
    
    // Once memory is flushed, destroy the process tracker entirely
    mmu->removeProcess(pid);
}



void printVariableValue(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    // Validate process existence
    Process* proc = mmu->getProcess(pid);
    if(proc == nullptr) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    // Locate variable by name
    Variable *var = nullptr;
    for (int i = 0; i < proc->variables.size(); i++){
        if(proc->variables[i]->name == var_name){
            var = proc->variables[i];
            break;
        }
    }
    if (var == nullptr) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    // Identify underlying data type size for memory extraction
    uint32_t element_size = 0;
    switch(var->type){
        case DataType::Char:element_size = 1; 
        break;
        case DataType::Short:element_size = 2; 
        break;
        case DataType::Int:element_size = 4; 
        break;
        case DataType::Float:element_size = 4; 
        break;
        case DataType::Long:element_size = 8; 
        break;
        case DataType::Double:element_size = 8; 
        break;
        default:element_size = 1; 
        break;
    }

    // Calculate display limit (maximum 4 array elements)
    int total_elements = var->size / element_size;
    int print_limit = (total_elements > 4) ? 4 : total_elements;

    // Loop through elements, translating and extracting each one
    for (int i = 0; i < print_limit; i++) {
        uint32_t virtual_address = var->virtual_address + (i * element_size);
        int physical_add = page_table->getPhysicalAddress(pid, virtual_address);

        if (physical_add == -1) {
            std::cout << "error: invalid physical address" << std::endl;
            return;
        }

        // Extract raw bytes into the correct C++ variable type and print
        if (var->type == DataType::Char) {
            char val; 
            memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        } else if (var->type == DataType::Short) {
            short val; 
            memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        } else if (var->type == DataType::Int) {
            int val;
             memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        } else if (var->type == DataType::Float) {
            float val;
            memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        } else if (var->type == DataType::Long) {
            long val; 
            memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        } else if (var->type == DataType::Double) {
            double val; 
            memcpy(&val, &memory[physical_add], element_size);
            std::cout << val;
        }

        // Format with commas, suppressing the final trailing comma
        if (i < print_limit - 1) {
            std::cout << ", ";
        }
    }

    // Append truncation suffix if the array exceeded the print limit
    if (total_elements > 4) {
        std::cout << ", ... [" << total_elements << " items]";
    }
    std::cout << std::endl;
}