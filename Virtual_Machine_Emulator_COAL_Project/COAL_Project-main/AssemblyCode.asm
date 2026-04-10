.386
.model flat, stdcall
.stack 4096
ExitProcess PROTO, dwExitCode:DWORD


INCLUDE Irvine32.inc

.data
    ; Menu and UI strings
    welcomeMsg    BYTE "=== Virtual Machine with Memory Management ===", 0Dh, 0Ah, 0
    menuPrompt    BYTE "Please select an option:", 0Dh, 0Ah
                  BYTE "1. Calculator", 0Dh, 0Ah
                  BYTE "2. String Operations", 0Dh, 0Ah
                  BYTE "3. Memory Management", 0Dh, 0Ah
                  BYTE "4. Exit Program", 0Dh, 0Ah
                  BYTE "Enter your choice (1-4): ", 0
    invalidChoice BYTE "Invalid choice! Please enter 1-4.", 0Dh, 0Ah, 0
    continueMsg   BYTE "Press any key to continue...", 0Dh, 0Ah, 0
    
    ; Calculator section
    calcTitle     BYTE "===== Calculator Module =====", 0Dh, 0Ah, 0
    calcMenu      BYTE "Select operation:", 0Dh, 0Ah
                  BYTE "1. Addition", 0Dh, 0Ah
                  BYTE "2. Subtraction", 0Dh, 0Ah
                  BYTE "3. Multiplication", 0Dh, 0Ah
                  BYTE "4. Division", 0Dh, 0Ah
                  BYTE "5. Return to Main Menu", 0Dh, 0Ah
                  BYTE "Enter your choice (1-5): ", 0
    enterFirst    BYTE "Enter first number: ", 0
    enterSecond   BYTE "Enter second number: ", 0
    calcResult    BYTE "Result: ", 0
    divByZeroMsg  BYTE "Error: Division by zero!", 0Dh, 0Ah, 0
    remainderMsg  BYTE " Remainder: ", 0
    usePrevResult BYTE "Use previous result as first number? (1=Yes, 0=No): ", 0
    newCalcPrompt BYTE "Perform new calculation? (1=Yes, 0=No/Exit): ", 0

    
    ; String section  
    stringTitle   BYTE "===== String Operations Module =====", 0Dh, 0Ah, 0
    stringMenu    BYTE "Select string operation:", 0Dh, 0Ah
                  BYTE "1. String Reverse", 0Dh, 0Ah
                  BYTE "2. String Concatenation", 0Dh, 0Ah
                  BYTE "3. Copy String", 0Dh, 0Ah
                  BYTE "4. Compare Strings", 0Dh, 0Ah
                  BYTE "5. Return to Main Menu", 0Dh, 0Ah
                  BYTE "Enter your choice (1-5): ", 0
                  
    msg1 BYTE "Enter first string: ", 0
    msg2 BYTE "Enter second string: ", 0
    reversedStr   BYTE "Reversed string: ", 0
    concatResult  BYTE "Concatenated string: ", 0
    copyResult    BYTE "Copied string: ", 0
    compareEqual  BYTE "Strings are EQUAL!", 0Dh, 0Ah, 0
    compareNotEqual BYTE "Strings are NOT equal!", 0Dh, 0Ah, 0

    str1     BYTE 100 DUP(?)        ; buffer for first string
    str2     BYTE 100 DUP(?)        ; buffer for second string
    result   BYTE 200 DUP(?)        ; large buffer for concatenation result
    
    ; Memory management section
    menuTitle        BYTE "===== Memory Management Module =====", 0Dh, 0Ah, 0
    menuOption1      BYTE "1. Create Matrix", 0Dh, 0Ah
                     BYTE "2. Display Matrix", 0Dh, 0Ah
                     BYTE "3. Add Matrices", 0Dh, 0Ah
                     BYTE "4. Free Matrix Memory", 0Dh, 0Ah
                     BYTE "5. Return to Main Menu", 0Dh, 0Ah
                     BYTE "Enter your choice (1-5): ", 0
    
    ; Matrix operation strings
    matrixSizePrompt BYTE "Enter matrix size (n for n x n matrix): ", 0
    matrixElemPrompt BYTE "Enter element [", 0
    matrixElemPrompt2 BYTE "]: ", 0
    matrixCreatedMsg BYTE "Matrix successfully created and allocated!", 0Dh, 0Ah, 0
    matrixFreedMsg   BYTE "Matrix successfully freed from memory!", 0Dh, 0Ah, 0
    matrixAddResult  BYTE "Matrix addition result:", 0Dh, 0Ah, 0
    matrixDisplayRow BYTE "Row ", 0
    matrixDisplayCol BYTE ": ", 0
    spaceChar        BYTE " ", 0
    
    ; Error messages
    noMatrixMsg      BYTE "Error: No matrix allocated. Please create matrix first.", 0Dh, 0Ah, 0
    invalidChoiceMsg BYTE "Error: Invalid choice. Please try again.", 0Dh, 0Ah, 0
    
    ; Matrix pointers and size
    matrixA          DWORD 0    ; Pointer to first matrix
    matrixB          DWORD 0    ; Pointer to second matrix  
    matrixC          DWORD 0    ; Pointer to result matrix
    matrixSize       DWORD 0    ; Size of matrix (n)
    matrixAllocated  BYTE 0     ; Flag: 1 if memory allocated, 0 if not

     ; Calculator variables
    prevResult    DWORD 0          ; Store previous result
    usePrev       DWORD 0          ; Flag to use previous result
    firstNum      DWORD 0          ; First operand
    secondNum     DWORD 0          ; Second operand
    remainder     DWORD 0          ; Remainder 

.code
main PROC
    call DisplayWelcomeMessage
    
    MenuLoop:
        call DisplayMenu
        call ReadUserChoice
        call ExecuteChoice
    
        ; After each operation, pause and return to menu
        mov edx, OFFSET continueMsg
        call WriteString
        call ReadChar           ; Wait for key press
        call Clrscr             ; Clear screen for fresh menu
        jmp MenuLoop
    
main ENDP

; Display welcome message
DisplayWelcomeMessage PROC
    mov edx, OFFSET welcomeMsg
    call WriteString
    call Crlf
    ret
DisplayWelcomeMessage ENDP

; Display the main menu
DisplayMenu PROC
    mov edx, OFFSET menuPrompt
    call WriteString
    ret
DisplayMenu ENDP

; Read user input (1-4)
ReadUserChoice PROC
    call ReadInt            ; Read integer into EAX
    ret
ReadUserChoice ENDP

; Execute based on user choice in EAX
ExecuteChoice PROC
    cmp eax, 1
    je CalculatorSection
    cmp eax, 2
    je StringSection
    cmp eax, 3
    je MemorySection
    cmp eax, 4
    je ExitProgram
    
    ; Invalid choice
    mov edx, OFFSET invalidChoice
    call WriteString
    ret
    
    CalculatorSection:
        call CalculatorModule
        ret
    
    StringSection:
        call StringModule
        ret
    
    MemorySection:
        call MemoryModule
        ret
    
    ExitProgram:
        INVOKE ExitProcess, 0
ExecuteChoice ENDP




;; ==================== CALCULATOR MODULE ====================
; Main procedure for arithmetic operation
; Handles Addition, Subtraction, Multiplication, Division
; ============================================================

CalculatorModule PROC
    ; Initialize previous result to 0
    mov prevResult, 0
    
    ; Main menu loop for calculator operations
    CalcMenuLoop:
        call DisplayCalcMenu      ; Display calculator menu options to user
        call ReadInt              ; Read user's menu choice as integer
        call Crlf                 ; Print New Line
        
        ; Compare user input with menu options and jump to appropriate handler
        cmp eax, 1                ; Check if user selected option 1
        je Addition               ; Jump to Addition handler
        cmp eax, 2                ; Check if user selected option 2  
        je Subtraction            ; Jump to Subtraction handler
        cmp eax, 3                ; Check if user selected option 3
        je Multiplication         ; Jump to Multiplication handler
        cmp eax, 4                ; Check if user selected option 4
        je Division               ; Jump to Division handler
        cmp eax, 5                ; Check if user selected option 5
        je CalcEnd                ; Jump to Exit handler
        
        ; ========== INVALID INPUT HANDLER ==========
        ; Display error message for invalid menu selection
        mov edx, OFFSET invalidChoiceMsg  ; Load address of error message
        call WriteString                  ; Display the error message
        jmp CalcMenuLoop                  ; Return to menu to allow user to try again

    ; ========== MENU OPTION HANDLERS ==========
    Addition:
        call AdditionProcedure       ; Call procedure to perform addition
        jmp AskForNewCalculation     ; Ask if user wants new calculation
    
    Subtraction:
        call SubtractionProcedure    ; Call procedure to perform subtraction
        jmp AskForNewCalculation     ; Ask if user wants new calculation
    
    Multiplication:
        call MultiplicationProcedure ; Call procedure to perform multiplication
        jmp AskForNewCalculation     ; Ask if user wants new calculation
    
    Division:
        call DivisionProcedure       ; Call procedure to perform division
        jmp AskForNewCalculation     ; Ask if user wants new calculation

    AskForNewCalculation:
        call Crlf
        mov edx, OFFSET newCalcPrompt
        call WriteString
        call ReadInt
        cmp eax, 1
        je CalcMenuLoop             ; Continue with new calculation
        
    CalcEnd:                         ; ========== EXIT HANDLER ==========
        ret                          ; Return from CalculatorModule procedure

CalculatorModule ENDP

; =============================================
; Display Calculator Menu
; =============================================
DisplayCalcMenu PROC
    call Clrscr                    ; Clear screen for fresh calculator interface
    mov edx, OFFSET calcTitle      ; Load address of calculator title string
    call WriteString               ; Display the calculator title
    call Crlf                      ; Output new line
    
    ; Display previous result if available
    cmp prevResult, 0
    je NoPrevResult
    mov edx, OFFSET calcResult
    call WriteString
    mov eax, prevResult
    call WriteInt
    call Crlf
    call Crlf
NoPrevResult:
    
    mov edx, OFFSET calcMenu       ; Load address of calculator menu string
    call WriteString               ; Display the menu options
    
    ret                            ; Return from procedure
DisplayCalcMenu ENDP

; =============================================
; Get Input Numbers Procedure
; =============================================
GetInputNumbers PROC
    ; Ask if user wants to use previous result
    cmp prevResult, 0
    je GetFirstNumber              ; No previous result available
    
    mov edx, OFFSET usePrevResult
    call WriteString
    call ReadInt
    mov usePrev, eax
    
    cmp usePrev, 1
    jne GetFirstNumber
    
    ; Use previous result as first number
    mov eax, prevResult
    mov firstNum, eax
    jmp GetSecondNumber

GetFirstNumber:
    mov edx, OFFSET enterFirst
    call WriteString
    call ReadInt
    mov firstNum, eax

GetSecondNumber:
    mov edx, OFFSET enterSecond
    call WriteString
    call ReadInt
    mov secondNum, eax
    
    ret
GetInputNumbers ENDP

; =============================================
; Addition Procedure
; =============================================
AdditionProcedure PROC
    call GetInputNumbers          ; Get both numbers from user
    
    ; Perform addition
    mov eax, firstNum
    add eax, secondNum
    mov prevResult, eax           ; Store result for potential future use
    
    ; Display result
    mov edx, OFFSET calcResult
    call WriteString
    call WriteInt
    call Crlf
    
    ret
AdditionProcedure ENDP

; =============================================
; Subtraction Procedure
; =============================================
SubtractionProcedure PROC
    call GetInputNumbers          ; Get both numbers from user
    
    ; Perform subtraction
    mov eax, firstNum
    sub eax, secondNum
    mov prevResult, eax           ; Store result for potential future use
    
    ; Display result
    mov edx, OFFSET calcResult
    call WriteString
    call WriteInt
    call Crlf
    
    ret
SubtractionProcedure ENDP

; =============================================
; Multiplication Procedure
; =============================================
MultiplicationProcedure PROC
    call GetInputNumbers          ; Get both numbers from user
    
    ; Perform multiplication
    mov eax, firstNum
    imul eax, secondNum           ; Signed multiplication
    mov prevResult, eax           ; Store result for potential future use
    
    ; Display result
    mov edx, OFFSET calcResult
    call WriteString
    call WriteInt
    call Crlf
    
    ret
MultiplicationProcedure ENDP

; =============================================
; Division Procedure
; =============================================
DivisionProcedure PROC
    call GetInputNumbers          ; Get both numbers from user
    
    ; Check for division by zero
    cmp secondNum, 0
    jne PerformDivision
    
    ; Division by zero error
    mov edx, OFFSET divByZeroMsg
    call WriteString
    ret

PerformDivision:
    ; Prepare for division
    mov eax, firstNum
    cdq                          ; Convert doubleword to quadword (sign extend EAX into EDX:EAX) 
    mov ebx, secondNum           ; Move divisor to EBX
    idiv ebx                     ; Signed division: EDX:EAX / EBX
    
    ; Store quotient as previous result
    mov prevResult, eax
    
    ; Save remainder before it gets overwritten
    mov remainder, edx                     ; Save remainder on stack
    
    ; Display quotient
    mov edx, OFFSET calcResult
    call WriteString
    mov eax, prevResult
    call WriteInt
    
    ; Check and display remainder
    mov eax, remainder
    cmp eax, 0
    je DivisionComplete         ; if remainder is 0, no need to continue with the result
    
    ; Display remainder
    mov edx, OFFSET remainderMsg
    call WriteString
    call WriteInt

DivisionComplete:
    call Crlf
    ret
DivisionProcedure ENDP




;; ==================== STRING MANIPULATION MODULE ====================
; Main procedure for string manipulation operation
; Handles string reverse, concatenation, copy, and compare
; ===================================================================

StringModule PROC
    ; Main menu loop for string operations
    StringMenuLoop:
        call DisplayStringMenu         ; Display string operations menu to user
        call ReadInt                   ; Read user's menu choice as integer
        call Crlf                      ; Print New Line
        
        ; Compare user input with menu options and jump to appropriate handler
        cmp eax, 1                     ; Check if user selected option 1
        je StringReverseOp             ; Jump to String Reverse handler
        cmp eax, 2                     ; Check if user selected option 2
        je StringConcatenationOp       ; Jump to String Concatenation handler
        cmp eax, 3                     ; Check if user selected option 3
        je StringCopyOp                ; Jump to String Copy handler
        cmp eax, 4                     ; Check if user selected option 4
        je StringCompareOp             ; Jump to String Compare handler
        cmp eax, 5                     ; Check if user selected option 5
        je StringEnd                   ; Jump to Exit handler
        
        ; ========== INVALID INPUT HANDLER ==========
        ; Display error message for invalid menu selection
        mov edx, OFFSET invalidChoiceMsg  ; Load address of error message
        call WriteString                  ; Display the error message
        jmp StringMenuLoop                ; Return to menu to allow user to try again

    ; ========== MENU OPTION HANDLERS ==========
    
    StringReverseOp:
        call ReverseStringProcedure         ; Call procedure to reverse string
        jmp StringMenuLoop                  ; Return to main menu after completion

    StringConcatenationOp:
        call ConcatenateStringsProcedure    ; Call procedure to concatenate strings
        jmp StringMenuLoop                  ; Return to main menu after completion

    StringCopyOp:
        call CopyStringProcedure            ; Call procedure to copy string
        jmp StringMenuLoop                  ; Return to main menu after completion

    StringCompareOp:
        call CompareStringsProcedure        ; Call procedure to compare strings
        jmp StringMenuLoop                  ; Return to main menu after completion

    StringEnd:                             ; ========== EXIT HANDLER ==========
        ret                                ; Return from StringModule procedure

StringModule ENDP

; =============================================
; Display String Menu
; =============================================
DisplayStringMenu PROC
    call Clrscr                    ; Clear screen for fresh string interface
    mov edx, OFFSET stringTitle    ; Load address of string title string
    call WriteString               ; Display the string title
    call Crlf                      ; Output new line
    
    mov edx, OFFSET stringMenu     ; Load address of string menu string
    call WriteString               ; Display the menu options
    
    ret                            ; Return from procedure
DisplayStringMenu ENDP

; =============================================
; Reverse String Procedure
; =============================================
ReverseStringProcedure PROC
    mov edx, OFFSET msg1
    call WriteString               ; Prompt user for string
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString                ; Read string into str1

    mov esi, OFFSET str1
    call StrLength                 ; Irvine32: returns string length in EAX
    mov ecx, eax                   ; Store length in ECX (loop counter)
    mov esi, OFFSET str1
    add esi, ecx                   ; Move ESI to end of string
    dec esi                        ; Point to last valid char
    mov edi, OFFSET result         ; Destination for reversed string

    RevLoop:
        movzx eax, BYTE PTR [esi]  ; Move character into EAX (zero-extended)
        mov BYTE PTR [edi], al     ; Store in result
        dec esi                    ; Move backward in source
        inc edi                    ; Move forward in destination
        loop RevLoop               ; Repeat for each character

    mov BYTE PTR [edi], 0          ; Null-terminate reversed string

    mov edx, OFFSET reversedStr
    call WriteString
    mov edx, OFFSET result
    call WriteString               ; Display reversed string
    call Crlf
    call PauseForInput             ; Wait for key press
    ret
ReverseStringProcedure ENDP

; =============================================
; Concatenate Strings Procedure
; =============================================
ConcatenateStringsProcedure PROC
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString                ; Get first string

    mov edx, OFFSET msg2
    call WriteString
    mov edx, OFFSET str2
    mov ecx, SIZEOF str2
    call ReadString                ; Get second string

    ; Copy str1 into result
    push OFFSET result             ; dest
    push OFFSET str1               ; src
    call MyStrCopy

    ; Append str2 to result
    push OFFSET result             ; dest (append to)
    push OFFSET str2               ; src
    call MyStrCat

    mov edx, OFFSET concatResult
    call WriteString
    mov edx, OFFSET result
    call WriteString               ; Show concatenated result
    call Crlf
    call PauseForInput
    ret
ConcatenateStringsProcedure ENDP

; =============================================
; Copy String Procedure
; =============================================
CopyStringProcedure PROC
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString                ; Read string

    push OFFSET result             ; dest
    push OFFSET str1               ; src
    call MyStrCopy                 ; Copy str1 -> result

    mov edx, OFFSET copyResult
    call WriteString
    mov edx, OFFSET result
    call WriteString               ; Print copied string
    call Crlf
    call PauseForInput
    ret
CopyStringProcedure ENDP

; =============================================
; Compare Strings Procedure
; =============================================
CompareStringsProcedure PROC
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString                ; Read first string

    mov edx, OFFSET msg2
    call WriteString
    mov edx, OFFSET str2
    mov ecx, SIZEOF str2
    call ReadString                ; Read second string

    push OFFSET str2               ; Parameter 2
    push OFFSET str1               ; Parameter 1
    call MyStrCompare              ; Compare both

    cmp eax, 0                     ; Result from MyStrCompare
    jne NotEqual
    mov edx, OFFSET compareEqual   ; Strings are equal
    call WriteString
    call Crlf
    call PauseForInput
    ret

    NotEqual:
        mov edx, OFFSET compareNotEqual  ; Strings not equal
        call WriteString
        call Crlf
        call PauseForInput
    ret
CompareStringsProcedure ENDP

; =============================================
; Helper Procedure: Pause For Input
; RENAMED from WaitMsg to avoid conflict with Irvine32 library
; =============================================
PauseForInput PROC
    mov edx, OFFSET continueMsg
    call WriteString
    call ReadChar                  ; Wait for key press
    ret
PauseForInput ENDP

; =============================================
; Custom String Functions
; =============================================

;-------------------------------------------------------------
; MyStrCopy(dest, src)
; Copies a string from src -> dest (including null terminator)
; Parameters: [esp+8] = src, [esp+12] = dest
;-------------------------------------------------------------
MyStrCopy PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]               ; Source string pointer
    mov edi, [ebp+12]              ; Destination pointer

    CopyLoop:
        movzx eax, BYTE PTR [esi]  ; Read one byte into EAX
        mov BYTE PTR [edi], al     ; Write to destination
        inc esi                    ; Next source char
        inc edi                    ; Next dest position
        cmp al, 0                  ; End of string?
        jne CopyLoop               ; If not, continue

    pop ebp
    ret 8                          ; Remove parameters
MyStrCopy ENDP

;-------------------------------------------------------------
; MyStrCat(dest, src)
; Appends src at the end of dest
; Parameters: [esp+8] = src, [esp+12] = dest
;-------------------------------------------------------------
MyStrCat PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]               ; src
    mov edi, [ebp+12]              ; dest

    FindEnd:
        movzx eax, BYTE PTR [edi]  ; Find null terminator in dest
        cmp al, 0
        je CopyStart
        inc edi
        jmp FindEnd

    CopyStart:
        movzx eax, BYTE PTR [esi]  ; Copy from src -> dest
        mov BYTE PTR [edi], al
        inc esi
        inc edi
        cmp al, 0
        jne CopyStart              ; Continue until null terminator

    pop ebp
    ret 8
MyStrCat ENDP

;-------------------------------------------------------------
; MyStrCompare(s1, s2)
; Compares two strings character by character
; Parameters: [esp+8] = s1, [esp+12] = s2
; Returns:
;   EAX = 0 -> Equal
;   EAX = 1 -> Not Equal
;-------------------------------------------------------------
MyStrCompare PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]               ; String 1
    mov edi, [ebp+12]              ; String 2

    CompareLoop:
        movzx eax, BYTE PTR [esi]  ; Read byte from s1
        movzx ebx, BYTE PTR [edi]  ; Read byte from s2
        cmp eax, ebx               ; Compare characters
        jne NotEqualStrings        ; Mismatch -> not equal
        cmp eax, 0                 ; Check for end of string
        je EqualStrings            ; Both ended -> equal
        inc esi
        inc edi
        jmp CompareLoop            ; Next characters

    EqualStrings:
        mov eax, 0                 ; Strings equal
        jmp ComparisonDone
    
    NotEqualStrings:
        mov eax, 1                 ; Strings not equal
    
    ComparisonDone:
        pop ebp
        ret 8
MyStrCompare ENDP




;; ==================== MEMORY MANAGEMENT MODULE ====================
; Main procedure for memory management operations
; Handles matrix creation, display, addition, and memory management
; ===================================================================

MemoryModule PROC
    ; Main menu loop for memory management operations
    MemoryMenuLoop:
        call DisplayMemoryMenu  ; Display memory management menu options to user
        call ReadInt            ; Read user's menu choice as integer
        call Crlf               ; Print New Line
        
        ; Compare user input with menu options and jump to appropriate handler
        cmp eax, 1              ; Check if user selected option 1
        je CreateMatrix             ; Jump to Create Matrix handler
        cmp eax, 2              ; Check if user selected option 2  
        je DisplayMatrix            ; Jump to Display Matrix handler
        cmp eax, 3              ; Check if user selected option 3
        je AddMatrices              ; Jump to Matrix Addition handler
        cmp eax, 4              ; Check if user selected option 4
        je FreeMemory               ; Jump to Free Memory handler
        cmp eax, 5              ; Check if user selected option 5
        je MemoryEnd                ; Jump to Exit handler
        
        ; ========== INVALID INPUT HANDLER ==========
        ; Display error message for invalid menu selection
        mov edx, OFFSET invalidChoiceMsg  ; Load address of error message
        call WriteString                  ; Display the error message

        jmp MemoryMenuLoop                ; Return to menu to allow user to try again

    ; ========== MENU OPTION HANDLERS ==========
    
    CreateMatrix:
        call CreateMatrixProcedure       ; Call procedure to create a new matrix in memory
        jmp MemoryMenuLoop               ; Return to main menu after completion

    DisplayMatrix:
        call DisplayMatrixProcedure      ; Call procedure to display matrix contents
        jmp MemoryMenuLoop               ; Return to main menu after completion

    AddMatrices:
        call MatrixAdditionProcedure     ; Call procedure to perform matrix addition operation
        jmp MemoryMenuLoop               ; Return to main menu after completion

    FreeMemory:
        call FreeMemoryProcedure         ; Call procedure to free allocated matrix memory
        jmp MemoryMenuLoop               ; Return to main menu after completion

    MemoryEnd:                           ; ========== CLEANUP AND EXIT HANDLER ==========
        ; Check if any matrices are currently allocated before exiting
        cmp matrixAllocated, 0           ; Compare allocation flag with 0
        je NoCleanupNeeded               ; Skip cleanup if no matrices allocated
        
        ; Clean up all allocated matrix memory before exiting module
        call FreeAllMatrices             ; Free any allocated memory
    
    NoCleanupNeeded:
        ret                              ; Return from MemoryModule procedure

MemoryModule ENDP

; =============================================
; Display Menu
; =============================================
DisplayMemoryMenu PROC
    mov edx, OFFSET menuTitle        ; Load address of menu title string
    call WriteString                 ; Display the menu title
    call Crlf                        ; Output new line
    
    mov edx, OFFSET menuOption1      ; Load address of menu option string
    call WriteString                 ; Display the menu option
    
    ret                              ; Return from procedure
DisplayMemoryMenu ENDP

; =============================================
; Create Matrix Procedure
; =============================================
CreateMatrixProcedure PROC
    ; Free existing matrices if any
    cmp matrixAllocated, 0                ; Check if matrices are currently allocated
    je NoFreeNeeded                       ; Skip freeing if no matrices exist
    call FreeAllMatrices                  ; Free any previously allocated matrices

    NoFreeNeeded:
        ; Get matrix size from user
        mov edx, OFFSET matrixSizePrompt  ; Load address of size prompt message
        call WriteString                  ; Ask user for matrix size
        call ReadInt                      ; Read integer input from user
        mov matrixSize, eax               ; Store the matrix size
    
        ; Calculate total memory needed: n * n * 4 bytes (for DWORD elements)
        mov eax, matrixSize               ; Load matrix size into EAX
        mul matrixSize                    ; Multiply EAX by matrixSize (EAX = n * n)
        mov ebx, eax                      ; Save element count in EBX
        shl eax, 2                        ; Multiply by 4 (shift left 2 = ×4 for DWORD size)
        mov edx, eax                      ; Save total byte size in EDX
    
        ; Allocate memory for matrix A
        call GetProcessHeap               ; Get handle to process heap
        push 8                            ; Push HEAP_ZERO_MEMORY flag (initialize to zero)
        push edx                          ; Push size in bytes to allocate
        push eax                          ; Push heap handle
        call HeapAlloc                    ; Allocate memory block
        mov matrixA, eax                  ; Store pointer to allocated memory for matrix A
    
        ; Allocate memory for matrix B
        call GetProcessHeap               ; Get handle to process heap
        push 8                            ; Push HEAP_ZERO_MEMORY flag
        push edx                          ; Push size in bytes to allocate
        push eax                          ; Push heap handle
        call HeapAlloc                    ; Allocate memory block
        mov matrixB, eax                  ; Store pointer to allocated memory for matrix B
    
        ; Allocate memory for matrix C (result)
        call GetProcessHeap               ; Get handle to process heap
        push 8                            ; Push HEAP_ZERO_MEMORY flag
        push edx                          ; Push size in bytes to allocate
        push eax                          ; Push heap handle
        call HeapAlloc                    ; Allocate memory block
        mov matrixC, eax                  ; Store pointer to allocated memory for matrix C
    
        ; Input values for matrix A
        mov edx, OFFSET matrixDisplayRow  ; Load address of matrix display prefix
        call WriteString                  ; Display the prefix text
        mov al, 'A'                       ; Load 'A' character for matrix identifier
        call WriteChar                    ; Display matrix identifier
        mov al, ':'                       ; Load colon character
        call WriteChar                    ; Display colon
        call Crlf                         ; Output new line
        mov esi, matrixA                  ; Load pointer to matrix A into ESI
        call InputMatrixValues            ; Call procedure to input matrix values
    
        ; Input values for matrix B
        mov edx, OFFSET matrixDisplayRow  ; Load address of matrix display prefix
        call WriteString                  ; Display the prefix text
        mov al, 'B'                       ; Load 'B' character for matrix identifier
        call WriteChar                    ; Display matrix identifier
        mov al, ':'                       ; Load colon character
        call WriteChar                    ; Display colon
        call Crlf                         ; Output new line
        mov esi, matrixB                  ; Load pointer to matrix B into ESI
        call InputMatrixValues            ; Call procedure to input matrix values
    
        mov matrixAllocated, 1            ; Set allocation flag to indicate matrices exist
        mov edx, OFFSET matrixCreatedMsg  ; Load address of success message
        call WriteString                  ; Display creation success message
        call Crlf                         ; Output new line
    
        ret                               ; Return from procedure
CreateMatrixProcedure ENDP

; =============================================
; Input Matrix Values
; =============================================
InputMatrixValues PROC
    mov ecx, matrixSize    ; Set outer loop counter for rows
    mov edi, 0             ; Initialize row index to 0
    
    RowLoop:
        push ecx               ; Save outer loop counter
        mov ecx, matrixSize    ; Set inner loop counter for columns
        mov ebx, 0             ; Initialize column index to 0
        
        ColLoop:
            ; Display prompt for element
            push edx                            ; Save EDX register
            mov edx, OFFSET matrixElemPrompt    ; Load address of element prompt part 1
            call WriteString                    ; Display "Enter element ["
            
            ; Display row index, col index
            mov eax, edi                        ; Load current row index
            call WriteDec                       ; Display row number
            mov al, ','                         ; Load comma character
            call WriteChar                      ; Display comma
            mov eax, ebx                        ; Load current column index
            call WriteDec                       ; Display column number
            
            mov edx, OFFSET matrixElemPrompt2   ; Load address of element prompt part 2
            call WriteString                    ; Display "]: "
            pop edx                             ; Restore EDX register
            
            ; Read integer value
            call ReadInt           ; Read integer input from user
            mov [esi], eax         ; Store value in current matrix position
            
            ; Move to next element
            add esi, 4             ; Advance pointer to next DWORD element
            inc ebx                ; Increment column index
            loop ColLoop           ; Continue inner loop for columns
            
        inc edi                ; Increment row index
        pop ecx                ; Restore outer loop counter
        loop RowLoop           ; Continue outer loop for rows
        
        ret                    ; Return from procedure
InputMatrixValues ENDP

; =============================================
; Display Matrix Procedure
; =============================================
DisplayMatrixProcedure PROC
    cmp matrixAllocated, 0          ; Check if matrices are allocated
    jne MatricesAllocated          ; Jump if matrices exist
    
    mov edx, OFFSET noMatrixMsg     ; Load address of error message
    call WriteString                ; Display "No matrices created" message
    ret                             ; Return early
    
    MatricesAllocated:
        ; Display matrix A
        mov edx, OFFSET matrixDisplayRow    ; Load address of display prefix
        call WriteString                    ; Display "Matrix "
        mov al, 'A'                         ; Load matrix identifier 'A'
        call WriteChar                      ; Display 'A'
        mov al, ':'                         ; Load colon character
        call WriteChar                      ; Display ':'
        call Crlf                           ; Output new line
        
        mov esi, matrixA                    ; Load pointer to matrix A
        call DisplaySingleMatrix            ; Display matrix A contents
        call Crlf                           ; Output blank line
        
        ; Display matrix B
        mov edx, OFFSET matrixDisplayRow    ; Load address of display prefix
        call WriteString                    ; Display "Matrix "
        mov al, 'B'                         ; Load matrix identifier 'B'
        call WriteChar                      ; Display 'B'
        mov al, ':'                         ; Load colon character
        call WriteChar                      ; Display ':'
        call Crlf                           ; Output new line
        
        mov esi, matrixB                    ; Load pointer to matrix B
        call DisplaySingleMatrix            ; Display matrix B contents
        call Crlf                           ; Output blank line

        ret                                 ; Return from procedure
DisplayMatrixProcedure ENDP

; =============================================
; Display Single Matrix
; =============================================
DisplaySingleMatrix PROC
    mov ecx, matrixSize    ; Set outer loop counter for rows
    mov edi, 0             ; Initialize row index to 0
    
    display_RowLoop:
        push ecx               ; Save outer loop counter
        
        ; Display row label
        mov edx, OFFSET matrixDisplayRow    ; Load address of display prefix
        call WriteString                    ; Display "Matrix "
        mov eax, edi                        ; Load current row index
        call WriteDec                       ; Display row number
        mov edx, OFFSET matrixDisplayCol    ; Load address of column separator
        call WriteString                    ; Display "]: "
        
        mov ecx, matrixSize                 ; Set inner loop counter for columns
        
        DisplayColLoop:
            mov eax, [esi]                  ; Load current matrix element
            call WriteInt                   ; Display the integer value
            mov edx, OFFSET spaceChar       ; Load address of space character
            call WriteString                ; Display space between elements
            
            add esi, 4                      ; Advance pointer to next DWORD element
            loop DisplayColLoop             ; Continue inner loop for columns
            call Crlf                       ; Output new line after each row

        inc edi                             ; Increment row index
        pop ecx                             ; Restore outer loop counter
        loop display_RowLoop                ; Continue outer loop for rows
    
    ret                                     ; Return from procedure
DisplaySingleMatrix ENDP

; =============================================
; Matrix Addition Procedure
; =============================================
MatrixAdditionProcedure PROC
    cmp matrixAllocated, 0         ; Check if matrices are allocated
    jne AdditionPossible           ; Jump if matrices exist
    
    mov edx, OFFSET noMatrixMsg    ; Load address of error message
    call WriteString               ; Display "No matrices created" message
    ret                            ; Return early
    
    AdditionPossible:
        ; Perform matrix addition: C = A + B
        mov esi, matrixA      ; Load pointer to matrix A
        mov edi, matrixB      ; Load pointer to matrix B  
        mov ebx, matrixC      ; Load pointer to matrix C (result)
        
        ; Calculate total elements
        mov eax, matrixSize   ; Load matrix size
        mul matrixSize        ; Multiply size by itself (EAX = n * n)
        mov ecx, eax          ; Set loop counter to total elements
        
        AdditionLoop:
            mov eax, [esi]        ; Get element from matrix A
            add eax, [edi]        ; Add element from matrix B
            mov [ebx], eax        ; Store result in matrix C
            
            ; Move to next elements
            add esi, 4            ; Advance matrix A pointer
            add edi, 4            ; Advance matrix B pointer
            add ebx, 4            ; Advance matrix C pointer
            loop AdditionLoop     ; Continue for all elements
        
        ; Display result
        mov edx, OFFSET matrixAddResult    ; Load address of result message
        call WriteString                   ; Display "Matrix C (A+B):"
        
        mov esi, matrixC                   ; Load pointer to result matrix
        call DisplaySingleMatrix           ; Display the resulting matrix
        call Crlf                          ; Output blank line

        ret                   ; Return from procedure
MatrixAdditionProcedure ENDP

; =============================================
; Free Memory Procedure
; =============================================
FreeMemoryProcedure PROC
    cmp matrixAllocated, 0    ; Check if matrices are allocated
    jne FreeMatrices          ; Jump if matrices exist
    
    mov edx, OFFSET noMatrixMsg    ; Load address of error message
    call WriteString               ; Display "No matrices created" message
    ret                            ; Return early
    
    FreeMatrices:
        call FreeAllMatrices              ; Call procedure to free all matrices
        mov edx, OFFSET matrixFreedMsg    ; Load address of success message
        call WriteString                  ; Display "Matrices freed" message
    
    ret                               ; Return from procedure
FreeMemoryProcedure ENDP

; =============================================
; Free All Matrices
; =============================================
FreeAllMatrices PROC
    ; Free matrix A
    cmp matrixA, 0              ; Check if matrix A exists
    je free_matrixB             ; Skip if matrix A is null
    
    call GetProcessHeap         ; Get handle to process heap
    push 0                      ; No flags for HeapFree
    push matrixA                ; Pointer to memory block to free
    push eax                    ; Heap handle
    call HeapFree               ; Free matrix A memory
    mov matrixA, 0              ; Clear matrix A pointer
    
    free_matrixB:
        cmp matrixB, 0          ; Check if matrix B exists
        je free_matrixC         ; Skip if matrix B is null
    
        call GetProcessHeap     ; Get handle to process heap
        push 0                  ; No flags for HeapFree
        push matrixB            ; Pointer to memory block to free
        push eax                ; Heap handle
        call HeapFree           ; Free matrix B memory
        mov matrixB, 0          ; Clear matrix B pointer
    
    free_matrixC:
        cmp matrixC, 0          ; Check if matrix C exists
        je free_complete        ; Skip if matrix C is null
    
        call GetProcessHeap     ; Get handle to process heap
        push 0                  ; No flags for HeapFree
        push matrixC            ; Pointer to memory block to free
        push eax                ; Heap handle
        call HeapFree           ; Free matrix C memory
        mov matrixC, 0          ; Clear matrix C pointer
    
    free_complete:
        mov matrixAllocated, 0    ; Clear allocation flag
        ret                       ; Return from procedure
FreeAllMatrices ENDP

END main
