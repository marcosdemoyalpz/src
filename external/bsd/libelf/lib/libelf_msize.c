/*	$NetBSD: libelf_msize.m4,v 1.6 2010/02/22 10:48:33 darran Exp $	*/

/*-
 * Copyright (c) 2006 Joseph Koshy
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS `AS IS' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if HAVE_NBTOOL_CONFIG_H
# include "nbtool_config.h"
#endif

#include <sys/cdefs.h>
/* __FBSDID("$FreeBSD: src/lib/libelf/libelf_msize.m4,v 1.2.10.1.2.1 2009/10/25 01:10:29 kensmith Exp $"); */

#include <sys/types.h>

#include <assert.h>
#include <libelf.h>
#include <string.h>

#include "_libelf.h"

/* WARNING: GENERATED FROM /usr/src/external/bsd/libelf/dist/libelf_msize.m4. */

struct msize {
	size_t	msz32;
	size_t	msz64;
};



static struct msize msize[ELF_T_NUM] = {
#if	1
    [ELF_T_ADDR] = { .msz32 = sizeof(Elf32_Addr), .msz64 = sizeof(Elf64_Addr) },
#endif
#if	1
    [ELF_T_BYTE] = { .msz32 = 1, .msz64 = 1 },
#endif
#if	__LIBELF_HAVE_ELF_CAP
    [ELF_T_CAP] = { .msz32 = sizeof(Elf32_Cap), .msz64 = sizeof(Elf64_Cap) },
#endif
#if	1
    [ELF_T_DYN] = { .msz32 = sizeof(Elf32_Dyn), .msz64 = sizeof(Elf64_Dyn) },
#endif
#if	1
    [ELF_T_EHDR] = { .msz32 = sizeof(Elf32_Ehdr), .msz64 = sizeof(Elf64_Ehdr) },
#endif
#if	1
    [ELF_T_HALF] = { .msz32 = sizeof(Elf32_Half), .msz64 = sizeof(Elf64_Half) },
#endif
#if	__LIBELF_HAVE_ELF_MOVE
    [ELF_T_LWORD] = { .msz32 = sizeof(Elf32_Lword), .msz64 = sizeof(Elf64_Lword) },
#endif
#if	__LIBELF_HAVE_ELF_MOVE
    [ELF_T_MOVE] = { .msz32 = sizeof(Elf32_Move), .msz64 = sizeof(Elf64_Move) },
#endif
#if	__LIBELF_HAVE_ELF_MOVE
    [ELF_T_MOVEP] = { .msz32 = 0, .msz64 = 0 },
#endif
#if	1
    [ELF_T_NOTE] = { .msz32 = 1, .msz64 = 1 },
#endif
#if	1
    [ELF_T_OFF] = { .msz32 = sizeof(Elf32_Off), .msz64 = sizeof(Elf64_Off) },
#endif
#if	1
    [ELF_T_PHDR] = { .msz32 = sizeof(Elf32_Phdr), .msz64 = sizeof(Elf64_Phdr) },
#endif
#if	1
    [ELF_T_REL] = { .msz32 = sizeof(Elf32_Rel), .msz64 = sizeof(Elf64_Rel) },
#endif
#if	1
    [ELF_T_RELA] = { .msz32 = sizeof(Elf32_Rela), .msz64 = sizeof(Elf64_Rela) },
#endif
#if	1
    [ELF_T_SHDR] = { .msz32 = sizeof(Elf32_Shdr), .msz64 = sizeof(Elf64_Shdr) },
#endif
#if	1
    [ELF_T_SWORD] = { .msz32 = sizeof(Elf32_Sword), .msz64 = sizeof(Elf64_Sword) },
#endif
#if	__LIBELF_HAVE_ELF_VERS
    [ELF_T_SXWORD] = { .msz32 = 0, .msz64 = sizeof(Elf64_Sxword) },
#endif
#if	__LIBELF_HAVE_ELF_SYMINFO
    [ELF_T_SYMINFO] = { .msz32 = sizeof(Elf32_Syminfo), .msz64 = sizeof(Elf64_Syminfo) },
#endif
#if	1
    [ELF_T_SYM] = { .msz32 = sizeof(Elf32_Sym), .msz64 = sizeof(Elf64_Sym) },
#endif
#if	__LIBELF_HAVE_ELF_VERS
    [ELF_T_VDEF] = { .msz32 = sizeof(Elf32_Verdef), .msz64 = sizeof(Elf64_Verdef) },
#endif
#if	__LIBELF_HAVE_ELF_VERS
    [ELF_T_VNEED] = { .msz32 = sizeof(Elf32_Verneed), .msz64 = sizeof(Elf64_Verneed) },
#endif
#if	1
    [ELF_T_WORD] = { .msz32 = sizeof(Elf32_Word), .msz64 = sizeof(Elf64_Word) },
#endif
#if	__LIBELF_HAVE_ELF_VERS
    [ELF_T_XWORD] = { .msz32 = 0, .msz64 = sizeof(Elf64_Xword) },
#endif

};

size_t
_libelf_msize(Elf_Type t, int elfclass, unsigned int version)
{
	size_t sz;

	assert(elfclass == ELFCLASS32 || elfclass == ELFCLASS64);
	assert((signed) t >= ELF_T_FIRST && t <= ELF_T_LAST);

	if (version != EV_CURRENT) {
		LIBELF_SET_ERROR(VERSION, 0);
		return (0);
	}

	sz = (elfclass == ELFCLASS32) ? msize[t].msz32 : msize[t].msz64;

	return (sz);
}
