USE64
;pop	r11
;push	rax
mov     r11, [rsp]
push	r9
mov     r9, r8
mov     r8, rcx
mov     rcx, rdx
mov     rdx, rsi
mov     rsi, rdi
;int sz = 8 * (INTEGER_ARGS - 6 + std::max(SSE_ARGS - 8, 0));
mov     rax, rsp
add     rax, 0x12345678 ; + sz
add     rsp, 0x12344578
lbl:
cmp     rax, rsp
je      lbl2
add     rsp, 0x00000008
mov     rdi, [rsp]
mov     [rsp-0x8],rdi
jmp     lbl
lbl2:
mov     [rsp], r11
sub     rsp, 0x00000008 ; + sz
mov     rdi, 0x0123456789abcdef
mov     rax, 0x0123456789abcdef
call    rax
pop     r9
;mov     rax, rsp
;add     rax, 0x00000000 ; + sz
;mov	r11, [rax]
mov     r11, [rsp + 123456]
;pop	r9
;push	r11
mov     [rsp], r11
ret




