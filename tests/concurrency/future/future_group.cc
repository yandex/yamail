#include <stdexcept>
#include <gtest/gtest.h>
#include <yamail/concurrency/future/future_group.h>

namespace {

using namespace yamail::concurrency::future;

TEST(future_group, and_group_get_value) {
    promise<void> first;
    promise<void> second;
    future<void> group = op(first) && op(second);
    first.set();
    second.set();
    group.get();
    SUCCEED();
}

TEST(future_group, and_group_catch_std_exception) {
    const std::string message = "some error";
    promise<void> first;
    promise<void> second;
    future<void> group = op(first) && op(second);
    first.set_exception(std::runtime_error(message));
    second.set_exception(std::runtime_error(message));
    try {
        group.get();
    } catch (const std::runtime_error &e) {
        ASSERT_EQ(std::string(e.what()), message);
    }
}

struct some_error : public std::runtime_error {
    some_error(const std::string& message) : std::runtime_error(message) {}
};

TEST(future_group, and_group_catch_user_exception) {
    const std::string message = "some error";
    promise<void> first;
    promise<void> second;
    future<void> group = op(first) && op(second);
    first.set_exception(some_error(message));
    second.set_exception(some_error(message));
    try {
        group.get();
    } catch (const some_error &e) {
        ASSERT_EQ(std::string(e.what()), message);
    }
}

TEST(future_group, and_group_set_with_set_exception) {
    const std::string message = "some error";
    promise<void> first;
    promise<void> second;
    future<void> group = op(first) && op(second);
    first.set();
    second.set_exception(std::runtime_error(message));
    try {
        group.get();
    } catch (const std::runtime_error &e) {
        ASSERT_EQ(std::string(e.what()), message);
    }
}

TEST(future_group, and_group_dont_rewrite_exception) {
    const std::string someMessage = "some error";
    const std::string anotherMessage = "another error";
    promise<void> first;
    promise<void> second;
    future<void> group = op(first) && op(second);
    first.set_exception(std::runtime_error(someMessage));
    second.set_exception(std::runtime_error(anotherMessage));
    try {
        group.get();
    } catch (const std::runtime_error &e) {
        ASSERT_EQ(std::string(e.what()), someMessage);
    }
}

TEST(future_group, and_group_wait_all_promises_on_set) {
    promise<void> first;
    promise<void> second;
    first.set();
    future<void> group = op(first) && op(second);
    ASSERT_FALSE(group.ready());
    second.set();
    group.get();
    SUCCEED();
}

TEST(future_group, and_group_wait_all_promises_on_set_exception) {
    const std::string message = "some error";
    promise<void> first;
    promise<void> second;
    first.set_exception(std::runtime_error(message));
    future<void> group = op(first) && op(second);
    ASSERT_FALSE(group.ready());
    second.set();
    EXPECT_THROW(group.get(), std::runtime_error);
}

}
