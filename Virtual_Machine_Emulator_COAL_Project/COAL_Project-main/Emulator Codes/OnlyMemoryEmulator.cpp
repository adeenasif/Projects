#include <iostream>           // Standard input/output stream operations
#include <fstream>            // File stream operations for reading/writing files
#include <sstream>            // String stream operations for string parsing
#include <unordered_map>      // Hash table container for key-value pairs
#include <vector>             // Dynamic array container for sequences
#include <string>             // String class and character operations
#include <algorithm>          // Algorithms library (sort, find, transform)
#include <stack>              // Stack container (Last-In-First-Out)
#include <cstdlib>            // General utilities (memory, conversions, exit)

using namespace std;          // Use standard namespace to avoid std:: prefix

class VirtualMachine {
    private:
        unordered_map<string, int> registers;        // Storage for CPU registers (name-value pairs)
        unordered_map<string, string> stringMemory;  // Storage for named string constants
        vector<string> programMemory;                // Stores program instructions as strings
        unordered_map<string, int> labels;           // Maps label names to instruction addresses
        int programCounter;                          // Tracks current instruction position [EIP equivalent]
        bool running;                                // VM execution state (true=running, false=stopped)

        bool ZF, SF, OF, CF;                         // Status flags: Zero, Sign, Overflow, Carry
        stack<int> callStack;                        // Stores return addresses for CALL/RET instructions
        stack<int> dataStack;                        // General purpose stack for data operations
        unordered_map<int, int> virtualMemory;       // Simulates memory address space (address-value pairs)
        int nextMemoryAddress = 0x1000;              // Next available memory address (starts at 0x1000)
        unordered_map<string, int> matrixPointers;   // Stores matrix names and base memory addresses
        int matrixSize;                              // Size dimension for allocated matrices
        bool matrixAllocated;                        // Tracks if matrix memory is currently allocated

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
        }
        
        void InitializeStringMemory() {                  // Method to set up predefined string messages
            stringMemory["welcomeMsg"] = "=== Virtual Machine with Memory Management ===\n";
            stringMemory["menuPrompt"] = "Please select an option:\n1. Memory Management\n2. Exit Program\nEnter your choice (1-2): ";
            stringMemory["invalidChoice"] = "Invalid choice! Please enter 1-4.\n";
            stringMemory["continueMsg"] = "Press 'Z' key to continue...\n";

            // Memory management section
            stringMemory["memoryTitle"] = "===== Memory Management Module =====\n";
            stringMemory["memoryMenu"] = "1. Create Matrix\n2. Display Matrix\n3. Add Matrices\n4. Free Matrix Memory\n5. Return to Main Menu\nEnter your choice (1-5): ";
            stringMemory["matrixSizePrompt"] = "Enter matrix size (n for n x n matrix): ";
            stringMemory["matrixElemPrompt"] = "Enter element [";
            stringMemory["matrixElemPrompt2"] = "]: ";
            stringMemory["matrixCreatedMsg"] = "Matrix successfully created and allocated!\n";
            stringMemory["matrixFreedMsg"] = "Matrix successfully freed from memory!\n";
            stringMemory["matrixAddResult"] = "Matrix addition result:\n";
            stringMemory["matrixDisplayRow"] = "Row ";
            stringMemory["matrixDisplayCol"] = ": ";
            stringMemory["spaceChar"] = " ";
            stringMemory["matrixALabel"] = "Matrix A:\n";
            stringMemory["matrixBLabel"] = "Matrix B:\n";
            
            // Error messages
            stringMemory["noMatrixMsg"] = "Error: No matrix allocated. Please create matrix first.\n";
            stringMemory["invalidChoiceMsg"] = "Error: Invalid choice. Please try again.\n";
            stringMemory["inputBuffer"] = "";
            stringMemory["ClearCharacter"] = "Z";
        }
        
        int AllocateVirtualMemory(int size) {            // Allocates contiguous block in virtual memory
            int address = nextMemoryAddress;             // Get next available memory address
            for (int i = 0; i < size; i++) {             // Loop through each element to allocate
                virtualMemory[address + i * 4] = 0;      // Initialize memory location to zero (4-byte aligned)
            }
            nextMemoryAddress += size * 4;               // Update next available address (4 bytes per element)
            cout << "  -> Allocated " << size << " elements at address 0x" << hex << address << dec << endl;
            return address;                              // Return base address of allocated block
        }

        void FreeVirtualMemory(int address, int size) {                 // Deallocates memory block at given address
            for (int i = 0; i < size; i++) {                            // Loop through each element to free
                virtualMemory.erase(address + i * 4);                   // Remove memory entry from virtualMemory map
            }
            cout << "  -> Freed memory at address 0x" << hex << address << dec << endl;
        }

        int ReadVirtualMemory(int address) {                            // Reads value from virtual memory address
            if (virtualMemory.find(address) != virtualMemory.end()) {   // Check if address exists in memory
                return virtualMemory[address];                          // Return value stored at memory address
            }
            cout << "  -> ERROR: Memory read from invalid address 0x" << hex << address << dec << endl;
            return 0;                                                   // Return 0 for invalid memory access
        }

        void WriteVirtualMemory(int address, int value) {               // Writes value to virtual memory address
            virtualMemory[address] = value;                             // Store value in virtualMemory map at address
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
        
        bool executeInstruction(const string& instruction) { // Execute single instruction, return whether to increment PC
            vector<string> tokens = Tokenize(instruction);   // Split instruction into opcode and operands
            if (tokens.empty()) return true;                 // Return true for empty lines (increment PC)
            
            string opcode = tokens[0];                       // Extract instruction mnemonic (first token)
            bool incrementPC = true;                         // Default: move to next instruction after execution

            // ========== MEMORY MANAGEMENT INSTRUCTIONS ==========
            if (opcode == "ALLOC") {                                // Allocate memory block instruction
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
                matrixPointers["matrixA"] = AllocateVirtualMemory(totalElements); // Allocate matrix A
                matrixPointers["matrixB"] = AllocateVirtualMemory(totalElements); // Allocate matrix B
                matrixPointers["matrixC"] = AllocateVirtualMemory(totalElements); // Allocate matrix C
                
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
            else if (opcode == "PRINT_STR") {                       // Print string from string memory
                if (tokens.size() > 1 && stringMemory.find(tokens[1]) != stringMemory.end()) {
                    cout << stringMemory[tokens[1]];                // Output predefined string
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
            else if (opcode == "WRITE_INT") {                       // Output integer value
                if (tokens.size() > 1 && IsRegister(tokens[1])) {
                    cout << "  WRITE_INT " << tokens[1] << endl;
                    cout << registers[tokens[1]];                   // Print register value
                }
            }else if (opcode == "READ_CHAR") {                      // Read a single character from user
                char c;
                cin >> c;
            }

            // ========== ARITHMETIC INSTRUCTIONS ========== 
            else if (opcode == "ADD") {                             // Add two registers
                if (tokens.size() > 2 && IsRegister(tokens[1]) && IsRegister(tokens[2])) {
                    cout << "  ADD " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];            // Store original value for overflow detection
                    registers[tokens[1]] += registers[tokens[2]];   // Add source to destination register
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set status flags for ADD operation
                    int result = registers[tokens[1]];
                    ZF = (result == 0);                                             // Zero Flag: result is zero
                    SF = (result < 0);                                              // Sign Flag: result is negative
                    OF = (oldValue > 0 && registers[tokens[2]] > 0 && result < 0) ||// Positive overflow
                        (oldValue < 0 && registers[tokens[2]] < 0 && result > 0);   // Negative overflow
                    CF = false;                                                     // No carry flag for signed arithmetic ??
                    
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "SUB") {                             // Subtract two registers
                // Check 1: Verify instruction has at least 3 tokens (opcode + 2 operands)      Check 2: Verify first operand (tokens[1]) is a valid register name       Check 3: Verify second operand (tokens[2]) is a valid register name
                if (tokens.size() > 2 && IsRegister(tokens[1]) && IsRegister(tokens[2])) {      // Overall: Ensure instruction has proper "OP REGISTER, REGISTER" format
                    cout << "  SUB " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];            // Store original value for overflow detection
                    registers[tokens[1]] -= registers[tokens[2]];   // Subtract source from destination
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set status flags for SUB operation
                    int result = registers[tokens[1]];
                    ZF = (result == 0);                             // Zero Flag: result is zero
                    SF = (result < 0);                              // Sign Flag: result is negative
                    OF = (oldValue >= 0 && registers[tokens[2]] < 0 && result < 0) || (oldValue < 0 && registers[tokens[2]] > 0 && result > 0); // Overflow cases
                    CF = false;                                     // No carry flag for signed arithmetic
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "MOV") {                             // Move value to register
                if (tokens.size() > 2 && IsRegister(tokens[1])) {   // Check 1: Verify instruction has at least 3 tokens (opcode + 2 operands)      Check 2: Verify first operand (tokens[1]) is a valid register name
                    cout << "  MOV " << tokens[1] << ", " << tokens[2] << endl;
                    if (IsRegister(tokens[2])) { registers[tokens[1]] = registers[tokens[2]]; } // If the second token is also a Register than it's a Register-to-register move
                    else { registers[tokens[1]] = stoi(tokens[2]); }                            // Immediate value to register
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // MOV affects Zero and Sign flags
                    int result = registers[tokens[1]];
                    ZF = (result == 0);                      // Set Zero Flag
                    SF = (result < 0);                       // Set Sign Flag
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << endl;
                }
            }

            // ========== COMPARISON AND BRANCHING ==========
            else if (opcode == "CMP") {                      // Compare two values
                if (tokens.size() > 2) {                     // Check 1: Verify instruction has at least 3 tokens (opcode + 2 operands)
                    cout << "  CMP " << tokens[1] << ", " << tokens[2] << endl;
                    int val1, val2;                          // Values to compare
                    
                    // Parse first operand (can be register, immediate, or special variable)
                    if (tokens[1] == "matrixAllocated") { val1 = matrixAllocated ? 1 : 0; } // Special variable: matrix allocation flag
                    else if (IsRegister(tokens[1])) { val1 = registers[tokens[1]]; }        // Register operand
                    else { val1 = stoi(tokens[1]); }                                        // Immediate value
                    
                    // Parse second operand (can be register, immediate, or special variable)
                    if (tokens[2] == "matrixAllocated") { val2 = matrixAllocated ? 1 : 0; } // Special variable: matrix allocation flag
                    else if (IsRegister(tokens[2])) { val2 = registers[tokens[2]]; }        // Register operand
                    else { val2 = stoi(tokens[2]); }                                        // Immediate value
                    
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
            else if (opcode == "JE") {                       // Jump if equal (ZF == 1)
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
            else if (opcode == "JNE") {                      // Jump if not equal (ZF == 0)
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
            else if (opcode == "JL") {                       // Jump if less (SF != OF)
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
            else if (opcode == "JLE") {                      // Jump if less or equal (ZF || (SF != OF))
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
            else if (opcode == "JMP") {                      // Unconditional jump
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
            else if (opcode == "CLRSC") {                    // Clear screen instruction
                cout << "  CLRSC instruction executed" << endl;
                if (registers["R3"] == 90) {                 // Check if R3 contains ASCII 'Z' (90)
                    system("cls");                           // Clear console screen
                    cout << "  -> Screen cleared (Z character detected in R3)" << endl;
                } else {
                    cout << "  -> Screen NOT cleared - R3 does not contain 'Z' character" << endl;
                }
            }
            else if (opcode == "HALT") {                     // Stop program execution
                running = false;                             // Set VM running flag to false
                cout << "  -> Program halted." << endl;      // Display halt message
            }
            return incrementPC;                              // Return whether to increment program counter
        }
        
        void InputMatrixValues(int baseAddress) {             // Read matrix values from user input
            for (int i = 0; i < matrixSize; i++) {            // Iterate through each row of matrix
                for (int j = 0; j < matrixSize; j++) {        // Iterate through each column of matrix
                    cout << stringMemory["matrixElemPrompt"] << i << "," << j << stringMemory["matrixElemPrompt2"]; // Display prompt for element [i][j]
                    int value;
                    cin >> value;                             // Read integer value from user
                    int elementAddress = GetMatrixElementAddress(baseAddress, i, j, matrixSize); // Calculate memory address
                    WriteVirtualMemory(elementAddress, value);// Store value in virtual memory
                }
            }
        }

        void DisplayMatrix(int baseAddress) {                 // Print matrix contents to console
            for (int i = 0; i < matrixSize; i++) {            // Iterate through each row
                cout << stringMemory["matrixDisplayRow"] << i << stringMemory["matrixDisplayCol"]; // Display row header
                for (int j = 0; j < matrixSize; j++) {        // Iterate through each column
                    int elementAddress = GetMatrixElementAddress(baseAddress, i, j, matrixSize); // Get element memory address
                    int value = ReadVirtualMemory(elementAddress); // Read value from virtual memory
                    cout << value << stringMemory["spaceChar"];    // Print value followed by space
                }
                cout << endl;                                      // New line after each row
            }
        }

        void FreeAllMatrices() {                             // Deallocate memory for all matrices
            if (matrixPointers["matrixA"] != 0) {            // Check if matrix A is allocated
                FreeVirtualMemory(matrixPointers["matrixA"], matrixSize * matrixSize); // Free matrix A memory
                matrixPointers["matrixA"] = 0;               // Reset matrix A pointer to unallocated
            }
            if (matrixPointers["matrixB"] != 0) {            // Check if matrix B is allocated
                FreeVirtualMemory(matrixPointers["matrixB"], matrixSize * matrixSize); // Free matrix B memory
                matrixPointers["matrixB"] = 0;               // Reset matrix B pointer to unallocated
            }
            if (matrixPointers["matrixC"] != 0) {            // Check if matrix C is allocated
                FreeVirtualMemory(matrixPointers["matrixC"], matrixSize * matrixSize); // Free matrix C memory
                matrixPointers["matrixC"] = 0;               // Reset matrix C pointer to unallocated
            }
            
            matrixSize = 0;                                  // Reset matrix size to zero
            matrixAllocated = false;                         // Set allocation flag to false
        }

        vector<string> Tokenize(const string& line) {        // Split instruction line into individual tokens
            vector<string> tokens;                           // Vector to store resulting tokens
            stringstream ss(line);                           // Create string stream from input line
            string token;                                    // Temporary storage for each token
            while (ss >> token) {                            // Extract tokens separated by whitespace
                token.erase(remove(token.begin(), token.end(), ','), token.end()); // Remove comma characters from token
                tokens.push_back(token);                     // Add cleaned token to result vector
            }
            return tokens;                                   // Return vector of tokens
        }

        bool IsRegister(const string& token) {               // Check if token represents a valid register name
            // First character must be 'R'     Token must be exactly 2 characters long      Second character must be a digit (0-5)
            return token[0] == 'R' && token.size() == 2 && isdigit(token[1]);                           
        }
};


int main() {
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
        testFile << "ContinueMessage:\n";
        testFile << "    PRINT_STR continueMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "ScreenClear:\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    MOV R3, R0\n";
        testFile << "    CLRSC\n";
        testFile << "    RET\n";
        testFile << "\n";

        // Choice execution
        testFile << "ExecuteChoice:\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE MemorySection\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE ExitProgram\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "InvalidChoiceMessage:\n";
        testFile << "    PRINT_STR invalidChoiceMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "MemorySection:\n";
        testFile << "    CALL MemoryModule\n";
        testFile << "    RET\n";

        // EXIT PROGRAM
        testFile << "ExitProgram:\n";
        testFile << "    HALT\n";
        testFile << "\n";
        // memory management module
        testFile << "; ========== MEMORY MANAGEMENT MODULE ==========\n";
        testFile << "MemoryModule:\n";
        testFile << "MemoryMenuLoop:\n";
        testFile << "    PRINT_STR memoryTitle\n";
        testFile << "    PRINT_STR memoryMenu\n";
        testFile << "    READ_INT R0\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE CreateMatrix\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE DisplayMatrix\n";
        testFile << "    CMP R0, 3\n";
        testFile << "    JE AddMatrices\n";
        testFile << "    CMP R0, 4\n";
        testFile << "    JE FreeMemory\n";
        testFile << "    CMP R0, 5\n";
        testFile << "    JE MemoryEnd\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "\n";
        testFile << "; ========== CREATE MATRIX PROCEDURE ==========\n";
        testFile << "CreateMatrix:\n";
        testFile << "    CMP matrixAllocated, 0\n";
        testFile << "    JE NoFreeNeeded\n";
        testFile << "    FREE_ALL_MATRICES\n";
        testFile << "NoFreeNeeded:\n";
        testFile << "    PRINT_STR matrixSizePrompt\n";
        testFile << "    READ_INT R0\n";
        testFile << "    STORE_MATRIX_SIZE\n";
        testFile << "    MATRIX_ALLOC_MEM\n";
        testFile << "    INPUT_MATRIX_A\n";
        testFile << "    INPUT_MATRIX_B\n";
        testFile << "    PRINT_STR matrixCreatedMsg\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "\n";
        testFile << "; ========== DISPLAY MATRIX PROCEDURE ==========\n";
        testFile << "DisplayMatrix:\n";
        testFile << "    CMP matrixAllocated, 0\n";
        testFile << "    JNE MatricesExist\n";
        testFile << "    PRINT_STR noMatrixMsg\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "MatricesExist:\n";
        testFile << "    DISPLAY_MATRIX_A\n";
        testFile << "    DISPLAY_MATRIX_B\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "\n";
        testFile << "; ========== ADD MATRICES PROCEDURE ==========\n";
        testFile << "AddMatrices:\n";
        testFile << "    CMP matrixAllocated, 0\n";
        testFile << "    JNE CanAddMatrices\n";
        testFile << "    PRINT_STR noMatrixMsg\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "CanAddMatrices:\n";
        testFile << "    MATRIX_ADD_OPERATION\n";
        testFile << "    PRINT_STR matrixAddResult\n";
        testFile << "    DISPLAY_MATRIX_C\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "\n";
        testFile << "; ========== FREE MEMORY PROCEDURE ==========\n";
        testFile << "FreeMemory:\n";
        testFile << "    CMP matrixAllocated, 0\n";
        testFile << "    JNE CanFreeMemory\n";
        testFile << "    PRINT_STR noMatrixMsg\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "CanFreeMemory:\n";
        testFile << "    FREE_ALL_MATRICES\n";
        testFile << "    PRINT_STR matrixFreedMsg\n";
        testFile << "    JMP MemoryMenuLoop\n";
        testFile << "\n";
        testFile << "; ========== MEMORY MODULE END ==========\n";
        testFile << "MemoryEnd:\n";
        testFile << "    CMP matrixAllocated, 0\n";
        testFile << "    JE NoCleanupNeeded\n";
        testFile << "    FREE_ALL_MATRICES\n";
        testFile << "NoCleanupNeeded:\n";
        testFile << "    RET\n";
    testFile.close();
    
    vm.LoadProgram("memory_program.asm");
    vm.run();
    
    return 0;
}
