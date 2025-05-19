#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Account.h"
#include "Transaction.h"

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::Ref;
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::NiceMock;

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

TEST(TransactionTests, SuccessfulTransaction) {
    const int initial1 = 1000;
    const int initial2 = 2000;
    const int sum = 500;
    
    NiceMock<MockAccount> acc1(1, initial1);
    NiceMock<MockAccount> acc2(2, initial2);
    MockTransaction transaction;
    
   
    
    testing::InSequence seq;
    
    EXPECT_CALL(acc1, Lock()).Times(1);
    EXPECT_CALL(acc2, Lock()).Times(1);
    
    EXPECT_CALL(acc2, ChangeBalance(sum)).Times(1);
    EXPECT_CALL(acc1, ChangeBalance(-(sum + transaction.fee()))).Times(1);
    
    EXPECT_CALL(acc1, Unlock()).Times(1);
    EXPECT_CALL(acc2, Unlock()).Times(1);
    
    EXPECT_CALL(transaction, SaveToDataBase(Ref(acc1), Ref(acc2), sum)).Times(1);

    EXPECT_TRUE(transaction.Make(acc1, acc2, sum));
}

TEST(TransactionTests, FailedTransactionDueToInsufficientFunds) {
    const int initial1 = 100;
    const int initial2 = 2000;
    const int sum = 500;
    
    NiceMock<MockAccount> acc1(1, initial1);
    NiceMock<MockAccount> acc2(2, initial2);
    MockTransaction transaction;
    
    testing::InSequence seq;
    
    EXPECT_CALL(acc1, Lock()).Times(1);
    EXPECT_CALL(acc2, Lock()).Times(1);
    
    
    EXPECT_CALL(acc2, ChangeBalance(sum)).Times(1);
    
   
    ON_CALL(acc1, GetBalance()).WillByDefault(Return(initial1));
    
  
    EXPECT_CALL(acc2, ChangeBalance(-sum)).Times(1);
    
    EXPECT_CALL(acc1, Unlock()).Times(1);
    EXPECT_CALL(acc2, Unlock()).Times(1);
    
    
    EXPECT_CALL(transaction, SaveToDataBase(Ref(acc1), Ref(acc2), sum)).Times(1);

    EXPECT_FALSE(transaction.Make(acc1, acc2, sum));
}

TEST(TransactionTests, DatabaseOutputFormat) {
    const int initial1 = 1000;
    const int initial2 = 2000;
    const int sum = 500;
    
    Transaction transaction;
    NiceMock<MockAccount> acc1(1, initial1);
    NiceMock<MockAccount> acc2(2, initial2);
    
   
    ON_CALL(acc1, GetBalance())
        .WillByDefault(Return(initial1 - sum - transaction.fee()));
    ON_CALL(acc2, GetBalance())
        .WillByDefault(Return(initial2 + sum));
    
    testing::internal::CaptureStdout();
    transaction.Make(acc1, acc2, sum);
    std::string output = testing::internal::GetCapturedStdout();
    
 
    EXPECT_THAT(output, HasSubstr("1 send to 2 $500"));
    EXPECT_THAT(output, HasSubstr("Balance 1 is 499")); 
    EXPECT_THAT(output, HasSubstr("Balance 2 is 2500"));
}
