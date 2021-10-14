
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
int prev_bound = DEFAULT_BOUND;    // initialise
int current_idx = 0;               // current index is where we can write the next element

mutex buffer_lock;
// - my_buffer vector
// - current_bound
// - prev_bound

// mutex bound_lock -> merged with bound_lock
mutex log_lock;
mutex current_idx_lock;
vector<int> my_buffer;
vector<string> buffer_log;

int getBufferSize()
{
    buffer_lock.lock();
    int size = my_buffer.size();
    buffer_lock.unlock();
    return size;
}

int getLogSize()
{
    log_lock.lock();
    int size = buffer_log.size();
    log_lock.unlock();
    return size;
}

int getCurrentBound()
{
    buffer_lock.lock();
    int bound = current_bound;
    buffer_lock.unlock();
    return bound;
}

int getCurrentIndex()
{
    current_idx_lock.lock();
    int idx = current_idx;
    current_idx_lock.unlock();
    return idx;
}

int getPrevBound()
{
    buffer_lock.lock();
    int bound = prev_bound;
    buffer_lock.unlock();
    return bound;
}

int getValueAt(int ind)
{
    int res;
    buffer_lock.lock();
    if (ind >= my_buffer.size())
    {
        res = NULL;
    }
    else
    {
        res = my_buffer.at(ind);
    }
    buffer_lock.unlock();
    return res;
}

string readLogAt(int ind)
{
    string res;
    log_lock.lock();
    if (ind >= buffer_log.size())
    {
        res = "No log at " + to_string(ind);
    }
    else
    {
        res = buffer_log.at(ind);
    }
    log_lock.unlock();
    return res;
}

void bufferReset()
{
    current_bound = DEFAULT_BOUND;
    prev_bound = DEFAULT_BOUND;
    current_idx = 0;

    // TODO ? LOCK things? how to
    // be sure this is never called within a thread while
    // other threads are trying to compete for resoures?
    buffer_lock.unlock();
    log_lock.unlock();
    current_idx_lock.unlock();

    my_buffer.clear();

    // TODO ? add something to buffer_log?
    buffer_log.clear();
}

void writeLog(string msg)
{
    log_lock.lock();
    // * *  CRITICAL SECTION LOG **
    buffer_log.push_back(msg);
    // * * END CRITICAL SECTION LOG **
    log_lock.unlock();
}

void readLog()
{
    log_lock.lock();
    // * * CRITICAL SECTION LOG **
    for (int i = 0; i < buffer_log.size(); i++)
    {
        cout << "log for operation " << i << " " << buffer_log.at(i) << endl;
    }
    // * * END CRITICAL SECTION LOG **
    log_lock.unlock();
}

void bufferAdd(int addition)
{
    buffer_lock.lock();
    // * * CRITICAL SECTION BUFFER* *
    if (current_bound == UNBOUNDED || my_buffer.size() < current_bound)
    {
        my_buffer.push_back(addition);
        string msg = "added element " + to_string(addition) + " to buffer succesfully";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
    else
    {
        string msg = "failed adding element " + to_string(addition) + ". Buffer out of bound";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER **
        buffer_lock.unlock();
    }
}

int bufferReturnRemoved()
{
    buffer_lock.lock();
    // * * CRITICAL SECTION BUFFER * *
    int removed_element;
    if (my_buffer.size() > 0)
    {
        removed_element = my_buffer.back();
        my_buffer.pop_back();
        string msg = "got and removed element " + to_string(removed_element) + " from buffer succesfully";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
    else
    {
        string msg = "failed getting element. Buffer empty. returned NULL";
        removed_element = NULL; // RETURN if no value
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
    return removed_element;
}

void bufferRemove()
{
    buffer_lock.lock();
    // * * CRITICAL SECTION BUFFER * *
    if (my_buffer.size() > 0)
    {
        int removed_element = my_buffer.back();
        my_buffer.pop_back();
        string msg = "removed element " + to_string(removed_element) + " from buffer succesfully";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
    else
    {
        string msg = "failed removing element. Buffer empty";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
}

void setBound(int requestSize)
{
    string msg;
    // * * CRITICAL SECTION BUFFER * *
    buffer_lock.lock();
    if (requestSize > UNBOUNDED)
    {
        if (requestSize < my_buffer.size())
        {
            int changed_bound = current_bound;
            my_buffer.resize(requestSize);
            current_bound = requestSize;
            string msg = "set bound from " + to_string(changed_bound) + " to " + to_string(requestSize) + " succesfully";
            writeLog(msg);
            // * * END CRITICAL SECTION BUFFER * *
            buffer_lock.unlock();
        }
        else
        {
            int changed_bound = current_bound;
            current_bound = requestSize;
            string msg = "set bound from " + to_string(changed_bound) + " to " + to_string(requestSize) + " succesfully";
            writeLog(msg);
            // * *  END CRITICAL SECTION BUFFER * *
            buffer_lock.unlock();
        }
    }

    else
    {
        current_bound = requestSize;
        string msg = "made buffer unbounded successfully ";
        writeLog(msg);
        // * * END CRITICAL SECTION BOUND * *
        buffer_lock.unlock();
    }
}

/*
Toggles the boundedness of the buffer. Changes the bound back
to a previous bound value if the bound is already infinite.
*/
void toggleBounds()
{
    buffer_lock.lock(); 
    // * * CRITICAL SECTION BUFFER * *
    if (current_bound== UNBOUNDED)
    {
        current_bound = prev_bound;
        string msg = "toggled to bounded buffer (" + to_string(prev_bound) + " elements)";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
    else
    {
        prev_bound = current_bound;
        current_bound = UNBOUNDED;
        string msg = "toggled to unbounded buffer";
        writeLog(msg);
        // * * END CRITICAL SECTION BUFFER * *
        buffer_lock.unlock();
    }
}
