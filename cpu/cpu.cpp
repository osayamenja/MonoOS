//
// Author:  Osayamen Aimuyo
// Created: 1/29/2023.
//

#include <iomanip>
#include <unistd.h>
#include <sstream>

#include "../computer/computer.h"
#include "../computer/print.h"
#include "../memory/memory.h"
#include "../scheduler/scheduler.h"
#include "../shell/shell.h"
#include "../utility/utility.h"
#include "cpu.h"

REGS registers;
ExecutionStatus executionStatus;
ExecutionStatus cpu_operation(int pid){
    executionStatus = Executing;
    int TQ_counter = 0;
    while(TQ_counter < TQ && executionStatus != Exit && executionStatus != IODelay){
        cpu_fetch_instruction();
        cpu_execute_instruction(pid);
        registers.PC++;
        TQ_counter++;
    }
    if(TQ_counter == TQ) return TQExpiration;
    else return executionStatus;
}

void cpu_fetch_instruction(){
    // fetch first word
    registers.MAR = cpu_mem_address(registers.PC);
    mem_read(); // MBR = Mem[MAR]
    registers.IR0 = registers.MBR;

    registers.PC++;

    //fetch second word
    registers.MAR = cpu_mem_address(registers.PC);
    mem_read();
    registers.IR1 = registers.MBR;
}

int cpu_mem_address(int m_addr){ // get page number
    return registers.Base + m_addr;
}

void cpu_execute_instruction(int pid){
    switch (registers.IR0) {
        case 1:
            registers.AC = registers.IR1;
            break;

        case 2:
            if(current_mem_instr_time == mem_delay){ // needed for simulating IO delay for multi-level feedback queue.
                executionStatus = Executing;
                current_mem_instr_time = 0;
                registers.MAR = cpu_mem_address(registers.IR1);
                mem_read();
                registers.AC = registers.MBR;
            }
            else{
                executionStatus = IODelay;
                current_mem_instr_time++;
                registers.PC -= 2; // needed to ensure that the IO instruction reoccurs until its delay ends.
            }
            break;

        case 3:
            if(current_mem_instr_time == mem_delay){
                executionStatus = Executing;
                current_mem_instr_time = 0;
                registers.MAR = cpu_mem_address(registers.IR1);
                mem_read();
                registers.AC += registers.MBR;
            }
            else{
                executionStatus = IODelay;
                current_mem_instr_time++;
                registers.PC -= 2;
            }
            break;

        case 4:
            if(current_mem_instr_time == mem_delay){
                executionStatus = Executing;
                current_mem_instr_time = 0;
                registers.MAR = cpu_mem_address(registers.IR1);
                mem_read();
                registers.AC *= registers.MBR;
            }
            else{
                executionStatus = IODelay;
                current_mem_instr_time++;
                registers.PC -= 2;
            }
            break;

        case 5:
            if(current_mem_instr_time == mem_delay){
                executionStatus = Executing;
                current_mem_instr_time = 0;
                registers.MAR = cpu_mem_address(registers.IR1);
                registers.MBR = registers.AC;
                mem_write();
            }
            else{
                executionStatus = IODelay;
                current_mem_instr_time++;
                registers.PC -= 2;
            }
            break;

        case 6:
            if(registers.AC > 0) {
                registers.PC = --registers.IR1; // This decrement prevents PC from being incremented twice.
            }
            break;

        case 7:{
            print_print(pid, registers.AC);
            break;
        }

        case 8:
            usleep(registers.IR1);
            break;

        case 9:
            shell_command(registers.IR1);
            break;

        case 0:
            executionStatus = Exit;
            break;

        default:
            break;
    }
}

void cpu_dump_registers(){
    std::stringstream dump_stream;
    dump_stream << generate_header();
    dump_stream << std::setw(17) <<"Register Dump" << std::endl;
    dump_stream << generate_header();

    dump_stream <<"Register: Contents" << std::endl;
    dump_stream << "AC: " << registers.AC << std::endl;
    dump_stream << "Base: " << registers.Base << std::endl;
    dump_stream << "MAR: " << registers.MAR << std::endl;
    dump_stream << "MBR: " << registers.MBR << std::endl;
    dump_stream << "IR0: " << registers.IR0 << std::endl;
    dump_stream << "IR1: " << registers.IR1 << std::endl;
    dump_stream << "PC: " << registers.PC << std::endl;

    std::string dump_str  = dump_stream.str();
    atomically_print_to_stdout(dump_str);
}