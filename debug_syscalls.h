#pragma once
const char* syscall_name(int syscall) {
    switch (syscall) {
#define SYSCALL(number, name)  case number: return #name;
#include "linux_syscalls.h"
#undef SYSCALL
    }
    return "unknown-syscall";
}