.globl setjmp
setjmp:
	movl	4(%esp),%ecx
	movl	0(%esp),%edx
	movl	%edx,0(%ecx)
	movl	%ebx,4(%ecx)
	movl	%esp,8(%ecx)
	movl	%ebp,12(%ecx)
	movl	%esi,16(%ecx)
	movl	%edi,20(%ecx)
	movl	%eax,24(%ecx)
	fnstcw	28(%ecx)
	xorl	%eax,%eax
    ret

.globl longjmp
longjmp:
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	movl	0(%edx),%ecx
	movl	4(%edx),%ebx
	movl	8(%edx),%esp
	movl	12(%edx),%ebp
	movl	16(%edx),%esi
	movl	20(%edx),%edi
	fninit
	fldcw	28(%edx)
	testl	%eax,%eax
	jnz	nz
	incl	%eax
nz:	movl	%ecx,0(%esp)
    ret
