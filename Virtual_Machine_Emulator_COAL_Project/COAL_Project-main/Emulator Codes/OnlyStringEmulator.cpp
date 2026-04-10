#include <iostream>           // Standard input/output stream operations
#include <fstream>            // File stream operations for reading/writing files
#include <sstream>            // String stream operations for string parsing
#include <unordered_map>      // Hash table container for key-value pairs
#include <vector>             // Dynamic array container for sequences
#include <string>             // String class and character operations
#include <algorithm>          // Algorithms library (sort, find, transform)
#include <stack>              // Stack container (Last-In-First-Out)
#include <cstdlib>            // General utilities (memory, conversions, exit)
#include <conio.h>            // For getch

using namespace std;          // Use standard namespace to avoid std:: prefix

class VirtualMachine {
    private:

        unordered_map<string, int> registers;           // Storage for CPU registers (name-value pairs)
        unordered_map<string, string> stringMemory;     // Storage for named string constants
        vector<string> programMemory;                   // Stores program instructions as strings
        unordered_map<string, int> labels;              // Maps label names to instruction addresses
        int programCounter;                             // Tracks current instruction position [EIP equivalent]
        bool running;                                   // VM execution state (true=running, false=stopped)
        
        bool ZF, SF, OF, CF;                            // Status flags: Zero, Sign, Overflow, Carry
        stack<int> callStack;                           // Stores return addresses for CALL/RET instructions
        stack<int> dataStack;                           // General purpose stack for data operations
        unordered_map<int, int> virtualMemory;          // Simulates memory address space (address-value pairs)
        int nextMemoryAddress = 0x1000;                 // Next available memory address (starts at 0x1000)
        unordered_map<string, int> matrixPointers;      // Stores matrix names and base memory addresses
        int matrixSize;                                 // Size dimension for allocated matrices
        bool matrixAllocated;                           // Tracks if matrix memory is currently allocated
        
        int firstNum, secondNum, remainder, prevResult; // Calculator variables
        bool usePrev;                                   // Flag to use previous result
        
        // String buffers
        unordered_map<string, int> stringBuffers;       // Maps buffer names to memory addresses
        unordered_map<string, int> stringVariables;     // For DWORD variables (addresses, lengths)
               
    public:
        VirtualMachine() {                               // Constructor - initializes virtual machine state
            for (int i = 0; i < 6; i++) {                // Loop to initialize 6 general purpose registers
                registers["R" + to_string(i)] = 0;       // Initialize register R0-R5 with value 0  [R0= EAX, R1 = EBX, R2 = ECX, R3 = EDX, R4 = ESI, R5 = EDI]
            }
            programCounter = 0;                          // Set program counter to start at first instruction [PC = EIP]
            running = true;                              // Set VM execution state to running
            ZF = false;                                  // Initialize Zero Flag to false
            SF = false;                                  // Initialize Sign Flag to false  
            OF = false;                                  // Initialize Overflow Flag to false
            CF = false;                                  // Initialize Carry Flag to false
            matrixSize = 0;                              // Initialize matrix size to zero (no allocation)
            matrixAllocated = false;                     // Set matrix allocated flag to false
            InitializeStringMemory();                    // Call method to set up predefined string constants
            
            // Memory Initialiser
            matrixPointers["matrixA"] = 0;               // Initialize matrixA pointer to 0 (unallocated)
            matrixPointers["matrixB"] = 0;               // Initialize matrixB pointer to 0 (unallocated)
            matrixPointers["matrixC"] = 0;               // Initialize matrixC pointer to 0 (unallocated)

            // Initialize calculator variables
            prevResult = 0;                              // Store previous result
            usePrev = false;                             // Flag to use previous result
            firstNum = 0;                                // First operand
            secondNum = 0;                               // Second operand
            remainder = 0;                               // Remainder
            
            // Initialize string buffers (allocate memory addresses for them)
            InitializeStringBuffers();
        }
        
        void InitializeStringBuffers() {
            // Allocate memory for string buffers (100 bytes each, 200 for result)
            stringBuffers["string1"] = AllocateVirtualMemory(100);           // 100 bytes
            stringBuffers["string2"] = AllocateVirtualMemory(100);           // 100 bytes
            stringBuffers["resultString"] = AllocateVirtualMemory(200);      // 200 bytes for concatenation
            stringBuffers["reversedString"] = AllocateVirtualMemory(100);    // 100 bytes
            stringBuffers["copiedString"] = AllocateVirtualMemory(100);      // 100 bytes
            
            // Initialize string variables (pointers and lengths)
            stringVariables["string1Addr"] = 0;
            stringVariables["string2Addr"] = 0;
            stringVariables["string1Length"] = 0;
            stringVariables["string2Length"] = 0;
            
            cout << "=== String Buffers Initialized ===" << endl;
            cout << "string1 at address: 0x" << hex << stringBuffers["string1"] << dec << endl;
            cout << "string2 at address: 0x" << hex << stringBuffers["string2"] << dec << endl;
            cout << "resultString at address: 0x" << hex << stringBuffers["resultString"] << dec << endl;
            cout << "reversedString at address: 0x" << hex << stringBuffers["reversedString"] << dec << endl;
            cout << "copiedString at address: 0x" << hex << stringBuffers["copiedString"] << dec << endl;
        }
        
        void InitializeStringMemory() {                                 // Method to set up predefined string messages
            stringMemory["welcomeMsg"] = "=== Virtual Machine with String Manipulation ===\n";
            stringMemory["menuPrompt"] = "Please select an option:\n1. String Operations \n2. Exit Program\nEnter your choice (1-2): ";
            stringMemory["invalidChoice"] = "Invalid choice! Please enter 1-4.\n";
            stringMemory["continueMsg"] = "Press any key to clear the screen...\n";
            
            // String section
            stringMemory["stringTitle"] = "===== String Operations Module =====\n";
            stringMemory["stringMenu"] = "Select string operation:\n1. String Reverse\n2. String Concatenation\n3. Copy String\n4. Compare Strings\n5. Return to Main Menu\nEnter your choice (1-5): ";
            stringMemory["stringPrompt1"] = "Enter first string: ";
            stringMemory["stringPrompt2"] = "Enter second string: ";
            stringMemory["originalStr"] = "Original string: ";
            stringMemory["reversedStr"] = "Reversed string: ";
            stringMemory["concatResult"] = "Concatenated string: ";
            stringMemory["copyResult"] = "Copied string: ";
            stringMemory["copySuccess"] = "String successfully copied to new variable!\n";
            stringMemory["compareEqual"] = "Strings are EQUAL!\n";
            stringMemory["compareNotEqual"] = "Strings are NOT equal!\n";
            
            // Error messages
            stringMemory["emptyStringMsg"] = "Error: Empty string detected!\n";
            stringMemory["invalidChoiceMsg"] = "Error: Invalid choice. Please try again.\n";
            stringMemory["inputBuffer"] = "";
            stringMemory["ClearCharacter"] = "Z";
        }

        int AllocateVirtualMemory(int size) {                           // Allocates contiguous block in virtual memory
            int address = nextMemoryAddress;                        // Get next available memory address
            for (int i = 0; i < size; i++) {                        // Loop through each element to allocate
                virtualMemory[address + i] = 0;                     // Initialize memory location to zero (byte-aligned)
            }
            nextMemoryAddress += size;                              // Update next available address
            cout << "  -> Allocated " << size << " bytes at address 0x" << hex << address << dec << endl;
            return address;                                         // Return base address of allocated block
        }

        void FreeVirtualMemory(int address, int size) {                 // Deallocates memory block at given address
            for (int i = 0; i < size; i++) {                            // Loop through each element to free
                virtualMemory.erase(address + i);                       // Remove memory entry from virtualMemory map
            }
            cout << "  -> Freed memory at address 0x" << hex << address << dec << endl;
        }

        int ReadVirtualMemory(int address) {                            // Reads value from virtual memory address
            if (virtualMemory.find(address) != virtualMemory.end()) {   // Check if address exists in memory
                return virtualMemory[address];                          // Return value stored at memory address
            }
            // cout << "  -> ERROR: Memory read from invalid address 0x" << hex << address << dec << endl;
            return 0;                                                   // Return 0 for invalid memory access
        }

        void WriteVirtualMemory(int address, int value) {               // Writes value to virtual memory address
            virtualMemory[address] = value;                             // Store value in virtualMemory map at address
        }
        
        // Helper to write string to memory (byte by byte)
        void WriteStringToMemory(int baseAddress, const string& str) {
            for (size_t i = 0; i < str.length(); i++) {
                WriteVirtualMemory(baseAddress + i, (int)(unsigned char)str[i]);
            }
            WriteVirtualMemory(baseAddress + str.length(), 0);  // Null terminator
        }
        
        // Helper to read string from memory
        string ReadStringFromMemory(int baseAddress, int maxLength = 200) {
            string result = "";
            for (int i = 0; i < maxLength; i++) {
                int charValue = ReadVirtualMemory(baseAddress + i);
                if (charValue == 0) break;  // Null terminator
                result += (char)charValue;
            }
            return result;
        }
        
        // Helper to get address of string buffer by name
        int GetStringBufferAddress(const string& bufferName) {
            if (stringBuffers.find(bufferName) != stringBuffers.end()) {
                return stringBuffers[bufferName];
            }
            cout << "  -> ERROR: String buffer '" << bufferName << "' not found!" << endl;
            return 0;
        }

        int GetMatrixElementAddress(int baseAddress, int row, int col, int size) {
            return baseAddress + (row * size + col) * 4;                // Calculate address: base + (row*size + col) * 4 bytes
        }
        
        void LoadProgram(const string& filename) {                      // Loads assembly program from file into memory
            ifstream file(filename);                                    // Open input file stream for reading
            string line;                                                // Store each line read from file
            vector<string> tempProgram;                                 // Temporary storage for program instructions
            int lineNum = 0;                                            // Track current line number during loading
            
            cout << "=== LOADING PROGRAM ===" << endl;                  // Print loading header
            
            while (getline(file, line)) {                               // Read file line by line until EOF
                size_t commentPos = line.find(';');                     // Find position of comment delimiter
                if (commentPos != string::npos) {                       // Check if comment exists in line
                    line = line.substr(0, commentPos);                  // Remove comment portion from line
                }
                line.erase(0, line.find_first_not_of(" \t"));           // Remove leading whitespace and tabs
                line.erase(line.find_last_not_of(" \t") + 1);           // Remove trailing whitespace and tabs
                
                if (!line.empty()) {                                    // Check if line is not empty after cleaning
                    cout << "Line " << lineNum << ": " << line << endl; // Print processed line
                    tempProgram.push_back(line);                        // Add instruction to temporary program storage
                    
                    if (line.back() == ':') {                           // Check if line ends with colon (label definition)
                        string label = line.substr(0, line.length() - 1); // Extract label name without colon
                        labels[label] = lineNum;                        // Store label with its line number in labels map
                        cout << "  -> LABEL FOUND: '" << label << "' at position " << lineNum << endl;
                    }
                    lineNum++;                                          // Increment line counter for next instruction
                }
            }
            file.close();                                               // Close the input file
            programMemory = tempProgram;                                // Copy temporary program to program memory
            cout << "\n=== PROGRAM LOADED ===" << endl;                 // Print loading completion header
            cout << "Total instructions: " << programMemory.size() << endl; // Display instruction count
            cout << "Labels found: " << labels.size() << endl;          // Display number of labels found
            for (auto& label : labels) {                                // Iterate through all labels in map
                cout << "  " << label.first << " -> line " << label.second << endl; // Print label mapping
            }
            cout << "======================\n" << endl;                 // Print section footer
        }
        
        void run() {                                                    // Main VM execution loop
            programCounter = 0;                                         // Initialize program counter [PC = EIP] to start of program
            
            while (programCounter < programMemory.size() && running) {  // Loop while within bounds and VM running
                string instruction = programMemory[programCounter];     // Fetch instruction at current PC
                cout << "\n[PC=" << programCounter << "] Executing: " << instruction << endl; // Display execution info
                vector<string> tokens = Tokenize(instruction);          // Split instruction into tokens (opcode, operands)
                
                if (!tokens.empty()) {                                  // Check if instruction has valid tokens
                    string opcode = tokens[0];                          // Extract first token as opcode
                    if (opcode.back() == ':') {                         // Check if current line is a label definition
                        programCounter++;                               // Skip label line (no execution needed)
                        continue;                                       // Move to next instruction
                    }
                    
                    if (opcode == "CALL") {                             // Handle function CALL instruction
                        if (tokens.size() > 1) {                        // Verify CALL has target label operand
                            string label = tokens[1];                   // Extract label name from tokens
                            if (labels.find(label) != labels.end()) {   // Check if label exists in label map
                                callStack.push(programCounter + 1);     // Push return address (next instruction) onto stack
                                programCounter = labels[label];         // Jump PC to label address
                                cout << "  -> CALL: jumping to " << label << " at line " << programCounter << endl;
                                continue;                               // Skip PC increment for direct jump
                            } else {
                                cout << "  -> ERROR: Label '" << label << "' not found!" << endl; // Label error
                            }
                        }
                    } else if (opcode == "RET") {                       // Handle return from function call
                        if (!callStack.empty()) {                       // Verify call stack has return address
                            int returnAddress = callStack.top();        // Get return address from stack top
                            callStack.pop();                            // Remove return address from stack
                            programCounter = returnAddress;             // Jump PC back to return address
                            cout << "  -> RET: returning to line " << programCounter << endl;
                            continue;                                   // Skip PC increment for direct jump
                        } else {
                            cout << "  -> ERROR: RET with empty call stack!" << endl; // Stack underflow error
                        }
                    }
                }
                bool shouldIncrementPC = executeInstruction(instruction); // Execute instruction, get PC increment flag
                if (shouldIncrementPC) { programCounter++; }              // Check if PC should advance to next instruction (if yes increment)
                
                if (programCounter >= programMemory.size()) {             // Check if PC reached end of program memory
                    cout << "Program reached end." << endl;               // Print program completion message
                    break;                                                // Exit execution loop
                }
            }
        }
        
        bool executeInstruction(const string& instruction) {            // Execute single instruction, return whether to increment PC
            vector<string> tokens = Tokenize(instruction);              // Split instruction into opcode and operands
            if (tokens.empty()) return true;                            // Return true for empty lines (increment PC)
            
            string opcode = tokens[0];                                  // Extract instruction mnemonic (first token)
            bool incrementPC = true;                                    // Default: move to next instruction after execution
            
            // ========== STACK OPERATIONS ==========
            if (opcode == "PUSH") {                                 // Check if instruction is PUSH
                if (tokens.size() > 1) {                                // Verify that at least 2 tokens exist (PUSH, operand)
                    // Remove colon if present (e.g., "PUSH R0:" -> "PUSH R0")
                    string operand = tokens[1];                         // Extract the operand token
                    if (operand.back() == ':') {                        // Check if colon is present at the end
                        operand = operand.substr(0, operand.length() - 1); // Remove the trailing colon
                    }
                    
                    int value;                                          // Declare variable to hold the value to push
                    if (IsRegister(operand)) {                          // Check if operand is a register
                        value = registers[operand];                     // Get value from the register
                    } else if (IsVariable(operand)) {                   // Check if operand is a variable
                        value = GetVariableValue(operand);              // Get value from the variable
                    } else {                                            // Operand must be an immediate value
                        value = stoi(operand);                          // Convert string to integer
                    }
                    dataStack.push(value);                              // Push the value onto the data stack
                    cout << "  -> PUSH: value = " << value  << ", stack size = " << dataStack.size() << endl;
                }
            }
            else if (opcode == "POP") {                             // Check if instruction is POP
                if (tokens.size() > 1) {                                // Verify that at least 2 tokens exist (POP, destination)
                    // Remove colon if present
                    string operand = tokens[1];                         // Extract the destination operand token
                    if (operand.back() == ':') {                        // Check if colon is present at the end
                        operand = operand.substr(0, operand.length() - 1); // Remove the trailing colon
                    }
                    
                    if (IsRegister(operand)) {                          // Check if destination is a register
                        if (!dataStack.empty()) {                       // Check if the stack is not empty
                            registers[operand] = dataStack.top();       // Get top value from stack and store in register
                            dataStack.pop();                            // Remove the top value from the stack
                            cout << "  -> POP: " << operand << " = "  << registers[operand] << ", stack size = "  << dataStack.size() << endl;
                        } else {                                        // Stack is empty
                            cout << "  -> ERROR: Stack underflow!" << endl; // Print error message
                        }
                    }
                }
            }
            
            // ========== MEMORY MANAGEMENT INSTRUCTIONS ==========
            else if (opcode == "ALLOC") {                           // Allocate memory block instruction
                if (tokens.size() > 2 && IsRegister(tokens[1]) && IsRegister(tokens[2])) {
                    int size = registers[tokens[1]];                // Get size from source register
                    int address = AllocateVirtualMemory(size);      // Allocate memory of specified size
                    registers[tokens[2]] = address;                 // Store base address in destination register
                    cout << "  -> ALLOC: allocated " << size << " elements, address in " << tokens[2] << endl;
                }
            }
            else if (opcode == "FREE") {                            // Deallocate memory block instruction
                if (tokens.size() > 2 && IsRegister(tokens[1]) && IsRegister(tokens[2])) {
                    int address = registers[tokens[1]];             // Get base address from register
                    int size = registers[tokens[2]];                // Get size from register
                    FreeVirtualMemory(address, size);               // Free the memory block
                    cout << "  -> FREE: freed memory at address in " << tokens[1] << endl;
                }
            }
            else if (opcode == "STORE") {                           // Store value to memory instruction
                if (tokens.size() > 2) {
                    string addrToken = tokens[1];                   // Token containing memory address
                    string valueToken = tokens[2];                  // Token containing value to store
                    int address = 0;                                // Parsed memory address
                    int value = 0;                                  // Parsed value to store
                    
                    if (addrToken[0] == '[' && addrToken.back() == ']') {               // Check for direct memory addressing [address]
                        string addrStr = addrToken.substr(1, addrToken.length() - 2);   // Remove brackets
                        if (addrStr.substr(0, 2) == "0x") {                             // Check if hexadecimal address
                            address = stoi(addrStr.substr(2), 0, 16);                   // Convert hex string to integer
                        } else {
                            address = stoi(addrStr);                                    // Convert decimal string to integer
                        }
                    } else if (IsRegister(addrToken)) {                                 // Check if address is in register
                        address = registers[addrToken];                                 // Get address from register
                    }

                    if (IsRegister(valueToken)) {                                       // Check if value is in register
                        value = registers[valueToken];                                  // Get value from register
                    } else {
                        value = stoi(valueToken);                                       // Parse immediate value
                    }
                    WriteVirtualMemory(address, value);                                 // Write value to memory address
                    cout << "  -> STORE: value " << value << " to address 0x" << hex << address << dec << endl;
                }
            }
            else if (opcode == "LOAD") {                            // Load value from memory to register
                if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    string addrToken = tokens[2];                   // Token containing memory address
                    int address = 0;                                // Parsed memory address
                    
                    if (addrToken[0] == '[' && addrToken.back() == ']') {               // Direct memory addressing
                        string addrStr = addrToken.substr(1, addrToken.length() - 2);   // Remove brackets
                        if (addrStr.substr(0, 2) == "0x") {                             // Hexadecimal address
                            address = stoi(addrStr.substr(2), 0, 16);                   // Convert hex to integer
                        } else {
                            address = stoi(addrStr);                                    // Convert decimal to integer
                        }
                    } else if (IsRegister(addrToken)) {                                 // Address stored in register
                        address = registers[addrToken];                                 // Get address from register
                    }
                    
                    int value = ReadVirtualMemory(address);                             // Read value from memory
                    registers[tokens[1]] = value;                                       // Store value in destination register
                    cout << "  -> LOAD: from address 0x" << hex << address << " to " << tokens[1] << " = " << value << dec << endl;
                }
            }
            else if (opcode == "GET_ELEMENT_ADDR") {                // Calculate matrix element address
                if (tokens.size() > 5 && IsRegister(tokens[1]) && IsRegister(tokens[2]) && 
                    IsRegister(tokens[3]) && IsRegister(tokens[4]) && IsRegister(tokens[5])) {
                    int baseAddr = registers[tokens[2]];            // Matrix base address
                    int row = registers[tokens[3]];                 // Row index
                    int col = registers[tokens[4]];                 // Column index
                    int size = registers[tokens[5]];                // Matrix dimension size
                    int elementAddr = GetMatrixElementAddress(baseAddr, row, col, size); // Calculate address
                    registers[tokens[1]] = elementAddr;             // Store calculated address in destination register
                    cout << "  -> GET_ELEMENT_ADDR: [" << row << "][" << col << "] -> 0x" << hex << elementAddr << dec << endl;
                }
            }

            // ========== MATRIX OPERATIONS ==========
            else if (opcode == "MATRIX_ALLOC_MEM") {                // Allocate memory for all matrices
                cout << "  -> MATRIX_ALLOC_MEM: Allocating memory for matrices" << endl;
                if (matrixAllocated) {                              // Check if matrices already allocated
                    FreeAllMatrices();                              // Free existing matrices first
                }
                int totalElements = matrixSize * matrixSize;                      // Calculate total elements per matrix
                matrixPointers["matrixA"] = AllocateVirtualMemory(totalElements * 4); // Allocate matrix A (4 bytes per element)
                matrixPointers["matrixB"] = AllocateVirtualMemory(totalElements * 4); // Allocate matrix B
                matrixPointers["matrixC"] = AllocateVirtualMemory(totalElements * 4); // Allocate matrix C
                
                registers["R1"] = matrixPointers["matrixA"]; // Store matrix A address in R1
                registers["R2"] = matrixPointers["matrixB"]; // Store matrix B address in R2
                registers["R3"] = matrixPointers["matrixC"]; // Store matrix C address in R3
                
                matrixAllocated = true;                      // Set allocation flag
            }
            else if (opcode == "INPUT_MATRIX_A") {                  // Input values for matrix A
                cout << "  -> INPUT_MATRIX_A: Reading values for Matrix A" << endl;
                cout << stringMemory["matrixALabel"];               // Display input prompt
                InputMatrixValues(matrixPointers["matrixA"]);       // Read matrix values from user
            }
            else if (opcode == "INPUT_MATRIX_B") {                  // Input values for matrix B
                cout << "  -> INPUT_MATRIX_B: Reading values for Matrix B" << endl;
                cout << stringMemory["matrixBLabel"];               // Display input prompt
                InputMatrixValues(matrixPointers["matrixB"]);       // Read matrix values from user
            }
            else if (opcode == "MATRIX_ADD_OPERATION") {            // Perform matrix addition C = A + B
                cout << "  -> MATRIX_ADD_OPERATION: Computing C = A + B" << endl;
                int addrA = matrixPointers["matrixA"];              // Matrix A base address
                int addrB = matrixPointers["matrixB"];              // Matrix B base address
                int addrC = matrixPointers["matrixC"];              // Matrix C base address
                
                for (int i = 0; i < matrixSize; i++) {              // Iterate through rows
                    for (int j = 0; j < matrixSize; j++) {          // Iterate through columns
                        int addrElemA = GetMatrixElementAddress(addrA, i, j, matrixSize); // Get A[i][j] address
                        int addrElemB = GetMatrixElementAddress(addrB, i, j, matrixSize); // Get B[i][j] address
                        int addrElemC = GetMatrixElementAddress(addrC, i, j, matrixSize); // Get C[i][j] address
                        
                        int valA = ReadVirtualMemory(addrElemA);    // Read value from matrix A
                        int valB = ReadVirtualMemory(addrElemB);    // Read value from matrix B
                        WriteVirtualMemory(addrElemC, valA + valB); // Store sum in matrix C
                    }
                }
            }
            else if (opcode == "DISPLAY_MATRIX_A") {                // Display matrix A contents
                cout << "  -> DISPLAY_MATRIX_A" << endl;
                cout << stringMemory["matrixALabel"];               // Display matrix label
                DisplayMatrix(matrixPointers["matrixA"]);           // Show matrix values
            }
            else if (opcode == "DISPLAY_MATRIX_B") {                // Display matrix B contents
                cout << "  -> DISPLAY_MATRIX_B" << endl;
                cout << stringMemory["matrixBLabel"];               // Display matrix label
                DisplayMatrix(matrixPointers["matrixB"]);           // Show matrix values
            }
            else if (opcode == "DISPLAY_MATRIX_C") {                // Display matrix C contents
                cout << "  -> DISPLAY_MATRIX_C" << endl;
                DisplayMatrix(matrixPointers["matrixC"]);           // Show matrix values
            }
            else if (opcode == "FREE_ALL_MATRICES") {               // Deallocate all matrix memory
                cout << "  -> FREE_ALL_MATRICES" << endl;
                FreeAllMatrices();                                  // Free matrix memory
            }
            else if (opcode == "CHECK_ALLOCATED") {                 // Check if matrices are allocated
                cout << "  -> CHECK_ALLOCATED" << endl;
                if (!matrixAllocated) {                             // If no matrices allocated
                    cout << stringMemory["noMatrixMsg"];            // Display error message
                }
            }
            else if (opcode == "STORE_MATRIX_SIZE") {               // Store matrix size from R0
                cout << "  -> STORE_MATRIX_SIZE" << endl;
                matrixSize = registers["R0"];                       // Set matrix size from register R0 [EAX]
                cout << "  -> Matrix size set to " << matrixSize << "x" << matrixSize << endl;
            }

            // ========== I/O OPERATIONS ==========
            else if (opcode == "PRINT_STR") {                       // Print string from string memory OR buffer
                if (tokens.size() > 1) {
                    string strName = tokens[1];
                    
                    // Check if it's a predefined string message
                    if (stringMemory.find(strName) != stringMemory.end()) {
                        cout << stringMemory[strName];              // Output predefined string
                    }
                    // Check if it's a string buffer (read from virtual memory)
                    else if (stringBuffers.find(strName) != stringBuffers.end()) {
                        int bufferAddr = stringBuffers[strName];
                        string str = ReadStringFromMemory(bufferAddr);
                        cout << str;                                // Output string from memory
                        cout << "  -> Printed from buffer '" << strName << "': '" << str << "'" << endl;
                    }
                    else {
                        cout << "  -> ERROR: String '" << strName << "' not found!" << endl;
                    }
                }
            }
            else if (opcode == "READ_INT") {                        // Read integer input from user
                if (tokens.size() > 1 && IsRegister(tokens[1])) {
                    cout << "  Enter value for " << tokens[1] << ": ";
                    string input;
                    cin >> input;                                   // Read user input
                    try {
                        registers[tokens[1]] = stoi(input);         // Try to convert to integer
                        cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << " (numeric)" << endl;
                    } catch (...) {                                 // If conversion fails
                        if (!input.empty()) {                       // If input not empty
                            registers[tokens[1]] = (int)input[0];   // Store ASCII value of first character
                            cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << " (ASCII: '" << (char)registers[tokens[1]] << "')" << endl;
                        } else {
                            registers[tokens[1]] = 0;               // Store 0 for empty input
                            cout << "  -> " << tokens[1] << " = 0 (empty input)" << endl;
                        }
                    }
                }
            }
            else if (opcode == "READ_STRING") {                     // Read string input from user
                if (tokens.size() > 1 && IsRegister(tokens[1])) {
                    cout << "  Enter string: ";
                    string input;
                    
                    // Clear any leftover newline from previous cin operations
                    if (cin.peek() == '\n') { cin.ignore();} 

                    getline(cin, input);                            // Read entire line including spaces
                    int bufferAddress = registers["R3"];            // Get buffer address from register R3 (convention: R3 holds target buffer address)
                    WriteStringToMemory(bufferAddress, input);      // Write string to memory (byte by byte)
                    registers[tokens[1]] = input.length();          // Store length in the specified register (usually R0)
                    
                    cout << "  -> READ_STRING: stored '" << input << "' at address 0x" << hex << bufferAddress << dec << ", length = " << input.length() << endl;
                    // Debug: Verify what was written to memory
                    cout << "  -> DEBUG: Reading back from memory: '"<< ReadStringFromMemory(bufferAddress) << "'" << endl;
                    for (int i = 0; i < input.length(); i++) {
                        cout << "  -> Memory[0x" << hex << (bufferAddress + i) << dec   << "] = " << ReadVirtualMemory(bufferAddress + i)  << " ('" << (char)ReadVirtualMemory(bufferAddress + i) << "')" << endl;
                    }
                }
            }
            else if (opcode == "WRITE_INT") {                       // Output integer value
                if (tokens.size() > 1 && IsRegister(tokens[1])) {
                    cout << "  WRITE_INT " << tokens[1] << endl;
                    cout << registers[tokens[1]];                   // Print register value
                }
            }
            else if (opcode == "READ_CHAR") {                       // Read a single character from user
                char c;
                cin >> c;
            }
            else if (opcode == "Crlf") {                            // Print newline (Irvine32 equivalent)
                cout << endl;
            }

            // ========== ARITHMETIC INSTRUCTIONS ========== 
            else if (opcode == "ADD") {                             // Add two registers or a variable into register
                if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    cout << "  ADD " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];            // Store original value for overflow detection
                    int operand2;                                   // A var to store the second operand (register or var)
                    
                    // Parse second operand (register, variable, or immediate)
                    if (IsRegister(tokens[2])) { operand2 = registers[tokens[2]]; }               // operand 2 is a register
                    else if (IsVariable(tokens[2])) { operand2 = GetVariableValue(tokens[2]);}    // operand 2 is a variable
                    else { operand2 = stoi(tokens[2]); }                                          // operand 2 is a immediate value

                    registers[tokens[1]] += operand2;   // Add source to destination register
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                
                    // Set status flags for ADD operation
                    int result = registers[tokens[1]];
                    ZF = (result == 0);                                             // Zero Flag: result is zero
                    SF = (result < 0);                                              // Sign Flag: result is negative
                    OF = (oldValue > 0 && operand2 > 0 && result < 0) ||            // Positive overflow
                        (oldValue < 0 && operand2 < 0 && result > 0);               // Negative overflow
                    CF = false;                                                     // No carry flag for signed arithmetic
                    
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "SUB") {                             // Subtract two registers or a var into register
                if (tokens.size() > 2 && IsRegister(tokens[1])) {   // Overall: Ensure instruction has proper "OP REGISTER, REGISTER" format
                    cout << "  SUB " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];            // Store original value for overflow detection
                    int operand2;                                   // A var to store the second operand (register or var)
                    
                    // Parse second operand (register, variable, or immediate)
                    if (IsRegister(tokens[2])) { operand2 = registers[tokens[2]]; }             // operand 2 is a register
                    else if (IsVariable(tokens[2])) { operand2 = GetVariableValue(tokens[2]);}  // operand 2 is a variable
                    else { operand2 = stoi(tokens[2]); }                                        // operand 2 is a immediate value
                    
                    registers[tokens[1]] -= operand2;   // Subtract source from destination
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set status flags for SUB operation
                    int result = registers[tokens[1]];
                    ZF = (result == 0);                             // Zero Flag: result is zero
                    SF = (result < 0);                              // Sign Flag: result is negative
                    OF = (oldValue >= 0 && operand2 < 0 && result < 0) || (oldValue < 0 && operand2 > 0 && result > 0); // Overflow cases
                    CF = false;                                     // No carry flag for signed arithmetic
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "IDIV") {                            // Division
                // Signed division: EDX:EAX / divisor
                if (tokens.size() > 1) {
                    cout << "  IDIV " << tokens[1] << endl;
                    int divisor;
                    
                    // Parse divisor (register, variable, or immediate)
                    if (IsRegister(tokens[1])) { divisor = registers[tokens[1]]; }              // divisor is a register
                    else if (IsVariable(tokens[1])) { divisor = GetVariableValue(tokens[1]); }  // divisor is a variable
                    else {  divisor = stoi(tokens[1]); }                                        // divisor is a immediate value
                    
                    if (divisor == 0) {
                        cout << "  -> ERROR: Division by zero!" << endl;
                        ZF = false; SF = false; OF = true; CF = true;
                    } else {
                        // Dividend is in R0:R1 (64-bit), result in R0, remainder in R1
                        long long dividend = (long long)registers["R0"] | ((long long)registers["R1"] << 32);
                        registers["R0"] = (int)(dividend / divisor);  // Quotient
                        registers["R1"] = (int)(dividend % divisor);  // Remainder
                        
                        cout << "  -> R0 (quotient) = " << registers["R0"] << endl;
                        cout << "  -> R1 (remainder) = " << registers["R1"] << endl;
                        
                        // Set flags for IDIV
                        ZF = (registers["R0"] == 0);
                        SF = (registers["R0"] < 0);
                        OF = false;  // IDIV doesn't typically set overflow flag
                        CF = false;  // IDIV doesn't typically set carry flag
                        
                        cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                    }
                }
            }
            else if (opcode == "IMUL") {                            // Multiplication
                // Signed multiplication
                if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    cout << "  IMUL " << tokens[1] << ", " << tokens[2] << endl;
                    int operand2;
                    
                    // Parse second operand (register, variable, or immediate)
                    if (IsRegister(tokens[2])) { operand2 = registers[tokens[2]]; } 
                    else if (IsVariable(tokens[2])) { operand2 = GetVariableValue(tokens[2]);}
                    else { operand2 = stoi(tokens[2]); }
                    
                    long long result = (long long)registers[tokens[1]] * (long long)operand2;
                    registers[tokens[1]] = (int)result;              // Store lower 32 bits
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set flags for IMUL
                    ZF = (registers[tokens[1]] == 0);
                    SF = (registers[tokens[1]] < 0);
                    // For IMUL, OF and CF are set if the result exceeds 32-bit signed range
                    OF = CF = (result > INT_MAX || result < INT_MIN);
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "MOV") {                             // Check if instruction is MOV
                if (tokens.size() > 2) {                                                       // Verify that at least 3 tokens exist (MOV, dest, src)
                    cout << "  MOV " << tokens[1] << ", " << tokens[2] << endl;                // Print the MOV instruction being executed
                    
                    // Handle MOV to register
                    if (IsRegister(tokens[1])) {                                               // Check if destination is a register
                        
                        // Handle "OFFSET bufferName" syntax
                        if (tokens[2] == "OFFSET" && tokens.size() > 3) {                      // Check if source uses OFFSET keyword
                            string bufferName = tokens[3];                                     // Extract the buffer name
                            int address = GetStringBufferAddress(bufferName);                  // Get the address of the buffer
                            registers[tokens[1]] = address;                                    // Store address in destination register
                            cout << "  -> " << tokens[1] << " = 0x" << hex << address  << dec << " (address of " << bufferName << ")" << endl;  // Print the address stored in hex format
                        }

                        // Regular MOV operations
                        else if (IsRegister(tokens[2])) {                                      // Check if source is also a register
                            registers[tokens[1]] = registers[tokens[2]];                       // Copy source register value to destination
                        }
                        else if (IsVariable(tokens[2])) {                                      // Check if source is a variable
                            registers[tokens[1]] = GetVariableValue(tokens[2]);                // Get variable value and store in destination register
                        }
                        else {                                                                 // Source must be an immediate value
                            registers[tokens[1]] = stoi(tokens[2]);                            // Convert string to integer and store in destination
                        }
                        
                        if (tokens[2] != "OFFSET") {                                           // Only print if not an OFFSET operation (already printed above)
                            cout << "  -> " << tokens[1] << " = " << registers[tokens[1]]  << endl; // Print the final value in the destination register
                        }
                        
                        // MOV to register affects flags
                        int result = registers[tokens[1]];                                     // Get the result value from the destination register
                        ZF = (result == 0);                                                    // Set Zero Flag if result is zero
                        SF = (result < 0);                                                     // Set Sign Flag if result is negative
                        cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << endl;               // Print the updated flag values
                    }
                    
                    // Handle MOV from calculator variables to registers (source is calculator variable)
                    else if (IsVariable(tokens[1])) {                                          // Check if destination is a variable
                        int value;                                                             // Declare variable to hold the value

                        if (IsRegister(tokens[2])) {                                           // Check if source is a register
                            value = registers[tokens[2]];                                      // Get value from source register
                        }
                        else if (IsVariable(tokens[2])) {                                      // Check if source is a variable
                            value = GetVariableValue(tokens[2]);                               // Get value from source variable
                        }
                        else {                                                                 // Source must be an immediate value
                            value = stoi(tokens[2]);                                           // Convert string to integer
                        }

                        SetVariableValue(tokens[1], value);                                    // Store the value in the destination variable
                        cout << "  -> " << tokens[1] << " = " << GetVariableValue(tokens[1]) << endl;   // Print the final value stored in the variable
                    }
                    
                    // Handle "MOV BYTE PTR [reg + offset], value"
                    else if (tokens[1] == "BYTE" && tokens.size() > 4 && tokens[2] == "PTR") { // Check for BYTE PTR memory operation syntax
                        // Parse memory address: [R5 + R1] or [R5 + 0]
                        string addrExpr = tokens[3];                                           // Extract address expression (should be like "[R5")
                        string offsetExpr = tokens[4];                                         // Extract offset expression (should be "+")
                        if (addrExpr[0] == '[') {                                              // Check if opening bracket is present
                            addrExpr = addrExpr.substr(1);                                     // Remove the opening bracket
                        }
                        
                        int baseAddr = 0;                                                      // Initialize base address to zero
                        int offset = 0;                                                        // Initialize offset to zero
                        
                        if (IsRegister(addrExpr)) {                                            // Check if address expression is a register
                            baseAddr = registers[addrExpr];                                    // Get the base address from the register
                        }
                        
                        // Handle offset (could be "+", "R1]", or "0]")
                        if (tokens.size() > 5) {                                               // Check if offset tokens exist
                            string offsetToken = tokens[5];                                    // Extract the offset token
                            if (offsetToken.back() == ']') {                                   // Check if closing bracket is present
                                offsetToken = offsetToken.substr(0, offsetToken.length() - 1); // Remove the closing bracket
                            }
                            
                            if (IsRegister(offsetToken)) {                                     // Check if offset is a register
                                offset = registers[offsetToken];                               // Get offset value from register
                            }
                            else {                                                             // Offset must be an immediate value
                                offset = stoi(offsetToken);                                    // Convert string to integer
                            }
                        }
                        
                        int finalAddress = baseAddr + offset;                                  // Calculate final memory address
                        
                        // Get value to store
                        int value = 0;                                                         // Initialize value variable
                        string valueToken = tokens[tokens.size() - 1];                         // Get the last token (the value to store)
                        if (valueToken.back() == ',') {                                        // Check if comma is present at the end
                            valueToken = valueToken.substr(0, valueToken.length() - 1);       // Remove the trailing comma
                        }
                        
                        if (IsRegister(valueToken)) {                                          // Check if value token is a register
                            value = registers[valueToken];                                     // Get value from the register
                        }
                        else {                                                                 // Value must be an immediate value
                            value = stoi(valueToken);                                          // Convert string to integer
                        }
                        
                        WriteVirtualMemory(finalAddress, value);                               // Write the value to virtual memory at final address
                        cout << "  -> MOV BYTE PTR: stored value " << value << " at address 0x" // Print operation confirmation
                             << hex << finalAddress << dec << endl;
                    }
                }
            }
            else if (opcode == "MOVZX") {                           // Check if instruction is MOVZX (move with zero-extend)
                if (tokens.size() > 3) {                                                       // Verify that at least 4 tokens exist
                    string destReg = tokens[1];                                                // Extract destination register
                    if (destReg.back() == ',') {                                               // Check if comma is present at the end
                        destReg = destReg.substr(0, destReg.length() - 1);                     // Remove the trailing comma
                    }
                    
                    if (IsRegister(destReg) && tokens[2] == "BYTE" && tokens[3] == "PTR") {   // Verify MOVZX BYTE PTR syntax
                        // tokens[4] should be something like "[R4" and remaining should be "+", "R1]"
                        int baseAddr = 0;                                                      // Initialize base address to zero
                        int offset = 0;                                                        // Initialize offset to zero
                        
                        // Parse [R4 + R1] format more carefully
                        if (tokens.size() >= 5) {                                              // Check if address tokens exist
                            string addrExpr = tokens[4];                                       // Extract address expression (e.g., "[R4")
                            if (addrExpr[0] == '[') {                                          // Check if opening bracket is present
                                addrExpr = addrExpr.substr(1);                                 // Remove the opening bracket
                            }
                            if (IsRegister(addrExpr)) {                                        // Check if address expression is a register
                                baseAddr = registers[addrExpr];                                // Get the base address from the register
                            }
                            
                            // Now parse the offset if it exists
                            if (tokens.size() >= 7 && tokens[5] == "+") {                      // Check if offset operator and tokens exist
                                string offsetToken = tokens[6];                                // Extract the offset token
                                // Remove closing bracket if present
                                if (offsetToken.back() == ']') {                               // Check if closing bracket is present
                                    offsetToken = offsetToken.substr(0, offsetToken.length() - 1); // Remove the closing bracket
                                }
                                
                                if (IsRegister(offsetToken)) {                                 // Check if offset is a register
                                    offset = registers[offsetToken];                           // Get offset value from register
                                }
                                else {                                                         // Offset must be an immediate value
                                    offset = stoi(offsetToken);                                // Convert string to integer
                                }
                            }
                        }
                        
                        int finalAddress = baseAddr + offset;                                  // Calculate final memory address
                        int byteValue = ReadVirtualMemory(finalAddress) & 0xFF;               // Read byte from virtual memory and mask to 8 bits (zero-extend)
                        registers[destReg] = byteValue;                                        // Store the zero-extended byte value in destination register
                        // Print operation confirmation
                        cout << "  -> MOVZX: loaded byte " << byteValue << " from address 0x" << hex << finalAddress << dec << " into " << destReg << endl;
                    }
                }
            }
            
            // ========== COMPARISON AND BRANCHING ==========
            else if (opcode == "CMP") {                             // Compare two values
                if (tokens.size() > 2) {                     // Check 1: Verify instruction has at least 3 tokens (opcode + 2 operands)
                    // Remove colons from operands
                    string op1 = tokens[1];
                    string op2 = tokens[2];
                    if (op1.back() == ':') op1 = op1.substr(0, op1.length() - 1);
                    if (op2.back() == ':') op2 = op2.substr(0, op2.length() - 1);
                    
                    cout << "  CMP " << op1 << ", " << op2 << endl;
                    int val1, val2;                          // Values to compare
                    
                    // Parse first operand (can be register, immediate, special variable, or calculator variable)
                    if (op1 == "matrixAllocated") { 
                        val1 = matrixAllocated ? 1 : 0; 
                    } else if (IsRegister(op1)) { 
                        val1 = registers[op1]; 
                    } else if (IsVariable(op1)) {
                        val1 = GetVariableValue(op1);
                    } else { 
                        val1 = stoi(op1); 
                    }
                    
                    // Parse second operand (can be register, immediate, special variable, or calculator variable)
                    if (op2 == "matrixAllocated") { 
                        val2 = matrixAllocated ? 1 : 0; 
                    } else if (IsRegister(op2)) { 
                        val2 = registers[op2]; 
                    } else if (IsVariable(op2)) {
                        val2 = GetVariableValue(op2);
                    } else { 
                        val2 = stoi(op2); 
                    }
                    
                    int result = val1 - val2;                                               // Compute comparison result
                    // Set status flags based on comparison
                    ZF = (result == 0);                          // Zero Flag: values are equal
                    SF = (result < 0);                           // Sign Flag: first value is less
                    OF = (val1 > 0 && val2 < 0 && result < 0) || // Overflow detection
                        (val1 < 0 && val2 > 0 && result > 0);
                    CF = false;                                 // No carry flag
                    cout << "  -> Comparison result: " << result << endl;
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "JE") {                              // Jump if equal (ZF == 1)
                if (tokens.size() > 1) {
                    if (ZF) {                                // Check Zero Flag
                        string label = tokens[1];            // Target label
                        if (labels.find(label) != labels.end()) {
                            programCounter = labels[label];  // Jump to label address
                            cout << "  -> Jump equal to " << label << " at line " << programCounter << endl;
                            incrementPC = false;             // Don't increment PC after jump
                        }
                    } else {
                        cout << "  -> JE condition false (ZF=" << ZF << "), not jumping" << endl;
                    }
                }
            }
            else if (opcode == "JNE") {                             // Jump if not equal (ZF == 0)
                if (tokens.size() > 1) {
                    if (!ZF) {                               // Check Zero Flag is false
                        string label = tokens[1];            // Target label
                        if (labels.find(label) != labels.end()) {
                            programCounter = labels[label];  // Jump to label address
                            cout << "  -> Jump not equal to " << label << " at line " << programCounter << endl;
                            incrementPC = false;             // Don't increment PC after jump
                        }
                    } else {
                        cout << "  -> JNE condition false (ZF=" << ZF << "), not jumping" << endl;
                    }
                }
            }
            else if (opcode == "JL") {                              // Jump if less (SF != OF)
                if (tokens.size() > 1) {
                    if (SF != OF) {                          // JL condition: Sign Flag != Overflow Flag
                        string label = tokens[1];            // Target label
                        if (labels.find(label) != labels.end()) {
                            programCounter = labels[label];  // Jump to label address
                            cout << "  -> Jump less to " << label << " at line " << programCounter << endl;
                            incrementPC = false;             // Don't increment PC after jump
                        }
                    } else {
                        cout << "  -> JL condition false (SF=" << SF << ", OF=" << OF << "), not jumping" << endl;
                    }
                }
            }
            else if (opcode == "JLE") {                             // Jump if less or equal (ZF || (SF != OF))
                if (tokens.size() > 1) {
                    if (ZF || (SF != OF)) {                  // JLE condition: equal OR less
                        string label = tokens[1];            // Target label
                        if (labels.find(label) != labels.end()) {
                            programCounter = labels[label];  // Jump to label address
                            cout << "  -> Jump less or equal to " << label << " at line " << programCounter << endl;
                            incrementPC = false;             // Don't increment PC after jump
                        }
                    } else {
                        cout << "  -> JLE condition false (ZF=" << ZF << ", SF=" << SF << ", OF=" << OF << "), not jumping" << endl;
                    }
                }
            }
            else if (opcode == "JGE") {                             // Jump if greater or equal (SF == OF)
                if (tokens.size() > 1) {
                    if (SF == OF) {                          // JGE condition
                        string label = tokens[1];
                        if (labels.find(label) != labels.end()) {
                            programCounter = labels[label];
                            cout << "  -> Jump greater or equal to " << label << " at line " << programCounter << endl;
                            incrementPC = false;
                        }
                    } else {
                        cout << "  -> JGE condition false (SF=" << SF << ", OF=" << OF << "), not jumping" << endl;
                    }
                }
            }
            else if (opcode == "JMP") {                             // Unconditional jump
                if (tokens.size() > 1) {
                    string label = tokens[1];                // Target label
                    if (labels.find(label) != labels.end()) {
                        programCounter = labels[label];      // Jump to label address
                        cout << "  -> Jumping to " << label << " at line " << programCounter << endl;
                        incrementPC = false;                 // Don't increment PC after jump
                    }
                }
            }

            // ========== SYSTEM INSTRUCTIONS ==========
            else if (opcode == "INC") {                             // Increment register by 1
                // Remove colon if present
                string operand = tokens[1];
                if (operand.back() == ':') {
                    operand = operand.substr(0, operand.length() - 1);
                }
                
                if (tokens.size() > 1 && IsRegister(operand)) {
                    cout << "  INC " << operand << endl;
                    registers[operand]++;
                    cout << "  -> " << operand << " = " << registers[operand] << endl;
                    
                    // Set flags
                    int result = registers[operand];
                    ZF = (result == 0);
                    SF = (result < 0);
                    OF = (result == INT_MIN);  // Overflow if wrapped around
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << endl;
                }
            }
            else if (opcode == "DEC") {                             // Decrement register by 1
                if (tokens.size() > 1 && IsRegister(tokens[1])) {
                    cout << "  DEC " << tokens[1] << endl;
                    registers[tokens[1]]--;
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set flags
                    int result = registers[tokens[1]];
                    ZF = (result == 0);
                    SF = (result < 0);
                    OF = (result == INT_MAX);  // Overflow if wrapped around
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << endl;
                }
            }
            else if (opcode == "CDQ") {
                // Convert Doubleword to Quadword (sign extend EAX into EDX:EAX)
                // In our simple VM, we'll simulate this for division
                if (registers["R0"] < 0) {
                    registers["R1"] = -1; // R1 is EDX equivalent (all bits 1 for negative)
                } else {
                    registers["R1"] = 0;  // R1 is EDX equivalent (all bits 0 for positive)
                }
                cout << "  -> CDQ: (R0:R1) EDX:EAX prepared for division" << endl;
            }
            else if (opcode == "CLRSC") {                           // Clear screen instruction
                cout << "  CLRSC instruction executed" << endl;
                _getch();;                                  // Waits for user to press any key
                system("cls");                              // Clear console screen
                cout << "  -> Screen cleared" << endl;
            }
            else if (opcode == "HALT") {                            // Stop program execution
                running = false;                             // Set VM running flag to false
                cout << "  -> Program halted." << endl;      // Display halt message
            }
            
            return incrementPC;                                     // Return whether to increment program counter
        }
        
        void InputMatrixValues(int baseAddress) {                       // Read matrix values from user input
            for (int i = 0; i < matrixSize; i++) {                      // Iterate through each row of matrix
                for (int j = 0; j < matrixSize; j++) {                  // Iterate through each column of matrix
                    cout << stringMemory["matrixElemPrompt"] << i << "," << j << stringMemory["matrixElemPrompt2"]; // Display prompt for element [i][j]
                    int value;
                    cin >> value;                             // Read integer value from user
                    int elementAddress = GetMatrixElementAddress(baseAddress, i, j, matrixSize); // Calculate memory address
                    WriteVirtualMemory(elementAddress, value);// Store value in virtual memory
                }
            }
        }

        // Helper function to display all matrix
        void DisplayMatrix(int baseAddress) {                           // Print matrix contents to console
            for (int i = 0; i < matrixSize; i++) {                      // Iterate through each row
                cout << stringMemory["matrixDisplayRow"] << i << stringMemory["matrixDisplayCol"]; // Display row header
                for (int j = 0; j < matrixSize; j++) {                  // Iterate through each column
                    int elementAddress = GetMatrixElementAddress(baseAddress, i, j, matrixSize); // Get element memory address
                    int value = ReadVirtualMemory(elementAddress);      // Read value from virtual memory
                    cout << value << stringMemory["spaceChar"];         // Print value followed by space
                }
                cout << endl;                                           // New line after each row
            }
        }

        // Helper function to free all matrix
        void FreeAllMatrices() {                                        // Deallocate memory for all matrices
            if (matrixPointers["matrixA"] != 0) {                       // Check if matrix A is allocated
                FreeVirtualMemory(matrixPointers["matrixA"], matrixSize * matrixSize * 4); // Free matrix A memory
                matrixPointers["matrixA"] = 0;                          // Reset matrix A pointer to unallocated
            }
            if (matrixPointers["matrixB"] != 0) {                       // Check if matrix B is allocated
                FreeVirtualMemory(matrixPointers["matrixB"], matrixSize * matrixSize * 4); // Free matrix B memory
                matrixPointers["matrixB"] = 0;                          // Reset matrix B pointer to unallocated
            }
            if (matrixPointers["matrixC"] != 0) {                       // Check if matrix C is allocated
                FreeVirtualMemory(matrixPointers["matrixC"], matrixSize * matrixSize * 4); // Free matrix C memory
                matrixPointers["matrixC"] = 0;                          // Reset matrix C pointer to unallocated
            }
            matrixSize = 0;                                             // Reset matrix size to zero
            matrixAllocated = false;                                    // Set allocation flag to false
        }

        // Helper function to get variable value
        int GetVariableValue(const string& varName) {
            if (varName =="prevResult") return prevResult;
            if (varName == "firstNum") return firstNum;
            if (varName == "secondNum") return secondNum;
            if (varName == "remainder") return remainder;
            if (varName == "usePrev") return usePrev ? 1 : 0;
            
            // Check string variables (addresses and lengths)
            if (stringVariables.find(varName) != stringVariables.end()) {
                return stringVariables[varName];
            }
            return 0; // default
        }
        
        // Helper function to set variable value
        void SetVariableValue(const string& varName, int value) {
            if (varName == "prevResult") prevResult = value;
            else if (varName == "firstNum") firstNum = value;
            else if (varName == "secondNum") secondNum = value;
            else if (varName == "remainder") remainder = value;
            else if (varName == "usePrev") usePrev = (value != 0);
            
            // Set string variables
            else if (stringVariables.find(varName) != stringVariables.end()) {
                stringVariables[varName] = value;
            }
        }
        
        vector<string> Tokenize(const string& line) {                   // Split instruction line into individual tokens
            vector<string> tokens;                                      // Vector to store resulting tokens
            stringstream ss(line);                                      // Create string stream from input line
            string token;                                               // Temporary storage for each token
            while (ss >> token) {                                       // Extract tokens separated by whitespace
                token.erase(remove(token.begin(), token.end(), ','), token.end()); // Remove comma characters from token
                tokens.push_back(token);                                // Add cleaned token to result vector
            }
            return tokens;                                              // Return vector of tokens
        }
        
        // Helper function to check if a token is a regitser name
        bool IsRegister(const string& token) {                          // Check if token represents a valid register name
            return token[0] == 'R' && token.size() == 2 && isdigit(token[1]);   // First character must be 'R'     Token must be exactly 2 characters long      Second character must be a digit (0-5)
        }
        
        // Helper function to check if a token is a variable name
        bool IsVariable(const string& token) {
            return token == "prevResult" || token == "firstNum" || token == "secondNum" || token == "remainder" || token == "usePrev" || token == "string1Addr" || token == "string2Addr" || token == "string1Length" || token == "string2Length";
        }
};


int main(){
    VirtualMachine vm;
    
    ofstream testFile("memory_program.asm");
        // Main program structure
        testFile << "START:\n";
        testFile << "    CALL DisplayWelcome\n";
        testFile << "    JMP MenuLoop\n";
        testFile << "    HALT\n";
        testFile << "\n";
        testFile << "MenuLoop:\n";
        testFile << "    CALL DisplayMenu\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    CALL ExecuteChoice\n";
        testFile << "    CALL ContinueMessage\n";
        testFile << "    CALL ScreenClear\n";
        testFile << "    JMP MenuLoop\n";
        testFile << "\n";

        // Core UI procedures
        testFile << "DisplayWelcome:\n";
        testFile << "    PRINT_STR welcomeMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "DisplayMenu:\n";
        testFile << "    PRINT_STR menuPrompt\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "ReadUserChoice:\n";
        testFile << "    READ_INT R0\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "ReadUserString:\n";
        testFile << "    READ_STRING R0\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "ContinueMessage:\n";
        testFile << "    PRINT_STR continueMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "ScreenClear:\n";
        testFile << "    PRINT_STR continueMsg\n";
        testFile << "    CLRSC\n";
        testFile << "    RET\n";
        testFile << "\n";

        // Choice execution
        testFile << "ExecuteChoice:\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE StringSection\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE ExitProgram\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "InvalidChoiceMessage:\n";
        testFile << "    PRINT_STR invalidChoiceMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "StringSection:\n";
        testFile << "    CALL StringModule\n";
        testFile << "    RET\n";
        testFile << "\n";

        // EXIT PROGRAM
        testFile << "ExitProgram:\n";
        testFile << "    HALT\n";
        testFile << "\n";

        // String manipulation module
        testFile << "; ========== STRING MANIPULATION MODULE ==========\n";
        testFile << "StringModule:\n";
        testFile << "    PUSH R0\n";
        testFile << "    PUSH R1\n";
        testFile << "    PUSH R2\n";
        testFile << "StringMenuLoop:\n";
        testFile << "    CALL DisplayStringMenu\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE StringReverse\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE StringConcatenation\n";
        testFile << "    CMP R0, 3\n";
        testFile << "    JE StringCopy\n";
        testFile << "    CMP R0, 4\n";
        testFile << "    JE StringCompare\n";
        testFile << "    CMP R0, 5\n";
        testFile << "    JE StringEnd\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    JMP StringMenuLoop\n";
        testFile << "\n";
        testFile << "StringReverse:\n";
        testFile << "    CALL StringReverseProcedure\n";
        testFile << "    JMP StringMenuLoop\n";
        testFile << "\n";
        testFile << "StringConcatenation:\n";
        testFile << "    CALL StringConcatenationProcedure\n";
        testFile << "    JMP StringMenuLoop\n";
        testFile << "\n";
        testFile << "StringCopy:\n";
        testFile << "    CALL StringCopyProcedure\n";
        testFile << "    JMP StringMenuLoop\n";
        testFile << "\n";
        testFile << "StringCompare:\n";
        testFile << "    CALL StringCompareProcedure\n";
        testFile << "    JMP StringMenuLoop\n";
        testFile << "\n";
        testFile << "StringEnd:\n";
        testFile << "    POP R2\n";
        testFile << "    POP R1\n";
        testFile << "    POP R0\n";
        testFile << "    RET\n";
        testFile << "\n";

        // String menu display
        testFile << "DisplayStringMenu:\n";
        testFile << "    PUSH R3\n";
        testFile << "    PRINT_STR stringTitle\n";
        testFile << "    PRINT_STR stringMenu\n";
        testFile << "    POP R3\n";
        testFile << "    RET\n";
        testFile << "\n";

        // String operation procedures
        testFile << "StringReverseProcedure:\n";
        testFile << "    PUSH R0\n";
        testFile << "    PUSH R1\n";
        testFile << "    PUSH R2\n";
        testFile << "    PUSH R3\n";
        testFile << "    PUSH R4\n";
        testFile << "    PUSH R5\n";
        testFile << "    PRINT_STR stringPrompt1\n";
        testFile << "    MOV R3, OFFSET string1\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "    CMP R0, 0\n";
        testFile << "    JE ReverseEmpty\n";
        testFile << "    MOV R4, R3\n";
        testFile << "    MOV R2, R0\n";
        testFile << "    MOV R1, 0\n";
        testFile << "\n";

        // Push all characters onto stack (reverses order)
        testFile << "ReversePushLoop:\n"; 
        testFile << "    MOVZX R0, BYTE PTR [R4 + R1]\n";
        testFile << "    PUSH R0\n";
        testFile << "    INC R1\n";
        testFile << "    CMP R1, R2\n";
        testFile << "    JL ReversePushLoop\n";
        testFile << "    MOV R1, 0\n";
        testFile << "    MOV R5, OFFSET reversedString\n";
        testFile << "\n";

        // Pop characters back in reverse order (LIFO)
        testFile << "ReversePopLoop:\n";
        testFile << "    POP R0\n";
        testFile << "    MOV BYTE PTR [R5 + R1], R0\n";
        testFile << "    INC R1\n";
        testFile << "    CMP R1, R2\n";
        testFile << "    JL ReversePopLoop\n";
        testFile << "\n";

        // Null terminate the reversed string
        testFile << "    MOV BYTE PTR [R5 + R1], 0\n";
        testFile << "\n";

        // Display results
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR originalStr\n";
        testFile << "    PRINT_STR string1\n";
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR reversedStr\n";
        testFile << "    PRINT_STR reversedString\n";
        testFile << "    Crlf\n";
        testFile << "    JMP ReverseDone\n";
        testFile << "\n";

        testFile << "ReverseEmpty:\n";
        testFile << "    PRINT_STR emptyStringMsg\n";
        testFile << "\n";

        testFile << "ReverseDone:\n";
        testFile << "    POP R5\n";
        testFile << "    POP R4\n";
        testFile << "    POP R3\n";
        testFile << "    POP R2\n";
        testFile << "    POP R1\n";
        testFile << "    POP R0\n";
        testFile << "    RET\n";
        testFile << "\n";

        // ========== STRING CONCATENATION PROCEDURE ==========
        testFile << "StringConcatenationProcedure:\n";
        testFile << "    PUSH R0\n";
        testFile << "    PUSH R1\n";
        testFile << "    PUSH R2\n";
        testFile << "    PUSH R3\n";
        testFile << "    PUSH R4\n";
        testFile << "    PUSH R5\n";
        testFile << "\n";

        // Get first string from user
        testFile << "    PRINT_STR stringPrompt1\n";
        testFile << "    MOV R3, OFFSET string1\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "    MOV R1, R0\n"; // string1Length = R0
        testFile << "\n";

        // Get second string from user
        testFile << "    PRINT_STR stringPrompt2\n";
        testFile << "    MOV R3, OFFSET string2\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "    MOV R2, R0\n"; // string2Length = R0
        testFile << "\n";

        // Setup for concatenation - copy first string to result
        testFile << "    MOV R4, OFFSET string1\n";  // R4 = source 1
        testFile << "    MOV R5, OFFSET resultString\n"; // R5 = destination
        testFile << "    MOV R0, 0\n"; // R0 = index counter
        testFile << "\n";

        // Copy first string to result buffer
        testFile << "ConcatLoop1:\n";
        testFile << "    CMP R0, R1\n";
        testFile << "    JGE ConcatLoop1Done\n";
        testFile << "    MOVZX R3, BYTE PTR [R4 + R0]\n";
        testFile << "    MOV BYTE PTR [R5 + R0], R3\n";
        testFile << "    INC R0\n";
        testFile << "    JMP ConcatLoop1\n";
        testFile << "\n";

        testFile << "ConcatLoop1Done:\n";
        // Now copy second string after the first one
        testFile << "    MOV R4, OFFSET string2\n"; // R4 = source 2
        testFile << "    MOV R3, 0\n"; // R3 = index for second string
        testFile << "\n";

        testFile << "ConcatLoop2:\n";
        testFile << "    CMP R3, R2\n";
        testFile << "    JGE ConcatLoop2Done\n";
        testFile << "    MOVZX R2, BYTE PTR [R4 + R3]\n";
        testFile << "    MOV BYTE PTR [R5 + R0], R2\n";
        testFile << "    INC R0\n";
        testFile << "    INC R3\n";
        testFile << "    JMP ConcatLoop2\n";
        testFile << "\n";

        testFile << "ConcatLoop2Done:\n";
        // Null terminate the concatenated string
        testFile << "    MOV BYTE PTR [R5 + R0], 0\n";
        testFile << "\n";

        // Display result to user
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR concatResult\n";
        testFile << "    PRINT_STR resultString\n";
        testFile << "    Crlf\n";
        testFile << "\n";

        testFile << "    POP R5\n";
        testFile << "    POP R4\n";
        testFile << "    POP R3\n";
        testFile << "    POP R2\n";
        testFile << "    POP R1\n";
        testFile << "    POP R0\n";
        testFile << "    RET\n";
        testFile << "\n";

        // ========== STRING COPY PROCEDURE ==========
        testFile << "StringCopyProcedure:\n";
        testFile << "    PUSH R0\n";
        testFile << "    PUSH R1\n";
        testFile << "    PUSH R2\n";
        testFile << "    PUSH R3\n";
        testFile << "    PUSH R4\n";
        testFile << "    PUSH R5\n";
        testFile << "\n";

        // Get source string from user
        testFile << "    PRINT_STR stringPrompt1\n";
        testFile << "    MOV R3, OFFSET string1\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "\n";

        // Setup copy operation
        testFile << "    MOV R4, OFFSET string1\n"; // R4 = source
        testFile << "    MOV R5, OFFSET copiedString\n"; // R5 = destination
        testFile << "    MOV R2, R0\n"; // R2 = string length
        testFile << "    MOV R1, 0\n"; // R1 = index counter
        testFile << "\n";

        testFile << "CopyLoop:\n";
        testFile << "    CMP R1, R2\n";
        testFile << "    JGE CopyDone\n";
        testFile << "    MOVZX R0, BYTE PTR [R4 + R1]\n";
        testFile << "    MOV BYTE PTR [R5 + R1], R0\n";
        testFile << "    INC R1\n";
        testFile << "    JMP CopyLoop\n";
        testFile << "\n";

        testFile << "CopyDone:\n";
        // Null terminate the copied string
        testFile << "    MOV BYTE PTR [R5 + R1], 0\n";
        testFile << "\n";

        // Display results to user
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR originalStr\n";
        testFile << "    PRINT_STR string1\n";
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR copyResult\n";
        testFile << "    PRINT_STR copiedString\n";
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR copySuccess\n";
        testFile << "\n";

        testFile << "    POP R5\n";
        testFile << "    POP R4\n";
        testFile << "    POP R3\n";
        testFile << "    POP R2\n";
        testFile << "    POP R1\n";
        testFile << "    POP R0\n";
        testFile << "    RET\n";
        testFile << "\n";

        // ========== STRING COMPARE PROCEDURE ==========
        testFile << "StringCompareProcedure:\n";
        testFile << "    PUSH R0\n";
        testFile << "    PUSH R1\n";
        testFile << "    PUSH R2\n";
        testFile << "    PUSH R3\n";
        testFile << "    PUSH R4\n";
        testFile << "    PUSH R5\n";
        testFile << "\n";

        // Get first string from user
        testFile << "    PRINT_STR stringPrompt1\n";
        testFile << "    MOV R3, OFFSET string1\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "    MOV R1, R0\n"; // string1Length = R0
        testFile << "\n";

        // Get second string from user
        testFile << "    PRINT_STR stringPrompt2\n";
        testFile << "    MOV R3, OFFSET string2\n";
        testFile << "    CALL ReadUserString\n";
        testFile << "    MOV R2, R0\n"; // string2Length = R0
        testFile << "\n";

        // Check if lengths are different
        testFile << "    CMP R1, R2\n";
        testFile << "    JNE StringsNotEqual\n";
        testFile << "\n";

        // Setup comparison - lengths are equal, compare character by character
        testFile << "    MOV R4, OFFSET string1\n"; // R4 = string1
        testFile << "    MOV R5, OFFSET string2\n"; // R5 = string2
        testFile << "    MOV R0, 0\n"; // R0 = index counter
        testFile << "\n";

        testFile << "CompareLoop:\n";
        testFile << "    CMP R0, R1\n";
        testFile << "    JGE StringsEqual\n";
        testFile << "\n";

        testFile << "    MOVZX R2, BYTE PTR [R4 + R0]\n"; // R2 = string1[index]
        testFile << "    MOVZX R3, BYTE PTR [R5 + R0]\n"; // R3 = string2[index]
        testFile << "\n";

        testFile << "    CMP R2, R3\n";
        testFile << "    JNE StringsNotEqual\n";
        testFile << "\n";

        // Check if null terminator
        testFile << "    CMP R2, 0\n";
        testFile << "    JE StringsEqual\n";
        testFile << "\n";

        testFile << "    INC R0\n";
        testFile << "    JMP CompareLoop\n";
        testFile << "\n";

        testFile << "StringsNotEqual:\n";
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR compareNotEqual\n";
        testFile << "    JMP CompareDone\n";
        testFile << "\n";

        testFile << "StringsEqual:\n";
        testFile << "    Crlf\n";
        testFile << "    PRINT_STR compareEqual\n";
        testFile << "\n";

        testFile << "CompareDone:\n";
        testFile << "    POP R5\n";
        testFile << "    POP R4\n";
        testFile << "    POP R3\n";
        testFile << "    POP R2\n";
        testFile << "    POP R1\n";
        testFile << "    POP R0\n";
        testFile << "    RET\n";
        testFile << "\n";

    testFile.close();
    
    vm.LoadProgram("memory_program.asm");
    vm.run();
    
    return 0;
}
