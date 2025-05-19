#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Account.h"
#include "Transaction.h"

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::Ref;
using ::testing::_;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

class MockTransaction : public Transaction {
public:
    MOCK_METHOD(void, SaveToDataBase, (Account& from, Account& to, int sum), (override));
};

TEST(TransactionTests, FeeManagement) {
    Transaction transaction;
    const int fee = 5;
    
    transaction.set_fee(fee);
    EXPECT_EQ(transaction.fee(), fee);
    
    transaction.set_fee(10);
    EXPECT_EQ(transaction.fee(), 10);
}

TEST(TransactionTests, MakeThrowsOnNegativeSum) {
    MockAccount acc1(1, 100);
    MockAccount acc2(2, 200);
    Transaction transaction;
    
    EXPECT_THROW(transaction.Make(acc1, acc2, -50), std::invalid_argument);
}

TEST(TransactionTests, MakeThrowsOnSameAccount) {
    MockAccount acc1(1, 100);
    Transaction transaction;
    
    EXPECT_THROW(transaction.Make(acc1, acc1, 50), std::logic_error);
}

TEST(TransactionTests, MakeThrowsOnSmallSum) {
    MockAccount acc1(1, 100);
    MockAccount acc2(2, 200);
    Transaction transaction;
    
    EXPECT_THROW(transaction.Make(acc1, acc2, 99), std::logic_error);
}

TEST(TransactionTests, MakeFailsWhenFeeTooHigh) {
    MockAccount acc1(1, 1000);
    MockAccount acc2(2, 2000);
    Transaction transaction;
    transaction.set_fee(500);
    
    EXPECT_FALSE(transaction.Make(acc1, acc2, 900));
}

TEST(TransactionTests, SuccessfulTransaction) {
    const int initial1 = 1000;
    const int initial2 = 2000;
    const int sum = 500;
    
    MockAccount acc1(1, initial1);
    MockAccount acc2(2, initial2);
    MockTransaction transaction;
    
    EXPECT_CALL(acc1, Lock()).Times(1);
    EXPECT_CALL(acc2, Lock()).Times(1);
    EXPECT_CALL(acc1, GetBalance())
        .Times(1)
        .WillOnce(Return(initial1));
    EXPECT_CALL(acc1, ChangeBalance(-(sum + transaction.fee()))).Times(1);
    EXPECT_CALL(acc2, ChangeBalance(sum)).Times(1);
    EXPECT_CALL(acc1, Unlock()).Times(1);
    EXPECT_CALL(acc2, Unlock()).Times(1);
    EXPECT_CALL(transaction, SaveToDataBase(Ref(acc1), Ref(acc2), sum))
        .Times(1);
    
    EXPECT_TRUE(transaction.Make(acc1, acc2, sum));
}

TEST(TransactionTests, FailedTransactionDueToInsufficientFunds) {
    const int initial1 = 100;
    const int initial2 = 2000;
    const int sum = 500;
    
    MockAccount acc1(1, initial1);
    MockAccount acc2(2, initial2);
    MockTransaction transaction;
    
    EXPECT_CALL(acc1, Lock()).Times(1);
    EXPECT_CALL(acc2, Lock()).Times(1);
    EXPECT_CALL(acc1, GetBalance())
        .Times(1)
        .WillOnce(Return(initial1));
    EXPECT_CALL(acc2, ChangeBalance(sum)).Times(1);
    EXPECT_CALL(acc2, ChangeBalance(-sum)).Times(1); // Rollback
    EXPECT_CALL(acc1, ChangeBalance(_)).Times(0);
    EXPECT_CALL(acc1, Unlock()).Times(1);
    EXPECT_CALL(acc2, Unlock()).Times(1);
    EXPECT_CALL(transaction, SaveToDataBase(Ref(acc1), Ref(acc2), sum))
        .Times(1);
    
    EXPECT_FALSE(transaction.Make(acc1, acc2, sum));
}

TEST(TransactionTests, DatabaseOutputFormat) {
    const int initial1 = 1000;
    const int initial2 = 2000;
    const int sum = 500;
    
    MockAccount acc1(1, initial1);
    MockAccount acc2(2, initial2);
    Transaction transaction;
    
    ON_CALL(acc1, GetBalance())
        .WillByDefault(Return(initial1 - sum - transaction.fee()));
    ON_CALL(acc2, GetBalance())
        .WillByDefault(Return(initial2 + sum));
    
    testing::internal::CaptureStdout();
    transaction.Make(acc1, acc2, sum);
    std::string output = testing::internal::GetCapturedStdout();
    
    std::string expected = "1 send to 2 $500\nBalance 1 is 499\nBalance 2 is 2500\n";
    EXPECT_EQ(output, expected);
}
