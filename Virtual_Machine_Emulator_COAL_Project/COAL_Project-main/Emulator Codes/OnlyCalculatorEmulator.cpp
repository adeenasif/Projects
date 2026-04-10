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
        
        // Calculator variables
        int prevResult;                              // Store previous result
        bool usePrev;                                 // Flag to use previous result
        int firstNum;                                // First operand
        int secondNum;                               // Second operand
        int remainder;                               // Remainder

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
            
            InitializeStringMemory();                    // Call method to set up predefined string constants
        
            // Initialize calculator variables
            prevResult = 0;
            usePrev = false;
            firstNum = 0;
            secondNum = 0;
            remainder = 0;
        }
        
        void InitializeStringMemory() {                  // Method to set up predefined string messages
            stringMemory["welcomeMsg"] = "=== Virtual Machine with Memory Management ===\n";
            stringMemory["menuPrompt"] = "Please select an option:\n1. Calculator\n2. String Operations\n3. Memory Management\n4. Exit Program\nEnter your choice (1-4): ";
            stringMemory["invalidChoice"] = "Invalid choice! Please enter 1-4.\n";
            stringMemory["continueMsg"] = "Press 'Z' key to continue...\n";

            // Calculator section
            stringMemory["calcTitle"] = "===== Calculator Module =====\n";
            stringMemory["calcMenu"] = "Select operation:\n1. Addition\n2. Subtraction\n3. Multiplication\n4. Division\n5. Return to Main Menu\nEnter your choice (1-5): ";
            stringMemory["enterFirst"] = "Enter first number: ";
            stringMemory["enterSecond"] = "Enter second number: ";
            stringMemory["calcResult"] = "Result: ";
            stringMemory["divByZeroMsg"] = "Error: Division by zero!\n";
            stringMemory["remainderMsg"] = " Remainder: ";
            stringMemory["usePrevResult"] = "Use previous result as first number? (1=Yes, 0=No): ";
            stringMemory["newCalcPrompt"] = "Perform new calculation? (1=Yes, 0=No/Exit): ";
            
            // Error messages
            stringMemory["noMatrixMsg"] = "Error: No matrix allocated. Please create matrix first.\n";
            stringMemory["invalidChoiceMsg"] = "Error: Invalid choice. Please try again.\n";
            stringMemory["inputBuffer"] = "";
            stringMemory["ClearCharacter"] = "Z";
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

            // ========== I/O OPERATIONS ==========
             if (opcode == "PRINT_STR") {                       // Print string from string memory
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
                    cout << registers[tokens[1]] << endl;                  // Print register value
                }
            }else if (opcode == "READ_CHAR") {                      // Read a single character from user
                char c;
                cin >> c;
            }

            // ========== ARITHMETIC INSTRUCTIONS ========== 
            else if (opcode == "ADD") {
                if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    cout << "  ADD " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];
                    int op2;
                    
                    // Parse second operand
                    if (IsRegister(tokens[2])) { 
                        op2 = registers[tokens[2]]; 
                    }
                    else if (tokens[2] == "prevResult") { 
                        op2 = prevResult;
                    }
                    else if (tokens[2] == "usePrev") { 
                        op2 = usePrev ? 1 : 0;  // Convert bool to int
                    }
                    else if (tokens[2] == "firstNum") { 
                        op2 = firstNum; 
                    }
                    else if (tokens[2] == "secondNum") { 
                        op2 = secondNum; 
                    }
                    else if (tokens[2] == "remainder") { 
                        op2 = remainder; 
                    }
                    else { 
                        op2 = stoi(tokens[2]); 
                    }

                    registers[tokens[1]] += op2;
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set status flags
                    int result = registers[tokens[1]];
                    ZF = (result == 0);
                    SF = (result < 0);
                    OF = (oldValue > 0 && op2 > 0 && result < 0) || (oldValue < 0 && op2 < 0 && result > 0);
                    CF = false;
                    
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "SUB") {
                if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    cout << "  SUB " << tokens[1] << ", " << tokens[2] << endl;
                    int oldValue = registers[tokens[1]];
                    int op2;

                    // Parse second operand  
                    if (IsRegister(tokens[2])) { 
                        op2 = registers[tokens[2]]; 
                    }
                    else if (tokens[2] == "prevResult") { 
                        op2 = prevResult;
                    }
                    else if (tokens[2] == "usePrev") { 
                        op2 = usePrev ? 1 : 0;  // Convert bool to int
                    }
                    else if (tokens[2] == "firstNum") { 
                        op2 = firstNum; 
                    }
                    else if (tokens[2] == "secondNum") { 
                        op2 = secondNum; 
                    }
                    else if (tokens[2] == "remainder") { 
                        op2 = remainder; 
                    }
                    else { 
                        op2 = stoi(tokens[2]); 
                    }

                    registers[tokens[1]] -= op2;  // FIXED: use op2 instead of registers[tokens[2]]
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set status flags
                    int result = registers[tokens[1]];
                    ZF = (result == 0);
                    SF = (result < 0);
                    OF = (oldValue >= 0 && op2 < 0 && result < 0) || (oldValue < 0 && op2 > 0 && result > 0);  // FIXED: use op2 consistently
                    CF = false;
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }           else if (opcode == "MOV") {
                if (tokens.size() > 2) {
                    cout << "  MOV " << tokens[1] << ", " << tokens[2] << endl;
                    
                    // Helper function to get value from source
                    auto getSourceValue = [&](const string& source) -> int {
                        if (IsRegister(source)) { 
                            return registers[source]; 
                        }
                        else if (source == "prevResult") { return prevResult; }
                        else if (source == "usePrev") { return usePrev ? 1 : 0; } // Convert bool to int
                        else if (source == "firstNum") { return firstNum; }
                        else if (source == "secondNum") { return secondNum; }
                        else if (source == "remainder") { return remainder; }
                        else { return stoi(source); }
                    };
        
                    // Helper function to set value to destination
                    auto setDestValue = [&](const string& dest, int value) {
                        if (IsRegister(dest)) { 
                            registers[dest] = value; 
                            // Set flags only for register destinations
                            ZF = (value == 0);
                            SF = (value < 0);
                            cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << endl;
                        }
                        else if (dest == "prevResult") { prevResult = value; }
                        else if (dest == "usePrev") { usePrev = (value != 0); } // Convert int to bool
                        else if (dest == "firstNum") { firstNum = value; }
                        else if (dest == "secondNum") { secondNum = value; }
                        else if (dest == "remainder") { remainder = value; }
                    };
                    int sourceValue = getSourceValue(tokens[2]);
                    setDestValue(tokens[1], sourceValue);
                    cout << "  -> " << tokens[1] << " = " << sourceValue << endl;}
            }
                
            // ========== COMPARISON AND BRANCHING ==========
            else if (opcode == "CMP") {
                if (tokens.size() > 2) {
                    cout << "  CMP " << tokens[1] << ", " << tokens[2] << endl;
            
                    // Helper function to get value
                    auto getValue = [&](const string& token) -> int {
                        if (token == "prevResult") { return prevResult; }
                        else if (token == "usePrev") { return usePrev ? 1 : 0; }
                        else if (token == "firstNum") { return firstNum; }
                        else if (token == "secondNum") { return secondNum; }
                        else if (token == "remainder") { return remainder; }
                        else if (IsRegister(token)) { return registers[token]; }
                        else { return stoi(token); }
                    };
        
                int val1 = getValue(tokens[1]);
                int val2 = getValue(tokens[2]);       
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
            else if (opcode == "CDQ") {
                // Convert Doubleword to Quadword (sign extend EAX into EDX:EAX)
                    int eax = registers["R0"]; // R0 is EAX equivalent
                if (eax < 0) {
                    registers["R1"] = -1; // R3 is EDX equivalent (all bits 1 for negative)
                } else {
                    registers["R1"] = 0;  // R3 is EDX equivalent (all bits 0 for positive)
                }
                cout << "  -> CDQ: EAX=" << eax << " -> EDX:EAX prepared for division" << endl;
            }
            else if (opcode == "IDIV") {
                // Signed division: EDX:EAX / divisor
                if (tokens.size() > 1) {
                    int divisor = 0;
                    
                    if (IsRegister(tokens[1])) { divisor = registers[tokens[1]]; }
                    else if (tokens[1] == "prevResult") { divisor = prevResult; }
                    else if (tokens[1] == "firstNum") { divisor = firstNum; }
                    else if (tokens[1] == "secondNum") { divisor = secondNum; }
                    else if (tokens[1] == "remainder") { divisor = remainder; }
                    else { divisor = stoi(tokens[1]); }

                    if (divisor == 0) {
                        cout << "  -> ERROR: Division by zero!" << endl;
                        ZF = false; SF = false; OF = true; CF = true;
                    } else {
                        long long dividend = (long long)registers["R0"] | ((long long)registers["R1"] << 32);
                        registers["R0"] = (int)(dividend / divisor);  // Quotient in R0
                        registers["R1"] = (int)(dividend % divisor);  // Remainder in R1 (to match ASM code)
                        
                        cout << "  -> R0 (quotient) = " << registers["R0"] << endl;
                        cout << "  -> R1 (remainder) = " << registers["R1"] << endl;
                        
                        // Set flags for IDIV
                        ZF = (registers["R0"] == 0);
                        SF = (registers["R0"] < 0);
                        OF = false;
                        CF = false;
                        
                        cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                    }
                }
            }
            else if (opcode == "IMUL") {
                if (tokens.size() == 2) {
                    // Single operand form: IMUL src (R0 = R0 * src)
                    cout << "  IMUL " << tokens[1] << endl;
                    int multiplier = 0;
                    
                    // Parse multiplier
                    if (IsRegister(tokens[1])) { 
                        multiplier = registers[tokens[1]]; 
                    }
                    else if (tokens[1] == "prevResult") { 
                        multiplier = prevResult;
                    }
                    else if (tokens[1] == "usePrev") { 
                        multiplier = usePrev ? 1 : 0;
                    }
                    else if (tokens[1] == "firstNum") { 
                        multiplier = firstNum; 
                    }
                    else if (tokens[1] == "secondNum") { 
                        multiplier = secondNum; 
                    }
                    else if (tokens[1] == "remainder") { 
                        multiplier = remainder; 
                    }
                    else { 
                        multiplier = stoi(tokens[1]); 
                    }

                    long long result = (long long)registers["R0"] * (long long)multiplier;
                    registers["R0"] = (int)result;
                    cout << "  -> R0 = " << registers["R0"] << endl;
                    
                    // Set flags
                    ZF = (registers["R0"] == 0);
                    SF = (registers["R0"] < 0);
                    OF = CF = (result > INT_MAX || result < INT_MIN);
                    
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
                else if (tokens.size() > 2 && IsRegister(tokens[1])) {
                    // Two operand form: IMUL dest, src
                    cout << "  IMUL " << tokens[1] << ", " << tokens[2] << endl;
                    int multiplier = 0;
                    
                    // Parse multiplier
                    if (IsRegister(tokens[2])) { 
                        multiplier = registers[tokens[2]]; 
                    }
                    else if (tokens[2] == "prevResult") { 
                        multiplier = prevResult;
                    }
                    else if (tokens[2] == "usePrev") { 
                        multiplier = usePrev ? 1 : 0;
                    }
                    else if (tokens[2] == "firstNum") { 
                        multiplier = firstNum; 
                    }
                    else if (tokens[2] == "secondNum") { 
                        multiplier = secondNum; 
                    }
                    else if (tokens[2] == "remainder") { 
                        multiplier = remainder; 
                    }
                    else { 
                        multiplier = stoi(tokens[2]); 
                    }

                    long long result = (long long)registers[tokens[1]] * (long long)multiplier;
                    registers[tokens[1]] = (int)result;
                    cout << "  -> " << tokens[1] << " = " << registers[tokens[1]] << endl;
                    
                    // Set flags
                    ZF = (registers[tokens[1]] == 0);
                    SF = (registers[tokens[1]] < 0);
                    OF = CF = (result > INT_MAX || result < INT_MIN);
                    
                    cout << "  -> Flags: ZF=" << ZF << " SF=" << SF << " OF=" << OF << " CF=" << CF << endl;
                }
            }
            else if (opcode == "HALT") {
                running = false;
                cout << "  -> HALT: Virtual machine stopping" << endl;
                incrementPC = false;
            }
            return incrementPC;                              // Return whether to increment program counter
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
        testFile << "    PRINT_STR continueMsg\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    MOV R3, R0\n";
        testFile << "    CLRSC\n";
        testFile << "    RET\n";
        testFile << "\n";

        // Choice execution
        testFile << "ExecuteChoice:\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE CalculatorSection\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE StringSection\n";
        testFile << "    CMP R0, 3\n";
        testFile << "    JE MemorySection\n";
        testFile << "    CMP R0, 4\n";
        testFile << "    JE ExitProgram\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "InvalidChoiceMessage:\n";
        testFile << "    PRINT_STR invalidChoiceMsg\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "CalculatorSection:\n";
        testFile << "    CALL CalculatorModule\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "StringSection:\n";
        testFile << "    CALL StringModule\n";
        testFile << "    RET\n";
        testFile << "\n";
        testFile << "MemorySection:\n";
        testFile << "    CALL MemoryModule\n";
        testFile << "    RET\n";

        // EXIT PROGRAM
        testFile << "ExitProgram:\n";
        testFile << "    HALT\n";
        testFile << "\n";

        // Calculator module
        testFile << "; ========== CALCULATOR MODULE ==========\n";
        testFile << "CalculatorModule:\n";
        testFile << "CalcMenuLoop:\n";
        testFile << "    CALL DisplayCalcMenu\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE Addition\n";
        testFile << "    CMP R0, 2\n";
        testFile << "    JE Subtraction\n";
        testFile << "    CMP R0, 3\n";
        testFile << "    JE Multiplication\n";
        testFile << "    CMP R0, 4\n";
        testFile << "    JE Division\n";
        testFile << "    CMP R0, 5\n";
        testFile << "    JE CalcEnd\n";
        testFile << "    CALL InvalidChoiceMessage\n";
        testFile << "    JMP CalcMenuLoop\n";
        testFile << "\n";
        testFile << "Addition:\n";
        testFile << "    CALL AdditionProcedure\n";
        testFile << "    JMP AskForNewCalculation\n";
        testFile << "\n";
        testFile << "Subtraction:\n";
        testFile << "    CALL SubtractionProcedure\n";
        testFile << "    JMP AskForNewCalculation\n";
        testFile << "\n";
        testFile << "Multiplication:\n";
        testFile << "    CALL MultiplicationProcedure\n";
        testFile << "    JMP AskForNewCalculation\n";
        testFile << "\n";
        testFile << "Division:\n";
        testFile << "    CALL DivisionProcedure\n";
        testFile << "    JMP AskForNewCalculation\n";
        testFile << "\n";
        testFile << "AskForNewCalculation:\n";
        testFile << "    PRINT_STR newCalcPrompt\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    CMP R0, 1\n";
        testFile << "    JE CalcMenuLoop\n";
        testFile << "\n";
        testFile << "CalcEnd:\n";
        testFile << "    RET\n";
        testFile << "\n";

        // Calculator sub-procedures
        testFile << "DisplayCalcMenu:\n";
        testFile << "    PRINT_STR calcTitle\n";
        testFile << "    CMP prevResult, 0\n";
        testFile << "    JE NoPrevResult\n";
        testFile << "    PRINT_STR calcResult\n";
        testFile << "    MOV R0, prevResult\n";
        testFile << "    WRITE_INT R0\n";
        testFile << "NoPrevResult:\n";
        testFile << "    PRINT_STR calcMenu\n";
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "GetInputNumbers:\n";
        testFile << "    CMP prevResult, 0\n";
        testFile << "    JE getFirstNumber\n";
        testFile << "    PRINT_STR usePrevResult\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    MOV usePrev, R0\n";
        testFile << "    CMP usePrev, 1\n";
        testFile << "    JNE getFirstNumber\n";
        testFile << "    MOV R0, prevResult\n";
        testFile << "    MOV firstNum, R0\n";
        testFile << "    JMP getsecondNumber\n";
        testFile << "\n";
        testFile << "getFirstNumber:\n";
        testFile << "    PRINT_STR enterFirst\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    MOV firstNum, R0\n";
        testFile << "\n";
        testFile << "getsecondNumber:\n";
        testFile << "    PRINT_STR enterSecond\n";
        testFile << "    CALL ReadUserChoice\n";
        testFile << "    MOV secondNum, R0\n";
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "AdditionProcedure:\n";
        testFile << "    CALL GetInputNumbers\n";
        testFile << "    MOV R0, firstNum\n";
        testFile << "    ADD R0, secondNum\n";
        testFile << "    MOV prevResult, R0\n";
        testFile << "    PRINT_STR calcResult\n";
        testFile << "    WRITE_INT R0\n";   // Display Result
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "SubtractionProcedure:\n";
        testFile << "    CALL GetInputNumbers\n";
        testFile << "    MOV R0, firstNum\n";
        testFile << "    SUB R0, secondNum\n";
        testFile << "    MOV prevResult, R0\n";
        testFile << "    PRINT_STR calcResult\n";
        testFile << "    WRITE_INT R0\n";   // Display Result
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "MultiplicationProcedure:\n";
        testFile << "    CALL GetInputNumbers\n";
        testFile << "    MOV R0, firstNum\n";
        testFile << "    IMUL R0, secondNum\n";
        testFile << "    MOV prevResult, R0\n";
        testFile << "    PRINT_STR calcResult\n";
        testFile << "    WRITE_INT R0\n";   // Display Result
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "DivisionProcedure:\n";
        testFile << "    CALL GetInputNumbers\n";
        testFile << "    CMP secondNum, 0\n";
        testFile << "    JNE PerfromDivision\n";
        testFile << "    PRINT_STR divByZeroMsg\n";
        testFile << "    RET\n";
        testFile << "\n";

        testFile << "PerfromDivision:\n";
        testFile << "    CMP secondNum, 0\n";
        testFile << "    MOV R0, firstNum\n";
        testFile << "    CDQ\n";
        testFile << "    MOV R1, secondNum\n";
        testFile << "    IDIV R1\n";
        testFile << "    MOV prevResult, R0\n";
        testFile << "    MOV remainder, R1\n";
        testFile << "    PRINT_STR calcResult\n";
        testFile << "    WRITE_INT R0\n";   // Display Result
        testFile << "    MOV R0, remainder\n";
        testFile << "    CMP R0, 0\n";
        testFile << "    JE DivisionComplete\n";
        testFile << "\n";
        testFile << "DivisionComplete:\n";
        testFile << "    PRINT_STR remainderMsg\n";
         testFile << "   WRITE_INT R0\n";   // Display Remainder Result
        testFile << "    RET\n";
        testFile << "\n";
    testFile.close();
        
    vm.LoadProgram("memory_program.asm");
    vm.run();
    
    return 0;
}
