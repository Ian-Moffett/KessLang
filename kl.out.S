; This assembly program was
; generated automatically by
; the KessLang compiler which
; is written by Ian Moffett.

global _start

section .text
_start: jmp _0

_0:
    jmp _1

f_test:
    mov eax, 4
    mov ecx, 'A'
    mov edx, 1
    int 0x80
    ret

_1:
    call f_test

_2:
    mov eax, 1
    mov ebx, 0
    int 0x80
