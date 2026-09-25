# 📐 Plan d'Action - Hypercom OSS Ready

## Phase 1: Security Audit (CRITIQUE) 🔒

### 1.1 Crypto Review
- [ ] Valider Noise Protocol implementation (noise_handshake_*.cpp)
- [ ] Vérifier HKDF-SHA256 usage
- [ ] Check: secure_memory ne leaks pas
- [ ] Audit: X25519 key exchange
- [ ] Test vectors pour chaque fonction crypto

### 1.2 Input Validation
- [ ] Valider tous les `frame_codec.cpp` → byte_reader
- [ ] Check: text_field_codec ne permet pas buffer overflows
- [ ] Audit: message parsing (hello, auth, etc.)
- [ ] SQL injection check (sql_binder.cpp, repositories)

### 1.3 Secret Management
- [ ] Scan: pas de secrets en dur (private keys, tokens)
- [ ] Vérifier: ServerStaticKey stockage sûr
- [ ] Check: identity_store permissions (file mode 0600)
- [ ] Audit: dm_session_keys ephemeral cleanup

### 1.4 Network & DoS
- [ ] Valider rate_limiter.cpp logic
- [ ] Check: connection_socket handling (race conditions)
- [ ] Audit: event_loop security
- [ ] Test: malformed packets handling

**Deliverable**: `SECURITY_AUDIT.md` avec failles + severity

---

## Phase 2: Compliance OSS 📦

### 2.1 License & Metadata
- [ ] Choisir license (MIT/GPL/Apache?)
- [ ] Ajouter LICENSE.md
- [ ] Vérifier dépendances licenses (libsodium=ISC, sqlite=public domain)
- [ ] Ajouter SPDX headers aux fichiers

### 2.2 Secrets Cleanup
- [ ] Scan: aucun hardcoded password/token/key
- [ ] Vérifier: .gitignore covers keystore, .env, etc.
- [ ] Check: build artifacts ne contiennent pas secrets

### 2.3 Documentation
- [ ] README.md (features, build, usage)
- [ ] ARCHITECTURE.md (diagram, component roles)
- [ ] CONTRIBUTING.md (dev setup, PR process)
- [ ] SECURITY.md (responsible disclosure)
- [ ] CODE_OF_CONDUCT.md

### 2.4 Build & Deploy
- [ ] CMake generates compile_commands.json
- [ ] Build reproductible (same hash)
- [ ] Docker/container support (optional)
- [ ] CI/CD pipeline example (.github/workflows)

**Deliverable**: Dossier `/docs` complet + LICENSE

---

## Phase 3: Code Quality ✨

### 3.1 Architecture & Patterns
- [ ] Identifier code duplication (message handlers)
- [ ] Refactor patterns répétitifs
- [ ] Valider separation of concerns
- [ ] Check: no circular dependencies

### 3.2 Testing
- [ ] Review test coverage (target: 80%+)
- [ ] Ajouter integration tests (client-server)
- [ ] Crypto function tests (vectors)
- [ ] Fuzz testing setup (existing?)

### 3.3 Warnings & Static Analysis
- [ ] Fix compiler warnings (-Wall -Wextra -Werror)
- [ ] Run clang-tidy
- [ ] Run cppcheck
- [ ] Zero ASAN/UBSAN issues

**Deliverable**: Clean build, all tests passing

---

## Phase 4: Implementation Order

```
WEEK 1: Security Audit
  - Day 1-2: Crypto review + validation
  - Day 3-4: Input validation + DoS
  - Day 5: Secret scan + report

WEEK 2: Compliance & Docs
  - Day 1-2: License + cleanup
  - Day 3-4: README + Architecture
  - Day 5: CONTRIBUTING + CI/CD

WEEK 3: Quality & Deploy
  - Day 1-2: Code review + refactors
  - Day 3-4: Testing + warnings fix
  - Day 5: Final validation + release prep
```

---

## Prompt pour Claude (réutilisable)

```
Tu es un code reviewer pour Hypercom (réseau social chiffré E2E en C++20).

MISSION: Préparer Hypercom pour l'open source
PRIORITÉ: 
  1. Trouver failles sécurité (crypto, validation, secrets)
  2. Compliance OSS (license, docs, CI/CD)
  3. Code quality (patterns, tests, warnings)

FICHIERS À AUDITER: common/, server/, client/
DELIVERABLES: SECURITY_AUDIT.md, ARCHITECTURE.md, dossier /docs/

COMMANDE: [AUDIT|IDENTIFY|FIX|DOCUMENT|TEST|DEPLOY]
```

---

## Success Metrics

✅ Security Audit: 0 failles critiques non-fixées  
✅ OSS Ready: License + 5 docs complètes  
✅ Code Quality: 80%+ test coverage, 0 warnings  
✅ Ready to push: GitHub + CI/CD working  

