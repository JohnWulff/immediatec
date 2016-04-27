# _immediatec_
_immediate C_ - _iC_ for short is a declarative extension of the procedural language _C_ - useful for control and robotics as well as dealing with events generated in a GUI.

It utilizes the syntax of _C_ to give meaning to statements that have no semantic support in _C_. In addition to standard variables, which are modified by the flow of instructions, _iC_ provides so called '_immediate_' variables, whose values are updated, whenever a change of input calls for an immediate change in output. An efficient Data Flow technique implements this strategy.

_iC_ provides programmers with built in operators, whose function is closely modelled on integrated circuits. The name _iC_ is a reminder of this fact. Logical _AND_, _OR_, _EXCLUSIVE-OR_ and _NOT_ as well as D flip-flops, SR flip-flops and many others are implemented in such a way, that their use in _iC_ follows the same design rules, which apply to their hardware counterparts. These rules have led to a well-developed hardware technology, whose effectiveness is demonstrated by the success of today's complex computer hardware. Particularly the concept of clocked functions plays an important role in the language _iC_. It gives the same protection against timing races in _iC_ programs, as it provides for hardware IC designs. But _iC_ is not a hardware description language nor a simulation language – it provides the functionality and some of the speed of field-programmable gate arrays to _C_ programmers.

Writing programs in the language _iC_ has the added quality, that many simple ideas and relationships, which should result in direct actions, can be written down _immediately_ in one line. The coding of call back routines and other overhead is not required. It was this thought, which also prompted the name "_immediate C_".

_immediate C_ is built with the usual _configure_ – _make_ – _make test_ – _make install_ commands. The src/README explains the details and how to make, run and debug iC apps.

_immediate C_ can execute direct Input/Output on a Raspberry Pi computer. Drivers are included for digital I/O on GPIO’s and PiFace cards as well as a driver for PWM analog output on GPIO’s and analog input from an MCP3008 A/D converter. See src/README.RPi for details.

The _immediate C_ Reference Manual doc/iC.pdf describes the _iC_ language and its built-in Functions in detail. The _iC_ Programming Manual is currently being written.
