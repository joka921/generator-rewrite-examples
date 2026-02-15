	.file	"iota_assembly_test.cpp"
	.text
	.p2align 4
	.type	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv, @function
_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv:
.LFB1906:
	.cfi_startproc
	endbr64
	movq	$0, 8(%rdi)
	ret
	.cfi_endproc
.LFE1906:
	.size	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv, .-_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv
	.p2align 4
	.type	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv, @function
_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv:
.LFB2033:
	.cfi_startproc
	endbr64
	cmpq	$1, 32(%rdi)
	movl	52(%rdi), %eax
	je	.L10
.L7:
	cmpl	%eax, 56(%rdi)
	jg	.L9
	movb	$1, 50(%rdi)
	ret
	.p2align 4,,10
	.p2align 3
.L9:
	movl	%eax, 16(%rdi)
	movq	$1, 32(%rdi)
	ret
	.p2align 4,,10
	.p2align 3
.L10:
	addl	$1, %eax
	movl	%eax, 52(%rdi)
	jmp	.L7
	.cfi_endproc
.LFE2033:
	.size	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv, .-_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv
	.p2align 4
	.globl	_Z11inline_iotaii
	.type	_Z11inline_iotaii, @function
_Z11inline_iotaii:
.LFB1604:
	.cfi_startproc
	endbr64
	subq	$88, %rsp
	.cfi_def_cfa_offset 96
	movq	%rdi, %rax
	leaq	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv(%rip), %rdi
	movq	%fs:40, %rcx
	movq	%rcx, 72(%rsp)
	leaq	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv(%rip), %rcx
	movq	%rdi, %xmm0
	movq	$0, 24(%rax)
	movb	$0, 50(%rax)
	movl	%esi, 52(%rax)
	movl	%edx, 56(%rax)
	movq	%rcx, %xmm1
	movl	16(%rsp), %ecx
	punpcklqdq	%xmm1, %xmm0
	movl	%ecx, 16(%rax)
	movzbl	48(%rsp), %ecx
	movaps	%xmm0, (%rsp)
	movb	%cl, 48(%rax)
	movzbl	49(%rsp), %ecx
	movups	%xmm0, (%rax)
	movdqa	.LC0(%rip), %xmm0
	movb	%cl, 49(%rax)
	movups	%xmm0, 32(%rax)
	movq	72(%rsp), %rdx
	subq	%fs:40, %rdx
	jne	.L14
	addq	$88, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L14:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1604:
	.size	_Z11inline_iotaii, .-_Z11inline_iotaii
	.p2align 4
	.globl	_Z15sum_inline_iotaii
	.type	_Z15sum_inline_iotaii, @function
_Z15sum_inline_iotaii:
.LFB1639:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	leaq	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv(%rip), %rdx
	movq	%rdx, %xmm0
	subq	$160, %rsp
	.cfi_def_cfa_offset 176
	movq	%fs:40, %rax
	movq	%rax, 152(%rsp)
	leaq	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE7destroyEPv(%rip), %rax
	movb	$0, 66(%rsp)
	movq	$0, 40(%rsp)
	movl	%edi, 68(%rsp)
	movl	%esi, 72(%rsp)
	movq	%rax, %xmm1
	movl	96(%rsp), %eax
	punpcklqdq	%xmm1, %xmm0
	movl	%eax, 32(%rsp)
	movzbl	128(%rsp), %eax
	movaps	%xmm0, 16(%rsp)
	cmpq	$0, 24(%rsp)
	movb	%al, 64(%rsp)
	movzbl	129(%rsp), %eax
	movaps	%xmm0, 80(%rsp)
	movdqa	.LC0(%rip), %xmm0
	movb	%al, 65(%rsp)
	movaps	%xmm0, 48(%rsp)
	je	.L19
	cmpl	%esi, %edi
	jge	.L18
	movl	%edi, 32(%rsp)
	movq	$1, 48(%rsp)
.L19:
	xorl	%ebx, %ebx
.L17:
	addl	32(%rsp), %ebx
	cmpq	$1, 48(%rsp)
	movl	68(%rsp), %eax
	je	.L37
.L22:
	cmpl	72(%rsp), %eax
	jl	.L24
	movq	40(%rsp), %rax
	movb	$1, 66(%rsp)
	testq	%rax, %rax
	jne	.L25
.L20:
	movq	24(%rsp), %rax
	testq	%rax, %rax
	je	.L27
	leaq	16(%rsp), %rdi
	call	*%rax
.L27:
	cmpq	$0, 40(%rsp)
	je	.L15
	leaq	40(%rsp), %rdi
	call	_ZNSt15__exception_ptr13exception_ptr10_M_releaseEv@PLT
.L15:
	movq	152(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L38
	addq	$160, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L24:
	.cfi_restore_state
	cmpb	$0, 66(%rsp)
	movl	%eax, 32(%rsp)
	movq	$1, 48(%rsp)
	je	.L17
	movq	40(%rsp), %rax
	testq	%rax, %rax
	je	.L20
.L25:
	leaq	8(%rsp), %rbx
	movq	%rax, 8(%rsp)
	movq	%rbx, %rdi
	call	_ZNSt15__exception_ptr13exception_ptr9_M_addrefEv@PLT
	movq	%rbx, %rdi
	call	_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE@PLT
	.p2align 4,,10
	.p2align 3
.L18:
	movb	$1, 66(%rsp)
	xorl	%ebx, %ebx
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L37:
	addl	$1, %eax
	movl	%eax, 68(%rsp)
	jmp	.L22
.L38:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1639:
	.size	_Z15sum_inline_iotaii, .-_Z15sum_inline_iotaii
	.p2align 4
	.globl	_Z17sum_callback_iotaii
	.type	_Z17sum_callback_iotaii, @function
_Z17sum_callback_iotaii:
.LFB1640:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi
	jge	.L40
	.p2align 4,,10
	.p2align 3
.L41:
	addl	%edi, %eax
	addl	$1, %edi
	cmpl	%edi, %esi
	jne	.L41
.L40:
	ret
	.cfi_endproc
.LFE1640:
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
