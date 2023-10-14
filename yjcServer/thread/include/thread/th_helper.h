#pragma once
#include<pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace yjcServer{
    inline pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
}