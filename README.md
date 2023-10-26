# yjcServer
现代c++ io_uring 协程服务器(大饼)  

遇坑记录:
1.在xx.cpp内定义全局变量，利用构造函数企图在main函数之前初始化时，如果main函数中没有用到相关内容，这时候将不会链接到最终的可执行文件中，导致初始化不会执行！
2.对于yaml的解析，一般是先定义一个全局的空g_xx,然后添加变化后的回调函数，之后再导入配置文件，就可以在加载过程中利用newvalue回调解析了
3.在c++20中，使用std::source_location可以非常方便的代替传统打印函数堆栈，使用[[likely/unlikey]]可以优化
4.c++20提供的std::format可以代替传统流式输出，格式化非常方便(spdlog中使用fmt库功能相同)
5.c++17提供了filesystem,可以方便代替传统用stat获取文件属性的非常繁琐的操作.
6.线程模块中，c++11的std::thread,std::mutex,std::condition_variable可以代替传统的pthread;  
  c++17提供了scope_mutex(保证多个锁的顺序),std::shared_mutex(读写锁)