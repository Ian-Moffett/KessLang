; This assembly program was
; generated automatically by
; the KessLang compiler which
; is written by Ian Moffett.

global _start

section .text
_start: jmp _0

_0:
    jmp _1

f_exit_ok:
    mov eax, 1
    mov ebx, 0
    int 0x80
    ret

_1:
    jmp _2

f_exit_err:
    mov eax, 1
    mov ebx, 1
    int 0x80
    ret

_2:
    call f_exit_ok

_3: jmp _5

section .data
_4: db "Will this be printed?", 0xA

section .text
_5:
    mov eax, 4
    mov ebx, 1
    mov ecx, _4
    mov edx, 22
    int 0x80

_6:
    mov eax, 1
    mov ebx, 0
    int 0x80
