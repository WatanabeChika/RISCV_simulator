#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "operation.h"

using std::cout;
using std::endl;
using std::string;
using std::hex;


unsigned pc = 0;
unsigned reg[32] = {0};
unsigned mem[500000] = {0};

void read_input(string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open the file." << endl;
        exit(1);
    }

    string line;
    unsigned address;
    unsigned value;

    while (std::getline(file, line)) {
        if (line[0] == '@') {
            // address line
            std::istringstream iss(line.substr(1));
            iss >> hex >> address;
        } 
        else {
            // data line
            std::istringstream iss(line);
            while (iss >> hex >> value) {
                mem[address] = value;
                ++address;
            }
        }
    }
    file.close();

    // test
    // for (int i = 0x1000; i < 0x1100; ++i) {
    //     cout << std::setw(2) << std::setfill('0') << hex << mem[i] << " ";
    //     if (i % 16 == 15) cout << endl;
    // }
}

unsigned Fetch(const unsigned &pc) {
    unsigned instruction = mem[pc];
    // 4 bytes
    for (int i = 1; i < 4; ++i) {
        instruction |= mem[pc + i] << (8 * i);
    }
    return instruction;
}

/* tmp_data: rd_index, rs1, rs2, imm, NONE */
void Decode(const unsigned &instruction, Op_type &type, Operation &op, unsigned (&tmp_data)[5]) {
    const unsigned opcode = instruction & 0x7f;
    const unsigned funct3 = (instruction >> 12) & 0x7;
    const unsigned funct7 = (instruction >> 25) & 0x7f;

    // get operation and its type
    op = getOperation(opcode, funct3, funct7, type);
    if (op == None) {
        cout << "NOT THAT OPERATION!!!" << endl;
        exit(1);
    }

    // save register_data and calculate immediate
    switch (type)
    {
        case R_type:
            tmp_data[0] = (instruction >> 7) & 0x1f;
            tmp_data[1] = reg[(instruction >> 15) & 0x1f];
            tmp_data[2] = reg[(instruction >> 20) & 0x1f];
            break;
        case I_type:
            tmp_data[0] = (instruction >> 7) & 0x1f;
            tmp_data[1] = reg[(instruction >> 15) & 0x1f];
            tmp_data[3] = unsigned(int(instruction) >> 20);
            break;
        case S_type:
            tmp_data[1] = reg[(instruction >> 15) & 0x1f];
            tmp_data[2] = reg[(instruction >> 20) & 0x1f];
            tmp_data[3] = ((instruction >> 7) & 0x1f) | ((unsigned(int(instruction) >> 25)) << 5);
            break;
        case B_type:
            tmp_data[1] = reg[(instruction >> 15) & 0x1f];
            tmp_data[2] = reg[(instruction >> 20) & 0x1f];
            tmp_data[3] = ((instruction >> 8) & 0xf) | (((instruction >> 25) & 0x3f) << 4) | (((instruction >> 7) & 0x1) << 10) | ((unsigned(int(instruction) >> 31)) << 11);
            break;
        case U_type:
            tmp_data[0] = (instruction >> 7) & 0x1f;
            tmp_data[3] = (instruction >> 12) << 12;
            break;
        case J_type:
            tmp_data[0] = (instruction >> 7) & 0x1f;
            tmp_data[3] = ((instruction >> 21) & 0x3ff) | (((instruction >> 20) & 0x1) << 10) | (((instruction >> 12) & 0xff) << 11) | ((unsigned(int(instruction) >> 31)) << 19);
            break;
        default:
            break;
    }
}

/* tmp_data: rd_index, rd_value, pct, mem_read, rs2 */
void Execute(const Operation &op, const Op_type &type, const unsigned &instruction, unsigned (&tmp_data)[5]) {
    // 10: no memory read; >20: zero-extend load; >10 && <20: sign-extend load; <10: store; the end number: bytes
    unsigned mem_read = 10;
    unsigned pct = pc;
    unsigned rd = 0;
    unsigned rs1 = tmp_data[1];
    unsigned rs2 = tmp_data[2];
    unsigned imm = tmp_data[3];

    // execute
    switch (type)
    { 
        case R_type:
            R_type_execute(op, rd, rs1, rs2);
            break;
        case I_type:
            I_type_execute(op, rd, rs1, imm, mem_read, pct);
            break;
        case S_type: 
            S_type_execute(op, rd, rs1, imm, mem_read);  
            break;
        case B_type:
            imm = imm << 1;
            B_type_execute(op, rs1, rs2, imm, pct);
            break;
        case U_type:
            U_type_execute(op, rd, imm, pct);
            break;
        case J_type:
            imm = imm << 1;
            J_type_execute(op, rd, imm, pct);
            break;
    }

    // save data
    tmp_data[1] = rd;
    tmp_data[2] = pct;
    tmp_data[3] = mem_read;
    tmp_data[4] = rs2; // for store
}

/* tmp_data: rd_index, rd_value, NONE, NONE, NONE */
void Memory(unsigned (&tmp_data)[5]) {
    unsigned rd = tmp_data[1];
    unsigned pct = tmp_data[2];
    unsigned mem_read = tmp_data[3];
    unsigned rs2 = tmp_data[4];

    // memory
    switch (mem_read)
    {
        case 10:
            break;
        // sign-extend load
        case 11:
            rd = int(mem[rd] && 0xff);
            break;
        case 12:
            rd = int(mem[rd] && 0xffff);
            break;
        case 14:
            rd = mem[rd];
            break;
        // zero-extend load
        case 21:
            rd = unsigned(mem[rd] && 0xff);
            break;
        case 22:
            rd = unsigned(mem[rd] && 0xffff);
            break;
        // store
        case 1:
            mem[rd] = rs2 & 0xff;
            break;
        case 2:
            mem[rd] = rs2 & 0xffff;
            break;
        case 4:
            mem[rd] = rs2;
            break;
        default:
            break;
    }

    // pc
    if (pct != pc) {
        pc = pct;
    } 
    else {
        pc += 4;
    }

    // save data
    tmp_data[1] = rd;
}

void WriteBack(const unsigned (&tmp_data)[5], const Op_type &type) {
    unsigned idx = tmp_data[0];
    unsigned rd = tmp_data[1];

    // write back
    if (type != S_type && type != B_type) {
        if (idx != 0) {
            reg[idx] = rd;
        }
    }
}

int main() {
    // read input data
    read_input("testcases/qsort.data");
    
    int clk = 0;
    unsigned instruction = 0;
    Operation op = None;
    Op_type type = None_type;
    unsigned tmp_data[5] = {0}; // between the levels

    // string q;
    while (true) {
        ++clk;
        instruction = Fetch(pc);
        // cout << hex << "instruction: " << instruction << endl;
        // cout << hex << "pc: " << pc << endl;
        if (instruction == 0x0ff00513) {
            cout << (((unsigned int)(reg[10]) & 255u)) << endl;
            break;
        }
        Decode(instruction, type, op, tmp_data);
        // cout << "OP num and type: " << op << " " << type << endl;
        // cout << hex << "rd_index: " << tmp_data[0] << " "  << "rs1: " << tmp_data[1] << " " << "rs2: " << tmp_data[2] << " " << "IMM: " << tmp_data[3] << endl;
        Execute(op, type, instruction, tmp_data);
        // cout << hex << "rd_index: " << tmp_data[0] << " " << "rd: " << tmp_data[1] << " " << "pct: " << tmp_data[2] << " " << "mem_read: " << tmp_data[3] << " " << "rs2: " << tmp_data[4] << endl;
        Memory(tmp_data);
        // out << hex << "rd_index: " << tmp_data[0] << " " << "rd: " << tmp_data[1] << endl;
        WriteBack(tmp_data, type);
        // cout << hex << "real rd: " << reg[tmp_data[0]] << " " << "next pc: " << pc << endl;
    }
    // cout << hex << int(0x1fff68) << endl;

    return 0;
}