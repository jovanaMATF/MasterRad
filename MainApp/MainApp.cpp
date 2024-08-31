#define WIN32_LEAN_AND_MEAN

#include "ProcessingElementHandler.h"

#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <stack>
#include <thread>
#include <iostream>
#include <fstream>

#define BUFFER_SIZE_FOR_SEND 524288/4
#define ARRAY_SIZE 2u
namespace
{
  constexpr char kNumberOfProcessingElements{ 16u };
  constexpr unsigned int kDefaultBufferLen{ BUFFER_SIZE_FOR_SEND };
}; // end of anonymous namespace


constexpr unsigned int RCV_BUFFER = 4u;


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
    virtual void execute(uint64_t leaf_result) = 0;

    virtual uint64_t get_result() const = 0;

    virtual std::array<uint8_t, ARRAY_SIZE> getData() =0;

    virtual int getCnt() = 0;

    virtual void setCnt(int c) = 0;

    virtual void addChilden(Task* child) = 0;

    virtual std::vector<Task*> getChildren() = 0;

    virtual void setState(TaskState s) = 0;

    virtual TaskState getState() = 0;

    virtual void setType(TaskType s) = 0;

    virtual TaskType getType() = 0;

    virtual void incrementRetCnt() = 0;

    virtual void setIndex(int i) = 0;

    virtual int getIndex() = 0;

    virtual Task* getParent() = 0;

    virtual void setParent(Task* p) = 0;

    virtual ~Task() {}
};


class AccumulateRootTask : public Task
{
public:
    AccumulateRootTask(std::array<uint8_t, ARRAY_SIZE> a, int b, int e, TaskState s,TaskType t, Task* p, int rc) : data(a), begin(b), end(e), state(s),type(t), parent(p), ref_cnt(rc) {}
    std::array<uint8_t, ARRAY_SIZE> getData() override
    {
        return this->data;
    }
    int getCnt()  override
    {
        return this->ref_cnt;
    }
    void setCnt(int c)
    {
        this->ref_cnt = c;
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
    void execute(uint64_t leaf_result) override
    {
        this->result += leaf_result;
    }
    Task* getParent()
    {
        return this->parent;
    }
    void setParent(Task* p)
    {
        this->parent = p;
        
    }
    uint64_t get_result() const override
    {
        return result;
    }
    void setIndex(int i)
    {
        this->index = i;
    }
    int getIndex()
    {
        return this->index;
    }
    void setType(TaskType t)
    {
        this->type = t;
    }
    TaskType getType()
    {
        return this->type;
    }
    ~AccumulateRootTask()
    {
        // dealoacate Child tasks -> Free memory
        for (auto child : this->getChildren())
        {
            free(child);
        }
    }

private:
    uint64_t result{ 0u };
    int begin;
    int end;
    TaskState state;
    TaskType type;
    int ref_cnt;
    std::array<uint8_t, ARRAY_SIZE> data{0};
    Task* parent;
    std::vector<Task*> children = {};
    int index;
    Algorithm algorithm = Algorithm::accumulate;
};

class AccumulateLeafTask : public Task
{
public:
    AccumulateLeafTask(std::array<uint8_t, ARRAY_SIZE> a, int b, int e, TaskState s, TaskType t, Task* p, int rc) : data(a), begin(b), end(e), state(s), type(t), parent(p), ref_cnt(rc) {}
    std::array<uint8_t, ARRAY_SIZE> getData() override
    {
        return this->data;
    }
    int getCnt()  override
    {
        return this->ref_cnt;
    }
    void setCnt(int c)
    {
        this->ref_cnt = c;
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
    }
    int getIndex()
    {
        return this->index;
    }
    void execute(uint64_t r) override
    {
        this->result += r;
    }
    uint64_t get_result() const override
    {
        return result;
    }
    void setType(TaskType t)
    {
        this->type = t;
    }
    TaskType getType()
    {
        return this->type;
    }

private:
    uint64_t result{ 0 };
    int begin;
    int end;
    TaskState state;
    TaskType type;
    int ref_cnt;
    std::array<uint8_t, ARRAY_SIZE> data{0};
    Task* parent;
    std::vector<Task*> children = {};
    int index;
   // Algorithm algorithm = Algorithm::accumulate;
};


void formAccumulateDAG(std::array<uint8_t, ARRAY_SIZE> a, int begin, int end, int k, Task* parent)
{
    // Izlaz iz rekurzije       
    if ((end - begin) <= k)
    {
        std::array<uint8_t, ARRAY_SIZE> subVector;
        subVector.fill(0);
        std::copy(a.begin(), a.end(), subVector.begin());
        Task* accumulated = new AccumulateLeafTask(subVector, 0, subVector.size(), TaskState::not_scheduled, TaskType::leaf, parent, 0);
        accumulated->setParent(parent);
        parent->addChilden(accumulated);
        parent->incrementRetCnt();
        return;
    }
    else
    {
        uint64_t m = (begin + end) / 2;
        formAccumulateDAG(a, begin, m, k, parent);
        formAccumulateDAG(a, m, end, k, parent);
        return;
    }

}

struct CompareTask {
    bool operator()(Task* t1, Task* t2) {
        return t1->getCnt() > t2->getCnt(); // Pretpostavljamo da manji "cnt" znači veći prioritet
    }
};

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
private:
    std::priority_queue<Task*, std::vector<Task*>, CompareTask> process_queue{};
    // koji je procesni element po redu
    int index;
 
    ProcessingElementIdentification m_pe_id = static_cast<ProcessingElementIdentification>(99);
};



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



void scheduleTasks(Task *root, std::priority_queue<Task*, std::vector<Task*>, CompareTask> queue, int n, ProcessingElementHandler<kNumberOfProcessingElements, RCV_BUFFER>& peHandler)
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
        i = (i + 1) % kNumberOfProcessingElements;
        queue.pop();
    } while (!queue.empty());

    int length_pe = process_elements.size();
   // std::cout << "len: " << length_pe << "\n";
#if 0 
    for (int i = 0; i < n; i++)
    {
        std::cout << i << " ";
        process_elements[i]->writeCnt();
    }
#endif


    for (int i = 0; i < kNumberOfProcessingElements; ++i)
    {
        const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(i);
        std::priority_queue<Task*, std::vector<Task*>, CompareTask>& process_q = process_elements[i]->getQueue();
       
        if (!process_q.empty())
        {
            Task* node = process_q.top();
            const auto& taskRefCnt{ node->getCnt() };
            const auto& taskState{ node->getState() };
           
            if (TaskState::ready_for_schedule == taskState)
            {
                assert(0 == taskRefCnt);
                //std::cout << "SEND: " << sizeof(node->getData());
                peHandler.SendDataToProcessingElement(pe_id, (char*)node->getData().data(), 1);
              
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
                        peHandler.ReadDataFromProcessingElement(pe_id, rcv_buffer.data(), sizeof(unsigned int));
                      
                        // Process received data
                        Task* node = process_q.top();
                        node->getParent()->execute((uint64_t)rcv_buffer[0]);

                        // Azuriraj stanje trenutnog taska
                        node->setState(TaskState::processed); // ToDo: processed -> processed

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
                           // std::cout << "PARENT state: " << (int)p->getState() << "\n";
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
            //std::cout << "PE " << j  << " queu: " << process_q.size() << "\n";
            if (!process_q.empty())
            {

                Task* node = process_q.top();
                const auto& taskRefCnt{ node->getCnt() };
                const auto& taskState{ node->getState() };
                const auto& taskType{ node->getType() };
                
                if (TaskState::ready_for_schedule == taskState)
                {
                    if (TaskType::leaf == taskType)
                    {
                        assert(0 == taskRefCnt);

                        peHandler.SendDataToProcessingElement(pe_id, (char*)node->getData().data(), 1);

                        node->setState(TaskState::scheduled);

                    }
                    else
                    {   
                        std::cout << "RETURN!!" << "\n";
                        std::cout << "taskState: " << (int)taskState << " ref cnt: " << taskRefCnt << " task type: " << (int)taskType << "\n";
                        //uint64_t r = node->get_result();
                        //std::cout << "RESULT: "<< std::endl;                                                                                                 Task::get_result() << std::endl;
                        return;
                    }

                }
            }
        }

    } while (TaskState::ready_for_schedule != root->getState());
    std::cout << "DONE!" << std::endl;
}


int __cdecl main(int argc, char** argv)
{
  // Get PEs to Network Adress mapping 

  const std::map<ProcessingElementIdentification, NetworkAddress>& processingElementToNetworkMapping = GetProcessingElementToNetworkMapping();

  std::array<uint8_t, ARRAY_SIZE> array;

  array.fill(1);

  int k = 128;
  // Initialize PE Handler
  ProcessingElementHandler<kNumberOfProcessingElements, RCV_BUFFER> peHandler(processingElementToNetworkMapping);

  // Connect main app to PE servers
  peHandler.ConnectToProcessingElements();


  // const auto start_main{ std::chrono::steady_clock::now() };
  Task* root = new AccumulateRootTask(array, 0, array.size(), TaskState::not_scheduled, TaskType::root, nullptr, 0);

 // std::cout << "formAccumulateDAG START" << std::endl;
  formAccumulateDAG(array, 0, kDefaultBufferLen, k, root);
  //std::cout << "formAccumulateDAG STOP" << std::endl;
  std::priority_queue<Task*, std::vector<Task*>, CompareTask> concurrent_queue;
  
  for (Task* child : root->getChildren())
  {
      if (child->getCnt() == 0)
      {
          child->setState(TaskState::ready_for_schedule);
       //   cnt_child++;
       //   std::cout << cnt_child << std::endl;
          // concurrent_queue.push(child);
      }
      concurrent_queue.push(child);
  }
  concurrent_queue.push(root);
 
  std::cout << concurrent_queue.size();
  //sortQueue(concurrent_queue);

  const auto start_main{ std::chrono::steady_clock::now() };

  scheduleTasks(root, concurrent_queue, kNumberOfProcessingElements, peHandler);
  
  std::cout << "\n" << root->get_result() << "\n";
  const auto end_main{ std::chrono::steady_clock::now() };
  const std::chrono::duration<long double, std::milli> elapsed_seconds{ end_main - start_main };
  std::cout << "MAIN execution time in milliseconds : " << elapsed_seconds.count() << '\n';

  std::ofstream outfile;

  outfile.open("meassurements.txt", std::ios_base::app); // append instead of overwrite
  outfile << "[BUFFER_SIZE_FOR_SEND= " << BUFFER_SIZE_FOR_SEND;
  outfile << " kNumberOfProcessingElements= " << (int)kNumberOfProcessingElements;
  outfile << " k= " << k << "]" << std::endl;
  outfile << "MAIN execution time in milliseconds: " << elapsed_seconds.count() << '\n';


  return 0;
}