;-------------------------------------------------------------
;   Program: String Manipulation Menu
;   Author : Dev / Hashir
;   Description:
;       This program performs string operations using MASM
;       and the Irvine32 library.
;       Operations include:
;         1. Reverse a string
;         2. Concatenate two strings
;         3. Copy a string
;         4. Compare two strings
;       The user can repeatedly choose an option until Exit.
;-------------------------------------------------------------

INCLUDE Irvine32.inc     ; include library for I/O routines

.data
;-------------------------------------------------------------
;  Data section: All variables and messages
;-------------------------------------------------------------
menuMsg  BYTE "String Manipulation Menu",0Dh,0Ah,0
optMsg   BYTE "1. Reverse String",0Dh,0Ah,
               "2. Concatenate Strings",0Dh,0Ah,
               "3. Copy String",0Dh,0Ah,
               "4. Compare Strings",0Dh,0Ah,
               "5. Exit",0Dh,0Ah,
               "Enter choice: ",0

msg1     BYTE "Enter first string: ",0
msg2     BYTE "Enter second string: ",0
msgRes   BYTE "Result: ",0
equalMsg BYTE "Strings are equal.",0
notMsg   BYTE "Strings are NOT equal.",0

str1     BYTE 100 DUP(?)        ; buffer for first string
str2     BYTE 100 DUP(?)        ; buffer for second string
result   BYTE 200 DUP(?)        ; large buffer for concatenation result
choice   BYTE ?                 ; store user’s numeric menu choice

.code
main PROC
;-------------------------------------------------------------
;  Main menu loop: Displays menu, reads choice, and executes
;-------------------------------------------------------------
MenuLoop:
    call Clrscr                  ; clear the console screen

    mov edx, OFFSET menuMsg      ; print menu heading
    call WriteString
    mov edx, OFFSET optMsg       ; print options
    call WriteString

    call ReadInt                 ; read integer input (waits for Enter)
    mov choice, al               ; store low byte of input (1–5)

    ; Compare input to valid menu options
    cmp eax, 1
    je Reverse                   ; if 1 ? Reverse String
    cmp eax, 2
    je Concat                    ; if 2 ? Concatenate
    cmp eax, 3
    je CopyStr                   ; if 3 ? Copy
    cmp eax, 4
    je Compare                   ; if 4 ? Compare
    cmp eax, 5
    je ExitProgram               ; if 5 ? Exit
    jmp MenuLoop                 ; invalid ? show menu again


;=============================================================
; 1. Reverse String
;=============================================================
Reverse:
    mov edx, OFFSET msg1
    call WriteString             ; prompt user
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString              ; input string ? str1

    mov esi, OFFSET str1
    call StrLength               ; Irvine32: returns string length in EAX
    mov ecx, eax                 ; store length in ECX (loop counter)
    mov esi, OFFSET str1
    add esi, ecx                 ; move ESI to end of string
    dec esi                      ; point to last valid char
    mov edi, OFFSET result       ; destination for reversed string

RevLoop:
    movzx eax, BYTE PTR [esi]    ; move character into EAX (zero-extended)
    mov BYTE PTR [edi], al       ; store in result
    dec esi                      ; move backward in source
    inc edi                      ; move forward in destination
    loop RevLoop                 ; repeat for each character

    mov BYTE PTR [edi], 0        ; null-terminate reversed string

    mov edx, OFFSET msgRes
    call WriteString
    mov edx, OFFSET result
    call WriteString             ; display reversed string
    call Crlf
    call WaitMsg                 ; wait for key press
    jmp MenuLoop                 ; return to menu


;=============================================================
; 2. Concatenate Strings
;=============================================================
Concat:
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString              ; get first string

    mov edx, OFFSET msg2
    call WriteString
    mov edx, OFFSET str2
    mov ecx, SIZEOF str2
    call ReadString              ; get second string

    ; Copy str1 into result
    push OFFSET result           ; dest
    push OFFSET str1             ; src
    call MyStrCopy

    ; Append str2 to result
    push OFFSET result           ; dest (append to)
    push OFFSET str2             ; src
    call MyStrCat

    mov edx, OFFSET msgRes
    call WriteString
    mov edx, OFFSET result
    call WriteString             ; show concatenated result
    call Crlf
    call WaitMsg
    jmp MenuLoop


;=============================================================
; 3. Copy String
;=============================================================
CopyStr:
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString              ; read string

    push OFFSET result           ; dest
    push OFFSET str1             ; src
    call MyStrCopy               ; copy str1 ? result

    mov edx, OFFSET msgRes
    call WriteString
    mov edx, OFFSET result
    call WriteString             ; print copied string
    call Crlf
    call WaitMsg
    jmp MenuLoop


;=============================================================
; 4. Compare Strings
;=============================================================
Compare:
    mov edx, OFFSET msg1
    call WriteString
    mov edx, OFFSET str1
    mov ecx, SIZEOF str1
    call ReadString              ; read first string

    mov edx, OFFSET msg2
    call WriteString
    mov edx, OFFSET str2
    mov ecx, SIZEOF str2
    call ReadString              ; read second string

    push OFFSET str2             ; parameter 2
    push OFFSET str1             ; parameter 1
    call MyStrCompare            ; compare both

    cmp eax, 0                   ; result from MyStrCompare
    jne _notEqual
    mov edx, OFFSET equalMsg     ; strings are equal
    call WriteString
    call Crlf
    call WaitMsg
    jmp MenuLoop

_notEqual:
    mov edx, OFFSET notMsg       ; strings not equal
    call WriteString
    call Crlf
    call WaitMsg
    jmp MenuLoop


;=============================================================
; Exit Program
;=============================================================
ExitProgram:
    call Crlf
    exit                         ; terminate program
main ENDP


;=============================================================
;  Custom String Functions
;=============================================================

;-------------------------------------------------------------
; MyStrCopy(dest, src)
; Copies a string from src ? dest (including null terminator)
;-------------------------------------------------------------
MyStrCopy PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]             ; source string pointer
    mov edi, [ebp+12]            ; destination pointer

_copyLoop:
    movzx eax, BYTE PTR [esi]    ; read one byte into EAX
    mov BYTE PTR [edi], al       ; write to destination
    inc esi                      ; next source char
    inc edi                      ; next dest position
    cmp al, 0                    ; end of string?
    jne _copyLoop                ; if not, continue

    pop ebp
    ret 8                        ; remove parameters
MyStrCopy ENDP


;-------------------------------------------------------------
; MyStrCat(dest, src)
; Appends src at the end of dest
;-------------------------------------------------------------
MyStrCat PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]             ; src
    mov edi, [ebp+12]            ; dest

_findEnd:
    movzx eax, BYTE PTR [edi]    ; find null terminator in dest
    cmp al, 0
    je _copyStart
    inc edi
    jmp _findEnd

_copyStart:
    movzx eax, BYTE PTR [esi]    ; copy from src ? dest
    mov BYTE PTR [edi], al
    inc esi
    inc edi
    cmp al, 0
    jne _copyStart               ; continue until null terminator

    pop ebp
    ret 8
MyStrCat ENDP


;-------------------------------------------------------------
; MyStrCompare(s1, s2)
; Compares two strings character by character
; Returns:
;   EAX = 0 ? Equal
;   EAX = 1 ? Not Equal
;-------------------------------------------------------------
MyStrCompare PROC
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]             ; string 1
    mov edi, [ebp+12]            ; string 2

_cmpLoop:
    movzx eax, BYTE PTR [esi]    ; read byte from s1
    movzx ebx, BYTE PTR [edi]    ; read byte from s2
    cmp eax, ebx                 ; compare characters
    jne _notEq                   ; mismatch ? not equal
    cmp eax, 0                   ; check for end of string
    je _eq                       ; both ended ? equal
    inc esi
    inc edi
    jmp _cmpLoop                 ; next characters

_eq:
    mov eax, 0                   ; strings equal
    jmp _done
_notEq:
    mov eax, 1                   ; strings not equal
_done:
    pop ebp
    ret 8
MyStrCompare ENDP

END main
