A Qt-based application for my Data Structure and Algorithms course.

It mimic a minimal desktop environment (user space) of an operating system with its own DSL (domain-specific language) called SudoScript.
Sudo is a parody of it being a pseudo operating system (not classified as real operating system due to lack of actual kernel that communicates with hardware) and sudo being a linux command.

The idea is that it will sit on a minimal linux kernel (such as Alpine Linux) so that it can be used on any device.

Features:
DSL - It uses a ASM like structure:
- NEW VAR NUM x 10 (Creates a number variable "x" that holds a value of 10)
- NEW VAR BOOL y false (A boolean y)
- MATH ADD x 15 (Adds "x" (10) and a literal value (15))
- LGC EQ y x 10 (Compares "x" and 10, puts the value in "y")

APP Development - Packages scripts into applications that can be used by the environment.
Turing Complete - It has the basic components for logic such as control flow and loops.
