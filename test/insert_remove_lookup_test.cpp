/**
 * @file insert_lookup_test.cpp
 * @author Haoguang Yang (yang1510@purdue.edu)
 * @version 0.1
 * @date 2023-11-30
 *
 * @copyright Copyright (c) 2023 Haoguang Yang
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 *
 */

#include "gtest/gtest.h"
#include "nd_lookup_table/avl_tree.hpp"
#include "nd_lookup_table/lookup_table.hpp"

TEST(nd_lookup_table, insertAndDeleteLookupTest) {
  LUT::LookupTable<double, double> table;
  // Left Left Rotation Test
  table.insert(3., 10.);
  table.insert(2., 20.);
  table.insert(1., 30.);

  // After the above insertions, a Left Left rotation should be triggered
  // The tree should become: 2, 1, 3

  EXPECT_EQ(table.lookup(1.5), 25.);  // Check if the lookup works correctly after rotation
  EXPECT_EQ(table.lookup(2.5), 15.);

  // Right Left Rotation Test
  table.insert(4., 40.);
  table.insert(3.5, 50.);

  // After the above insertions, a Right Left rotation should be triggered
  // The tree should become: 2, 1, 3.5, -, -, 3, 4

  EXPECT_EQ(table.lookup(3.25), 30.0);
  EXPECT_EQ(table.lookup(3.75), 45.0);
  EXPECT_EQ(table.lookup(2.5), 15.);

  // Right Right Rotation Test
  table.insert(5., 60.);

  // After the above insertions, a Right Right rotation should be triggered
  // The tree should become: 3.5, 2, 4, 1, 3, -, 5

  EXPECT_EQ(table.lookup(2.75), 12.5);
  EXPECT_EQ(table.lookup(3.25), 30.0);
  EXPECT_EQ(table.lookup(3.75), 45.0);
  EXPECT_EQ(table.lookup(4.5), 50.);

  // Reachability test
  EXPECT_EQ(table.lookup(1.), 30.);
  EXPECT_EQ(table.lookup(2.), 20.);
  EXPECT_EQ(table.lookup(3.), 10.);
  EXPECT_EQ(table.lookup(3.5), 50.);
  EXPECT_EQ(table.lookup(4.), 40.);
  EXPECT_EQ(table.lookup(5.), 60.);

  // start removing elements
  table.remove(5., 60.);
  EXPECT_EQ(table.lookup(3.25), 30.0);
  EXPECT_EQ(table.lookup(3.75), 45.0);
  EXPECT_EQ(table.lookup(2.5), 15.);

  table.remove(3.5, 50.);
  table.remove(4., 40.);
  EXPECT_EQ(table.lookup(1.5), 25.);
  EXPECT_EQ(table.lookup(2.5), 15.);

  table.remove(2., 20.);
  EXPECT_EQ(table.lookup(2.0), 20.);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
