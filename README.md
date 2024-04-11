# Calculatrice en C (cal-c)

Une petite calculatrice simple et rapide écrite en C pour le terminal.

## Ce qu'elle sait faire

- Calculer des opérations classiques : addition, soustraction, multiplication, division.
- Gérer les parenthèses et les priorités de calcul (par exemple `2 + 3 * 4`).
- Gérer les puissances (`2^3`) et les fonctions usuelles comme `sqrt`, `sin`, `cos`, `abs`, `min`, `max`.
- Deux modes :
  - Mode normal (nombres à virgule).
  - Mode exact (fractions simplifiées, comme `1/3 + 1/6 = 1/2`).
- Afficher clairement où se trouve l'erreur si on tape une formule incorrecte.

## Comment compiler et lancer

Il faut avoir un compilateur C (comme `gcc` ou `clang`) et `cmake`.

```bash
# 1. Compiler le projet
cmake -B build
cmake --build build

# 2. Faire un calcul direct
./build/calc "2 + 2"

# 3. Lancer le mode interactif
./build/calc
```

Dans le mode interactif, vous pouvez taper vos calculs directement. Pour quitter, tapez `:quit`.
