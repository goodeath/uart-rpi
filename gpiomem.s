@.include "fileio.s"
@.equ pagelen, 4096
@.equ setregoffset, 28
@.equ clrregoffset, 40
@.equ PROT_READ, 0x1
@.equ PROT_WRITE, 0x2
@.equ MAP_SHARED, 0x01
@.equ PROT_RDWR, PROT_READ|PROT_WRITE
@.equ GPIO_OFFSET, 0x200000
@.equ PERIPH, 0x20000000



@ Macro to map memory for GPIO Registers
.macro mapMem
    fopen devmem, FLAG @ open /dev/mem
    movs r4, r0 @ fd for memmap
    @ check for error and print error msg if necessary
    BPL 1f @ pos number file opened ok
    MOV R1, #1 @ stdout
    LDR R2, =memOpnsz @ Error msg
    LDR R2, [R2]
    fwrite R1, memOpnErr, R2 @ print the error
    B _end
@ Set up can call the mmap2 Linux service
1: 
    @ldr r5, =gpioaddr @ address we want / 4096
    @ldr r5, [r5] @ load the address
    mov r5, #0
    mov r0, #0 @ let linux choose a
    mov r1, #pagelen @ size of mem we want
    LDR r2, =PROT_RDWR
    mov r3, #MAP_SHARED @ mem share options
    mov r7, #222 @ mmap2 service num
    svc 0 @ call service
    movs r8, r0 @ keep the returned virt addr
    @ check for error and print error msg
    @ if necessary.
    BPL 2f @ pos number file opened ok
    MOV R1, #1 @ stdout
    LDR R2, =memMapsz @ Error msg
    LDR R2, [R2]
    fwrite R1, memMapErr, R2 @ print the error
    B _end
2:
.endm
@ Macro nanoSleep to sleep .1 second
@ Calls Linux nanosleep entry point which is function 162.
@ Pass a reference to a timespec in both r0 and r1
@ First is input time to sleep in seconds and nanoseconds.
@ Second is time left to sleep if interrupted (which we ignore)
.macro nanoSleep
 ldr r0, =timespecsec
 ldr r1, =timespecsec
 mov r7, #162
 svc 0
.endm
.macro GPIODirectionOut pin
 ldr r2, =\pin @ offset of select register
 ldr r2, [r2] @ load the value
 ldr r1, [r8, r2] @ address of register
 ldr r3, =\pin @ address of pin table
 add r3, #4 @ load amount to shift from table
 ldr r3, [r3] @ load value of shift amt
 mov r0, #0b111 @ mask to clear 3 bits
 lsl r0, r3 @ shift into position
 bic r1, r0 @ clear the three bits
 mov r0, #1 @ 1 bit to shift into pos
 lsl r0, r3 @ shift by amount from table
 orr r1, r0 @ set the bit
 str r1, [r8, r2] @ save it to reg to do work
.endm
.macro GPIOTurnOn pin, value
 mov r2, r8 @ address of gpio regs
 add r2, #setregoffset @ off to set reg
 mov r0, #1 @ 1 bit to shift into pos
 ldr r3, =\pin @ base of pin info table
 add r3, #8 @ add offset for shift amt
 ldr r3, [r3] @ load shift from table
 lsl r0, r3 @ do the shift
 str r0, [r2] @ write to the register
.endm
.macro GPIOTurnOff pin, value
 mov r2, r8 @ address of gpio regs
 add r2, #clrregoffset @ off set of clr reg
 mov r0, #1 @ 1 bit to shift into pos
 ldr r3, =\pin @ base of pin info table
 add r3, #8 @ add offset for shift amt
 ldr r3, [r3] @ load shift from table
 lsl r0, r3 @ do the shift
 str r0, [r2] @ write to the register
.endm
.data
@timespecsec: .word 0
@timespecnano: .word 100000000
@devmem: .asciz "/dev/gpiomem"
memOpnErr: .asciz "Failed to open /dev/mem\n"
memOpnsz: .word .-memOpnErr
memMapErr: .asciz "Failed to map memory\n"
memMapsz: .word .-memMapErr
 .align 4 @ realign after strings
@ mem address of gpio register / 4096
gpioaddr: .word PERIPH + GPIO_OFFSET

pin17: .word 4 @ offset to select register
 .word 21 @ bit offset in select register
 .word 17 @ bit offset in set & clr register
pin22: .word 8 @ offset to select register
 .word 6 @ bit offset in select register
 .word 22 @ bit offset in set & clr register
pin27: .word 8 @ offset to select register
 .word 21 @ bit offset in select register
 .word 27 @ bit offset in set & clr register
pin6: .word 0
 .word 15
 .word 6
