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

#ifndef PINKTRACE_GUARD_GCC_H
#define PINKTRACE_GUARD_GCC_H 1

/**
 * \file
 * Pink's GCC macros
 **/

#if !defined(SPARSE) && defined(__GNUC__) && __GNUC__ >= 3

#define pink_noreturn __attribute__((noreturn))
#define pink_pure __attribute__((pure))
#define pink_unused __attribute__((unused))
#define pink_likely(x) __builtin_expect(!!(x), 1)
#define pink_unlikely(x) __builtin_expect(!!(x), 0)

#else
/**
 * GCC noreturn attribute
 **/
#define pink_noreturn
/**
 * GCC pure attribute
 **/
#define pink_pure
/**
 * GCC unused attribute
 **/
#define pink_unused
/**
 * GCC builtin_expect macro
 **/
#define pink_likely(x) (x)
/**
 * GCC builtin_expect macro
 **/
#define pink_unlikely(x) (x)

#endif /* !defined(SPARSE) ... */

#endif /* !PINKTRACE_GUARD_GCC_H */
