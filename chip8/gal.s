;
; 6502 LFSR PRNG - 16-bit
; Brad Smith, 2019
; http://rainwarrior.ca
;

; A 16-bit Galois LFSR

; Possible feedback values that generate a full 65535 step sequence:
; $2D = %00101101
; $39 = %00111001
; $3F = %00111111
; $53 = %01010011
; $BD = %10111101
; $D7 = %11010111

; $39 is chosen for its compact bit pattern

.zeropage
seed: .res 4 ; seed can be 2-4 bytes
_seed = seed ; C alias
.exportzp seed
.exportzp _seed

.code

; overlapped version, computes all 8 iterations in an overlapping fashion
; 69 cycles
; 35 bytes

galois16o:
	lda seed+1
	tay ; store copy of high byte
	; compute seed+1 ($39>>1 = %11100)
	lsr ; shift to consume zeroes on left...
	lsr
	lsr
	sta seed+1 ; now recreate the remaining bits in reverse order... %111
	lsr
	eor seed+1
	lsr
	eor seed+1
	eor seed+0 ; recombine with original low byte
	sta seed+1
	; compute seed+0 ($39 = %111001)
	tya ; original high byte
	sta seed+0
	asl
	eor seed+0
	asl
	eor seed+0
	asl
	asl
	asl
	eor seed+0
	sta seed+0
	rts

; C wrappers

.export _galois16o


_galois16o:
	jsr galois16o
	ldx #0
	rts
