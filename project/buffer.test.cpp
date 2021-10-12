#include <gtest/gtest.h>
#include <stdlib.h>
#include <fcntl.h>
#include <thread>
#include <vector>

using namespace std;

#define BUFFER "../build/buffer -t"

// declarations of methods you want to test (should match exactly)
void bufferReset();
void writeLog(string msg, int position);
void readLog();
void bufferAdd(int addition);
int bufferReturnRemoved();
void bufferRemove();
void setBound(int requestSize);
void toggleBounds();
int getBufferSize();
int getValueAt(int ind);

const int AMT_TESTS = 10;
const int AMT_ACTIONS = 100;
const int AMT_HALF = 50;


namespace {

// helpers 
void taskWithInput(int number, void (*func)(int)){
    for (int i = 0; i < AMT_ACTIONS; i++){
        func(number);
    }
}

void taskNoInput(void  (*f)()){
    for (int i = 0; i < AMT_ACTIONS; i ++){
        f();
    }
}


/*
 █████╗  ██████╗████████╗██╗   ██╗ █████╗ ██╗         ████████╗███████╗███████╗████████╗███████╗
██╔══██╗██╔════╝╚══██╔══╝██║   ██║██╔══██╗██║         ╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██╔════╝
███████║██║        ██║   ██║   ██║███████║██║            ██║   █████╗  ███████╗   ██║   ███████╗
██╔══██║██║        ██║   ██║   ██║██╔══██║██║            ██║   ██╔══╝  ╚════██║   ██║   ╚════██║
██║  ██║╚██████╗   ██║   ╚██████╔╝██║  ██║███████╗       ██║   ███████╗███████║   ██║   ███████║
╚═╝  ╚═╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝       ╚═╝   ╚══════╝╚══════╝   ╚═╝   ╚══════╝                                                                                
*/


// This test ruins the starting values for all other tests,
// forcing them to have to call bufferReset. Sadly. How to
TEST(Buffer, ruinStartingValues){
    bufferAdd(10);
    toggleBounds();
}

TEST(Buffer, bufferInitialSize){
    
    bufferReset();
    EXPECT_EQ(0,getBufferSize());

    }

TEST(Buffer, dualAddTest){

    bufferReset();
    
    thread firstRemover(taskWithInput, 1, bufferAdd);
    thread secondRemover(taskWithInput, 2, bufferAdd);
    
    firstRemover.join();
    secondRemover.join();

    // expect the amount of actions per thread times the amount of threads
    EXPECT_EQ(getBufferSize(), AMT_ACTIONS*2);

    }

TEST(Buffer, dualConcurrentAddTest){
    bool flag1;
    bool flag2;
    bool testsuccess = false;
    int TESTVALUE_1 = 1;
    int TESTVALUE_2 = 2;

     for (size_t TEST_INDEX = 0; TEST_INDEX < AMT_TESTS; TEST_INDEX++)
    {   
        if (testsuccess == true){
            break;
        }
        bufferReset();
        setBound(AMT_ACTIONS);

        thread firstAdder(taskWithInput, TESTVALUE_1, bufferAdd);
        thread secondAdder(taskWithInput, TESTVALUE_2, bufferAdd);

        firstAdder.join();
        secondAdder.join();

        for (size_t i = 0; i < AMT_HALF; i++)
        {
            if (getValueAt(i) == TESTVALUE_1){
                flag1 = true;
            }else if (getValueAt(i) == TESTVALUE_2){
                flag2 = true;
            }
        }
        if (flag1 && flag2){
            testsuccess = true;
        } else {
            flag1 = false;
            flag2 = false;
        }
    }

    EXPECT_EQ(testsuccess, true);

}














}