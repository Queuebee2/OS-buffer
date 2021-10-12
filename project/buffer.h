
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

// #define DEBUGMODE 0
// #define DEBUGs(x) do { if (DEBUGMODE>0) { std::cerr << x << std::endl; } } while (0)
// #define DEBUG(x)  do { if (DEBUGMODE>1) { std::cerr << "debug:: " << x << std::endl; } } while (0)
// #define DEBUG2(x) do { if (DEBUGMODE>1) { std::cerr << #x << ": " << x << std::endl; } } while (0)

const int UNBOUNDED = -1;
const int DEFAULT_BOUND = UNBOUNDED;

int current_bound = DEFAULT_BOUND; // initialise as unbounded buffer
int prev_bound    = DEFAULT_BOUND; // initialise
int current_idx   =  0;            // current index is where we can write the next element

mutex buffer_lock;
// - my_buffer vector
// - current_bound
// - prev_bound

// mutex bound_lock -> merged with bound_lock
mutex log_lock;
mutex current_idx_lock;
vector<int> my_buffer;
vector<string> buffer_log;


void bufferReset(){
    current_bound = DEFAULT_BOUND;
    prev_bound    = DEFAULT_BOUND; 
    current_idx   =  0;

    buffer_lock.unlock();
    log_lock.unlock();
    current_idx_lock.unlock();

    my_buffer.clear();
    buffer_log.clear();
}

void writeLog(string msg, int position){
    // TODO READER WRITER PROBLEM.
    // waiting on log_lock
    log_lock.lock();
    buffer_log[position]= msg;
    log_lock.unlock();
}

void readLog(){
    
    log_lock.lock();
    
    for (int i = 0; i < buffer_log.size() ; i ++)
    {
        cout << "log for operation " << i << " " << buffer_log.at(i) << endl;
    }
    log_lock.unlock();
    cout << ' ';
    
}


void bufferAdd(int addition)
{
    string msg;

    current_idx_lock.lock();
    buffer_lock.lock();

    // get the msg index for current operation
    log_lock.lock();
    int msg_ind = buffer_log.size();
    buffer_log.push_back("");
    log_lock.unlock();

     // * * CRITICAL SECTION BUFFER, CURRENT_IDX * *
    if (current_bound == UNBOUNDED || current_idx < current_bound) 
    {
        current_idx++;
        // * * END CRITICAL SECTION CURRENT_IDX * *
        current_idx_lock.unlock();

        my_buffer.push_back(addition);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();

        msg = "added element " + to_string(addition) + " to buffer succesfully";
    } else 
    {
        // * * END CRITICAL SECTION BUFFER, CURRENT_IDX * *
        current_idx_lock.unlock();
        buffer_lock.unlock();
        msg = "failed adding element " + to_string(addition) +". Buffer out of bound";
    }

    writeLog(msg, msg_ind);
}

// TODO: int bufferGet()... 

int bufferReturnRemoved(){
    string msg;
    int removed_element;

    current_idx_lock.lock();
    // get the msg index for current operation
    log_lock.lock();
    int msg_ind = buffer_log.size();
    buffer_log.push_back("");
    log_lock.lock();
    // * * CRITICAL SECTION CURRENT_IDX * *
    if(current_idx > 0)
    { 
        current_idx--;
        // * * END CRITICAL SECTION CURRENT_IDX * *
        current_idx_lock.unlock();
   
        buffer_lock.lock();
        // * * CRITICAL SECTION BUFFER * *
        removed_element = my_buffer.back(); 
        my_buffer.pop_back();
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();

        msg = "got and removed element " + to_string(removed_element) + " from buffer succesfully";
    }
    else
    {
        // * * END CRITICAL SECTION CURRENT_IDX * *
        current_idx_lock.unlock();
        msg = "failed getting element. Buffer empty. returned NULL";
        removed_element = NULL; // RETURN if no value?
    }
    
    writeLog(msg,msg_ind);
    return removed_element;

}

void bufferRemove(){
    string msg;

    current_idx_lock.lock();
    // get the msg index for current operation
    log_lock.lock();
    int msg_ind = buffer_log.size();
    buffer_log.push_back("");
    log_lock.lock();
    // * * CRITICAL SECTION CURRENT_IDX * *
    if(current_idx > 0)
    { 
        current_idx--;
        // * * END CRITICAL SECTION CURRENT_IDX * *
        current_idx_lock.unlock();
   
        buffer_lock.lock();

         
        // * * CRITICAL SECTION BUFFER * *
        int removed_element = my_buffer.back(); 
        my_buffer.pop_back();
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();

        msg = "removed element " + to_string(removed_element) + " from buffer succesfully";
    }
    else
    {
        // * * END CRITICAL SECTION CURRENT_IDX * *
        current_idx_lock.unlock();
        msg = "failed removing element. Buffer empty";
    }
    
    writeLog(msg,msg_ind);

}


void setBound(int requestSize)
{   string msg;
    int msg_ind;

    if(requestSize > UNBOUNDED)
    { 

        // * * CRITICAL SECTION CURRENT_IDX * *  
        current_idx_lock.lock();
        if (requestSize < current_idx)
        {
            current_idx = requestSize;
        }
        current_idx_lock.unlock();
        // * * END CRITICAL SECTION CURRENT_IDX * *  
        
        // * *  CRITICAL SECTION BUFFER * *  
        buffer_lock.lock();
        // reserve sport in log
         // get the msg index for current operation
         log_lock.lock();
         msg_ind = buffer_log.size();
         buffer_log.push_back("");
         log_lock.unlock();
        int changed_bound = current_bound;     
        current_bound = requestSize;
        // * * END CRITICAL SECTION BUFFER * *  

        buffer_lock.unlock();
        msg = "set bound from " + to_string(changed_bound) + " to " + to_string(requestSize) + " succesfully";
    }
    
    else
    {   
        // * * CRITICAL SECTION BOUND * *  
        buffer_lock.lock(); 
         // get the msg index for current operation
         log_lock.lock();
         msg_ind = buffer_log.size();
         buffer_log.push_back("");
         log_lock.unlock();
        current_bound = requestSize;
        // * * END CRITICAL SECTION BOUND * *  
        buffer_lock.unlock();
        msg = "made buffer unbounded successfully ";
    }


    writeLog(msg, msg_ind);
}


/*
Toggles the boundedness of the buffer. Changes the bound back
to a previous bound value if the bound is already infinite.
*/
void toggleBounds()
{
    string msg;

    buffer_lock.lock();
    // get the msg index for current operation
    log_lock.lock();
    int msg_ind = buffer_log.size();
    buffer_log.push_back("");
    log_lock.unlock();
    
    // * * CRITICAL SECTION BUFFER * *  
    if (current_bound == UNBOUNDED)
    {
        current_bound = prev_bound;
        // * * END CRITICAL SECTION BUFFER * *  
        buffer_lock.unlock();
        msg = "toggled to bounded buffer (" + to_string(prev_bound) + " elements)";
    } else 
    {
        prev_bound = current_bound;
        current_bound = UNBOUNDED;
        // * * END CRITICAL SECTION BUFFER * *  
        buffer_lock.unlock();
        msg = "toggled to unbounded buffer";
    }
    writeLog(msg, msg_ind);
}
