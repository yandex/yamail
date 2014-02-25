#include <yamail/memory/allocator.h>

#include <list>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

struct byte_100
{
    char data[100];
};


typedef boost::chrono::system_clock system_clock;
typedef boost::chrono::system_clock::time_point time_point;
using namespace yamail::memory;

template <typename T, typename List>
void pusher(time_point start, List& l, boost::atomic<size_t>& count)
{
    boost::this_thread::sleep_until(start);
    try
    {
        while(true)
        {
            l.push_back(T());
            ++count;
        }
    }
    catch(...){}
}

template<typename T, typename Limiter>
void test(size_t th, size_t limit)
{
    typedef limited_allocator<T, Limiter> l_allocator;
    typedef std::list<T, l_allocator> l_list;

    time_point start = system_clock::now() + boost::chrono::milliseconds(100);
    std::list<l_list> collector;
    boost::atomic<size_t> count(0);
    size_t total_limit = 1024 * 1024 * limit;
    Limiter limiter(total_limit);
    l_allocator allocator(limiter);
    boost::thread_group gr;

    for(size_t i=0; i<th; i++)
    {
        collector.push_back(l_list(allocator));
        gr.add_thread(new boost::thread(pusher<T, l_list>,
                start, boost::ref(collector.back()), boost::ref(count)));

    }

    gr.join_all();

    typedef std::_List_node<T> list_node;
    size_t node_size = sizeof(list_node);
    size_t expected = total_limit / node_size;

    std::cout << "Expected " << expected << ", produced " << count.load() << std::endl;
}

struct profiler
{
    profiler()
    { start = system_clock::now();}

    ~profiler()
    {
        boost::chrono::milliseconds msec = boost::chrono::duration_cast<boost::chrono::milliseconds>(system_clock::now() - start);
        std::cout << "Spent: " << msec << std::endl;
    }

    time_point start;
};

int main()
{
    {
        profiler p;
        std::cout << "Test strict limiter:\n";
        test<byte_100, strict_limiter>(20, 20);
    }
    std::cout << std::endl;
    {
        profiler p;
        std::cout << "Test fuzzy limiter:\n";
        test<byte_100, fuzzy_limiter>(20, 20);
    }
}


