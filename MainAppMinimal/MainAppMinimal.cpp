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

// Write function to split array (vector) of length n into subarrays
// 2^7 = 128 -> k = 8 (16 nizova po 8 elemenata)


#define BUFFER_SIZE_FOR_SEND 2097152/4
#define K 1024/2
#define ARRAY_SIZE 2u


namespace
{
    constexpr char kNumberOfProcessingElements{ 2u };
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
    virtual void execute(int m) = 0;

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
    void setType(TaskType t)
    {
        this->type = t;
    }

    virtual TaskType getType()
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
    MinimalElementRootTask(std::array<int, ARRAY_SIZE> a, int b, int e, TaskState s, TaskType t,Task* p) : array(a), begin(b), end(e), state(s), type(t), parent(p) {}
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
    virtual TaskType getType()
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
        // for(auto j:this->childrenIndexes)
        //     this->index = j;
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



void formMinimalDAG(std::array<int, ARRAY_SIZE> array, int begin, int end, int k, Task* parent)
{
    int m = begin + (end - begin) / 2;
    std::array<int, ARRAY_SIZE> array1{ 0 };
    std::copy(array.begin(), array.end(), array1.begin());
    std::array<int, ARRAY_SIZE> array2{ 0 };
    std::copy(array.begin(), array.end(), array2.begin());
    if ((end - begin) / 2 <= k)
    {
        Task* leftLeaf = new MinimalElementLeafTask(array1, 0, array1.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
        //   leftLeaf->printContent();

        Task* rightLeaf = new MinimalElementLeafTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
        // rightLeaf->printContent();

        parent->addChilden(leftLeaf);
        parent->addChilden(rightLeaf);
        parent->incrementRetCnt();
        parent->incrementRetCnt();
        return;
    }
    else
    {

        Task* leftRoot = new MinimalElementRootTask(array1, 0, array1.size(), TaskState::not_scheduled, TaskType::root, parent);
        leftRoot->setType(TaskType::root);
        parent->addChilden(leftRoot);
        parent->incrementRetCnt();
        Task* rightRoot = new MinimalElementRootTask(array2, 0, array2.size(), TaskState::not_scheduled, TaskType::root, parent);
        rightRoot->setType(TaskType::root);
        parent->addChilden(rightRoot);
        parent->incrementRetCnt();
        parent = leftRoot;
        formMinimalDAG(array, begin, m, k, parent);
        parent = rightRoot;
        formMinimalDAG(array, m, end, k, parent);
        return;

    }

}



Task* formMinimalElementDAG(std::array<int, ARRAY_SIZE> array)
{
    MinimalElementRootTask* root = new MinimalElementRootTask(array, 0, array.size() - 1, TaskState::not_scheduled, TaskType::root, nullptr);

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

#if 0
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
#endif

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
    std::priority_queue<Task*, std::vector<Task*>, CompareTask>*copy_queue = q;
    while (!copy_queue->empty())
    {
        temp.push_back(copy_queue->top());
        copy_queue->pop();
    }
    std::sort(temp.begin(), temp.end(), [](Task* a, Task* b) {return a->getCnt() < b->getCnt(); });

    for (int i = 0; i < temp.size(); i++)
    {
        copy_queue->push(temp.at(i));
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
    // ulazni queue rasporedi u vektor procesenih elemenata
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

    char data_for_send[3] = {0,2,2};
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
            const auto& taskBegin{ node->getBegin() };
            const auto& taskEnd{ node->getEnd() };
            
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
                // Obradi task samo ako je red pun
                if (!process_q.empty())
                {

                    // Obradi task samo ako su podaci primljeni 
                    if (true == peHandler.IsResponseFromProcessingElementAvailable(pe_id))
                    {
                        // Procitaj podtake
                        peHandler.ReadDataFromProcessingElement(pe_id, rcv_buffer.data(), sizeof(char));

                        // Process received data
                        // Dodaj medjurezultat na roditelja
                        // note -> Get perent -> execute...

                        Task* node = process_q.top();
                        node->setMinimal(static_cast<unsigned int>(rcv_buffer[0]));
                        // Azuriraj stanje trenutnog taska
                        node->setState(TaskState::processed); 

                        // Azuriraj ref_cnt i state roditelja  
                        Task* p = node->getParent();
                        if (p != nullptr)
                        {
                            int c = p->getCnt();
                            c--;
                            p->setCnt(c);
                            // update status roditelja ako je spreman za obradu
                            if (0 == c && TaskState::not_scheduled == p->getState())
                            {
                                p->setState(TaskState::ready_for_schedule);
                            }
                          //   std::cout << "PARENT state: " << (int)p->getState() << "\n";
                        }

                        process_q.pop();

                        //  std::cout << "TASK processed!" << std::endl;
                    }
                }
            }
        }
        // std::cout << "#";

         // Rasporedi taskove sa redova ciji je front == ref_cnt == 0 (ready for sechdule)
        for (int j = 0; j < kNumberOfProcessingElements; ++j)
        {

            const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(j);
            std::priority_queue<Task*, std::vector<Task*>, CompareTask>& process_q = process_elements[j]->getQueue();

           // sortQueue(&process_q);

            if (!process_q.empty())
            {

                Task* node = process_q.top();
                const auto& taskRefCnt{ node->getCnt() };
                const auto& taskState{ node->getState() };
                const auto& taskType{ node->getType() };
                const auto& taskBegin{ node->getBegin() };
                const auto& taskEnd{ node->getEnd() };
               std::cout << "task ref " << (int)taskRefCnt << "\n";
                //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                //process_elements[j]->setStartTime(start);
                if (TaskState::ready_for_schedule == taskState)
                { 
                 //  std::cout << "AAAAA\n";
                    if (node == root)
                    {
                        std::cout << "RETURN!!" << "\n";
                        data_for_send[1] = node->getChildren()[0]->getMinimal();
                        data_for_send[2] = node->getChildren()[1]->getMinimal();
                      //  std::cout << "taskState: " << (int)taskState << " ref cnt: " << taskRefCnt << " task type: " << (int)taskType << "\n";
                        //uint64_t r = node->get_result();
                        //std::cout << "RESULT: "<< std::endl;     
                        peHandler.SendDataToProcessingElement(pe_id, (char*)data_for_send, sizeof(char) * 3);
                        node->setState(TaskState::scheduled);
                      //  return;
                        
                    }
                    else
                    {
                        assert(0 == taskRefCnt);

                        data_for_send[0] = (char)taskType;
                       // std::cout << "TYPE: " << (int)taskType << "\n";
                        if (TaskType::root == taskType)
                        {
                            data_for_send[1] = node->getChildren()[0]->getMinimal();
                            data_for_send[2] = node->getChildren()[1]->getMinimal();

                        }
                        //  peHandler.SendDataToProcessingElement(pe_id, (char*)node->getData().data(), 1);
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
    Task* root1 = new MinimalElementRootTask(array, 0, array.size(), TaskState::not_scheduled, TaskType::root, nullptr);

    std::cout << "formMinimalElementDAG START" << std::endl;;
    formMinimalDAG(array, 0, BUFFER_SIZE_FOR_SEND, K, root1);
    std::cout << "formMinimalElementDAG STOP" << std::endl;

    //std::queue<Task*> tree_queue2;
   // addGraphToQueue(root1, tree_queue2);
 
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> tree_queue2;
    addGraphToQueue(root1, tree_queue2);
    std::cout << "queu size: " << tree_queue2.size() << "\n";
    /* for (Task* child : root->getChildren())
         tree_queue.push(child);

        sortQueue(&tree_queue2);
         std::cout << "Processing START" << std::endl;

        std::queue<Task*> tree_queue3 = tree_queue2;
     while (!tree_queue3.empty())
    {
        Task* node = tree_queue3.front();
        std::cout << " ref_cnt: " << node->getCnt() << " ";
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
    const auto start_main{ std::chrono::steady_clock::now() };
 
    scheduleTasks(root1, tree_queue2, kNumberOfProcessingElements, peHandler);
    
    const auto end_main{ std::chrono::steady_clock::now() };
    const std::chrono::duration<long double, std::milli> elapsed_seconds{ end_main - start_main };
    std::ofstream outfile;

    outfile.open("meassurements_minimal.txt", std::ios_base::app); // append instead of overwrite
    outfile << "[BUFFER_SIZE_FOR_SEND= " << BUFFER_SIZE_FOR_SEND;
    outfile << " kNumberOfProcessingElements= " << (int)kNumberOfProcessingElements;
    outfile << " k= " << K << "]" << std::endl;
    outfile << "MAIN execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    std::cout << "MAIN execution time in milliseconds : " << elapsed_seconds.count() << '\n';

    return 0;
}