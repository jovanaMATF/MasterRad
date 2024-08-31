#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <queue>
#include <array>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <typeinfo>
#include <chrono>
#include <stack>
#include <thread>
#include <fstream>
#include <functional>

// Write function to split array (vector) of length n into subarrays
// 2^7 = 128 -> k = 8 (16 nizova po 8 elemenata)

#define BUFFER_SIZE_FOR_SEND 2097152/4
#define K 4096/8
#define ARRAY_SIZE 2u

enum class TaskState : uint8_t
{
    not_scheduled = 0,
    scheduled = 1,
    proceed = 2,
    ready_for_schedule = 3

};
enum class Algorithm : uint8_t
{
    accumulate = 0
};


class Task
{
public:
    virtual void execute(int m) = 0;

    virtual std::array<int , ARRAY_SIZE> getData() = 0;

    virtual int getCnt() = 0;

    virtual void setCnt(int cnt) = 0;
    virtual void setState(TaskState s) = 0;

    virtual TaskState getState() = 0;

    virtual void addChilden(Task* child) = 0;

    virtual std::vector<Task*> getChildren() = 0;
    virtual int getMinimal() = 0;
    virtual void setMinimal(int m) = 0;
    virtual void incrementRetCnt() = 0;
    virtual void printContent() const = 0;

    virtual void setIndex(int i) = 0;
    virtual int getIndex() = 0;

    virtual Task* getParent() = 0;

    virtual void setParent(Task* p) = 0;

    virtual void addChildrenIndex(int i) = 0;
    virtual  std::vector<int>  getChildrenIndexes() = 0;

    virtual ~Task() {}
};

class MinimalElementLeafTask : public Task
{
public:
    MinimalElementLeafTask(std::array<int , ARRAY_SIZE> a, int b, int e, TaskState s, Task* p, int rc) : data(a), begin(b), end(e), state(s), parent(p), ref_cnt(rc) {}
    std::array<int , ARRAY_SIZE> getData() override
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

    void setCnt(int cnt)
    {
        this->ref_cnt = cnt;
    }
    TaskState getState()
    {
        return this->state;
    }
    void setState(TaskState s)
    {
        this->state = s;
    }
    Task* getParent()
    {
        return this->parent;
    }
    void setParent(Task* p)
    {
        this->parent = p;
    }
    void setIndex(int i)
    {
        this->index = i;
    }
    int getIndex()
    {
        return this->index;
    }

    void addChildrenIndex(int i)
    {

    }
    std::vector<int>  getChildrenIndexes()
    {
        return {};
    }
    int getMinimal()
    {
        return this->minimal;
    }
    void setMinimal(int m)
    {
        this->minimal = m;
    }
    void execute(int m) override
    {
        for (auto element : data)
        {
            if (element < this->minimal)
            {
                this->minimal = element;
            }
        }
    }
    void printContent() const override {
        std::cout << "LeafTask array segment: [";
        for (int i = begin; i <= end; ++i) {
            std::cout << data[i];
            if (i < end) std::cout << ", ";
        }
        std::cout << "]\n";
    }

private:
    uint64_t result{ 0 };
    int begin;
    int end;
    TaskState state;
    int ref_cnt;
    std::array<int , ARRAY_SIZE> data;
    Task* parent;
    std::vector<Task*> children = {};
    int minimal;
    int index;
    std::vector<int> childrenIndexes = {};
};


class MinimalElementRootTask : public Task
{
public:
    MinimalElementRootTask(std::array<int , ARRAY_SIZE> a, int b, int e, TaskState s, Task* p) : array(a), begin(b), end(e), state(s), parent(p) {}
    std::array<int , ARRAY_SIZE> getData() override
    {
        return this->array;
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
    TaskState getState()
    {
        return this->state;
    }
    void setState(TaskState s)
    {
        this->state = s;
    }
    Task* getParent()
    {
        return this->parent;
    }
    void setParent(Task* p)
    {
        this->parent = p;
    }
    void setIndex(int i)
    {
        this->index = i;
        // for(auto j:this->childrenIndexes)
        //     this->index = j;
    }
    int getIndex()
    {
        return this->index;
    }
    
    int getMinimal()
    {
        return this->minimal;
    }
    void setMinimal(int m)
    {
        this->minimal = m;
    }
    void execute(int m) override
    {
        if (m < this->minimal)
        {
            this->minimal = m;
        }

    }

    void setCnt(int cnt)
    {
        this->ref_cnt = cnt;
    }

    ~MinimalElementRootTask()
    {
        // dealoacate Child tasks -> Free memory
    }
    void printContent() const override {
        std::cout << "RootTask array segment: [";
        for (int i = begin; i <= end; ++i) {
            std::cout << array[i];
            if (i < end) std::cout << ", ";
        }
        std::cout << "]\n";
    }

    std::vector<int>  getChildrenIndexes()
    {
        return this->childrenIndexes;
    }

    void setChildrenIndexes(std::vector<int>  ci)
    {
        this->childrenIndexes = ci;
    }

    void addChildrenIndex(int i)
    {
        this->childrenIndexes.push_back(i);
    }

private:
    uint64_t result{ 0u };
    int begin;
    int end;
    TaskState state;
    int ref_cnt{ 0u };
    std::array<int , ARRAY_SIZE> array;
    Task* parent;
    std::vector<Task*> children = {};
    int index;
    int minimal;
    std::vector<int>  childrenIndexes;
};


// UTILITY FUNCTIONS
// Function to print an array
void printArray(std::array<int ,BUFFER_SIZE_FOR_SEND> A, int size)
{
    for (auto i = 0; i < size; i++)
        std::cout << A[i] << " ";
    std::cout << std::endl;
}


int cnt_leaf = 0;
void formMinimalDAG(std::array<int , ARRAY_SIZE> array, int begin, int end, int k, Task* parent)
{
    int m = (begin + end) / 2;
    std::array<int , ARRAY_SIZE> array1{ 0 };
    std::copy(array.begin(), array.end(), array1.begin());
    std::array<int , ARRAY_SIZE> array2{ 0 };
    std::copy(array.begin(), array.end(), array2.begin());
    if ((end - begin) / 2 <= k)
    {
        Task* leftLeaf = new MinimalElementLeafTask(array1, 0, array1.size(), TaskState::not_scheduled, parent, 0);
        //   leftLeaf->printContent();
      
        Task* rightLeaf = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, parent, 0);
        // rightLeaf->printContent();
        cnt_leaf++;
        cnt_leaf++;
        parent->addChilden(leftLeaf);
        parent->addChilden(rightLeaf);
        parent->incrementRetCnt();
        parent->incrementRetCnt();
        return;
    }
    else
    {
       
        Task* leftRoot = new MinimalElementRootTask(array1, 0, array1.size(), TaskState::not_scheduled, parent);
      
        parent->addChilden(leftRoot);
        parent->incrementRetCnt();
        Task* rightRoot = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, parent);
        parent->addChilden(rightRoot);
        parent->incrementRetCnt();
        parent = leftRoot;
        formMinimalDAG(array, begin,m, k, parent);
        parent = rightRoot;
        formMinimalDAG(array, m, end, k, parent);
        return;

    }

}


Task* formMinimalElementDAG(std::array<int , ARRAY_SIZE> array)
{
    MinimalElementRootTask* root = new MinimalElementRootTask(array, 0, array.size(), TaskState::not_scheduled, nullptr);
    root->execute(3);

    return root;
}

void writeData(Task* t)
{
    std::cout << "[";
    for (int e : t->getData())
        std::cout << e << " ";
    std::cout << "]";

}


struct CompareTask {
    bool operator()(Task* t1, Task* t2) {
        return t1->getCnt() > t2->getCnt(); // Pretpostavljamo da manji "cnt" znači veći prioritet
    }
};

void addGraphToQueue(Task* root, std::priority_queue<Task*, std::vector<Task*>, CompareTask>& task_queue)
{
    if (!root) return;

    std::queue<Task*> bfs_queue;
    bfs_queue.push(root);

    while (!bfs_queue.empty())
    {
        Task* node = bfs_queue.front();
        bfs_queue.pop();
        if (node->getCnt() == 0)
        {
            node->setState(TaskState::ready_for_schedule);
        }
        task_queue.push(node); // Dodajte u min-heap prioritetni red

        for (Task* child : node->getChildren())
        {
            bfs_queue.push(child);
        }
    }
}

class ProcessElement
{
public:
    ProcessElement(std::queue<Task*>q, int i) : process_queue(q), index(i) {}
    ~ProcessElement() {}
    void setIndex(int i)
    {
        this->index = i;
    }
    int getIndex()
    {
        return this->index;
    }

    void addTask(Task* t)
    {
        process_queue.push(t);
    }
    bool findTask(Task* t)
    {
        while (!process_queue.empty())
        {
            if (process_queue.front() == t)
                return true;
            process_queue.pop();
        }
        return false;
    }

    int getSize()
    {
        return process_queue.size();
    }

    std::queue<Task*> getQueue()
    {
        return this->process_queue;
    }

    void writeCnt()
    {
        std::queue<Task*>* copy_queue = &process_queue;

        // traversing on copyqueue
        // until it becomes empty
        while (!copy_queue->empty()) {

            // printing front element of queue
            Task* node = copy_queue->front();
            std::cout << node->getCnt() << " ";

            // deleting element from
            // queue using std :: pop()
            // function
            copy_queue->pop();
        }
    }

private:
    std::queue<Task*> process_queue;
    // koji je procesni element po redu
    int index;

};


bool checkIfTaskIsDone(Task* t)
{
    if (t->getState() == TaskState::proceed)
        return true;
    return false;
}

void sortQueue(std::queue<Task*>* q)
{
    std::vector<Task*> temp = {};
    std::queue<Task*>* copy_queue = q;
    while (!copy_queue->empty())
    {
        temp.push_back(copy_queue->front());
        //    std::cout << copy_queue->front()->getCnt() << " ";
        copy_queue->pop();
    }
    std::sort(temp.begin(), temp.end(), [](Task* a, Task* b) {return a->getCnt() < b->getCnt(); });

    for (int i = 0; i < temp.size(); i++)
    {
        copy_queue->push(temp.at(i));
        //    std::cout << temp.at(i)->getCnt() << " ";
    }
    q = copy_queue;
}


bool allQueuesEmpty(const std::vector<ProcessElement*>& process_elements)
{
    for (auto& element : process_elements) {
        if (element != nullptr && !element->getQueue().empty())
        {
            return false; // Našli smo neprazan red, vraćamo false
        }
    }
    return true; // Svi redovi su prazni, vraćamo true
}


int main()
{
    std::array<int , ARRAY_SIZE> array = {};
    array.fill(0);
    int n, k;


    for (int i = 0; i < array.size(); i++)
    {
        array[i] = i % 255;
    }
    Task* root1 = new MinimalElementRootTask(array, 0, array.size() - 1, TaskState::not_scheduled, nullptr);

    std::cout << "formMinimalElementDAG START" << std::endl;
    //Task *root = formMinimalElementDAG(array);
   
    formMinimalDAG(array, 0, BUFFER_SIZE_FOR_SEND, K, root1);
    std::cout << "formMinimalElementDAG STOP" << std::endl;
    std::queue<Task*> tree_queue2;
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> task_queue_prio;
    addGraphToQueue(root1, task_queue_prio);

    std::cout << task_queue_prio.size();
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> tree_queue3 = task_queue_prio;
    while (!tree_queue3.empty())
    {
        Task* node = tree_queue3.top();
        std::cout << " ref_cnt: " << node->getCnt() << " ";
        // node->execute();
        tree_queue3.pop();

    }
    std::cout << "Processing STOP" << std::endl;

    std::array<uint8_t, BUFFER_SIZE_FOR_SEND> array_serial;
    array_serial.fill(0);
    for (int i = 0; i < array_serial.size(); i++)
    {
        array_serial[i] = 255;
    }
    array_serial[5] = 77;
  
    int r = 108;
    std::ofstream outfile;
    int len = tree_queue2.size() - 1;
    const auto start_m{ std::chrono::steady_clock::now() };
    for (int i = 0; i < cnt_leaf; i++)
    {
        // const auto start_task{ std::chrono::steady_clock::now() };
        for (int m = 0; m < 16; m++)
        {
            //  const auto start_task{ std::chrono::steady_clock::now() };
            for (int j = 0; j < BUFFER_SIZE_FOR_SEND; j++)
            {
                if (r > array_serial[j])
                    r = array_serial[j];

            }
        }
       // const auto end_task{ std::chrono::steady_clock::now() };
       // const std::chrono::duration<long double, std::milli> elapsed_seconds_t{ end_task - start_task };
      //  std::cout << "MAIN execution time in `milliseconds task: " << elapsed_seconds_t.count() << '\n';
    }
    const auto end_m{ std::chrono::steady_clock::now() };
    const std::chrono::duration<long double, std::milli> elapsed_seconds_m{ end_m - start_m };
    std::cout << "MAIN execution time in `milliseconds task: " << elapsed_seconds_m.count() << '\n';
    std::cout << "min: " << r << "\n";
    outfile.open("meassurements_serial_min.txt", std::ios_base::app); // append instead of overwrite
    outfile << "[BUFFER_SIZE_FOR_SEND= " << BUFFER_SIZE_FOR_SEND;
    outfile << " kNumberOfProcessingElements= " << 1;
    outfile << " k= " << K << "]" << std::endl;
    outfile << "MAIN execution time in milliseconds serial: " << elapsed_seconds_m.count() << '\n';
    std::cout << "MAIN execution time in milliseconds serial: " << elapsed_seconds_m.count() << '\n';


    return 0;
}