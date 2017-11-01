# Devoir2Algo

TODO :
- Utilisation d'un masque à partir d'une image (pour le nuage de points) ;
=> Valeur aléatoire et test si à l'intérieur du masque => Ajout
- Tests sur points : si dans forme : verif si le nouveau point est trop proche d'un ancien (min(tout) > valeur) ;
- Calcul des distances ultrametriques à partir des valeurs rentrées en dur dans l'arbre ;
- DONC, la distance maximale sera = MAX(poids(arbre)) ;
- Ne pas oublier que le graph est complet ;
- Updater un tableau (n*n) de poids à chaque fois ;


Notes & idées
/**
Prim :
* ARBRE => On crée un arbre à partir du premier sommet (duquel on garde que le nom) ;
* ANCIEN => On garde tous les autres sommets pour en faire un arbre (on garde les arrêtes mêmes si elles sont sur l'arbre couvrant min) ;
* ---
* On prend toutes les arêtes de ANCIEN pointant vers un sommet de ARBRE
* On en sélectionne la minimale
* Le sommet reliant est ajouté (sans arête du tout)
* On ajoute l'arête reliante aux deux sommets
* ===> Réitère jusqu'à plus rester de sommets
*/

/**
Prim-Tas
* ARBRE => On crée un arbre à partir du premier sommet (duquel on garde que le nom) ;
* ANCIEN => On garde tous les autres sommets pour en faire un arbre (on garde les arrêtes mêmes si elles sont sur l'arbre couvrant min) ;
* MIN_HEAP => Vide
* ---
* On ajoute dans MIN_HEAP tous les sommets liés au nouveau sommet de ARBRE (PUSH)
* On ajoute le sommet au sommet de MIN_HEAP à ARBRE (POP + remove de ANCIEN)
* On ajoute l'arête reliante
* ===> Réitère jusqu'à plus rester de sommets
*/

/**
Kruskal
* ARBRE => On crée un arbre vide;
* LISTE => Liste des arêtes
* Les trie dans l'ordre croissant
* ---
1ère arête
* Ajoute les deux sommets à ARBRE (sans arête)
* Lie les deux par leur arête reliante
* pop l'arête min de LISTE
* ---
* Prend la nouvelle arête min de LISTE
* 
* Vérif si UN SEUL des deux sommets de l'arête fait partie de ARBRE* 
*	Si oui :
* 	Ajoute le nouveau sommet et l'arête reliante
* 	Si non :
* 	Ajoute les deux sommets à ARBRE & l'arête
* 
* Vérif si les deux sommets de l'arête font partie de ARBRE
* 	Si non :
* 	Ajoute le sommet et l'arête puis continue
* 	Si oui :
* 	Vérifie par un parcours en profondeur (par ses arêtes) si le sommet1 atteint sommet2 dans ARBRE
* 		Si trouvé :
* 		Ignore
* 		Si non :
* 		Ajoute l'arête
* ===> Réitère jusqu'à plus rester de sommets
*/

/**
Kruskal-Foret
* ARBRE => On crée un arbre vide;
* 
* TODO le reste
*/


// ETAPE 1 : Algorithme adapté
// ETAPE 2 : Listage des fonctions à faire (Pour faire "propre")
// ETAPE 3 : Faire fonctions
// ETAPE 4 : Faire tests

// ETAPE ? : Ultra métrique à partir des notes
// ETAPE ? : Représentation graphique
