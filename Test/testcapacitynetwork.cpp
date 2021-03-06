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

#include "QuantumRouting/capacitynetwork.h"
#include "Support/random.h"
#include "Support/tostring.h"

#include "gtest/gtest.h"

#include <glog/logging.h>

#include <ctime>
#include <set>
#include <stdexcept>

namespace uiiit {
namespace qr {

struct TestCapacityNetwork : public ::testing::Test {
  CapacityNetwork::EdgeVector exampleEdges() {
    return CapacityNetwork::EdgeVector({
        {0, 1},
        {1, 2},
        {2, 3},
        {0, 4},
        {4, 3},
    });
  }

  //   /--> 1 -- >2 -+
  //  /              v
  // 0               3   all weights are 4, except 0->4 which is 1
  //  \              ^
  //   \---> 4 ------+
  CapacityNetwork::WeightVector exampleEdgeWeights() {
    return CapacityNetwork::WeightVector({
        {0, 1, 4},
        {1, 2, 4},
        {2, 3, 4},
        {0, 4, 1},
        {4, 3, 4},
    });
  }

  //
  //  +----> 1 <----+ +---> 4 ----+
  //  |             | |           |
  //  |             v v           v
  //  0              3            6 all weights are 1
  //  |             ^ ^           ^
  //  |             | |           |
  //  +----> 2 <----+ +---> 5 ----+
  //
  CapacityNetwork::WeightVector anotherExampleEdgeWeights() {
    return CapacityNetwork::WeightVector({
        {0, 1, 1},
        {0, 2, 1},
        {1, 3, 1},
        {2, 3, 1},
        {3, 1, 1},
        {3, 2, 1},
        {3, 4, 1},
        {3, 5, 1},
        {4, 3, 1},
        {4, 6, 1},
        {5, 3, 1},
        {5, 6, 1},
    });
  }
};

TEST_F(TestCapacityNetwork, test_random_weights) {
  support::UniformRv myRv(0, 100, std::time(nullptr), 0, 0);

  for (const auto myBidir : std::vector({true, false})) {
    LOG(INFO) << "bidir = " << myBidir;
    CapacityNetwork myNetwork(exampleEdges(), myRv, myBidir);

    const auto myWeights = myNetwork.weights();
    ASSERT_EQ(myBidir ? 10 : 5, myWeights.size());
    std::set<double> myWeightSet;
    for (const auto& elem : myWeights) {
      myWeightSet.insert(std::get<2>(elem));
      ASSERT_TRUE(std::get<2>(elem) >= 0 and std::get<2>(elem) <= 100)
          << "(" << std::get<0>(elem) << "," << std::get<1>(elem) << ") ["
          << std::get<2>(elem) << "]";
    }
    ASSERT_EQ(5, myWeightSet.size());

    if (VLOG_IS_ON(1)) {
      myNetwork.toDot("TestCapacityNetwork.test_random_weights-" +
                      std::to_string(myBidir) + ".dot");
    }
  }
}

TEST_F(TestCapacityNetwork, test_measurement_probability) {
  CapacityNetwork myNetwork(exampleEdgeWeights());
  ASSERT_FLOAT_EQ(1, myNetwork.measurementProbability());
  myNetwork.measurementProbability(0.314);
  ASSERT_FLOAT_EQ(0.314, myNetwork.measurementProbability());
  ASSERT_THROW(myNetwork.measurementProbability(-0.5), std::runtime_error);
  ASSERT_THROW(myNetwork.measurementProbability(2), std::runtime_error);
}

TEST_F(TestCapacityNetwork, test_graph_properties) {
  CapacityNetwork myNetwork(exampleEdgeWeights());
  ASSERT_EQ(5, myNetwork.numNodes());
  ASSERT_EQ(5, myNetwork.numEdges());
  ASSERT_FLOAT_EQ(17, myNetwork.totalCapacity());
  ASSERT_EQ(0, myNetwork.inDegree().first);
  ASSERT_EQ(2, myNetwork.inDegree().second);
  ASSERT_EQ(0, myNetwork.outDegree().first);
  ASSERT_EQ(2, myNetwork.outDegree().second);

  ASSERT_EQ(std::vector<double>({5, 4, 4, 0, 4}), myNetwork.nodeCapacities());
}

TEST_F(TestCapacityNetwork, test_reachable_nodes) {
  CapacityNetwork myNetwork(anotherExampleEdgeWeights());

  std::size_t myDiameter;

  const auto myAll = myNetwork.reachableNodes(0, 99, myDiameter);
  ASSERT_EQ(4, myDiameter);
  ASSERT_EQ(7, myAll.size());
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3, 4, 5, 6}), myAll.find(0)->second);
  ASSERT_EQ(std::set<unsigned long>({2, 3, 4, 5, 6}), myAll.find(1)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 3, 4, 5, 6}), myAll.find(2)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 4, 5, 6}), myAll.find(3)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3, 5, 6}), myAll.find(4)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3, 4, 6}), myAll.find(5)->second);
  ASSERT_EQ(std::set<unsigned long>({}), myAll.find(6)->second);

  const auto mySome = myNetwork.reachableNodes(0, 2, myDiameter);
  ASSERT_EQ(7, myAll.size());
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3}), mySome.find(0)->second);
  ASSERT_EQ(std::set<unsigned long>({2, 3, 4, 5}), mySome.find(1)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 3, 4, 5}), mySome.find(2)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 4, 5, 6}), mySome.find(3)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3, 5, 6}), mySome.find(4)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 3, 4, 6}), mySome.find(5)->second);
  ASSERT_EQ(std::set<unsigned long>({}), mySome.find(6)->second);

  const auto myTwo = myNetwork.reachableNodes(2, 2, myDiameter);
  ASSERT_EQ(7, myTwo.size());
  ASSERT_EQ(std::set<unsigned long>({3}), myTwo.find(0)->second);
  ASSERT_EQ(std::set<unsigned long>({2, 4, 5}), myTwo.find(1)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 4, 5}), myTwo.find(2)->second);
  ASSERT_EQ(std::set<unsigned long>({6}), myTwo.find(3)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 5}), myTwo.find(4)->second);
  ASSERT_EQ(std::set<unsigned long>({1, 2, 4}), myTwo.find(5)->second);
  ASSERT_EQ(std::set<unsigned long>({}), myTwo.find(6)->second);

  const auto myNone = myNetwork.reachableNodes(99, 99, myDiameter);
  ASSERT_EQ(7, myNone.size());
  for (const auto& elem : myNone) {
    ASSERT_TRUE(elem.second.empty());
  }
}

TEST_F(TestCapacityNetwork, test_route_flows) {
  CapacityNetwork myNetwork(exampleEdgeWeights());
  myNetwork.measurementProbability(0.5);

  // no route existing
  std::vector<CapacityNetwork::FlowDescriptor> myFlows({
      {3, 0, 1.0},
  });
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_TRUE(myFlows[0].thePath.empty());
  ASSERT_EQ(1, myFlows[0].theDijsktra);

  // add an unfeasible and a feasible route
  myFlows.clear();
  myFlows.emplace_back(3, 0, 1.0);
  myFlows.emplace_back(0, 3, 1.0);
  myNetwork.route(myFlows);
  ASSERT_EQ(2, myFlows.size());
  ASSERT_TRUE(myFlows[0].thePath.empty());
  ASSERT_FLOAT_EQ(0, myFlows[0].theGrossRate);
  ASSERT_EQ(1, myFlows[0].theDijsktra);
  ASSERT_EQ(std::vector<unsigned long>({1, 2, 3}), myFlows[1].thePath);
  ASSERT_FLOAT_EQ(4, myFlows[1].theGrossRate);
  ASSERT_EQ(2, myFlows[1].theDijsktra);
  ASSERT_EQ(CapacityNetwork::WeightVector({
                {0, 1, 0},
                {1, 2, 0},
                {2, 3, 0},
                {0, 4, 1},
                {4, 3, 4},
            }),
            myNetwork.weights());

  // the same route is not feasible anymore
  myFlows.clear();
  myFlows.emplace_back(0, 3, 1.0);
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_TRUE(myFlows[0].thePath.empty());

  // request with smaller capacity, but cannot be admitted due to constraint
  myFlows.clear();
  myFlows.emplace_back(0, 3, 0.5);
  myNetwork.route(myFlows,
                  [](const auto& aFlow) { return aFlow.thePath.size() == 1; });
  ASSERT_EQ(1, myFlows.size());
  ASSERT_TRUE(myFlows[0].thePath.empty());

  // same request without constrating can be admitted
  myFlows.clear();
  myFlows.emplace_back(0, 3, 0.5);
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_EQ(std::vector<unsigned long>({4, 3}), myFlows[0].thePath);
  ASSERT_FLOAT_EQ(1, myFlows[0].theGrossRate);
  ASSERT_EQ(CapacityNetwork::WeightVector({
                {0, 1, 0},
                {1, 2, 0},
                {2, 3, 0},
                {0, 4, 0},
                {4, 3, 3},
            }),
            myNetwork.weights());

  // add a request for an adjacent node
  myFlows.clear();
  myFlows.emplace_back(4, 3, 3);
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_EQ(std::vector<unsigned long>({3}), myFlows[0].thePath);
  ASSERT_FLOAT_EQ(3, myFlows[0].theGrossRate);
  ASSERT_EQ(CapacityNetwork::WeightVector({
                {0, 1, 0},
                {1, 2, 0},
                {2, 3, 0},
                {0, 4, 0},
                {4, 3, 0},
            }),
            myNetwork.weights());
  ASSERT_FLOAT_EQ(0, myNetwork.totalCapacity());

  // no request can be served now
  myFlows.clear();
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < 5; j++) {
      if (i != j) {
        myFlows.emplace_back(i, j, 0.001);
      }
    }
  }
  myNetwork.route(myFlows);
  for (const auto& myFlow : myFlows) {
    ASSERT_TRUE(myFlow.thePath.empty());
    ASSERT_FLOAT_EQ(0, myFlow.theGrossRate);
  }

  // add ill-formed requests
  myFlows.clear();
  myFlows.emplace_back(0, 0, 1);
  ASSERT_THROW(myNetwork.route(myFlows), std::runtime_error);
  myFlows.clear();
  myFlows.emplace_back(0, 1, 0);
  ASSERT_THROW(myNetwork.route(myFlows), std::runtime_error);
  myFlows.clear();
  myFlows.emplace_back(0, 1, -1);
  ASSERT_THROW(myNetwork.route(myFlows), std::runtime_error);
  myFlows.clear();
  myFlows.emplace_back(0, 99, 1);
  ASSERT_THROW(myNetwork.route(myFlows), std::runtime_error);
  myFlows.clear();
  myFlows.emplace_back(99, 0, 1);
  ASSERT_THROW(myNetwork.route(myFlows), std::runtime_error);
}

TEST_F(TestCapacityNetwork, test_route_flows_another) {
  // swap weights
  auto myWeights = exampleEdgeWeights();
  for (auto& elem : myWeights) {
    auto& myWeight = std::get<2>(elem);
    if (myWeight == 1) {
      myWeight = 4;
    } else {
      myWeight = 1;
    }
  }

  CapacityNetwork myNetwork(myWeights);
  myNetwork.measurementProbability(0.5);

  std::vector<CapacityNetwork::FlowDescriptor> myFlows({
      {0, 3, 0.1},
  });
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_EQ(1, myFlows[0].theDijsktra);
  ASSERT_EQ(std::vector<unsigned long>({4, 3}), myFlows[0].thePath);
}

TEST_F(TestCapacityNetwork, test_route_apps) {
  CapacityNetwork myNetwork(exampleEdgeWeights());
  myNetwork.measurementProbability(0.5);
  std::vector<CapacityNetwork::AppDescriptor> myApps;

  // ill-formed requests
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {0}, 1},
  });
  ASSERT_THROW(myNetwork.route(myApps, 1, 1), std::runtime_error);
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {42}, 1},
  });
  ASSERT_THROW(myNetwork.route(myApps, 1, 1), std::runtime_error);
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {1}, 0},
  });
  ASSERT_THROW(myNetwork.route(myApps, 1, 1), std::runtime_error);
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {1}, -1},
  });
  ASSERT_THROW(myNetwork.route(myApps, 1, 1), std::runtime_error);
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {1}, 1},
  });
  ASSERT_THROW(myNetwork.route(myApps, 0, 1), std::runtime_error);
  ASSERT_THROW(myNetwork.route(myApps, -1, 1), std::runtime_error);
  ASSERT_THROW(myNetwork.route(myApps, 1, 0), std::runtime_error);

  // no route existing
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {3, {2, 0}, 1},
      {2, {1}, 1},
  });
  myNetwork.route(myApps, 1.4, 99);
  ASSERT_EQ(2, myApps.size());
  ASSERT_EQ(0, myApps[0].theAllocated.size());
  ASSERT_FLOAT_EQ(0, myApps[0].grossRate());
  ASSERT_EQ(0, myApps[1].theAllocated.size());
  ASSERT_FLOAT_EQ(0, myApps[1].grossRate());

  // existing routes
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {2, 3}, 1},
      {1, {3}, 1},
  });
  myNetwork.route(myApps, 1.4, 99);
  ASSERT_EQ(2, myApps.size());
  ASSERT_TRUE(myApps[0].theRemainingPaths.empty());
  ASSERT_EQ(8, myApps[0].theVisits);
  ASSERT_EQ(2, myApps[0].theAllocated.size());
  ASSERT_EQ(1, myApps[0].theAllocated[2].size());
  ASSERT_EQ(CapacityNetwork::AppDescriptor::Hops({1, 2}),
            myApps[0].theAllocated[2][0].theHops);
  ASSERT_EQ(1, myApps[0].theAllocated[3].size());
  ASSERT_EQ(CapacityNetwork::AppDescriptor::Hops({4, 3}),
            myApps[0].theAllocated[3][0].theHops);
  ASSERT_TRUE(myApps[1].theRemainingPaths.empty());
  ASSERT_EQ(4, myApps[1].theVisits);
  ASSERT_EQ(1, myApps[1].theAllocated.size());
  ASSERT_EQ(1, myApps[1].theAllocated[3].size());
  ASSERT_EQ(CapacityNetwork::AppDescriptor::Hops({2, 3}),
            myApps[1].theAllocated[3][0].theHops);

  double myGrossRate = 0;
  double myNetRate   = 0;
  for (const auto& myApp : myApps) {
    myGrossRate += myApp.grossRate();
    myNetRate += myApp.netRate();
  }
  ASSERT_FLOAT_EQ(5, myGrossRate);
  ASSERT_FLOAT_EQ(2.5, myNetRate);
  ASSERT_FLOAT_EQ(7, myNetwork.totalCapacity());
  const auto myWeights = myNetwork.weights();
  ASSERT_EQ(3, myWeights.size());
  ASSERT_EQ(0, std::get<0>(myWeights[0]));
  ASSERT_EQ(1, std::get<1>(myWeights[0]));
  ASSERT_FLOAT_EQ(1.9, std::get<2>(myWeights[0]));
  ASSERT_EQ(2, std::get<0>(myWeights[1]));
  ASSERT_EQ(3, std::get<1>(myWeights[1]));
  ASSERT_FLOAT_EQ(2.1, std::get<2>(myWeights[1]));
  ASSERT_EQ(4, std::get<0>(myWeights[2]));
  ASSERT_EQ(3, std::get<1>(myWeights[2]));
  ASSERT_FLOAT_EQ(3, std::get<2>(myWeights[2]));

  // consume the remaining capacity
  myApps = std::vector<CapacityNetwork::AppDescriptor>({
      {0, {1, 2, 3, 4}, 1}, // only 0->1 is still available
      {2, {0, 1, 3, 4}, 1}, // same for 2->3
      {4, {0, 1, 2, 3}, 1}, // same for 4->3
  });
  myNetwork.route(myApps, 0.1, 99);
  ASSERT_EQ(3, myApps.size());
  ASSERT_EQ(1, myApps[0].theAllocated.size());
  ASSERT_EQ(1, myApps[1].theAllocated.size());
  ASSERT_EQ(1, myApps[2].theAllocated.size());
  ASSERT_EQ(1, myApps[0].theAllocated[1].size());
  ASSERT_EQ(1, myApps[1].theAllocated[3].size());
  ASSERT_EQ(1, myApps[2].theAllocated[3].size());
  ASSERT_EQ(58, myApps[0].theVisits);
  ASSERT_EQ(64, myApps[1].theVisits);
  ASSERT_EQ(92, myApps[2].theVisits);
  ASSERT_FLOAT_EQ(0, myNetwork.totalCapacity());
}

TEST_F(TestCapacityNetwork, test_add_capacity_to_edge) {
  CapacityNetwork myNetwork(exampleEdgeWeights());
  myNetwork.measurementProbability(0.5);

  // add one (admissible) flow
  const auto myCapacityTot = myNetwork.totalCapacity();
  std::vector<CapacityNetwork::FlowDescriptor> myFlows({
      {0, 3, 1.0},
  });
  myNetwork.route(myFlows);
  ASSERT_EQ(1, myFlows.size());
  ASSERT_EQ(std::vector<unsigned long>({1, 2, 3}), myFlows[0].thePath);
  ASSERT_FLOAT_EQ(4, myFlows[0].theGrossRate);
  ASSERT_EQ(myCapacityTot - myFlows[0].thePath.size() * myFlows[0].theGrossRate,
            myNetwork.totalCapacity());

  // re-add the capacity along the path
  myNetwork.addCapacityToPath(0, {1, 2, 3}, myFlows[0].theGrossRate);
  ASSERT_EQ(myCapacityTot, myNetwork.totalCapacity());

  // re-add an identical flow
  std::vector<CapacityNetwork::FlowDescriptor> myOtherFlows({
      {0, 3, 1.0},
  });
  myNetwork.route(myOtherFlows);

  // add capacity partially
  ASSERT_EQ(myFlows[0].thePath, myOtherFlows[0].thePath);
  myNetwork.addCapacityToPath(2, {3}, myOtherFlows[0].theGrossRate);
  ASSERT_EQ(myCapacityTot - 2 * myOtherFlows[0].theGrossRate,
            myNetwork.totalCapacity());

  // remove too much capacity
  ASSERT_THROW(myNetwork.addCapacityToPath(2, {3}, -10), std::runtime_error);

  // non-existing edge
  ASSERT_THROW(myNetwork.addCapacityToPath(1, {0}, 1), std::runtime_error);

  ASSERT_NO_THROW(myNetwork.addCapacityToPath(0, {1}, 1));
}

} // namespace qr
} // namespace uiiit
