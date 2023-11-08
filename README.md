# yjcServer
现代c++ io_uring 协程服务器(大饼)  

## 原理:
传统服务器采用epoll+函数回调的方式，本项目采用io_uring+c++20协程  
优点:  
    ·封装完成后，业务代码可以采用同步的逻辑编写，实际执行仍为异步，减轻程序员的心理负担  
    ·io_uring利用共享内存的方式减少系统调用次数，(可能)具有比epoll更强的性能  
缺点:  
    ·知识较新，目前文档/资料尚且稀少，学习起来较为困难。  
    ·底层逻辑，尤其是c++20的无栈协程较为抽象，编写起来较为困难。
结构:
    在需要阻塞/异步执行的前面加上co_await关键字，这样协程会暂停，向io_uring提交任务，附加协程的handle后跳转。  
    等到任务执行完成，利用cqe就可以拿到handle,通过resume方法恢复协程的执行  
    这样，业务代码不再需要通过回调函数将原来的逻辑中断，更加简洁直观  
优化:  
    ·通过Multishot连发机制，使得提交一个sqe即可获取多个cqe，进一步提高性能。  
    ·通过内核中的buf_ring,解决常规基于就绪模型中需要预先准备大量buffer造成内存紧张的缺点.  
    ·TODO  

## 遇坑记录:  
1.在xx.cpp内定义全局变量，利用构造函数企图在main函数之前初始化时，如果main函数中没有用到相关内容，这时候将不会链接到最终的可执行文件中，导致初始化不会执行！  
2.对于yaml的解析，一般是先定义一个全局的空g_xx,然后添加变化后的回调函数，之后再导入配置文件，就可以在加载过程中利用newvalue回调解析了  
3.在c++20中，使用std::source_location可以非常方便的代替传统打印函数堆栈，使用[[likely/unlikey]]可以优化  
4.c++20提供的std::format可以代替传统流式输出，格式化非常方便(spdlog中使用fmt库功能相同)  
5.c++17提供了filesystem,可以方便代替传统用stat获取文件属性的非常繁琐的操作.  
6.线程模块中，c++11的std::thread,std::mutex,std::condition_variable可以代替传统的pthread;  
7.c++17提供了scope_mutex(保证多个锁的顺序),std::shared_mutex(读写锁)  