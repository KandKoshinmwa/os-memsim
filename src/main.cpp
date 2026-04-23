#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include "mmu.h"
#include "pagetable.h"
#include <stringStream>
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
        stringstream ss(command);
        string cmd;
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
            string var_name;
            string data_type_str;
            uint32_t num_elements;

            ss >> pid >> var_name >> data_type_str >>num_elements;
            DataType data_type;

            if(data_type_str == "char"){
                data_type = Char;
            }else if(data_type_str == "short"){
                data_type = Short;
            }else if(data_type_str == "int"){
                data_type = Int;
            }else if(data_type_str == "float"){
                data_type = Float;
            }else if(data_type_str == "long"){
                data_type = Long;
        }     else if(data_type_str == "double"){
                data_type = Double;
            }else{
                std::cout << "Invalid data type" << std::endl;
                continue;
            }
            allocateVariable(pid, var_name, data_type, num_elements, mmu, page_table);
        }else if (cmd == "set"){
            //set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N>
            uint32_t pid;
            string var_name;
            int offset;
            int value;
            vector <string> values;
            string current_value;


            ss>> pid>> var_name >> offset;
            
            while(ss >> current_value){
                //we want to convert it to an integer here, change maybe needed if we implement parsing in the function.
                values.push_back(current_value);
            }
            for(int i = 0; i < values.size(); i++){
                setVariable(pid, var_name, offset + i, &values[i], mmu, page_table, memory);
            }
        }else if (cmd == "free"){
        }else if (cmd == "terminate"){
        }else if (cmd == "print"){
            
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
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //   - print pid
  
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address
    
}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
    
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
