section .data
new: db 10
minus: db "-", 0

section .text
global exit
global string_length
global print_string
global print_char
global print_newline
global print_uint
global read_char
global read_word
global string_equals
global string_copy
global parse_int
global parse_uint
 
; Принимает код возврата и завершает текущий процесс
exit: 
    xor rax, rax
    mov rax, 60
    syscall
    ret 

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
.loop:
    cmp byte[rdi+rax],0
    je .end
    inc rax
    jmp .loop
.end:
    ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    xor rax, rax
    mov rsi, rdi
    call string_length
    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    syscall
    ret


; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rax, 1
    mov rsi, rsp
    mov rdi, 1
    mov rdx, 1
    syscall
    pop rdi
    ret


; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    xor rax, rax
    mov rdi, new
    call print_char
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    xor rax, rax
    xor rcx, rcx
    mov rax, rdi
    mov rdi, 10
    push 0x00
.loop:
    xor rdx, rdx
    div rdi
    add rdx, '0'
    push rdx
    test rax, rax
    jnz .loop
.out:
    pop rdi
    cmp rdi, 0x00
    je .exit
    call print_char
    jmp .out
.exit:
    ret

print_int:
    xor rax, rax
    test rdi, rdi
    jns .unsigned
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
.unsigned:
    call print_uint
    ret


; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    call string_length
    mov rcx, rax
    push rdi
    mov rdi, rsi
    call string_length
    pop rdi
    cmp rcx, rax
    js .not
    cld
    repe cmpsb
    jecxz .yes
.not:
     mov rax, 0x00
     ret
.yes:
     mov rax, 0x01
     ret


; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    xor rax, rax
    xor rdi, rdi
    mov rdx, 1
    push 0
    mov rsi, rsp
    syscall
    pop rax
    ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
        mov r9, rdi
        mov r10, rsi
        push r9
        xor r8, r8
        mov r8, 0
.loop1:
        push r9
        push r10
        push r8
        call read_char
        pop r8
        pop r10
        pop r9
        test rax, rax
        jz .end
        cmp rax, ' '
        jle .loop1
.loop2:
        cmp  r8, r10
        jae .not
        mov [r9], al
        inc r9
        inc r8
        push r9
        push r10
        push r8
        call read_char
        pop r8
        pop r10
        pop r9
        cmp rax, ' '
        jg .loop2
.end:
        mov byte[r9], 0x00
        pop r9
        mov rax, r9
        mov rdx, r8
        ret
.not:
        pop r9
        mov byte[r9], 0x00
        mov rax, 0
        ret

 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
        xor rax, rax
        xor r9, r9
        xor r8, r8
        mov rcx, 10
        push r13
        mov r13, 0
        mov r11, 0

.loop2:
        cmp byte[rdi+r9], 48
        jb .no
        cmp byte[rdi+r9], 57
        ja .no
        xor rdx, rdx
        mul rcx
        mov r8b, byte[rdi+r9]
        sub r8b, '0'
        add rax, r8
        inc r13
        inc r9
        jmp .loop2

.no:
        cmp r13, 0
        jne .end
        xor rax, rax
        xor rdx, rdx
        ret
.end:
        cmp r11, 0
        jne .neg
        mov rdx, r13
        pop r13
        ret
.neg:
        neg rax
        mov rdx, r13
        pop r13
        ret





; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    	cmp byte[rdi], '-'
	jne parse_uint
	inc rdi
	call parse_uint
	inc rdx
	neg rax
	ret 



; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    	xor rax, rax
.loop:
	xor rcx, rcx
	mov cl, byte[rdi]
	mov byte[rsi], cl
	cmp rsi, rdx                           
	jz .exit
	inc rdi
	inc rsi
	test rcx, rcx
	jnz .loop
.exit:
	xor rax, rax
	ret

