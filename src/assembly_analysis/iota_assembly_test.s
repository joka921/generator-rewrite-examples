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
	.section	.text.unlikely,"ax",@progbits
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4
	.globl	_Z15sum_inline_iotaii
	.type	_Z15sum_inline_iotaii, @function
_Z15sum_inline_iotaii:
.LFB1639:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1639
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	leaq	_ZN14InlineCoroImplIZ11inline_iotaiiE9CoroFrameN6detail24inline_generator_promiseIiEELb1EE6resumeEPv(%rip), %rdx
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	movq	%rdx, %xmm0
	subq	$168, %rsp
	.cfi_def_cfa_offset 192
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
	je	.L46
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
	je	.L32
	leaq	16(%rsp), %rdi
	call	*%rax
.L32:
	cmpq	$0, 40(%rsp)
	je	.L15
	leaq	40(%rsp), %rdi
	call	_ZNSt15__exception_ptr13exception_ptr10_M_releaseEv@PLT
.L15:
	movq	152(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L45
	addq	$168, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
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
	leaq	8(%rsp), %rbp
	movq	%rax, 8(%rsp)
	movq	%rbp, %rdi
	call	_ZNSt15__exception_ptr13exception_ptr9_M_addrefEv@PLT
	movq	152(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L45
	movq	%rbp, %rdi
.LEHB0:
	call	_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE@PLT
.LEHE0:
	.p2align 4,,10
	.p2align 3
.L18:
	movb	$1, 66(%rsp)
	xorl	%ebx, %ebx
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L46:
	addl	$1, %eax
	movl	%eax, 68(%rsp)
	jmp	.L22
.L45:
	call	__stack_chk_fail@PLT
.L37:
	endbr64
	movq	%rax, %rbx
	jmp	.L28
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA1639:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1639-.LLSDACSB1639
.LLSDACSB1639:
	.uleb128 .LEHB0-.LFB1639
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L37-.LFB1639
	.uleb128 0
.LLSDACSE1639:
	.text
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDAC1639
	.type	_Z15sum_inline_iotaii.cold, @function
_Z15sum_inline_iotaii.cold:
.LFSB1639:
.L28:
	.cfi_def_cfa_offset 192
	.cfi_offset 3, -24
	.cfi_offset 6, -16
	cmpq	$0, 8(%rsp)
	je	.L29
	movq	%rbp, %rdi
	call	_ZNSt15__exception_ptr13exception_ptr10_M_releaseEv@PLT
.L29:
	movq	24(%rsp), %rax
	testq	%rax, %rax
	je	.L31
	leaq	16(%rsp), %rdi
	call	*%rax
.L31:
	cmpq	$0, 40(%rsp)
	je	.L34
	leaq	40(%rsp), %rdi
	call	_ZNSt15__exception_ptr13exception_ptr10_M_releaseEv@PLT
.L34:
	movq	152(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L47
	movq	%rbx, %rdi
.LEHB1:
	call	_Unwind_Resume@PLT
.LEHE1:
.L47:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1639:
	.section	.gcc_except_table
.LLSDAC1639:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSEC1639-.LLSDACSBC1639
.LLSDACSBC1639:
	.uleb128 .LEHB1-.LCOLDB1
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSEC1639:
	.section	.text.unlikely
	.text
	.size	_Z15sum_inline_iotaii, .-_Z15sum_inline_iotaii
	.section	.text.unlikely
	.size	_Z15sum_inline_iotaii.cold, .-_Z15sum_inline_iotaii.cold
.LCOLDE1:
	.text
.LHOTE1:
	.p2align 4
	.globl	_Z17sum_callback_iotaii
	.type	_Z17sum_callback_iotaii, @function
_Z17sum_callback_iotaii:
.LFB1640:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	cmpl	%esi, %edi
	jge	.L49
	.p2align 4,,10
	.p2align 3
.L50:
	addl	%edi, %eax
	addl	$1, %edi
	cmpl	%edi, %esi
	jne	.L50
.L49:
	ret
	.cfi_endproc
.LFE1640:
	.size	_Z17sum_callback_iotaii, .-_Z17sum_callback_iotaii
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC0:
	.quad	0
	.quad	-1
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
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
