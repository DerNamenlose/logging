C++-based logging framework. This framework offers a C++ approach to writing
log messages. It uses modern compiler features to optimize away trace output
in release code and offers a rich set of customizable features like multithread
support, hierarchical loggers, different character sets and even wholly 
different log targets.

*Note:* This library uses C++11 features. If you are for some reason limited
to a compiler without a decent C++11 support please have a look at
https://bitbucket.org/namenlos/logging, which provides a roughly similar
feature set for older compilers.