[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api006.nas"]

		GLOBAL	_api_wincallback

[SECTION .text]

_api_wincallback:	; void api_wincallback(int win, char * msgs);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,21
		MOV		EBX,[ESP+20]	; win
		MOV		ESI,[ESP+24]	; msgs
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
