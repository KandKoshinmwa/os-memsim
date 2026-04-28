#include <algorithm>
#include "pagetable.h"


PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    

    // Collect all currently assigned physical frame numbers
    std::vector<int> used_frames;
    for (auto const& pair : _table) {
        used_frames.push_back(pair.second);
    }

    // Sort frames to efficiently find the lowest available gap
    std::sort(used_frames.begin(), used_frames.end());

    // Find the lowest available frame number to recycle freed memory safely
    int frame = 0;
    for (int used : used_frames) {
        if (frame == used) {
            frame++; 
        }
    }

    // Map the virtual page entry to the newly assigned physical frame
    _table[entry] = frame;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        int frame_number = _table[entry];
        address = (frame_number * _page_size) + page_offset;
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        // Locate the delimiter separating the PID and Page Number
        size_t split = keys[i].find('|');
        
        // Extract the strings directly
        std::string pid_str = keys[i].substr(0, split);
        std::string page_str = keys[i].substr(split + 1);
        
        //  Get the Frame Number 
        int frame_number = _table[keys[i]];
        
        printf(" %-4s | %11s | %13d\n", 
               pid_str.c_str(), 
               page_str.c_str(), 
               frame_number);
    }
}
void PageTable::removeEntry(uint32_t pid, int page_number)
{
    // Reconstruct the unique map key for the specific page
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    // Erase the mapping from the table to free up the frame
    _table.erase(entry);
}