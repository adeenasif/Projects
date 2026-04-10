## Virtual Machine Emulator

A comprehensive virtual machine emulator with arithmetic operation, string manipulation and memory management, matrix operations, and assembly instruction execution capabilities.

### Completed Features

- **Core Virtual Machine Architecture**
  - 6 General Purpose Registers (R0-R5)
  - Program Counter (EIP equivalent)
  - Status Flags (ZF, SF, OF, CF)
  - Call Stack for function calls
  - Data Stack for operations

- **Instruction Set Architecture (ISA) Used need it in our own language**
  - Arithmetic: ADD, SUB, IMUL, IDIV, MOV
  - Memory: ALLOC, FREE, STORE, LOAD
  - Control Flow: CMP, JMP, JE, JNE, JL, JLE, CALL, RET
  - I/O: PRINT_STR, READ_INT, WRITE_INT, READ_CHAR
  - Matrix Operations: MATRIX_ALLOC_MEM, INPUT_MATRIX_A/B, MATRIX_ADD_OPERATION
  - System: CLRSC, HALT, CDQ

- **User Interface Modules**
  - Main menu system with modular navigation
  - Calculator module framework (fully implemented)
  - String operations module framework
  - Memory management module (fully implemented)
  - Proper module switching (option 5 returns to main menu)

- **Calculator Operations Module**
  - Addition procedure implementation
  - Subtraction procedure implementation  
  - Multiplication procedure implementation
  - Division procedure implementation
  - With option to reuse previous result value

- **Memory Management**
  - Virtual memory allocation and deallocation
  - Matrix memory management (A, B, C matrices)
  - Memory read/write operations
  - Address calculation for matrix elements

- **Matrix Operations Module**
  - Dynamic matrix allocation
  - Matrix input/output
  - Matrix addition
  - Memory-efficient storage using base addresses
 
- **String Operations Module**
  - String reverse procedure
  - String concatenation procedure
  - String copy procedure
  - String comparison procedure

### Remaining Implementation
- Color The Output
- Write our own custom ISA

## Project Structure File
- AssemblyCode.asm : Holds the original assembly code
- VirtualEmulator.cpp : Holds the original emulator code
- Virtual_Emulator_GrpPrototype.cpp : A simple prototype to get an idea on how the program will flow<br>
- Folder (Assembly Code): Holds the individual code of calculator, and memory .asm files.
- Folder (Emulator Codes): Holds the individual code of calculator, and memory .cpp files.
