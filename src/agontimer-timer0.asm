;
; Title:	AGON Interrupt handler for timer0 in userspace
; Author:	Jeroen Venema
; Created:	22/01/2023
; Last Updated:	22/01/2023

			INCLUDE "ez80f92.inc"

			.ASSUME	ADL = 1
			SEGMENT CODE
			
			XDEF	_timer1_handler
			XDEF	_timer1

; AGON Timer 0 Interrupt Handler
;
_timer1_handler:	
			DI
			PUSH	AF
			IN0		A,(TMR1_CTL)		; Clear the timer interrupt
			PUSH	BC
			LD		BC, (_timer1)		; Increment the delay timer
			INC		BC
			LD		(_timer1), BC
			POP		BC
			POP		AF
			EI
			RETI.L
	
			SEGMENT DATA
			
_timer1			DS	3