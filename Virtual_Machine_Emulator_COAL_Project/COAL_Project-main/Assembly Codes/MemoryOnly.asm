.386
.model flat, stdcall
.stack 4096
ExitProcess PROTO, dwExitCode:DWORD     ; Written so that terminal doesnt close unless we tell it so


INCLUDE Irvine32.inc

.data
    ; Menu and UI strings
    welcomeMsg    BYTE "=== Virtual Machine with Memory Management ===", 0Dh, 0Ah, 0
    menuPrompt    BYTE "Please select an option:", 0Dh, 0Ah
                  BYTE "1. Memory Management", 0Dh, 0Ah
                  BYTE "2. Exit Program", 0Dh, 0Ah
                  BYTE "Enter your choice (1-4): ", 0
    invalidChoice BYTE "Invalid choice! Please enter 1-2.", 0Dh, 0Ah, 0
    continueMsg   BYTE "Press any key to continue...", 0Dh, 0Ah, 0 
    
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
    je MemorySection
    cmp eax, 2
    je ExitProgram
    
    ; Invalid choice
    mov edx, OFFSET invalidChoice
    call WriteString
    ret
    
    MemorySection:
        call MemoryModule
        ret
    
    ExitProgram:
        INVOKE ExitProcess, 0
ExecuteChoice ENDP




;; ==================== MEMORY MANAGEMENT MODULE ====================
; Main procedure for memory management operations
; Handles matrix creation, display, addition, and memory management
; ===================================================================

MemoryModule PROC
    ; Main menu loop for memory management operations
    call Clrscr                    ; Clear screen for fresh calculator interface
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
; Create Matrix Procedure - FIXED VERSION [Definately Final ONE]
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

        inc edi                         ; Increment row index
        pop ecx                         ; Restore outer loop counter
        loop display_RowLoop            ; Continue outer loop for rows
    
    ret                                 ; Return from procedure
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
; Free All Matrices - FIXED VERSION
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
