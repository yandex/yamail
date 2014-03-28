#include <yamail/memory/limiters_repository.h>

#include <list>
#include <vector>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>

typedef std::string suid;

typedef boost::chrono::system_clock system_clock;
typedef boost::chrono::system_clock::time_point time_point;
using namespace yamail::memory;

TEST(limiters_repository, set_get_limiters)
{
    limiters_repository<suid> repository;
    repository.init_factory(composite_limiter_factory::STRICT);

    ASSERT_EQ(repository.global_limiter().available(), 0);
    ASSERT_EQ(repository.session_limiter().available(), 0);

    // Global limiter only one
    repository.global_limit(510);
    ASSERT_EQ(repository.global_limiter().available(), 510);
    repository.global_limiter().acquire(10);
    ASSERT_EQ(repository.global_limiter().available(), 500); // less to 10

    // Session limiter each time new
    repository.session_limit(400);
    ASSERT_EQ(repository.session_limiter().available(), 400);
    repository.session_limiter().acquire(10);
    ASSERT_EQ(repository.session_limiter().available(), 400); // the same
}

TEST(limiters_repository, make_limiter)
{
    limiters_repository<suid> repository;
    repository.init_factory(composite_limiter_factory::STRICT);
    repository.global_limit(500, "global");
    repository.session_limit(400);

    composite_limiter limit = repository.make_limiter("composite", "session");
    // By default it consists of global and session limiters
    ASSERT_EQ(limit.limiters().size(), 2);
    ASSERT_EQ(limit.name(), "composite");
    ASSERT_EQ(limit.used(), 0);
    // we expect that the first limiter is session because
    // it's should be smaller then all other
    composite_limiter::container vec;
    vec = limit.limiters();
    EXPECT_EQ(vec[0].name(), std::string("session"));
    EXPECT_EQ(vec[1].limit(), 500);
}

limiter limiter_by_name(std::string name, const composite_limiter& limiter)
{
    composite_limiter::container vec = limiter.limiters();
    composite_limiter::container::const_iterator it = vec.begin();
    for(; it != vec.end(); it++)
        if((*it).name() == name)
            return *it;

    throw std::runtime_error("limiter with name " + name + " does not exist");
    return make_unlimited_limiter(0); // hide warnings
}

TEST(limiters_repository, one_suid_for_two_climiters)
{
    typedef composite_limiter c_limiter;

    limiters_repository<suid> repository;
    repository.init_factory(composite_limiter_factory::STRICT);
    repository.global_limit(400);
    repository.session_limit(400);
    repository.suid_limit(400);

    c_limiter limiter1 = repository.make_limiter("composite1", "session1");
    c_limiter limiter2 = repository.make_limiter("composite2", "session2");

    ASSERT_NO_THROW(repository.upgrade_limiter_with("vasya_pupkin", limiter1));
    ASSERT_EQ(limiter1.limiters().size(), 3);
    ASSERT_NO_THROW(limiter_by_name("suid_vasya_pupkin", limiter1));
    limiter u_lim = limiter_by_name("suid_vasya_pupkin", limiter1);
    ASSERT_EQ(u_lim.available(), 400);
    ASSERT_EQ(u_lim.limit(), 400);

    // acquire 100 byte from limiter1, so in suid limiter is available 300 (400 - 100)
    limiter1.acquire(100);
    ASSERT_EQ(u_lim.available(), 300);

    // the same suid used for limiter2
    limiter2.acquire(100);
    // the memory used from limiter2 will be applied to suid limiter
    ASSERT_NO_THROW(repository.upgrade_limiter_with("vasya_pupkin", limiter2));
    ASSERT_NO_THROW(limiter_by_name("suid_vasya_pupkin", limiter2));

    ASSERT_EQ(u_lim.available(), 200);
}

TEST(limiters_repository, independent_suid_limiters)
{
    typedef composite_limiter c_limiter;

    limiters_repository<suid> repository;
    repository.init_factory(composite_limiter_factory::STRICT);
    repository.global_limit(400);
    repository.session_limit(400);
    repository.suid_limit(400);

    c_limiter limiter1 = repository.make_limiter("composite1", "session1");
    c_limiter limiter2 = repository.make_limiter("composite2", "session2");

    ASSERT_NO_THROW(repository.upgrade_limiter_with("42", limiter1));
    limiter1.acquire(100);
    limiter u42_lim = limiter_by_name("suid_42", limiter1);
    ASSERT_EQ(u42_lim.available(), 300);

    ASSERT_NO_THROW(repository.upgrade_limiter_with("69", limiter2));
    limiter u69_lim = limiter_by_name("suid_69", limiter2);
    ASSERT_EQ(u69_lim.available(), 400);
}

TEST(limiters_repository, release_suid_limiter)
{
    typedef composite_limiter c_limiter;

    limiters_repository<suid> repository;
    repository.init_factory(composite_limiter_factory::STRICT);
    repository.global_limit(400);
    repository.session_limit(400);
    repository.suid_limit(400);

    {
        c_limiter lim = repository.make_limiter("composite1", "session1");
        lim.acquire(100);
        repository.upgrade_limiter_with("69", lim);
        limiter u_lim = limiter_by_name("suid_69", lim);
        ASSERT_EQ(u_lim.available(), 300);
    }
    // Now limiter associated with suid = 69 was released
    ASSERT_EQ(repository.suid_storage_size(), 0);

    c_limiter lim = repository.make_limiter("composite1", "session1");
    repository.upgrade_limiter_with("69", lim);
    limiter u_lim = limiter_by_name("suid_69", lim);
    ASSERT_EQ(u_lim.available(), 400);
}

TEST(limiters_repository, release_all_suid_limiters)
{
    typedef composite_limiter limiter;
    typedef std::vector<limiter> limiter_list;

    limiters_repository<int> repository;
    repository.init_factory(composite_limiter_factory::STRICT);
    repository.suid_limit(1);

    {
        limiter_list limiters;
        for(int i=0; i<100; i++)
        {
            limiter l = repository.make_limiter();
            repository.upgrade_limiter_with(i, l);
            limiters.push_back(l);
        }
        ASSERT_EQ(repository.suid_storage_size(), 100);
        limiters.clear();
    }
    ASSERT_EQ(repository.suid_storage_size(), 0);
}

#if 0
void pusher(limiters_repository<int>& repo, std::list<composite_limiter>& storage, boost::mutex& mtx, time_point stop)
{
    while(system_clock::now() < stop)
    {
        std::list<composite_limiter> local_storage;
        for(int i=0; i<20000; i++)
        {
            composite_limiter lim = repo.make_limiter();
            repo.upgrade_limiter_with(i, lim);
            local_storage.push_back(lim);
        }

        boost::mutex::scoped_lock lock(mtx);
        storage.splice(storage.end(), local_storage);
    }
}
void deleter(std::list<composite_limiter>& storage, boost::mutex& mtx, time_point stop)
{
    while(system_clock::now() < stop)
    {
        std::list<composite_limiter> local_storage;
        {
            boost::mutex::scoped_lock lock(mtx);
            local_storage.splice(local_storage.end(), storage);
        }
    }
}

TEST(limiters_repository, thread_safety)
{
    boost::mutex mtx;
    limiters_repository<int> repo;
    std::list<composite_limiter> storage;

    time_point stop_time = system_clock::now() + boost::chrono::seconds(10);

    boost::thread th1 = boost::thread(pusher, boost::ref(repo), boost::ref(storage), boost::ref(mtx), stop_time);
    boost::thread th2 = boost::thread(deleter, boost::ref(storage), boost::ref(mtx), stop_time);

    th1.join();
    th2.join();

    storage.clear();
    ASSERT_EQ(repo.suid_storage_size(), 0);

}
#endif
