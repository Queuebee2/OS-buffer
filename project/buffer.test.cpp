#include <gtest/gtest.h>
#include <stdlib.h>
#include <fcntl.h>
#include <thread>
#include <vector>

using namespace std;

// GTEST info message hack https://stackoverflow.com/a/48924764/6934388
#define GTEST_COUT_INFO std::cerr << "\033[33m" << "[   INFO   ] " <<  "\033[37m"

// declarations of methods you want to test (should match exactly)
void bufferReset();
void writeLog(string msg, int position);
void readLog();
void bufferAdd(int addition);
int bufferReturnRemoved();
void bufferRemove();
void setBound(int requestSize);
void toggleBounds();

// helpers from buffer to access member variables
int getBufferSize();
int getLogSize();
int getValueAt(int ind);
int getCurrentBound();
int getPrevBound();
string readLogAt(int ind);
// int getCurrentIndex(); deprecated

const int UNBOUNDED = -1; // TODO how to just use the same value from buffer.cpp instead of reinitializing here?

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

void taskNoInputReturnInt(int  (*f)()){
    for (int i = 0; i < AMT_ACTIONS; i ++){
        int r = f();
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


TEST(BufferBasic, bufferReset){
    // mess starting values
    bufferReset(); // weird thing to do as this assumes it works correctly already.

    bufferAdd(10);
    toggleBounds();

    // try reset
    bufferReset();
    
    // confirm reset worked (nonexhaustive)
    EXPECT_EQ(getCurrentBound(), -1);
    EXPECT_EQ(getPrevBound(), -1);
    EXPECT_EQ(getBufferSize(), 0);
    EXPECT_EQ(getLogSize(), 0);
}

TEST(BufferBasic, bufferInitialSize){

    bufferReset();
    EXPECT_EQ(0,getBufferSize());
    EXPECT_EQ(0,getLogSize());

    }

TEST(BufferBasic, toggleBounds){
    bufferReset();
    EXPECT_EQ(getCurrentBound(), UNBOUNDED);
    bufferReset();
    toggleBounds();
    EXPECT_EQ(getCurrentBound(), UNBOUNDED);
    bufferReset();
    setBound(10);
    EXPECT_EQ(getCurrentBound(), 10);
    toggleBounds();
    EXPECT_EQ(getCurrentBound(), UNBOUNDED);
    toggleBounds();
    EXPECT_EQ(getCurrentBound(), 10);
}

TEST(BufferBasic, singleAdd){
    int TESTVAL = 100;

    bufferReset();
    
    bufferAdd(TESTVAL);
    EXPECT_EQ(getValueAt(0), TESTVAL);
    EXPECT_EQ(getBufferSize(), 1);

}

TEST(BufferBasic, singleRemove){
    int TESTVAL = 100;
    bufferReset();
    
    bufferAdd(TESTVAL);
    bufferRemove();
    EXPECT_EQ(getBufferSize(), 0);
}

// renamed from testChangeBoundAdd
TEST(BufferBasic, automaticBoundChange){
    int TESTVAL1 = 100;
    int TESTVAL2 = 200;
    int TESTBOUND1 = 20;
    int TESTBOUND2 = 33;
    int TESTBOUND3 = 100;

    bufferReset();

    thread t1(taskWithInput, TESTBOUND1, setBound);
    thread t2(taskWithInput, TESTVAL1, bufferAdd);
    thread t3(taskWithInput, TESTBOUND2, setBound);
    thread t4(taskWithInput, TESTBOUND3, setBound);
    thread t5(taskWithInput, TESTVAL2, bufferAdd);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    // TODO EXPECT_EQ...
    
}

TEST(BufferBasic, automaticBoundRestrict){
    int TESTVAL1 = 100;
    int TESTVAL2 = 200;
    int TESTBOUND1 = 3;
    int TESTBOUND2 = 5;
    int TESTBOUND3 = 7;

    bufferReset();

    thread t1(taskWithInput, TESTBOUND1, setBound);
    thread t2(taskWithInput, TESTVAL1, bufferAdd);
    thread t3(taskWithInput, TESTBOUND2, setBound);
    thread t4(taskWithInput, TESTBOUND3, setBound);
    thread t5(taskWithInput, TESTVAL2, bufferAdd);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    
    // the last set bound should be equal or lower to the max of the bounds
    bool bBoundWithinBounds = getCurrentBound() <= (max({TESTBOUND1, TESTBOUND2, TESTBOUND3}));
    EXPECT_EQ(bBoundWithinBounds, true);

    //assert whether there are no more elements than restricted by bound
    // deprecated when current_indx isnt used anymore
    // bool bIndexWithinBounds = getCurrentIndex() < getCurrentBound(); 
    // EXPECT_EQ(bIndexWithinBounds, true);
}

TEST(BufferBasic, singleRemoveWithRetrieval){
    int TESTVAL = 100;
    bufferReset();
    
    bufferAdd(TESTVAL);
    int r = bufferReturnRemoved();
    EXPECT_EQ(getBufferSize(), 0);
    EXPECT_EQ(r, TESTVAL);

}

TEST(Buffer, doubleAdd){
    int TESTVAL1 = 100;
    int TESTVAL2 = 200;
    bufferReset();
    
    bufferAdd(TESTVAL1);
    EXPECT_EQ(getValueAt(0), TESTVAL1);

    bufferAdd(TESTVAL2);
    EXPECT_EQ(getValueAt(1), TESTVAL2);

}

TEST(Buffer, maxValueAdd){
    bufferReset();
    bufferAdd(INT16_MAX);
    bufferAdd(INT32_MAX);
    // bufferAdd(INT64_MAX);
    EXPECT_EQ(getValueAt(0), INT16_MAX);
    EXPECT_EQ(getValueAt(1), INT32_MAX);
    // EXPECT_EQ(getValueAt(2), INT64_MAX);
    
}

TEST(Buffer, returnLastElementAfterBoundChange){
    int TESTVAL1 = 100;
    int TESTVAL2 = 200;
    bufferReset();

    bufferAdd(TESTVAL1);
    bufferAdd(TESTVAL1);
    bufferAdd(TESTVAL2);

    setBound(2);

    int r = bufferReturnRemoved();

    EXPECT_EQ(r, TESTVAL1);

    

}

TEST(Buffer, minValueAddition){
    bufferReset();
    bufferAdd(INT16_MIN);
    bufferAdd(INT32_MIN);
    // bufferAdd(INT64_MIN);
    EXPECT_EQ(getValueAt(0), INT16_MIN);
    EXPECT_EQ(getValueAt(1), INT32_MIN);
    // EXPECT_EQ(getValueAt(2), INT64_MIN);
}


TEST(Buffer, dualAddition){

    bufferReset();
    
    thread firstRemover(taskWithInput, 1, bufferAdd);
    thread secondRemover(taskWithInput, 2, bufferAdd);
    
    firstRemover.join();
    secondRemover.join();

    // expect the amount of actions per thread times the amount of threads
    EXPECT_EQ(getBufferSize(), AMT_ACTIONS*2);

    }

TEST(BufferConcurrency, dualConcurrentAddition){
    bool flag1;
    bool flag2;
    int TESTVALUE_1 = 1;
    int TESTVALUE_2 = 2;

     for (size_t TEST_INDEX = 0; TEST_INDEX < AMT_TESTS; TEST_INDEX++)
    {   
        // if (testsuccess == true){
        //     break;
        // }
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
            return SUCCEED();
        } else {
            flag1 = false;
            flag2 = false;
        }
    }
    GTEST_COUT_INFO << "this test can fail by chance!" << endl;
    FAIL();

}


TEST(BufferConcurrency, addAndRemoveEverything){

    int TESTVALUE_1 = 1;
    int TESTVALUE_2 = 2;

    bufferReset();

    // add 2 * AMT_ACTIONS values
    thread t1(taskWithInput, TESTVALUE_1, bufferAdd);
    thread t2(taskWithInput, TESTVALUE_2, bufferAdd);

    t1.join();
    t2.join();

    // remove 2 * AMT_ACTIONS values
    thread t3(taskNoInput, bufferRemove);
    thread t4(taskNoInput, bufferRemove);

    t3.join();
    t4.join();

    EXPECT_EQ(getBufferSize(), 0);

} 


TEST(BufferConcurrency, addAndRemoveEverythingWithRetrieval){

    int TESTVALUE_1 = 1;
    int TESTVALUE_2 = 2;

    bufferReset();

    // add 2 * AMT_ACTIONS values
    thread t1(taskWithInput, TESTVALUE_1, bufferAdd);
    thread t2(taskWithInput, TESTVALUE_2, bufferAdd);

    t1.join();
    t2.join();

    // remove 2 * AMT_ACTIONS values
    thread t3(taskNoInputReturnInt, bufferReturnRemoved);
    thread t4(taskNoInputReturnInt, bufferReturnRemoved);

    t3.join();
    t4.join();

    EXPECT_EQ(getBufferSize(), 0);

} 

TEST(BufferConcurrency, removeTooMany){
    int TESTVAL = 100;

    bufferReset();
    
    bufferAdd(TESTVAL);

    thread t1(taskNoInput, bufferRemove);
    thread t2(taskNoInput, bufferRemove);

    t1.join();
    t2.join();

    string expected = "added element " + to_string(TESTVAL) + " to buffer succesfully";
    EXPECT_EQ(readLogAt(0), expected);
    EXPECT_EQ(getBufferSize(), 0);
    EXPECT_EQ(getLogSize(), (AMT_ACTIONS*2) + 1);
    // todo EXPECT_EQ for some values in log to see if the fails are logged correctly
}

TEST(BufferConcurrency, addAndRemove){
    int TESTVAL1 = 100;
    int TESTVAL2 = 200;

    bufferReset();

    thread t1(taskWithInput, TESTVAL1, bufferAdd);
    thread t2(taskWithInput, TESTVAL2, bufferAdd);
    thread t3(taskNoInput, bufferRemove);
    thread t4(taskNoInput, bufferRemove);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    EXPECT_EQ(getLogSize(), 4*AMT_ACTIONS);
    // todo: write EXPECT_EQs for log

}






}