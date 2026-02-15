	.file	"iota_assembly_test.cpp"
	.text
	.p2align 4
	.globl	_Z11inline_iotaii
	.type	_Z11inline_iotaii, @function
_Z11inline_iotaii:
.LFB1599:
	.cfi_startproc
	endbr64
	subq	$72, %rsp
	.cfi_def_cfa_offset 80
	movdqa	.LC0(%rip), %xmm0
	movq	%fs:40, %rcx
	movq	%rcx, 56(%rsp)
	movl	(%rsp), %ecx
	movq	$0, 8(%rdi)
	movb	$0, 34(%rdi)
	movl	%esi, 36(%rdi)
	movl	%edx, 40(%rdi)
	movups	%xmm0, 16(%rdi)
	movl	%ecx, (%rdi)
	movzbl	32(%rsp), %ecx
	movb	%cl, 32(%rdi)
	movzbl	33(%rsp), %ecx
	movb	%cl, 33(%rdi)
	movq	56(%rsp), %rdx
	subq	%fs:40, %rdx
	jne	.L5
	movq	%rdi, %rax
	addq	$72, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L5:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1599:
	.size	_Z11inline_iotaii, .-_Z11inline_iotaii
	.p2align 4
	.globl	_Z15sum_inline_iotaii
	.type	_Z15sum_inline_iotaii, @function
_Z15sum_inline_iotaii:
.LFB1637:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	addq	$-128, %rsp
	.cfi_def_cfa_offset 144
	movdqa	.LC0(%rip), %xmm0
	movq	%fs:40, %rax
	movq	%rax, 120(%rsp)
	movl	64(%rsp), %eax
	movb	$0, 50(%rsp)
	movq	$0, 24(%rsp)
	movl	%edi, 52(%rsp)
	movl	%esi, 56(%rsp)
	movaps	%xmm0, 32(%rsp)
	movl	%eax, 16(%rsp)
	movzbl	96(%rsp), %eax
	movb	%al, 48(%rsp)
	movzbl	97(%rsp), %eax
	movb	%al, 49(%rsp)
	cmpl	%esi, %edi
	jge	.L7
	movl	%edi, 16(%rsp)
	xorl	%ebx, %ebx
	movq	$1, 32(%rsp)
.L8:
	addl	16(%rsp), %ebx
	cmpq	$1, 32(%rsp)
	movl	52(%rsp), %eax
	je	.L22
.L11:
	cmpl	56(%rsp), %eax
	jl	.L13
	movq	24(%rsp), %rax
	movb	$1, 50(%rsp)
	testq	%rax, %rax
	jne	.L14
.L9:
	cmpq	$0, 24(%rsp)
	je	.L6
	leaq	24(%rsp), %rdi
	call	_ZNSt15__exception_ptr13exception_ptr10_M_releaseEv@PLT
.L6:
	movq	120(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L23
	subq	$-128, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L13:
	.cfi_restore_state
	cmpb	$0, 50(%rsp)
	movl	%eax, 16(%rsp)
	movq	$1, 32(%rsp)
	je	.L8
	movq	24(%rsp), %rax
	testq	%rax, %rax
	je	.L9
.L14:
	leaq	8(%rsp), %rbx
	movq	%rax, 8(%rsp)
	movq	%rbx, %rdi
	call	_ZNSt15__exception_ptr13exception_ptr9_M_addrefEv@PLT
	movq	%rbx, %rdi
	call	_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE@PLT
	.p2align 4,,10
	.p2align 3
.L7:
	movb	$1, 50(%rsp)
	xorl	%ebx, %ebx
	jmp	.L9
	.p2align 4,,10
	.p2align 3
.L22:
	addl	$1, %eax
	movl	%eax, 52(%rsp)
	jmp	.L11
.L23:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1637:
	.size	_Z15sum_inline_iotaii, .-_Z15sum_inline_iotaii
	.p2align 4
	.globl	_Z17sum_callback_iotaii
	.type	_Z17sum_callback_iotaii, @function
_Z17sum_callback_iotaii:
.LFB1638:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi
	jge	.L25
	.p2align 4,,10
	.p2align 3
.L26:
	addl	%edi, %eax
	addl	$1, %edi
	cmpl	%edi, %esi
	jne	.L26
.L25:
	ret
	.cfi_endproc
.LFE1638:
	.size	_Z17sum_callback_iotaii, .-_Z17sum_callback_iotaii
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC0:
	.quad	0
	.quad	-1
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
