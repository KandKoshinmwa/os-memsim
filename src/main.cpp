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
        // Handle command
        // TODO: implement this!

        std::stringstream ss(command);
        std::string cmd;
        ss >> cmd;

        if(cmd == "create"){
            int text_size;
            int data_size;
            ss >> text_size >> data_size;
            createProcess(text_size, data_size, mmu, page_table);
            //either create Process will print the PID or we can add it here
            
        }else if (cmd == "allocate"){
            //allocate <PID> <var_name> <data_type> <number_of_elements>
            uint32_t pid;
            std::string var_name;
            std::string data_type_str;
            uint32_t num_elements;

            ss >> pid >> var_name >> data_type_str >>num_elements;
            
            DataType type;

            if(data_type_str == "char"){
                type= DataType::Char;
            }else if(data_type_str == "short"){
                type = DataType::Short;
            }else if(data_type_str == "int"){
               type = DataType::Int;
            }else if(data_type_str == "float"){
                type = DataType::Float;
            }else if(data_type_str == "long"){
                type = DataType::Long;
        }     else if(data_type_str == "double"){
                type = DataType::Double;
            }else{
                std::cout << "Invalid data type" << std::endl;
                 std::cout << "> ";
                std::getline(std::cin, command);
            }
            allocateVariable(pid, var_name, type, num_elements, mmu, page_table);
        }else if (cmd == "set"){
            //set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N>
            uint32_t pid;
            std::string var_name;
            int offset;
            int value;
            std::vector<std::string> values;
            std::string current_value;


            ss>> pid>> var_name >> offset;
            
            while(ss >> current_value){
                //we want to convert it to an integer here, change maybe needed if we implement parsing in the function.
                values.push_back(current_value);
            }
            for(int i = 0; i < values.size(); i++){
                setVariable(pid, var_name, offset + i, &values[i], mmu, page_table, memory);
            }
        }else if (cmd == "free"){
            uint32_t pid;
            std::string var_name;
            ss >> pid >> var_name;

            freeVariable(pid,var_name,mmu,page_table);

        }else if (cmd == "terminate"){
            uint32_t pid;

            ss >> pid;
            terminateProcess(pid,mmu,page_table);

        }else if (cmd == "print"){
            // get the parameter based on how you worked stringstream
            std::string par;
            ss >> par;
            if(par == "mmu"){
                mmu ->print();
            }
            else if (par == "page"){
                page_table->print();
            }
            //Error handling: Should we add error message if use input "print <wrong name>", even on other cmds
           
        }else{
            std::cout << "Invalid command" << std::endl;
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
    // TODO: implement this!
    //   - create new process in the MMU
        uint32_t pid = mmu->createProcess();
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //NOTE: will need to implement allocateVariable() first for this to work
    allocateVariable(pid, "<TEXT>", DataType::Char, text_size, mmu, page_table);
    allocateVariable(pid, "<GLOBALS>", DataType::Char, data_size, mmu, page_table);
    allocateVariable(pid, "<STACK>", DataType::Char, 1024, mmu, page_table);

    std::cout << "Created process with PID: " << pid << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
   // get process 
   Process* proc = mmu->getProcess(pid);
   if(proc == nullptr){
        std::cout << "error process not found" << std::endl;
        return;
    }
    //calculate size of variable
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

//find the first free hole
uint32_t allocated_size = 0;
bool found = false;

for (int i = 0; i < proc->variables.size(); i++){
    Variable* var = proc->variables[i];
    
    if(var->type == DataType::FreeSpace && var->size >= size){
        allocated_size = var->virtual_address;

        //shrink the free space variable
        var->virtual_address += size;
        var->size -= size;

        found = true;
        break;
    }
}

//if no free hole found, allocate new page
if(!found){
    //find last variable and allocate after it
    uint32_t last_address = 0;
    for (int i = 0; i < proc->variables.size(); i++){
        uint32_t end_address = proc->variables[i]->virtual_address + proc->variables[i]->size;
        if(end_address > last_address){
            last_address = end_address;
        }
    }
    allocated_size = last_address;

    //add page table entry for new page
    int page_needed = (size + page_table->getPageSize() - 1) / page_table->getPageSize();
    int start_page = allocated_size / page_table->getPageSize();
    for(int i = 0; i < page_needed; i++){
        page_table->addEntry(pid, start_page + i);
    }
}

    //print and insert MMU entry
    mmu->addVariableToProcess(pid, var_name, type, size, allocated_size);
    std::cout << allocated_size << std::endl;
}


void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    //get process
    Process* proc = mmu->getProcess(pid);
    if(proc == nullptr){
        std::cout << "error process not found" << std::endl;
        return;
    }
    //   - look up physical address for variable based on its virtual address / offset
    Variable *var = nullptr;
    for (int i = 0; i < proc->variables.size(); i++){
        if(proc->variables[i]->name == var_name){
            var = proc->variables[i];
            break;
        }
    }
    if (var== nullptr){
        std::cout << "error variable not found" << std::endl;
        return;
    }
    //   - insert `value` into `memory` at physical address
    uint32_t virtual_address = var-> virtual_address + offset; // calculate virtual address
    
    int physical_add = page_table->getPhysicalAddress(pid, var->virtual_address + offset); // convert to physical
    
    //get element size based on data type
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
    memcpy(&memory[physical_add], value, element_size); // copy value to memory
}


void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    
    //   - free all pages associated with given process
}
