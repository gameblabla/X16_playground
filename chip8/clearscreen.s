.segment "CODE"
.export _clear_screen

; By MooingLemur

_clear_screen:
	; Set CTRL to 0
	stz $9F25

	; Set primary address low byte to 0x0000
	stz $9F20

	; Set primary address high byte to 0x0000
	stz $9F21

	; Set auto-increment to 1
	lda #%00010000
	sta $9F22

	; Set the data to 2
	lda #2

	; Set y to 160
	ldy #160

	; Set x to 240 (loop 76800 times)
	ldx #240

loop:
	; Write 0 to the calculated address
	stz $9F23

	; Decrement x
	dex

	; Loop until x = 0
	bne loop

	; Reset x to 240
	ldx #240

	; Decrement y
	dey

	; Loop until y = 0
	bne loop

	; Reset y to 160
	ldy #160

	; Decrement y
	dec

	; Loop until y = 255
	bne loop

	rts
	
