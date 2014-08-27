#include <stdexcept>
#include <gtest/gtest.h>
#include <yamail/concurrency/future/future_group.h>

namespace {

using namespace yamail::concurrency::future;

TEST(future_group, and_group_set) {
    promise<void> a;
    promise<void> b;
    future<void> group = op(a) && op(b);
    a.set();
    b.set();
    group.get();
    SUCCEED();
}

TEST(future_group, and_group_set_exception) {
    const std::string message = "some error";
    promise<void> a;
    promise<void> b;
    future<void> group = op(a) && op(b);
    a.set_exception(std::runtime_error(message));
    b.set_exception(std::runtime_error(message));
    try {
        group.get();
    } catch (const std::runtime_error &e) {
        ASSERT_EQ(std::string(e.what()), message);
    }
}

TEST(future_group, and_group_set_with_set_exception) {
    const std::string message = "some error";
    promise<void> a;
    promise<void> b;
    future<void> group = op(a) && op(b);
    a.set();
    b.set_exception(std::runtime_error(message));
    try {
        group.get();
    } catch (const std::runtime_error &e) {
        ASSERT_EQ(std::string(e.what()), message);
    }
}

}
