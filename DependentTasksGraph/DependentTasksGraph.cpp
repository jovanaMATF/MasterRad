#define WIN32_LEAN_AND_MEAN

#include "ProcessingElementHandler.h"

#include <iostream>
#include <fstream>
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

#define BUFFER_SIZE_FOR_SEND 2097152
#define K 512
#define ARRAY_SIZE 2u

namespace
{
    constexpr char kNumberOfProcessingElements{ 16u };
    constexpr unsigned int kDefaultBufferLen{ BUFFER_SIZE_FOR_SEND };
}; // end of anonymous namespace


constexpr unsigned int RCV_BUFFER = 1u;

enum class TaskState : uint8_t
{
    not_scheduled = 0,
    scheduled = 1,
    ready_for_schedule = 2,
    processed = 3

};
enum class Algorithm : uint8_t
{
    accumulate = 0
};

enum class TaskType : uint8_t
{
    leaf = 0,
    root = 1
};


class Task
{
public:
    virtual void execute() = 0;

    virtual std::array<int, ARRAY_SIZE> getData() = 0;

    virtual int getCnt() = 0;

    virtual void setCnt(int cnt) = 0;
    
    virtual void setState(TaskState s) = 0;

    virtual TaskState getState() = 0;

    virtual void addChilden(Task* child) = 0;

    virtual std::vector<Task*> getChildren() = 0;
    
    virtual unsigned int getMinimal() = 0;
    
    virtual void setMinimal(unsigned int m) = 0;
    
    virtual void incrementRetCnt() = 0;
    
    virtual void printContent() const = 0;

    virtual void setIndex(int i) = 0;
    
    virtual int getIndex() = 0;

    virtual Task* getParent() = 0;

    virtual void setParent(Task* p) = 0;

    virtual void addChildrenIndex(int i) = 0;
    
    virtual  std::vector<int>  getChildrenIndexes() = 0;

    virtual void setType(TaskType s) = 0;

    virtual TaskType getType() = 0;

    virtual int getBegin() = 0;
    
    virtual int getEnd() = 0;
    
    virtual void setBegin(int b) = 0;
    
    virtual void setEnd(int e) = 0;
    
    virtual ~Task() {}
};

class MinimalElementLeafTask : public Task
{
public:
    MinimalElementLeafTask(std::array<int, ARRAY_SIZE> a, int b, int e, TaskState s, TaskType t, Task* p, int rc) : data(a), begin(b), end(e), state(s), type(t), parent(p), ref_cnt(rc) {}
    std::array<int, ARRAY_SIZE> getData() override
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
    int getBegin()
    {
        return this->begin;
    }
    int getEnd()
    {
        return this->end;
    }
    void setBegin(int b)
    {
        this->begin = b;
    }
    void setEnd(int e)
    {
        this->end = e;
    }
    void addChildrenIndex(int i)
    {

    }
    std::vector<int>  getChildrenIndexes()
    {
        return {};
    }
    unsigned int getMinimal()
    {
        return this->minimal;
    }
    void setMinimal(unsigned int m)
    {
        this->minimal = m;
    }
    void execute() override
    {
        
    }
    void setType(TaskType t)
    {
        this->type = t;
    }
    TaskType getType()
    {
        return this->type;
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
    TaskType type;
    int ref_cnt;
    std::array<int, ARRAY_SIZE> data;
    Task* parent;
    std::vector<Task*> children = {};
    unsigned int minimal;
    int index;
    std::vector<int> childrenIndexes = {};
};


class MinimalElementRootTask : public Task
{
public:
    MinimalElementRootTask(std::array<int, ARRAY_SIZE> a, int b, int e, TaskState s, TaskType t, Task* p) : array(a), begin(b), end(e), state(s), type(t), parent(p) {}
    std::array<int, ARRAY_SIZE> getData() override
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
    void setType(TaskType t)
    {
        this->type = t;
    }
    TaskType getType()
    {
        return this->type;
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
    int getBegin()
    {
        return this->begin;
    }
    int getEnd()
    {
        return this->end;
    }
    void setBegin(int b)
    {
        this->begin = b;
    }
    void setEnd(int e)
    {
        this->end = e;
    }
    unsigned int getMinimal()
    {
        return this->minimal;
    }
    void setMinimal(unsigned int m)
    {
        this->minimal = m;
    }
    void execute() override
    {

    }
    void setCnt(int cnt)
    {
        this->ref_cnt = cnt;
    }

    ~MinimalElementRootTask()
    {
        // dealoacate Child tasks -> Free memory
        for (auto child : this->getChildren())
        {
            free(child);
        }
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
    TaskType type;
    int ref_cnt{ 0u };
    std::array<int, ARRAY_SIZE> array;
    Task* parent;
    std::vector<Task*> children = {};
    int index;
    unsigned int minimal;
    std::vector<int>  childrenIndexes;
};


// UTILITY FUNCTIONS
// Function to print an array
void printArray(std::array<int, ARRAY_SIZE> A, int size)
{
    for (auto i = 0; i < size; i++)
        std::cout << A[i] << " ";
    std::cout << std::endl;
}


void formDependDAG(std::array<int, ARRAY_SIZE> array, int begin, int end, int k, Task* parent)
{
    std::array<int, ARRAY_SIZE> array1{ 0 };
    std::copy(array.begin(), array.end(), array1.begin());
    std::array<int, ARRAY_SIZE> array2{ 0 };
    std::copy(array.begin(), array.end(), array2.begin());
    std::array<int, ARRAY_SIZE> array3{ 0 };
    std::copy(array.begin(), array.end(), array3.begin());

    // first graph root child
    Task* root_1 = new MinimalElementRootTask(array1, 0, array1.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_1->setType(TaskType::root);
    parent->addChilden(root_1);
    parent->incrementRetCnt();
    // second graph root child
    Task* root_2 = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_2->setType(TaskType::root);
    parent->addChilden(root_2);
    parent->incrementRetCnt(); 
    // third graph root child
    Task* root_3 = new MinimalElementRootTask(array3, 0, array3.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_2->setType(TaskType::root);
    parent->addChilden(root_3);
    parent->incrementRetCnt();


    // first root child graph part
    parent = root_1;
    Task* root_4 = new MinimalElementRootTask(array1, 0, array1.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_4->setType(TaskType::root);
    parent->addChilden(root_4);
    parent->incrementRetCnt();

    parent = root_4;
    Task* root_5= new MinimalElementRootTask(array1, 0, array1.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_5->setType(TaskType::root);
    parent->addChilden(root_5);
    parent->incrementRetCnt();

    
    // second root child graph part
    parent = root_2;
    Task* root_6 = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_6->setType(TaskType::root);
    parent->addChilden(root_6);
    parent->incrementRetCnt();
    Task* leaf_7 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_7->setType(TaskType::leaf);
    parent->addChilden(leaf_7);
    parent->incrementRetCnt();
    Task* leaf_8 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_8->setType(TaskType::leaf);
    parent->addChilden(leaf_8);
    parent->incrementRetCnt();
    Task* leaf_9 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_9->setType(TaskType::leaf);
    parent->addChilden(leaf_9);
    parent->incrementRetCnt();

    parent = root_6;
    Task* leaf_10 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_10->setType(TaskType::leaf);
    parent->addChilden(leaf_10);
    parent->incrementRetCnt();


    // third root child graph part
    parent = root_3;
    Task* root_11 = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_11->setType(TaskType::root);
    parent->addChilden(root_11);
    parent->incrementRetCnt();

    parent = root_11;
    Task* root_12 = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::root, parent);
    root_12->setType(TaskType::root);
    parent->addChilden(root_12);
    parent->incrementRetCnt();
    Task* leaf_13 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_13->setType(TaskType::leaf);
    parent->addChilden(leaf_13);
    parent->incrementRetCnt();
    Task* leaf_14 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_14->setType(TaskType::leaf);
    parent->addChilden(leaf_14);
    parent->incrementRetCnt();

    parent = root_12;
    Task* root_15 = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent);
    root_15->setType(TaskType::leaf);
    parent->addChilden(root_15);
    parent->incrementRetCnt();
    Task* leaf_16 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_16->setType(TaskType::leaf);
    parent->addChilden(leaf_16);
    parent->incrementRetCnt();

    parent = root_15;
    Task* leaf_17 = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
    leaf_17->setType(TaskType::leaf);
    parent->addChilden(leaf_17);
    parent->incrementRetCnt();
   
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
    ProcessElement(int i) :
        index(i),
        m_pe_id{ static_cast<ProcessingElementIdentification>(i) }
    {}

    ~ProcessElement() {}
    void setIndex(int i)
    {
        this->index = i;
    }
    int getIndex()
    {
        return this->index;
    }

    const ProcessingElementIdentification& GetProcessingElementIdentification()
    {
        return m_pe_id;
    }

    void addTask(Task* t)
    {
        this->process_queue.push(t);
    }

    bool findTask(Task* t)
    {
        while (!process_queue.empty())
        {
            if (process_queue.top() == t)
                return true;
            process_queue.pop();
        }
        return false;
    }

    int getSize()
    {
        return process_queue.size();
    }

    void writeCnt()
    {
        std::priority_queue<Task*, std::vector<Task*>, CompareTask> copy_queue = process_queue;

        // traversing on copyqueue
        // until it becomes empty
        while (!copy_queue.empty()) {

            // printing front element of queue
            Task* node = copy_queue.top();

            // deleting element from
            // queue using std :: pop()
            // function
            copy_queue.pop();
        }

    }

    std::priority_queue<Task*, std::vector<Task*>, CompareTask>& getQueue()
    {
        return this->process_queue;
    }
    std::chrono::steady_clock::time_point getStartTime()
    {
        return this->start_time;
    }
    std::chrono::steady_clock::time_point getEndTime()
    {
        return this->end_time;
    }
    void setStartTime(std::chrono::steady_clock::time_point s)
    {
        this->start_time = s;
    }
    void setEndTime(std::chrono::steady_clock::time_point s)
    {
        this->end_time = s;
    }
    std::chrono::duration<long double, std::milli> getExeTime()
    {
        return this->execution_time;
    }
    void setExeTime(std::chrono::duration<long double, std::milli> s)
    {
        this->execution_time = s;
    }
private:
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> process_queue{};
    // koji je procesni element po redu
    int index;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::chrono::duration<long double, std::milli> execution_time{ 0.0 };
    ProcessingElementIdentification m_pe_id = static_cast<ProcessingElementIdentification>(99);
};


void sortQueue(std::priority_queue<Task*, std::vector<Task*>, CompareTask>* q)
{
    std::vector<Task*> temp = {};
    std::priority_queue<Task*, std::vector<Task*>, CompareTask>* copy_queue = q;
    while (!copy_queue->empty())
    {
        temp.push_back(copy_queue->top());
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

void scheduleTasks(Task* root, std::priority_queue<Task*, std::vector<Task*>, CompareTask> queue, int n, ProcessingElementHandler<kNumberOfProcessingElements, RCV_BUFFER>& peHandler)
{
    // vektor redova za procesne elemente
    static int index = 0;
    static std::vector<ProcessElement*> process_elements(n);
    std::array<char, 4> rcv_buffer;
    rcv_buffer.fill(0);
    // ulazni queue rasporedi u vektor procesesnih elemenata
    int i = 0;

    for (int i = 0; i < kNumberOfProcessingElements; ++i)
    {
        process_elements[i] = new ProcessElement(i);
    }

    do
    {
        Task* node = queue.top();
        process_elements[i]->addTask(node);
        //  std::cout << "type: " << (int)node->getType() << "\n";
        i = (i + 1) % kNumberOfProcessingElements;
        queue.pop();
    } while (!queue.empty());

    int length_pe = process_elements.size();
    //std::cout << "len: " << length_pe << "\n";
#if 0 
    for (int i = 0; i < n; i++)
    {
        std::cout << i << " ";
        process_elements[i]->writeCnt();
    }
#endif

    char data_for_send[3] = { 0,2,2 };
    for (int i = 0; i < kNumberOfProcessingElements; ++i)
    {
        const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(i);
        std::priority_queue<Task*, std::vector<Task*>, CompareTask>& process_q = process_elements[i]->getQueue();

        if (!process_q.empty())
        {
            Task* node = process_q.top();
            const auto& taskRefCnt{ node->getCnt() };
            const auto& taskState{ node->getState() };
            const auto& taskType{ node->getType() };
          
            if (TaskState::ready_for_schedule == taskState)
            {
                assert(0 == taskRefCnt);
                data_for_send[0] = (char)taskType;
                peHandler.SendDataToProcessingElement(pe_id, (char*)data_for_send, sizeof(char) * 3);
                node->setState(TaskState::scheduled);
            }
        }
    }

    do
    {
        // Proveri podatke sa PEs i obradi ih
        if (true == peHandler.IsResponseFromAnyProcessingElementsAvailable())
        {
            // Prodji kroz svaki red i ako je task gotov obradi ga
            for (int j = 0; j < kNumberOfProcessingElements; ++j)
            {
                const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(j);
                std::priority_queue<Task*, std::vector<Task*>, CompareTask>& process_q = process_elements[j]->getQueue();
              
                // Obradi task samo ako red nije prazan
                if (!process_q.empty())
                {

                    // Obradi task samo ako su podaci primljeni 
                    if (true == peHandler.IsResponseFromProcessingElementAvailable(pe_id))
                    {
                        // Procitaj podtake
                        peHandler.ReadDataFromProcessingElement(pe_id, rcv_buffer.data(), sizeof(char));

                        // Process received data.
                        Task* node = process_q.top();
                        //  node->getParent()->execute((uint64_t)rcv_buffer[0]);

                        // Azuriraj stanje trenutnog taska
                        node->setState(TaskState::processed);

                        // Azuriraj ref_cnt i state roditelja  
                        Task* p = node->getParent();
                        if (p != nullptr)
                        {
                            int c = p->getCnt();
                            c--;
                            p->setCnt(c);
                            if (0 == c && TaskState::not_scheduled == p->getState())
                            {
                                p->setState(TaskState::ready_for_schedule);
                            }
                        }

                        process_q.pop();

                        //  std::cout << "TASK processed!" << std::endl;
                    }
                }
            }
        }

         // Rasporedi taskove sa redova ciji je front == ref_cnt == 0 (ready for sechdule)
        for (int j = 0; j < kNumberOfProcessingElements; ++j)
        {

            const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(j);
            std::priority_queue<Task*, std::vector<Task*>, CompareTask>& process_q = process_elements[j]->getQueue();

            const auto start_m{ std::chrono::steady_clock::now() };
            //sortQueue(&process_q);
            const auto end_m{ std::chrono::steady_clock::now() };
            const std::chrono::duration<long double, std::milli> elapsed_seconds_s{ end_m - start_m };
            
            if (!process_q.empty())
            {

                Task* node = process_q.top();
                const auto& taskRefCnt{ node->getCnt() };
                const auto& taskState{ node->getState() };
                const auto& taskType{ node->getType() };
                
                if (TaskState::ready_for_schedule == taskState)
                {
                   
                    if (node == root)
                    {
                        std::cout << "RETURN!!" << "\n";
                        data_for_send[0] = (char)taskType;
                        peHandler.SendDataToProcessingElement(pe_id, (char*)data_for_send, sizeof(char) * 3);

                        node->setState(TaskState::scheduled);
                        return;

                    }
                    else
                    {
                        assert(0 == taskRefCnt);

                        data_for_send[0] = (char)taskType;
                       
                        peHandler.SendDataToProcessingElement(pe_id, (char*)data_for_send, sizeof(char) * 3);
                       
                        node->setState(TaskState::scheduled);

                    }

                }
            }
        }

    } while (TaskState::scheduled != root->getState());
    std::cout << "DONE!" << std::endl;
}

int main()
{
    // Get PEs to Network Adress mapping 
    const std::map<ProcessingElementIdentification, NetworkAddress>& processingElementToNetworkMapping = GetProcessingElementToNetworkMapping();
    ProcessingElementHandler<kNumberOfProcessingElements, RCV_BUFFER> peHandler(processingElementToNetworkMapping);
    // Connect main app to PE servers
    peHandler.ConnectToProcessingElements();

    std::array<int, ARRAY_SIZE> array = {};
    array.fill(0);

    for (int i = 0; i < array.size(); i++)
    {
        array[i] = (i + 1) % 255;
    }
    Task* root = new MinimalElementRootTask(array, 0, array.size(), TaskState::not_scheduled, TaskType::root, nullptr);

    std::cout << "formDependDAG START" << std::endl;
    formDependDAG(array, 0, BUFFER_SIZE_FOR_SEND, K, root);
    std::cout << "formDependDAG STOP" << std::endl;

    std::priority_queue<Task*, std::vector<Task*>, CompareTask> tree_queue2;
    addGraphToQueue(root, tree_queue2);
    std::cout << "queue size: " << tree_queue2.size() << "\n";
  
    const auto start_main{ std::chrono::steady_clock::now() };
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> tree_queue3 = tree_queue2;
   /* while (!tree_queue3.empty())
   {
        Task* node = tree_queue3.top();
 
          node->execute();
          tree_queue3.pop();
          std::cout << " data: ";
          writeData(node);
          std::cout << node->getChildren().size();
          std::cout << " data: " << node;
          std::cout << std::endl << " result:" << node->get_result() << std::endl;
          std::cout << "\n";
          std::cout << ".";
   }
   */
    

    scheduleTasks(root, tree_queue2, kNumberOfProcessingElements, peHandler);

    const auto end_main{ std::chrono::steady_clock::now() };
    const std::chrono::duration<long double, std::milli> elapsed_seconds{ end_main - start_main };
    std::ofstream outfile;

    outfile << "MAIN execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    std::cout << "MAIN execution time in milliseconds : " << elapsed_seconds.count() << '\n';
    return 0;
}