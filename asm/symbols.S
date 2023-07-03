
.extern time_me
.extern woah
.extern _Z1tv
.extern main
    .data
    .globl __patch_eligible
__patch_eligible:
	.ascii "time_me\0"
	.quad time_me
	.ascii "woah\0"
	.quad woah
	.ascii "_Z1tv\0"
	.quad _Z1tv
	.ascii "main\0"
	.quad main
    .byte 0xff
