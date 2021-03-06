//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// order_by_queries_sql_test.cpp
//
// Identification: test/sql/order_by_queries_sql_test.cpp
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "catalog/catalog.h"
#include "common/harness.h"
#include "executor/create_executor.h"
#include "optimizer/optimizer.h"
#include "optimizer/simple_optimizer.h"
#include "planner/create_plan.h"

#include "sql/sql_tests_util.h"

namespace peloton {
namespace test {

class OrderByQueriesSQLTests : public PelotonTest {};

void CreateAndLoadTable() {
  // Create a table first
  SQLTestsUtil::ExecuteSQLQuery(
      "CREATE TABLE test(a INT PRIMARY KEY, b INT, c INT, d VARCHAR);");

  // Insert tuples into table
  SQLTestsUtil::ExecuteSQLQuery("INSERT INTO test VALUES (1, 22, 333, 'abcd');");
  SQLTestsUtil::ExecuteSQLQuery("INSERT INTO test VALUES (2, 33, 111, 'bcda');");
  SQLTestsUtil::ExecuteSQLQuery("INSERT INTO test VALUES (3, 11, 222, 'bcd');");
}

TEST_F(OrderByQueriesSQLTests, OrderByWithColumnsTest) {
  catalog::Catalog::GetInstance()->CreateDatabase(DEFAULT_DB_NAME, nullptr);

  CreateAndLoadTable();

  std::vector<StatementResult> result;
  std::vector<FieldInfo> tuple_descriptor;
  std::string error_message;
  int rows_changed;
  std::unique_ptr<optimizer::AbstractOptimizer> optimizer(
      new optimizer::SimpleOptimizer());

  std::string query("SELECT a, b FROM test ORDER BY b;");
  auto select_plan = SQLTestsUtil::GeneratePlanWithOptimizer(optimizer, query);
  EXPECT_EQ(select_plan->GetPlanNodeType(), PlanNodeType::ORDERBY);

  // test small int
  SQLTestsUtil::ExecuteSQLQueryWithOptimizer(
      optimizer, query, result, tuple_descriptor, rows_changed, error_message);
  // Check the return value
  // Should be: 3, 1, 2
  EXPECT_EQ(0, rows_changed);
  EXPECT_EQ("3", SQLTestsUtil::GetResultValueAsString(result, 0));
  EXPECT_EQ("1", SQLTestsUtil::GetResultValueAsString(result, 2));
  EXPECT_EQ("2", SQLTestsUtil::GetResultValueAsString(result, 4));

  // free the database just created
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  catalog::Catalog::GetInstance()->DropDatabaseWithName(DEFAULT_DB_NAME, txn);
  txn_manager.CommitTransaction(txn);
}

TEST_F(OrderByQueriesSQLTests, OrderByWithoutColumnsTest) {
  catalog::Catalog::GetInstance()->CreateDatabase(DEFAULT_DB_NAME, nullptr);

  CreateAndLoadTable();

  std::vector<StatementResult> result;
  std::vector<FieldInfo> tuple_descriptor;
  std::string error_message;
  int rows_changed;
  std::unique_ptr<optimizer::AbstractOptimizer> optimizer(
      new optimizer::SimpleOptimizer());

  std::string query("SELECT a FROM test ORDER BY b;");
  auto select_plan = SQLTestsUtil::GeneratePlanWithOptimizer(optimizer, query);
  EXPECT_EQ(select_plan->GetPlanNodeType(), PlanNodeType::ORDERBY);

  // test small int
  SQLTestsUtil::ExecuteSQLQueryWithOptimizer(
      optimizer, query, result, tuple_descriptor, rows_changed, error_message);
  // Check the return value
  // Should be: 3, 1, 2
  EXPECT_EQ(0, rows_changed);
  EXPECT_EQ("3", SQLTestsUtil::GetResultValueAsString(result, 0));
  EXPECT_EQ("1", SQLTestsUtil::GetResultValueAsString(result, 1));
  EXPECT_EQ("2", SQLTestsUtil::GetResultValueAsString(result, 2));

  // free the database just created
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  catalog::Catalog::GetInstance()->DropDatabaseWithName(DEFAULT_DB_NAME, txn);
  txn_manager.CommitTransaction(txn);
}


TEST_F(OrderByQueriesSQLTests, OrderByWithColumnsAndLimitTest) {
  catalog::Catalog::GetInstance()->CreateDatabase(DEFAULT_DB_NAME, nullptr);

  CreateAndLoadTable();

  std::vector<StatementResult> result;
  std::vector<FieldInfo> tuple_descriptor;
  std::string error_message;
  int rows_changed;
  std::unique_ptr<optimizer::AbstractOptimizer> optimizer(
      new optimizer::SimpleOptimizer());

  std::string query("SELECT a, b, d FROM test ORDER BY d LIMIT 2;");
  auto select_plan = SQLTestsUtil::GeneratePlanWithOptimizer(optimizer, query);
  EXPECT_EQ(select_plan->GetPlanNodeType(), PlanNodeType::LIMIT);
  EXPECT_EQ(select_plan->GetChildren()[0]->GetPlanNodeType(), PlanNodeType::ORDERBY);
  // test small int
  SQLTestsUtil::ExecuteSQLQueryWithOptimizer(
      optimizer, query, result, tuple_descriptor, rows_changed, error_message);
  // Check the return value
  // Should be: 1, 3
  EXPECT_EQ(0, rows_changed);
  EXPECT_EQ("1", SQLTestsUtil::GetResultValueAsString(result, 0));
  EXPECT_EQ("3", SQLTestsUtil::GetResultValueAsString(result, 3));

  // free the database just created
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  catalog::Catalog::GetInstance()->DropDatabaseWithName(DEFAULT_DB_NAME, txn);
  txn_manager.CommitTransaction(txn);
}

TEST_F(OrderByQueriesSQLTests, OrderByWithoutColumnsAndLimitTest) {
  catalog::Catalog::GetInstance()->CreateDatabase(DEFAULT_DB_NAME, nullptr);

  CreateAndLoadTable();

  std::vector<StatementResult> result;
  std::vector<FieldInfo> tuple_descriptor;
  std::string error_message;
  int rows_changed;
  std::unique_ptr<optimizer::AbstractOptimizer> optimizer(
      new optimizer::SimpleOptimizer());

  std::string query("SELECT a FROM test ORDER BY d LIMIT 2;");
  auto select_plan = SQLTestsUtil::GeneratePlanWithOptimizer(optimizer, query);
  EXPECT_EQ(select_plan->GetPlanNodeType(), PlanNodeType::LIMIT);
  EXPECT_EQ(select_plan->GetChildren()[0]->GetPlanNodeType(), PlanNodeType::ORDERBY);

  SQLTestsUtil::ExecuteSQLQueryWithOptimizer(
      optimizer, query, result, tuple_descriptor, rows_changed, error_message);
  // Check the return value
  // Should be: 1, 3
  EXPECT_EQ(0, rows_changed);
  EXPECT_EQ("1", SQLTestsUtil::GetResultValueAsString(result, 0));
  EXPECT_EQ("3", SQLTestsUtil::GetResultValueAsString(result, 1));

  // free the database just created
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  catalog::Catalog::GetInstance()->DropDatabaseWithName(DEFAULT_DB_NAME, txn);
  txn_manager.CommitTransaction(txn);
}}  // namespace test
}  // namespace peloton
