/*
              __ __ __
             |__|__|  | __
             |  |  |  ||__|
  ___ ___ __ |  |  |  |
 |   |   |  ||  |  |  |    Ubiquitous Internet @ IIT-CNR
 |   |   |  ||  |  |  |    C++ quantum routing libraries and tools
 |_______|__||__|__|__|    https://github.com/ccicconetti/quantum-routing

Licensed under the MIT License <http://opensource.org/licenses/MIT>
Copyright (c) 2022 C. Cicconetti <https://ccicconetti.github.io/>

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "QuantumRouting/poissonpointprocess.h"

#include "gtest/gtest.h"

#include <set>

namespace uiiit {
namespace qr {

struct TestPoissonPointProcess : public ::testing::Test {};

TEST_F(TestPoissonPointProcess, test_grid) {
  const double            W = 1000;
  const double            H = 1;
  PoissonPointProcessGrid myPppGrid(10, 42, W, H);

  const auto myDrop = myPppGrid();
  ASSERT_EQ(9, myDrop.size());

  for (const auto& elem : myDrop) {
    ASSERT_TRUE(std::get<0>(elem) >= 0 and std::get<0>(elem) <= W)
        << "(" << std::get<0>(elem) << "," << std::get<1>(elem) << ")";
    ASSERT_TRUE(std::get<1>(elem) >= 0 and std::get<1>(elem) <= H)
        << "(" << std::get<0>(elem) << "," << std::get<1>(elem) << ")";
  }

  std::set<std::size_t> myDropSizes;
  for (auto i = 0; i < 100; i++) {
    myDropSizes.insert(myPppGrid().size());
  }

  ASSERT_GT(*myDropSizes.begin(), 0); // no drop with 0 elements
  ASSERT_EQ(18, myDropSizes.size());
}

} // namespace qr
} // namespace uiiit
