#include <cstdio>
#include <thread>
#include <chrono>
#include "BlockingQueue.hpp"

class Test
{
//public:
//    Test() {printf("Test()\n");}
//    Test(const Test &copy) {printf("Test(const Test &copy)\n");}
//    Test& operator=(const Test &copy){printf("Test::assign\n");}
};

#define MAXFRAMES 10

int main(int argc, char **argv)
{
    //BlockingQueue<int> queue(5);
    NSA::BlockingQueue<Test> queue(5);
    bool done = false;

    std::thread producer1([&]()
    {
        for (int i = 0; i < MAXFRAMES; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(4));
            printf("Produecer1: %d\n", i);
            queue.push(Test());
        }

        done = true;
    });

    std::thread producer2([&]()
    {
        for (int i = 0; i < MAXFRAMES; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(4));
            printf("Produce2: %d\n", i);
            queue.push(Test());
        }

        done = true;
    });


    std::thread consumer([&](){
        int i = 0;
        for (int i = 0; i < MAXFRAMES; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            Test dst;
            queue.pop(&dst);
            printf("Consumed: %d\n", i);
        }
    });

    producer1.join();
    producer2.join();
    consumer.join();
}