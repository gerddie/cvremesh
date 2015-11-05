# cvremesh

This software implements (partially) the paper


    Valette, S. and Chassery, J.-M., "Approximated Centroidal Voronoi
    Diagrams for Uniform Polygonal Mesh Coarsening", 
    Computer Graphics Forum, 23: 381–389, 2004
    doi: 10.1111/j.1467-8659.2004.00769.x

Not implemented is the topological correction that is only described
in the paper as "flipping some triangles".

In addition to the centroida voronoi energy used for re-meshing,
a perimeter/area energy is implemented. For details how to use the library
please look at the implementation file *src/main.c*. 


The implementation requires the GNU Triangulated Surface Library

    http://gts.sourceforge.net/

and GNU make.

The library and test cases can be compiled by

    ./autogen.sh 
    make





