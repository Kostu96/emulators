#include "cpu.hpp"
#include "disasm.hpp"

#include <cassert>

namespace PSX {

    void CPU::reset()
    {
        m_cpuStatus.PC = 0xBFC00000;
        m_nextPC = 0xBFC00004;

        m_cop0Status.SR = 0;

        //m_nextInstruction = 0;
        m_pendingLoad = { RegIndex{ 0 }, 0 };
    }

    void CPU::clock(DisassemblyLine& disasmLine)
    {
        Instruction inst = load32(m_cpuStatus.PC);
        disasm(m_cpuStatus.PC, inst, disasmLine);
        m_cpuStatus.PC = m_nextPC;
        m_nextPC += 4;

        setReg(m_pendingLoad.regIndex, m_pendingLoad.value);
        m_pendingLoad = { RegIndex{ 0 }, 0 };

        switch (inst.opcode())
        {
        case 0x00:
            switch (inst.subfn())
            {
            case 0x00: op_SLL(inst.regD(), inst.regT(), inst.shift()); break;

            case 0x03: op_SRA(inst.regD(), inst.regT(), inst.shift()); break;

            case 0x08: op_JR(inst.regS()); break;
            case 0x09: op_JALR(inst.regD(), inst.regS()); break;

            case 0x12: op_MFLO(inst.regD()); break;

            case 0x1A: op_DIV(getReg(inst.regS()), getReg(inst.regT())); break;

            case 0x20: op_ADD(inst.regD(), inst.regS(), getReg(inst.regT())); break;
            case 0x21: op_ADDU(inst.regD(), inst.regS(), getReg(inst.regT())); break;

            case 0x23: op_SUBU(inst.regD(), inst.regS(), getReg(inst.regT())); break;
            case 0x24: op_AND(inst.regD(), inst.regS(), getReg(inst.regT())); break;
            case 0x25: op_OR(inst.regD(), inst.regS(), getReg(inst.regT())); break;

            case 0x2B: op_SLTU(inst.regD(), getReg(inst.regS()), getReg(inst.regT())); break;
            default:
                assert(false && "Unhandled opcode!");
            }
            break;
        case 0x01: op_BXX(inst.regS(), inst.imm_se_jump(), inst.word); break;
        case 0x02: op_J(inst.imm_jump()); break;
        case 0x03: op_JAL(inst.imm_jump()); break;

        case 0x04: branch(getReg(inst.regS()) == getReg(inst.regT()), inst.imm_se_jump()); break; // BEQ
        case 0x05: branch(getReg(inst.regS()) != getReg(inst.regT()), inst.imm_se_jump()); break; // BNE
        case 0x06: branch(static_cast<s32>(getReg(inst.regS())) <= 0, inst.imm_se_jump()); break; // BLEZ
        case 0x07: branch(static_cast<s32>(getReg(inst.regS())) > 0, inst.imm_se_jump()); break; // BGTZ

        case 0x08: op_ADD(inst.regT(), inst.regS(), inst.imm_se()); break;
        case 0x09: op_ADDU(inst.regT(), inst.regS(), inst.imm_se()); break;
        case 0x0A: op_SLTI(inst.regT(), getReg(inst.regS()), inst.imm_se()); break;

        case 0x0C: op_AND(inst.regT(), inst.regS(), inst.imm()); break;
        case 0x0D: op_OR(inst.regT(), inst.regS(), inst.imm()); break;

        case 0x0F: op_LUI(inst.regT(), inst.imm()); break;
        case 0x10:
            switch (inst.copfn())
            {
            case 0x00: op_MFC0(inst.regD(), inst.regT()); break;
            case 0x04: op_MTC0(inst.regD(), inst.regT()); break;
            default:
                assert(false && "Unhandled opcode!");
            }
            break;

        case 0x20: op_LB(inst.regT(), inst.regS(), inst.imm_se()); break;

        case 0x23: op_LW(inst.regT(), inst.regS(), inst.imm_se()); break;
        case 0x24: op_LBU(inst.regT(), inst.regS(), inst.imm_se()); break;

        case 0x28: op_SB(inst.regT(), inst.regS(), inst.imm_se()); break;
        case 0x29: op_SH(inst.regT(), inst.regS(), inst.imm_se()); break;

        case 0x2B: op_SW(inst.regT(), inst.regS(), inst.imm_se()); break;
        default:
            assert(false && "Unhandled opcode!");
        }

        std::memcpy(m_cpuStatus.regs, m_helperCPURegs, REGISTER_COUNT * sizeof(u32));

        assert(getReg(RegIndex{ 0 }) == 0 && "GPR zero value was changed!");
    }

    CPU::CPU()
    {
        setReg(RegIndex{ 0 }, 0);
    }

    void CPU::setReg(RegIndex index, u32 value)
    {
        assert(index.i < REGISTER_COUNT && "Index out of bounds!");

        m_helperCPURegs[index.i] = value;
        m_helperCPURegs[0] = 0;
    }

    void CPU::branch(bool condition, u32 offset)
    {
        if (condition) {
            m_nextPC += offset;
            m_nextPC -= 4;
        }
    }

    void CPU::op_MFC0(RegIndex copIndex, RegIndex cpuIndex)
    {
        m_pendingLoad = { cpuIndex, m_cop0Status.regs[copIndex.i] };
    }

    void CPU::op_MTC0(RegIndex copIndex, RegIndex cpuIndex)
    {
        m_cop0Status.regs[copIndex.i] = getReg(cpuIndex);
    }

    void CPU::op_BXX(RegIndex s, u32 offset, u32 opcode)
    {
        u32 isBGEZ = (opcode >> 16) & 1;
        bool isLink = (opcode >> 20) & 1;

        u32 test = static_cast<s32>(getReg(s)) < 0;
        test = test ^ isBGEZ;

        if (isLink) setReg(RegIndex{ 31 }, m_cpuStatus.PC);
        
        branch(test, offset);
    }

    void CPU::op_SLL(RegIndex d, RegIndex t, u32 shift)
    {
        setReg(d, getReg(t) << shift);
    }

    void CPU::op_SRA(RegIndex d, RegIndex t, u32 shift)
    {
        setReg(d, static_cast<s32>(getReg(t)) >> shift);
    }

    void CPU::op_OR(RegIndex target, RegIndex lhs, u32 rhs)
    {
        setReg(target, getReg(lhs) | rhs);
    }

    void CPU::op_AND(RegIndex target, RegIndex lhs, u32 rhs)
    {
        setReg(target, getReg(lhs) & rhs);
    }

    void CPU::op_J(u32 immediate)
    {
        m_nextPC = (m_cpuStatus.PC & 0xF0000000) | immediate;
    }

    void CPU::op_JAL(u32 immediate)
    {
        setReg(RegIndex{ 31 }, m_nextPC);
        op_J(immediate);
    }

    void CPU::op_JR(RegIndex s)
    {
        m_nextPC = getReg(s);
    }

    void CPU::op_JALR(RegIndex d, RegIndex s)
    {
        setReg(d, m_cpuStatus.PC);
        m_nextPC = getReg(s);
    }

    void CPU::op_ADD(RegIndex d, RegIndex s, u32 rhs)
    {
        // TODO: check this code for egde cases
        s32 a = getReg(s);
        s32 b = (s32)rhs;
        s32 result = a + b;
        if (a > 0 && b > 0 && result < 0 ||
            a < 0 && b < 0 && result > 0)
            assert(false && "Overflow exception!");
        else
           setReg(d, result);
    }

    void CPU::op_ADDU(RegIndex d, RegIndex s, u32 rhs)
    {
        setReg(d, getReg(s) + rhs);
    }

    void CPU::op_SUBU(RegIndex d, RegIndex s, u32 rhs)
    {
        setReg(d, getReg(s) - rhs);
    }

    void CPU::op_LUI(RegIndex t, u32 immediate)
    {
        setReg(t, immediate << 16);
    }

    void CPU::op_SB(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        store8(getReg(s) + immediate, getReg(t) & 0xFF);
    }

    void CPU::op_SH(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        store16(getReg(s) + immediate, getReg(t) & 0xFFFF);
    }

    void CPU::op_SW(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        store32(getReg(s) + immediate, getReg(t));
    }

    void CPU::op_LB(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        s8 value = load8(getReg(s) + immediate);
        m_pendingLoad = { t, static_cast<u32>(value) };
    }

    void CPU::op_LBU(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        m_pendingLoad = { t, load8(getReg(s) + immediate) };
    }

    void CPU::op_LW(RegIndex t, RegIndex s, u32 immediate)
    {
        if (m_cop0Status.SR & 0x10000)
            return; // isolated cache bit is set

        m_pendingLoad = { t, load32(getReg(s) + immediate) };
    }

    void CPU::op_SLTU(RegIndex d, u32 lhs, u32 rhs)
    {
        setReg(d, lhs < rhs);
    }

    void CPU::op_SLTI(RegIndex t, s32 lhs, s32 rhs)
    {
        setReg(t, lhs < rhs);
    }

    void CPU::op_DIV(s32 numerator, s32 denominator)
    {
        if (denominator == 0) {
            // division by 0
            m_cpuStatus.HI = numerator;
            m_cpuStatus.LO = (numerator >= 0) ? 0xFFFFFFFF : 1;
        }
        else if (static_cast<u32>(numerator) == 0x80000000 && denominator == -1) {
            m_cpuStatus.HI = 0;
            m_cpuStatus.LO = 0x80000000;
        }
        else {
            m_cpuStatus.HI = numerator % denominator;
            m_cpuStatus.LO = numerator / denominator;
        }
    }

    void CPU::op_MFLO(RegIndex d)
    {
        setReg(d, m_cpuStatus.LO);
    }

} // namespace PSX
