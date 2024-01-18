#ifndef OPERATION_H
#define OPERATION_H

enum Operation
{
    LUI,
    AUIPC,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    None
};

enum Op_type
{
    R_type,
    I_type,
    S_type,
    B_type,
    U_type,
    J_type,
    None_type
};

Operation getOperation(const unsigned &opcode, const unsigned &funct3, const unsigned &funct7, Op_type &type);

void R_type_execute(const Operation &op, unsigned &rd, const unsigned &rs1, const unsigned &rs2);

void I_type_execute(const Operation &op, unsigned &rd, const unsigned &rs1, const unsigned &imm, unsigned &mem_read, unsigned &pc);

void U_type_execute(const Operation &op, unsigned &rd, const unsigned &imm, unsigned &pc);

void J_type_execute(const Operation &op, unsigned &rd, const unsigned &imm, unsigned &pc);

void S_type_execute(const Operation &op, unsigned &rs1, const unsigned &rs2, const unsigned &imm, unsigned &mem_read);

void B_type_execute(const Operation &op, const unsigned &rs1, const unsigned &rs2, const unsigned &imm, unsigned &pc);

#endif // OPERATION_H