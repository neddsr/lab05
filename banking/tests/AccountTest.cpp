#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../Account.h"

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

TEST(AccountTest, ConstructorSetsValues) {
    Account acc(123, 1000);
    EXPECT_EQ(acc.id(), 123);
    EXPECT_EQ(acc.GetBalance(), 1000);
}

TEST(AccountTest, LockUnlockBehavior) {
    Account acc(1, 100);
    EXPECT_NO_THROW(acc.Lock());
    EXPECT_THROW(acc.Lock(), std::runtime_error);
    EXPECT_NO_THROW(acc.Unlock());
}

TEST(AccountTest, ChangeBalanceRequiresLock) {
    Account acc(1, 100);
    EXPECT_THROW(acc.ChangeBalance(10), std::runtime_error);
    
    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(10));
    EXPECT_EQ(acc.GetBalance(), 110);
}

TEST(AccountTest, MockTest) {
    MockAccount mockAcc(1, 100);
    EXPECT_CALL(mockAcc, Lock()).Times(1);
    EXPECT_CALL(mockAcc, Unlock()).Times(1);
    EXPECT_CALL(mockAcc, GetBalance()).WillOnce(testing::Return(100));
    
    mockAcc.Lock();
    EXPECT_EQ(mockAcc.GetBalance(), 100);
    mockAcc.Unlock();
}
