Dunnart
=======

Dunnart is a prototype constraint-based diagram editor. It includes standard
diagram editing capabilities as well as advanced features such as
constraint-based geometric placement tools (alignment, distribution,
separation, non-overlap, and page containment), automatic object-avoiding
poly-line connector routing, and continuous network layout.

Note: Dunnart is alpha software.  It is in the process of being rewritten, and
while many things work it still contains many bug and lacks key features.  Use
it at your own risk!

### Downloads ###

Nightly alpha builds for Mac OS X, Linux and Windows can be found here:

 *  [http://www.csse.monash.edu.au/~mwybrow/dunnart/nightlies/][nightlies]

### How to cite? ###

If you refer to Dunnart in your own research or publications, please cite 
the following reference:

 *  Tim Dwyer, Kim Marriott, and Michael Wybrow.  
    Dunnart: A constraint-based network diagram authoring tool.  
    In Proceedings of 16th International Symposium on Graph Drawing (GD'08),  
    LNCS 5417, pages 420–431. Springer-Verlag, 2009. [DOI][doi]


### Further information ###

The concept behind Dunnart is to use constraint-based methods to free users
from manually maintaining relationships within their diagrams. These could
include geometric relationships, non-overlap, object-avoiding connector routes
or graph layout. In Dunnart, the user can define these features of their layout
and they will be automatically maintained throughout further editing. The user
can override or alter these relationships at any time.

Dunnart is a research prototype. As such, it lacks some of the features and
polish of a professional diagram editor. any of the significant features of
Dunnart are implemented in the open-source Adaptagrams constraint layout
libraries and are described in my publications and/or PhD thesis.

![Dunnart screenshot](https://raw.github.com/mjwybrow/dunnart/master/libdunnartcanvas/doc/DunnartScreen.png)

### Building ###

Dunnart depends only on the [Qt cross-platform application and UI 
framework][qt].  All other components are included in the repository.  
Dunnart can be built by issuing the following commands:

    qmake -recursive -config release dunnart.pro  
    make


### libdunnartcanvas - Reusable constraint-based canvas component ###

Dunnart is built using `libdunnartcanvas`, a reusable Qt-based canvas widget
that extends our low level Adaptagrams constraint-based layout libraries and
provides interactive constrain-based layout and placement tools.  See the image
below for an overview of the architecture.

Adaptagrams is a library of tools and reusable code for adaptive diagramming
applications, for example: drawing tools, automated document and diagram
layout, smart presentation software, graph drawing, chart layout, etc.

![Dunnart architecture](https://raw.github.com/mjwybrow/dunnart/master/libdunnartcanvas/doc/DunnartStructure.png)

### Authors ###

Dunnart is written and maintained by [Michael Wybrow][mw]. The latest version of the source code can be found here:

 *  [https://github.com/mjwybrow/dunnart/][dunnartrepo]

The Adaptagrams layout algorithms are developed by [Kim Marriott][km] and members of his Adaptive Diagrams Research Group at [Monash University][monash] in Melbourne, Australia.  The Adaptagrams libraries were originally written by [Tim Dwyer][td] and [Michael Wybrow][mw].

The latest Adaptagrams source code can be found in the github repository:

 *  [https://github.com/mjwybrow/adaptagrams/][adaptagramsrepo]

### License ###

Dunnart and `libdunnartcanvas` are available as open source under the terms of 
the GNU GPL.  We are also open to dual-licensing `libdunartcanvas` for 
commercial use.

Copyright (C) 2003-2007  Michael Wybrow  
Copyright (C) 2006-2011  Monash University

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


[td]: http://www.csse.monash.edu.au/~tdwyer/
[km]: http://www.csse.monash.edu.au/~kmarriott/
[mw]: http://www.csse.monash.edu.au/~mwybrow/
[monash]: http://wwww.csse.monash.edu.au/
[dunnart]: http://www.dunnart.org/
[qt]: http://qt.nokia.com/
[adaptagramsrepo]: https://github.com/mjwybrow/adaptagrams/
[dunnartrepo]: https://github.com/mjwybrow/dunnart/
[repo]: https://github.com/mjwybrow/adaptagrams/
[nightlies]:http://www.csse.monash.edu.au/~mwybrow/dunnart/nightlies/
[doi]:http://dx.doi.org/10.1007/978-3-642-00219-9_41
