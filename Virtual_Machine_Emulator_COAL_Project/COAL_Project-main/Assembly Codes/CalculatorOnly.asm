.386
.model flat, stdcall
.stack 4096
ExitProcess PROTO, dwExitCode:DWORD     ; Written so that terminal doesnt close unless we tell it so

INCLUDE Irvine32.inc

.data
    ; Menu and UI strings
    welcomeMsg    BYTE "=== Virtual Machine with Calculator Operation ===", 0Dh, 0Ah, 0
    menuPrompt    BYTE "Please select an option:", 0Dh, 0Ah
                  BYTE "1. Calculator", 0Dh, 0Ah
                  BYTE "2. Exit Program", 0Dh, 0Ah
                  BYTE "Enter your choice (1-4): ", 0
    invalidChoice BYTE "Invalid choice! Please enter 1-2.", 0Dh, 0Ah, 0
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
    invalidChoiceMsg BYTE "Invalid choice! Please enter 1-5.", 0Dh, 0Ah, 0
    
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
    je ExitProgram
    
    ; Invalid choice
    mov edx, OFFSET invalidChoice
    call WriteString
    ret
    
    CalculatorSection:
        call CalculatorModule
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

END main
