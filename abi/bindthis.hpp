//todo support linux
#pragma once
#if defined(__x86_64__) || defined(_M_AMD64)
#else
#error "YAL::BindThis only supports x64 yet"
#endif
#include<Windows.h>
#include"../core.hpp"
#define YAL_BINDTHIS_CNTREG 3
#include<string>
namespace YAL::ABI{
    static constexpr string_view ASM_MOVE_REG[YAL_BINDTHIS_CNTREG]={"\x4d\x89\xc1","\x49\x89\xd0","\x48\x89\xca"}; //r8->r9 dx->8 cx->dx //win64 RCX,RDX,R8,R9
    static constexpr string_view ASM_JMP_NEXT64="\xff\x25\x00\x00\x00\x00"sv;
    static constexpr string_view ASM_SET_THIS="\x48\xb9"sv;
    static std::string _writePayload(void* thi,void* call){
        std::string buf;
        for(int i=0;i<YAL_BINDTHIS_CNTREG;++i){
            buf.append(ASM_MOVE_REG[i]);
        }
        buf.append(ASM_SET_THIS);
        buf.append((char*)&thi,8);
        buf.append(ASM_JMP_NEXT64);
        buf.append((char*)&call,8);
        return buf;
    }
    static void* _allocPayloadRWX(std::string_view payload){
        void* x=VirtualAlloc(nullptr,payload.size(),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
        memcpy(x,payload.data(),payload.size());
        DWORD dummy;
        VirtualProtect(x,payload.size(),PAGE_EXECUTE_READ,&dummy);
        return x;
    }
    static void* MakeThisBind(void* thi,void* call){
        return _allocPayloadRWX(_writePayload(thi,call));
    }
};