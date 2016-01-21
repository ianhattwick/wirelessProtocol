# wirelessProtocol

Preliminary documentation at:
https://docs.google.com/document/d/1V11XvFumxHX2onc_xKBniX4ng0pU1wuHFccsd6tmu9w/edit?usp=sharing
__________

This doc documents code for wireless communication between a microcontroller and PC, and also provides an explanation for the considerations you will face implementing this kind of communication. 

Goals:
reliable bidirectional communication for serial communication 
dual approach - streaming vs acknowledgment based
low-latency or fixed latency, or some other way of dealing with timing

In this doc:
An overview of a wireless communication system
An overview of SLIP encoding
A description of the data protocol
Documentation of C code for a microcontroller to SLIP encode and format the data
Documentation of a Max patch for doing the same on the computer
Documentation of settings for different wireless transceivers
Todo list, suggestions for future functionality, etc.
