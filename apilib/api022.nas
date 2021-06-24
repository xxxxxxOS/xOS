[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api006.nas"]

		GLOBAL	_api_cursorvis

[SECTION .text]

_api_cursorvis:	; void api_cursorvis(char vis);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,22
		MOV		EBX,[ESP+20]	; vis
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
