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

#include "QuantumRouting/networkfactory.h"

#include "QuantumRouting/poissonpointprocess.h"
#include "QuantumRouting/qrutils.h"
#include "Support/random.h"

#include <glog/logging.h>

#include <exception>
#include <fstream>
#include <stdexcept>

namespace uiiit {
namespace qr {

std::unique_ptr<CapacityNetwork>
makeCapacityNetworkPpp(support::RealRvInterface& aEprRv,
                       const std::size_t         aSeed,
                       const double              aMu,
                       const double              aGridLength,
                       const double              aThreshold,
                       const double              aLinkProbability,
                       std::vector<Coordinate>&  aCoordinates) {
  const auto MANY_TRIES = 1000000u;

  auto myPppSeed = aSeed;
  for (unsigned myTry = 0; myTry < MANY_TRIES; myTry++) {
    auto myCoordinates =
        PoissonPointProcessGrid(aMu, myPppSeed, aGridLength, aGridLength)();
    const auto myEdges =
        findLinks(myCoordinates, aThreshold, aLinkProbability, aSeed);
    if (qr::bigraphConnected(myEdges)) {
      myCoordinates.swap(aCoordinates);
      return std::make_unique<qr::CapacityNetwork>(myEdges, aEprRv, true);

    } else {
      VLOG(1) << "graph with seed " << myPppSeed << " not connected, try again";
      myPppSeed += 1000000;
    }
  }

  throw std::runtime_error("Could not find a connected network after " +
                           std::to_string(MANY_TRIES) + " tries");
}

std::unique_ptr<CapacityNetwork>
makeCapacityNetworkGraphMl(support::RealRvInterface& aEprRv,
                           std::ifstream&            aGraphMl,
                           std::vector<Coordinate>&  aCoordinates) {
  const auto myEdges = findLinks(aGraphMl, aCoordinates);
  for (const auto& myEdge : myEdges) {
    VLOG(2) << '(' << myEdge.first << ',' << myEdge.second << ')';
  }
  if (qr::bigraphConnected(myEdges)) {
    return std::make_unique<qr::CapacityNetwork>(myEdges, aEprRv, true);
  }

  throw std::runtime_error("The GraphML network is not fully connected");
}

} // namespace qr
} // namespace uiiit
