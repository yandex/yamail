#include <yamail/memory/limited_allocator.h>

#include <list>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>

//#define DEBUG_OUTPUT

struct byte_100
{
    char data[100];
};


typedef boost::chrono::system_clock system_clock;
typedef boost::chrono::system_clock::time_point time_point;
using namespace yamail::memory;

//#########################################################
// Part of code for recognizing real size of node in list

template<typename T>
class x_allocator: public std::allocator<T>
{
    typedef typename std::allocator<T> base_t;

public:
    typedef typename base_t::size_type size_type;
    typedef typename base_t::difference_type difference_type;
    typedef typename base_t::pointer pointer;
    typedef typename base_t::const_pointer const_pointer;
    typedef typename base_t::reference reference;
    typedef typename base_t::const_reference const_reference;
    typedef typename base_t::value_type value_type;

    template <typename U> struct rebind {
        typedef x_allocator<U> other;
    };

public:
    x_allocator(): allocated_(new size_t(0))
    { }

    template <typename U>
    x_allocator (x_allocator<U> const& allocator) _noexcept
      : base_t (allocator)
      , allocated_(allocator.allocated())
    { }

    pointer allocate(size_type n,
            typename base_t::template rebind<void>::other::const_pointer hint=0)
    {
        pointer p = this->base_t::allocate(n, hint);
        *allocated_ += n * sizeof(T);
        return p;
    }

    void deallocate(pointer p, size_type n)
    {
        this->base_t::deallocate(p, n);
        *allocated_ -= n * sizeof(T);
    }

    boost::shared_ptr<size_t> allocated() const {return allocated_; }

private:
    boost::shared_ptr<size_t> allocated_;
};

template<typename T>
size_t sizeof_listnode()
{
    typedef std::list<T, x_allocator<T> > x_list;
    x_allocator<T> xa;
    x_list xl(xa);
    xl.push_back(T());
    return *xa.allocated();
}

// Utils

struct safe_cout
{
    template <typename T>
    safe_cout& operator<<(const T& t)
    {
        buf << t;
        return *this;
    }

    ~safe_cout()
    {
#ifdef DEBUG_OUTPUT
        if(!buf.str().empty())
        {
            boost::mutex::scoped_lock lock(global_mutex);
            std::cout << buf.str() << std::endl;
        }
#endif
    }

    std::stringstream buf;
    static boost::mutex global_mutex;
};
boost::mutex safe_cout::global_mutex;

struct profiler
{
    profiler()
    { start = system_clock::now();}

    ~profiler()
    {
        boost::chrono::milliseconds msec = boost::chrono::duration_cast<boost::chrono::milliseconds>(system_clock::now() - start);
        safe_cout() << "Spent: " << msec ;
    }

    time_point start;
};

size_t make_mb(size_t count)
{ return 1024 * 1024 * count; }

template <typename T>
std::string to_string(const T& t)
{
    return boost::lexical_cast<std::string>(t);
}

//#########################################################

template <typename T, typename List>
void pusher(time_point start, List& l, boost::atomic<size_t>& count)
{
    boost::this_thread::sleep_until(start);
    size_t local_counter = 0;
    try
    {
        while(true)
        {
            l.push_back(T());
            ++local_counter;
        }
    }
    catch(...){}
    count += local_counter;
}

template<typename T>
bool test_limiter(size_t th, limiter limit)
{
    typedef limited_allocator<T, limiter> l_allocator;
    typedef std::list<T, l_allocator> l_list;

    time_point start = system_clock::now() + boost::chrono::milliseconds(100);
    std::list<l_list> collector;
    boost::atomic<size_t> count(0);
    l_allocator allocator(limit);
    boost::thread_group gr;

    for(size_t i=0; i<th; i++)
    {
        collector.push_back(l_list(allocator));
        gr.add_thread(new boost::thread(pusher<T, l_list>,
                start, boost::ref(collector.back()), boost::ref(count)));

    }

    gr.join_all();

    size_t expected = limit.limit() / sizeof_listnode<T>();

    safe_cout()
        << (expected == count.load() ? "PASSED" : "FAILED") << ": "
        << "Expected " << expected << ", produced " << count.load();

    return expected == count.load();
}

struct pusher_resault_data
{
    pusher_resault_data(): pushed(0), local_limit(0), group_number(0){}

    size_t pushed;
    size_t local_limit;
    size_t group_number;
    std::string limiter;
};

template <typename T, typename List>
void pusher_x(time_point start, List& l, pusher_resault_data& res)
{
    boost::this_thread::sleep_until(start);
    size_t local_counter = 0;
    try
    {
        while(true)
        {
            l.push_back(T());
            ++local_counter;
        }
    }
    catch(const limiter_exhausted& ex)
    {
        std::string tmp(ex.what());
        // extract the name of exhausted limiter (it's the last limiter in sequence)
        res.limiter = tmp.substr(tmp.rfind('/') + 1, tmp.size() - tmp.rfind('/') - 1);
        //safe_cout() << ex.what();
    }
    res.pushed = local_counter;
}

template <typename T>
bool test_expected_elements(size_t limit_mb, size_t produced, std::string& msg)
{
    size_t expected = limit_mb / sizeof_listnode<T>();
    bool res = (produced <= expected);
    msg = "expected " + to_string(expected) + " produced " + to_string(produced);
    return res;
}

template <typename CompositeLimiteImpl, typename LimiterImpl>
bool test_composite_limiter(size_t th, limiter global, std::vector<limiter> groups)
{
    typedef limited_allocator<byte_100, composite_limiter> l_allocator;
    typedef std::list<byte_100, l_allocator> l_list;

    std::list<pusher_resault_data> res;
    std::list<l_list> collector;
    boost::thread_group gr;
    time_point start = system_clock::now() + boost::chrono::milliseconds(100);

    for(size_t i = 0; i < th; i++)
    {
        composite_limiter l = make_composite_limiter<CompositeLimiteImpl>("composite_" + to_string(i));
        l.add(global);

        size_t gr_number = i % groups.size();
        l.add(groups[gr_number]);

        size_t local_limit = make_mb((i % 5)+1);
        l.add(make_limiter<LimiterImpl>(local_limit, "local"));

        l_allocator allocator(l);
        pusher_resault_data data;
        data.local_limit = local_limit;
        data.group_number = gr_number;
        res.push_back(data);

        collector.push_back(l_list(allocator));
        gr.add_thread(new boost::thread(pusher_x<byte_100, l_list>,
                start, boost::ref(collector.back()), boost::ref(res.back())));
    }

    gr.join_all();
    bool test_resault = true;

    safe_cout() << "Check local limiters:";
    std::list<pusher_resault_data>::iterator it = res.begin();
    for(; it != res.end(); it++)
    {
        std::string msg;
        bool result = test_expected_elements<byte_100>((*it).local_limit, (*it).pushed, msg);

        safe_cout printer;
        if(!result || ((*it).limiter == "local"))
            printer << (result ? "[pass]" : "[FAILED]") << ": " << msg;

        if((*it).limiter.find("local") == 0)
            printer << " (*)";

        test_resault &= result;
    }

    safe_cout() << "Check group limiters:";
    std::map<size_t, size_t> gr_pushed;
    it = res.begin();
    for(; it != res.end(); it++)
    {
        if(gr_pushed.find((*it).group_number) == gr_pushed.end())
            gr_pushed.insert(std::make_pair((*it).group_number, (*it).pushed));
        else
            gr_pushed[(*it).group_number] += (*it).pushed;
    }

    std::map<size_t, size_t>::iterator gr_it = gr_pushed.begin();
    for(; gr_it != gr_pushed.end(); gr_it++)
    {
        std::string msg;
        bool result = test_expected_elements<byte_100>(groups[(*gr_it).first].limit(), (*gr_it).second, msg);
        safe_cout() << (result ? "[pass]" : "[FAILED]") << ": " << groups[(*gr_it).first].name() << ", " << msg;

        test_resault &= result;
    }

    safe_cout() << "Check global limiter:";
    size_t total_pushed = 0;
    it = res.begin();
    for(; it != res.end(); it++)
        total_pushed += (*it).pushed;

    std::string msg;
    bool result = test_expected_elements<byte_100>(global.limit(), total_pushed, msg);
    safe_cout() << (result ? "[pass]" : "[FAILED]") << ": " << msg;
    test_resault &= result;

    return test_resault;
}


template <typename Limiter>
bool test_return_memory(const Limiter& lim)
{
    typedef limited_allocator<byte_100, Limiter> l_allocator;
    typedef std::list<byte_100, l_allocator> l_list;

    l_allocator allocator(lim);
    l_list tmp_list(allocator);
    int64_t pushed = 0;
    try
    {
        while(true)
        {
            tmp_list.push_back(byte_100());
            ++pushed;
        }
    }
    catch(...){}

    tmp_list.clear();
    // repeat and check how many was pushed
    try
    {
        while(true)
        {
            tmp_list.push_back(byte_100());
            --pushed;
        }
    }
    catch(...){}

    return pushed == 0;
}

#if 0
TEST(limited_allocator, performanse)
{
    {
        safe_cout() << "Test strict limiter:";
        { profiler p; test_limiter<byte_100>(1, make_strict_limiter(make_mb(20))); }
        { profiler p; test_limiter<byte_100>(1, make_strict_limiter(make_mb(40))); }
        { profiler p; test_limiter<byte_100>(1, make_strict_limiter(make_mb(80))); }
    }

    {
        safe_cout() << "\n" << "Test fuzzy limiter:";
        { profiler p; test_limiter<byte_100>(1, make_fuzzy_limiter(make_mb(20))); }
        { profiler p; test_limiter<byte_100>(1, make_fuzzy_limiter(make_mb(40))); }
        { profiler p; test_limiter<byte_100>(1, make_fuzzy_limiter(make_mb(80))); }
    }

    {
        safe_cout() << "\n" << "Test basic limiter:";
        { profiler p; test_limiter<byte_100>(1, make_basic_limiter(make_mb(20))); }
        { profiler p; test_limiter<byte_100>(1, make_basic_limiter(make_mb(40))); }
        { profiler p; test_limiter<byte_100>(1, make_basic_limiter(make_mb(80))); }
    }

    {
        safe_cout() << "\n" << "Test composite fuzzy limiter:";
        limiter global = make_fuzzy_limiter(make_mb(30), "global"); // will exhausted
        std::vector<limiter> groups;
        groups.push_back(make_fuzzy_limiter(make_mb(9), "group_1")); // will exhausted
        groups.push_back(make_fuzzy_limiter(make_mb(8), "group_2")); // will exhausted
        groups.push_back(make_fuzzy_limiter(make_mb(20), "group_3"));
        profiler p;
        test_composite_limiter<composite_fuzzy_limiter, fuzzy_limiter>(20, global, groups);
    }

    {
        safe_cout() << "\n" << "Test composite strict limiter:";
        limiter global = make_basic_limiter(make_mb(30), "global"); // will exhausted
        std::vector<limiter> groups;
        groups.push_back(make_basic_limiter(make_mb(9), "group_1")); // will exhausted
        groups.push_back(make_basic_limiter(make_mb(8), "group_2")); // will exhausted
        groups.push_back(make_basic_limiter(make_mb(20), "group_3"));
        profiler p;
        test_composite_limiter<composite_strict_limiter, basic_limiter>(20, global, groups);
    }

}
#endif

TEST(limited_allocator, limiter)
{

    ASSERT_TRUE((test_limiter<byte_100>(1, make_basic_limiter(make_mb(10)))));
    ASSERT_TRUE((test_limiter<byte_100>(20, make_strict_limiter(make_mb(10)))));
    ASSERT_TRUE((test_limiter<byte_100>(20, make_fuzzy_limiter(make_mb(10)))));

    ASSERT_TRUE(test_return_memory(make_basic_limiter(make_mb(1))));
    ASSERT_TRUE(test_return_memory(make_strict_limiter(make_mb(1))));
    ASSERT_TRUE(test_return_memory(make_fuzzy_limiter(make_mb(1))));
}

TEST(limited_allocator, composite_fuzzy_limiter)
{
    {
        limiter global = make_fuzzy_limiter(make_mb(10), "global"); // will exhausted
        std::vector<limiter> groups;
        groups.push_back(make_fuzzy_limiter(make_mb(4), "group_1")); // will exhausted
        groups.push_back(make_fuzzy_limiter(make_mb(4), "group_2")); // will exhausted
        groups.push_back(make_fuzzy_limiter(make_mb(6), "group_3"));
        ASSERT_TRUE((test_composite_limiter<composite_fuzzy_limiter, fuzzy_limiter>(5, global, groups)));

        composite_limiter composite = make_composite_fuzzy_limiter();
        composite.add(global); // higher
        composite.add(groups[1]); // less
        composite.add(groups[0]); // less
        ASSERT_TRUE((test_return_memory(composite)));
    }
}

TEST(limited_allocator, composite_strict_limiter)
{
    {
        limiter global = make_strict_limiter(make_mb(10), "global"); // will exhausted
        std::vector<limiter> groups;
        groups.push_back(make_basic_limiter(make_mb(4), "group_1")); // will exhausted
        groups.push_back(make_basic_limiter(make_mb(4), "group_2")); // will exhausted
        groups.push_back(make_basic_limiter(make_mb(6), "group_3"));
        ASSERT_TRUE((test_composite_limiter<composite_strict_limiter, fuzzy_limiter>(5, global, groups)));

        composite_limiter composite = make_composite_strict_limiter();
        composite.add(global);
        composite.add(groups[1]);
        composite.add(groups[0]);
        ASSERT_TRUE((test_return_memory(composite)));

    }
}

