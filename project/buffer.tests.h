#include <string>
#include "buffer.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#ifndef DEBUGMODE
#define DEBUGMODE 0
#endif
#ifndef DEBUG
#define DEBUG(x)  do { if (DEBUGMODE>1) { std::cerr << "debug:: " << x << std::endl; } } while (0)
#endif

const int MAX_BOUND_LIMIT = 100;
const int AMT_ACTIONS = 10;
const string WHITE_COLOR  = "\033[37m";
const string RED_COLOR    = "\033[31m";
const string GREEN_COLOR  = "\033[32m";
const string YELLOW_COLOR = "\033[33m";

using namespace std;

void printbuffer(string msg = ""){
    cout << msg << endl;
    for (auto number : my_buffer){
        cout << number << ", ";
    } cout << endl;
}

//** 
// helper functions for asserting the result
//
int assertIntEqual(int val1, int val2, string msg){
    int r = 1;
    cout << msg << " ? ";
    if (val1 == val2){
        cout << GREEN_COLOR << "SUCCES" << WHITE_COLOR ;
        r = 0;
    } else {
        cout << RED_COLOR << "FAIL" << WHITE_COLOR;
    }
    cout << endl;
    return r;
}

int assertStringEqual(string val1, string val2, string msg){
    int r = 1;
    cout << msg << " ? ";
    if ((val1.compare(val2)) == 0){
        cout << GREEN_COLOR << "SUCCES" << WHITE_COLOR ;
        r = 0;
    } else {
        cout << RED_COLOR << "FAIL" << WHITE_COLOR;
    }
    cout << endl;
    return r;
}
//** 
// tasks for performing a specific action on the buffer.
//
void taskbufferAdd() {
    int actions = 0;
	while (actions < AMT_ACTIONS) {
        int r = (rand());
        cout << "Task wants to bufferAdd " << r << endl;
        bufferAdd(r);
        cout << "Task bufferAdded " << r << endl;
        actions++;
	}
}

void taskResize(){
    int actions = 0;
	while (actions < AMT_ACTIONS) {
        int r = (rand()%MAX_BOUND_LIMIT) + 1;
        cout << "Task wants to resize to "<< r<< endl;
        setBound(r);
        cout << "Task resized to: " << r << endl;
        actions++;
	}
}

void taskToggleBound(){
    int actions = 0;
	while (actions < AMT_ACTIONS) {
        cout << "Task wants to toggle bounds " << endl;
        toggleBounds();
        cout << "Task toggled bounds " << endl;
        actions++;
	}
}

void taskAdd(int number){
    int actions = 0;
	while (actions < AMT_ACTIONS){
        DEBUG("adding an element: " << number);
        bufferAdd(number);
        actions++;
    }
}

void taskRemove(){
    int actions = 0;
	while (actions < AMT_ACTIONS){
        DEBUG("trying to remove element" << actions);
        bufferRemove();
        actions++;
    }
}

void taskRemoveWithGet(){
    int actions = 0;
	while (actions < AMT_ACTIONS){
        DEBUG("trying to get removed element" << actions);
        int result = bufferReturnRemoved();
        DEBUG("got " << result << " for element " << actions << "successfully");
        actions++;
    }
}

void taskBound(int bound){
    setBound(bound);
}


/*
- call add
- thread 10 call add
- setboud 10 en thread 10 add..
- kunnen we met 2 threads elementen concurrently toevoegen?
    ( t1 add(1)  en t2 add(2)...)
- 
*/

/*
Below we specify all the tests
*/
int test1(){
    setBound(100);
    cout << "Starting  4 threads for test 1" << endl;
    thread bufferAdder(taskbufferAdd);
    thread resizer(taskResize);
    thread remover(taskRemove);
    thread toggler(taskToggleBound);
    cout << "joining threads" << endl;
    bufferAdder.join();
    resizer.join();
    remover.join();
    toggler.join();
    cout << "Reading log" << endl;
    readLog();
    return 0;
}


int addTest() {
    cout << "Running: " << YELLOW_COLOR << "addTest" << WHITE_COLOR << endl;
    bufferReset();
    thread firstRemover(taskAdd, 1);
    thread secondRemover(taskAdd, 2);    
    firstRemover.join();
    secondRemover.join();
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    assertIntEqual(my_buffer.size(), 20, "buffer size is 20 as expected");
    return 0;
}


int removeCompleteWithGetTest(){
    cout << "Running: " << YELLOW_COLOR << "removeWithGetTest" << WHITE_COLOR << endl;
    bufferReset();
    taskAdd(1);
    taskAdd(42);
    cout << "current index for buffer: " << current_idx << endl;
    printbuffer("starting buffer: ");
    thread firstRemover(taskRemoveWithGet);
    thread secondRemover(taskRemoveWithGet);
    firstRemover.join();
    secondRemover.join();
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    assertIntEqual(my_buffer.size(), 0, "resulting buffer is empty");
    return 0;
}

int removeCompleteTest(){
    cout << "Running: " << YELLOW_COLOR << "removeCompleteTest" << WHITE_COLOR << endl;
    bufferReset();
    taskAdd(1);
    taskAdd(2);
    cout << "current index for buffer: " << current_idx << endl;
    printbuffer("starting buffer: ");
    cout << "starting threads" <<endl;
    thread firstRemover(taskRemove);
    thread secondRemover(taskRemove);
    firstRemover.join();
    secondRemover.join();
    printbuffer("resulting buffer: ");
    cout << "Reading log" << endl;
    readLog();
    return 0;
}

int removeTooManyTest(){
    cout << "Running: " << YELLOW_COLOR << "removeTooManyTest" << WHITE_COLOR << endl;
    bufferReset();
    taskAdd(1);
    printbuffer("starting buffer: ");
    thread firstAdder(taskRemove);
    thread secondAdder(taskRemove);
    firstAdder.join();
    secondAdder.join();
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    return 0;
}

int testConcurrentAdding(){
    bool flag1;
    bool flag2;
    int TESTVALUE_1 = 1;
    int TESTVALUE_2 = 2;
    cout << "Running: " << YELLOW_COLOR << "testConcurrentAdding" << WHITE_COLOR << endl;
    for (size_t TEST_INDEX = 0; TEST_INDEX < 100; TEST_INDEX++)
    {   
        bufferReset();
        setBound(20);
        // assert taskAdd tries to insert a value 10 times
        thread firstAdder(taskAdd, TESTVALUE_1);
        thread secondAdder(taskAdd, TESTVALUE_2);
        firstAdder.join();
        secondAdder.join();
        for (size_t i = 0; i < (1 + ((my_buffer.size() - 1) / 2)); i++)
        {
            if (my_buffer[i] == TESTVALUE_1){
                flag1 = true;
            } else if (my_buffer[i] == TESTVALUE_2){
                flag2 = true;
            }
        }
        // if we found both testvalues in the first half,
        // we expect the threads had concurrent access.
        // warning: it could also mean the threads were alternating (?!)
        if (flag1 && flag2){
            cout << "Write 2 values concurrently" << " ? ";
            cout << GREEN_COLOR << "SUCCES" << WHITE_COLOR << endl; ;
            return 0;
        } else {
            flag1 = false;
            flag2 = false;
        }
    }
    // if all 100 tests did not have any mixes of values 1 and 2 in the
    // first half of the buffer, the concurrency test failed.
    
    cout << "Write 2 values concurrently" << " ? ";        
    cout << RED_COLOR << "FAIL" << WHITE_COLOR << endl;
    return -1;
}

int addAndRemoveTest(){
    cout << "Running: " << YELLOW_COLOR << "addAndRemoveTest" << WHITE_COLOR << endl;
    bufferReset();
    thread firstAdder(taskAdd,1);
    thread firstRemover(taskRemove);
    thread secondAdder(taskAdd,2);
    thread secondRemover(taskRemove);
    firstRemover.join();
    firstAdder.join();
    secondAdder.join();
    secondRemover.join();
    //assert whether all tasks have been performed
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    return 0;
}

int testChangeBound(){
    cout << "Running: " << YELLOW_COLOR << "testChangeBound" << WHITE_COLOR << endl;
    bufferReset();
    // todo put jobs in loop so they actually compete
    thread firstBounder(taskBound,10);
    thread secondBounder(taskBound,20);
    thread thirdBounder(taskBound,5);
    firstBounder.join();
    secondBounder.join();
    thirdBounder.join();
    //assert whether current Bound is either 10,20 or 5
    cout << "Reading log" << endl;
    readLog();
    cout << "final bound: " << current_bound << endl;
    return 0;
}

int testChangeBoundAdd(){
    cout << "Running: " << YELLOW_COLOR << "testChangeBoundAdd" << WHITE_COLOR << endl;
    bufferReset();
    thread firstBounder(taskBound,20);
    thread firstAdder(taskAdd, 1);
    thread secondBounder(taskBound,33);
    thread thirdBounder(taskBound,100);
    thread secondAdder(taskAdd, 2);
    firstBounder.join();
    secondBounder.join();
    thirdBounder.join();
    firstAdder.join();
    secondAdder.join();
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    assertIntEqual(current_idx, 20, "All elements are added to buffer");
    return 0;
}

int testRestrictBoundAdd(){
    cout << "Running: " << YELLOW_COLOR << "testREstrictBoundAdd" << WHITE_COLOR << endl;
    bufferReset();
    thread firstBounder(taskBound,2);
    thread firstAdder(taskAdd, 1);
    thread secondBounder(taskBound,3);
    thread thirdBounder(taskBound,5);
    thread secondAdder(taskAdd, 2);
    firstBounder.join();
    secondBounder.join();
    thirdBounder.join();
    firstAdder.join();
    secondAdder.join();
    cout << "Reading log" << endl;
    readLog();
    printbuffer("resulting buffer: ");
    // assert whether buffer bound is 5 or smaller
    if(current_bound < 6){
        cout << "bound restricted succesfully \n";
    }
    //assert whether there are no more elements than restricted by bound
    if((current_idx-1)< current_bound ){
        cout << "items added within bound succesfully \n";
    }
    return 0;
}

int testToggleBounds(){
    cout << "Running: " << YELLOW_COLOR << "testToggleBounds" << WHITE_COLOR << endl;
    bufferReset();
    assertIntEqual(current_bound, UNBOUNDED, "Buffer is unbounded initially");
    bufferReset();
    toggleBounds();
    assertIntEqual(current_bound, UNBOUNDED, "Buffer is unbounded after toggle (no previous)");
    bufferReset();
    setBound(10);
    assertIntEqual(current_bound, 10, "Buffer set to 10");
    toggleBounds();
    assertIntEqual(current_bound, UNBOUNDED, "Buffer correctly toggled back to unbounded");
    toggleBounds();
    assertIntEqual(current_bound, 10, "Buffer correctly toggled to previous value (10)");
    return 0;
}

int runAllTests(){
    addTest();
    removeCompleteTest(); // deadlocks ?
    removeTooManyTest();  // deadlocks ?
    testConcurrentAdding();
    testToggleBounds();
    removeCompleteWithGetTest();
    addAndRemoveTest();   // segmentation fault?
    testChangeBound();
    testChangeBoundAdd();
    testRestrictBoundAdd();
    return 0;
}

