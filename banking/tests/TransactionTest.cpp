#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../Transaction.h"
#include "../Account.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Exactly;

class MockAccount : public Account {
 public:
  MOCK_METHOD(int, id, (), (const, override));
  MOCK_METHOD(void, Lock, (), (override));
  MOCK_METHOD(void, Unlock, (), (override));
  MOCK_METHOD(int, GetBalance, (), (const, override));
  MOCK_METHOD(void, ChangeBalance, (int), (override));
};

class TransactionTest : public ::testing::Test {
 protected:
  Transaction transaction_;
  MockAccount from_;
  MockAccount to_;
};

TEST_F(TransactionTest, ThrowsIfSameAccount) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(1));

  EXPECT_THROW(transaction_.Make(from_, to_, 100), std::logic_error);
}

TEST_F(TransactionTest, ThrowsIfNegativeSum) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_THROW(transaction_.Make(from_, to_, -10), std::invalid_argument);
}

TEST_F(TransactionTest, ThrowsIfSumTooSmall) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_THROW(transaction_.Make(from_, to_, 50), std::logic_error);
}

TEST_F(TransactionTest, ReturnsFalseIfFeeTooHigh) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_FALSE(transaction_.Make(from_, to_, 1));
}

TEST_F(TransactionTest, ReturnsFalseIfInsufficientFunds) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_CALL(from_, Lock()).Times(1);
  EXPECT_CALL(to_, Lock()).Times(1);
  EXPECT_CALL(from_, Unlock()).Times(1);
  EXPECT_CALL(to_, Unlock()).Times(1);

  EXPECT_CALL(from_, GetBalance()).WillOnce(Return(100)); // balance < sum + fee (101)
  EXPECT_CALL(to_, ChangeBalance(_)).Times(0);
  EXPECT_CALL(from_, ChangeBalance(_)).Times(0);

  bool result = transaction_.Make(from_, to_, 100);
  EXPECT_FALSE(result);
}

TEST_F(TransactionTest, SuccessfulTransaction) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_CALL(from_, Lock()).Times(1);
  EXPECT_CALL(to_, Lock()).Times(1);
  EXPECT_CALL(from_, Unlock()).Times(1);
  EXPECT_CALL(to_, Unlock()).Times(1);

  EXPECT_CALL(from_, GetBalance()).WillOnce(Return(200));

  // Сначала зачисление на to
  EXPECT_CALL(to_, ChangeBalance(100)).Times(1);

  // Затем списание с from (сумма + fee = 101)
  EXPECT_CALL(from_, ChangeBalance(-101)).Times(1);

  bool result = transaction_.Make(from_, to_, 100);
  EXPECT_TRUE(result);
}

TEST_F(TransactionTest, DebitFailsAfterCreditRollback) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_CALL(from_, Lock()).Times(1);
  EXPECT_CALL(to_, Lock()).Times(1);
  EXPECT_CALL(from_, Unlock()).Times(1);
  EXPECT_CALL(to_, Unlock()).Times(1);

  EXPECT_CALL(from_, GetBalance()).WillOnce(Return(150));  

  EXPECT_CALL(to_, ChangeBalance(100)).Times(1);


  EXPECT_CALL(from_, GetBalance()).WillOnce(Return(50)); 
  EXPECT_CALL(from_, ChangeBalance(_)).Times(0);
  EXPECT_CALL(to_, ChangeBalance(-100)).Times(1); 

  bool result = transaction_.Make(from_, to_, 100);
  EXPECT_FALSE(result);
}

TEST_F(TransactionTest, SaveToDatabaseIsCalledOnSuccess) {
  EXPECT_CALL(from_, id()).WillRepeatedly(Return(1));
  EXPECT_CALL(to_, id()).WillRepeatedly(Return(2));

  EXPECT_CALL(from_, Lock()).Times(1);
  EXPECT_CALL(to_, Lock()).Times(1);
  EXPECT_CALL(from_, Unlock()).Times(1);
  EXPECT_CALL(to_, Unlock()).Times(1);

  EXPECT_CALL(from_, GetBalance()).WillOnce(Return(200));
  EXPECT_CALL(to_, ChangeBalance(100)).Times(1);
  EXPECT_CALL(from_, ChangeBalance(-101)).Times(1);




  bool result = transaction_.Make(from_, to_, 100);
  EXPECT_TRUE(result);
}
