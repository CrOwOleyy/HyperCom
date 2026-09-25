# Claude Instructions for Hypercom

## 🎯 Mission Principale
Préparer **Hypercom** (réseau social chiffré E2E) pour l'**open source** en:
1. Auditant la sécurité (crypto, validation, secrets)
2. Préparant la compliance OSS (license, docs, CI/CD)
3. Améliorant la qualité du code (patterns, tests, warnings)

## 📋 Contexte Projet
- **Language**: C++20 avec CMake
- **Architecture**: Client (ImGui), Server (Linux), Common (crypto/protocol)
- **Security**: Noise Protocol, X25519, ChaCha20-Poly1305, libsodium
- **DB**: SQLite3
- **Dependances**: libsodium (ISC), sqlite3 (public domain), imgui (MIT)

## 🔍 Commandes Disponibles

Quand l'utilisateur dit:

| Commande | Action |
|----------|--------|
| **AUDIT** | Code review complet (sécurité + compliance) |
| **IDENTIFY** | Lister tous les problèmes trouvés + severity |
| **FIX** | Corriger les problèmes (priorité: critiques d'abord) |
| **DOCUMENT** | Créer/améliorer docs (README, ARCHITECTURE, etc.) |
| **TEST** | Ajouter/améliorer tests, valider fixes |
| **DEPLOY** | Préparer pour release (build, CI/CD, etc.) |

## ⚠️ Priority Levels

**CRITIQUE** (fix immédiatement):
- Failles crypto (Noise, key derivation, etc.)
- Input validation → buffer overflows
- Secrets en dur (keys, passwords)
- SQL injection

**HAUTE** (fix cette semaine):
- Race conditions (threading)
- DoS vectors (rate limiting)
- Memory leaks/ASAN warnings
- Code injection risks

**MOYENNE** (nice to have):
- Code duplication
- Missing tests
- Compiler warnings
- Documentation gaps

## 📂 Fichiers Clés

```
common/
  ├── crypto/      ← Noise, key derivation, DM encryption
  ├── protocol/    ← Message parsing, frame codec
  └── util/        ← Logger, hex codec, secure_memory

server/
  ├── db/          ← Repositories, SQL safety
  ├── handlers/    ← Message processing, session guard
  ├── net/         ← Network, rate limiting, event loop
  └── admin/       ← Admin commands

client/
  ├── net/         ← Client session, noise transport
  ├── keystore/    ← Identity storage
  ├── cli/         ← CLI commands
  └── ui/          ← ImGui rendering
```

## 🚨 Security Focus Areas

### Crypto
- ✅ Vérifier: Noise Protocol correct implementation
- ✅ Vérifier: No hardcoded keys/nonces
- ✅ Vérifier: Secure random generation
- ✅ Vérifier: Key derivation (HKDF) correct

### Validation
- ✅ Vérifier: byte_reader bounds checking
- ✅ Vérifier: frame_codec length validation
- ✅ Vérifier: message parsing → no overflows
- ✅ Vérifier: SQL prepared statements (no injection)

### Secrets
- ✅ Vérifier: ServerStaticKey not in logs
- ✅ Vérifier: Keystore permissions (0600)
- ✅ Vérifier: Git ignores .env, keys, etc.
- ✅ Vérifier: CI/CD masks secrets

## 📊 Deliverables Template

**After AUDIT:**
```
## Security Findings

### CRITIQUE (0)
- None ✅

### HAUTE (N)
- [Issue]: Description. Fix: [What to do]

### MOYENNE (N)
- ...

### Next Steps
1. FIX all CRITIQUE issues
2. Then: IDENTIFY all findings
3. Then: FIX HAUTE + MOYENNE
```

**After DOCUMENT:**
```
Created/Updated:
- ✅ README.md
- ✅ ARCHITECTURE.md
- ✅ CONTRIBUTING.md
- ✅ SECURITY.md
- ✅ CODE_OF_CONDUCT.md
```

## 🛠️ Development Rules

1. **No shortcuts**: Don't skip security checks for speed
2. **Backwards compatibility**: Try to keep API stable
3. **Test before merge**: All tests pass, 0 warnings
4. **Document changes**: Every code change needs docs update
5. **Security first**: Always prioritize security over features

## 💬 How to Invoke

```bash
# In project root (Hypercom/)
# Claude will see this CLAUDE.md automatically

# Example invocations:
# "AUDIT the security"
# "IDENTIFY critical issues"
# "FIX all CRITIQUE security findings"
# "DOCUMENT the architecture"
# "TEST all the fixes"
# "DEPLOY and prepare for OSS"
```

## 📝 Status Tracking

After each step, update status:

```markdown
# Status

- [x] SPEC.md created
- [x] PLAN.md created
- [ ] AUDIT started
- [ ] Security findings identified
- [ ] Critical fixes applied
- [ ] Documentation complete
- [ ] Tests passing
- [ ] Ready for OSS
```

