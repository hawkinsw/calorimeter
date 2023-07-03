    .extern trampoline_out 
    .extern trampoline_in 

    .globl exit_target
    .type exit_target, @function;
    .align 1<<5;
    exit_target:
    .cfi_startproc;

    /*
     * Now get ready to call recall_prior_ra
     */
    subq    $64,%rsp
    .cfi_adjust_cfa_offset 64
    movq    %rax,(%rsp)
    .cfi_rel_offset rax, 0
    movq    %rcx,8(%rsp)
    .cfi_rel_offset rcx, 8
    movq    %rdx,16(%rsp)
    .cfi_rel_offset rdx, 16
    movq    %rsi,24(%rsp)
    .cfi_rel_offset rsi, 24
    movq    %rdi,32(%rsp)
    .cfi_rel_offset rdi, 32
    movq    %r8,40(%rsp)
    .cfi_rel_offset r8, 40
    movq    %r9,48(%rsp)
    .cfi_rel_offset r9, 48


    call trampoline_in
    movq %rax, %r15

    /*
     * Now restore ...
     */
    movq    48(%rsp),%r9
    .cfi_restore r9
    movq    40(%rsp),%r8
    .cfi_restore r8
    movq    32(%rsp),%rdi
    .cfi_restore rdi
    movq    24(%rsp),%rsi
    .cfi_restore rsi
    movq    16(%rsp),%rdx
    .cfi_restore rdx
    movq    8(%rsp),%rcx
    .cfi_restore rcx
    movq    (%rsp),%rax
    .cfi_restore rax
    addq    $64,%rsp
    .cfi_adjust_cfa_offset -64
    jmpq *%r15 
    .cfi_endproc;
    .size exit_target,.-exit_target

    .globl entry_target;
    .type entry_target, @function;
    .align 1<<5;
    entry_target:
    .cfi_startproc;

    /*
     * Save the RA we are overwriting.
     */
    movq 8(%rsp), %r14
    /*
     * put exit_target in its place.
     */
    lea exit_target, %r15
    movq %r15, 8(%rsp)

    movq 0(%rsp), %r15

    /*
     * Now get ready to call remember_prior_ra
     */
    subq    $64,%rsp
    .cfi_adjust_cfa_offset 64
    movq    %rax,(%rsp)
    .cfi_rel_offset rax, 0
    movq    %rcx,8(%rsp)
    .cfi_rel_offset rcx, 8
    movq    %rdx,16(%rsp)
    .cfi_rel_offset rdx, 16
    movq    %rsi,24(%rsp)
    .cfi_rel_offset rsi, 24
    movq    %rdi,32(%rsp)
    .cfi_rel_offset rdi, 32
    movq    %r8,40(%rsp)
    .cfi_rel_offset r8, 40
    movq    %r9,48(%rsp)
    .cfi_rel_offset r9, 48


    movq %r15, %rsi
    movq %r14, %rdi
    call trampoline_out 

    /*
     * Now restore ...
     */
    movq    48(%rsp),%r9
    .cfi_restore r9
    movq    40(%rsp),%r8
    .cfi_restore r8
    movq    32(%rsp),%rdi
    .cfi_restore rdi
    movq    24(%rsp),%rsi
    .cfi_restore rsi
    movq    16(%rsp),%rdx
    .cfi_restore rdx
    movq    8(%rsp),%rcx
    .cfi_restore rcx
    movq    (%rsp),%rax
    .cfi_restore rax
    addq    $64,%rsp
    .cfi_adjust_cfa_offset -64
    ret
    .cfi_endproc;
    .size entry_target,.-entry_target