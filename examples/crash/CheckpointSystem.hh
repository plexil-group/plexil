#include "Value.hh"
#include <iostream>

#ifndef _H_CheckpointSystem
#define _H_CheckpointSystem

//Lookups
bool did_crash();
int32_t num_active_crashes();
int32_t num_total_crashes();
Value get_checkpoint_state(const std::string checkpoint_name,int32_t boot_num);
Value get_checkpoint_time(const std::string checkpoint_name, int32_t boot_num);
Value get_time_of_boot(int32_t boot_num);
Value get_time_of_crash(int32_t boot_num);

//Commands
Value set_checkpoint(const std::string& checkpoint_name);
Value set_safe_reboot(bool b);
Value delete_crash(int32_t crash_number);
#endif
