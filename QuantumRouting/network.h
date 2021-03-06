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

#pragma once

#include "Support/macros.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

namespace uiiit {
namespace qr {

/**
 * @brief Abstract quantum network.
 *
 * Objects of this type cannot be created directly.
 * Instances are not copyable.
 */
class Network
{
  MOVEONLY(Network);

 public:
  virtual ~Network() = default;

 protected:
  Network() {
    // noop
  }

  /**
   * @brief Provide some utilities for manipulating graphs.
   *
   * @tparam GRAPH The boost graph library type used.
   */
  template <class GRAPH>
  class Utils final
  {
   public:
    using Edge   = typename GRAPH::edge_descriptor;
    using Vertex = typename GRAPH::vertex_descriptor;

    //! Add a directed edge, test it, and set weight.
    template <class WEIGHT>
    static Edge
    addEdge(GRAPH& aGraph, Vertex aSrc, Vertex aDst, WEIGHT aWeight) {
      Edge myEdge;
      auto mySuccess = false;

      boost::tie(myEdge, mySuccess) = boost::add_edge(aSrc, aDst, aGraph);
      assert(mySuccess);

      boost::get(boost::edge_weight, aGraph, myEdge) = aWeight;

      return myEdge;
    }

    class EdgeWeightWrite
    {
     public:
      EdgeWeightWrite(const GRAPH& aGraph)
          : theGraph(aGraph) {
        // noop
      }
      template <class VertexOrEdge>
      void operator()(std::ostream& aOut, const VertexOrEdge& aEdge) const {
        aOut << "[label=\"" << boost::get(boost::edge_weight, theGraph, aEdge)
             << "\"]";
      }

     private:
      const GRAPH& theGraph;
    };

    //! Write to a dot file.
    static void toDot(const GRAPH& aGraph, const std::string& aFilename) {

      std::ofstream myOut(aFilename);
      if (not myOut) {
        throw std::runtime_error("could not open file for writing: " +
                                 aFilename);
      }
      write_graphviz(
          myOut, aGraph, boost::default_writer(), EdgeWeightWrite(aGraph));
    }
  };
}; // namespace qr

} // namespace qr
} // end namespace uiiit
