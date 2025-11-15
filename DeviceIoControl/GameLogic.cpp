#include "framework.h"
#include "GameLogic.h"
#include "client.h"
#include "NetworkServer.h"
#include "Offsets.h"
#include "Config.h"
#include "common_packets.h"
#include "RTModel.h"
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <cmath>

// PhysX Libs
#pragma comment(lib, "PhysXFoundation_64.lib")
#pragma comment(lib, "PhysX_64.lib")
#pragma comment(lib, "PhysXCommon_64.lib")
#pragma comment(lib, "PhysXCooking_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")

// Zydis Libs
#pragma comment(lib, "Zydis.lib")
#pragma comment(lib, "Zycore.lib")

extern int g_game_width;
extern int g_game_height;

GameLogic* GameLogic::s_instance = nullptr;
static std::atomic<uint64_t> g_player_update_loop_counter{ 0 };


// decryptor members
static ZydisDecoder g_decoder;
static ZydisFormatter g_formatter;
constexpr uintptr_t MAGIC_MASK = 0x0000FF0000000000;
constexpr std::uintptr_t MAGIC = 0x00004A0000000000;
std::unordered_map<uintptr_t, uintptr_t> g_remotePointerCache;
const uintptr_t IMAGE_BASE = 0x140000000;

GameLogic::GameLogic(MemoryAccessClient& k, NetworkServerUdp& ns)
    : kernel(k), networkServer(ns)
{
    s_instance = this;
}

GameLogic::~GameLogic() {
    stop();
    s_instance = nullptr;
}

bool GameLogic::initialize() {
    while (true)
    {
        pid = kernel.get_process_id_by_name(L"DeltaForceClient-Win64-Shipping.exe");
        if (pid == 0) {
            std::cerr << "[GameLogic] Error: Game process not found.\n";
        }

        baseAddress = kernel.get_module_base_address(pid, L"DeltaForceClient-Win64-Shipping.exe");
        if (baseAddress != IMAGE_BASE)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }

        RTModel::Init();
        if (!AddVectoredExceptionHandler(1, VectoredExceptionHandler)) {
            std::cout << "AddVectoredExceptionHandler Failed!\n";
            return 1;
        }

        ZydisDecoderInit(&g_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
        ZydisFormatterInit(&g_formatter, ZYDIS_FORMATTER_STYLE_INTEL);

        std::cout << "[Main] Loading PE..." << std::endl;
        if (!loader.Load("DeltaForceClient-Win64-Shipping.exe", IMAGE_BASE)) {
            std::cerr << "[Main] Failed to load PE" << std::endl;
            system("pause");
            return 1;
        }
        std::cout << "[GameLogic] PhysX Initialized." << std::endl;

        std::cout << "[GameLogic] Initialized. PID: " << pid << ", BaseAddress: 0x" << std::hex << baseAddress << std::dec << std::endl;

        return true;
    }
}

// ------------------------- dynamic decrypt functions -------------------------

uintptr_t GameLogic::GetRegisterValue(PCONTEXT context, ZydisRegister reg) {
    switch (reg) {
    case ZYDIS_REGISTER_RAX: return context->Rax;
    case ZYDIS_REGISTER_RBX: return context->Rbx;
    case ZYDIS_REGISTER_RCX: return context->Rcx;
    case ZYDIS_REGISTER_RDX: return context->Rdx;
    case ZYDIS_REGISTER_RSI: return context->Rsi;
    case ZYDIS_REGISTER_RDI: return context->Rdi;
    case ZYDIS_REGISTER_RBP: return context->Rbp;
    case ZYDIS_REGISTER_RSP: return context->Rsp;
    case ZYDIS_REGISTER_R8:  return context->R8;
    case ZYDIS_REGISTER_R9:  return context->R9;
    case ZYDIS_REGISTER_R10: return context->R10;
    case ZYDIS_REGISTER_R11: return context->R11;
    case ZYDIS_REGISTER_R12: return context->R12;
    case ZYDIS_REGISTER_R13: return context->R13;
    case ZYDIS_REGISTER_R14: return context->R14;
    case ZYDIS_REGISTER_R15: return context->R15;
    case ZYDIS_REGISTER_EAX: return context->Rax & 0xFFFFFFFF;
    case ZYDIS_REGISTER_EBX: return context->Rbx & 0xFFFFFFFF;
    case ZYDIS_REGISTER_ECX: return context->Rcx & 0xFFFFFFFF;
    case ZYDIS_REGISTER_EDX: return context->Rdx & 0xFFFFFFFF;
    case ZYDIS_REGISTER_ESI: return context->Rsi & 0xFFFFFFFF;
    case ZYDIS_REGISTER_EDI: return context->Rdi & 0xFFFFFFFF;
    case ZYDIS_REGISTER_EBP: return context->Rbp & 0xFFFFFFFF;
    case ZYDIS_REGISTER_ESP: return context->Rsp & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R8D: return context->R8 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R9D: return context->R9 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R10D: return context->R10 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R11D: return context->R11 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R12D: return context->R12 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R13D: return context->R13 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R14D: return context->R14 & 0xFFFFFFFF;
    case ZYDIS_REGISTER_R15D: return context->R15 & 0xFFFFFFFF;
    default: return 0;
    }
}

void GameLogic::SetRegisterValue(PCONTEXT context, ZydisRegister reg, uintptr_t value) {
    switch (reg) {
    case ZYDIS_REGISTER_RAX: context->Rax = value; break;
    case ZYDIS_REGISTER_RBX: context->Rbx = value; break;
    case ZYDIS_REGISTER_RCX: context->Rcx = value; break;
    case ZYDIS_REGISTER_RDX: context->Rdx = value; break;
    case ZYDIS_REGISTER_RSI: context->Rsi = value; break;
    case ZYDIS_REGISTER_RDI: context->Rdi = value; break;
    case ZYDIS_REGISTER_RBP: context->Rbp = value; break;
    case ZYDIS_REGISTER_RSP: context->Rsp = value; break;
    case ZYDIS_REGISTER_R8:  context->R8 = value; break;
    case ZYDIS_REGISTER_R9:  context->R9 = value; break;
    case ZYDIS_REGISTER_R10: context->R10 = value; break;
    case ZYDIS_REGISTER_R11: context->R11 = value; break;
    case ZYDIS_REGISTER_R12: context->R12 = value; break;
    case ZYDIS_REGISTER_R13: context->R13 = value; break;
    case ZYDIS_REGISTER_R14: context->R14 = value; break;
    case ZYDIS_REGISTER_R15: context->R15 = value; break;
    case ZYDIS_REGISTER_EAX: context->Rax = (context->Rax & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_EBX: context->Rbx = (context->Rbx & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_ECX: context->Rcx = (context->Rcx & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_EDX: context->Rdx = (context->Rdx & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_ESI: context->Rsi = (context->Rsi & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_EDI: context->Rdi = (context->Rdi & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_EBP: context->Rbp = (context->Rbp & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_ESP: context->Rsp = (context->Rsp & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R8D: context->R8 = (context->R8 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R9D: context->R9 = (context->R9 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R10D: context->R10 = (context->R10 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R11D: context->R11 = (context->R11 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R12D: context->R12 = (context->R12 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R13D: context->R13 = (context->R13 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R14D: context->R14 = (context->R14 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    case ZYDIS_REGISTER_R15D: context->R15 = (context->R15 & 0xFFFFFFFF00000000) | (value & 0xFFFFFFFF); break;
    default: break;
    }
}


bool GameLogic::FixBaseDisplacementMemoryAccess(PCONTEXT context, uintptr_t value) {
    uint8_t* instructionPointer = reinterpret_cast<uint8_t*>(context->Rip);

    // 解码指令
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

    if (ZYAN_FAILED(ZydisDecoderDecodeFull(&g_decoder, instructionPointer, ZYDIS_MAX_INSTRUCTION_LENGTH,
        &instruction, operands))) {
        return false;
    }

    char buffer[256];
    ZydisFormatterFormatInstruction(&g_formatter, &instruction, operands,
        instruction.operand_count_visible, buffer, sizeof(buffer),
        reinterpret_cast<ZyanU64>(instructionPointer), nullptr);


    // 处理 MOV 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_MOV && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;
        if (operands[0].size == 64)
        {
            value |= MAGIC;
        }
        SetRegisterValue(context, destReg, value);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 MOVZX 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_MOVZX && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取源操作数的大小
        ZyanU16 srcSize = operands[1].size;

        // 根据源操作数大小进行零扩展
        uintptr_t extendedValue;
        switch (srcSize) {
        case 8:  // 从8位零扩展到目标大小
            extendedValue = value & 0xFF;
            break;
        case 16: // 从16位零扩展到目标大小
            extendedValue = value & 0xFFFF;
            break;
        case 32: // 从32位零扩展到64位
            extendedValue = value & 0xFFFFFFFF;
            break;
        default:
            std::cout << "不支持的源操作数大小: " << srcSize << std::endl;
            return false;
        }

        SetRegisterValue(context, destReg, extendedValue);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 ADD 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_ADD && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取目标寄存器的当前值
        uintptr_t currentValue = GetRegisterValue(context, destReg);

        // 计算新的值：当前值 + 从内存读取的值
        uintptr_t newValue;

        // 根据操作数大小进行处理
        switch (operands[0].size) {
        case 8:  // 8位操作数
            newValue = (currentValue & 0xFFFFFFFFFFFFFF00) | ((currentValue + value) & 0xFF);
            break;
        case 16: // 16位操作数
            newValue = (currentValue & 0xFFFFFFFFFFFF0000) | ((currentValue + value) & 0xFFFF);
            break;
        case 32: // 32位操作数
            newValue = (currentValue & 0xFFFFFFFF00000000) | ((currentValue + value) & 0xFFFFFFFF);
            break;
        case 64: // 64位操作数
            newValue = currentValue + value;
            break;
        default:
            std::cout << "不支持的操作数大小: " << operands[0].size << std::endl;
            return false;
        }

        SetRegisterValue(context, destReg, newValue);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 SUB 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_SUB && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取目标寄存器的当前值
        uintptr_t currentValue = GetRegisterValue(context, destReg);

        // 计算新的值：当前值 - 从内存读取的值
        uintptr_t newValue;

        // 根据操作数大小进行处理
        switch (operands[0].size) {
        case 8:  // 8位操作数
            newValue = (currentValue & 0xFFFFFFFFFFFFFF00) | ((currentValue - value) & 0xFF);
            break;
        case 16: // 16位操作数
            newValue = (currentValue & 0xFFFFFFFFFFFF0000) | ((currentValue - value) & 0xFFFF);
            break;
        case 32: // 32位操作数
            newValue = (currentValue & 0xFFFFFFFF00000000) | ((currentValue - value) & 0xFFFFFFFF);
            break;
        case 64: // 64位操作数
            newValue = currentValue - value;
            break;
        default:
            std::cout << "不支持的操作数大小: " << operands[0].size << std::endl;
            return false;
        }

        SetRegisterValue(context, destReg, newValue);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 IMUL 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_IMUL && instruction.operand_count_visible == 3) {
        // 检查操作数类型：寄存器，内存，立即数
        if (operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY &&
            operands[2].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {

            ZydisRegister destReg = operands[0].reg.value;
            int64_t immediate = operands[2].imm.value.s;

            // 执行有符号乘法：目标寄存器 = 内存值 * 立即数
            int64_t result;

            // 根据操作数大小进行处理
            switch (operands[0].size) {
            case 8:  // 8位操作数
                result = (int8_t)value * (int8_t)immediate;
                break;
            case 16: // 16位操作数
                result = (int16_t)value * (int16_t)immediate;
                break;
            case 32: // 32位操作数
                result = (int32_t)value * (int32_t)immediate;
                break;
            case 64: // 64位操作数
                result = (int64_t)value * immediate;
                break;
            default:
                std::cout << "不支持的操作数大小: " << operands[0].size << std::endl;
                return false;
            }


            SetRegisterValue(context, destReg, (uintptr_t)result);
            context->Rip += instruction.length;
            return true;
        }
    }

    // 处理 IMUL 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_IMUL && instruction.operand_count_visible == 2) {
        // 检查操作数类型：寄存器，内存
        if (operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY) {

            ZydisRegister destReg = operands[0].reg.value;

            // 获取目标寄存器的当前值
            int64_t currentValue = (int64_t)GetRegisterValue(context, destReg);

            // 执行有符号乘法：目标寄存器 = 当前值 * 内存值
            int64_t result;

            // 根据操作数大小进行处理
            switch (operands[0].size) {
            case 8:  // 8位操作数
                result = (int8_t)currentValue * (int8_t)value;
                break;
            case 16: // 16位操作数
                result = (int16_t)currentValue * (int16_t)value;
                break;
            case 32: // 32位操作数
                result = (int32_t)currentValue * (int32_t)value;
                break;
            case 64: // 64位操作数
                result = currentValue * (int64_t)value;
                break;
            default:
                std::cout << "不支持的操作数大小: " << operands[0].size << std::endl;
                return false;
            }


            SetRegisterValue(context, destReg, (uintptr_t)result);
            context->Rip += instruction.length;
            return true;
        }
    }

    // 处理 AND 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_AND && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取目标寄存器的当前值
        uintptr_t currentValue = GetRegisterValue(context, destReg);

        // 计算新的值：当前值 AND 从内存读取的值
        uintptr_t newValue = currentValue & value;

        SetRegisterValue(context, destReg, newValue);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 OR 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_OR && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取目标寄存器的当前值
        uintptr_t currentValue = GetRegisterValue(context, destReg);

        // 计算新的值：当前值 OR 从内存读取的值
        uintptr_t newValue = currentValue | value;

        std::cout << std::hex << "OR 操作: " << currentValue << " | " << value
            << " = " << newValue << std::endl;

        SetRegisterValue(context, destReg, newValue);
        context->Rip += instruction.length;
        return true;
    }

    // 处理 XOR 指令
    if (instruction.mnemonic == ZYDIS_MNEMONIC_XOR && instruction.operand_count_visible > 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
        ZydisRegister destReg = operands[0].reg.value;

        // 获取目标寄存器的当前值
        uintptr_t currentValue = GetRegisterValue(context, destReg);

        // 计算新的值：当前值 XOR 从内存读取的值
        uintptr_t newValue = currentValue ^ value;

        SetRegisterValue(context, destReg, newValue);
        context->Rip += instruction.length;
        return true;
    }

    if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL && instruction.operand_count_visible == 1 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {

        // 从内存中读取的是函数指针
        uintptr_t functionPointer = value;
        // 获取返回地址（下一条指令）
        uintptr_t returnAddress = context->Rip + instruction.length;

        // 将返回地址压入栈中
        context->Rsp -= 8; // 64 位系统，栈是 8 字节对齐
        SIZE_T bytesWritten;
        if (!WriteProcessMemory(GetCurrentProcess(), (LPVOID)context->Rsp, &returnAddress, 8, &bytesWritten)) {
            std::cout << "压入返回地址失败" << std::endl;
            return false;
        }

        // 设置指令指针为函数地址
        context->Rip = functionPointer;

        return true;
    }

    std::cout << "不支持的指令: " << ZydisMnemonicGetString(instruction.mnemonic) << std::endl;
    return false;
}


LONG WINAPI GameLogic::VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord || !ExceptionInfo->ContextRecord) {
        // 无法安全处理
        return EXCEPTION_CONTINUE_SEARCH;
    }

    PEXCEPTION_RECORD exceptionRecord = ExceptionInfo->ExceptionRecord;
    PCONTEXT context = ExceptionInfo->ContextRecord;

    if (exceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (exceptionRecord->NumberParameters < 2) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    uintptr_t faultAddress = exceptionRecord->ExceptionInformation[1];
    if ((faultAddress & MAGIC_MASK) != MAGIC)
    {
        // 不是我们的魔法地址，让系统处理
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (!s_instance) {
        printf("VEH Error: s_instance is NULL! Cannot read remote memory.\n");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    uint8_t* instructionPointer = reinterpret_cast<uint8_t*>(context->Rip);
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

    if (ZYAN_FAILED(ZydisDecoderDecodeFull(&g_decoder, instructionPointer, ZYDIS_MAX_INSTRUCTION_LENGTH, &instruction, operands))) {
        printf("VEH: Zydis decode failed at 0x%p. Aborting simulation.\n", (void*)context->Rip);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    int memOperandIndex = -1;
    for (int i = 0; i < instruction.operand_count; ++i) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
            memOperandIndex = i;
            break;
        }
    }

    if (memOperandIndex == -1) {
        printf("VEH: Crashing instruction at 0x%p has no memory operand. Aborting.\n", (void*)context->Rip);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 获取内存操作数的大小（例如：8, 16, 32, 64 位）
    ZyanU16 memOpSize = operands[memOperandIndex].size;

    uintptr_t realReadAddress = faultAddress ^ MAGIC;
    uint64_t remote_value_read = 0; // 零初始化

    try
    {
        // 使用 __try 块来保护 s_instance->read 本身
        // 这是防止双重错误的最后一道防线
        switch (memOpSize)
        {
        case 8:  remote_value_read = s_instance->read<uint8_t>(realReadAddress); break;
        case 16: remote_value_read = s_instance->read<uint16_t>(realReadAddress); break;
        case 32: remote_value_read = s_instance->read<uint32_t>(realReadAddress); break; // 你的 imul 会走这里
        case 64: remote_value_read = s_instance->read<uint64_t>(realReadAddress); break;
        default:
            printf("VEH: Unsupported memory operand size (%d) at 0x%p. Aborting.\n", memOpSize, (void*)context->Rip);
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }
    catch (...) // 捕获 s_instance->read 可能抛出的任何C++异常（如果它会抛出的话）
    {
        printf("VEH: s_instance->read C++ exception. Aborting simulation.\n");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 这里会导致 crash，2025/10/31
    //if (memOpSize == 64)
    //{
    //    bool is_invalid_pointer = (remote_value_read == 0) ||
    //        (remote_value_read < 0x10000) ||
    //        ((remote_value_read & 0xFFFF000000000000) == 0xFFFF000000000000);

    //    if (is_invalid_pointer)
    //    {
    //        printf("VEH Error: Remote 64-bit read at 0x%p returned INVALID POINTER (Value=0x%llX). Aborting.\n",
    //            (void*)realReadAddress, remote_value_read);
    //        return EXCEPTION_CONTINUE_SEARCH;
    //    }
    //}

    // 调用模拟函数
    // remote_value_read 现在是一个被安全读取并零扩展到 64 位的
    bool simulation_success = FixBaseDisplacementMemoryAccess(context, remote_value_read);

    if (simulation_success)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else
    {
        // 模拟失败（例如，不支持的指令）
        printf("VEH: FixBase FAILED (Instruction at 0x%p not supported?).\n", (void*)context->Rip);
        // 让 __except 块来捕获
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

bool GameLogic::CallDecFuncSafely(DecFunc_t DecFunc, ULONG64 remoteTableAddr, int* position, DWORD size, WORD handler) {
    __try {
        g_remotePointerCache.clear();
        DecFunc(MAGIC | remoteTableAddr, position, size, handler);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[CallDecFunc] Exception caught: 0x" << std::hex
            << GetExceptionCode() << std::dec << std::endl;
        return false;
    }
}

// 辅助函数：定义什么是异常坐标
// 我们只检查 a3 == 12 (Vector3) 的情况
static bool is_decrypted_coord_abnormal(const int* a2, unsigned int a3)
{
    // 如果不是在解密 Vector3 (12 字节)，我们不进行检查
    if (a3 != 12) {
        return false;
    }

    // 将 int* 转回 float* 来读取坐标
    const float* coords = reinterpret_cast<const float*>(a2);
    const float x = coords[0];
    const float y = coords[1];
    const float z = coords[2];


    const float threshold = 1.0e7f;

    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        std::fabs(x) > threshold ||
        std::fabs(y) > threshold ||
        std::fabs(z) > threshold)
    {
        // 坐标是无穷大、NaN 或 超出阈值
        return true; // 判定为异常
    }

    // 坐标在正常范围内
    return false;
}

// 动态解密
void __fastcall GameLogic::decrypt(ULONG32 Pid, int* a2, unsigned int a3, __int16 a4) const
{
    // 如果句柄是 0xffff，说明未加密，直接返回
    if (a4 == 0xffff)
    {
        //std::cout << "[Main] Data is NOT encrypted (handler=0xffff)..." << std::endl;
        return;
    }

    // 读取游戏当前使用的解密函数包装器
    uintptr_t decFuncPtr = read<uintptr_t>(0x15432F5A8);
    uintptr_t targetFuncRVA;
    uintptr_t encTableAddr;

    ULONG64 effectiveTableAddr = 0;

    switch (decFuncPtr)
    {
    case 0x142843E80:
        //std::cout << "[Main] Using decrypt variant 1" << std::endl;
        targetFuncRVA = 0x1427DEC50 - IMAGE_BASE;  // 对应 sub_1427DEC50
        encTableAddr = 0x153FBF8B0;
        effectiveTableAddr = encTableAddr;
        break;

    case 0x142843EA0:
        //std::cout << "[Main] Using decrypt variant 2" << std::endl;
        targetFuncRVA = 0x1427ECBC0 - IMAGE_BASE;  // 对应 sub_1427ECBC0
        encTableAddr = 0x153FBF838;
        effectiveTableAddr = encTableAddr;
        break;

    case 0x142843EC0:
        //std::cout << "[Main] Using decrypt variant 3" << std::endl;
        targetFuncRVA = 0x1427FAD40 - IMAGE_BASE;  // 对应 sub_1427FAD40
        encTableAddr = 0x153FBF7C8;
        effectiveTableAddr = encTableAddr;
        break;

    case 0x142843EE0:
        //std::cout << "[Main] Using decrypt variant 4" << std::endl;
        targetFuncRVA = 0x1428097D0 - IMAGE_BASE;  // 对应 sub_1428097D0
        encTableAddr = 0x153FBF710;
        effectiveTableAddr = encTableAddr;
        break;

    case 0x142843F00:
        //std::cout << "[Main] Using decrypt variant 5" << std::endl;
        targetFuncRVA = 0x1428177F0 - IMAGE_BASE;  // 对应 sub_1428177F0
        encTableAddr = 0x153FBF990;
        effectiveTableAddr = encTableAddr;
        break;

    case 0x142843F20:
        //std::cout << "[Main] Using decrypt variant 6" << std::endl;
        targetFuncRVA = 0x1428261A0 - IMAGE_BASE;  // 对应 sub_1428261A0
        encTableAddr = 0x153FBF4F0;
        effectiveTableAddr = read<ULONG64>(encTableAddr);
        break;

    case 0x142843F40:
        //std::cout << "[Main] Using decrypt variant 7" << std::endl;
        targetFuncRVA = 0x142834A10 - IMAGE_BASE;
        encTableAddr = 0x153FBF4F8;
        effectiveTableAddr = read<ULONG64>(encTableAddr);
        break;

    case 0x142843F60:
        std::cout << "[Main] Using decrypt variant 8" << std::endl;
        targetFuncRVA = 0x14099D820 - IMAGE_BASE;  // 对应 sub_14099D820
        encTableAddr = 0x15435AB98;
        effectiveTableAddr = encTableAddr;
        break;

    default:
        std::cerr << "[Main] Unknown decrypt function: 0x" << std::hex << decFuncPtr << std::dec << std::endl;
        return;
    }

    if (effectiveTableAddr == 0 && (decFuncPtr == 0x142843F20 || decFuncPtr == 0x142843F40))
    {
        return;
    }


    // 加载对应的函数
    void* funcPtr = this->loader.GetFunctionByRVA(targetFuncRVA);
    if (!funcPtr) {
        std::cerr << "[Main] Failed to load function!" << std::endl;
        return;
    }

    DecFunc_t DecFunc = (DecFunc_t)funcPtr;

    if (CallDecFuncSafely(DecFunc, MAGIC | effectiveTableAddr, a2, a3, a4)) {
        // 解密成功，什么也不用做
    }
    else {
        std::cerr << "[Main] DecFunc failed or crashed\n";
    }
}

// 解密 >> 4
ULONG64 GameLogic::decrypt_shift(ULONG64 Address) const
{
    static ULONG32 dword_154350830;
    if (!dword_154350830)
    {
        dword_154350830 = read<ULONG32>(0x154350830);
    }
    static ULONG32 dword_1543507E8;
    if (!dword_1543507E8)
    {
        dword_1543507E8 = read<ULONG32>(0x1543507E8);
    }

    ULONG64 Temp = read<ULONG64>(Address);
    ULONG64 Ret = Temp >> 4;

    if (!Ret) return NULL;

    ULONG32 Offset = Temp & 0xF;
    uintptr_t targetFuncRVA = 0;
    uintptr_t encTableAddr = 0;

    ULONG64 effectiveTableAddr = 0;
    if (_bittest(reinterpret_cast<const LONG*>(&dword_154350830), Offset))
    {
        switch (dword_1543507E8)
        {
        case 1:
            //sub_14099D820(&qword_15435AB98, (int*)&v9, 4u, v5);
            //return v9 & 0xFFFFFFFFFFFFFFF0uLL;
            targetFuncRVA = 0x14099D820 - IMAGE_BASE;  // 对应 sub_14099D820
            encTableAddr = 0x15435AB98;
            effectiveTableAddr = encTableAddr;
            break;
        case 2:
            //sub_140973130(&qword_15435ABA8, &v9, 4, v5);
            //return v9 & 0xFFFFFFFFFFFFFFF0uLL;
            targetFuncRVA = 0x140973130 - IMAGE_BASE;  // 对应 sub_140973130
            encTableAddr = 0x15435ABA8;
            effectiveTableAddr = encTableAddr;
            break;
        case 3:
            //sub_1409813D0(&qword_15435ABB8, &v9, 4, v5);
            //return v9 & 0xFFFFFFFFFFFFFFF0uLL;
            targetFuncRVA = 0x1409813D0 - IMAGE_BASE;  // 对应 sub_1409813D0
            encTableAddr = 0x15435ABB8;
            effectiveTableAddr = encTableAddr;
            break;
        case 4:
            //sub_14098F3A0(&qword_15435ABC8, &v9, 4, v5);
            //return v9 & 0xFFFFFFFFFFFFFFF0uLL;
            targetFuncRVA = 0x14098F3A0 - IMAGE_BASE;  // 对应 sub_14098F3A0
            encTableAddr = 0x15435ABC8;
            effectiveTableAddr = encTableAddr;
            break;
        }
    }

    // 加载对应的函数
    void* funcPtr = this->loader.GetFunctionByRVA(targetFuncRVA);
    if (!funcPtr) {
        std::cerr << "[Main] Failed to load function!" << std::endl;
        return Ret;
    }

    DecFunc_t DecFunc = (DecFunc_t)funcPtr;
    //decrypt(pid, (int*)&Ret, 4, Offset);
    if (CallDecFuncSafely(DecFunc, MAGIC | effectiveTableAddr, (int*)&Ret, 4, Offset)) {
        return Ret;
    }
    std::cerr << "[Main] DecFunc failed or crashed\n";

    return Ret;
}

// 读取加密坐标
inline Vector3 GameLogic::read_enc_location(ULONG64 comp_ptr) const
{
    FEncVector enc = read<FEncVector>(comp_ptr + Offsets::RelativeLocation);
    if (enc.EncHandler.bEncrypted) {
        decrypt(pid, reinterpret_cast<int*>(&enc.X), 12, enc.EncHandler.Index);
    }
    return { enc.X, enc.Y, enc.Z };
}

// ------------------------- dynamic decrypt end -------------------------

void GameLogic::start() {
    if (!initialize()) return;
    running = true;
    viewUpdateThread = std::thread(&GameLogic::viewAndSelfUpdateLoop, this);
    actorDiscoveryThread = std::thread(&GameLogic::actorDiscoveryLoop, this);
    playerStateUpdateThread = std::thread(&GameLogic::playerStateUpdateLoop, this);
    staticStateUpdateThread = std::thread(&GameLogic::staticStateUpdateLoop, this);
    consoleThread = std::thread(&GameLogic::consoleLoop, this);
    mapModelThread = std::thread(&GameLogic::mapModelLoop, this);
    aimbotThread = std::thread(&GameLogic::aimbotLoop, this);
}

void GameLogic::stop() {
    running = false;
    auto join = [](std::thread& t) { if (t.joinable()) t.join(); };
    join(viewUpdateThread);
    join(actorDiscoveryThread);
    join(playerStateUpdateThread);
    join(staticStateUpdateThread);
    join(consoleThread);
    join(mapModelThread);
    join(aimbotThread);

    RTModel::DelModel();
    std::cout << "[GameLogic] All threads stopped." << std::endl;
}


// 清空 decrypt 的缓存
void GameLogic::clear_decrypt_cache() const {
    std::unique_lock lock(decryptCacheMutex);
    cachedDecFuncPtr = 0;
    cachedDecFunc = nullptr;
    cachedEffectiveTableAddr = 0;
}

// 清空 decrypt_shift 的缓存
void GameLogic::clear_shift_cache() const {
    std::unique_lock lock(shiftCacheMutex);
    shiftCache.clear();
}

std::string GameLogic::get_main_world(ULONG64 Uworld)
{
    std::string Ret_Name = "";

    ULONG64 WorldComposition = read<ULONG64>(Uworld + Offsets::WorldComposition);
    ULONG32 MainWorld = read<ULONG32>(WorldComposition + Offsets::MainWorld);
    std::string MainWorld_Name = get_name_fast(MainWorld);

    if (MainWorld_Name.find("Iris_Entry") != std::string::npos)
    {
        Ret_Name = u8"Lobby";
    }
    else if (MainWorld_Name.find("Dam_Iris_") != std::string::npos)
    {
        Ret_Name = u8"Dam";
    }
    else if (MainWorld_Name.find("Forrest_") != std::string::npos) {
        Ret_Name = u8"Valley";
    }
    else if (MainWorld_Name.find("SpaceCenter_") != std::string::npos)
    {
        Ret_Name = u8"Space Center";
    }
    else if (MainWorld_Name.find("Brakkesh_") != std::string::npos)
    {
        Ret_Name = u8"Baqqash";
    }
    else if (MainWorld_Name.find("Prison") != std::string::npos)
    {
        Ret_Name = u8"Tidal Prison";
    }
    else
    {
        Ret_Name = u8"NULL";
    }

    std::cout << "[GameLogic] Get_MainWorld_Name. " << std::hex << "WorldComposition: 0x" << WorldComposition << ", MainWorld: " << MainWorld << ", MainWorld_Name: " << MainWorld_Name << ", Ret_Name: " << Ret_Name << std::dec << std::endl;

    return Ret_Name;
}

// 地图模型加载线程
void GameLogic::mapModelLoop() {
    using namespace std::chrono_literals;

    std::string lastMapName = "";
    bool lastInMatch = false;

    // 针对当前已加载模型，记录文件的最后修改时间
    fs::file_time_type loadedModelMTime{};
    // 同一张地图下的重试计数
    int retryCount = 0;
    // 最大重试次数
    constexpr int kMaxRetries = 10;

    auto getModelPathFor = [](const std::string& mapName) -> std::string {
        if (mapName == "Dam")           return "LModes/linghaodaba";
        if (mapName == "Valley")        return "LModes/changgongxigu";
        if (mapName == "Baqqash")       return "LModes/bakeshen";
        if (mapName == "Space Center")  return "LModes/hangtianjidi";
        if (mapName == "Tidal Prison")  return "LModes/chaoxijianyu";
        // Lobby 或未知地图不加载
        return "";
        };

    auto safeGetMTime = [](const std::string& p) -> std::optional<fs::file_time_type> {
        try {
            if (p.empty()) return std::nullopt;
            if (!fs::exists(p)) return std::nullopt;
            return fs::last_write_time(p);
        }
        catch (...) {
            return std::nullopt;
        }
        };

    while (running) {
        bool inMatch = false;
        {
            std::shared_lock lk(sharedData.mutex);
            inMatch = sharedData.is_in_match;
        }

        if (!inMatch) {
            // 刚刚离开比赛：清理状态
            if (lastInMatch) {
                std::cout << "[PhysX] Left match. Unloading map model..." << std::endl;
                RTModel::DelModel();
                RTModel::Init();
                mapLoaded = false;
                lastMapName.clear();
                currentMapName.clear();
                retryCount = 0;
                loadedModelMTime = {};
            }
            lastInMatch = false;
            std::this_thread::sleep_for(1s);
            continue;
        }

        lastInMatch = true;

        // 识别当前地图
        ULONGLONG uworld = read<ULONGLONG>(baseAddress + Offsets::Uworld);
        if (!uworld) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        std::string mapName = get_main_world(uworld);
        currentMapName = mapName;
        const std::string modelPath = getModelPathFor(mapName);

        // 地图发生变化：重置状态
        if (mapName != lastMapName) {
            retryCount = 0; // 新地图从 0 次开始计数
            // 若上一张地图已加载，切换地图时先清理
            if (mapLoaded) {
                std::cout << "[PhysX] Map changed from " << lastMapName
                    << " to " << mapName << ". Reloading..." << std::endl;
                RTModel::DelModel();
                RTModel::Init();
                mapLoaded = false;
                loadedModelMTime = {};
            }
            lastMapName = mapName;
        }

        // Lobby 或未知地图：不加载，稍后再看
        if (modelPath.empty()) {
            std::this_thread::sleep_for(500ms);
            continue;
        }

        // 文件热重载：检测模型文件时间戳是否变化
        if (enableFileHotReload && mapLoaded) {
            if (auto mt = safeGetMTime(modelPath)) {
                if (loadedModelMTime != fs::file_time_type{} && *mt != loadedModelMTime) {
                    std::cout << "[PhysX] Detected model file change for " << modelPath
                        << ". Hot reloading..." << std::endl;
                    reloadRequested.store(true, std::memory_order_relaxed);
                }
            }
        }

        //  手动热重载：外部把 reloadRequested 置 true 即可触发
        if (mapLoaded && reloadRequested.exchange(false, std::memory_order_acq_rel)) {
            RTModel::DelModel();
            RTModel::Init();
            mapLoaded = false;
            // 重置计数以便重新加载
            retryCount = 0;
            loadedModelMTime = {};
            std::cout << "[PhysX] Hot reload requested. Model unloaded, will reload." << std::endl;
        }

        // 若已经加载且没有热重载需求，则放松循环频率
        if (mapLoaded) {
            std::this_thread::sleep_for(2s);
            continue;
        }

        // 执行加载 //
        // 只要在同一张地图 & 仍在对局内 & 未超出最大重试次数，就持续尝试
        while (running && inMatch && !mapLoaded && retryCount < kMaxRetries && currentMapName == mapName) {
            std::cout << "[PhysX] Loading map: " << mapName
                << " (model: " << modelPath << "), attempt "
                << (retryCount + 1) << "/" << kMaxRetries << "..." << std::endl;

            // 每次尝试前确保物理引擎是干净的
            RTModel::DelModel();
            RTModel::Init();

            bool loadSuccess = RTModel::OfflineUpdate(modelPath);
            if (loadSuccess) {
                std::cout << "[PhysX] Map model loaded successfully." << std::endl;
                mapLoaded = true;
                retryCount = 0;

                // 记录文件修改时间用于文件热重载
                if (auto mt = safeGetMTime(modelPath)) {
                    loadedModelMTime = *mt;
                }
                else {
                    loadedModelMTime = {};
                }
                break;
            }
            ++retryCount;
            std::cerr << "[PhysX] FAILED to load map model: " << modelPath
                << " (attempt " << retryCount << "/" << kMaxRetries << ")" << std::endl;

            // 退避一小会：可按需改为指数退避
            std::this_thread::sleep_for(800ms);

            // 更新 inMatch 状态，避免卡在重试循环
            {
                std::shared_lock lk(sharedData.mutex);
                inMatch = sharedData.is_in_match;
            }
        }

        // 达到最大重试次数仍失败：等待地图或状态变化再说
        if (!mapLoaded && retryCount >= kMaxRetries) {
            std::cerr << "[PhysX] Reached max retry count for map '" << mapName
                << "'. Will wait for state/map change before trying again." << std::endl;
            std::this_thread::sleep_for(5s);
        }
        else {
            // 正常巡检节奏
            std::this_thread::sleep_for(2s);
        }
    }
}

// 检查骨骼世界坐标是否有效
static bool is_valid_bone_position(const DirectX::XMFLOAT3& pos) {
    if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z) ||
        std::isinf(pos.x) || std::isinf(pos.y) || std::isinf(pos.z)) {
        return false;
    }

    // 检查异常大/小值
    const float threshold = 1.0e8f; // 例如: +/- 1亿
    if (std::abs(pos.x) > threshold || std::abs(pos.y) > threshold || std::abs(pos.z) > threshold) {
        return false; // 无效：数值超出合理范围
    }

    if (pos.x == 0.f && pos.y == 0.f && pos.z == 0.f) {
        return false; // 无效：坐标为0
    }

    return true; // 坐标有效
}

bool GameLogic::WorldToScreen(const Vector3& worldPos, DirectX::XMFLOAT2& screenPos) const {
    DirectX::XMFLOAT4X4 view_matrix_copy;
    {
        std::shared_lock lock(sharedData.mutex);
        view_matrix_copy = sharedData.view_matrix;
    }

    const DirectX::XMMATRIX m = XMLoadFloat4x4(&view_matrix_copy);
    const DirectX::XMVECTOR v = DirectX::XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    const DirectX::XMVECTOR clip = XMVector4Transform(v, m);
    const float w = DirectX::XMVectorGetW(clip);
    if (w <= 0.01f) return false;
    const DirectX::XMVECTOR ndc = DirectX::XMVectorScale(clip, 1.0f / w);
    const float cx = static_cast<float>(g_game_width) * 0.5f;
    const float cy = static_cast<float>(g_game_height) * 0.5f;
    screenPos.x = cx + DirectX::XMVectorGetX(ndc) * cx;
    screenPos.y = cy - DirectX::XMVectorGetY(ndc) * cy;
    return true;
}

std::filesystem::path GameLogic::get_temp_lua_path() {
    return std::filesystem::temp_directory_path() / "configc.lua";
}

bool GameLogic::atomic_write_text(const std::filesystem::path& path, const std::string& text) {
    try {
        std::filesystem::path temp_path = path;
        temp_path += ".tmp";

        std::ofstream ofs(temp_path, std::ios::out | std::ios::trunc);
        if (!ofs) {
            std::cerr << "[Aimbot] Cannot open temp file for writing: " << temp_path << std::endl;
            return false;
        }
        ofs << text;
        ofs.close();

        if (!ofs) {
            std::cerr << "[Aimbot] Failed to write to temp file: " << temp_path << std::endl;
            std::filesystem::remove(temp_path);
            return false;
        }

        std::filesystem::rename(temp_path, path); // 原子性替换
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[Aimbot] Filesystem error during atomic write: " << e.what() << std::endl;
        // 尝试非原子写入作为后备
        try {
            std::ofstream ofs_fallback(path, std::ios::out | std::ios::trunc);
            if (ofs_fallback) {
                ofs_fallback << text;
                ofs_fallback.close();
                return ofs_fallback.good();
            }
        }
        catch (...) {}
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "[Aimbot] Exception during atomic write: " << e.what() << std::endl;
        return false;
    }
}

// ------------------ MAP ------------------ 
uint64_t GameLogic::get_map_object(uint64_t PlayerController) {
    if (!PlayerController) return 0;

    uint64_t MyHUD = read<uint64_t>(PlayerController + Offsets::MyHUD);
    if (!MyHUD) return 0;

    uint64_t HUDarray = read<uint64_t>(MyHUD + Offsets::HUDarray);
    int32_t Count = read<int32_t>(MyHUD + Offsets::HUDarray + 8);

    //std::cout << "[GameLogic] GetMapObject: MyHUD=0x" << std::hex << MyHUD << ", HUDarray=0x" << HUDarray << ", Count=" << std::dec << Count << std::endl;

    if (Count > 0 && Count < 300)
    {
        for (int i = 0; i < Count; i++)
        {
            uint64_t Obj = read<uint64_t>(HUDarray + 8 * i);
            if (!Obj) continue;

            uint64_t ChildControllersArray = read<uint64_t>(Obj + Offsets::ChildControllers);
            int32_t TCount = read<int32_t>(Obj + Offsets::ChildControllers + 8);

            if (TCount > 0 && TCount < 300) {
                for (int j = 0; j < TCount; j++)
                {
                    uint64_t UBaseUIController = read<uint64_t>(ChildControllersArray + 8 * j);
                    if (!UBaseUIController) continue;

                    uint64_t View = read<uint64_t>(UBaseUIController + Offsets::View);
                    if (!View) continue;

                    std::string MapFname = get_name_fast(read<int>(View + Offsets::ObjectID));

                    // [cite: 1262-1263] 通过Fname查找
                    if (MapFname.find("WBP_MiniMap_Mobile_C") != std::string::npos) {
                        miniMapObjPtr = View;
                    }

                    if (MapFname.find("WBP_Map_Main_PC_C") != std::string::npos) {
                        mapObjPtr = View; // 缓存大地图对象
                    }

                    //std::cout << "[GameLogic] GetMapObject: Obj=0x" << std::hex << Obj << ", UBaseUIController=0x" << UBaseUIController << ", View=0x" << View << ", MapFname=" << MapFname << std::dec << std::endl;
                }
            }
        }
    }
    return mapObjPtr;
}

/**
 * @brief 通过已解密的 RootComponent 指针快速获取 Yaw
 */
float GameLogic::get_my_yaw_from_root(ULONGLONG root_component_ptr) const
{
    // 检查指针有效性
    if (root_component_ptr)
    {
        // 读取 FTransform
        FTransform c2w = read<FTransform>(root_component_ptr + Offsets::ComponentToWorld);

        // 获取旋转四元数
        const auto& q = c2w.Rotation;

        // const float YawY = 2.0f * (q.W * q.Z + q.X * q.Y);
        const float YawY = 2.0f * (q.w * q.z + q.x * q.y);

        // const float YawX = (1.0f - 2.0f * (q.Y * q.Y + q.Z * q.Z));
        const float YawX = (1.0f - 2.0f * (q.y * q.y + q.z * q.z));

        // Yaw = atan2f(YawY, YawX) * RAD_TO_DEG;
        float yawRadians = atan2f(YawY, YawX);
        return DirectX::XMConvertToDegrees(yawRadians);
    }
    return 0.0f;
}

// 其实地图上已经显示队友及本人朝向了，此函数的作用不是很大，但保留以备不时之需
float GameLogic::get_my_yaw() const
{
    if (cached_player_controller_ptr)
    {
        if (sharedData.my_pawn_ptr)
        {
            ULONGLONG root_component_ptr = decrypt_shift(sharedData.my_pawn_ptr + Offsets::RootComponent);
            return get_my_yaw_from_root(root_component_ptr);
        }
    }
    return 0.0f;
}

// ------------------------------------ 

USHORT GameLogic::calculate_xor_key1(int name_length) {
    USHORT xor_key;
    constexpr int key1 = 31, key2 = 223, key3 = 207, key4 = 12, key5 = 64, key6 = 128, key7 = 127;
    switch (name_length % 9) {
    case 0: xor_key = (name_length + (name_length & key1) + key6) | key7; break;
    case 1: xor_key = (name_length + (name_length ^ key2) + key6) | key7; break;
    case 2: xor_key = (name_length + (name_length | key3) + key6) | key7; break;
    case 3: xor_key = (33 * name_length + key6) | key7; break;
    case 4: xor_key = (name_length + (name_length >> 2) + key6) | key7; break;
    case 5: xor_key = (3 * name_length + 133) | key7; break;
    case 6: xor_key = (name_length + ((4 * name_length) | 5) + key6) | key7; break;
    case 7: xor_key = (name_length + ((name_length >> 4) | 7) + key6) | key7; break;
    case 8: xor_key = (name_length + (name_length ^ key4) + key6) | key7; break;
    default: xor_key = (name_length + (name_length ^ key5) + key6) | key7; break;
    }
    return xor_key;
}

std::string GameLogic::resolve_name_internal(int key) const
{
    if (key <= 0) return "";
    int chunk_index = key >> 18;
    int name_offset = key & 0x3FFFF;
    ULONGLONG gname_ptr = baseAddress + Offsets::Gname;
    ULONGLONG name_pool_chunk_ptr = read<ULONGLONG>(gname_ptr + static_cast<unsigned long long>(chunk_index) * 8 + 8);
    if (!name_pool_chunk_ptr) return "";
    ULONGLONG name_entry_address = name_pool_chunk_ptr + static_cast<unsigned long long>(name_offset) * 2;
    USHORT name_entry = read<USHORT>(name_entry_address);
    if (name_entry == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        name_entry = read<USHORT>(name_entry_address);
        if (name_entry == 0) return "";
    }
    int name_length = name_entry >> 6;
    if (name_length <= 0 || name_length > 1024) return "";
    bool is_wide = (name_entry & 1) != 0;
    USHORT xor_key = calculate_xor_key1(name_length);

    if (is_wide) {
        std::vector<BYTE> wide_buffer_bytes(static_cast<size_t>(name_length) * 2);
        if (!read_bytes(name_entry_address + 2, wide_buffer_bytes.data(), wide_buffer_bytes.size())) return "";
        for (int i = 0; i < name_length * 2; i += 4) {
            if (i + 1 < name_length * 2) { *reinterpret_cast<USHORT*>(&wide_buffer_bytes[i]) ^= xor_key; }
        }
        std::wstring w(reinterpret_cast<wchar_t*>(wide_buffer_bytes.data()), name_length);
        char narrow[1024] = { 0 };
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, narrow, sizeof(narrow) - 1, NULL, NULL);
        return std::string(narrow);
    }
    else {
        std::vector<char> buffer(name_length + 1, 0);
        if (!read_bytes(name_entry_address + 2, buffer.data(), name_length)) return "";
        for (int i = 0; i < name_length; ++i) buffer[i] ^= static_cast<char>(xor_key & 0xFF);
        return std::string(buffer.data());
    }
}

std::string GameLogic::get_name_fast(int key) {
    if (key <= 0) return "";
    {
        std::shared_lock lk(nameCacheMutex);
        auto it = nameCache.find(key);
        if (it != nameCache.end()) return it->second;
    }
    std::string name = resolve_name_internal(key);
    if (!name.empty()) {
        std::unique_lock lk(nameCacheMutex);
        nameCache.emplace(key, name);
    }
    return name;
}

Vector3 GameLogic::get_actor_location(ULONGLONG actor_ptr) const
{
    if (!actor_ptr) return { 0, 0, 0 };

    //ULONG64 root_component_ptr = read<ULONGLONG>(actor_ptr + Offsets::RootComponent) >> 4;
    ULONG64 root_component_ptr = decrypt_shift(actor_ptr + Offsets::RootComponent);
    if (!root_component_ptr) return { 0, 0, 0 };

    FEncVector encrypted_location = read<FEncVector>(root_component_ptr + Offsets::RelativeLocation);

    if (encrypted_location.EncHandler.bEncrypted) {
        if (pid == 0) {
            std::cerr << "[Error] get_actor_location: PID is not set!" << std::endl;
            return { 0, 0, 0 };
        }

        decrypt(pid, reinterpret_cast<int*>(&encrypted_location.X), 12, encrypted_location.EncHandler.Index);

        if (g_cfg.debug.load()) {
            std::cout << "[Debug] Actor Location Decrypted - X: " << encrypted_location.X
                << ", Y: " << encrypted_location.Y
                << ", Z: " << encrypted_location.Z
                << ", Index: " << encrypted_location.EncHandler.Index << std::endl;
        }
    }

    Vector3 result_location = { encrypted_location.X, encrypted_location.Y, encrypted_location.Z };
    if (!valid_location(result_location)) {
        if (g_cfg.debug.load()) {
            std::cerr << "[Debug] get_actor_location: Invalid or failed location check: ActorPtr=0x" << std::hex << actor_ptr
                << ", RootCompPtr=0x" << root_component_ptr << std::dec
                << ", Coords=(" << result_location.x << ", " << result_location.y << ", " << result_location.z << ")" << std::endl;
        }
        return { 0, 0, 0 };
    }

    return result_location;
}

void GameLogic::get_bone_world_positions(ULONGLONG mesh_ptr, DirectX::XMFLOAT3* out_skeleton) const {
    using namespace DirectX;
    ULONGLONG bone_array_ptr = read<ULONGLONG>(mesh_ptr + Offsets::BoneArray);
    if (!bone_array_ptr) return;
    std::vector<FBoneTransform> bone_transforms(NUM_BONES);
    read_bytes(bone_array_ptr, bone_transforms.data(), NUM_BONES * sizeof(FBoneTransform));
    FTransform component_to_world = read<FTransform>(mesh_ptr + Offsets::ComponentToWorld);

    if (component_to_world.EncHandler.bEncrypted)
    {
        if (pid != 0) {
            decrypt(pid, reinterpret_cast<int*>(&component_to_world.Translation), sizeof(Vector3), component_to_world.EncHandler.Index);
            if (g_cfg.debug.load()) {
                std::cout << u8"[DEBUG BONE] C2W - Translation: ("
                    << component_to_world.Translation.x << ", " << component_to_world.Translation.y << ", " << component_to_world.Translation.z << ")" << std::endl;
            }
        }
        else {}
    }

    XMMATRIX component_matrix = FTransformToMatrix(component_to_world);
    for (int i = 0; i < NUM_BONES; ++i) {
        XMMATRIX bone_matrix = FBoneTransformToMatrix(bone_transforms[i]);
        XMMATRIX world_matrix = bone_matrix * component_matrix;
        out_skeleton[i].x = world_matrix.r[3].m128_f32[0];
        out_skeleton[i].y = world_matrix.r[3].m128_f32[1];
        out_skeleton[i].z = world_matrix.r[3].m128_f32[2];
    }
}


bool GameLogic::name_blacklist_strict(const std::string& n) {
    return false;
}

bool GameLogic::container_strict_blacklist(const std::string& n) {
    static const char* BL2[] = { "LF_Transport_Ai", "SpawnPoint_Safe", "BP_EFX_Sence_Ai", "HallDisplayCame", "BP_ForceSafeBox" };
    for (auto* k : BL2) if (n.find(k) != std::string::npos) return true;
    return false;
}

// TODO: 进一步完善，部分小保险柜被误识别
bool GameLogic::match_container_keywords(const std::string& n) {
    static const char* KW[] = {
        "SafeBox", "Safe", "Vault", "Locker", "Chest", "WarChest", "Case", "Suitcase", "TravelCase",
        "StorageBox", "Cabinet", "Tool", "ToolBox", "Tool_Cabinet", "Drawer", "ComputerCase", "Server",
        "Hacker_PC", "Terminal", "Console", "Medical", "MedicalPile", "FirstAid", "Ammo", "Ammunition",
        "HiddenStash", "Stash", "ExpressBox", "Package", "Parcel", u8"保险箱", u8"保险柜", u8"工具柜",
        u8"工具箱", u8"抽屉", u8"储物箱", u8"储物柜", u8"战备箱", u8"电脑机箱", u8"医疗箱", u8"弹药箱",
        u8"快递箱", u8"藏匿物", u8"口袋", u8"服务器", "航空", "Cloth", "Suit", "BirdNest", "Aviation", "Air"
    };
    if (container_strict_blacklist(n)) return false;
    for (auto* k : KW) if (n.find(k) != std::string::npos) return true;
    return false;
}

bool GameLogic::valid_location(const Vector3& p) {
    auto bad = [](float x) { return !std::isfinite(x) || std::fabs(x) > 1.0e6f; };
    if (bad(p.x) || bad(p.y) || bad(p.z)) return false;
    if (p.x == 0.f && p.y == 0.f && p.z == 0.f) return false;
    return true;
}

bool GameLogic::is_in_match_check(ULONGLONG my_pawn) {
    if (!my_pawn) return false;
    return true;
}

int GameLogic::categorize_armor_level(BYTE raw_level) {
    if (raw_level >= 105 && raw_level <= 108) return 1;
    if (raw_level >= 81 && raw_level <= 84) return 2;
    if (raw_level >= 57 && raw_level <= 60) return 3;
    if (raw_level >= 33 && raw_level <= 36) return 4;
    if (raw_level >= 9 && raw_level <= 12) return 5;
    if (raw_level >= 241 && raw_level <= 244) return 6;
    return 0;
}

bool GameLogic::is_dead_body_class(const std::string& name) {
    return (name.find("BP_Inventory_CarryBody_C") != std::string::npos) || (name.find("BP_Inventory_DeadBody_C") != std::string::npos);
}

StaticClass GameLogic::classify_by_marking_type(BYTE t) {
    switch (t) {
    case 1: return StaticClass::HackerPC;
    case 2: return StaticClass::Item;
    case 3: return StaticClass::Container;
    case 4: return StaticClass::Safe;
    default: return StaticClass::Unknown;
    }
}

std::string GameLogic::get_ai_type(ULONGLONG actor_ptr) const {
    ULONGLONG tag_ptr = read<ULONGLONG>(actor_ptr + Offsets::AICharacterTag);
    BYTE index = read<BYTE>(tag_ptr);
    switch (index) {
    case 0:  return u8"假人";
    case 1:  return u8"盾兵";
    case 2:  return u8"步兵";
    case 3:  return u8"狙击兵";
    case 4:  return u8"护盾兵";
    case 5:  return u8"机枪兵";
    case 6:  return u8"空降兵";
    case 8:  return u8"喷火兵";
    case 9:  return u8"火箭兵";
    case 10: return u8"BOSS";
    case 11: return u8"BoyBand";
    case 12: return u8"鳄鱼";
    case 13: return u8"非人类";
    default: return u8"未知AI(" + std::to_string(index) + ")";
    }
}

std::string GameLogic::get_operator_name(ULONGLONG player_state_ptr) const
{
    if (player_state_ptr > 0) {
        BYTE id = read<BYTE>(player_state_ptr + Offsets::HeroID);
        switch (id) {
        case 30: return u8"红狼";
        case 25: return u8"威龙";
        case 27: return u8"蜂医";
        case 29: return u8"牧羊人";
        case 35: return u8"乌鲁鲁";
        case 28: return u8"露娜";
        case 26: return u8"骇爪";
        case 36: return u8"蛊";
        case 37: return u8"深蓝";
        case 38: return u8"无名";
        case 39: return u8"疾风";
        case 40: return u8"银翼";
        default: return u8"干员(" + std::to_string(id) + ")";
        }
    }
    return u8"获取失败";
}

std::string GameLogic::get_item_display_name(const ULONGLONG item_component_ptr) const
{
    if (!item_component_ptr) return "";
    ULONGLONG t1 = read<ULONGLONG>(item_component_ptr + Offsets::ItemName_1);
    if (!t1) return "";
    ULONGLONG t2 = read<ULONGLONG>(t1 + Offsets::ItemName_1);
    if (!t2) return "";
    ULONGLONG t3 = read<ULONGLONG>(t2 + Offsets::ItemName_2);
    if (!t3) return "";
    ULONGLONG displayNamePtr = read<ULONGLONG>(t3 + Offsets::ItemName_3);
    if (!displayNamePtr) return "";

    wchar_t wbuf[64] = { 0 };
    if (read_bytes(displayNamePtr, wbuf, sizeof(wbuf) - sizeof(wchar_t))) {
        char narrow[128] = { 0 };
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, narrow, sizeof(narrow) - 1, NULL, NULL);
        return std::string(narrow);
    }
    return "";
}

// TODO: 进一步完善，部分容器可能仍然存在翻译错误
std::string GameLogic::translate_container_name(const std::string& fullName) {
    if (fullName.find(u8"大武器箱") != std::string::npos || fullName.find("Militarybox6") != std::string::npos) return u8"大武器箱";
    if (fullName.find(u8"野外战备箱") != std::string::npos || fullName.find("WarChest") != std::string::npos) return u8"野外战备箱";
    if (fullName.find(u8"航空储物箱") != std::string::npos || fullName.find("Aviation") != std::string::npos || fullName.find("AirCargoContainer") != std::string::npos) return u8"航空储物箱";
    if (fullName.find(u8"高级储物箱") != std::string::npos || fullName.find("AdvancedStorageBox") != std::string::npos) return u8"高级储物箱";
    if (fullName.find(u8"大工具盒") != std::string::npos || fullName.find("toolbox_big") != std::string::npos) return u8"收纳盒";
    if (fullName.find(u8"快递箱") != std::string::npos || fullName.find("ExpressBox") != std::string::npos || fullName.find("PaperBox") != std::string::npos || fullName.find("OfficeBox") != std::string::npos) return u8"快递箱";
    if (fullName.find(u8"藏匿物") != std::string::npos || fullName.find("HiddenStash") != std::string::npos || fullName.find("WellLid") != std::string::npos) return u8"藏匿物";
    if (fullName.find(u8"手提箱") != std::string::npos || fullName.find("Suitcase") != std::string::npos || fullName.find("Handsuitcase") != std::string::npos) return u8"手提箱";
    if (fullName.find("Lxrysuitcase") != std::string::npos || fullName.find("flightcase") != std::string::npos) return u8"高级旅行箱";
    if (fullName.find(u8"旅行包") != std::string::npos || fullName.find("TravelBag") != std::string::npos || fullName.find("ShoppingBag") != std::string::npos || fullName.find("operationalbag") != std::string::npos) return u8"旅行包";
    if (fullName.find(u8"大蛇皮口袋") != std::string::npos || fullName.find("BigBag") != std::string::npos) return u8"旅行包";
    if (fullName.find(u8"个人储物柜") != std::string::npos || fullName.find("Locker") != std::string::npos) return u8"储物柜";
    if (fullName.find(u8"小保险箱") != std::string::npos || fullName.find("SafeBox_S") != std::string::npos) return u8"小保险箱";
    if (fullName.find(u8"保险箱") != std::string::npos || fullName.find("SafeBox") != std::string::npos) return u8"保险箱";
    if (fullName.find(u8"保险柜") != std::string::npos || fullName.find("StrongBox") != std::string::npos) return u8"保险柜";
    if (fullName.find(u8"衣服") != std::string::npos || fullName.find("Cloth") != std::string::npos) return u8"衣服";
    if (fullName.find(u8"鸟窝") != std::string::npos || fullName.find("BirdNest") != std::string::npos || fullName.find("Nest") != std::string::npos) return u8"鸟窝";
    if (fullName.find(u8"垃圾") != std::string::npos || fullName.find("Garbage") != std::string::npos || fullName.find("Trash") != std::string::npos || fullName.find("Dupmster") != std::string::npos) return u8"垃圾箱";
    if (fullName.find(u8"医疗物资堆") != std::string::npos || fullName.find("MedicalPile") != std::string::npos || fullName.find("MedSupplyPile") != std::string::npos) return u8"医疗物资";
    if (fullName.find(u8"医疗包") != std::string::npos || fullName.find("MedicalKit") != std::string::npos) return u8"医疗包";
    if (fullName.find(u8"弹药箱") != std::string::npos || fullName.find("AmmoBox") != std::string::npos || fullName.find("Militarybox2") != std::string::npos) return u8"弹药箱";
    if (fullName.find(u8"工具柜") != std::string::npos || fullName.find("Tool_Cabinet") != std::string::npos) return u8"工具柜";
    if (fullName.find(u8"抽屉柜") != std::string::npos || fullName.find("filecabinet") != std::string::npos || fullName.find("CabinetDrawer") != std::string::npos) return u8"抽屉柜";
    if (fullName.find(u8"电脑机箱") != std::string::npos || fullName.find("ComputerCase") != std::string::npos) return u8"电脑机箱";
    if (fullName.find(u8"服务器") != std::string::npos || fullName.find("Server") != std::string::npos) return u8"服务器";
    if (fullName.find(u8"收纳盒") != std::string::npos || fullName.find("StorageBox") != std::string::npos || fullName.find("PlasticCrate") != std::string::npos || fullName.find("LF_StorageBox_A_4") != std::string::npos) return u8"收纳盒";
    if (fullName.find("Militarybox8") != std::string::npos || fullName.find("Militarybox7") != std::string::npos) return u8"野外物资箱";
    if (fullName.find("Supplies") != std::string::npos) return u8"罗德岛特殊补给箱";
    if (fullName.find("Hacker_PC") != std::string::npos || fullName.find("Interact_Computer") != std::string::npos) return u8"骇客电脑";
    if (fullName.find("BP_Interactor_CodedLock") != std::string::npos) return u8"密码门";

    return "";
}


// TODO: 骇客电脑的读取逻辑仍然存在问题，待进一步调试
std::optional<bool> GameLogic::try_read_open_state_once(const ULONGLONG actor, const StaticClass cls) const
{
    BYTE v = 0;
    if (cls == StaticClass::PlayerLootBox) {
        v = read<BYTE>(actor + Offsets::bLooted);
    }
    else {
        v = read<BYTE>(actor + Offsets::bIsOpened);
        if (cls == StaticClass::HackerPC)
        {
            v = read<BYTE>(actor + Offsets::bDonwload) == 0;
        }
    }
    if (v <= 1) return (v != 0);
    if (v == 2 || v == 3) return (v != 0);
    if (g_cfg.debug.load()) std::cerr << "[Debug] try_read_open_state_once: Read unexpected state value " << (int)v << " for actor 0x" << std::hex << actor << std::dec << std::endl;
    return std::nullopt;
}

std::string GameLogic::get_held_weapon(ULONGLONG actor_ptr) const
{
    if (!actor_ptr) return "";
    ULONGLONG weapon_ptr = read<ULONGLONG>(actor_ptr + Offsets::CacheCurWeapon);
    if (!weapon_ptr) return "";
    long long weapon_id = read<long long>(weapon_ptr + Offsets::WeaponID);
    return get_weapon_name_from_id(weapon_id);
}

// TODO: 完善武器ID映射表
std::string GameLogic::get_weapon_name_from_id(long long id) {
    if (id == 0) return "";
    static const std::unordered_map<long long, const char*> weapon_map = {
        {18010000017, "SG552"}, {18010000010, "AKS-74U"}, {18010000001, "M4A1"}, {18010000037, "AS-Val"},
        {18010000031, "CAR-15"}, {18010000024, "PTR-32"}, {18010000023, "G3"}, {18010000021, "SCAR-H"},
        {18010000018, "AK-12"}, {18010000016, "M7"}, {18010000015, "AUG"}, {18010000012, "ASh-12"},
        {18010000008, "QBZ95-1"}, {18010000006, "AKM"}, {18010000014, "M16A4"}, {18010000013, "K416"},
        {18010000038, "QBZ191"}, {18010000043, "MK47"},
        {18020000004, "UZI"}, {18020000010, "MP7"}, {18020000009, u8"勇士"}, {18020000008, "SR-3M"},
        {18020000006, "SMG-45"}, {18020000005, u8"野牛"}, {18020000003, "Vector"}, {18020000002, "P90"},
        {18020000001, "MP5"}, {18020000011, "QCQ171"},
        {18030000004, "M870"}, {18030000002, "S12K"}, {18030000001, "M1014"},
        {18040000002, "M249"}, {18040000001, "PKM"}, {18040000003, "M250"}, {18040000004, "QJB201"},
        {18050000031, "PSG-1"}, {18050000007, "SR-25"}, {18050000006, "SKS"}, {18050000005, "M14"},
        {18050000004, "SVD"}, {18050000003, "VSS"}, {18050000002, "Mini-14"}, {18050000008, "SR9"},
        {18050000032, u8"Marlin杠杆步枪"},
        {18060000011, "AWM"}, {18060000009, "M700"}, {18060000008, "R93"}, {18060000007, "SV-98"},
        {18070000010, "G17"}, {18070000006, "93R"}, {18070000005, "G18"}, {18070000004, u8"沙漠之鹰"},
        {18070000003, u8"357左轮"}, {18070000002, "QSZ92G"}, {18070000033, "M1911"},
        {18100000009, u8"小刀"}, {18100000011, u8"防爆盾"}, {18080000011, u8"虎蹲炮"}, {18080000010, u8"三联装手炮"},
        {18070000030, u8"激素枪"}, {18130000002, u8"侦察箭矢"}, {18130000001, u8"电击箭矢"},
        {21020300006, u8"声波陷阱"}, {21010000022, u8"破片手雷"}, {21020300007, u8"烟幕无人机"}, {21010000001, u8"增强型破片手雷"}
    };
    auto it = weapon_map.find(id);
    return (it != weapon_map.end()) ? it->second : u8"未知武器";
}

DirectX::XMMATRIX GameLogic::FTransformToMatrix(const FTransform& transform) {
    using namespace DirectX;
    XMMATRIX mScale = XMMatrixScaling(transform.Scale3D.x, transform.Scale3D.y, transform.Scale3D.z);
    XMMATRIX mRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&transform.Rotation));
    XMMATRIX mTranslation = XMMatrixTranslation(transform.Translation.x, transform.Translation.y, transform.Translation.z);
    return mScale * mRotation * mTranslation;
}

DirectX::XMMATRIX GameLogic::FBoneTransformToMatrix(const FBoneTransform& transform) {
    using namespace DirectX;
    XMMATRIX mScale = XMMatrixScaling(transform.Scale3D.x, transform.Scale3D.y, transform.Scale3D.z);
    XMMATRIX mRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&transform.Rotation));
    XMMATRIX mTranslation = XMMatrixTranslation(transform.Translation.x, transform.Translation.y, transform.Translation.z);
    return mScale * mRotation * mTranslation;
}

void GameLogic::clear_console() {
    COORD topLeft = { 0,0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

void GameLogic::viewAndSelfUpdateLoop() {
    while (running) {
        DirectX::XMFLOAT4X4 vm{};
        Vector3 myLoc{ 0,0,0 };
        int myTeam = -1;
        bool inMatch = false;
        ULONGLONG my_pawn = 0;
        bool isMapOpen = false;
        Mapinfo mapInfo{};
        float myYaw = 0.f;
        int mapId = 0;
        float current_fov = 90.0f;

        if (cached_player_controller_ptr && cached_view_matrix_final_ptr) {
            my_pawn = read<ULONGLONG>(cached_player_controller_ptr + Offsets::LocalPlayerPtr_PawnOffset);

            if (my_pawn && is_in_match_check(my_pawn)) {
                inMatch = true;

                if (cached_my_root_component_ptr == 0) {
                    // 仅在缓存为空时调用一次解密
                    cached_my_root_component_ptr = decrypt_shift(my_pawn + Offsets::RootComponent);
                    if (cached_my_root_component_ptr == 0) {
                        std::cerr << "[ViewLoop] Failed to get my root component, retrying next frame..." << std::endl;
                    }
                }

                if (cached_my_root_component_ptr) {
                    // read_enc_location 接受的就是 root_comp_ptr
                    myLoc = read_enc_location(cached_my_root_component_ptr);
                    // 使用新的辅助函数
                    myYaw = get_my_yaw_from_root(cached_my_root_component_ptr);
                }
                else {
                    // 如果 root_component_ptr 获取失败，则回退
                    myLoc = { 0,0,0 };
                    myYaw = 0.f;
                }

                read_bytes(cached_view_matrix_final_ptr, &vm, sizeof(vm));
                ULONGLONG my_ps = read<ULONGLONG>(my_pawn + Offsets::PlayerState);
                if (my_ps) myTeam = read<int>(my_ps + Offsets::TeamID + teamIdDelta);

                if (g_game_height > 0) {
                    float p_22 = vm._22;
                    if (p_22 > 0.001f) {
                        float vfov_rad = 2.0f * std::atan(1.0f / p_22);
                        float aspect_ratio = static_cast<float>(g_game_width) / static_cast<float>(g_game_height);
                        float hfov_rad = 2.0f * std::atan(std::tan(vfov_rad / 2.0f) * aspect_ratio);
                        current_fov = DirectX::XMConvertToDegrees(hfov_rad);

                        // 约束到一个合理的范围，防止矩阵变换导致异常值
                        if (current_fov < 50.0f || current_fov > 150.0f) {
                            current_fov = 90.0f;
                        }
                    }
                    else {
                        current_fov = 90.0f;
                    }
                }
                else {
                    current_fov = 90.0f;
                }

                // 读取地图信息
                if (mapObjPtr == 0) {
                    get_map_object(cached_player_controller_ptr);
                }

                if (mapObjPtr != 0) {
                    isMapOpen = read<char>(mapObjPtr + Offsets::preShouldDrawOffset) != 0;
                    mapInfo = read<Mapinfo>(mapObjPtr + Offsets::MapinfoOffset);
                }

                // myYaw 已在上面通过 get_my_yaw_from_root 获取

                // 读取 MapId
                ULONGLONG uworld_ptr = read<ULONGLONG>(baseAddress + Offsets::Uworld);
                if (uworld_ptr) {
                    ULONGLONG ADFMGameState = read<ULONGLONG>(uworld_ptr + Offsets::GameState);
                    if (ADFMGameState) {
                        mapId = read<int>(ADFMGameState + Offsets::MapConfig + 0x10);
                    }
                }
            }
            else {
                if (cached_my_root_component_ptr != 0) {
                    // 仅当我们确实在对局中时才打印
                    std::cout << "[ViewLoop] Left match. Clearing root component cache." << std::endl;
                }
                cached_player_controller_ptr = 0;
                cached_view_matrix_final_ptr = 0;
                cached_my_root_component_ptr = 0; // 清空自己的root
                inMatch = false;
                mapObjPtr = 0; // 退出对局时重置
                miniMapObjPtr = 0;
            }
        }

        if (!inMatch) {
            ULONGLONG lp1 = read<ULONGLONG>(baseAddress + Offsets::Uworld);
            if (lp1) {
                ULONGLONG lp2 = decrypt_shift(lp1 + Offsets::OwningGameInstance); // 仍然需要调用
                if (lp2) {
                    ULONGLONG lp3 = read<ULONGLONG>(lp2 + Offsets::LocalPlayers);
                    if (lp3) {
                        ULONGLONG lp4 = read<ULONGLONG>(lp3 + 0);  // TODO: 只读一遍
                        if (lp4) {
                            ULONGLONG player_controller = read<ULONGLONG>(lp4 + Offsets::PlayerController);
                            my_pawn = read<ULONGLONG>(player_controller + Offsets::LocalPlayerPtr_PawnOffset);

                            if (my_pawn && is_in_match_check(my_pawn)) {

                                // 刚找到Pawn，立即缓存所有指针
                                inMatch = true;
                                cached_player_controller_ptr = player_controller;

                                // 查找 ViewMatrix
                                ULONGLONG vmb = read<ULONGLONG>(baseAddress + Offsets::ViewMatrix_Base);
                                if (vmb) {
                                    ULONGLONG mp = read<ULONGLONG>(vmb + Offsets::ViewMatrix_Offset1) + Offsets::ViewMatrix_Offset2;
                                    if (mp) {
                                        cached_view_matrix_final_ptr = mp;
                                        read_bytes(cached_view_matrix_final_ptr, &vm, sizeof(vm));
                                    }
                                }

                                // 计算 FOV
                                float p_22 = vm._22;
                                if (p_22 > 0.001f && g_game_height > 0) {
                                    float vfov_rad = 2.0f * std::atan(1.0f / p_22);
                                    float aspect_ratio = static_cast<float>(g_game_width) / static_cast<float>(g_game_height);
                                    float hfov_rad = 2.0f * std::atan(std::tan(vfov_rad / 2.0f) * aspect_ratio);
                                    current_fov = DirectX::XMConvertToDegrees(hfov_rad);
                                    if (current_fov < 50.0f || current_fov > 150.0f) {
                                        current_fov = 90.0f;
                                    }
                                }
                                else {
                                    current_fov = 90.0f;
                                }

                                cached_my_root_component_ptr = decrypt_shift(my_pawn + Offsets::RootComponent);
                                if (cached_my_root_component_ptr) {
                                    myLoc = read_enc_location(cached_my_root_component_ptr);
                                    myYaw = get_my_yaw_from_root(cached_my_root_component_ptr);
                                }
                                else {
                                    std::cerr << "[ViewLoop] Failed to get my root component on match entry." << std::endl;
                                    myLoc = { 0,0,0 };
                                    myYaw = 0.f;
                                }

                                // 获取 TeamID
                                ULONGLONG my_ps = read<ULONGLONG>(my_pawn + Offsets::PlayerState);
                                if (my_ps) {
                                    int t0 = read<int>(my_ps + Offsets::TeamID);
                                    int t4 = read<int>(my_ps + Offsets::TeamID + 4);
                                    auto plausible = [](int v) { return v > 0 && v < 64; };
                                    teamIdDelta = (plausible(t4) && !plausible(t0)) ? 4 : 0;
                                    myTeam = read<int>(my_ps + Offsets::TeamID + teamIdDelta);
                                }

                                // 获取地图对象
                                get_map_object(cached_player_controller_ptr);
                                if (mapObjPtr != 0) {
                                    isMapOpen = read<char>(mapObjPtr + Offsets::preShouldDrawOffset) != 0;
                                    mapInfo = read<Mapinfo>(mapObjPtr + Offsets::MapinfoOffset);
                                }

                                // 获取 MapID
                                ULONGLONG ADFMGameState = read<ULONGLONG>(lp1 + Offsets::GameState);
                                if (ADFMGameState) {
                                    mapId = read<int>(ADFMGameState + Offsets::MapConfig + 0x10);
                                }
                            }
                        }
                    }
                }
            }
        }

        {
            std::unique_lock lk(sharedData.mutex);
            sharedData.my_location = myLoc;
            sharedData.my_team_id = myTeam;
            sharedData.is_in_match = inMatch;
            sharedData.my_pawn_ptr = my_pawn;
            sharedData.view_matrix = vm;
            sharedData.camera_fov = current_fov;
        }

        if (inMatch && networkServer.is_client_connected() && valid_location(myLoc)) {
            ViewUpdatePacket pkt;
            pkt.header.type = PacketType::VIEW_UPDATE;
            pkt.view_matrix = vm;
            pkt.my_location = { myLoc.x, myLoc.y, myLoc.z };
            pkt.game_width = g_game_width;
            pkt.game_height = g_game_height;
            pkt.is_main_map_open = isMapOpen;
            pkt.map_info = { mapInfo.X, mapInfo.Y, mapInfo.W, mapInfo.H, mapInfo.MapX, mapInfo.MapY, mapId };
            pkt.my_team_id = myTeam;
            pkt.my_pawn_ptr = my_pawn;
            pkt.my_yaw = myYaw;
            networkServer.send_packet(&pkt, sizeof(pkt));
            //std::cout << "mapinfo:" << mapInfo.X << "," << mapInfo.Y << "," << mapInfo.W << "," << mapInfo.H << "," << mapInfo.MapX << "," << mapInfo.MapY << ", mapId:" << mapId << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / 240));
    }
}

static std::mt19937 rng{ std::random_device{}() };
static std::uniform_real_distribution<float> dist_noise(-1.0f, 1.0f);

const std::vector g_valid_body_bones = {
    BoneID::Hips,
    BoneID::Head_Joint,
    BoneID::Head,
    BoneID::Neck,
    BoneID::RightArm,
    BoneID::RightForeArm,
    BoneID::RightForeArmTwist,
    BoneID::LeftArm,
    BoneID::LeftForeArm,
    BoneID::LeftForeArmTwist,
    BoneID::RightUpLeg,
    BoneID::RightLeg,
    BoneID::RightFoot,
    BoneID::LeftUpLeg,
    BoneID::LeftLeg,
    BoneID::LeftFoot,
};

bool GameLogic::read_is_firing_flag() const
{
    uint64_t my_pawn = 0;
    {
        std::shared_lock lk(sharedData.mutex);
        my_pawn = sharedData.my_pawn_ptr;
    }

    if (my_pawn == 0) {
        return false;
    }


    uint64_t blackboard_ptr = read<uint64_t>(my_pawn + Offsets::BlackBoardOffset);
    if (blackboard_ptr == 0) {
        return false;
    }

    char is_firing_flag = read<char>(blackboard_ptr + Offsets::bIsFiringOffset);

    return (is_firing_flag != 0);
}

/**
 * @brief 获取骨骼的瞄准优先级
 * * 数字越小，优先级越高 (0 = 最高优先级)
 * * 用于实现 "向上修正，向下锁定" 逻辑。
 * @param bone_id 骨骼ID
 * @return int 优先级 (0=最高, 999=最低)
 */
int get_bone_priority(BoneID bone_id) {
    switch (bone_id) {
        // 核心躯干 (高优先级)
    case BoneID::Head:
        return 0; // 最高优先级
    case BoneID::Neck:
        return 1;
    case BoneID::Spine2: // 上胸部
        return 2;
    case BoneID::Spine1: // 胸部
        return 2;
    case BoneID::Spine:  // 下胸部/腹部
        return 3;
    case BoneID::Hips:   // 臀部/裆部
        return 3;

        // 四肢 (中优先级)
    case BoneID::LeftShoulder:
    case BoneID::RightShoulder:
        return 4;
    case BoneID::LeftUpLeg:
    case BoneID::RightUpLeg:
        return 4;

        // 四肢末端 (低优先级)
    case BoneID::LeftArm:
    case BoneID::RightArm:
        return 5;
    case BoneID::LeftLeg:
    case BoneID::RightLeg:
        return 5;
    case BoneID::LeftForeArm:
    case BoneID::RightForeArm:
        return 5;

        // 手脚 (最低优先级)
    case BoneID::LeftHand:
    case BoneID::RightHand:
    case BoneID::LeftFoot:
    case BoneID::RightFoot:
    case BoneID::LeftToeBase:
    case BoneID::RightToeBase:
        return 6;

        // 其他所有骨骼 (如 IK, 武器, 道具点)
    default:
        return 999;
    }
}
WeaponType get_weapon_type(long long weapon_id) {

    // 步枪
    if (weapon_id >= 18010000001 && weapon_id <= 18010000050) {
        return WeaponType::Rifle;
    }
    // 冲锋枪
    if (weapon_id >= 18020000001 && weapon_id <= 18020000011) {
        return WeaponType::SMG;
    }
    // 霰弹枪
    if (weapon_id >= 18030000001 && weapon_id <= 18030000005) {
        return WeaponType::Shotgun;
    }
    // 轻机枪
    if (weapon_id >= 18040000001 && weapon_id <= 18040000004) {
        return WeaponType::LMG;
    }
    // 射手步枪
    if (weapon_id >= 18050000001 && weapon_id <= 18050000031) {
        return WeaponType::DMR;
    }
    // 狙击步枪
    if ((weapon_id >= 18060000001 && weapon_id <= 18060000011) || (weapon_id >= 18150000001 && weapon_id <= 18150000002)) {
        return WeaponType::Sniper;
    }
    // 手枪
    if (weapon_id >= 18070000002 && weapon_id <= 18070000033) {
        return WeaponType::Pistol;
    }

    // 其他所有 ID (发射器, 道具, 近战等) 归为未知
    return WeaponType::Unknown;
}

void GameLogic::aimbotLoop() {
    auto lua_config_path = get_temp_lua_path(); // 获取临时lua文件路径
    float remain_dx = 0.0f; // 上一帧剩余的水平移动量
    float remain_dy = 0.0f; // 上一帧剩余的垂直移动量
    long long loop_counter = 0; // 循环计数器，用于日志节流
    bool is_on_target = false; // 用于自动扳机

    constexpr float BASE_RECOIL_FOV = 90.0f;
    const float AUTO_TRIGGER_RADIUS_SQ = 35.0f * 35.0f; // 自动扳机的像素半径

    // 模拟压枪所需的状态
    auto firing_start_time = std::chrono::steady_clock::now();
    WeaponType current_weapon_type = WeaponType::Unknown; // 缓存当前武器类型
    RecoilSetting current_recoil_setting; // 缓存当前武器的压枪设置

    // 自瞄参数
    float random_strength = 0.8f;
    float head_aim_offset_pixels = 3.5f;
    float min_dynamic_smooth = 1.0f;
    float max_dynamic_smooth_multiplier = 1.8f;
    float dynamic_smooth_distance_factor = 0.4f;

    // 瞄准状态，用于在帧之间保持优先级锁定
    uint64_t locked_target_ptr = 0;
    BoneID locked_target_bone = BoneID::root;
    int locked_target_priority = 999;

    std::cout << "[Aimbot] Server-side loop started (Write Mode with Recoil)." << std::endl;

    // 初始化缓存的压枪设置
    {
        std::shared_lock lock(g_cfg.recoil_settings_mutex);
        if (g_cfg.weapon_recoil_settings.size() == static_cast<size_t>(WeaponType::MaxTypes)) {
            current_recoil_setting = g_cfg.weapon_recoil_settings[static_cast<int>(WeaponType::Unknown)];
        }
        else {
            current_recoil_setting = { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };
            std::cerr << "[Aimbot] Error: g_cfg.weapon_recoil_settings not properly initialized or wrong size!" << std::endl;
        }
    }

    while (running.load(std::memory_order_relaxed)) { // 主循环
        loop_counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(8));

        is_on_target = false; // 默认每帧重置为 false

        Vector3 my_loc;
        int my_team = -1;
        bool is_in_match = false;
        uint64_t my_pawn = 0;
        float current_fov = BASE_RECOIL_FOV;

        {
            std::shared_lock lock(sharedData.mutex);
            my_loc = sharedData.my_location;
            my_team = sharedData.my_team_id;
            is_in_match = sharedData.is_in_match;
            my_pawn = sharedData.my_pawn_ptr;
            current_fov = sharedData.camera_fov;
        }

        bool aimbot_on = g_cfg.aimbot_enabled.load(std::memory_order_relaxed);
        bool recoil_on = g_cfg.simulated_recoil_enabled.load(std::memory_order_relaxed);

        if (!is_in_match || my_pawn == 0 || !valid_location(my_loc)) {
            if (remain_dx != 0.0f || remain_dy != 0.0f) {
                // 确保写入所有标志
                atomic_write_text(lua_config_path, "step_dx = 0\nstep_dy = 0\nis_recoiling = false\nis_on_target = false\nis_single_fire = false\n");
            }
            remain_dx = 0.0f; remain_dy = 0.0f;
            locked_target_ptr = 0;
            locked_target_bone = BoneID::root;
            locked_target_priority = 999;
            firing_start_time = std::chrono::steady_clock::now();
            current_weapon_type = WeaponType::Unknown;
            continue;
        }

        float recoil_x = 0.0f;
        float recoil_y = 0.0f;
        bool is_firing_state = read_is_firing_flag();

        // --- 武器检测 和 连点/压枪 模式设置 ---
        bool is_single_fire_mode = false;
        {
            long long current_weapon_id = 0;
            WeaponType new_weapon_type = WeaponType::Unknown;

            if (my_pawn) {
                uint64_t weapon_ptr = read<uint64_t>(my_pawn + Offsets::CacheCurWeapon);
                if (weapon_ptr) {
                    current_weapon_id = read<long long>(weapon_ptr + Offsets::WeaponID);
                }
            }
            new_weapon_type = get_weapon_type(current_weapon_id);

            if (new_weapon_type != current_weapon_type) {
                current_weapon_type = new_weapon_type;
            }

            // 根据当前武器类型设置连点标志
            switch (current_weapon_type) {
            case WeaponType::Shotgun:
            case WeaponType::DMR:
            case WeaponType::Sniper:
            case WeaponType::Pistol:
                is_single_fire_mode = true;
                break;
                // 默认 (Rifle, SMG, LMG, Unknown) 都是全自动压枪
            default:
                is_single_fire_mode = false;
                break;
            }
        }
        // --- 武器检测结束 ---


        if (recoil_on && !is_single_fire_mode) { // 只有全自动武器才计算压枪
            {
                std::shared_lock lock(g_cfg.recoil_settings_mutex);
                int index = static_cast<int>(current_weapon_type);
                if (index >= 0 && index < g_cfg.weapon_recoil_settings.size()) {
                    current_recoil_setting = g_cfg.weapon_recoil_settings[index];
                }
                else {
                    current_recoil_setting = g_cfg.weapon_recoil_settings[static_cast<int>(WeaponType::Unknown)];
                }
            }

            if (is_firing_state) {
                auto now = std::chrono::steady_clock::now();
                auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - firing_start_time).count();
                float delay_ms = current_recoil_setting.recoil_delay_ms;
                float multiplier = current_recoil_setting.recoil_multiplier;

                if (delay_ms > 0 && duration_ms >= delay_ms) {
                    recoil_x = current_recoil_setting.recoil_x_2 * multiplier;
                    recoil_y = current_recoil_setting.recoil_y_2 * multiplier;
                }
                else {
                    recoil_x = current_recoil_setting.recoil_x * multiplier;
                    recoil_y = current_recoil_setting.recoil_y * multiplier;
                }
                if (current_fov > 0.0f && std::abs(current_fov - BASE_RECOIL_FOV) > 0.01f) {
                    float fov_scale = BASE_RECOIL_FOV / current_fov;
                    recoil_x *= fov_scale;
                    recoil_y *= fov_scale;
                }
            }
            else {
                firing_start_time = std::chrono::steady_clock::now();
            }
        }
        else {
            firing_start_time = std::chrono::steady_clock::now();
        }

        // 自瞄计算
        float aim_move_x = 0.0f;
        float aim_move_y = 0.0f;
        bool target_found_this_frame = false;

        if (aimbot_on) {
            float screen_center_x = static_cast<float>(g_game_width) / 2.0f;
            float screen_center_y = static_cast<float>(g_game_height) / 2.0f;
            float fov_pixels = g_cfg.aimbot_fov.load(std::memory_order_relaxed);
            float fov_pixels_sq = fov_pixels * fov_pixels;
            float base_smooth = std::max(1.0f, g_cfg.aimbot_smooth.load(std::memory_order_relaxed));
            float max_dist = g_cfg.aimbot_max_distance.load(std::memory_order_relaxed);
            bool visible_only_cfg = g_cfg.aimbot_visible_only.load(std::memory_order_relaxed);
            bool ignore_ai = g_cfg.aimbot_ignore_ai.load(std::memory_order_relaxed);
            bool use_priority_lock_mode = !g_cfg.aimbot_aim_nearest_on_visible.load(std::memory_order_relaxed);
            bool map_is_loaded = mapLoaded.load(std::memory_order_relaxed);

            std::vector<AimbotBoneSetting> bones_to_check_cfg;
            {
                std::shared_lock lock(g_cfg.aimbot_bones_mutex);
                bones_to_check_cfg = g_cfg.aimbot_target_bones;
            }
            if (bones_to_check_cfg.empty() || !use_priority_lock_mode) {
                bones_to_check_cfg.clear();
                for (const auto& bone_id : g_valid_body_bones) {
                    bones_to_check_cfg.push_back({ bone_id, true });
                }
            }

            uint64_t frame_best_actor_ptr = 0;
            BoneID frame_best_bone_id = BoneID::root;
            int frame_best_priority = 999;
            float frame_best_dist_sq = fov_pixels_sq;
            DirectX::XMFLOAT2 frame_best_screen_pos = { 0.0f, 0.0f };

            std::vector<CachedPlayer> current_players;
            {
                std::shared_lock lock(playerCacheMutex);
                current_players.reserve(playerCache.size());
                for (const auto& pair : playerCache) {
                    current_players.push_back(pair.second);
                }
            }

            if (use_priority_lock_mode && locked_target_ptr != 0) {
                bool locked_target_still_valid = false;
                for (const auto& p : current_players) {
                    if (p.actor_ptr != locked_target_ptr) continue;

                    if (!p.client_knows_about_it || (p.team_id == my_team && p.team_id != -1) || p.health <= 0.1f) {
                        break;
                    }
                    float distance_m = p.location.Distance(my_loc) / 100.0f;
                    if (distance_m > max_dist) {
                        break;
                    }

                    locked_target_still_valid = true;

                    for (const auto& bone_setting : bones_to_check_cfg) {
                        if (!bone_setting.enabled) continue;

                        int bone_index = static_cast<int>(bone_setting.bone_id);
                        if (bone_index < 0 || bone_index >= NUM_BONES || !is_valid_bone_position(p.skeleton[bone_index])) continue;

                        Vector3 bone_world_pos = { p.skeleton[bone_index].x, p.skeleton[bone_index].y, p.skeleton[bone_index].z };
                        bool is_bone_visible = true;
                        if (visible_only_cfg && map_is_loaded) {
                            is_bone_visible = RTModel::IsVisible(my_loc, bone_world_pos);
                        }
                        if (!is_bone_visible) continue;

                        DirectX::XMFLOAT2 bone_screen_pos;
                        if (WorldToScreen(bone_world_pos, bone_screen_pos)) {
                            float dist_x = bone_screen_pos.x - screen_center_x;
                            float dist_y = bone_screen_pos.y - screen_center_y;
                            float dist_sq = dist_x * dist_x + dist_y * dist_y;

                            if (dist_sq < fov_pixels_sq) {
                                int new_priority = get_bone_priority(bone_setting.bone_id);

                                if (new_priority <= locked_target_priority) {
                                    if (new_priority < frame_best_priority || (new_priority == frame_best_priority && dist_sq < frame_best_dist_sq)) {
                                        frame_best_priority = new_priority;
                                        frame_best_dist_sq = dist_sq;
                                        frame_best_bone_id = bone_setting.bone_id;
                                        frame_best_actor_ptr = p.actor_ptr;
                                        frame_best_screen_pos = bone_screen_pos;
                                        target_found_this_frame = true;
                                    }
                                }
                            }
                        }
                    }
                    break;
                }

                if (!locked_target_still_valid || !target_found_this_frame) {
                    locked_target_ptr = 0;
                    locked_target_bone = BoneID::root;
                    locked_target_priority = 999;
                    target_found_this_frame = false;
                }
            }

            for (const auto& p : current_players) {
                if (use_priority_lock_mode && locked_target_ptr != 0 && p.actor_ptr == locked_target_ptr) {
                    continue;
                }

                if (!p.client_knows_about_it || p.actor_ptr == my_pawn || (p.team_id == my_team && p.team_id != -1) || p.health <= 0.1f || (ignore_ai && !p.is_ai)) continue;
                float distance_m = p.location.Distance(my_loc) / 100.0f;
                if (distance_m > max_dist) continue;

                int player_best_priority = 999;
                float player_best_dist_sq = fov_pixels_sq;
                BoneID player_best_bone_id = BoneID::root;
                DirectX::XMFLOAT2 player_best_screen_pos = { 0.0f, 0.0f };
                bool player_target_found = false;

                for (const auto& bone_setting : bones_to_check_cfg) {
                    if (!bone_setting.enabled) continue;

                    int bone_index = static_cast<int>(bone_setting.bone_id);
                    if (bone_index < 0 || bone_index >= NUM_BONES || !is_valid_bone_position(p.skeleton[bone_index])) continue;

                    Vector3 bone_world_pos = { p.skeleton[bone_index].x, p.skeleton[bone_index].y, p.skeleton[bone_index].z };
                    bool is_bone_visible = true;
                    if (visible_only_cfg && map_is_loaded) {
                        is_bone_visible = RTModel::IsVisible(my_loc, bone_world_pos);
                    }
                    if (!is_bone_visible) continue;

                    DirectX::XMFLOAT2 bone_screen_pos;
                    if (WorldToScreen(bone_world_pos, bone_screen_pos)) {
                        float dist_x = bone_screen_pos.x - screen_center_x;
                        float dist_y = bone_screen_pos.y - screen_center_y;
                        float dist_sq = dist_x * dist_x + dist_y * dist_y;

                        if (dist_sq < fov_pixels_sq) {
                            if (use_priority_lock_mode) {
                                int new_priority = get_bone_priority(bone_setting.bone_id);
                                if (new_priority < player_best_priority || (new_priority == player_best_priority && dist_sq < player_best_dist_sq)) {
                                    player_best_priority = new_priority;
                                    player_best_dist_sq = dist_sq;
                                    player_best_bone_id = bone_setting.bone_id;
                                    player_best_screen_pos = bone_screen_pos;
                                    player_target_found = true;
                                }
                            }
                            else {
                                if (dist_sq < player_best_dist_sq) {
                                    player_best_dist_sq = dist_sq;
                                    player_best_bone_id = bone_setting.bone_id;
                                    player_best_screen_pos = bone_screen_pos;
                                    player_target_found = true;
                                    player_best_priority = get_bone_priority(player_best_bone_id);
                                }
                            }
                        }
                    }
                }

                if (player_target_found) {
                    if (use_priority_lock_mode) {
                        if (player_best_priority < frame_best_priority || (player_best_priority == frame_best_priority && player_best_dist_sq < frame_best_dist_sq)) {
                            frame_best_priority = player_best_priority;
                            frame_best_dist_sq = player_best_dist_sq;
                            frame_best_bone_id = player_best_bone_id;
                            frame_best_actor_ptr = p.actor_ptr;
                            frame_best_screen_pos = player_best_screen_pos;
                            target_found_this_frame = true;
                        }
                    }
                    else {
                        if (player_best_dist_sq < frame_best_dist_sq) {
                            frame_best_dist_sq = player_best_dist_sq;
                            frame_best_bone_id = player_best_bone_id;
                            frame_best_actor_ptr = p.actor_ptr;
                            frame_best_screen_pos = player_best_screen_pos;
                            frame_best_priority = player_best_priority;
                            target_found_this_frame = true;
                        }
                    }
                }
            }

            if (target_found_this_frame) {
                if (use_priority_lock_mode && (locked_target_ptr != frame_best_actor_ptr || locked_target_bone != frame_best_bone_id)) {
                    locked_target_ptr = frame_best_actor_ptr;
                    locked_target_bone = frame_best_bone_id;
                    locked_target_priority = frame_best_priority;
                }

                if (frame_best_bone_id == BoneID::Head) {
                    frame_best_screen_pos.y += head_aim_offset_pixels;
                }

                float move_x_ideal = frame_best_screen_pos.x - screen_center_x;
                float move_y_ideal = frame_best_screen_pos.y - screen_center_y;

                float ideal_dist_sq = move_x_ideal * move_x_ideal + move_y_ideal * move_y_ideal;
                if (ideal_dist_sq <= AUTO_TRIGGER_RADIUS_SQ) {
                    is_on_target = true;
                }

                float dist_pixels = std::sqrt(ideal_dist_sq);
                float dynamic_smooth = base_smooth;
                if (dist_pixels > 1.0f) {
                    float distance_factor = std::clamp(dist_pixels / (fov_pixels * dynamic_smooth_distance_factor), 0.5f, max_dynamic_smooth_multiplier);
                    dynamic_smooth = base_smooth / distance_factor;
                    dynamic_smooth = std::max(min_dynamic_smooth, dynamic_smooth);
                }

                aim_move_x = move_x_ideal / dynamic_smooth;
                aim_move_y = move_y_ideal / dynamic_smooth;

                aim_move_x += dist_noise(rng) * random_strength;
                aim_move_y += dist_noise(rng) * random_strength;
            }
            else {
                locked_target_ptr = 0;
                locked_target_bone = BoneID::root;
                locked_target_priority = 999;
            }
        }
        else {
            locked_target_ptr = 0;
            locked_target_bone = BoneID::root;
            locked_target_priority = 999;
        }

        // 合并移动量
        float total_dx = aim_move_x + recoil_x + remain_dx;
        float total_dy = aim_move_y + recoil_y + remain_dy;

        int step_dx = static_cast<int>(std::round(total_dx));
        int step_dy = static_cast<int>(std::round(total_dy));

        remain_dx = total_dx - step_dx;
        remain_dy = total_dy - step_dy;

        // 写入 Lua 文件
        std::stringstream ss;
        ss << "step_dx = " << step_dx << "\n";
        ss << "step_dy = " << step_dy << "\n";
        // 只有在非单发模式（全自动）下，才发送压枪标志
        ss << "is_recoiling = " << (recoil_on && is_firing_state && !is_single_fire_mode ? "true" : "false") << "\n";
        ss << "is_on_target = " << (is_on_target ? "true" : "false") << "\n";
        ss << "is_single_fire = " << (is_single_fire_mode ? "true" : "false") << "\n"; // [新] 写入武器模式
        bool write_ok = atomic_write_text(lua_config_path, ss.str());

    } // 结束 while(running)

    // 确保写入所有标志
    atomic_write_text(lua_config_path, "step_dx = 0\nstep_dy = 0\nis_recoiling = false\nis_on_target = false\nis_single_fire = false\n");
    std::cout << "[Aimbot] Server-side loop stopped." << std::endl;
}

// TODO: 动静态实体的 搜索、清理 速度优化，目前感知较强的是 ITEM 静态实体在被队友捡起后仍然会持续被广播一段时间
void GameLogic::actorDiscoveryLoop() {
    while (running) {
        if (networkServer.newClientJustConnected.load()) {
            std::cout << "[GameLogic] New client connected. Performing full state sync..." << std::endl;
            {
                std::shared_lock lk(playerCacheMutex);
                for (const auto& [actor_id, p] : playerCache) {
                    if (p.client_knows_about_it) {
                        PlayerCreatePacket pkt;
                        pkt.header.type = PacketType::ENTITY_CREATE;
                        pkt.actor_id = p.actor_ptr;
                        strncpy_s(pkt.name, p.name.c_str(), _TRUNCATE);
                        strncpy_s(pkt.ai_check, (p.is_ai ? "Is AI" : "Is Player"), _TRUNCATE);
                        strncpy_s(pkt.ai_type, p.ai_type.c_str(), _TRUNCATE);
                        strncpy_s(pkt.operator_name, p.operator_name.c_str(), _TRUNCATE);
                        pkt.team_id = p.team_id;
                        pkt.helmet_level = p.helmet_level;
                        pkt.armor_level = p.armor_level;
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                }
            }
            {
                std::shared_lock lk(staticCacheMutex);
                for (const auto& [actor_id, s] : staticCache) {
                    if (!valid_location(s.location)) continue;

                    if (s.cls == StaticClass::Item) {
                        ItemCreatePacket pkt;
                        pkt.header.type = PacketType::ITEM_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        std::string name = get_item_display_name(s.item_component_ptr);
                        if (name.empty()) continue;
                        strncpy_s(pkt.name, name.c_str(), _TRUNCATE);
                        pkt.price = read<int>(s.item_component_ptr + Offsets::ItemPrice);
                        pkt.quality = read<int>(s.item_component_ptr + Offsets::Quality);
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                    else if (s.cls == StaticClass::HackerPC) {
                        HackerPcCreatePacket pkt;
                        pkt.header.type = PacketType::HACKER_PC_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        if (s.class_name.find("CodedLock") != std::string::npos) {
                            strncpy_s(pkt.name, u8"密码门", _TRUNCATE);
                            pkt.password = read<int32_t>(s.actor_ptr + Offsets::PwdSum);
                        }
                        else {
                            strncpy_s(pkt.name, u8"骇客电脑", _TRUNCATE);
                            pkt.password = read<int32_t>(s.actor_ptr + Offsets::Password);
                        }
                        pkt.is_looted = s.is_opened;
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        if (pkt.password > 100 && pkt.password < 10000) {
                            networkServer.send_packet(&pkt, sizeof(pkt));
                        }
                    }
                    else if (s.cls == StaticClass::Container || s.cls == StaticClass::Safe || s.cls == StaticClass::PlayerLootBox) {
                        BoxCreatePacket pkt;
                        pkt.header.type = PacketType::BOX_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        std::string pretty;
                        if (s.cls == StaticClass::PlayerLootBox) {
                            pretty = (s.box_owner == LootBoxOwnerType::Player) ? u8"玩家盒子" : u8"人机盒子";
                        }
                        else {
                            pretty = translate_container_name(s.class_name);
                        }
                        if (pretty.empty()) continue;
                        strncpy_s(pkt.name, pretty.c_str(), _TRUNCATE);
                        pkt.is_looted = s.is_opened;
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                    else if (s.cls == StaticClass::ExtractionPoint) {
                        ExtractionCreatePacket pkt;
                        pkt.header.type = PacketType::EXTRACTION_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        strncpy_s(pkt.name, u8"撤离点", _TRUNCATE);
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                }
            }
            std::cout << "[GameLogic] Full state sync completed." << std::endl;
            networkServer.newClientJustConnected = false;
        }

        bool in_match_now = false;
        ULONGLONG my_pawn_ptr = 0;
        {
            std::shared_lock lk(sharedData.mutex);
            in_match_now = sharedData.is_in_match;
            my_pawn_ptr = sharedData.my_pawn_ptr;
        }

        if (!in_match_now) {
            bool cleared = false;
            auto clear_cache = [&](auto& cache, auto& mutex) {
                std::unique_lock lk(mutex);
                if (!cache.empty()) {
                    for (auto const& [key, val] : cache) {
                        EntityDestroyPacket pkt{ {PacketType::ENTITY_DESTROY}, key };
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                    cache.clear();
                    cleared = true;
                }
                };
            clear_cache(playerCache, playerCacheMutex);
            clear_cache(staticCache, staticCacheMutex);
            {
                std::unique_lock lk(pendingActorsMutex);
                if (!pendingActors.empty()) pendingActors.clear();
            }
            if (cleared) {
                std::cout << "[GameLogic] Left match, caches cleared." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        std::vector<ULONGLONG> actor_pointers;
        ULONGLONG uworld_ptr = read<ULONGLONG>(baseAddress + Offsets::Uworld);
        if (uworld_ptr) {
            ULONGLONG levels_ptr = read<ULONGLONG>(uworld_ptr + Offsets::UlevelS);
            int levels_count = read<int>(uworld_ptr + Offsets::UlevelSCount);
            if (levels_ptr && levels_count > 0 && levels_count < 1000) {
                for (int i = 0; i < levels_count; ++i) {
                    ULONGLONG ulevel = read<ULONGLONG>(levels_ptr + static_cast<unsigned long long>(i) * 8);
                    if (!ulevel) continue;
                    ULONGLONG actors_ptr = read<ULONGLONG>(ulevel + Offsets::Actor);
                    int actors_count = read<int>(ulevel + Offsets::Count);
                    if (actors_ptr && actors_count > 0 && actors_count < 8192) {
                        std::vector<ULONGLONG> tmp(actors_count);
                        if (read_bytes(actors_ptr, tmp.data(), static_cast<size_t>(actors_count) * sizeof(ULONGLONG))) {
                            actor_pointers.insert(actor_pointers.end(), tmp.begin(), tmp.end());
                        }
                    }
                }
            }
        }

        if (actor_pointers.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::unordered_set seen_this_frame(actor_pointers.begin(), actor_pointers.end());

        {
            std::unique_lock lk(pendingActorsMutex);
            for (auto it = pendingActors.begin(); it != pendingActors.end();) {
                ULONGLONG actor_ptr = it->first;
                it->second++;

                if (seen_this_frame.find(actor_ptr) == seen_this_frame.end() || it->second > 100) {
                    if (g_cfg.debug.load()) std::cout << "[Debug] Removing Actor 0x" << std::hex << actor_ptr << " from pending list (timeout/disappeared)." << std::dec << std::endl;
                    it = pendingActors.erase(it);
                    continue;
                }

                CachedStatic s{};
                s.actor_ptr = actor_ptr;
                s.object_id = read<int>(actor_ptr + Offsets::ObjectID);
                //s.root_component_ptr = read<ULONGLONG>(actor_ptr + Offsets::RootComponent) >> 4;
                s.root_component_ptr = decrypt_shift(actor_ptr + Offsets::RootComponent);

                if (s.object_id <= 0 || !s.root_component_ptr) {
                    ++it; continue;
                }

                s.location = read_enc_location(s.root_component_ptr);
                if (!valid_location(s.location)) {
                    if (g_cfg.debug.load()) std::cout << "[Debug] Pending Actor 0x" << std::hex << actor_ptr << " location still invalid, retrying." << std::dec << std::endl;
                    ++it; continue;
                }

                s.class_name = get_name_fast(s.object_id);
                if (!is_dead_body_class(s.class_name)) {
                    it = pendingActors.erase(it);
                    continue;
                }

                ULONGLONG owner_ps_ptr = read<ULONGLONG>(actor_ptr + Offsets::OwnerPlayerState);
                if (!owner_ps_ptr) {
                    if (g_cfg.debug.load()) std::cout << "[Debug] Pending Actor 0x" << std::hex << actor_ptr << " owner info missing, retrying." << std::dec << std::endl;
                    ++it; continue;
                }

                if (g_cfg.debug.load()) std::cout << "[Debug] Processing Pending Actor 0x" << std::hex << actor_ptr << " as Dead Body Box." << std::dec << std::endl;
                s.cls = StaticClass::PlayerLootBox;
                s.box_owner = (read<BYTE>(owner_ps_ptr + Offsets::bIsABot) != 0) ? LootBoxOwnerType::Player : LootBoxOwnerType::AI;

                BoxCreatePacket pkt;
                pkt.header.type = PacketType::BOX_CREATE;
                pkt.actor_id = s.actor_ptr;
                std::string pretty = (s.box_owner == LootBoxOwnerType::Player) ? u8"玩家盒子" : u8"人机盒子";
                strncpy_s(pkt.name, pretty.c_str(), _TRUNCATE);
                auto open_state = try_read_open_state_once(s.actor_ptr, s.cls);
                pkt.is_looted = open_state.has_value() ? open_state.value() : false;
                s.is_opened = pkt.is_looted;
                s.last_sent_opened_state = s.is_opened;
                pkt.world_location = { s.location.x, s.location.y, s.location.z };
                networkServer.send_packet(&pkt, sizeof(pkt));

                {
                    std::unique_lock static_lk(staticCacheMutex);
                    staticCache[actor_ptr] = s;
                }
                it = pendingActors.erase(it);

            }
        }

        for (const auto actor_ptr : actor_pointers) {
            if (!actor_ptr || actor_ptr == my_pawn_ptr) continue;

            bool in_player_cache, in_static_cache, in_pending_cache;
            { std::shared_lock lk(playerCacheMutex); in_player_cache = playerCache.count(actor_ptr); }
            { std::shared_lock lk(staticCacheMutex); in_static_cache = staticCache.count(actor_ptr); }
            { std::shared_lock lk(pendingActorsMutex); in_pending_cache = pendingActors.count(actor_ptr); }
            if (in_player_cache || in_static_cache || in_pending_cache) continue;

            int object_id = read<int>(actor_ptr + Offsets::ObjectID);
            if (object_id <= 0) continue;
            std::string class_name = get_name_fast(object_id);
            if (class_name.empty()) continue;

            //        if (class_name.find("Character") != std::string::npos)
            //        {
            //            if (
            //                class_name == "CharacterEquip"
            //                || class_name == "CharacterFashionComponent"
            //                || class_name == "DFMCharacterAmbientAudioFSM"
            //                || class_name == "BP_PropertyReplicationCharacterHealth_C_PropertyReplicationActor_UniqueName"
            //                || class_name == "AICharacterSignificanceComponent"
            //                || class_name == "SOLCharacterEquipComponent"
            //                || class_name == "DFMCharacterBattleClass"
            //                || class_name == "DFMCharacterFSM_Main_C"
            //                || class_name == "BP_DFMCharacter_SafeHouse_C"
            //                || class_name == "BP_CharacterLODSystem_C"
            //                || class_name == "BP_HallCharacter_C"
            //                || class_name == "BP_DFMCharacter_CS1P"
            //                || class_name == "BP_HallCharacter"
            //                )
            //            {
            //                continue;
            //            }
                        //std::cout << class_name << std::endl;
            //        }
            if (class_name.find("BP_DFMCharacter") != std::string::npos || class_name.find("BP_DFMAICharacter") != std::string::npos || class_name.find("BP_RangeTargetCharacter_C") != std::string::npos) {
                CachedPlayer p{};
                p.actor_ptr = actor_ptr;
                p.class_name = class_name;
                std::string lower_class_name = class_name;
                std::transform(lower_class_name.begin(), lower_class_name.end(), lower_class_name.begin(), ::tolower);
                p.is_ai = lower_class_name.find("aicharacter") != std::string::npos || lower_class_name.find("_ai_") != std::string::npos || class_name.find("BP_RangeTargetCharacter_C") != std::string::npos;

                if (p.is_ai) {
                    p.team_id = 99;
                    p.name = "AI";
                    p.ai_type = get_ai_type(p.actor_ptr);
                }
                else {
                    p.player_state_ptr = read<ULONGLONG>(actor_ptr + Offsets::PlayerState);
                    p.team_id = -1;
                }
                {
                    std::unique_lock lk(playerCacheMutex);
                    playerCache[actor_ptr] = p;
                }
            }

            // TODO: 撤离点识别条件过于宽松（会误报），需优化
            else if (class_name.find("ExitTrigger") != std::string::npos && class_name.find("BP_PlayerExit") != std::string::npos) {
                CachedStatic s{};
                s.actor_ptr = actor_ptr;
                s.cls = StaticClass::ExtractionPoint;
                //s.root_component_ptr = read<ULONGLONG>(actor_ptr + Offsets::RootComponent) >> 4;
                s.root_component_ptr = decrypt_shift(actor_ptr + Offsets::RootComponent);
                if (!s.root_component_ptr) continue;

                s.location = read_enc_location(s.root_component_ptr);
                if (!valid_location(s.location)) {
                    if (g_cfg.debug.load()) std::cout << "[Debug] Extraction Actor 0x" << std::hex << actor_ptr << " location invalid, skipping." << std::dec << std::endl;
                    continue;
                }

                ExtractionCreatePacket pkt;
                pkt.header.type = PacketType::EXTRACTION_CREATE;
                pkt.actor_id = s.actor_ptr;
                pkt.world_location = { s.location.x, s.location.y, s.location.z };
                strncpy_s(pkt.name, u8"撤离点", _TRUNCATE);
                networkServer.send_packet(&pkt, sizeof(pkt));
                {
                    std::unique_lock lk(staticCacheMutex);
                    staticCache[actor_ptr] = s;
                }
            }
            else {
                CachedStatic s{};
                s.actor_ptr = actor_ptr;
                s.object_id = object_id;
                s.class_name = class_name;
                //s.root_component_ptr = read<ULONGLONG>(actor_ptr + Offsets::RootComponent) >> 4;
                s.root_component_ptr = decrypt_shift(actor_ptr + Offsets::RootComponent);
                if (!s.root_component_ptr) continue;

                s.location = read_enc_location(s.root_component_ptr);
                if (!valid_location(s.location)) {
                    if (is_dead_body_class(class_name)) {
                        if (g_cfg.debug.load()) std::cout << "[Debug] New Dead Body Actor 0x" << std::hex << actor_ptr << " location invalid, adding to pending." << std::dec << std::endl;
                        std::unique_lock lk(pendingActorsMutex);
                        pendingActors[actor_ptr] = 0;
                    }
                    else if (g_cfg.debug.load()) {
                        std::cout << "[Debug] Static Actor 0x" << std::hex << actor_ptr << " (" << class_name << ") location invalid, skipping." << std::dec << std::endl;
                    }
                    continue;
                }

                bool should_process_static = false;
                bool is_hacker_pc = class_name.find("BP_Interact_Computer_C") != std::string::npos;
                bool is_password_door = class_name.find("BP_Interactor_CodedLock") != std::string::npos;
                StaticClass byType = StaticClass::Unknown;

                if (is_hacker_pc || is_password_door) {
                    s.cls = StaticClass::HackerPC;
                    should_process_static = true;
                }
                else if (is_dead_body_class(class_name)) {
                    ULONGLONG owner_ps_ptr = read<ULONGLONG>(actor_ptr + Offsets::OwnerPlayerState);
                    if (!owner_ps_ptr) {
                        if (g_cfg.debug.load()) std::cout << "[Debug] New Dead Body Actor 0x" << std::hex << actor_ptr << " owner info missing, adding to pending." << std::dec << std::endl;
                        std::unique_lock lk(pendingActorsMutex);
                        pendingActors[actor_ptr] = 0;
                    }
                    else {
                        s.cls = StaticClass::PlayerLootBox;
                        s.box_owner = (read<BYTE>(owner_ps_ptr + Offsets::bIsABot) != 0) ? LootBoxOwnerType::Player : LootBoxOwnerType::AI;
                        should_process_static = true;
                    }
                }
                else {
                    BYTE mt = read<BYTE>(actor_ptr + Offsets::MarkingItemType);
                    byType = classify_by_marking_type(mt);

                    if (byType == StaticClass::Item || class_name.find("InventoryPickup") != std::string::npos) {
                        s.cls = StaticClass::Item;
                        s.item_component_ptr = read<ULONGLONG>(actor_ptr + Offsets::ItemComponent);
                        if (s.item_component_ptr) should_process_static = true;
                    }
                    else if (match_container_keywords(class_name) || byType == StaticClass::Container || byType == StaticClass::Safe) {
                        if (!container_strict_blacklist(class_name)) {
                            s.cls = (byType != StaticClass::Unknown && byType != StaticClass::HackerPC) ? byType : StaticClass::Container;
                            should_process_static = true;
                        }
                    }
                }

                if (should_process_static) {
                    if (s.cls == StaticClass::Item) {
                        ItemCreatePacket pkt;
                        pkt.header.type = PacketType::ITEM_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        std::string name = get_item_display_name(s.item_component_ptr);
                        if (name.empty()) continue;
                        strncpy_s(pkt.name, name.c_str(), _TRUNCATE);
                        pkt.price = read<int>(s.item_component_ptr + Offsets::ItemPrice);
                        pkt.quality = read<int>(s.item_component_ptr + Offsets::Quality);
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }
                    else if (s.cls == StaticClass::HackerPC) {
                        HackerPcCreatePacket pkt;
                        pkt.header.type = PacketType::HACKER_PC_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        if (is_password_door) {
                            strncpy_s(pkt.name, u8"密码门", _TRUNCATE);
                            // TODO: 已知脑机接口的密码可以读取但是密码错误，怀疑偏移有问题，待确认
                            pkt.password = read<int32_t>(s.actor_ptr + Offsets::PwdSum);
                        }
                        else {
                            strncpy_s(pkt.name, u8"骇客电脑", _TRUNCATE);
                            pkt.password = read<int32_t>(s.actor_ptr + Offsets::Password);
                        }
                        if (pkt.password > 100 && pkt.password < 10000) {
                            auto open_state = try_read_open_state_once(s.actor_ptr, s.cls);
                            pkt.is_looted = open_state.has_value() ? open_state.value() : false;
                            s.is_opened = pkt.is_looted;
                            s.last_sent_opened_state = s.is_opened;
                            networkServer.send_packet(&pkt, sizeof(pkt));
                        }
                        else {
                            if (g_cfg.debug.load()) std::cout << "[Debug] Actor 0x" << std::hex << actor_ptr << " (" << (is_password_door ? "Password Door" : "Hacker PC") << ") has invalid password: " << std::dec << pkt.password << ", skipping." << std::endl;
                            continue;
                        }
                    }
                    else {
                        BoxCreatePacket pkt;
                        pkt.header.type = PacketType::BOX_CREATE;
                        pkt.actor_id = s.actor_ptr;
                        std::string pretty;
                        if (s.cls == StaticClass::PlayerLootBox) {
                            pretty = (s.box_owner == LootBoxOwnerType::Player) ? u8"玩家盒子" : u8"人机盒子";
                        }
                        else {
                            pretty = translate_container_name(s.class_name);
                        }
                        if (pretty.empty()) continue;
                        strncpy_s(pkt.name, pretty.c_str(), _TRUNCATE);
                        auto open_state = try_read_open_state_once(s.actor_ptr, s.cls);
                        pkt.is_looted = open_state.has_value() ? open_state.value() : false;
                        s.is_opened = pkt.is_looted;
                        s.last_sent_opened_state = s.is_opened;
                        pkt.world_location = { s.location.x, s.location.y, s.location.z };
                        networkServer.send_packet(&pkt, sizeof(pkt));
                    }

                    {
                        std::unique_lock lk(staticCacheMutex);
                        staticCache[actor_ptr] = s;
                    }
                }
            }
        }

        auto send_destroy_packet = [&](ULONGLONG actor_id) {
            EntityDestroyPacket pkt{ {PacketType::ENTITY_DESTROY}, actor_id };
            networkServer.send_packet(&pkt, sizeof(pkt));
            };
        {
            std::unique_lock lk(playerCacheMutex);
            for (auto it = playerCache.begin(); it != playerCache.end();) {
                if (seen_this_frame.find(it->first) == seen_this_frame.end()) {
                    if (it->second.client_knows_about_it) send_destroy_packet(it->first);
                    it = playerCache.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
        {
            std::unique_lock lk(staticCacheMutex);
            for (auto it = staticCache.begin(); it != staticCache.end();) {
                if (seen_this_frame.find(it->first) == seen_this_frame.end()) {
                    send_destroy_packet(it->first);
                    it = staticCache.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

bool GameLogic::is_actor_aiming_at(
    const Vector3& actorPos,
    const DirectX::XMFLOAT4& actorRot,
    const Vector3& targetPos,
    float fov_degrees)
{
    using namespace DirectX;

    // 根据 Actor 的旋转四元数获取其正前方朝向向量
    XMVECTOR baseFwd = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR rotQuat = XMLoadFloat4(&actorRot);
    XMVECTOR actorFwd = XMVector3Rotate(baseFwd, rotQuat);
    actorFwd = XMVector3Normalize(actorFwd);

    // 获取从 Actor 到目标点(你)的方向向量
    XMVECTOR vActorPos = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&actorPos));
    XMVECTOR vTargetPos = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetPos));
    XMVECTOR vToTarget = XMVectorSubtract(vTargetPos, vActorPos);
    vToTarget = XMVector3Normalize(vToTarget);

    // 计算两个向量的点积，归一化向量的点积等于它们之间夹角的余弦值
    XMVECTOR vDot = XMVector3Dot(actorFwd, vToTarget);
    float dotProduct = XMVectorGetX(vDot);

    // 计算FOV，我们将检查目标是否在指定的 "锥形" 视野内
    float fov_radians = XMConvertToRadians(fov_degrees * 0.5f);
    float cosThreshold = cos(fov_radians);

    // 比较，如果点积 > 阈值, 说明夹角小于 FOV/2, 目标在锥形内
    return dotProduct > cosThreshold;
}

// TODO: 虽然已经做了并行处理，但仍然需要优化
void GameLogic::playerStateUpdateLoop() {
    const int MAX_VERIFICATION_RETRIES = 150; // 最大队伍ID验证重试次数
    const auto aim_persistence_duration = std::chrono::milliseconds(5500); // 瞄准状态持续显示时间

    const int SKELETON_VISIBILITY_INTERVAL = 1; // 骨骼和可见性更新间隔
    const int STATE_AIMING_INTERVAL = 3;        // 状态 (血量/武器/瞄准) 更新间隔

    std::cout << "[GameLogic] Player update loop started." << std::endl;

    while (running.load(std::memory_order_relaxed)) {
        // 获取并递增循环计数器，用于频率控制
        uint64_t current_loop_count = g_player_update_loop_counter.fetch_add(1, std::memory_order_relaxed);

        // 判断本轮循环需要执行哪些更新
        bool should_update_skeleton_visibility = (current_loop_count % SKELETON_VISIBILITY_INTERVAL == 0);
        bool should_update_state_aiming = (current_loop_count % STATE_AIMING_INTERVAL == 0);

        // 获取必要的共享数据
        Vector3 my_loc;
        int my_team_id = -1;
        bool in_match_now = false;
        ULONGLONG my_pawn_ptr = 0;
        {
            std::shared_lock lk(sharedData.mutex); // 使用读锁获取共享数据
            in_match_now = sharedData.is_in_match;
            my_loc = sharedData.my_location;
            my_team_id = sharedData.my_team_id;
            my_pawn_ptr = sharedData.my_pawn_ptr;
        }

        // 如果不在对局中或自身数据无效，则跳过本轮更新
        if (!in_match_now || my_team_id == -1 || my_pawn_ptr == 0 || !valid_location(my_loc)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 稍微等待
            continue;
        }

        // 获取当前所有玩家的 actor_ptr
        std::vector<ULONGLONG> keys;
        {
            std::shared_lock lk(playerCacheMutex); // 使用读锁获取玩家列表
            keys.reserve(playerCache.size());
            for (const auto& kv : playerCache) keys.push_back(kv.first);
        }

        // 如果没有玩家需要更新，则跳过
        if (keys.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 120)); // 维持循环频率
            continue;
        }

        // 使用 std::async 并行处理每个玩家
        std::vector<std::future<void>> tasks;
        tasks.reserve(keys.size());
        bool map_is_loaded = mapLoaded.load(std::memory_order_relaxed); // 获取地图加载状态

        for (const auto key : keys) {
            // 为每个玩家启动一个异步任务
            tasks.push_back(std::async(std::launch::async, [this, key, my_loc, my_team_id, aim_persistence_duration, map_is_loaded, should_update_skeleton_visibility, should_update_state_aiming] {
                // 获取玩家缓存数据的副本
                std::optional<CachedPlayer> player_copy_opt;
                {
                    std::shared_lock lk(playerCacheMutex); // 读锁获取副本
                    auto it = playerCache.find(key);
                    if (it != playerCache.end()) {
                        player_copy_opt = it->second; // 复制数据
                    }
                }
                // 如果在获取副本时玩家已被移除，则直接返回
                if (!player_copy_opt.has_value()) return;
                CachedPlayer p = player_copy_opt.value(); // 使用副本进行操作

                // 基础数据读取与检查

                // 读取队伍ID，如果是真人玩家且队伍ID未知
                if (!p.is_ai && p.team_id == -1) {
                    if (p.player_state_ptr == 0) p.player_state_ptr = read<ULONGLONG>(p.actor_ptr + Offsets::PlayerState);
                    if (p.player_state_ptr) p.team_id = read<int>(p.player_state_ptr + Offsets::TeamID + teamIdDelta.load());
                }

                // 读取血量
                float current_health = p.health; // 先用缓存中的值
                if (should_update_state_aiming) {
                    if (p.health_set_ptr == 0) { // 尝试获取 HealthSet 指针
                        ULONGLONG health_comp_ptr = read<ULONGLONG>(p.actor_ptr + Offsets::HealthComp);
                        if (health_comp_ptr) p.health_set_ptr = read<ULONGLONG>(health_comp_ptr + Offsets::HealthSet);
                    }
                    if (p.health_set_ptr) { // 如果指针有效，读取实时血量
                        current_health = read<float>(p.health_set_ptr + Offsets::DFMHealthDataComponent);
                    }
                }

                // 检查玩家是否应被销毁 (血量 <= 0.1 或已撤离)
                bool should_be_destroyed = (current_health <= 0.1f);
                if (!should_be_destroyed && p.player_state_ptr && read<BYTE>(p.player_state_ptr + Offsets::bFinishGame) > 0) {
                    should_be_destroyed = true; // 玩家已撤离
                }

                if (should_be_destroyed) {
                    std::unique_lock lk(playerCacheMutex); // 写锁修改缓存
                    auto it = playerCache.find(key);
                    if (it != playerCache.end()) {
                        if (it->second.client_knows_about_it) { // 如果客户端知道这个玩家，发送销毁包
                            EntityDestroyPacket destroy_pkt{ {PacketType::ENTITY_DESTROY}, p.actor_ptr };
                            networkServer.send_packet(&destroy_pkt, sizeof(destroy_pkt));
                        }
                        playerCache.erase(it); // 从缓存中移除
                    }
                    return; // 结束此玩家的处理
                }

                // 验证队伍ID，如果队伍ID仍未知，增加重试次数，超时则移除
                if (p.team_id == -1) {
                    std::unique_lock lk(playerCacheMutex); // 写锁修改缓存
                    auto it = playerCache.find(key);
                    if (it != playerCache.end()) {
                        it->second.verification_retries++;
                        if (it->second.verification_retries > MAX_VERIFICATION_RETRIES) {
                            if (g_cfg.debug.load()) std::cout << "[Debug] Player entity 0x" << std::hex << p.actor_ptr << " verification timed out, removing." << std::dec << std::endl;
                            // 如果客户端已知，发送销毁包
                            if (it->second.client_knows_about_it) {
                                EntityDestroyPacket destroy_pkt{ {PacketType::ENTITY_DESTROY}, p.actor_ptr };
                                networkServer.send_packet(&destroy_pkt, sizeof(destroy_pkt));
                            }
                            playerCache.erase(it); // 超时移除
                        }
                    }
                    return; // 未验证成功，不发送更新
                }

                // 处理队友，如果是队友，确保客户端不知道他，然后跳过更新
                if (p.team_id == my_team_id) {
                    std::unique_lock lk_team(playerCacheMutex); // 写锁修改缓存
                    auto it_team = playerCache.find(key);
                    if (it_team != playerCache.end()) {
                        if (it_team->second.client_knows_about_it) { // 如果客户端之前知道他
                            EntityDestroyPacket destroy_pkt{ {PacketType::ENTITY_DESTROY}, p.actor_ptr };
                            networkServer.send_packet(&destroy_pkt, sizeof(destroy_pkt)); // 告诉客户端删除
                            it_team->second.client_knows_about_it = false; // 标记为客户端未知
                        }
                        // 更新缓存中的队伍ID（可能之前是-1）
                        it_team->second.team_id = p.team_id;
                        it_team->second.verification_retries = 0; // 重置验证计数器
                    }
                    return; // 跳过队友的数据包发送
                }

                // 发送创建包
                bool just_created = false;
                if (!p.client_knows_about_it) {
                    // 读取玩家名称、干员、护甲等静态信息
                    if (!p.is_ai) {
                        if (p.name.empty() || p.name == "AI") { // 避免重复读取名字
                            ULONGLONG name_ptr = read<ULONGLONG>(p.player_state_ptr + Offsets::PlayerNamePrivate);
                            if (name_ptr) {
                                wchar_t buffer[32] = { 0 };
                                if (read_bytes(name_ptr, buffer, sizeof(buffer) - sizeof(wchar_t))) {
                                    char narrow[64] = { 0 };
                                    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, narrow, sizeof(narrow) - 1, nullptr, nullptr);
                                    p.name = (strlen(narrow) > 0) ? narrow : "Player";
                                }
                                else p.name = "Player";
                            }
                            else p.name = "Player";
                        }
                        if (p.operator_name.empty()) { // 避免重复读取干员
                            p.operator_name = get_operator_name(p.player_state_ptr);
                        }
                        // 读取护甲信息
                        ULONGLONG equip_comp_ptr = read<ULONGLONG>(p.actor_ptr + Offsets::CharacterEquipComponent);
                        if (equip_comp_ptr) {
                            ULONGLONG equip_info_array_ptr = read<ULONGLONG>(equip_comp_ptr + Offsets::EquipmentInfoArray);
                            if (equip_info_array_ptr) {
                                BYTE raw_helmet = read<BYTE>(equip_info_array_ptr + Offsets::EquipmentSlot_Helmet);
                                BYTE raw_armor = read<BYTE>(equip_info_array_ptr + Offsets::EquipmentSlot_Armor);
                                p.helmet_level = categorize_armor_level(raw_helmet);
                                p.armor_level = categorize_armor_level(raw_armor);
                            }
                        }
                    }
                    else { // 如果是 AI
                        if (p.ai_type.empty()) p.ai_type = get_ai_type(p.actor_ptr);
                        if (p.name.empty()) p.name = "AI";
                        p.team_id = 99; // AI 默认队伍ID
                    }

                    // 构建并发送创建包
                    PlayerCreatePacket create_pkt;
                    create_pkt.header.type = PacketType::ENTITY_CREATE;
                    create_pkt.actor_id = p.actor_ptr;
                    strncpy_s(create_pkt.name, p.name.c_str(), _TRUNCATE);
                    strncpy_s(create_pkt.ai_check, (p.is_ai ? "Is AI" : "Is Player"), _TRUNCATE);
                    strncpy_s(create_pkt.ai_type, p.ai_type.c_str(), _TRUNCATE);
                    strncpy_s(create_pkt.operator_name, p.operator_name.c_str(), _TRUNCATE);
                    create_pkt.team_id = p.team_id;
                    create_pkt.helmet_level = p.helmet_level;
                    create_pkt.armor_level = p.armor_level;
                    networkServer.send_packet(&create_pkt, sizeof(create_pkt));

                    p.client_knows_about_it = true; // 标记客户端现在知道了
                    just_created = true; // 标记为刚创建
                }

                // 更新位置和旋转，每次循环都执行
                Vector3 current_loc = get_actor_location(p.actor_ptr); // 读取实时位置
                if (!valid_location(current_loc)) { return; } // 如果位置无效，则跳过后续处理
                p.location = current_loc; // 更新副本中的位置

                if (p.root_component_ptr == 0) p.root_component_ptr = decrypt_shift(p.actor_ptr + Offsets::RootComponent);
                if (p.root_component_ptr) {
                    FTransform c2w = read<FTransform>(p.root_component_ptr + Offsets::ComponentToWorld);
                    p.rotation = c2w.Rotation; // 更新副本中的旋转
                }

                // 发送位置包，高频
                PlayerLocationPacket loc_pkt;
                loc_pkt.header.type = PacketType::PLAYER_LOCATION_UPDATE;
                loc_pkt.actor_id = p.actor_ptr;
                loc_pkt.location = { p.location.x, p.location.y, p.location.z };
                loc_pkt.rotation = p.rotation;
                float dx = my_loc.x - p.location.x, dy = my_loc.y - p.location.y, dz = my_loc.z - p.location.z;
                loc_pkt.distance = std::sqrt(dx * dx + dy * dy + dz * dz) / 100.f; // 计算距离
                loc_pkt.pawn_ptr = p.actor_ptr; // 包含Pawn指针
                networkServer.send_packet(&loc_pkt, sizeof(loc_pkt));

                // 更新骨骼和可见性
                bool final_bone_visibility[NUM_BONES]; // 用于存储可见性结果
                memset(final_bone_visibility, 0, sizeof(final_bone_visibility)); // 默认所有骨骼不可见
                bool skeleton_updated_this_frame = false;

                if (should_update_skeleton_visibility) {
                    ULONGLONG mesh_ptr = read<ULONGLONG>(p.actor_ptr + Offsets::Mesh); // 获取 Mesh 指针
                    if (mesh_ptr) {
                        get_bone_world_positions(mesh_ptr, p.skeleton); // 读取所有骨骼的世界坐标
                        skeleton_updated_this_frame = true; // 标记骨骼已更新

                        // 执行可见性检查，中频
                        if (map_is_loaded && valid_location(my_loc)) {
                            // 对所有有效的骨骼坐标进行射线检测
                            std::vector<std::future<bool>> visibility_tasks;
                            visibility_tasks.reserve(NUM_BONES);
                            for (int i = 0; i < NUM_BONES; ++i) {
                                if (is_valid_bone_position(p.skeleton[i])) {
                                    Vector3 bone_pos = { p.skeleton[i].x, p.skeleton[i].y, p.skeleton[i].z };
                                    Vector3 my_current_loc = my_loc; // 捕获当前循环的 my_loc
                                    visibility_tasks.emplace_back(std::async(std::launch::async, [bone_pos, my_current_loc]() -> bool {
                                        return RTModel::IsVisible(my_current_loc, bone_pos); // 并行执行射线检测
                                        }));
                                }
                                else {
                                    // 对于无效骨骼，直接返回 false
                                    visibility_tasks.emplace_back(std::async(std::launch::async, [] { return false; }));
                                }
                            }
                            // 等待所有可见性检查完成并收集结果
                            for (int i = 0; i < NUM_BONES; ++i) {
                                try { final_bone_visibility[i] = visibility_tasks[i].get(); }
                                catch (...) { final_bone_visibility[i] = false; } // 异常处理
                            }
                        }
                        else {
                            // 地图未加载或自身位置无效，将所有有效的骨骼标记为可见
                            for (int i = 0; i < NUM_BONES; ++i) {
                                final_bone_visibility[i] = is_valid_bone_position(p.skeleton[i]);
                            }
                        }
                    }
                    else { // 如果没有 Mesh 指针
                        memset(p.skeleton, 0, sizeof(p.skeleton)); // 清空骨骼数据
                        memset(final_bone_visibility, 0, sizeof(final_bone_visibility)); // 全部设为不可见
                        skeleton_updated_this_frame = true; // 标记为已更新(清空也是更新)
                    }

                    // 发送骨骼包，中频
                    PlayerSkeletonPacket skel_pkt;
                    skel_pkt.header.type = PacketType::PLAYER_SKELETON_UPDATE;
                    skel_pkt.actor_id = p.actor_ptr;
                    memcpy(skel_pkt.skeleton, p.skeleton, sizeof(skel_pkt.skeleton));
                    memcpy(skel_pkt.bone_visibility, final_bone_visibility, sizeof(skel_pkt.bone_visibility));
                    networkServer.send_packet(&skel_pkt, sizeof(skel_pkt));
                }

                // 更新状态 (血量/武器/瞄准)，低频
                bool is_currently_aiming_this_frame = false; // 当前帧是否在瞄准
                bool send_aiming_at_me_flag = false; // 最终是否发送瞄准标志
                std::string current_held_weapon = p.held_weapon; // 先用缓存值

                if (should_update_state_aiming) {
                    // 读取当前武器
                    current_held_weapon = get_held_weapon(p.actor_ptr);

                    // 添加护甲读取逻辑
                    int current_helmet_level = 0;
                    int current_armor_level = 0;
                    float current_helmet_hp = 0.f;
                    float current_armor_hp = 0.f;
                    float max_helmet_hp = 0.f;
                    float max_armor_hp = 0.f;

                    if (p.equip_info_array_ptr == 0) {
                        ULONGLONG equip_comp_ptr = read<ULONGLONG>(p.actor_ptr + Offsets::CharacterEquipComponent);
                        if (equip_comp_ptr) {
                            p.equip_info_array_ptr = read<ULONGLONG>(equip_comp_ptr + Offsets::EquipmentInfoArray);
                        }
                    }

                    if (p.equip_info_array_ptr) {
                        BYTE raw_helmet = read<BYTE>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Helmet);
                        BYTE raw_armor = read<BYTE>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Armor);
                        current_helmet_level = categorize_armor_level(raw_helmet);
                        current_armor_level = categorize_armor_level(raw_armor);

                        if (current_helmet_level > 0) {
                            current_helmet_hp = read<float>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Helmet + 0x18);
                            max_helmet_hp = read<float>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Helmet + 0x1C);
                        }
                        if (current_armor_level > 0) {
                            current_armor_hp = read<float>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Armor + 0x18);
                            max_armor_hp = read<float>(p.equip_info_array_ptr + Offsets::EquipmentSlot_Armor + 0x1C);
                        }

                        p.helmet_level = current_helmet_level;
                        p.armor_level = current_armor_level;
                        p.helmet_hp = current_helmet_hp;
                        p.armor_hp = current_armor_hp;
                        p.armor_max_hp = max_armor_hp;
                        p.helmet_max_hp = max_helmet_hp;
                    }

                    // 执行瞄准检查
                    auto now = std::chrono::steady_clock::now(); // 获取当前时间
                    if (map_is_loaded && valid_location(my_loc)) {
                        Vector3 enemyViewPos = p.location; // 默认使用 Actor 位置
                        int headIdx = static_cast<int>(BoneID::Head);
                        // 如果骨骼数据在本帧更新过且头部坐标有效，使用头部坐标作为观察点
                        if (skeleton_updated_this_frame && headIdx < NUM_BONES && is_valid_bone_position(p.skeleton[headIdx])) {
                            enemyViewPos = { p.skeleton[headIdx].x, p.skeleton[headIdx].y, p.skeleton[headIdx].z };
                        }
                        else if (!skeleton_updated_this_frame && player_copy_opt.has_value()) {
                            // 如果骨骼数据本帧未更新，尝试使用上一帧缓存的头部坐标
                            const auto& cached_skel = player_copy_opt->skeleton;
                            if (headIdx < NUM_BONES && is_valid_bone_position(cached_skel[headIdx])) {
                                enemyViewPos = { cached_skel[headIdx].x, cached_skel[headIdx].y, cached_skel[headIdx].z };
                            }
                        }
                        // 检查敌人是否大致朝向我
                        if (is_actor_aiming_at(enemyViewPos, p.rotation, my_loc, 170.0f)) {
                            // 如果朝向我，再进行一次射线检测确认视线是否通畅
                            if (RTModel::IsVisible(enemyViewPos, my_loc)) {
                                is_currently_aiming_this_frame = true; // 确实在瞄准我
                            }
                        }
                    }
                    // 更新瞄准状态和时间戳
                    if (is_currently_aiming_this_frame) {
                        p.last_aiming_at_me_time = now; // 更新最后瞄准时间
                        send_aiming_at_me_flag = true;  // 立即发送瞄准标志
                    }
                    else {
                        // 如果当前没在瞄准，检查距离上次瞄准是否在持续时间内
                        auto time_since_last_aim = std::chrono::duration_cast<std::chrono::milliseconds>(now - p.last_aiming_at_me_time);
                        if (time_since_last_aim < aim_persistence_duration) {
                            send_aiming_at_me_flag = true; // 仍在持续时间内，继续发送标志
                        }
                        else {
                            send_aiming_at_me_flag = false; // 超时，不发送标志
                        }
                    }

                    // 发送状态包，低频
                    PlayerStatePacket state_pkt;
                    state_pkt.header.type = PacketType::PLAYER_STATE_UPDATE;
                    state_pkt.actor_id = p.actor_ptr;
                    state_pkt.health = current_health; // 使用本帧读取或缓存的血量
                    strncpy_s(state_pkt.held_weapon, current_held_weapon.c_str(), _TRUNCATE);
                    state_pkt.is_aiming_at_me = send_aiming_at_me_flag;
                    state_pkt.helmet_level = current_helmet_level;
                    state_pkt.armor_level = current_armor_level;
                    state_pkt.helmet_hp = current_helmet_hp;
                    state_pkt.armor_hp = current_armor_hp;
                    state_pkt.armor_max_hp = max_armor_hp;
                    state_pkt.helmet_max_hp = max_helmet_hp;

                    networkServer.send_packet(&state_pkt, sizeof(state_pkt));
                }

                // 将本次更新的数据写回缓存
                {
                    std::unique_lock lk(playerCacheMutex); // 写锁更新缓存
                    auto it = playerCache.find(key);
                    if (it != playerCache.end()) {
                        // 更新基础信息，如果刚创建或验证通过
                        if (just_created || it->second.team_id == -1) {
                            it->second.name = p.name;
                            it->second.operator_name = p.operator_name;
                            it->second.helmet_level = p.helmet_level;
                            it->second.armor_level = p.armor_level;
                            it->second.client_knows_about_it = true; // 确保标记为已知
                            it->second.verification_retries = 0;    // 重置验证计数器
                        }
                        // 更新队伍ID，如果刚验证通过
                        it->second.team_id = p.team_id;
                        // 更新指针
                        it->second.player_state_ptr = p.player_state_ptr;
                        it->second.health_set_ptr = p.health_set_ptr;
                        it->second.root_component_ptr = p.root_component_ptr;
                        // 更新动态数据
                        it->second.location = p.location; // 位置总是更新
                        it->second.rotation = p.rotation; // 旋转总是更新
                        it->second.equip_info_array_ptr = p.equip_info_array_ptr;
                        // 按需更新 (只在对应频率的帧更新)
                        if (should_update_state_aiming) {
                            it->second.health = current_health;
                            it->second.held_weapon = current_held_weapon;
                            it->second.last_aiming_at_me_time = p.last_aiming_at_me_time; // 更新瞄准时间戳
                            it->second.helmet_level = p.helmet_level;
                            it->second.armor_level = p.armor_level;
                            it->second.helmet_hp = p.helmet_hp;
                            it->second.armor_hp = p.armor_hp;
                        }
                        if (should_update_skeleton_visibility && skeleton_updated_this_frame) {
                            memcpy(it->second.skeleton, p.skeleton, sizeof(p.skeleton)); // 更新骨骼数据
                        }
                        // bone_visibility 不存储在缓存中，随骨骼包发送
                    }
                }

                })); // 结束 std::async 的 lambda
        } // 结束 for (const auto key : keys)

        // 等待所有异步任务完成
        for (auto& task : tasks) {
            try { task.get(); } // 获取任务结果
            catch (const std::exception& e) {
                std::cerr << "[Error] Exception in playerStateUpdateLoop task: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "[Error] Unknown exception in playerStateUpdateLoop task." << std::endl;
            }
        }

        // 控制循环频率
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 144)); // ~144Hz
    }
    std::cout << "[GameLogic] Player update loop stopped." << std::endl;
}

void GameLogic::staticStateUpdateLoop() {
    while (running) {
        bool in_match_now = false;
        {
            std::shared_lock lk(sharedData.mutex);
            in_match_now = sharedData.is_in_match;
        }

        if (!in_match_now) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::vector<ULONGLONG> keys;
        {
            std::shared_lock lk(staticCacheMutex);
            if (staticCache.empty()) {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            keys.reserve(staticCache.size());
            for (const auto& kv : staticCache) keys.push_back(kv.first);
        }

        for (const auto key : keys) {
            std::unique_lock lk(staticCacheMutex);
            auto it = staticCache.find(key);
            if (it == staticCache.end()) continue;
            auto& s = it->second;

            if (s.cls == StaticClass::ExtractionPoint) continue;

            if (s.cls != StaticClass::Item && s.root_component_ptr) {
                Vector3 current_location = read<Vector3>(s.root_component_ptr + Offsets::RelativeLocation);

                if (valid_location(current_location) &&
                    (std::fabs(current_location.x - s.location.x) > 5.0f ||
                        std::fabs(current_location.y - s.location.y) > 5.0f ||
                        std::fabs(current_location.z - s.location.z) > 5.0f))
                {
                    if (g_cfg.debug.load()) std::cout << "[Debug] Static Actor 0x" << std::hex << s.actor_ptr << " moved significantly, sending destroy." << std::dec << std::endl;
                    EntityDestroyPacket destroy_pkt{ {PacketType::ENTITY_DESTROY}, s.actor_ptr };
                    networkServer.send_packet(&destroy_pkt, sizeof(destroy_pkt));
                    staticCache.erase(it);
                    continue;
                }
                if (!valid_location(current_location) && g_cfg.debug.load()) {
                    std::cerr << "[Debug] staticStateUpdateLoop: Actor 0x" << std::hex << s.actor_ptr << " current location invalid, skipping move check." << std::dec << std::endl;
                }
            }

            if (s.cls == StaticClass::Container || s.cls == StaticClass::Safe || s.cls == StaticClass::PlayerLootBox || s.cls == StaticClass::HackerPC) {
                auto open_state = try_read_open_state_once(s.actor_ptr, s.cls);

                if (open_state.has_value()) {
                    bool current_opened_state = open_state.value();
                    if (current_opened_state != s.last_sent_opened_state) {
                        if (g_cfg.debug.load()) std::cout << "[Debug] Static Actor 0x" << std::hex << s.actor_ptr << " state changed to: " << (current_opened_state ? "Opened/Looted" : "Closed/Unlooted") << std::dec << std::endl;

                        s.is_opened = current_opened_state;

                        if (s.cls == StaticClass::HackerPC) {
                            HackerPcCreatePacket pkt;
                            pkt.header.type = PacketType::HACKER_PC_CREATE;
                            pkt.actor_id = s.actor_ptr;
                            pkt.world_location = { s.location.x, s.location.y, s.location.z };
                            if (s.class_name.find("CodedLock") != std::string::npos) {
                                strncpy_s(pkt.name, u8"密码门", _TRUNCATE);
                                pkt.password = read<int32_t>(s.actor_ptr + Offsets::PwdSum);
                            }
                            else {
                                strncpy_s(pkt.name, u8"骇客电脑", _TRUNCATE);
                                pkt.password = read<int32_t>(s.actor_ptr + Offsets::Password);
                            }
                            pkt.is_looted = s.is_opened;
                            if (pkt.password > 100 && pkt.password < 10000) {
                                networkServer.send_packet(&pkt, sizeof(pkt));
                            }
                        }
                        else {
                            EntityStateUpdatePacket pkt;
                            pkt.header.type = PacketType::ENTITY_UPDATE_STATE;
                            pkt.actor_id = s.actor_ptr;
                            pkt.is_opened = s.is_opened;
                            networkServer.send_packet(&pkt, sizeof(pkt));
                        }
                        s.last_sent_opened_state = s.is_opened;
                    }
                }
                else if (g_cfg.debug.load()) {
                    std::cerr << "[Debug] staticStateUpdateLoop: Failed to read valid open state for Actor 0x" << std::hex << s.actor_ptr << std::dec << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GameLogic::consoleLoop() const {
    while (running) {
        size_t p_cache_size, s_cache_size, pending_size;
        Vector3 my_loc_for_console;
        bool inMatch;

        {
            std::shared_lock lk(sharedData.mutex);
            my_loc_for_console = sharedData.my_location;
            inMatch = sharedData.is_in_match;
        }
        {
            std::shared_lock lk(playerCacheMutex);
            p_cache_size = playerCache.size();
        }
        {
            std::shared_lock lk(staticCacheMutex);
            s_cache_size = staticCache.size();
        }
        {
            std::shared_lock lk(pendingActorsMutex);
            pending_size = pendingActors.size();
        }

        //if (!g_cfg.debug.load()) clear_console();

        std::cout << "--- OwODeviceIOControl -- [v12.6 - @FangYan] ---\n"
            << "Network: " << (networkServer.is_client_connected() ? "Client Connected" : "Waiting for Client...") << "\n"
            << "InMatch: " << (inMatch ? "Yes" : "No") << "\n";

        if (!inMatch) {
            std::cout << "Status: Not in a match, waiting...\n";
        }
        else {
            std::cout << "My Location: (" << std::fixed << std::setprecision(1)
                << my_loc_for_console.x / 100.f << ", " << my_loc_for_console.y / 100.f << ", " << my_loc_for_console.z / 100.f << ")\n"
                << "Cached Players: " << p_cache_size
                << " | Cached Statics: " << s_cache_size
                << " | Pending Actors: " << pending_size << "\n";
        }
        std::cout << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}