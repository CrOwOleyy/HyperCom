#!/usr/bin/env python3
"""Regenerates common/crypto/recovery_wordlist.cpp from the BIP-39 list.

Run from the repository root: python scripts/gen_recovery_wordlist.py
"""
words = open('common/crypto/data/bip39_english.txt').read().split()
assert len(words) == 2048
out = ['// GENERATED from common/crypto/data/bip39_english.txt by',
       '// scripts/gen_recovery_wordlist.py -- do not edit by hand.',
       '// The BIP-39 English list (2048 words); recovery_phrase_test checks its',
       '// SHA-256 so a stray edit cannot silently change every phrase.',
       '#include "common/crypto/recovery_wordlist.hpp"', '',
       'namespace hypercom::crypto {', '',
       'std::array<std::string_view, RECOVERY_WORDLIST_SIZE> const',
       '    RECOVERY_WORDLIST = {{']
line = '   '
for w in words:
    tok = ' "%s",' % w
    if len(line) + len(tok) > 80:
        out.append(line)
        line = '   '
    line += tok
out.append(line)
out += ['}};', '', '} // namespace hypercom::crypto', '']
open('common/crypto/recovery_wordlist.cpp', 'w', newline='\n').write('\n'.join(out))
