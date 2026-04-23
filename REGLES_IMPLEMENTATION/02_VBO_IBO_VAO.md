# VBO, IBO et VAO

## VBO — Vertex Buffer Object

Stocke les données des attributs de vertex en mémoire GPU.

### Création et remplissage (dans l'initialisation)

```cpp
GLuint VBO;
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, taille_en_octets, donnees, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER, 0);
```

### Utilisation (dans le rendu)

```cpp
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glVertexAttribPointer(location, nb_composants, GL_FLOAT, GL_FALSE, stride, (void*)offset);
glEnableVertexAttribArray(location);
glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
```

Quand un VBO est bindé, le dernier paramètre de `glVertexAttribPointer` est un **offset relatif** (pas une adresse absolue).

### Stockage compact (recommandé par le cours)

Stocker tous les attributs dans un seul VBO, entrelacés :

```
{X,Y,Z, NX,NY,NZ, U,V}, {X,Y,Z, NX,NY,NZ, U,V}, ...
```

Le stride est `sizeof(float) * nombre_total_composants` (ex: `8 * sizeof(float)` pour position+normale+UV).

Les offsets sont calculés composant par composant :
- position : `0`
- normale : `(void*)(3 * sizeof(float))`
- UV : `(void*)(6 * sizeof(float))`

## IBO — Index Buffer Object

Évite la duplication des sommets en utilisant des indices.

### Création

```cpp
GLuint IBO;
glGenBuffers(1, &IBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, taille, indices, GL_STATIC_DRAW);
```

### Utilisation

```cpp
glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_SHORT, nullptr);
```

Types possibles : `GL_UNSIGNED_BYTE` (8 bits), `GL_UNSIGNED_SHORT` (16 bits), `GL_UNSIGNED_INT` (32 bits).

## VAO — Vertex Array Object

Enregistre la configuration des attributs pour éviter de la refaire à chaque frame.

### Ce qu'un VAO enregistre

- Les appels à `glBindBuffer(GL_ARRAY_BUFFER, ...)` et `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ...)`
- Les appels à `glVertexAttribPointer(...)`
- Les appels à `glEnableVertexAttribArray(...)` / `glDisableVertexAttribArray(...)`

### Création et initialisation

```cpp
GLuint VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

glBindBuffer(GL_ARRAY_BUFFER, VBO);
glVertexAttribPointer(loc_pos, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
glEnableVertexAttribArray(loc_pos);
glVertexAttribPointer(loc_norm, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*4));
glEnableVertexAttribArray(loc_norm);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

glBindVertexArray(0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
```

### Erreur classique à éviter

Ne jamais unbind le VBO **avant** de unbind le VAO :

```cpp
// MAUVAIS
glBindBuffer(GL_ARRAY_BUFFER, 0);  // écrase la référence dans le VAO
glBindVertexArray(0);

// BON
glBindVertexArray(0);              // d'abord unbind le VAO
glBindBuffer(GL_ARRAY_BUFFER, 0);  // ensuite on peut unbind le VBO
```

### Rendu avec VAO

```cpp
glBindVertexArray(VAO);
glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_SHORT, nullptr);
```

Ou sans IBO :

```cpp
glBindVertexArray(VAO);
glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
```

## Destruction

Toujours libérer la mémoire en quittant :

```cpp
glDeleteBuffers(1, &VBO);
glDeleteBuffers(1, &IBO);
glDeleteVertexArrays(1, &VAO);
```
