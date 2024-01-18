#include "operation.h"


Operation getOperation(const unsigned &opcode, const unsigned &funct3, const unsigned &funct7, Op_type &type) {
    switch (opcode)
    {
    case 0b0110011:
        type = R_type;
        switch (funct3)
        {
        case 0b000:
            return funct7==0 ? ADD : SUB;
        case 0b001:
            return SLL;
        case 0b010:
            return SLT;
        case 0b011:
            return SLTU;
        case 0b100:
            return XOR;
        case 0b101:
            return funct7==0 ? SRL : SRA;
        case 0b110:
            return OR;
        case 0b111:
            return AND;
        default:
            break;
        }
        break;

    case 0b0010011:
        type = I_type;
        switch (funct3)
        {
        case 0b000:
            return ADDI;
        case 0b010:
            return SLTI;
        case 0b011:
            return SLTIU;
        case 0b100:
            return XORI;
        case 0b110:
            return ORI;
        case 0b111:
            return ANDI;
        case 0b001:
            return SLLI;
        case 0b101:
            return funct7==0 ? SRLI : SRAI;
        default:
            break;
        }
        break;

    case 0b1100011: 
        type = B_type;
        switch (funct3)
        {
        case 0b000:
            return BEQ;
        case 0b001:
            return BNE;
        case 0b100:
            return BLT;
        case 0b101:
            return BGE;
        case 0b110:
            return BLTU;
        case 0b111:
            return BGEU;
        default:
            break;
        }
        break;

    case 0b0000011: 
        type = I_type;
        switch (funct3)
        {
        case 0b000:
            return LB;
        case 0b001:
            return LH;
        case 0b010:
            return LW;
        case 0b100:
            return LBU;
        case 0b101:
            return LHU;
        default:
            break;
        }
        break;

    case 0b0100011: 
        type = S_type;
        switch (funct3)
        {
        case 0b000:
            return SB;
        case 0b001:
            return SH;
        case 0b010:
            return SW;
        default:
            break;
        }
        break;

    case 0b1100111:
        type = I_type;
        return JALR;

    case 0b1101111: 
        type = J_type;
        return JAL;

    case 0b0010111:
        type = U_type;
        return AUIPC;

    case 0b0110111:
        type = U_type;
        return LUI;
    
    default:
        type = None_type;
        break;
    }
    return None;
}

void R_type_execute(const Operation &op, unsigned &rd, const unsigned &rs1, const unsigned &rs2) {
    switch (op)
    {
    case ADD:
        rd = rs1 + rs2;
        break;
    case SUB:
        rd = rs1 - rs2;
        break;
    case SLL:
        rd = rs1 << (rs2 & 0x1f);
        break;
    case SLT:
        rd = (int)rs1 < (int)rs2;
        break;
    case SLTU:
        rd = rs1 < rs2;
        break;
    case XOR:   
        rd = rs1 ^ rs2;
        break;
    case SRL:
        rd = rs1 >> (rs2 & 0x1f);
        break;
    case SRA:
        rd = (int)rs1 >> (rs2 & 0x1f);
        break;
    case OR:
        rd = rs1 | rs2;
        break;
    case AND:
        rd = rs1 & rs2;
        break;
    default:
        break;
    }
}

void I_type_execute(const Operation &op, unsigned &rd, const unsigned &rs1, const unsigned &imm, unsigned &mem_read, unsigned &pc) {
    switch (op)
    {
    case ADDI:
        rd = rs1 + int(imm);
        break;
    case SLTI:
        rd = (int)rs1 < (int)imm;
        break;
    case SLTIU:
        rd = rs1 < imm;
        break;
    case XORI:
        rd = rs1 ^ int(imm);
        break;
    case ORI:
        rd = rs1 | int(imm);
        break;
    case ANDI:
        rd = rs1 & int(imm);
        break;
    case SLLI:
        rd = rs1 << (imm & 0x1f);
        break;
    case SRLI:
        rd = rs1 >> (imm & 0x1f);
        break;
    case SRAI:
        rd = (int)rs1 >> (imm & 0x1f);
        break;
    case LB:
        rd = rs1 + int(imm);
        // sign-extend load
        mem_read = 11;
        break;
    case LH:
        rd = rs1 + int(imm);
        mem_read = 12;
        break;
    case LW:
        rd = rs1 + int(imm);
        mem_read = 14;
        break;
    case LBU:
        rd = rs1 + int(imm);
        // zero-extend load
        mem_read = 21;
        break;
    case LHU:
        rd = rs1 + int(imm);
        mem_read = 22;
        break;
    case JALR:
        rd = pc + 4;
        pc = rs1 + int(imm);
        break;
    default:
        break;
    }
}

void U_type_execute(const Operation &op, unsigned &rd, const unsigned &imm, unsigned &pc) {
    switch (op)
    {
    case LUI:
        rd = imm;
        break;
    case AUIPC:
        rd = pc + imm;
        break;
    default:
        break;
    }
}

void J_type_execute(const Operation &op, unsigned &rd, const unsigned &imm, unsigned &pc) {
    switch (op)
    {
    case JAL:
        rd = pc + 4;
        pc = (pc + int(imm)) & 0x1fff;
        break;
    default:
        break;
    }
}

void B_type_execute(const Operation &op, const unsigned &rs1, const unsigned &rs2, const unsigned &imm, unsigned &pc) {
    switch (op)
    {
    case BEQ:
        if (rs1 == rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    case BNE:
        if (rs1 != rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    case BLT:
        if ((int)rs1 < (int)rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    case BGE:
        if ((int)rs1 >= (int)rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    case BLTU:
        if (rs1 < rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    case BGEU:
        if (rs1 >= rs2) {
            pc = (pc + int(imm)) & 0x1fff;
        }
        break;
    default:
        break;
    }
}

void S_type_execute(const Operation &op, unsigned &rd, const unsigned &rs1, const unsigned &imm, unsigned &mem_read) {
    switch (op)
    {
    case SB:
        rd = rs1 + int(imm);
        // store
        mem_read = 1;
        break;
    case SH:
        rd = rs1 + int(imm);
        mem_read = 2;
        break;
    case SW:
        rd = rs1 + int(imm);
        mem_read = 4;
        break;
    default:
        break;
    }
}