# Règles Générales d'Implémentation

## Règle n°1 — Code issu UNIQUEMENT des cours et TPs

Toute implémentation doit utiliser exclusivement les concepts, fonctions et techniques vus dans les documents suivants :

### Cours
- OpenGL - cours partie 01 (pipeline, shaders, VBO, IBO, VAO)
- OpenGL - présentation partie 02 (handles, shader objects, uniforms, attributs)
- TD OpenGL moderne 4A partie 01 (VBO, IBO, VAO pas à pas)
- Quelques précisions sur le pipeline OpenGL (versions GLSL, #version 330)
- Complément TP - Transformations et Projections (matrices monde, vue, projection)

### TPs
- TP OpenGL moderne 02 - rendu 3D simple (cube, depth test, culling, normales, dragon)
- TP Textures (glTexImage2D, stb_image, sampler2D, multi-texturing)
- TP illumination partie 01 (Lambert, Phong, Blinn-Phong, matériaux)
- TP gamma et ambient simple (correction gamma sRGB, illumination hémisphérique)
- TP à rendre - Préparation au Projet (caméra orbitale, LookAt, Blinn-Phong)

**Interdit** : ne jamais introduire de bibliothèque, technique ou concept qui n'apparaît pas dans ces documents.

---

## Règle n°2 — Aucun commentaire dans le code

Le code rendu ne doit contenir **aucun commentaire**. Ni `//`, ni `/* */`, ni `#`.

---

## Règle n°3 — Code le plus simple et naturel possible (Style humain)

- Pas de sur-ingénierie, pas d'abstractions inutiles, pas de classes C++ complexes.
- Écrire le code comme un étudiant qui applique directement ce qu'il a vu en cours.
- **Style très humain, aéré et "imparfait"** :
  - Éviter le code compacté à l'extrême (pas de longues lignes de code sur une seule ligne).
  - Ajouter des sauts de ligne réguliers pour séparer les blocs logiques.
  - Aligner le code C++ de manière "classique" (ex: accolades type Allman `{` à la ligne sur les fonctions).
  - Utiliser des noms de variables explicites mais simples (`worldDragon`, pas `mD`).
  - Tolérer des petites inconsistances naturelles d'espacement, pour ne pas paraître généré par une machine.
- Si le cours montre une façon de faire, utiliser exactement cette façon (tableaux de `float[16]`, calculs à la main, `struct` en C).

---

## Règle n°4 — Répondre aux attentes du TP

- Lire l'énoncé du TP et implémenter exactement ce qui est demandé
- Ne pas ajouter de fonctionnalités bonus sauf si explicitement demandé
- Respecter les noms de variables et structures suggérés dans les énoncés

---

## Règle n°5 — Stack technique

| Élément | Choix |
|---------|-------|
| API graphique | OpenGL 3.3 Core Profile |
| Version GLSL | `#version 330` (ou `#version 330 core`) |
| Fenêtrage | GLFW |
| Chargement images | stb_image (`stb/stb_image.h`) |
| Maths | Fonctions maison (pas de GLM sauf si vu en cours) |
| Plateforme | macOS avec `GLFW_OPENGL_FORWARD_COMPAT` |
