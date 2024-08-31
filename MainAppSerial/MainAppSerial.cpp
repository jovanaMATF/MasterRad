#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <queue>
#include <array>
#include <numeric>
#include <cstring>
#include <cassert>
#include <chrono>
#include <thread>
#include <stack>
#include <iostream>
#include <fstream>
#include <map>

// Write function to split array (vector) of length n into subarrays
// 2^7 = 128 -> k = 8 (16 nizova po 8 elemenata)
#define BUFFER_SIZE_FOR_SEND 262144
#define ARRAY_SIZE 2u
#define K 256
enum class State : uint8_t
{
    not_scheduled = 0,
    scheduled = 1,
    processed = 2

};
enum class Algorithm : uint8_t
{
    accumulate = 0
};

int grain_size = 0;
class Task
{
public:
    virtual void execute(int m) = 0;

    virtual uint64_t get_result() const = 0;

    virtual std::array<uint8_t, ARRAY_SIZE> getData() = 0;

    virtual int getCnt() = 0;

    virtual void addChilden(Task* child) = 0;

    virtual std::vector<Task*> getChildren() = 0;

    virtual void incrementRetCnt() = 0;

    virtual ~Task() {}
    //  virtual Task* get_parent();

};


class AccumulateRootTask : public Task
{
public:
    AccumulateRootTask(std::array<uint8_t, ARRAY_SIZE> a, int b, int e, State s, Task* p, int rc) : data(a), begin(b), end(e), state(s), parent(p), ref_cnt(rc) {}
    std::array<uint8_t, ARRAY_SIZE> getData() override
    {
        return this->data;
    }
    int getCnt() override
    {
        return this->ref_cnt;
    }
    void addChilden(Task* child) override
    {
        this->children.push_back(child);
        return;
    }
    std::vector<Task*> getChildren() override
    {
        return this->children;
    }
    void incrementRetCnt() override
    {
        this->ref_cnt++;
    }

    void execute(int m) override
    {
        //assert (ref_cnt == 0)
        result += m;
    }

    uint64_t get_result() const override
    {
        return result;
    }


    ~AccumulateRootTask()
    {
        // dealoacate Child tasks -> Free memory
    }

private:
    uint64_t result{ 0u };
    int begin;
    int end;
    State state;
    int ref_cnt;
    std::array<uint8_t, ARRAY_SIZE> data;
    Task* parent;
    std::vector<Task*> children = {};
    Algorithm algorithm = Algorithm::accumulate;
};

class AccumulateLeafTask : public Task
{
public:
    AccumulateLeafTask(std::array<uint8_t, ARRAY_SIZE> a, int b, int e, State s, Task* p, int rc) : data(a), begin(b), end(e), state(s), parent(p), ref_cnt(rc) {}
    std::array<uint8_t, ARRAY_SIZE> getData() override
    {
        return this->data;
    }
    int getCnt() override
    {
        return this->ref_cnt;
    }
    void addChilden(Task* child) override
    {
        this->children.push_back(child);
        return;
    }
    std::vector<Task*> getChildren() override
    {
        return this->children;
    }
    void incrementRetCnt() override
    {
        this->ref_cnt++;
    }
    //  Task* get_parent()
    // {
    //     return this->parent;
    // }

    void execute(int m) override
    {
        uint64_t r = 0;
        for (int i = 0; i < data.size(); i++)
        {
            r += this->data[i];
        }

        result = r;
        this->parent->execute(r);
    }

    uint64_t get_result() const override
    {
        return result;
    }


private:
    uint64_t result{ 0 };
    int begin;
    int end;
    State state;
    int ref_cnt;
    std::array<uint8_t, ARRAY_SIZE> data;
    Task* parent;
    std::vector<Task*> children = {};
    Algorithm algorithm = Algorithm::accumulate;
};

void formAccumulateDAG(std::array<uint8_t, ARRAY_SIZE> a, int begin, int end, int k, Task* parent)
{

    // Izlaz iz rekurzije
    if ((end - begin) <= k)
    {
        std::array<uint8_t, ARRAY_SIZE> subVector;
        subVector.fill(0);
        std::copy(a.begin(), a.end(), subVector.begin());
        Task* accumulated = new AccumulateLeafTask(subVector, 0, subVector.size(), State::not_scheduled, parent, 0);
       
        parent->addChilden(accumulated);
        parent->incrementRetCnt();
        return;
    }
    else
    {
        int m = (begin + end) / 2;
        formAccumulateDAG(a, begin, m, k, parent);
        formAccumulateDAG(a, m, end, k, parent);

        return;
    }

}

void writeData(Task* t)
{
    std::cout << "[";
    for (int e : t->getData())
        std::cout << e << " ";
    std::cout << "]";

}



int main()
{
    std::array<uint8_t, ARRAY_SIZE> array;
    array.fill(1);

    int n, k = K;

    grain_size = k;

    Task* root = new AccumulateRootTask(array, 0, ARRAY_SIZE, State::not_scheduled, nullptr, 0);

    formAccumulateDAG(array, 0, BUFFER_SIZE_FOR_SEND, k, root);

    std::queue<Task*> tree_queue;
    for (Task* child : root->getChildren())
        tree_queue.push(child);

    tree_queue.push(root);
    const auto start_main{ std::chrono::steady_clock::now() };
    std::cout << "size = " << tree_queue.size() << std::endl;
    int len = tree_queue.size() - 1;
    // std::cout << "Processing START" << std::endl;
   /* while (!tree_queue.empty())
    {
        Task* node = tree_queue.front();
        std::cout << " ref_cnt: " << node->getCnt() << " ";
        node->execute(0);
        tree_queue.pop();
        std::cout << " data: ";
        writeData(node);
        std::cout << " data: " << node;
        std::cout << std::endl << " result:" << node->get_result() << std::endl;
        std::cout << "\n";
        std::cout << ".";
    }
    */
    // std::cout << "Processing STOP" << std::endl;

    std::cout << "result = " << root->get_result() << std::endl;
    const auto end_main{ std::chrono::steady_clock::now() };
    const std::chrono::duration<long double, std::milli> elapsed_seconds{ end_main - start_main };
    std::cout << "MAIN execution time in `milliseconds: " << elapsed_seconds.count() << '\n';

    std::cout << std::endl;

    // int * test = new int[len*BUFFER_SIZE_FOR_SEND];
    unsigned int r = 0;
    std::array<uint8_t, BUFFER_SIZE_FOR_SEND> array_serial;
    array_serial.fill(0);
    array_serial[0] = 1;
    const uint8_t* p = array_serial.data();
    std::ofstream outfile;
    const auto start_m {std::chrono::steady_clock::now() };
    for (int i = 0; i < len; i++)
    {
        for (int m = 0; m < 16; m++)
        {
            //  const auto start_task{ std::chrono::steady_clock::now() };
            for (int j = 0; j < BUFFER_SIZE_FOR_SEND; j++)
            {
                //r += p[j];
                r += array_serial[j];
               

            }
        }
    }
        
     //   const auto end_task{ std::chrono::steady_clock::now() };
     //   const std::chrono::duration<long double, std::milli> elapsed_seconds_t{ end_task - start_task };
     //  std::cout << "MAIN execution time in `milliseconds task: " << elapsed_seconds_t.count() << '\n';

    
    const auto end_m{ std::chrono::steady_clock::now() };
    const std::chrono::duration<long double, std::milli> elapsed_seconds_m{ end_m - start_m };
   
    outfile.open("meassurements_serial.txt", std::ios_base::app); // append instead of overwrite
    outfile << "[BUFFER_SIZE_FOR_SEND= " << BUFFER_SIZE_FOR_SEND;
    outfile << " kNumberOfProcessingElements= " << 1;
    outfile << " k= " << k << "]" << std::endl;
    outfile << "MAIN execution time in milliseconds serial: " << elapsed_seconds_m.count() << '\n';
    std::cout << "MAIN execution time in milliseconds serial: " << elapsed_seconds_m.count() << '\n';
    return 0;
}