/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/*
 * Copyright (c) 2010 Ali Polatel <alip@exherbo.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>

#include <asm/ptrace_offsets.h>
#include <asm/rse.h>

#include <pinktrace/internal.h>
#include <pinktrace/pink.h>

#define ORIG_ACCUM	(PT_R15)

static bool
pink_util_peek_ia64(pid_t pid, int narg, long *res)
{
	unsigned long *out0, cfm, sof, sol;
	long rbs_end;

	if (pink_unlikely(!pink_util_peek(pid, PT_AR_BSP, &rbs_end)))
		return false;
	if (pink_unlikely(!pink_util_peek(pid, PT_CFM, (long *)&cfm)))
		return false;

	sof = (cfm >> 0) & 0x7f;
	sol = (cfm >> 7) & 0x7f;
	out0 = ia64_rse_skip_regs((unsigned long *)rbs_end, -sof + sol);

	return pink_util_moven(pid, (unsigned long)ia64_rse_skip_regs(out0, narg), (char *)res, sizeof(long));
}

pink_bitness_t
pink_bitness_get(pink_unused pid_t pid)
{
	return PINK_BITNESS_64;
}

bool
pink_util_get_syscall(pid_t pid, pink_unused pink_bitness_t bitness, long *res)
{
	return pink_util_peek(pid, ORIG_ACCUM, res);
}

bool
pink_util_set_syscall(pid_t pid, pink_unused pink_bitness_t bitness, long scno)
{
	return pink_util_poke(pid, ORIG_ACCUM, scno);
}

bool
pink_util_get_return(pid_t pid, long *res)
{
	long r8, r10;

	assert(res != NULL);

	if (pink_unlikely(!pink_util_peek(pid, PT_R8, &r8)
				|| !pink_util_peek(pid, PT_R10, &r10)))
		return false;

	*res = (r10 != 0) ? -r8 : r8;
	return true;
}

bool
pink_util_set_return(pid_t pid, long ret)
{
	long r8, r10;

	r8 = (ret < 0) ? -ret : ret;
	r10 = (ret < 0) ? -1 : 0;

	return pink_util_poke(pid, PT_R8, r8) && pink_util_poke(pid, PT_R10, r10);
}

bool
pink_util_get_arg(pid_t pid, pink_unused pink_bitness_t bitness, unsigned ind, long *res)
{
	assert(ind < PINK_MAX_INDEX);
	assert(res != NULL);

	return pink_util_peek_ia64(pid, ind, res);
}

bool
pink_decode_simple(pid_t pid, pink_bitness_t bitness, unsigned ind, void *dest, size_t len)
{
	long addr;

	assert(ind < PINK_MAX_INDEX);
	assert(dest != NULL);

	return pink_util_get_arg(pid, bitness, ind, &addr) && pink_util_moven(pid, addr, dest, len);
}

bool
pink_decode_string(pid_t pid, pink_bitness_t bitness, unsigned ind, char *dest, size_t len)
{
	long addr;

	assert(ind < PINK_MAX_INDEX);
	assert(dest != NULL);

	return pink_util_get_arg(pid, bitness, ind, &addr) && pink_util_movestr(pid, addr, dest, len);
}

char *
pink_decode_string_persistent(pid_t pid, pink_bitness_t bitness, unsigned ind)
{
	long addr;

	assert(ind < PINK_MAX_INDEX);

	if (pink_unlikely(!pink_util_get_arg(pid, bitness, ind, &addr)))
		return NULL;

	return pink_util_movestr_persistent(pid, addr);
}

bool
pink_encode_simple(pid_t pid, pink_bitness_t bitness, unsigned ind, const void *src, size_t len)
{
	long addr;

	assert(ind < PINK_MAX_INDEX);

	return pink_util_get_arg(pid, bitness, ind, &addr) && pink_util_putn(pid, addr, src, len);
}

bool
pink_encode_simple_safe(pid_t pid, pink_bitness_t bitness, unsigned ind, const void *src, size_t len)
{
	long addr;

	assert(ind < PINK_MAX_INDEX);

	return pink_util_get_arg(pid, bitness, ind, &addr) && pink_util_putn_safe(pid, addr, src, len);
}

bool
pink_decode_socket_call(pid_t pid, pink_bitness_t bitness, long *subcall_r)
{
	long addr;

	assert(subcall_r != NULL);

	/* No decoding needed */
	return pink_util_get_syscall(pid, bitness, subcall_r);
}

bool
pink_decode_socket_fd(pid_t pid, pink_bitness_t bitness, unsigned ind, long *fd)
{
	assert(fd != NULL);

	/* No decoding needed */
	return pink_util_get_arg(pid, bitness, ind, fd);
}

bool
pink_decode_socket_address(pid_t pid, pink_bitness_t bitness, unsigned ind, long *fd_r, pink_socket_address_t *addr_r)
{
	long addr, addrlen;

	assert(ind < PINK_MAX_INDEX);
	assert(addr_r != NULL);

	/* No decoding needed */
	if (pink_unlikely(fd_r && !pink_util_get_arg(pid, bitness, 0, fd_r)))
		return false;
	if (pink_unlikely(!pink_util_get_arg(pid, bitness, ind, &addr)))
		return false;
	if (pink_unlikely(!pink_util_get_arg(pid, bitness, ind + 1, &addrlen)))
		return false;

	return pink_internal_decode_socket_address(pid, addr, addrlen, addr_r);
}
