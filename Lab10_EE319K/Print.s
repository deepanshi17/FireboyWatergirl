; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
		PUSH 	{R4, LR}
		CMP 	R0, #10
		BLO		done
		MOV		R2, #10
		UDIV	R3, R0, R2
		MUL		R1, R3, R2
		SUB		R1, R0, R1 		;R1 = N%10
		PUSH 	{R1, R5}
		MOVS	R0, R3			;N = N/10
		BL		LCD_OutDec		;LCD_OutDec(N/10)
		POP		{R0, R5}
done	ADD		R0, R0, #'0'
		BL		ST7735_OutChar
		POP		{R4, LR}

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix

;// variable can be resolution (milli) and adding decimal (0x2E) 
;// LCD_OutChar(0x30+n/1000)
;;//thousand’s digit
;;n = n%1000
;;LCD_OutChar(0x2E)
;;//decimal point
;;LCD_OutChar(0x30+n/100)
;;//hundred’s digit
;;n = n%100
;;LCD_OutChar(0x30+n/10)
;;//ten’s digit
;;LCD_OutChar (0x30+n%10)
;;//one’s digit

; binding phase 
n 		EQU 0 
dec		EQU 4 
	
; allocation phase 
		PUSH {R4, LR} 
		SUB SP, #8  		; Allocates three 32 bit variables in stack 
		MOV R11, SP 		; will use R11 as frame pointer 
; access phase
	
		
		STR R0,[R11, #n] 	; R0 input is now in n 
		MOV R0, #0x2E 			 		
		STR R0, [R11, #dec] ; dec now has '.' 
		
		LDR R0, [R11, #n] 
		LDR R1, =9999 	
		CMP R0, R1
		BHI asterisk 
		
		LDR R0, [R11, #n] 	; R1 now has n input 
		MOV R1, #1000 
		UDIV R0, R0, R1  	; R0 has n/1000 
		ADD R0, R0, #0x30 	; R0 added to ascii 0 
		BL ST7735_OutChar
		
		; n%1000 
		LDR R0, [R11, #n] 
		MOV R1, #1000 		; 
		UDIV R3, R0, R1		; R3 = R0/1000 
		MUL R2, R3, R1		; R2 = R3*1000 
		SUB R0, R0, R2		; 
		STR R0, [R11, #n] 	; 
		
		LDR R0, [R11, #dec] ; outputs a decimal  
		BL ST7735_OutChar 	 

		LDR R0, [R11, #n] 	; 0x30 + n/100 
		MOV R1, #100 
		UDIV R0, R0, R1 
		ADD R0, R0, #0x30 
		BL ST7735_OutChar 	
		
		; n%100 
		LDR R0, [R11, #n] 
		MOV R1, #100		; 
		UDIV R3, R0, R1		; 
		MUL R2, R3, R1		;  
		SUB R0, R0, R2		; 
		STR R0, [R11, #n] 	; 
		
		LDR R0, [R11, #n] 
		MOV R1, #10 
		UDIV R0, R0, R1		; n/10 
		ADD R0, R0, #0x30
		BL ST7735_OutChar 	 
		
		;n%10 
		LDR R0, [R11, #n] 
		MOV R1, #10		; 
		UDIV R3, R0, R1		; 
		MUL R2, R3, R1		; 
		SUB R0, R0, R2		; 
		STR R0, [R11, #n] 	; 
		
		LDR R0, [R11, #n] 
		ADD R0, R0, #0x30 
		BL ST7735_OutChar  
		
		B Done
		
asterisk 
		MOV R0, #0x2A
		BL ST7735_OutChar
		
		LDR R0, [R11, #dec] ; outputs a decimal  
		BL ST7735_OutChar 
		
		MOV R10, #3 
		
astloop	MOV R0, #0x2A
		BL ST7735_OutChar
		SUBS R10, R10, #1 
		BNE astloop
		
;deallocation phase 
Done	ADD SP, #8 
		POP {R4, LR} 
		BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
