# SoN - sea of nodes IR

The project represents realization JIT and AOT compiler based on sea of nodes intermediate representation and is created with learning purposes.

## Terminology

**Loop** - LLVM definition.

## Project structure

* Node.hpp / Node.cpp - definition of sea of nodes intermediate representation.
* Function.cpp / Function.hpp - graph structure, that manages nodes memory and provides basic algorithms: DFS, RPO, dominator tree building (Lengauer-Tarjan), graph linearization. Besides a graph validation is implemented based on the instruction visitor.
* InstVisitor.hpp - visit every node in the graph with calling of customized handlers.
* DomTree.hpp - provide interface for checking of domination. The implementation is based on immediate dominators tree.
* Loop.hpp - LoopInfo structure, that provide information about loops (including nested loops). As loop we consider LLVM loop.


## References

### Sea of nodes
* [A simple graph-based intermediate representation](https://dl.acm.org/doi/10.1145/202530.202534)
* [Semantic reasoning about the sea of nodes](https://dl.acm.org/doi/10.1145/3178372.3179503)


### Dominator tree
* [A fast algorithm for finding dominators in a flowgraph](https://www.cs.princeton.edu/courses/archive/fall03/cs528/handouts/a%20fast%20algorithm%20for%20finding.pdf)